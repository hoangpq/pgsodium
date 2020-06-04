#include "pgsodium.h"
PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pgsodium_randombytes_random);
Datum
pgsodium_randombytes_random(PG_FUNCTION_ARGS)
{
	PG_RETURN_UINT32(randombytes_random());
}

PG_FUNCTION_INFO_V1(pgsodium_randombytes_uniform);
Datum
pgsodium_randombytes_uniform(PG_FUNCTION_ARGS)
{
	uint32_t upper_bound = PG_GETARG_UINT32(0);
	PG_RETURN_UINT32(randombytes_uniform(upper_bound));
}

PG_FUNCTION_INFO_V1(pgsodium_randombytes_buf);
Datum
pgsodium_randombytes_buf(PG_FUNCTION_ARGS)
{
	size_t size = PG_GETARG_UINT32(0);
	unsigned long long result_size = VARHDRSZ + size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	randombytes_buf(VARDATA(result), size);
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_secretbox_keygen);
Datum
pgsodium_crypto_secretbox_keygen(PG_FUNCTION_ARGS)
{
	unsigned long long result_size = VARHDRSZ + crypto_secretbox_KEYBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	crypto_secretbox_keygen(CHARDATA(result));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_secretbox_noncegen);
Datum
pgsodium_crypto_secretbox_noncegen(PG_FUNCTION_ARGS)
{
	int result_size = VARHDRSZ + crypto_secretbox_NONCEBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	randombytes_buf(VARDATA(result), crypto_secretbox_NONCEBYTES);
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_secretbox);
Datum
pgsodium_crypto_secretbox(PG_FUNCTION_ARGS)
{
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *nonce = PG_GETARG_BYTEA_P(1);
	bytea *key = PG_GETARG_BYTEA_P(2);
	size_t message_size = crypto_secretbox_MACBYTES + VARSIZE_ANY_EXHDR(message);
	unsigned long long result_size = VARHDRSZ + message_size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	crypto_secretbox_easy(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(nonce),
		CHARDATA(key));

	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_secretbox_open);
Datum
pgsodium_crypto_secretbox_open(PG_FUNCTION_ARGS)
{
	int success;
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *nonce = PG_GETARG_BYTEA_P(1);
	bytea *key = PG_GETARG_BYTEA_P(2);
	size_t message_size = VARSIZE_ANY_EXHDR(message) - crypto_secretbox_MACBYTES;
	unsigned long long result_size = VARHDRSZ + message_size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	success = crypto_secretbox_open_easy(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(nonce),
		CHARDATA(key));

	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));

	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_auth);
Datum
pgsodium_crypto_auth(PG_FUNCTION_ARGS)
{
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *key = PG_GETARG_BYTEA_P(1);
	int result_size = VARHDRSZ + crypto_auth_BYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	crypto_auth(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(key));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_auth_verify);
Datum
pgsodium_crypto_auth_verify(PG_FUNCTION_ARGS)
{
	int success;
	bytea *mac = PG_GETARG_BYTEA_P(0);
	bytea *message = PG_GETARG_BYTEA_P(1);
	bytea *key = PG_GETARG_BYTEA_P(2);

	success = crypto_auth_verify(
		CHARDATA(mac),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(key));
	PG_RETURN_BOOL(success == 0);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_auth_keygen);
Datum
pgsodium_crypto_auth_keygen(PG_FUNCTION_ARGS)
{
	unsigned long long result_size = VARHDRSZ + crypto_auth_KEYBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	crypto_secretbox_keygen(CHARDATA(result));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_generichash);
