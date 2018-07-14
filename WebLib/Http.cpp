#include "Http.h"

void HtmlFile::loadFile(const char * filename)
{
	FILE * file;
	if (fopen_s(&file, filename, "rb") == 0) {
		fseek(file, 0, SEEK_END);
		size_t fsize = ftell(file);
		fseek(file, 0, SEEK_SET);
		data = new char[fsize + 1];
		fread(data, 1, fsize, file);
		data[fsize] = '\0';
		this->size = fsize;
		fclose(file);

	}
}

HtmlFile::HtmlFile(HttpFrame frame)
{
	data = new char[frame.content.size() + 1];
	memcpy_s(data, frame.content.size(), frame.content.c_str(), frame.content.size());
	data[frame.content.size()] = '\0';
	size = frame.content.size();
	printf("Html file size: %d \n", size);
}

void HtmlFile::saveFile(const char * filename)
{
	FILE * file;
	if (fopen_s(&file, filename, "wb") == 0) {
		fwrite(data, 1, size, file);
		fclose(file);
	}
}

HttpClient::HttpClient(char * hostname, unsigned short port)
{
	hostent * server = gethostbyname(hostname);
	if (!server) printf("Cannot find host \n");
	sock = socket(AF_INET, SOCK_STREAM, NULL);
	addrData.sin_family = AF_INET;
	addrData.sin_port = htons(port);
	addrData.sin_addr = *((LPIN_ADDR)*server->h_addr_list);
}

int HttpClient::getMessage(HttpFrame & frame)
{
	size_t size;
	if (ioctlsocket(sock, FIONREAD, (u_long*)&size) == SOCKET_ERROR) return -1;
//	printf("Receiving msg of size: %d \n", size);
	if (size <= 0) return -3;
	char * buffer = new char[size + 1];
	if (recv_s(&buffer, size) == SOCKET_ERROR) return -2;
	buffer[size] = '\0';
	if (std::string(buffer).find("Content-Length:") != std::string::npos) {
		std::string msg(buffer);
		std::string s = msg.substr(msg.find("Content-Length:") + 15);
		s = s.substr(0, s.find("\r\n"));
		int cLength = std::stoi(s);
//		printf("Size: %s --> %d \n", s.c_str(), cLength);
		size_t headerSize = msg.find("\r\n\r\n") + 4;
		if (headerSize == std::string::npos) printf("Can't find double space \n");
		if (size - headerSize < cLength) {
			char * missing = new char[cLength - (size - headerSize)];
			if (recv_s(&missing, cLength - (size - headerSize)) == SOCKET_ERROR) return -6;
//			printf("Was missing: %d bytes \n", cLength - (size - headerSize));
			char * newBuf = new char[size + (cLength - (size - headerSize)) + 1];
			memcpy_s(newBuf, size + (cLength - (size - headerSize)), buffer, size);
			memcpy_s(newBuf + size, cLength - (size - headerSize), missing, cLength - (size - headerSize));
			newBuf[size + (cLength - (size - headerSize))] = '\0';
//			printf("%s \n", newBuf);
			delete[] buffer;
			buffer = newBuf;
			delete[] missing;
		} 
		char * content = new char[cLength + 1];
		memcpy_s(content, cLength, buffer + headerSize, cLength);
		content[cLength] = '\0';
		frame.content = content;
		frame.data += frame.content;
		delete[] content;
	}
	else if (std::string(buffer).find("Transfer-Encoding: chunked") != std::string::npos) {
		std::string msg(buffer);
		std::string s = msg.substr(msg.find("\r\n\r\n") + 4);
		frame.content = s;
		s = s.substr(0, s.find("\r\n"));
		std::stringstream ss;
		ss << std::hex << s;
		size_t chunkSize;
		ss >> chunkSize;
		size_t headerSize = msg.find("\r\n\r\n") + 4;
//		printf("Headersize: %d. Chunksize: %d (%s) \n", headerSize, chunkSize, s.c_str());
		frame.content = frame.content.substr(frame.content.find("\r\n") + 2);
		if (size - headerSize - s.size() - 2 < chunkSize) {
//			printf("Missing from chunk 1 \n");
			char * firstMissing = new char[chunkSize - (size - headerSize - s.size() - 2) + 1];
			recv_s(&firstMissing, chunkSize - (size - headerSize - s.size() - 2));
			firstMissing[chunkSize - (size - headerSize - s.size() - 2)] = '\0';
			frame.content += firstMissing;
			delete[] firstMissing;
		}
		FD_SET tempSet;
		while (chunkSize != 0) {
			FD_ZERO(&tempSet);
			FD_SET(sock, &tempSet);
			select(0, &tempSet, 0, 0, 0);
			u_long msize;
			ioctlsocket(sock, FIONREAD, &msize);
			char * chunk = new char[msize + 1];
			recv(sock, chunk, msize, NULL);
			chunk[msize] = '\0';
//			printf("Got msg: %d bytes: %s \n", msize, chunk);
			ss.clear();
			std::string textSize = std::string(chunk).substr(0, std::string(chunk).find("\r\n"));
			ss << std::hex << textSize;
			ss >> chunkSize;
			if (textSize.empty()) chunkSize = 0;
//			printf("Chunk size: %d (%s)\n", chunkSize, textSize.c_str());
			frame.content += chunk;
			if (msize - textSize.size() - 2 < chunkSize) {
				char * cMissing = new char[chunkSize - (msize - textSize.size() - 2) + 1];
				recv_s(&cMissing, chunkSize - (msize - textSize.size() - 2));
				cMissing[chunkSize - (msize - textSize.size() - 2)] = '\0';
				frame.content += cMissing;
				delete[] cMissing;
			}
			delete[] chunk;
		}
		frame.data += frame.content;
	}
	frame.load(buffer);
	delete[] buffer;
	return 0;
}

