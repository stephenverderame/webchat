#ifndef SSL_H
#define SSL_H
#ifndef _WIN32
#include "openssl//ssl.h"
#include "openssl//err.h"
#include "openssl//bio.h"
#else
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#endif
class openssl {
public:
	openssl() {
		SSL_library_init();
		SSL_load_error_strings();
	}
};
#endif
