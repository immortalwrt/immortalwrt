// SPDX-License-Identifier: GPL-2.0-or-later

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#define PX5G_VERSION "0.3"

static void openssl_error(const char *message)
{
	fprintf(stderr, "error: %s\n", message);
	ERR_print_errors_fp(stderr);
}

static int parse_uint(const char *value, unsigned int *result)
{
	char *end;
	unsigned long number;

	errno = 0;
	number = strtoul(value, &end, 10);
	if (errno || end == value || *end || *value == '-' || number > UINT_MAX)
		return -1;

	*result = number;
	return 0;
}

static BIO *open_output(const char *path, bool cert, int *fd)
{
	mode_t mode = S_IRUSR | S_IWUSR;

	if (cert)
		mode |= S_IRGRP | S_IROTH;

	*fd = path ? open(path, O_WRONLY | O_CREAT | O_TRUNC, mode) : STDERR_FILENO;
	if (*fd < 0) {
		fprintf(stderr, "error: %s: %s\n", path, strerror(errno));
		return NULL;
	}

	return BIO_new_fd(*fd, BIO_NOCLOSE);
}

static int close_output(BIO *bio, int fd, bool close_fd)
{
	int ret = 0;

	if (BIO_flush(bio) <= 0 || (close_fd && fsync(fd) < 0))
		ret = -1;
	BIO_free(bio);
	if (close_fd && close(fd) < 0)
		ret = -1;

	if (ret)
		fprintf(stderr, "error: I/O error: %s\n", strerror(errno));
	return ret;
}

static int write_key(EVP_PKEY *key, const char *path, bool pem)
{
	BIO *bio;
	int fd;
	int ret;

	bio = open_output(path, false, &fd);
	if (!bio) {
		if (path && fd >= 0)
			close(fd);
		return 1;
	}

	ret = pem ? PEM_write_bio_PrivateKey_traditional(bio, key, NULL, NULL, 0,
							 NULL, NULL) :
		i2d_PrivateKey_bio(bio, key);
	if (ret != 1) {
		openssl_error("failed to encode private key");
		BIO_free(bio);
		if (path)
			close(fd);
		return 1;
	}

	return close_output(bio, fd, path != NULL) != 0;
}

static int write_cert(X509 *cert, const char *path, bool pem)
{
	BIO *bio;
	int fd;
	int ret;

	bio = open_output(path, true, &fd);
	if (!bio) {
		if (path && fd >= 0)
			close(fd);
		return 1;
	}

	ret = pem ? PEM_write_bio_X509(bio, cert) : i2d_X509_bio(bio, cert);
	if (ret != 1) {
		openssl_error("failed to encode certificate");
		BIO_free(bio);
		if (path)
			close(fd);
		return 1;
	}

	return close_output(bio, fd, path != NULL) != 0;
}

static EVP_PKEY *gen_key(bool rsa, unsigned int bits, uint64_t exponent,
			 const char *curve)
{
	EVP_PKEY_CTX *ctx;
	EVP_PKEY *key = NULL;
	OSSL_PARAM params[3], *param = params;

	ctx = EVP_PKEY_CTX_new_from_name(NULL, rsa ? "RSA" : "EC", NULL);
	if (!ctx)
		goto error;

	if (rsa) {
		fprintf(stderr, "Generating RSA private key, %u bit long modulus\n", bits);
		*param++ = OSSL_PARAM_construct_uint(OSSL_PKEY_PARAM_BITS, &bits);
		*param++ = OSSL_PARAM_construct_uint64(OSSL_PKEY_PARAM_RSA_E, &exponent);
	} else {
		fprintf(stderr, "Generating EC private key\n");
		*param++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME,
							 (char *)curve, 0);
	}
	*param = OSSL_PARAM_construct_end();

	if (EVP_PKEY_keygen_init(ctx) <= 0 ||
	    EVP_PKEY_CTX_set_params(ctx, params) <= 0 ||
	    EVP_PKEY_generate(ctx, &key) <= 0)
		goto error;

	EVP_PKEY_CTX_free(ctx);
	return key;

error:
	openssl_error("key generation failed");
	EVP_PKEY_CTX_free(ctx);
	return NULL;
}

