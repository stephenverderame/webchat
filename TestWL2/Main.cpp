#include <HttpClient.h>
#include <SSLSocket.h>
#include <File.h>
int main() {
	WinsockContext;
	HttpClient client(make_stream("http://www.cplusplus.com"));
	client.method("GET").put("Host", "www.cplusplus.com").put("Connection", "close").put("Accept", "text/html").put(AutoHttpHeaders::AcceptedEncodings);
	client.bufferHeaders();
	HttpResponse resp = client.getResponse();
	printf("%d ", resp.responseCode);
	std::cout << resp.responseMessage << "\n";
	if (resp.responseCode == HTTP::RespCode::ok) {
		client.getStream().seekg(0);
		while (client.getStream().available())
			std::cout << (char)client.getStream().get();
		std::cout << resp.content << "\n"; 
		resp.headers.for_each([](StreamView& s, StreamView& v) {
			std::cout << s << ": " << v << "\n";
			});
		File output("test.html");
		output << resp.content;

	}

}