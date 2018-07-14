#ifndef SSL_H
#define SSL_H
#include "openssl//ssl.h"
#include "openssl//err.h"
#include "openssl//bio.h"

class openssl {
public:
	openssl() {
		SSL_library_init();
		SSL_load_error_strings();
	}
};
#endif
