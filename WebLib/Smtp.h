#ifndef SMTP_H
#define SMTP_H
#include "Base.h"
#include "Base64.h"
#include "SSL.h"
#define COM_SMTP_TLS_PORT 587
#define COM_SMTP_SSL_PORT 465
#define GMAIL_HOST "smtp.gmail.com"
#define GMAIL_PORT 587
#define YAHOO_HOST "smtp.mail.yahoo.com"
#define YAHOO_PORT 587
#define OPTONLINE_HOST "mail.optimum.net"
#define OPTONLINE_PORT 465
#define AOL_HOST "smtp.aol.com"
#define AOL_PORT 465
#define HOTMAIL_HOST "smtp.live.com"
#define HOTMAIL_PORT 587
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
