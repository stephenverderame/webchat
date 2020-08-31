#include <HttpClient.h>
#include <SSLSocket.h>
#include <File.h>
#include <JSON.h>
#include <string>
int main() {
	File input("testjson.txt", FileMode::read);
	JSONObject j;
	input >> j;
	srand(clock());
	auto o = j.getObj("quiz")->keyList();
	auto quiz = j.getObj("quiz")->getObj(o[rand() % o.size()]);
	o = quiz->keyList();
	auto q = quiz->getObj(o[rand() % o.size()]);
	std::cout << q->get("question") << "\n";
	auto choices = q->getArray("options");
	for (int i = 0; i < choices->size(); ++i) {
		std::cout << "\t" << choices->get(i) << "\n";
	}
	std::string a;
	std::getline(std::cin, a);
	if (q->get("answer") == a.c_str()) std::cout << "Correct!\n";
	else std::cout << "Wrong!\n";
	std::cout << j;
	WinsockContext;
	HttpClient client(make_stream("https://stackoverflow.com"));
	client.method("GET").put("Host", "stackoverflow.com").put("Connection", "close").put("Accept", "text/html").put(AutoHttpHeaders::AcceptedEncodings);
	client.bufferHeaders();
	HttpResponse resp = client.getResponse();
	printf("%d ", resp.responseCode);
	std::cout << resp.responseMessage << "\n";
	if (resp.responseCode == HTTP::RespCode::ok) {
//		while (client.getStream().available())
//			std::cout << (char)client.getStream().get();
		std::cout << resp.content << "\n"; 
		resp.headers.for_each([](StreamView& s, StreamView& v) {
			std::cout << s << ": " << v << "\n";
			});
		File output("test.html");
		output << resp.content;

	}

}