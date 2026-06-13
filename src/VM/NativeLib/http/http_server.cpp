#include "http_server.h"
#include "../../../../lib/httplib.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unordered_map>

constexpr int MAX_PORT = 65535;

struct HttpServerContext
{
	std::unique_ptr<httplib::Server> server;
	std::thread serverThread;
	std::atomic<bool> running = false;
	int port{};

	std::queue<int64_t> pendingRequests;
	std::mutex queueMutex;
	std::condition_variable queueCv;

	HttpServerContext() = default;

	~HttpServerContext()
	{
		if (running)
		{
			if (server)
				server->stop();
			running = false;
		}
		if (serverThread.joinable())
		{
			serverThread.join();
		}
	}

	HttpServerContext(const HttpServerContext&) = delete;
	HttpServerContext& operator=(const HttpServerContext&) = delete;
};

struct HttpRequestContext
{
	httplib::Request req;
	httplib::Response res;
	bool responded = false;
	std::mutex respMutex;
	std::condition_variable respCv;
	int64_t serverHandle{};
};

static std::unordered_map<int64_t, std::shared_ptr<HttpServerContext>> g_servers;
static std::unordered_map<int64_t, std::shared_ptr<HttpRequestContext>> g_requests;
static std::mutex g_serversMutex;
static std::mutex g_requestsMutex;
static std::atomic<int64_t> g_nextServerHandle = 1;
static std::atomic<int64_t> g_nextRequestHandle = 1;

Value NativeHttpServerStart(const std::vector<Value>& args)
{
	const auto port = AsInt(args[0]);
	if (port <= 0 || port > MAX_PORT)
	{
		throw std::runtime_error("Invalid port number");
	}

	auto handle = g_nextServerHandle++;
	auto ctx = std::make_shared<HttpServerContext>();
	ctx->port = port;
	ctx->running = true;
	ctx->server = std::make_unique<httplib::Server>();

	auto handler = [weakCtx = std::weak_ptr(ctx), handle](const httplib::Request& req, httplib::Response& res) {
		const auto ctx = weakCtx.lock();
		if (!ctx)
		{
			res.status = 503;
			return;
		}

		const auto reqHandle = g_nextRequestHandle++;
		auto reqCtx = std::make_shared<HttpRequestContext>();
		reqCtx->req = req;
		reqCtx->serverHandle = handle;

		{
			std::lock_guard lock(g_requestsMutex);
			g_requests[reqHandle] = reqCtx;
		}

		{
			std::lock_guard lock(ctx->queueMutex);
			ctx->pendingRequests.push(reqHandle);
		}
		ctx->queueCv.notify_one();

		{
			std::unique_lock lock(reqCtx->respMutex);
			reqCtx->respCv.wait(lock, [&reqCtx] { return reqCtx->responded; });
			res = reqCtx->res;
		}

		{
			std::lock_guard lock(g_requestsMutex);
			g_requests.erase(reqHandle);
		}
	};

	// 2. Регистрируем этот обработчик на все пути (.*) и все популярные HTTP-методы
	ctx->server->Get(".*", handler);
	ctx->server->Post(".*", handler);
	ctx->server->Put(".*", handler);
	ctx->server->Patch(".*", handler);
	ctx->server->Delete(".*", handler);
	ctx->server->Options(".*", handler);

	ctx->serverThread = std::thread([ctx]() {
		ctx->server->listen("0.0.0.0", ctx->port);
		ctx->running = false;
		{
			std::lock_guard lock(ctx->queueMutex);
			ctx->queueCv.notify_all();
		}
	});

	{
		std::lock_guard lock(g_serversMutex);
		g_servers[handle] = std::move(ctx);
	}
	return handle;
}

Value NativeHttpServerStop(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	std::shared_ptr<HttpServerContext> ctx;
	{
		std::lock_guard lock(g_serversMutex);
		const auto it = g_servers.find(handle);
		if (it == g_servers.end())
		{
			throw std::runtime_error("Invalid server handle");
		}

		ctx = it->second;
		g_servers.erase(it);
	}

	if (ctx->running)
	{
		ctx->server->stop();
		if (ctx->serverThread.joinable())
		{
			ctx->serverThread.join();
		}
		ctx->running = false;
		{
			std::lock_guard lock(ctx->queueMutex);
			ctx->queueCv.notify_all();
		}
	}
	return {};
}

