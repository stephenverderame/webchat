#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H
/*libcryptoMD.lib
libcryptoMDd.lib
libcryptoMT.lib
libcryptoMTd.lib
libsslMD.lib
libsslMDd.lib
libsslMT.lib
libsslMTd.lib*/
#include "Base.h"
#include "SHA1.h"
#include <regex>
#include "Base64.h"
#define WS_CONTROL(code) ((code) == 8 || (code) == 9 || (code) == 10)
#define WS_OK(code) ((code) == 0 || WS_CONTROL(code))
#define WS_NOT_UPGRADE -809
enum WebSocketOpcodes {
	ws_continuation,
	ws_text,
	ws_binary,
	ws_close = 8,
	ws_ping,
	ws_pong
};
class WebsockClient : public Client {
	friend class WebsockListener;
private:
	std::string protocol;
private:
	fsize_t decodeTotalFrameSize(char * frame);
	int decodeFrame(char * message, char *& output, fsize_t & payloadLength);
	int handshake(std::string acceptedProtocols);
public:
	int sendData(char * payload, fsize_t payloadSize, int dataType, bool continues = false);
	int readData(char *& output, fsize_t & payloadLength, int & opcode);
	WebsockClient(SOCKET sock, SOCKADDR_IN data);
	WebsockClient(SOCKET sock);
	WebsockClient() {};
	const std::string getProtocol() const { return protocol; }
};
class WebsockListener : public Listener {
private:
	std::string acceptedProtocols;
public:
	WebsockListener(unsigned short port) : Listener(port) {};
	WebsockClient accept(int & error);
};
#endif