int HttpClient::sendHtmlFile(HtmlFile file)
{
	std::string message = "Http/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " + std::to_string(file.size) + "\r\nContent-Type: text/html\r\n\r\n" + file.data;
	if (send_s((char*)message.c_str(), message.size()) == SOCKET_ERROR) return SOCKET_ERROR;
	return 0;
}

int HttpClient::sendMessage(HttpFrame msg)
{
	if(send_s((char*)msg.data.data()) == SOCKET_ERROR)
		return SOCKET_ERROR;
	return 0;
}

int HttpClient::sendMessage(char * rawMsg)
{
	if (send_s(rawMsg) == SOCKET_ERROR)
		return SOCKET_ERROR;
	return 0;
}

void HttpClient::changeHost(char * hostname, unsigned short port)
{
	hostent * server = gethostbyname(hostname);
	addrData.sin_port = htons(port);
	addrData.sin_addr = *((LPIN_ADDR)*server->h_addr_list);
	sock = socket(AF_INET, SOCK_STREAM, NULL);
}

int HttpClient::connectClient()
{
	return connect(sock, (SOCKADDR*)&addrData, sizeof(addrData));
}

void HttpFrame::load(char * frame)
{
	data = frame;
	if (data.size() <= 3) return;
	protocol = data.substr(0, data.find('/') - 1);
	file = data.substr(data.find('/'));
	file = file.substr(0, file.find(' '));
	std::regex header(".+:.+");
	std::smatch match;
	std::string::const_iterator searchStart(data.cbegin());
	while (std::regex_search(searchStart, data.cend(), match, header)) {
		std::string label = match[0].str().substr(0, match[0].str().find(':'));
		std::string line = match[0].str();
		std::string data = line.substr(line.find(':') + 2);
		headers[label] = data;
		searchStart += match.position() + match.length();
	}

}

void HttpFrame::composeResponse()
{
	data = "Http/1.1 " + responseCode + "\r\n";
	for (auto it = headers.begin(); it != headers.end(); it++) {
		data += (*it).first + ": " + (*it).second + "\r\n";
	}
	data += "\r\n\r\n";
	if (content.size() > 0) data += content;

}
void HttpFrame::composeRequest() {
	data = protocol + " " + file + " Http/1.1\r\n";
	for (auto it = headers.begin(); it != headers.end(); it++) {
		data += (*it).first + ": " + (*it).second + "\r\n";
	}
	data += "\r\n\r\n";
	if (content.size() > 0) data += content;
}
HttpClient HttpListener::accept(int & error)
{

	unsigned int size = sizeof(addrData);
	SOCKET connection = ::accept(sock, (SOCKADDR*)&addrData, &size);
	if (connection == INVALID_SOCKET) error = -1;
	else error = 0;
	return HttpClient(connection);
}
