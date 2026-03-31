## TSOP - Typestate-Oriented Programming

* Экспериментальные языки: [Plaid](https://www.cs.cmu.edu/~aldrich/plaid/) и [Obsidian](https://obsidian-lang.com/) (из Университета Карнеги-Меллон)
* Статья с абзацем про TSOP: https://habr.com/ru/companies/productivity_inside/articles/327898/

```pseudocode
import http.{Request, Response, Url, Headers, Body}
import io.{Bytes, Console}

protocol HTTPConnection {
  state Idle {
    fn connect(url: String) -> Connecting
  }
  
  state Connecting {
    fn onResponse(handler: Response -> Unit) -> Connected
    fn onTimeout() -> Closed
  }
  
  state Connected {
    fn sendRequest(req: Request) -> WaitingResponse
    fn close() -> Closed
  }
  
  state WaitingResponse {
    fn onData(data: Bytes) -> WaitingResponse
    fn onComplete() -> Idle
    fn onError() -> Closed
  }
  
  state Closed { }
}

class HttpClient implements HTTPConnection {
  private var socket: TCPSocket? = null
  private var responseBuilder: ResponseBuilder = new ResponseBuilder()
  private var currentState: State = Idle()
  
  public fn HttpClient() {
    this.currentState = Idle()
  }

  state Idle {
    fn connect(url: String) -> Connecting {
      print("Connecting to ${url}...")
      this.socket = TCPSocket.connect(url, 80)
      return Connecting(this.socket)
    }
  }
  
  state Connecting(socket: TCPSocket) {
    fn onResponse(handler: Response -> Unit) -> Connected {
      socket.onConnect(() -> {
        print("Connected!")
        this.transitionTo(Connected(socket, handler))
      })
      
      return this
    }
    
    fn onTimeout() -> Closed {
      print("Connection timeout!")
      socket.close()
      return Closed()
    }
  }
  
  state Connected(socket: TCPSocket, responseHandler: Response -> Unit) {
    fn sendRequest(req: Request) -> WaitingResponse {
      print("Sending request: ${req.method} ${req.path}")
      
      val requestText = buildRequest(req)
      socket.write(requestText.getBytes())
      return WaitingResponse(socket, responseHandler, new ResponseBuilder())
    }
    
    fn close() -> Closed {
      print("Closing connection")
      socket.close()
      return Closed()
    }
  }
  
  state WaitingResponse(
    socket: TCPSocket, 
    responseHandler: Response -> Unit,
    builder: ResponseBuilder
  ) {
    fn onData(data: Bytes) -> WaitingResponse {
      builder.append(data)
      
      if builder.headersComplete() {
        val contentLength = builder.getContentLength()
        
        if builder.bodySize() >= contentLength {
          return this.onComplete()
        }
      }
      
      return this
    }
    
    fn onComplete() -> Idle {
      print("Response complete!")
      val response = builder.build()
      responseHandler(response)
      return Idle()
    }
    
    fn onError() -> Closed {
      print("Error during response")
      socket.close()
      return Closed()
    }
  }
  
  state Closed { }
   
  private fn buildRequest(req: Request): String {
    return """
      ${req.method} ${req.path} HTTP/1.1
      Host: ${req.host}
      User-Agent: TSOP-Client/1.0
      Accept: */*
    """
  }
}

public fn main() {
  val client = new HttpClient()
  
  // TSOP гарантирует, что мы не можем:
  // - client.sendRequest() в Idle (ошибка компиляции)
  // - client.onResponse() в Connected (ошибка компиляции)
  // - и так далее...
  
  val connection = client
    .connect("example.com")        // Idle -> Connecting
    .onResponse((response) -> {     // Connecting -> Connected (после установки)
      print("Got response: ${response.statusCode}")
      print("Body: ${response.body}")
    })
  
  // В этот момент connection всё ещё в Connecting, 
  // но onResponse зарегистрировал callback.
  // Когда соединение установится, клиент автоматически 
  // перейдёт в Connected и можно будет отправить запрос
  
  // Но для этого нам нужен способ получить Connected из callback.
  // Давайте добавим более удобный async/await синтаксис:
  
  // Асинхронный вариант с await
  async fn fetchExample() {
    val client = new HttpClient()
    
    val connected = await client
      .connect("example.com")
      .onResponse()
    
    val waiting = connected.sendRequest(
      Request(method: "GET", path: "/", host: "example.com")
    )
    val idle = await waiting
    
    print("Request completed")
  }
}

// 🔐 TSOP-безопасный вариант с явным указанием состояний
public fn typeSafeGet() {
  let client: HttpClient @Idle = new HttpClient()
  
  let connecting: HttpClient @Connecting = client.connect("example.com")
  
  let connectedOrClosed = connecting.onResponse(handleResponse)
  
  match connectedOrClosed {
    case connected: HttpClient @Connected => {
      let waiting = connected.sendRequest(createGetRequest())
    }
    case closed: HttpClient @Closed => {
      print("Connection failed")
    }
  }
}
```
