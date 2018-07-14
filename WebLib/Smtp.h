#ifndef SMTP_H
#define SMTP_H
#include "Base.h"
#include "Base64.h"
#include "SSL.h"
#define GMAIL_HOST "smtp.gmail.com"
#define GMAIL_PORT 587
class SmtpClient : public Client {
private:
	std::string user;
	std::string password;
	std::string rcpt;
	std::string subject;
	std::string message;
	std::string host;
public:
	SmtpClient(char * hostName, unsigned short port = STD_SMTP_PORT);
	void login(char * user, char * password);
	int sendMessage(char * recp, char * subject, char * message);
};
#endif