static int add_name(X509_NAME *name, const char *subject)
{
	char *copy, *next, *entry, *value;
	int ret = 1;

	if (!subject[0])
		return 1;
	if (subject[0] != '/' || strchr(subject, ';'))
		return 0;

	copy = strdup(subject + 1);
	if (!copy)
		return 0;

	next = copy;
	while ((entry = strsep(&next, "/"))) {
		value = strchr(entry, '=');
		if (!value) {
			ret = 0;
			break;
		}
		*value++ = '\0';
		if (!X509_NAME_add_entry_by_txt(name, entry, MBSTRING_UTF8,
						    (unsigned char *)value, -1, -1, 0)) {
			ret = 0;
			break;
		}
	}

	free(copy);
	return ret;
}

static int add_extension(X509 *cert, int nid, const char *value)
{
	X509V3_CTX ctx;
	X509_EXTENSION *extension;
	int ret;

	X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
	X509V3_set_ctx_nodb(&ctx);
	extension = X509V3_EXT_nconf_nid(NULL, &ctx, nid, value);
	if (!extension)
		return 0;

	ret = X509_add_ext(cert, extension, -1);
	X509_EXTENSION_free(extension);
	return ret;
}

static int make_certificate(X509 **result, EVP_PKEY *key, const char *subject,
			    unsigned int days, const char *san, const char *eku,
			    char *from_string, char *to_string)
{
	X509 *cert = NULL;
	X509_NAME *name;
	uint64_t serial;
	time_t from, to;
	struct tm tm;

	cert = X509_new();
	if (!cert || !X509_set_version(cert, 2))
		goto error;

	if (RAND_bytes((unsigned char *)&serial, sizeof(serial)) != 1 ||
	    !ASN1_INTEGER_set_uint64(X509_get_serialNumber(cert), serial))
		goto error;

	from = time(NULL);
	if (from < 1000000000)
		from = 1000000000;
	to = from + (time_t)days * 86400;
	if (to < from)
		to = INT_MAX;

	if (!gmtime_r(&from, &tm) ||
	    !strftime(from_string, 20, "%Y%m%d%H%M%S", &tm) ||
	    !gmtime_r(&to, &tm) ||
	    !strftime(to_string, 20, "%Y%m%d%H%M%S", &tm) ||
	    !ASN1_TIME_set(X509_getm_notBefore(cert), from) ||
	    !ASN1_TIME_set(X509_getm_notAfter(cert), to))
		goto error;

	name = X509_get_subject_name(cert);
	if (!add_name(name, subject)) {
		fprintf(stderr, "error: invalid subject\n");
		goto error;
	}
	if (!X509_set_issuer_name(cert, name) || !X509_set_pubkey(cert, key) ||
	    !add_extension(cert, NID_basic_constraints, "CA:FALSE") ||
	    /* AKID keyid: always reads the issuer SKID from this certificate. */
	    !add_extension(cert, NID_subject_key_identifier, "hash") ||
	    !add_extension(cert, NID_authority_key_identifier, "keyid:always") ||
	    (san && !add_extension(cert, NID_subject_alt_name, san)) ||
	    (eku && !add_extension(cert, NID_ext_key_usage, eku)) ||
	    !X509_sign(cert, key, EVP_sha256()))
		goto error;

	*result = cert;
	return 1;

error:
	openssl_error("failed to generate certificate");
	X509_free(cert);
	return 0;
}

static int append_san(char **san, const char *value)
{
	size_t old_len = *san ? strlen(*san) : 0;
	size_t value_len = strlen(value);
	char *new_san;

	new_san = realloc(*san, old_len + value_len + 2);
	if (!new_san)
		return 0;
	if (old_len)
		new_san[old_len++] = ',';
	memcpy(new_san + old_len, value, value_len + 1);
	/* OpenSSL's SAN parser uses lowercase "email". */
	if (!strncmp(value, "EMAIL:", 6))
		memcpy(new_san + old_len, "email:", 6);
	*san = new_san;
	return 1;
}

