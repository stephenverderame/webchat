#include <HttpClient.h>
#include <SSLSocket.h>
#include <File.h>
int main() {
	WinsockContext;
/*	StreamView test("ahfdhgudnsdgfufnduisgsgsevhjdhsgshfkehdbdjhbwhfbuhwbjkwbnKB", 0, strlen("ahfdhgudnsdgfufnduisgsgsevhjdhsgshfkehdbdjhbwhfbuhwbjkwbnKB"));
	auto s = test.find_mt("sev");
	auto t = test.sub(s, s + 3);
	std::cout << t << "\n";*/
	HttpClient client(std::make_unique<Socket>(Address("www.cplusplus.com"), FDMethod::TCP, 80));
	client.method("GET").put("Host", "www.cplusplus.com").put("Connection", "close").put("Accept", "text/html").put(AutoHttpHeaders::AcceptedEncodings);
	client.bufferHeaders();
	HttpResponse resp = client.getResponse();
	printf("%d ", resp.responseCode);
	std::cout << resp.responseMessage << "\n";
	if (resp.responseCode == 200) {
		std::cout << resp.content << "\n";
		resp.headers.for_each([](StreamView & s, StreamView & v) {
			std::cout << s << ": " << v << "\n";
			});
		File output("test.html");
		output << resp.content << std::flush;
		
	}

}