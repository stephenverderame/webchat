# webchat
Cross platform C++ library implementing common internet protocols such as HTTP, HTTPS, SMTP, Websockets. Used to create a simple online chat website with a C++ backend.

This library simply implements common protocols on TCP/IP. The protocols supported are: HTTP, HTTPS, SMTP and Websockets.

## Usage:
* For Windows you must first initialize the RAII wrapper for Winsock. The `Winsock` class simply initializes and deinitializes the Winsock library in its constructor and destructor, respectively.
* When using an SSL protocol such as HTTPS, you must instantiate the RAII wrapper for Openssl using the `openssl` class

#### Classes
`HttpFrame` - encapsulates the HTTP headers and content of a request or a response
* `HttpFrame::protocol` - method (GET, POST, etc.)
* `HttpFrame::file` - path
* `HttpFrame::data` - entire request/response
* `HttpFrame::content` - the content (no headers)
* `HttpFrame::headers` - the HashMap containing headers and their values
* Before sending a request or a response you must call `HttpFrame::composeRequest()` or `HttpFrame::composeResponse()` respectively
`HttpListener` - binds a port and IP address
* returns clients with the `accept()` function

`HttpClient` - handles messages sent and received from a specific endpoint
* `HttpClient::getMessage_b()` - blocking mode of `getMessage()`
* `HttpClient::connectClient()` - must be called when not returned by an `HttpListener`, creates a connection to the specified endpoint
* `HttpClient::sendHtmlFile()` - sends an HTML File to the specified endpoint. Handles headers automatically
* `HttpClient::changeHost()` - makes a new connection to a different host
* Many HttpServers require the `"Host"` header to be set to the correct hostname

`FD` - encapuslates the `fd_set` to determine when a given Socket is ready for sending or receiving
* `FD:clear()` - clears all sockets' flags on the fd
* `FD::add()` - adds a socket to the FD. Can use on socket encapsulators such as Clients and Listeners
* `FD::inSet()` - determines if a specific socket encapsulator is set in the fd
* `FD::wait()` waits for the specified FDs

`SmtpClient` - sends messages over SMTP
* `SMTP::login()` - must be called prior to `send()`, saves and formats the user's credentials

`Listener` - binds a port
* `Listener::accept()` - returns a concrete client of the concrete listener upon a successful TCP handshake

`HttpsClient` and `HttpsListener` have the same interface as Http however:
* `HttpsListener` - must set the certificate with the `loadCert()` function
* `HttpsClient` - has no nonblocking read function 



Examples
HTTP Client:
```C++
HttpFrame frame;
		HttpClient test("www.cplusplus.com");
		int error = test.connectClient(); //must connect
		if (error != 0) printf("Connection error: %d :: %d \n", error, WSAGetLastError());
		frame.protocol = "GET";
		frame.file = "/";
		frame.headers["Connection"] = "close";
		frame.headers["Accept"] = "text/html";
		frame.headers["Accept-Language"] = "en-US,en;q=0.9";
		frame.headers["Host"] = "www.cplusplus.com";
		frame.composeRequest(); //remember to compose the request
		//printf("Sending: %s \n", frame.data.c_str());
		test.sendMessage(frame);
/*	FD testFd;
		testFd.clear();
		testFd.add(&test);
		FD::wait(&testFd);
    test.getMessage(frame)*/
		test.getMessage_b(frame);
		HtmlFile testFile(frame);
    
  ```
  
  #### For more information please see the examples
  * Note this library is currently being rewritten to support compression, be simpler and be more efficient
  
  
  #### WebChat Project:
  
  This project was a simple project to develope a website. The website is a simple chat room made using websockets
  The front-end is developed in html and javascript, and the backend is done using this weblib library and C++
  
