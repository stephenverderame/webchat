#include "openssl.h"
#include <openssl/ssl.h>

void InitSSL()
{
	static bool init = false;
	if (!init) {
		init = true;
		SSL_library_init();
		SSL_load_error_strings();
	}
}
