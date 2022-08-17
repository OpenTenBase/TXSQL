#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/dh.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <algorithm>
#include <string>
#include <vector>

#include "cdb_enterprise_encryption.h"

ulong rsa_bits_threshold;
ulong dh_bits_threshold;
ulong dsa_bits_threshold;

mysql_mutex_t dh_params_lock, dsa_params_lock, rsa_key_lock, dh_key_lock,
    dsa_key_lock;

static const char *KEY_FILE_PREFIX = "backup_";

static void handle_no_start_line_error_if_needed() {
  unsigned long err;
  err = ERR_peek_last_error();
  if (ERR_GET_LIB(err) == ERR_LIB_PEM &&
      ERR_GET_REASON(err) == PEM_R_NO_START_LINE) {
    ERR_clear_error();
  }
}

/* tranfer the cipher from uchar to hexadecimal. */
static void transfer(const uchar *from, char *to,
                     size_t len) /* the length of uchar */
{
  char tmp[3] = "";

  for (size_t i = 0; i < len; i++) {
    sprintf(tmp, "%02x", from[i]);
    to[i * 2] = tmp[0];
    to[i * 2 + 1] = tmp[1];
  }
}

/* tranfer the cipher from hexadecimal back to uchar. */
static void transfer_back(uchar *to, const char *from,
                          size_t len) /* the length of hexadecimal */
{
  size_t i = 0;
  int tmp = 0;

  for (; i < len / 2; i++) {
    sscanf(from + 2 * i, "%02x", &tmp);
    to[i] = (uchar)tmp;
  }
}

/* tranfer the string to lower case. */
static void str_tolower(const char *str, char *lower_str, size_t buf_len) {
  size_t str_len = strlen(str);
  size_t len = str_len > buf_len - 1 ? buf_len - 1 : str_len;
  size_t i = 0;

  for (; i < len; i++) {
    lower_str[i] = tolower(str[i]);
  }
  lower_str[len] = '\0';
}

/* calculate md5 digest. */
static void calc_md5_digest(char *source, uint size, uchar *result) {
  MD5_CTX c_md5;
  MD5_Init(&c_md5);
  MD5_Update(&c_md5, source, size);
  MD5_Final(result, &c_md5);
  OPENSSL_cleanse(&c_md5, sizeof(c_md5));
}

/* calculate sha1 digest. */
static void calc_sha1_digest(char *source, uint size, uchar *result) {
  SHA_CTX c_sha1;
  SHA1_Init(&c_sha1);
  SHA1_Update(&c_sha1, source, size);
  SHA1_Final(result, &c_sha1);
  OPENSSL_cleanse(&c_sha1, sizeof(c_sha1));
}

/* calculate sha224 digest. */
static void calc_sha224_digest(char *source, uint size, uchar *result) {
  SHA256_CTX c_sha224;
  SHA224_Init(&c_sha224);
  SHA224_Update(&c_sha224, source, size);
  SHA224_Final(result, &c_sha224);
  OPENSSL_cleanse(&c_sha224, sizeof(c_sha224));
}

/* calculate sha256 digest. */
static void calc_sha256_digest(char *source, uint size, uchar *result) {
  SHA256_CTX c_sha256;
  SHA256_Init(&c_sha256);
  SHA256_Update(&c_sha256, source, size);
  SHA256_Final(result, &c_sha256);
  OPENSSL_cleanse(&c_sha256, sizeof(c_sha256));
}

/* calculate sha384 digest. */
static void calc_sha384_digest(char *source, uint size, uchar *result) {
  SHA512_CTX c_sha384;
  SHA384_Init(&c_sha384);
  SHA384_Update(&c_sha384, source, size);
  SHA384_Final(result, &c_sha384);
  OPENSSL_cleanse(&c_sha384, sizeof(c_sha384));
}

/* calculate sha512 digest. */
static void calc_sha512_digest(char *source, uint size, uchar *result) {
  SHA512_CTX c_sha512;
  SHA512_Init(&c_sha512);
  SHA512_Update(&c_sha512, source, size);
  SHA512_Final(result, &c_sha512);
  OPENSSL_cleanse(&c_sha512, sizeof(c_sha512));
}

/* calculate the md5 digest of char-format priv_key. */
static void calc_privkey_digest(char *c_key, char *str) {
  uchar md5[MD5_DIGEST_LENGTH] = "";

  /* calculate the md5 digest of c_key. */
  calc_md5_digest(c_key, strlen(c_key), md5);

  transfer(md5, str, MD5_DIGEST_LENGTH);
  str[MD5_DIGEST_LENGTH * 2] = '\n';
}

/* calculate the md5 digest of evp_key-format priv_key. */
static void calc_privkey_digest(EVP_PKEY *evp_key, char *str) {
  char *c_key = nullptr;
  BIO *bio_mem = nullptr;
  uchar md5[MD5_DIGEST_LENGTH] = "";

  /* read evp_key into c_key. */
  bio_mem = BIO_new(BIO_s_mem());
  PEM_write_bio_PrivateKey(bio_mem, evp_key, nullptr, nullptr, 0, nullptr,
                           nullptr);

  size_t priv_len = BIO_pending(bio_mem);
  c_key = (char *)my_malloc(PSI_NOT_INSTRUMENTED, priv_len + 1, MYF(MY_WME));
  memset(c_key, 0, priv_len + 1);
  BIO_read(bio_mem, c_key, priv_len);

  /* calculate the md5 digest of c_key. */
  calc_md5_digest(c_key, strlen(c_key), md5);

  transfer(md5, str, MD5_DIGEST_LENGTH);
  str[MD5_DIGEST_LENGTH * 2] = '\n';

  my_free(c_key);
  BIO_free(bio_mem);
}

