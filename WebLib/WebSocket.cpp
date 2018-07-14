#include "WebSocket.h"

fsize_t WebsockClient::decodeTotalFrameSize(char * frame)
{
	char secondByte = 0;
	memcpy_s(&secondByte, 1, frame + 1, 1);
	unsigned long long size = secondByte & 0b01111111;
	unsigned long long headerSize = 2 + 4;
	if (size == 126) {
		unsigned short length;
		memcpy_s(&length, 2, frame + 2, 2);
		size = ntohs(length);
		headerSize += 2;
	}
	else if (size == 127) {
		unsigned long long length;
		memcpy_s(&length, 8, frame + 2, 8);
		size = ntohll(length);
		headerSize += 8;
	}
	return size + headerSize;
}

int WebsockClient::decodeFrame(char * message, char *& output, fsize_t & payloadLength)
{
	char read;
	memcpy_s(&read, 1, message + 1, 1);
	unsigned long long size = read & 0b01111111;
	int lastByte = 2;
	if (size == 126) {
		unsigned short rr;
		memcpy_s(&rr, 2, message + 2, 2);
		size = ntohs(rr);
		lastByte = 4;
	}
	else if (size == 127) {
		unsigned long long data;
		memcpy_s(&data, 8, message + 2, 8);
		size = ntohll(data);
		lastByte = 10;
	}
	payloadLength = size + 1;
	union {
		char maskBytes[4];
		int maskNum;
	} mask;
	memcpy_s(&mask.maskNum, 4, message + lastByte, 4);
	output = new char[(size + 1)];
	lastByte += 4;
	for (int i = 0; i < size; i++) {
		output[i] = message[lastByte + i] ^ mask.maskBytes[i % 4];
	}
	output[size] = '\0';
	return 0;
}

int WebsockClient::sendData(char * payload, fsize_t payloadSize, int dataType, bool continues)
{
	char firstByte = (1 << 7) | dataType;
	if (continues)
		firstByte = dataType;
	char secondByte = 0;
	int sizeofsize = 0;
	if (payloadSize <= 125) {
		secondByte = payloadSize;
	}
	else if (payloadSize <= USHRT_MAX) {
		secondByte = 126;
		sizeofsize = 2;
	}
	else {
		secondByte = 127;
		sizeofsize = 8;
	}
	char * frame = new char[payloadSize + 2 + sizeofsize];
	memcpy_s(frame, 1, &firstByte, 1);
	memcpy_s(frame + 1, 1, &secondByte, 1);
	if (sizeofsize != 0) {
		if (sizeofsize == 2) {
			//				printf("Sizeofsize is 2: %d \n", payloadSize);
			unsigned short size = htons(payloadSize);
			memcpy_s(frame + 2, sizeofsize, &size, sizeofsize);
		}
		else {
			unsigned long long size = htonll(payloadSize);
			memcpy_s(frame + 2, sizeofsize, &size, sizeofsize);
		}
	}
	memcpy_s(frame + 2 + sizeofsize, payloadSize, payload, payloadSize);

	size_t dataSent = 0;
	size_t ds;
	while (dataSent < 2 + sizeofsize + payloadSize) {
		ds = send(sock, frame + dataSent, (2 + sizeofsize + payloadSize) - dataSent, NULL);
		if (ds == SOCKET_ERROR) {
			delete[] frame;
			return WSAGetLastError();
		}
		dataSent += ds;
	}
	delete[] frame;
	return 0;
}

