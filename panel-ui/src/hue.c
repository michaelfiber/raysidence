#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include "hue.h"
#include "stdbool.h"

Hue HUE = {0};

// get list of servers.
// load server info.

static Hue servers[16] = {0};

#define HOST "discovery.meethue.com:443"

bool sslInitialized = false;

BIO *bio;
SSL *ssl;
SSL_CTX *ctx;

HueError *newError(int code, const char *msg)
{
	HueError *err = malloc(sizeof(HueError));
	err->code = code;
	err->msg = msg;
	return err;
}

HueError *HueLoadServers()
{
	if (!sslInitialized)
	{
		SSL_library_init();
		ctx = SSL_CTX_new(SSLv23_client_method());
		if (ctx == NULL)
		{
			return newError(1, "ctx is null");
		}

		bio = BIO_new_ssl_connect(ctx);
		BIO_set_conn_hostname(bio, HOST);

		if (BIO_do_connect(bio) <= 0)
		{
			return newError(2, "failed to connect");
		}

		printf("Connected!\n");
	}

	return NULL;
}
