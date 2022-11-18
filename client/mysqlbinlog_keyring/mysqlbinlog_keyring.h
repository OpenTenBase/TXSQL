#ifndef MYSQLBINLOG_KEYRING_INCLUDE
#define MYSQLBINLOG_KEYRING_INCLUDE

#include <memory>
#include <string>
#include <unordered_map>
typedef unsigned char uchar;

class mysqlbinlog_key {
public:
  std::string key_id;
  std::string key_type;
  std::string user_id;
  std::unique_ptr<uchar[]> key;
  size_t key_len;

  mysqlbinlog_key();
  mysqlbinlog_key(const mysqlbinlog_key &T);
  mysqlbinlog_key& operator= (const mysqlbinlog_key &T);
};
extern std::unordered_map<std::string, mysqlbinlog_key> g_hash_mysqlbinlog_key;

bool init_mysqlbinlog_key(std::string filename);

#endif  // MYSQLBINLOG_KEYRING_INCLUDED