/* check the key format of EVP_PKEY. */
static bool check_key_format(int type, char *key) {
  char head[KEY_HEADTAIL_LEN] = "", tail[KEY_HEADTAIL_LEN] = "";

  /* check input value. */
  if (PRIVATE_KEY_TYPE == type) {
    strcpy(head, "-----BEGIN PRIVATE KEY-----");
    strcpy(tail, "-----END PRIVATE KEY-----\n");
  } else if (PUBLIC_KEY_TYPE == type) {
    strcpy(head, "-----BEGIN PUBLIC KEY-----");
    strcpy(tail, "-----END PUBLIC KEY-----\n");
  } else {
    return false;
  }

  if (key == nullptr) return false;

  size_t key_len = strlen(key);
  size_t head_len = strlen(head);
  size_t tail_len = strlen(tail);

  if (key_len <= head_len + tail_len) {
    return false;
  } else if (strncmp(key, head, head_len)) {
    return false;
  } else if (strncmp(key + key_len - tail_len, tail, tail_len)) {
    return false;
  }

  return true;
}

/* check the if the system variables are available. */
static bool check_system_variables() {
  /* if this plugin is uninstalled, the sys variables
     inside would be set to 0 by its deinit function. */
  if (!rsa_bits_threshold || !dh_bits_threshold || !dsa_bits_threshold) {
    return false;
  }

  return true;
}

/* check rsa/dh/dsa key bits. */
static bool check_key_bits(
    std::string algo, /*algo has been transformed to lowercase */
    ulong bits, char *message) {
  if (bits < OPENSSL_SAFE_MINIMUM) {
    strcpy(message, "The key bits are too small.\n");
    return false;
  } else if (algo == "rsa" && bits > rsa_bits_threshold) {
    strcpy(message, "The key bits are too big for rsa.\n");
    return false;
  } else if (algo == "dh" && bits > dh_bits_threshold) {
    strcpy(message, "The key bits are too big for dh.\n");
    return false;
  } else if (algo == "dsa" && bits > dsa_bits_threshold) {
    strcpy(message, "The key bits are too big for dsa.\n");
    return false;
  }

  return true;
}

/* check algo bits for rsa/dh/dsa. */
static bool check_algo_bits(std::vector<std::string> algo_allow,
                            std::string algo, ulong bits, char *message) {
  transform(algo.begin(), algo.end(), algo.begin(), ::tolower);
  if (find(algo_allow.begin(), algo_allow.end(), algo) == algo_allow.end()) {
    strcpy(message, "Error algo input.\n");
    return false;
  } else if (!check_key_bits(algo, bits, message)) {
    return false;
  }

  return true;
}

/* record keys in key-file. */
static bool record_keys_to_file(EVP_PKEY *evp_key, char *file_name,
                                mysql_mutex_t &lock) {
  BIO *bio_file = nullptr;
  char c_digest[MD5_DIGEST_LENGTH * 2 + 1] = "";

  calc_privkey_digest(evp_key, c_digest);

  if (!(bio_file = BIO_new_file(file_name, "a"))) return true;

  mysql_mutex_lock(&lock);
  BIO_write(bio_file, c_digest, MD5_DIGEST_LENGTH * 2 + 1);
  PEM_write_bio_PUBKEY(bio_file, evp_key);

  mysql_mutex_unlock(&lock);
  BIO_free(bio_file);
  return false;
}

/*****************************************************************************
Check input parameters for modify_public_params.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool modify_public_params_init(
    UDF_INIT *initid, /*!< in: Used to convey
                        infos between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the algo and"
           " the key length in bits.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != INT_RESULT) {
    strcpy(message, "Wrong parameter types.\n");
    return true;
  }

  if (args->args[0] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 1 cannot be null\n");
    return true;
  }
  if (args->args[1] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 2 cannot be null\n");
    return true;
  }

  ulong bits = *((ulong *)args->args[1]);
  std::vector<std::string> algo_allow;

  algo_allow.push_back("dh");
  algo_allow.push_back("dsa");
  if (!check_algo_bits(algo_allow, args->args[0], bits, message)) {
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void modify_public_params_deinit(UDF_INIT *) {}

/*****************************************************************************
Create/modify the public parameters required for DSA/DH.
@retval     Check input parameters!     Success.
@retval     nullptr                     Failure.
*****************************************************************************/
PLUGIN_EXPORT char *modify_public_params(
    UDF_INIT *,     /*!< in: Used to convey infos
                      between functions */
    UDF_ARGS *args, /*!< in: Array of arguments */
    char *result,   /*!< out: size:256, to hold short
                      result */
    ulong *length,  /*!< out: length of result */
    char *is_null,  /*!< out: true if result is nullptr */
    char *error)    /*!< out: true if error occurs,
                      is_null=1 will be set */
{
  *is_null = 0;
  *error = 0;

  char param_file_name[KEY_FILENAME_LEN] = "";
  char lower_algo[ALGORITHM_LEN] = "";
  ulong bits = *((ulong *)args->args[1]);

  str_tolower(args->args[0], lower_algo, ALGORITHM_LEN);
  sprintf(param_file_name, "%s%d.pem", lower_algo, (int)bits);

  if (strcasecmp(args->args[0], "dh") == 0) {
    DH *dh = DH_new();
    if (dh == nullptr) {
      *error = 1;
      return nullptr;
    }

    if (DH_generate_parameters_ex(dh, (int)bits, DH_GENERATOR_2, nullptr) !=
        1) {
      DH_free(dh);
      *error = 1;
      return nullptr;
    }

    /* record the public params. */
    BIO *bio_write = BIO_new_file(param_file_name, "w");
    if (!bio_write) {
      DH_free(dh);
      *error = 1;
      return nullptr;
    }

    mysql_mutex_lock(&dh_params_lock);
    PEM_write_bio_DHparams(bio_write, dh);
    mysql_mutex_unlock(&dh_params_lock);

    BIO_free(bio_write);
    DH_free(dh);
  } else {
    DSA *dsa = DSA_new();
    if (dsa == nullptr) {
      *error = 1;
      return nullptr;
    }

    if (DSA_generate_parameters_ex(dsa, (int)bits, nullptr, 0, nullptr, nullptr,
                                   nullptr) != 1) {
      DSA_free(dsa);
      *error = 1;
      return nullptr;
    }

    /* record the public params. */
    BIO *bio_write = BIO_new_file(param_file_name, "w");
    if (!bio_write) {
      DSA_free(dsa);
      *error = 1;
      return nullptr;
    }

    mysql_mutex_lock(&dsa_params_lock);
    PEM_write_bio_DSAparams(bio_write, dsa);
    mysql_mutex_unlock(&dsa_params_lock);

    BIO_free(bio_write);
    DSA_free(dsa);
  }

  strcpy(result, "Successfully modified!");
  *length = strlen(result);

  return result;
}