int WebsockClient::readData(char *& output, fsize_t & payloadLength, int & opcode)
{
	if (sock == INVALID_SOCKET) return -32;
	u_long package_size;
	fsize_t dataRead = 0;
	fsize_t dr = 0;
	fsize_t firstLength = 0;
	if (ioctlsocket(sock, FIONREAD, &package_size) == SOCKET_ERROR) {
		printf("Error reading: %d \n", WSAGetLastError());
		return WSAGetLastError();
	}
	if (package_size <= 0) return 1;
	char * buf = new char[package_size + 1];
	dr = recv(sock, buf, package_size, NULL);
	if (dr == SOCKET_ERROR) {
		delete[] buf;
		return WSAGetLastError();
	}
	*(buf + package_size) = '\0';
	fsize_t actualSize = decodeTotalFrameSize(buf);
	if (package_size < actualSize) {
		//			printf("Read size less than %u \n", actualSize);
		char * backBuffer = new char[package_size + 1];
		memcpy_s(backBuffer, package_size + 1, buf, package_size + 1);
		delete[] buf;
		buf = new char[actualSize + 1];
		memcpy_s(buf, actualSize, backBuffer, package_size);
		delete[] backBuffer;
		dataRead = package_size;
		dr = 0;
		while (dataRead < actualSize) {
			dr = recv(sock, buf + dataRead, actualSize - dataRead, NULL);
			if (dr == SOCKET_ERROR) {
				delete[] buf;
				return WSAGetLastError();
			}
			else if (dr == 0) break;
			dataRead += dr;
		}
		buf[actualSize] = '\0';

	}
	if (char opcode = *buf & 0b00001111 == ws_ping) {
		char keep = *buf >> 4;
		opcode = ws_pong;
		char firstByte = (keep << 4) | opcode;
		memcpy_s(buf, 1, &firstByte, 1);
		size_t dataSent = 0;
		size_t ds;
		while (dataSent < package_size) {
			ds = send(sock, buf + dataSent, package_size - dataSent, NULL);
			if (ds == SOCKET_ERROR) {
				delete[] buf;
				return WSAGetLastError();
			}
			dataSent += ds;
		}
		delete[] buf;
		return ws_ping;
	}
	else if ((*buf & 0b00001111) == ws_close) {
		//			printf("Close frame recieved! \n");
		size_t dataSent = 0;
		size_t ds;
		while (dataSent < package_size) {
			ds = send(sock, buf + dataSent, package_size - dataSent, NULL);
			if (ds == SOCKET_ERROR) {
				delete[] buf;
				return WSAGetLastError();
			}
			dataSent += ds;
		}
		closeConnection();
		delete[]  buf;
		return ws_close;
	}
	else if ((*buf & 0b00001111) == ws_pong) {
		//			printf("Pong! \n");
		return ws_pong;
	}
	if (package_size > 0) {
		decodeFrame(buf, output, firstLength);
		payloadLength = firstLength;
		opcode = *buf & 0b00001111;
	}
	else return 1;

	char fin = (*buf) >> 7;
	if (!fin) {
		FD_SET tempRead;
		size_t totalLength = firstLength - 1;
		char * combinedPayloads = new char[totalLength];
		memcpy_s(combinedPayloads, totalLength, output, totalLength);
		while (fin != 1) {
			FD_ZERO(&tempRead);
			FD_SET(sock, &tempRead);
			select(0, &tempRead, NULL, NULL, NULL);

			package_size = 0;
			ioctlsocket(sock, FIONREAD, &package_size);
			char * contBuf = new char[package_size];
			dataRead = 0;
			dr = recv(sock, contBuf, package_size, NULL);
			if (dr == SOCKET_ERROR) {
				delete[] contBuf;
				return WSAGetLastError();
			}
			fsize_t actualSize = decodeTotalFrameSize(contBuf);
			if (package_size < actualSize) {
				char * backBuffer = new char[package_size];
				memcpy_s(backBuffer, package_size, contBuf, package_size);
				delete[] contBuf;
				contBuf = new char[actualSize];
				memcpy_s(contBuf, actualSize, backBuffer, package_size);
				delete[] backBuffer;
				dataRead = package_size;
				dr = 0;
				while (dataRead < actualSize) {
					dr = recv(sock, contBuf + dataRead, actualSize - dataRead, NULL);
					if (dr == SOCKET_ERROR) {
						delete[] contBuf;
						return WSAGetLastError();
					}
					else if (dr == 0) break;
					dataRead += dr;
				}
			}
			char * payload;
			fsize_t payloadLength = 0;
			decodeFrame(contBuf, payload, payloadLength);
			payloadLength--;
			char * backBuffer = new char[totalLength];
			memcpy_s(backBuffer, totalLength, combinedPayloads, totalLength);
			delete[] combinedPayloads;

			combinedPayloads = new char[totalLength + payloadLength];
			memcpy_s(combinedPayloads, totalLength, backBuffer, totalLength);
			memcpy_s(combinedPayloads + totalLength, payloadLength, payload, payloadLength);
			fin = contBuf[0] >> 7;
			totalLength += payloadLength;
			delete[] backBuffer;
			delete[] contBuf;
			delete[] payload;
			if (fin) break;
		}
		delete[] output;
		output = new char[totalLength + 1];
		memcpy_s(output, totalLength, combinedPayloads, totalLength);
		(output)[totalLength] = '\0';
		delete[] combinedPayloads;
		payloadLength = totalLength;
	}
	delete[] buf;
	return 0;
}

