#include <HttpClient.h>
#include <SSLSocket.h>
#include <File.h>
#include <JSON.h>
#include <string>
#include <StringMap.h>
#include <Listener.h>
#include <openssl.h>
int main() {
	File input("testjson.txt", FileMode::read);
	JSONObject j;
	input >> j;
	srand(clock());
	auto q1 = j.getObj("quiz");
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
		for(auto& p : resp.headers) {
			std::cout << p.first << ": " << p.second << "\n";
		}
		File output("test.html");
		output << resp.content;

	}
//	SocketListener sl(80, FDMethod::TCP);
	SSLListener sl("key.pem", "cert.pem", 443);
	while (true) {
		try {
			HttpClient client2(sl.accept());
			client2.response(HTTP::Resp::OK).put("Content-Type", "text/html").put("Content-Length", "13");
			client2.bufferHeaders();
			client2.getStream() << "<h1>Test</h1>";
			client2.send();
		} catch(StreamException & s){
			printf(s.what());
		}
	}


}