Value NativeHttpServerAccept(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	std::shared_ptr<HttpServerContext> ctx;
	{
		std::lock_guard lock(g_serversMutex);
		const auto it = g_servers.find(handle);
		if (it == g_servers.end())
		{
			throw std::runtime_error("Invalid server handle");
		}
		ctx = it->second;
	}

	std::unique_lock lock(ctx->queueMutex);
	ctx->queueCv.wait(lock, [&ctx] {
		return !ctx->pendingRequests.empty() || !ctx->running;
	});

	if (!ctx->running && ctx->pendingRequests.empty())
	{
		throw std::runtime_error("Server stopped while waiting for request");
	}

	int64_t reqHandle = ctx->pendingRequests.front();
	ctx->pendingRequests.pop();
	return reqHandle;
}

Value NativeHttpRequestMethod(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	std::shared_ptr<HttpRequestContext> reqCtx;
	{
		std::lock_guard lock(g_requestsMutex);
		const auto it = g_requests.find(handle);
		if (it == g_requests.end())
		{
			throw std::runtime_error("Invalid request handle");
		}
		reqCtx = it->second;
	}
	return MakeString(reqCtx->req.method);
}

Value NativeHttpRequestPath(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	std::shared_ptr<HttpRequestContext> reqCtx;
	{
		std::lock_guard lock(g_requestsMutex);
		const auto it = g_requests.find(handle);
		if (it == g_requests.end())
		{
			throw std::runtime_error("Invalid request handle");
		}
		reqCtx = it->second;
	}
	return MakeString(reqCtx->req.path);
}

Value NativeHttpRequestHeader(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	const auto name = AsString(args[1]);
	std::shared_ptr<HttpRequestContext> reqCtx;
	{
		std::lock_guard lock(g_requestsMutex);
		const auto it = g_requests.find(handle);
		if (it == g_requests.end())
		{
			throw std::runtime_error("Invalid request handle");
		}
		reqCtx = it->second;
	}
	if (reqCtx->req.has_header(name))
	{
		return MakeString(reqCtx->req.get_header_value(name));
	}
	return MakeString("");
}

Value NativeHttpRequestBody(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	std::shared_ptr<HttpRequestContext> reqCtx;
	{
		std::lock_guard lock(g_requestsMutex);
		const auto it = g_requests.find(handle);
		if (it == g_requests.end())
		{
			throw std::runtime_error("Invalid request handle");
		}
		reqCtx = it->second;
	}
	return MakeString(reqCtx->req.body);
}

Value NativeHttpRequestRespond(const std::vector<Value>& args)
{
	const auto reqHandle = AsInt(args[0]);
	const auto status = AsInt(args[1]);
	const auto body = AsString(args[2]);
	std::string contentType = "application/json";
	if (args.size() >= 4 && IsString(args[3]))
	{
		contentType = AsString(args[3]);
	}

	std::shared_ptr<HttpRequestContext> reqCtx;
	int64_t serverHandle = 0;
	{
		std::lock_guard lock(g_requestsMutex);
		const auto it = g_requests.find(reqHandle);
		if (it == g_requests.end())
		{
			throw std::runtime_error("Invalid request handle");
		}
		reqCtx = it->second;
		serverHandle = reqCtx->serverHandle;
	}

	{
		std::unique_lock lock(reqCtx->respMutex);
		reqCtx->res.status = status;
		reqCtx->res.set_content(body, contentType);
		reqCtx->responded = true;

		reqCtx->res.set_header("Access-Control-Allow-Origin", "*");
		reqCtx->res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
		reqCtx->res.set_header("Access-Control-Allow-Headers", "Content-Type");
	}
	reqCtx->respCv.notify_one();
	return serverHandle;
}