/*****************************************************************************
Check input parameters for create_asymmetric_priv_key.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool create_asymmetric_priv_key_init(
    UDF_INIT *initid, /*!< in: Used to convey
                        infos between functions */
    UDF_ARGS *args,   /*!< in: Array of
                        arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the algo and"
           " the key length in bits.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != INT_RESULT) {
    strcpy(message, "Wrong parameter types.\n");
    return true;
  }

  if (args->args[0] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 1 cannot be null\n");
    return true;
  }
  if (args->args[1] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 2 cannot be null\n");
    return true;
  }

  ulong bits = *((ulong *)args->args[1]);
  std::vector<std::string> algo_allow;

  algo_allow.push_back("rsa");
  algo_allow.push_back("dh");
  algo_allow.push_back("dsa");

  if (!check_algo_bits(algo_allow, args->args[0], bits, message)) {
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void create_asymmetric_priv_key_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
Create an RSA/DSA/DH private key.
If a public params hasn't been created, it'll automatically create one.
priv_key & pub_key will be created together, and then pub_key and the
digest of priv_key will be recorded in key file.
@retval     priv_key     Success.
@retval     nullptr      Failure.
*****************************************************************************/
PLUGIN_EXPORT char *create_asymmetric_priv_key(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold
                        short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result
                        is nullptr */
    char *error)      /*!< out: true if error occurs,
                        is_null=1 will be set */
{
  *is_null = 0;
  *error = 0;

  char param_file_name[KEY_FILENAME_LEN] = "",
       key_file_name[KEY_FILENAME_LEN] = "", lower_algo[ALGORITHM_LEN] = "";

  ulong bits = *((int64 *)args->args[1]);
  EVP_PKEY *evp_key = EVP_PKEY_new();

  str_tolower(args->args[0], lower_algo, ALGORITHM_LEN);
  sprintf(key_file_name, "%s%s.pem", KEY_FILE_PREFIX, lower_algo);

  if (strcasecmp(args->args[0], "rsa") == 0) {
    /* generate keys. */
    RSA *key = RSA_new();
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);

    if (RSA_generate_key_ex(key, (int)bits, bne, nullptr) != 1) {
      EVP_PKEY_free(evp_key);
      RSA_free(key);
      BN_free(bne);

      *error = 1;
      return nullptr;
    }
    BN_free(bne);

    /* assign key to evp_key, only need to free evp_key,
    free both evp_key and key will cause crash. */
    EVP_PKEY_assign_RSA(evp_key, key);
    if (record_keys_to_file(evp_key, key_file_name, rsa_key_lock)) {
      /* failed to write keys to key file. */
      EVP_PKEY_free(evp_key);
      RSA_free(key);

      *error = 1;
      return nullptr;
    }
  } else if (strcasecmp(args->args[0], "dh") == 0) {
    /* get public parameters. */
    DH *key = nullptr;
    sprintf(param_file_name, "dh%d.pem", (int)bits);

    BIO *bio_read = BIO_new_file(param_file_name, "r");
    if (!bio_read) {
      key = DH_new();
      DH_generate_parameters_ex(key, (int)bits, DH_GENERATOR_2, nullptr);

      BIO *bio_write = BIO_new_file(param_file_name, "w");
      if (!bio_write) {
        EVP_PKEY_free(evp_key);
        DH_free(key);

        *error = 1;
        return nullptr;
      }

      mysql_mutex_lock(&dh_params_lock);
      PEM_write_bio_DHparams(bio_write, key);

      mysql_mutex_unlock(&dh_params_lock);
      BIO_free(bio_write);
    } else {
      key = PEM_read_bio_DHparams(bio_read, nullptr, nullptr, nullptr);
      handle_no_start_line_error_if_needed();
      BIO_free(bio_read);
    }

    /* generate keys. */
    if (DH_generate_key(key) != 1) {
      EVP_PKEY_free(evp_key);
      DH_free(key);

      *error = 1;
      return nullptr;
    }

    /* assign key to evp_key, only need to free evp_key,
    free both evp_key and key will cause crash. */
    EVP_PKEY_assign_DH(evp_key, key);
    if (record_keys_to_file(evp_key, key_file_name, dh_key_lock)) {
      /* failed to write keys to key file. */
      EVP_PKEY_free(evp_key);
      DH_free(key);

      *error = 1;
      return nullptr;
    }
  } else {
    /* get public parameters. */
    DSA *key = nullptr;
    sprintf(param_file_name, "dsa%d.pem", (int)bits);

    BIO *bio_read = BIO_new_file(param_file_name, "r");
    if (!bio_read) {
      key = DSA_new();
      DSA_generate_parameters_ex(key, (int)bits, nullptr, 0, nullptr, nullptr,
                                 nullptr);

      BIO *bio_write = BIO_new_file(param_file_name, "w");
      if (!bio_write) {
        EVP_PKEY_free(evp_key);
        DSA_free(key);

        *error = 1;
        return nullptr;
      }

      mysql_mutex_lock(&dsa_params_lock);
      PEM_write_bio_DSAparams(bio_write, key);

      mysql_mutex_unlock(&dsa_params_lock);
      BIO_free(bio_write);
    } else {
      key = PEM_read_bio_DSAparams(bio_read, nullptr, nullptr, nullptr);
      handle_no_start_line_error_if_needed();
      BIO_free(bio_read);
    }

    /* generate keys. */
    if (DSA_generate_key(key) != 1) {
      EVP_PKEY_free(evp_key);
      DSA_free(key);

      *error = 1;
      return nullptr;
    }

    /* assign key to evp_key, only need to free evp_key,
    free both evp_key and key will cause crash. */
    EVP_PKEY_assign_DSA(evp_key, key);
    if (record_keys_to_file(evp_key, key_file_name, dsa_key_lock)) {
      /* failed to write keys to key file. */
      EVP_PKEY_free(evp_key);
      DSA_free(key);

      *error = 1;
      return nullptr;
    }
  }

  /* write the evp_key into bio_priv. */
  BIO *bio_priv = BIO_new(BIO_s_mem());
  PEM_write_bio_PrivateKey(bio_priv, evp_key, nullptr, nullptr, 0, nullptr,
                           nullptr);

  /* prepare returning the bio_mem format priv_key. */
  size_t priv_len = BIO_pending(bio_priv);
  initid->ptr =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, priv_len + 1, MYF(MY_WME));
  memset(initid->ptr, 0, priv_len + 1);
  BIO_read(bio_priv, initid->ptr, priv_len);
  *length = strlen(initid->ptr);

  EVP_PKEY_free(evp_key);
  BIO_free(bio_priv);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for create_asymmetric_pub_key.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool create_asymmetric_pub_key_init(
    UDF_INIT *initid, /*!< in: Used to convey
                        infos between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the algo"
           " and the priv_key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "Wrong parameter types.\n");
    return true;
  }

  if (args->args[0] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 1 cannot be null\n");
    return true;
  }

  if (args->args[1] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 2 cannot be null\n");
    return true;
  }

  if (strcasecmp(args->args[0], "rsa") != 0 &&
      strcasecmp(args->args[0], "dh") != 0 &&
      strcasecmp(args->args[0], "dsa") != 0) {
    strcpy(message,
           "Make sure to use the same algo"
           " as the priv_key.\n");
    return true;
  } else if (!check_key_format(PRIVATE_KEY_TYPE, args->args[1])) {
    strcpy(message, "Wrong priv_key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void create_asymmetric_pub_key_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
Return the desired RSA/DSA/DH public key.
This function will search the matched pub_key in key_file, the digest
of priv_key is used as the id of pub_key.
@retval     pub_key     Success
@retval     nullptr     Failure.
******************************************************************************/
PLUGIN_EXPORT char *create_asymmetric_pub_key(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold
                        short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result is nullptr */
    char *error)      /*!< out: true if error occurs,
                        is_null=1 will be set */
{
  *is_null = 0;
  *error = 0;

  char key_file_name[KEY_FILENAME_LEN] = "";
  char lower_algo[ALGORITHM_LEN] = "";
  char input_digest[MD5_DIGEST_LENGTH * 2 + 1] = "",
                                            read_digest[MD5_DIGEST_LENGTH * 2 +
                                                        1] = "";
  EVP_PKEY *evp_pub = nullptr;

  str_tolower(args->args[0], lower_algo, ALGORITHM_LEN);
  sprintf(key_file_name, "%s%s.pem", KEY_FILE_PREFIX, lower_algo);

  BIO *bio_file = BIO_new_file(key_file_name, "r");
  if (!bio_file) {
    *error = 1;
    return nullptr;
  }

  calc_privkey_digest(args->args[1], input_digest);

  /* searching the pub_key matches the inputed priv_key. */
  BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
  evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();

  while (evp_pub && memcmp(input_digest, read_digest, MD5_DIGEST_LENGTH)) {
    EVP_PKEY_free(evp_pub);

    BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
    evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
    handle_no_start_line_error_if_needed();
  }

  if (!evp_pub) {
    /* failed in searching pub_key. */
    BIO_free(bio_file);
    *error = 1;
    return nullptr;
  }

  /* write the evp_pub into bio_pub. */
  BIO *bio_pub = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(bio_pub, evp_pub);

  /* prepare returning bio_mem format pub_key. */
  size_t pub_len = BIO_pending(bio_pub);
  initid->ptr =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, pub_len + 1, MYF(MY_WME));
  memset(initid->ptr, 0, pub_len + 1);
  BIO_read(bio_pub, initid->ptr, pub_len);
  *length = strlen(initid->ptr);

  BIO_free(bio_file);
  EVP_PKEY_free(evp_pub);
  BIO_free(bio_pub);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for asymmetric_derive.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool asymmetric_derive_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: one dh_priv_key"
           " and the other dh_pub_key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }

  if (!check_key_format(PRIVATE_KEY_TYPE, args->args[0])) {
    strcpy(message, "Wrong priv_key format.\n");
    return true;
  } else if (!check_key_format(PUBLIC_KEY_TYPE, args->args[1])) {
    strcpy(message, "Wrong pub_key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void asymmetric_derive_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
The DH algorithm is a key negotiation algorithm.
It can use A's priv_key and B's pub_key to derive a symmetric key.
@retval     derive_key     Success.
@retval     nullptr        Failure.
******************************************************************************/
PLUGIN_EXPORT char *asymmetric_derive(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold
                        short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result is nullptr */
    char *error)      /*!< out: true if error occurs,
                        is_null=1 will be set */
{
  *is_null = 0;
  *error = 0;

  BIO *bio_priv = BIO_new(BIO_s_mem());
  BIO *bio_pub = BIO_new(BIO_s_mem());

  BIO_write(bio_priv, args->args[0], args->lengths[0]);
  BIO_write(bio_pub, args->args[1], args->lengths[1]);

  EVP_PKEY *evp_priv =
      PEM_read_bio_PrivateKey(bio_priv, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_pub, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();

  BIO_free(bio_priv);
  BIO_free(bio_pub);

  if (!evp_priv || !evp_pub) {
    /* failed in tranforming the input keys into EVP_PKEY format. */
    if (evp_priv) EVP_PKEY_free(evp_priv);
    if (evp_pub) EVP_PKEY_free(evp_pub);

    *error = 1;
    return nullptr;
  }

  DH *priv = EVP_PKEY_get1_DH(evp_priv);
  DH *pub = EVP_PKEY_get1_DH(evp_pub);

  EVP_PKEY_free(evp_priv);
  EVP_PKEY_free(evp_pub);

  int blen = DH_size(priv);
  uchar *key = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
  memset(key, 0, blen);

  if (DH_compute_key(key,
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                     pub->pub_key,
#else
                     DH_get0_pub_key(pub),
#endif
                     priv) == -1) {
    *error = 1;
    goto free_resource;
  }

  initid->ptr =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, 2 * blen + 1, MYF(MY_WME));
  memset(initid->ptr, 0, 2 * blen + 1);

  /* tranfer the symmetric key into hexadecimal format. */
  transfer(key, initid->ptr, blen);
  *length = 2 * blen;

free_resource:
  DH_free(priv);
  DH_free(pub);
  my_free(key);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for create_digest.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool create_digest_init(
    UDF_INIT *initid, /*!< in: Used to convey
                        infos between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the algo"
           " and the clear text.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }
  if (args->args[0] == nullptr) {
    strcpy(message, "Wrong parameter value. Argument 1 cannot be null\n");
    return true;
  }

  if (strcasecmp(args->args[0], "md5") != 0 &&
      strcasecmp(args->args[0], "sha1") != 0 &&
      strcasecmp(args->args[0], "sha224") != 0 &&
      strcasecmp(args->args[0], "sha256") != 0 &&
      strcasecmp(args->args[0], "sha384") != 0 &&
      strcasecmp(args->args[0], "sha512") != 0) {
    strcpy(message,
           "Choose'md5', 'sha1', 'sha224', sha256',"
           " 'sha384' or 'sha512' as the first parameter.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void create_digest_deinit(UDF_INIT *) {}

/*****************************************************************************
Generate a digest using MD5/SHA1/SHA224/SHA256/SHA384/SHA512.
@retval     digest     Success.
@retval     nullptr    Failure.
******************************************************************************/
PLUGIN_EXPORT char *create_digest(
    UDF_INIT *,     /*!< in: Used to convey infos
                      between functions */
    UDF_ARGS *args, /*!< in: Array of arguments */
    char *result,   /*!< out: size:256, to hold short result */
    ulong *length,  /*!< out: length of result */
    char *is_null,  /*!< out: true if result is nullptr */
    char *error)    /*!< out: true if error occurs, is_null=1
                      will be set */
{
  *is_null = 0;
  *error = 0;

  uchar md5[MD5_DIGEST_LENGTH] = "", sha1[SHA_DIGEST_LENGTH] = "",
        sha224[SHA224_DIGEST_LENGTH] = "", sha256[SHA256_DIGEST_LENGTH] = "",
        sha384[SHA384_DIGEST_LENGTH] = "", sha512[SHA512_DIGEST_LENGTH] = "";

  if (strcasecmp(args->args[0], "md5") == 0) {
    calc_md5_digest(args->args[1], args->lengths[1], md5);

    transfer(md5, result, MD5_DIGEST_LENGTH);
    *length = MD5_DIGEST_LENGTH;
  } else if (strcasecmp(args->args[0], "sha1") == 0) {
    calc_sha1_digest(args->args[1], args->lengths[1], sha1);

    transfer(sha1, result, SHA_DIGEST_LENGTH);
    *length = SHA_DIGEST_LENGTH;
  } else if (strcasecmp(args->args[0], "sha224") == 0) {
    calc_sha224_digest(args->args[1], args->lengths[1], sha224);

    transfer(sha224, result, SHA224_DIGEST_LENGTH);
    *length = SHA224_DIGEST_LENGTH;
  } else if (strcasecmp(args->args[0], "sha256") == 0) {
    calc_sha256_digest(args->args[1], args->lengths[1], sha256);

    transfer(sha256, result, SHA256_DIGEST_LENGTH);
    *length = SHA256_DIGEST_LENGTH;
  } else if (strcasecmp(args->args[0], "sha384") == 0) {
    calc_sha384_digest(args->args[1], args->lengths[1], sha384);

    transfer(sha384, result, SHA384_DIGEST_LENGTH);
    *length = SHA384_DIGEST_LENGTH;
  } else {
    calc_sha512_digest(args->args[1], args->lengths[1], sha512);

    transfer(sha512, result, SHA512_DIGEST_LENGTH);
    *length = SHA512_DIGEST_LENGTH;
  }

  return result;
}

/*****************************************************************************
Check input parameters for asymmetric_encrypt.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool asymmetric_encrypt_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey error
                        message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the message"
           " to encrypt and one rsa key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }

  if (!check_key_format(PRIVATE_KEY_TYPE, args->args[1]) &&
      !check_key_format(PUBLIC_KEY_TYPE, args->args[1])) {
    strcpy(message, "Wrong key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void asymmetric_encrypt_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
Encrypt message with RSA key.
The msg can be encrypted with a pub_key and decrypted with a priv_key,
or encrypted with a priv_key and decrypted with a pub_key.
@retval     cipher     Success.
@retval     nullptr    Failure.
******************************************************************************/
PLUGIN_EXPORT char *asymmetric_encrypt(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result is nullptr */
    char *error)      /*!< out: true if error occurs, is_null=1
                        will be set */
{
  *is_null = 0;
  *error = 0;

  uchar *cipher = nullptr;
  size_t blen;
  RSA *priv = nullptr, *pub = nullptr;

  /* the inputed key may be priv_key or pub_key. */
  BIO *bio_priv = BIO_new(BIO_s_mem());
  BIO *bio_pub = BIO_new(BIO_s_mem());

  BIO_write(bio_priv, args->args[1], args->lengths[1]);
  BIO_write(bio_pub, args->args[1], args->lengths[1]);

  EVP_PKEY *evp_priv =
      PEM_read_bio_PrivateKey(bio_priv, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_pub, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();

  BIO_free(bio_priv);
  BIO_free(bio_pub);

  /* check the key type. */
  if (evp_priv && evp_pub) {
    *error = 1;
    return nullptr;
  } else if (evp_priv) {
    priv = EVP_PKEY_get1_RSA(evp_priv);
    blen = RSA_size(priv);
    cipher = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
    memset(cipher, 0, blen);

    if (RSA_private_encrypt(args->lengths[0], (uchar *)args->args[0], cipher,
                            priv, RSA_PKCS1_PADDING) <= 0) {
      *error = 1;
      goto free_resource;
    }
  } else if (evp_pub) {
    pub = EVP_PKEY_get1_RSA(evp_pub);
    blen = RSA_size(pub);
    cipher = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
    memset(cipher, 0, blen);

    if (RSA_public_encrypt(args->lengths[0], (uchar *)args->args[0], cipher,
                           pub, RSA_PKCS1_PADDING) <= 0) {
      *error = 1;
      goto free_resource;
    }
  } else {
    *error = 1;
    return nullptr;
  }

  initid->ptr =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, 2 * blen + 1, MYF(MY_WME));
  memset(initid->ptr, 0, 2 * blen + 1);

  transfer(cipher, initid->ptr, blen);
  *length = strlen(initid->ptr);

free_resource:
  my_free(cipher);
  if (priv) RSA_free(priv);
  if (pub) RSA_free(pub);
  if (evp_priv) EVP_PKEY_free(evp_priv);
  if (evp_pub) EVP_PKEY_free(evp_pub);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for asymmetric_decrypt.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool asymmetric_decrypt_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the encrypted"
           " message and the other rsa key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }

  if (!check_key_format(PRIVATE_KEY_TYPE, args->args[1]) &&
      !check_key_format(PUBLIC_KEY_TYPE, args->args[1])) {
    strcpy(message, "Wrong key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void asymmetric_decrypt_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
Decrypt message with RSA key.
@retval     clear_msg     Success.
@retval     nullptr       Failure.
******************************************************************************/
PLUGIN_EXPORT char *asymmetric_decrypt(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result is nullptr */
    char *error)      /*!< out: true if error occurs, is_null=1
                 will be set */
{
  *is_null = 0;
  *error = 0;

  RSA *priv = nullptr, *pub = nullptr;
  size_t blen = args->lengths[0] / 2;
  uchar *cipher = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));

  /* transfer the cipher from hexadecimal format to uchar. */
  memset(cipher, 0, blen);
  transfer_back(cipher, args->args[0], args->lengths[0]);

  initid->ptr = (char *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
  memset(initid->ptr, 0, blen);

  /* the inputed key may be priv_key or pub_key. */
  BIO *bio_priv = BIO_new(BIO_s_mem());
  BIO *bio_pub = BIO_new(BIO_s_mem());

  BIO_write(bio_priv, args->args[1], args->lengths[1]);
  BIO_write(bio_pub, args->args[1], args->lengths[1]);

  EVP_PKEY *evp_priv =
      PEM_read_bio_PrivateKey(bio_priv, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_pub, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();

  BIO_free(bio_priv);
  BIO_free(bio_pub);

  /* check the key type. */
  if (evp_priv && evp_pub) {
    *error = 1;
    goto free_resource;
  } else if (evp_priv) {
    priv = EVP_PKEY_get1_RSA(evp_priv);

    if (RSA_private_decrypt(blen, cipher, (uchar *)initid->ptr, priv,
                            RSA_PKCS1_PADDING) <= 0) {
      *error = 1;
      goto free_resource;
    }
  } else if (evp_pub) {
    pub = EVP_PKEY_get1_RSA(evp_pub);

    if (RSA_public_decrypt(blen, cipher, (uchar *)initid->ptr, pub,
                           RSA_PKCS1_PADDING) <= 0) {
      *error = 1;
      goto free_resource;
    }
  } else {
    *error = 1;
    goto free_resource;
  }

  *length = strlen(initid->ptr);

free_resource:
  if (priv) RSA_free(priv);
  if (pub) RSA_free(pub);
  if (evp_priv) EVP_PKEY_free(evp_priv);
  if (evp_pub) EVP_PKEY_free(evp_pub);
  my_free(cipher);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for asymmetric_sign.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool asymmetric_sign_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 3) {
    strcpy(message,
           "Requires two parameters: the algo,"
           " the hash and the priv_key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT ||
             args->arg_type[2] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }

  if (strcasecmp(args->args[0], "rsa") != 0 &&
      strcasecmp(args->args[0], "dsa") != 0) {
    strcpy(message, "Choose 'rsa' or 'dsa' as the first parameter.\n");
    return true;
  } else if (!check_key_format(PRIVATE_KEY_TYPE, args->args[2])) {
    strcpy(message, "Wrong priv_key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void asymmetric_sign_deinit(UDF_INIT *initid) {
  my_free(initid->ptr);
}

/*****************************************************************************
Calculate digital signatures using RSA/DSA.
It's recommended to use with 'create_digest'.
To avoid an empty output when using RSA, the key has to be long enough.
@retval     signature     Success.
@retval     nullptr       Failure.
******************************************************************************/
PLUGIN_EXPORT char *asymmetric_sign(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *,           /*!< out: size:256, to hold short result */
    ulong *length,    /*!< out: length of result */
    char *is_null,    /*!< out: true if result is nullptr */
    char *error)      /*!< out: true if error occurs, is_null=1
                        will be set */
{
  *is_null = 0;
  *error = 0;

  size_t blen;
  uchar *signature = nullptr;
  size_t slen = 0;

  BIO *bio_mem = BIO_new(BIO_s_mem());
  BIO_write(bio_mem, args->args[2], args->lengths[2]);
  EVP_PKEY *evp_priv =
      PEM_read_bio_PrivateKey(bio_mem, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  BIO_free(bio_mem);

  if (!evp_priv) {
    *error = 1;
    return nullptr;
  }

  if (strcasecmp(args->args[0], "rsa") == 0) {
    RSA *priv = EVP_PKEY_get1_RSA(evp_priv);
    if (!priv) {
      *error = 1;
      goto free_resource;
    }

    blen = RSA_size(priv);
    signature = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
    memset(signature, 0, blen);

    if (RSA_sign(NID_sha512, (uchar *)args->args[1], args->lengths[1],
                 signature, (unsigned int *)&slen, priv) != 1) {
      /* failed in calculating digital signature. */
      RSA_free(priv);
      *error = 1;
      goto free_resource;
    }

    RSA_free(priv);
  } else {
    DSA *priv = EVP_PKEY_get1_DSA(evp_priv);
    if (!priv) {
      *error = 1;
      goto free_resource;
    }

    blen = DSA_size(priv);
    signature = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, blen, MYF(MY_WME));
    memset(signature, 0, blen);

    if (DSA_sign(0, (uchar *)args->args[1], args->lengths[1], signature,
                 (unsigned int *)&slen, priv) != 1) {
      /* failed in calculating signature. */
      DSA_free(priv);
      *error = 1;
      goto free_resource;
    }

    DSA_free(priv);
  }

  initid->ptr =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, slen * 2 + 1, MYF(MY_WME));
  memset(initid->ptr, 0, slen * 2 + 1);

  /* tranfer the signature into hexadecimal format. */
  transfer(signature, initid->ptr, slen);
  *length = strlen(initid->ptr);
  /* length may equal 0, when RSA key isn't long enough. */
  if (0 == *length) {
    *error = 1;
  }

free_resource:
  EVP_PKEY_free(evp_priv);
  my_free(signature);

  return initid->ptr;
}

/*****************************************************************************
Check input parameters for asymmetric_verify.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool asymmetric_verify_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (args->arg_count != 4) {
    strcpy(message,
           "Requires four parameters: the algo,"
           " the hash, the sig, the pub_key.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != STRING_RESULT ||
             args->arg_type[2] != STRING_RESULT ||
             args->arg_type[3] != STRING_RESULT) {
    strcpy(message, "All parameter types should be STRING_RESULT.\n");
    return true;
  }

  if (strcasecmp(args->args[0], "rsa") != 0 &&
      strcasecmp(args->args[0], "dsa") != 0) {
    strcpy(message, "Choose 'rsa' or 'dsa' as the first parameter.\n");
    return true;
  } else if (!check_key_format(PUBLIC_KEY_TYPE, args->args[3])) {
    strcpy(message, "Wrong pub_key format.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void asymmetric_verify_deinit(UDF_INIT *) {}

/*****************************************************************************
verify a RSA/DSA digital signature.
@retval     Succeed!     Success.
@retval     Failed!      Failure.
******************************************************************************/
PLUGIN_EXPORT char *asymmetric_verify(
    UDF_INIT *,     /*!< in: Used to convey infos
                      between functions */
    UDF_ARGS *args, /*!< in: Array of arguments */
    char *result,   /*!< out: size:256, to hold short result */
    ulong *length,  /*!< out: length of result */
    char *is_null,  /*!< out: true if result is nullptr */
    char *error)    /*!< out: true if error occurs, is_null=1
                      will be set */
{
  *is_null = 0;
  *error = 0;

  bool verified = false;

  BIO *bio_mem = BIO_new(BIO_s_mem());
  BIO_write(bio_mem, args->args[3], args->lengths[3]);
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_mem, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  BIO_free(bio_mem);

  if (!evp_pub) {
    *error = 1;
    return nullptr;
  }

  /* tranfer the signature from hexadecimal format to uchar. */
  size_t slen = args->lengths[2] / 2;
  uchar *signature =
      (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, slen, MYF(MY_WME));
  memset(signature, 0, slen);
  transfer_back(signature, args->args[2], args->lengths[2]);

  if (strcasecmp(args->args[0], "rsa") == 0) {
    RSA *pub = EVP_PKEY_get1_RSA(evp_pub);
    if (!pub) {
      *error = 1;
      goto free_resource;
    }

    verified = RSA_verify(NID_sha512, (uchar *)args->args[1], args->lengths[1],
                          signature, slen, pub);

    RSA_free(pub);
  } else {
    DSA *pub = EVP_PKEY_get1_DSA(evp_pub);
    if (!pub) {
      *error = 1;
      goto free_resource;
    }

    verified = DSA_verify(0, (uchar *)args->args[1], args->lengths[1],
                          signature, slen, pub);

    DSA_free(pub);
  }

  if (verified) {
    strcpy(result, "Succeed!");
  } else {
    strcpy(result, "Failed!");
  }

  *length = strlen(result);

free_resource:
  my_free(signature);
  EVP_PKEY_free(evp_pub);

  return result;
}

/*****************************************************************************
Check input parameters for count_key_records.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool count_key_records_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 1) {
    strcpy(message, "Requires one parameter: the algo.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT) {
    strcpy(message, "Wrong parameter types.\n");
    return true;
  }

  if (args->args[0] == nullptr) {
    strcpy(message,
           "Wrong parameter value. Argument 1 cannot be null or empty\n");
    return true;
  }

  if (strcasecmp(args->args[0], "rsa") != 0 &&
      strcasecmp(args->args[0], "dh") != 0 &&
      strcasecmp(args->args[0], "dsa") != 0) {
    strcpy(message, "Choose 'rsa', 'dh' or 'dsa' as the parameter.\n");
    return true;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void count_key_records_deinit(UDF_INIT *) {}

/*****************************************************************************
Return the counts of key-pairs in RSA/DSA/DH key-file.
@retval     counts of key-pairs     Success.
@retval     nullptr                 Failure.
*****************************************************************************/
PLUGIN_EXPORT int64 count_key_records(
    UDF_INIT *,     /*!< in: Used to convey infos
                      between functions */
    UDF_ARGS *args, /*!< in: Array of arguments */
    char *is_null,  /*!< out: true if result is nullptr */
    char *error)    /*!< out: true if error occurs, is_null=1 will be set */
{
  *is_null = 0;
  *error = 0;

  char key_file_name[KEY_FILENAME_LEN] = "";
  char lower_algo[ALGORITHM_LEN] = "";
  char read_digest[MD5_DIGEST_LENGTH * 2 + 1] = "";
  int64 cnt = 0;

  str_tolower(args->args[0], lower_algo, ALGORITHM_LEN);
  sprintf(key_file_name, "%s%s.pem", KEY_FILE_PREFIX, lower_algo);

  int r = access(key_file_name, F_OK);
  if (r < 0) {
    /* file not exist, so there is no key. */
    return 0;
  }

  BIO *bio_file = BIO_new_file(key_file_name, "r");
  if (!bio_file) {
    *error = 1;
    return false;
  }

  BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();

  while (evp_pub) {
    ++cnt;
    EVP_PKEY_free(evp_pub);

    BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
    evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
    handle_no_start_line_error_if_needed();
  }

  BIO_free(bio_file);

  return cnt;
}

/*****************************************************************************
Check input parameters for remove_key_records.
@retval     false     Success.
@retval     true      Failure. Error infos will be kept in message.
*****************************************************************************/
PLUGIN_EXPORT bool remove_key_records_init(
    UDF_INIT *initid, /*!< in: Used to convey infos
                        between functions */
    UDF_ARGS *args,   /*!< in: Array of arguments */
    char *message)    /*!< out: Used to convey
                        error message */
{
  if (!check_system_variables()) {
    strcpy(message, "The plugin works unnormally.\n");
    return true;
  } else if (args->arg_count != 2) {
    strcpy(message,
           "Requires two parameters: the algo"
           " and the counts of the keys to remove.\n");
    return true;
  } else if (args->arg_type[0] != STRING_RESULT ||
             args->arg_type[1] != INT_RESULT) {
    strcpy(message, "Wrong parameter types.\n");
    return true;
  }

  if (args->args[0] == nullptr) {
    strcpy(message,
           "Wrong parameter value. Argument 1 cannot be null or empty\n");
    return true;
  }

  if (strcasecmp(args->args[0], "rsa") != 0 &&
      strcasecmp(args->args[0], "dh") != 0 &&
      strcasecmp(args->args[0], "dsa") != 0) {
    strcpy(message, "Choose 'rsa', 'dh' or 'dsa' as the parameter.\n");
    return false;
  }

  initid->maybe_null = 1;
  return false;
}

PLUGIN_EXPORT void remove_key_records_deinit(UDF_INIT *initid) {
  free(initid->ptr);
}

/*****************************************************************************
Remove the first n key-pairs in RSA/DSA/DH key-file.
@retval     Successfully removed!     Success.
@retval     nullptr                   Failure.
******************************************************************************/
PLUGIN_EXPORT char *remove_key_records(
    UDF_INIT *,     /*!< in: Used to convey infos
                      between functions */
    UDF_ARGS *args, /*!< in: Array of arguments */
    char *result,   /*!< out: size:256, to hold short result */
    ulong *length,  /*!< out: length of result */
    char *is_null,  /*!< out: true if result is nullptr */
    char *error)    /*!< out: true if error occurs, is_null=1
                      will be set */
{
  *is_null = 0;
  *error = 0;

  char key_file_name[KEY_FILENAME_LEN] = "";
  char tmp_file_name[KEY_FILENAME_LEN] = "";
  char lower_algo[ALGORITHM_LEN] = "";
  int64 cnt = 0;
  int64 threshold = *((int64 *)args->args[1]);
  char read_digest[MD5_DIGEST_LENGTH * 2 + 1];

  str_tolower(args->args[0], lower_algo, ALGORITHM_LEN);
  sprintf(key_file_name, "%s%s.pem", KEY_FILE_PREFIX, lower_algo);
  sprintf(tmp_file_name, "tmp_%s.pem", lower_algo);

  /* create a tmp-file to record the remain key-pairs. */
  BIO *bio_file = BIO_new_file(key_file_name, "r");
  BIO *bio_tmp = BIO_new_file(tmp_file_name, "w");

  if (!bio_file || !bio_tmp) {
    if (bio_file) BIO_free(bio_file);
    if (bio_tmp) BIO_free(bio_tmp);

    *error = 1;
    return nullptr;
  }

  if (strcasecmp(args->args[0], "rsa") != 0) {
    mysql_mutex_lock(&rsa_key_lock);
  } else if (strcasecmp(args->args[0], "dh") != 0) {
    mysql_mutex_lock(&dh_key_lock);
  } else {
    mysql_mutex_lock(&dsa_key_lock);
  }

  BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
  EVP_PKEY *evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
  handle_no_start_line_error_if_needed();
  ++cnt;

  while (evp_pub) {
    if (cnt > threshold) {
      BIO_write(bio_tmp, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
      PEM_write_bio_PUBKEY(bio_tmp, evp_pub);
    }

    EVP_PKEY_free(evp_pub);

    BIO_read(bio_file, read_digest, MD5_DIGEST_LENGTH * 2 + 1);
    evp_pub = PEM_read_bio_PUBKEY(bio_file, nullptr, nullptr, nullptr);
    handle_no_start_line_error_if_needed();
    ++cnt;
  }

  /* remove the origin key-file, rename the tmp-file. */
  BIO_free(bio_file);
  BIO_free(bio_tmp);
  int r = remove(key_file_name);
  if (r < 0) {
    // TODO : remove file failed. plugin is not valid
  }

  r = rename(tmp_file_name, key_file_name);
  if (r < 0) {
    // TODO : rename file faile. plugin is not valid
  }

  if (strcasecmp(args->args[0], "rsa") != 0) {
    mysql_mutex_unlock(&rsa_key_lock);
  } else if (strcasecmp(args->args[0], "dh") != 0) {
    mysql_mutex_unlock(&dh_key_lock);
  } else {
    mysql_mutex_unlock(&dsa_key_lock);
  }

  strcpy(result, "Successfully removed!");
  *length = strlen(result);

  return result;
}