static int dokey(bool rsa, char **arg)
{
	EVP_PKEY *key;
	unsigned int bits = 512;
	uint64_t exponent = 65537;
	const char *curve = "P-256";
	const char *path = NULL;
	bool pem = true;
	int ret;

	while (*arg && **arg == '-') {
		if (!strcmp(*arg, "-out") && arg[1]) {
			path = *++arg;
		} else if (!strcmp(*arg, "-3")) {
			exponent = 3;
		} else if (!strcmp(*arg, "-der")) {
			pem = false;
		}
		arg++;
	}

	if (*arg && rsa) {
		if (parse_uint(*arg, &bits)) {
			fprintf(stderr, "error: invalid key size: %s\n", *arg);
			return 1;
		}
	} else if (*arg) {
		curve = *arg;
	}

	key = gen_key(rsa, bits, exponent, curve);
	if (!key)
		return 1;
	ret = write_key(key, path, pem);
	EVP_PKEY_free(key);
	return ret;
}

static int selfsigned(char **arg)
{
	EVP_PKEY *key = NULL;
	X509 *cert = NULL;
	const char *subject = "";
	const char *keypath = NULL, *certpath = NULL;
	const char *curve = "P-256";
	const char *eku = NULL;
	char *san = NULL;
	unsigned int bits = 512;
	unsigned int days = 30;
	uint64_t exponent = 65537;
	bool rsa = true;
	bool pem = true;
	char from_string[20], to_string[20];
	int ret = 1;

	while (*arg && **arg == '-') {
		if (!strcmp(*arg, "-der")) {
			pem = false;
		} else if (!strcmp(*arg, "-newkey") && arg[1]) {
			arg++;
			if (!strncmp(*arg, "rsa:", 4)) {
				rsa = true;
				if (parse_uint(*arg + 4, &bits)) {
					fprintf(stderr, "error: invalid key size: %s\n", *arg + 4);
					goto out;
				}
			} else if (!strcmp(*arg, "ec")) {
				rsa = false;
			} else {
				fprintf(stderr, "error: invalid algorithm\n");
				goto out;
			}
		} else if (!strcmp(*arg, "-days") && arg[1]) {
			if (parse_uint(*++arg, &days)) {
				fprintf(stderr, "error: invalid validity: %s\n", *arg);
				goto out;
			}
		} else if (!strcmp(*arg, "-pkeyopt") && arg[1]) {
			arg++;
			if (strncmp(*arg, "ec_paramgen_curve:", 18)) {
				fprintf(stderr, "error: invalid pkey option: %s\n", *arg);
				goto out;
			}
			curve = *arg + 18;
		} else if (!strcmp(*arg, "-keyout") && arg[1]) {
			keypath = *++arg;
		} else if (!strcmp(*arg, "-out") && arg[1]) {
			certpath = *++arg;
		} else if (!strcmp(*arg, "-subj") && arg[1]) {
			subject = *++arg;
		} else if (!strcmp(*arg, "-addext") && arg[1]) {
			arg++;
			if (!strncmp(*arg, "extendedKeyUsage=", 17)) {
				const char *value = *arg + 17;

				if (!strcmp(value, "serverAuth"))
					eku = "serverAuth";
				else if (!strcmp(value, "any"))
					eku = "2.5.29.37.0";
				else {
					fprintf(stderr, "error: invalid extended key usage: %s\n", value);
					goto out;
				}
			} else if (!strncmp(*arg, "subjectAltName=", 15)) {
				if (!append_san(&san, *arg + 15)) {
					fprintf(stderr, "error: invalid subject alternative name\n");
					goto out;
				}
			}
		}
		arg++;
	}

	key = gen_key(rsa, bits, exponent, curve);
	if (!key)
		goto out;
	if (!make_certificate(&cert, key, subject, days, san, eku,
			      from_string, to_string))
		goto out;
	if (keypath && write_key(key, keypath, pem))
		goto out;

	fprintf(stderr, "Generating selfsigned certificate with subject '%s'"
		" and validity %s-%s\n", subject, from_string, to_string);
	ret = write_cert(cert, certpath, pem);

out:
	free(san);
	X509_free(cert);
	EVP_PKEY_free(key);
	return ret;
}

int main(int argc, char **argv)
{
	(void)argc;

	if (argv[1]) {
		if (!strcmp(argv[1], "eckey"))
			return dokey(false, argv + 2);
		if (!strcmp(argv[1], "rsakey"))
			return dokey(true, argv + 2);
		if (!strcmp(argv[1], "selfsigned"))
			return selfsigned(argv + 2);
	}

	fprintf(stderr, "PX5G X.509 Certificate Generator Utility v" PX5G_VERSION
		" (OpenSSL)\n\nUsage: %s [eckey|rsakey|selfsigned]\n", argv[0]);
	return 1;
}
