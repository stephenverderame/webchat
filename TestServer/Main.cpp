#include <WebSocket.h>
#include <Https.h>
#include <Smtp.h>
#include <map>
typedef unsigned int ID;
std::map<ID, WebsockClient> clients;
std::map<ID, std::string> usernames;
std::map<ID, std::string> images;
std::vector<std::string> chatLog;
ID nextId = 0;
int main() {
	Winsock data;
	openssl sslData;
	WebsockListener listener(8032);
	HttpListener https;
//	https.loadCert("key.pem", "cert.pem");
	https.nonblocking(ENABLE);
//	https.nagle(DISABLE);
	FD read;
	HtmlFile chatFile("chat.html");
	HtmlFile bugFile("bug.html");
	printf("Server started! \n");
	while (true) {
		read.clear();
		read.add(&listener);
		read.add(&https);
		for (auto it = clients.begin(); it != clients.end(); it++)
			read.add(&(*it).second);

		FD::wait(&read);
//		printf("Data available \n");
		if (read.inSet(&listener)) {
			int code;
			WebsockClient client = listener.accept(code);
			if (code == 0) {
				printf("%s connected!\n", client.getIpAddress().c_str());
				clients.insert(std::pair<ID, WebsockClient>(nextId, client));
				srand(clock());
				int num = rand() % 10000;
				char user[15];
				sprintf_s(user, 15, "Guest%d", num);
				usernames.insert(std::make_pair(nextId, std::string(user)));
				printf("%d client \n", nextId);
				client.nagle(DISABLE);
				images.insert(std::make_pair(nextId, std::string("http://profilepics.xyz/wp-content/uploads/2018/01/Life-profile-pics-latest.png")));
				for (auto it = images.begin(); it != images.end(); it++) {
					if ((*it).first == nextId) continue;
					std::string message = "U2V0IFBpY3R1cmU6 " + std::to_string((*it).first) + " " + (*it).second;
					if (client.sendData((char*)message.data(), message.size(), ws_text) != 0)
						printf("Error: %d \n", WSAGetLastError());
				}
				for (auto it = chatLog.begin(); it != chatLog.end(); it++) {
					if(client.sendData((char*)(*it).data(), (*it).size(), ws_text) != 0)
						printf("Error2 %d \n", WSAGetLastError());
				}
				for (auto it = clients.begin(); it != clients.end(); it++) {
					if ((*it).first == nextId) continue;
					std::string msg = std::to_string(-1) + "#A new user has connected!";
					if((*it).second.sendData((char*)msg.c_str(), msg.size(), ws_text) != 0)
						printf("Error3 %d \n", WSAGetLastError());
				}
				nextId++;
				printf("End 1 \n");
			}
			else {
				printf("Could not accept websock listener with code: %d and %d \n", code, data.lastError());
			}
		}
		if (read.inSet(&https)) {
			printf("Https data \n");
			int code;
			HttpClient connection = https.accept(code);
			if (code == 0) {
				FD tempFd;
				tempFd.clear();
				tempFd.add(&connection);
				timeval t;
				t.tv_sec = 5; t.tv_usec = 0;
				FD::waitUntil(&t, &tempFd); 
				HttpFrame frame;
//				printf("Getting ssl message \n");
				code = connection.getMessage(frame);
				if (code == 0) {
					printf("Received:\n%s\n", frame.data.c_str());
					if (frame.protocol == "GET" && frame.file == "/" && frame.headers["Accept"].find("text/html") != std::string::npos)
						connection.sendHtmlFile(chatFile);
					else if (frame.protocol == "GET" && frame.file == "/bug" && frame.headers["Accept"].find("text/html") != std::string::npos)
						connection.sendHtmlFile(bugFile);
					else if (frame.protocol == "POST" && frame.file == "/bug") {
						std::string subject = frame.data.substr(frame.data.find("subject=") + 8);
						subject = subject.substr(0, subject.find("&"));
						for (int i = 0; i < subject.size(); i++) {
							if (subject[i] == '+')
								subject[i] = ' ';
						}
						printf("Sub: %s \n", subject.c_str());
						std::string msg = frame.data.substr(frame.data.find("subject=") + 8);
						msg = msg.substr(msg.find("mainText=") + 9);
						for (int i = 0; i < msg.size(); i++) {
							if (msg[i] == '+')
								msg[i] = ' ';
						}
						printf("Msg: %s \n", msg.c_str());
						SmtpClient emailClient(GMAIL_HOST, GMAIL_PORT);
						emailClient.login("email@gmail.com", "password omitted for obvious reasons");
						code = emailClient.sendMessage("email@gmail.com", (char*)subject.c_str(), (char*)msg.c_str());
						if (code != 0) printf("Email error: %d \n", code);
						HttpFrame response;
						response.responseCode = "200 OK";
						response.composeResponse();
						connection.sendMessage(response);
					}
				}
				else {
					printf("Reading error %d \n", code);
					printf("Got error %d \n", ERR_get_error());
					printf("WinsockError %d \n", WSAGetLastError());
				}

			}
			else {
				printf("Accept failure %d \n", code);
			}
			connection.closeConnection();
		}
		for (auto it = clients.begin(); it != clients.end(); it++) {
			ID i = (*it).first;
			WebsockClient client = (*it).second;
			if (client.isClosed()) { printf("Closed \n"); continue; }
			if (read.inSet(&client)) {
				printf("Something to read on %d \n", (*it).first);
				char * msg;
				fsize_t msgSize;
				int opcode;
				int code = client.readData(msg, msgSize, opcode);
				if (WS_OK(code) && !WS_CONTROL(code)) {
					if (std::regex_search(std::string(msg), std::regex(base64_encode("Set Username:")))) {
						std::smatch username;
						std::regex r("U2V0IFVzZXJuYW1lOg== (.*)");
						std::string s(msg);
						std::regex_search(s, username, r);
						printf("Username of %d: %s \n", i, username[1].str().c_str());
						usernames[i] = username[1].str(); 
						delete[] msg;
						continue;

					}
					else if (std::regex_search(std::string(msg), std::regex("^(U2V0IFBpYzo=)"))) {
						if (std::string(msg).size() > 1000000) {
							printf("Profile pic from %d is too big! \n", i);
							delete[] msg;
							continue;
						}
						printf("Image recieved from: %d \n", i);
						std::string data = std::string(msg).substr(std::string(msg).find('#') + 1, std::string(msg).size());
						images[i] = data;
						std::string message = "U2V0IFBpY3R1cmU6 " + std::to_string(i) + " " + images[i];
						printf("Sending back image \n");
						for (auto itt = clients.begin(); itt != clients.end(); itt++) {
							if ((*itt).second.isClosed()) continue;
							if ((*itt).first != i) {
								(*itt).second.sendData((char*)message.data(), message.size(), ws_text);

							}
						}
						delete[] msg;
						continue;

					}
					std::string message = std::to_string(i) + "#" + usernames[i] + ": " + msg;
					if (message.size() > 10000000) {
						printf("Message from %d is too big! \n", i);
						delete[] msg;
						continue;
					}
					else if (!std::regex_search(msg, std::regex("^data:(image)|(video)\/")) && message.size() > 1000) {
						printf("Txt message from %d is too big! \n", i);
						delete[] msg;
						continue;
					}
					printf("Message from %d \n", i, message.c_str());
					chatLog.push_back(message);
					for (auto itt = clients.begin(); itt != clients.end(); itt++) {
						if ((*itt).second.isClosed()) continue;
						if ((*itt).first != i) {
							(*itt).second.sendData((char*)message.data(), message.size(), ws_text);
						}
					}
					delete[] msg;
				}
				else if (code == ws_close) {
					clients[i].closeConnection();
					clients.erase(i);
					//					images.erase(i);
					printf("Client %d disconnected! \n", i);
					if (clients.size() > 0) {
						printf("kgj \n");
						for (auto it = clients.begin(); it != clients.end(); it++) {
							if ((*it).first == i) continue;
							std::string msg = std::to_string(-1) + "#" + usernames[i] + " has disconnected!";
							(*it).second.sendData((char*)msg.c_str(), msg.size(), ws_text);
						}
					}
					usernames.erase(i);
					break;
				}
				else if (!WS_OK(code)) {
					printf("Error code: %d \n", code);
					(*it).second.closeConnection();
					clients.erase(i);
					usernames.erase(i);
				}
			}
		}

	}
	chatFile.del();
	bugFile.del();
	return 0;
}