#pragma once

#include "gaia_octstr.hpp" // for OctStr
#include "leveldb/slice.h" // for Slice
#include <cstdlib>         // for size_t
#include <functional>      // for function
#include <map>             // for map
#include <memory>          // for shared_ptr
#include <mutex>           // for mutex
#include <set>             // for set
#include <stdint.h>        // for uint32_t
#include <string>          // for string
#include <tuple>           // for tuple
#include <unordered_map>   // for unordered_map
#include <vector>          // for vector
namespace leveldb
{
class DB;
}

#define KEY_DB_NOT_FOUND 1
#define KEY_DB_FOUND 0

namespace autocrypt
{

class KeyDB
{
public:
    KeyDB(const KeyDB &) = delete;
    KeyDB &operator=(const KeyDB &) = delete;
    KeyDB(KeyDB &&) = delete;
    KeyDB &operator=(KeyDB &&) = delete;

    static std::shared_ptr<KeyDB> inst(const std::string &db_path = "");
    static void reset();

    uint32_t getNewKeyIdRandom();
    uint32_t getNewKeyIdSequential(size_t size, uint32_t start = 0);
    void putValue(uint32_t keyid, const OctStr &privKey);
    void getValue(uint32_t keyid, OctStr &privKey);
    void delValue(uint32_t keyid);
    bool containsKey(uint32_t keyid);

    void put(const std::string &key, const std::string &value);
    void overWrite(const std::string &key, const std::string &value);
    void del(const leveldb::Slice &key);

    std::string get(const std::string &key);
    std::string get(const std::string &key, int &out_status);
    std::map<std::string, std::string> getPrefix(const leveldb::Slice &prefix);

    std::vector<std::tuple<std::string, std::string>> seek(const leveldb::Slice &start, uint32_t limit = 0);
    void seek(const leveldb::Slice &start, std::function<bool(const leveldb::Slice &key, const leveldb::Slice &val)> func, uint32_t limit = 0);
    void seekPrefix(const leveldb::Slice &prefix, std::function<void(const leveldb::Slice &key, const leveldb::Slice &val)> func, uint32_t limit = 0);
    void seekRange(const leveldb::Slice &start, const leveldb::Slice &end, std::function<void(const leveldb::Slice &key, const leveldb::Slice &val)> func);

    const std::string &getDbPath() { return db_path_; }

private:
    KeyDB() = default;
    void open();

    // SW Real time key 저장용 map
    // KeyStore에 넣지 않을 private key, symmetric key 저장용. 메모리에만 넣음
    std::unordered_map<uint32_t, OctStr> key_memory_;
    // KeyStore에 넣을 public key, symmetric key 관리용. 겹치지 않게 하기 위함
    std::set<uint32_t> key_id_set_;

    std::mutex mutex_;
    std::string db_path_;
    std::unique_ptr<leveldb::DB> db_ptr_;
};

} // namespace autocrypt