Datum
pgsodium_crypto_generichash(PG_FUNCTION_ARGS)
{
	bytea *data;
	bytea *result;
	bytea *keyarg;
	unsigned char *key = NULL;
	size_t keylen = 0;
	unsigned long long result_size;

	data = PG_GETARG_BYTEA_P(0);
	if (!PG_ARGISNULL(1))
	{
		keyarg = PG_GETARG_BYTEA_P(1);
		key = CHARDATA(keyarg);
		keylen = VARSIZE_ANY_EXHDR(keyarg);
	}

	result_size = VARHDRSZ + crypto_generichash_BYTES;
	result = _pgsodium_zalloc_bytea(result_size);

	crypto_generichash(
		CHARDATA(result),
		crypto_generichash_BYTES,
		CHARDATA(data),
		VARSIZE_ANY_EXHDR(data),
		key,
		keylen);
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_shorthash);
Datum
pgsodium_crypto_shorthash(PG_FUNCTION_ARGS)
{
	bytea *data;
	bytea *result;
	bytea *key;
	int result_size = VARHDRSZ + crypto_shorthash_BYTES;

	data = PG_GETARG_BYTEA_P(0);
	key = PG_GETARG_BYTEA_P(1);
	if (VARSIZE_ANY_EXHDR(key) != crypto_shorthash_KEYBYTES)
		PG_RETURN_NULL();

	result = _pgsodium_zalloc_bytea(result_size);

	crypto_shorthash(
		CHARDATA(result),
		CHARDATA(data),
		VARSIZE_ANY_EXHDR(data),
		CHARDATA(key));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_box_keypair);
Datum
pgsodium_crypto_box_keypair(PG_FUNCTION_ARGS)
{
    TupleDesc tupdesc;
	Datum values[2];
	bool nulls[2] = {false, false};
	HeapTuple tuple;
	Datum result;
	bytea *publickey;
	bytea *secretkey;
	size_t public_size = crypto_box_PUBLICKEYBYTES + VARHDRSZ;
	size_t secret_size = crypto_box_SECRETKEYBYTES + VARHDRSZ;

	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("function returning record called in context "
						"that cannot accept type record")));

	publickey = _pgsodium_zalloc_bytea(public_size);
	secretkey = _pgsodium_zalloc_bytea(secret_size);

	crypto_box_keypair(
		CHARDATA(publickey),
		CHARDATA(secretkey)
		);

	values[0] = PointerGetDatum(publickey);
	values[1] = PointerGetDatum(secretkey);
	tuple = heap_form_tuple(tupdesc, values, nulls);
	result = HeapTupleGetDatum(tuple);
	return result;
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_box_noncegen);
Datum
pgsodium_crypto_box_noncegen(PG_FUNCTION_ARGS)
{
	unsigned long long result_size = VARHDRSZ + crypto_box_NONCEBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	randombytes_buf(VARDATA(result), crypto_box_NONCEBYTES);
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_box);
Datum
pgsodium_crypto_box(PG_FUNCTION_ARGS)
{
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *nonce = PG_GETARG_BYTEA_P(1);
	bytea *publickey = PG_GETARG_BYTEA_P(2);
	bytea *secretkey = PG_GETARG_BYTEA_P(3);
	int success;

	size_t message_size = crypto_box_MACBYTES + VARSIZE_ANY_EXHDR(message);
	bytea *result = _pgsodium_zalloc_bytea(VARHDRSZ + message_size);
	success = crypto_box_easy(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(nonce),
		CHARDATA(publickey),
		CHARDATA(secretkey)
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));

	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_box_open);
