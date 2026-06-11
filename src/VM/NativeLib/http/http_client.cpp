#include "http_client.h"

#include <atomic>
#include <curl/curl.h>
#include <string>
#include <thread>

struct HttpContext
{
	CURL* curl;
	curl_slist* headers;
	std::string responseBody;
	long statusCode;
	std::atomic<bool> isDone = false;
	std::thread worker;

	HttpContext()
	{
		curl = curl_easy_init();
		headers = nullptr;
		statusCode = 0;
	}

	~HttpContext()
	{
		if (worker.joinable())
		{
			worker.join();
		}
		if (headers)
		{
			curl_slist_free_all(headers);
		}
		if (curl)
		{
			curl_easy_cleanup(curl);
		}
	}
};

static int64_t g_nextHttpHandle = 1;
static std::unordered_map<int64_t, std::unique_ptr<HttpContext>> g_httpContexts;

size_t WriteCallback(
	void* contents,
	const size_t size,
	const size_t nmemb,
	void* userp)
{
	static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
	return size * nmemb;
}

HttpContext* GetContextOrThrow(const int64_t handle)
{
	if (!g_httpContexts.contains(handle))
	{
		throw std::runtime_error("VM Error: Invalid or already destroyed HTTP handle!");
	}
	return g_httpContexts[handle].get();
}

Value NativeHttpInit(const std::vector<Value>&)
{
	const auto handle = g_nextHttpHandle++;

	auto ctx = std::make_unique<HttpContext>();
	curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, &ctx->responseBody);

	g_httpContexts[handle] = std::move(ctx);
	return handle;
}

Value NativeHttpSetOpt(const std::vector<Value>& args)
{
	const auto ctx = GetContextOrThrow(AsInt(args[0]));
	const auto optType = AsString(args[1]);
	const auto val = AsString(args[2]);

	if (optType == "URL")
	{
		curl_easy_setopt(ctx->curl, CURLOPT_URL, val.c_str());
	}
	else if (optType == "METHOD")
	{
		curl_easy_setopt(ctx->curl, CURLOPT_CUSTOMREQUEST, val.c_str());
	}
	else if (optType == "HEADER")
	{
		ctx->headers = curl_slist_append(ctx->headers, val.c_str());
	}
	else if (optType == "BODY")
	{
		curl_easy_setopt(ctx->curl, CURLOPT_POSTFIELDS, val.c_str());
	}
	else if (optType == "TIMEOUT")
	{
		const long timeoutMs = std::stol(val);
		curl_easy_setopt(ctx->curl, CURLOPT_TIMEOUT_MS, timeoutMs);
	}
	return {};
}

Value NativeHttpSend(const std::vector<Value>& args)
{
	const auto ctx = GetContextOrThrow(AsInt(args[0]));

	if (ctx->headers)
	{
		curl_easy_setopt(ctx->curl, CURLOPT_HTTPHEADER, ctx->headers);
	}
	const auto res = curl_easy_perform(ctx->curl);

	if (res != CURLE_OK)
	{
		ctx->statusCode = 0;
		ctx->responseBody = std::string(curl_easy_strerror(res));
	}
	else
	{
		curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &ctx->statusCode);
	}
	return {};
}

Value NativeHttpGetStatus(const std::vector<Value>& args)
{
	return GetContextOrThrow(AsInt(args[0]))->statusCode;
}

Value NativeHttpGetResponse(const std::vector<Value>& args)
{
	return MakeString(GetContextOrThrow(AsInt(args[0]))->responseBody);
}

Value NativeHttpCleanup(const std::vector<Value>& args)
{
	const auto handle = AsInt(args[0]);
	g_httpContexts.erase(handle);
	return {};
}

Value NativeHttpSendAsync(const std::vector<Value>& args)
{
	const auto ctx = GetContextOrThrow(AsInt(args[0]));
	if (ctx->headers)
	{
		curl_easy_setopt(ctx->curl, CURLOPT_HTTPHEADER, ctx->headers);
	}

	ctx->worker = std::thread([ctx]() {
		const auto res = curl_easy_perform(ctx->curl);

		if (res != CURLE_OK)
		{
			ctx->statusCode = 0;
			ctx->responseBody = std::string(curl_easy_strerror(res));
		}
		else
		{
			curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &ctx->statusCode);
		}
		ctx->isDone = true;
	});
	return {};
}

Value NativeHttpIsReady(const std::vector<Value>& args)
{
	const auto ctx = GetContextOrThrow(AsInt(args[0]));
	return ctx->isDone.load() ? 1 : 0;
}

Value NativeHttpAwait(const std::vector<Value>& args)
{
	const auto ctx = GetContextOrThrow(AsInt(args[0]));
	if (ctx->worker.joinable())
	{
		ctx->worker.join();
	}
	return Value{};
}