WebsockClient::WebsockClient(SOCKET sock, SOCKADDR_IN data)
{
	this->sock = sock;
	this->addrData = data;
}

WebsockClient::WebsockClient(SOCKET sock)
{
	this->sock = sock;
}

int WebsockClient::handshake(std::string acceptedProtocols)
{
	u_long size;
	FD_SET tempRead;
	FD_ZERO(&tempRead);
	FD_SET(sock, &tempRead);
	timeval time;
	time.tv_sec = 5;
	time.tv_usec = 0;
	if (select(0, &tempRead, NULL, NULL, &time) == SOCKET_ERROR)
		return WSAGetLastError();
	if (ioctlsocket(sock, FIONREAD, &size) == SOCKET_ERROR)
		return WSAGetLastError();
	char * buffer = new char[size + 1];
	size_t dataRead = 0;
	while (dataRead < size) {
		size_t r = recv(sock, buffer + dataRead, size - dataRead, NULL);
		if (r == SOCKET_ERROR) return WSAGetLastError() + 1000000;
		dataRead += r;
	}
	*(buffer + size) = '\0';
	std::string str(buffer);
	printf("Received:\n%s \n", buffer);
	delete[] buffer;
	if (std::regex_search(str, std::regex("GET"))) {
		if (!std::regex_search(str, std::regex("Upgrade: websocket"))) {
			return WS_NOT_UPGRADE;
		}
		const char * eol = "\r\n";
		char response[3000];
		std::regex r("Sec-WebSocket-Key: (.*)");
		std::smatch matches;
		if (!std::regex_search(str, matches, r)) return -2;
		std::string m = matches[1].str();
		std::regex whitespace("[[:space:]]");
		m = std::regex_replace(m, whitespace, "");
		printf("\n Found key: %s \n", m.c_str());
		char encoding[1500];
#ifdef _WIN32
		sprintf_s(encoding, 1500, "%s%s", m.c_str(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
#else
		sprintf(encoding, "%s%s", m.c_str(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
#endif
		char proc[100];
		if (acceptedProtocols.size() > 0) 
		{
			std::regex rr("Sec-WebSocket-Protocol: (.*)");
			std::smatch matches;
			std::regex_search(str, matches, rr);
			protocol = "none";
			for (int i = 1; i < matches.size(); i++) {
				std::string procS = matches[i].str();
				if (matches[i].str().find(',')) {
					procS = matches[i].str().substr(0, matches[i].str().find(','));
				}
				std::regex procAccept(procS);
				if (std::regex_search(acceptedProtocols, procAccept) && acceptedProtocols.length() > 0) {
					sprintf_s(proc, 100, "Sec-WebSocket-Protocol: %s", procS.c_str());
					protocol = procS;
				}
				else if (acceptedProtocols.length() > 0) {
					char * forbidden = "HTTP/1.1 403 Forbidden\r\n\r\n";
					send_s(forbidden);
					return -5;
				}
			}
		}
		char result[21];
		SHA1(result, encoding, strlen(encoding));
		std::string str(result);
		std::string encode = base64_encode(str);
		if (strlen(proc) > 0 && acceptedProtocols.size() > 0) {
			sprintf_s(response, 3000, "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Accept: %s\r\n%s\r\n\r\n", encode.c_str(), proc);
			printf("\n \n Protocol Response \n \n");
		}
		else
			sprintf_s(response, 3000, "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Accept: %s\r\n\r\n", encode.c_str());
		if(send_s(response) == SOCKET_ERROR)
			return WSAGetLastError();
		printf("Sent Response:\n%s \n", response);
	}
	else {
		return -1;
	}
	return 0;
}
WebsockClient WebsockListener::accept(int & error)
{
	unsigned int size = sizeof(addrData);
	printf("About to accept client \n");
	SOCKET connection = ::accept(sock, (SOCKADDR*)&addrData, &size);
	if (connection != INVALID_SOCKET) {
		WebsockClient client(connection);
		error = client.handshake(acceptedProtocols);
		return client;

	}
	else
		error = -894;
	return WebsockClient();
}

