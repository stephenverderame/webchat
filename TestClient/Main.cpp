#include <Http.h>
int main() {
	Winsock context;
//	openssl sslContext;
	while (true) {
		HttpFrame frame;
		HttpClient test("www.cplusplus.com");
		int error = test.connectClient();
		if (error != 0) printf("Connection error: %d :: %d \n", error, WSAGetLastError());
		frame.protocol = "GET";
		frame.file = "/";
		frame.headers["Connection"] = "close";
		frame.headers["Accept"] = "text/html";
		frame.headers["Accept-Language"] = "en-US,en;q=0.9";
		frame.headers["Host"] = "www.cplusplus.com";
		frame.composeRequest();
		//	printf("Sending: %s \n", frame.data.c_str());
		test.sendMessage(frame);
/*		FD testFd;
		testFd.clear();
		testFd.add(&test);
		FD::wait(&testFd);*/
		test.getMessage_b(frame);
		HtmlFile testFile(frame);
		testFile.saveFile("test.html");
		test.closeConnection();
		printf("Got page! \n");
		Sleep(5000);
	}

/*	HttpsClient https("stackoverflow.com");
	if(https.connectClient() != 0) printf("Connection error! \n");
	HttpFrame frame2;
	frame2.protocol = "GET";
	frame2.file = "/";
	frame2.headers["Connection"] = "close";
	frame2.headers["Accept"] = "text/html";
	frame2.headers["Accept-Language"] = "en-US,en;q=0.9";
	frame2.headers["Host"] = "stackoverflow.com";
	frame2.composeRequest();
	int code = https.sendMessage(frame2);
	if (code != 0) printf("Sending erorr %d \n", code);
	code = https.getMessage(frame2);
	if (code != 0) printf("Getting error %d \n", code);
	HtmlFile test2(frame2);
	test2.saveFile("test2.html");
	https.closeConnection();*/
	getchar();
	return 0;
}