Datum
pgsodium_crypto_box_open(PG_FUNCTION_ARGS)
{
	int success;
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *nonce = PG_GETARG_BYTEA_P(1);
	bytea *publickey = PG_GETARG_BYTEA_P(2);
	bytea *secretkey = PG_GETARG_BYTEA_P(3);

	size_t message_size = VARSIZE_ANY_EXHDR(message) - crypto_box_MACBYTES;
	bytea *result = _pgsodium_zalloc_bytea(VARHDRSZ + message_size);
	success = crypto_box_open_easy(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(nonce),
		CHARDATA(publickey),
		CHARDATA(secretkey));
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_sign_keypair);
Datum
pgsodium_crypto_sign_keypair(PG_FUNCTION_ARGS)
{
    TupleDesc tupdesc;
	Datum values[2];
	bool nulls[2] = {false, false};
	HeapTuple tuple;
	Datum result;
	bytea *publickey;
	bytea *secretkey;
	size_t public_size = crypto_sign_PUBLICKEYBYTES + VARHDRSZ;
	size_t secret_size = crypto_sign_SECRETKEYBYTES + VARHDRSZ;

	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("function returning record called in context "
						"that cannot accept type record")));

	publickey = _pgsodium_zalloc_bytea(public_size);
	secretkey = _pgsodium_zalloc_bytea(secret_size);
	crypto_sign_keypair(
		CHARDATA(publickey),
		CHARDATA(secretkey)
		);

	values[0] = PointerGetDatum(publickey);
	values[1] = PointerGetDatum(secretkey);
	tuple = heap_form_tuple(tupdesc, values, nulls);
	result = HeapTupleGetDatum(tuple);
	return result;
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_sign);
Datum
pgsodium_crypto_sign(PG_FUNCTION_ARGS)
{
	int success;
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *secretkey = PG_GETARG_BYTEA_P(1);
	unsigned long long signed_message_len;
	size_t message_size = crypto_sign_BYTES + VARSIZE_ANY_EXHDR(message);
	unsigned long long result_size = VARHDRSZ + message_size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	success = crypto_sign(
		CHARDATA(result),
		&signed_message_len,
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(secretkey)
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_sign_open);
Datum
pgsodium_crypto_sign_open(PG_FUNCTION_ARGS)
{
	int success;
	unsigned long long unsigned_message_len;
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *publickey = PG_GETARG_BYTEA_P(1);
	size_t message_size = VARSIZE_ANY_EXHDR(message) - crypto_sign_BYTES;
	unsigned long long result_size = VARHDRSZ + message_size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	SET_VARSIZE(result, result_size);
	success = crypto_sign_open(
		CHARDATA(result),
		&unsigned_message_len,
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(publickey)
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_sign_detached);
Datum
pgsodium_crypto_sign_detached(PG_FUNCTION_ARGS)
{
	int success;
	bytea *message = PG_GETARG_BYTEA_P(0);
	bytea *secretkey = PG_GETARG_BYTEA_P(1);
	size_t sig_size = crypto_sign_BYTES;
	unsigned long long result_size = VARHDRSZ + sig_size;
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	success = crypto_sign_detached(
		CHARDATA(result),
		NULL,
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(secretkey)
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_sign_verify_detached);
Datum
pgsodium_crypto_sign_verify_detached(PG_FUNCTION_ARGS)
{
	int success;
	bytea *sig = PG_GETARG_BYTEA_P(0);
	bytea *message = PG_GETARG_BYTEA_P(1);
	bytea *publickey = PG_GETARG_BYTEA_P(2);

	success = crypto_sign_verify_detached(
	        CHARDATA(sig),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(publickey)
		);
	if (success == 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}


PG_FUNCTION_INFO_V1(pgsodium_crypto_pwhash_saltgen);
Datum
pgsodium_crypto_pwhash_saltgen(PG_FUNCTION_ARGS)
{
	unsigned long long result_size = VARHDRSZ + crypto_pwhash_SALTBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	randombytes_buf(VARDATA(result), crypto_pwhash_SALTBYTES);
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_pwhash);
Datum
pgsodium_crypto_pwhash(PG_FUNCTION_ARGS)
{
	bytea *data;
	bytea *result;
	bytea *salt;
	int result_size = VARHDRSZ + crypto_box_SEEDBYTES;
	int success;

	data = PG_GETARG_BYTEA_P(0);
	salt = PG_GETARG_BYTEA_P(1);
	if (VARSIZE_ANY_EXHDR(salt) != crypto_pwhash_SALTBYTES)
		PG_RETURN_NULL();

	result = _pgsodium_zalloc_bytea(result_size);

	success = crypto_pwhash(
		CHARDATA(result),
		crypto_box_SEEDBYTES,
		VARDATA(data),
		VARSIZE_ANY_EXHDR(data),
		CHARDATA(salt),
		crypto_pwhash_OPSLIMIT_MODERATE,
		crypto_pwhash_MEMLIMIT_MODERATE,
		crypto_pwhash_ALG_DEFAULT
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_pwhash_str);
Datum
pgsodium_crypto_pwhash_str(PG_FUNCTION_ARGS)
{
	int success;
	bytea *password = PG_GETARG_BYTEA_P(0);
	bytea *result = _pgsodium_zalloc_bytea(crypto_pwhash_STRBYTES);

	success = crypto_pwhash_str(
		VARDATA(result),
		VARDATA(password),
		VARSIZE_ANY_EXHDR(password),
		crypto_pwhash_OPSLIMIT_MODERATE,
		crypto_pwhash_MEMLIMIT_MODERATE);

	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("out of memory in pwhash_str")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_pwhash_str_verify);
Datum
pgsodium_crypto_pwhash_str_verify(PG_FUNCTION_ARGS)
{
	int success;
	bytea *hashed_password = PG_GETARG_BYTEA_P(0);
	bytea *password = PG_GETARG_BYTEA_P(1);
	success = crypto_pwhash_str_verify(
		VARDATA(hashed_password),
		VARDATA(password),
		VARSIZE_ANY_EXHDR(password)
		);
	PG_RETURN_BOOL(success == 0);
}


PG_FUNCTION_INFO_V1(pgsodium_crypto_box_seal);
Datum
pgsodium_crypto_box_seal(PG_FUNCTION_ARGS)
{
	bytea *message = PG_GETARG_BYTEA_P(0);
    bytea *public_key = PG_GETARG_BYTEA_P(1);
    unsigned long long result_size = crypto_box_SEALBYTES + VARSIZE(message);

	bytea *result = _pgsodium_zalloc_bytea(result_size);

    crypto_box_seal(
		CHARDATA(result),
		CHARDATA(message),
		VARSIZE_ANY_EXHDR(message),
		CHARDATA(public_key));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_box_seal_open);
Datum
pgsodium_crypto_box_seal_open(PG_FUNCTION_ARGS)
{
	int success;
	bytea *ciphertext = PG_GETARG_BYTEA_P(0);
	bytea *public_key = PG_GETARG_BYTEA_P(1);
	bytea *secret_key = PG_GETARG_BYTEA_P(2);

	unsigned long long result_size = VARSIZE(ciphertext) - crypto_box_SEALBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	success = crypto_box_seal_open(
		CHARDATA(result),
		CHARDATA(ciphertext),
		VARSIZE_ANY_EXHDR(ciphertext),
		CHARDATA(public_key),
		CHARDATA(secret_key)
		);
	if (success != 0)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("crypto_box_seal_open: invalid message")));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_kdf_keygen);
Datum
pgsodium_crypto_kdf_keygen(PG_FUNCTION_ARGS)
{
	unsigned long long result_size = VARHDRSZ + crypto_kdf_KEYBYTES;
	bytea *result = _pgsodium_zalloc_bytea(result_size);
	crypto_kdf_keygen(CHARDATA(result));
	PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(pgsodium_crypto_kdf_derive_from_key);
Datum
pgsodium_crypto_kdf_derive_from_key(PG_FUNCTION_ARGS)
{
	size_t subkey_size = PG_GETARG_UINT32(0);
    unsigned long long result_size = VARHDRSZ + subkey_size;
    unsigned long long subkey_id = PG_GETARG_INT64(1);
    bytea *context = PG_GETARG_BYTEA_P(2);
    bytea *master_key = PG_GETARG_BYTEA_P(3);
	bytea *result = _pgsodium_zalloc_bytea(result_size);

	if (subkey_size < crypto_kdf_BYTES_MIN || subkey_size > crypto_kdf_BYTES_MAX)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("crypto_kdf_derive_from_key: invalid key size requested")));

	if (VARSIZE_ANY_EXHDR(context) != 8)
		ereport(
			ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("crypto_kdf_derive_from_key: context must be 8 bytes")));

    crypto_kdf_derive_from_key(
		CHARDATA(result),
		subkey_size,
		subkey_id,
		(const char*)VARDATA(context),
		(const unsigned char*)VARDATA(master_key));
	PG_RETURN_BYTEA_P(result);
}

void _PG_init(void)
{
	if (sodium_init() == -1)
	{
		elog(ERROR, "_PG_init: sodium_init() failed cannot initialize pgsodium");
		return;
	}
}
