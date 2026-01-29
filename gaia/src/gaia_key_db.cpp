#include "gaia_key_db.hpp"
#include "gaia_dir.hpp"       // for isDirExist, makeDirRecursive
#include "gaia_exception.hpp" // for SysException, THROWLOG, THROW
#include "gaia_log.hpp"       // for LOG_TRA, LOG_CRI, LOG_INF, LOG_DBG
#include "gaia_scope_guard.hpp"
#include "leveldb/db.h"       // for DB
#include "leveldb/iterator.h" // for Iterator
#include "leveldb/options.h"  // for ReadOptions, WriteOptions, Options
#include "leveldb/status.h"   // for Status
#include <algorithm>          // for max
#include <cstdint>            // for uint32_t
#include <ctime>              // for time
#include <map>                // for operator==, _Rb_tree_const_iterator, map
#include <set>                // for set
#include <utility>            // for move

namespace
{
std::string kDbPrefix = "cf:";
}

namespace vp
{

std::shared_ptr<KeyDB> KeyDB::inst(const std::string &db_path)
{
    static std::shared_ptr<KeyDB> instance{new KeyDB{}};
    if (!db_path.empty() && instance->db_path_.empty())
    {
        std::lock_guard<std::mutex> lock{instance->mutex_};
        instance->db_path_ = db_path;
        instance->open();
    }
    return instance;
}

void KeyDB::reset()
{
    auto instance = KeyDB::inst();
    std::lock_guard<std::mutex> lock{instance->mutex_};
    instance->db_ptr_.reset();
    instance->db_path_.clear();
    instance->key_memory_.clear();
    instance->key_id_set_.clear();
}

void KeyDB::open()
{
    if (db_ptr_)
    {
        return;
    }
    srand(time(NULL)); // NOLINT:Needed for now (for random keystoreId generation)

    if (db_path_.empty())
    {
        THROWLOG(SysException, "DB path is empty");
    }

    if (!isDirExist(db_path_))
    {
        LOG_INF("Generating new DB at {}", db_path_);
        makeDirRecursive(db_path_);
    }

    leveldb::DB *new_db_ptr = nullptr;
    auto scope_guard = MakeGuard(
        [&]
        {
            if (new_db_ptr != nullptr && !db_ptr_)
            {
                delete new_db_ptr;
            }
        });

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, db_path_, &new_db_ptr);

    if (status.ok())
    {
        LOG_INF("Loading DB from: {}", db_path_);
        db_ptr_ = std::unique_ptr<leveldb::DB>(new_db_ptr);
    }
    else
    {
        LOG_CRI("DB open failed. Probable cause 1) remaining LOCK (unexpected exit) 2) no space left on the device.");
        LOG_CRI("1) Remove data/LOCK");
        LOG_CRI("2) Space is needed to create LOCK file");

        THROWLOG(SysException, "DB open failed: {}", status.ToString());
    }

    auto key_datas = this->getPrefix(kDbPrefix);
    for (auto &item : key_datas)
    {
        try
        {
            uint32_t id = std::stoul(item.first);
            key_id_set_.insert(id);
        }
        catch (...)
        {
            LOG_DBG("Failed to convert key id({}) to integer type.", item.first);
        }
    }
}

void KeyDB::put(const std::string &key, const std::string &value)
{
    LOG_TRA("");
    std::string tmp_value{};
    leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), key, &tmp_value);

    if (status.ok() || status.IsNotFound())
    {
        status = db_ptr_->Put(leveldb::WriteOptions(), key, value);
    }
    if (!status.ok())
    {
        THROWLOG(SysException, "Failed to put {} by error {}", key, status.ToString());
    }
}

void KeyDB::overWrite(const std::string &key, const std::string &value)
{
    LOG_TRA("");
    db_ptr_->Delete(leveldb::WriteOptions(), key);
    db_ptr_->Put(leveldb::WriteOptions(), key, value);
}

std::string KeyDB::get(const std::string &key)
{
    LOG_TRA("");

    std::string value{};
    leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), key, &value);
    if (!status.ok())
    {
        if (status.IsNotFound())
        {
            LOG_INF("Key {} not found. This could happen in the initial device setup, or digest before certificates", key);
            THROW(SysException, "Not Found value by Key : {}", key);
        }

        THROWLOG(SysException, "Failed to get {} by error {}. Data corruption?", key, status.ToString());
    }
    return std::move(value);
}

std::string KeyDB::get(const std::string &key, int &out_status)
{
    LOG_TRA("");

    std::string value{};
    leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), key, &value);
    if (!status.ok())
    {
        if (status.IsNotFound())
        {
            out_status = KEY_DB_NOT_FOUND;
            return {};
        }

        out_status = -1;
    }
    out_status = KEY_DB_FOUND;
    return std::move(value);
}

void KeyDB::del(const leveldb::Slice &key)
{
    LOG_TRA("");

    leveldb::Status status = db_ptr_->Delete(leveldb::WriteOptions(), key);
    if (!status.ok())
    {
        THROWLOG(SysException, "delete failed : key({}), status({})", key.ToString(), status.ToString());
    }
}

#define MAX_LOOP_CNT 99999

std::vector<std::tuple<std::string, std::string>> KeyDB::seek(const leveldb::Slice &start, uint32_t limit)
{
    LOG_TRA("");

    std::vector<std::tuple<std::string, std::string>> values;

    uint32_t cnt = limit == 0 ? MAX_LOOP_CNT : limit;

    std::unique_ptr<leveldb::Iterator> it(db_ptr_->NewIterator(leveldb::ReadOptions()));
    for (it->Seek(start); it->Valid() && cnt > 0; it->Next(), cnt--)
    {
        values.push_back(std::make_tuple(it->key().ToString(), it->value().ToString()));
    }

    return values;
}

void KeyDB::seek(const leveldb::Slice &start, std::function<bool(const leveldb::Slice &key, const leveldb::Slice &val)> func, uint32_t limit) // NOLINT: function object should be passed by value (C++11 §25.1/10)
{
    LOG_TRA("");

    uint32_t cnt = limit == 0 ? MAX_LOOP_CNT : limit;

    std::unique_ptr<leveldb::Iterator> it(db_ptr_->NewIterator(leveldb::ReadOptions()));
    bool reqContinue = true;
    for (it->Seek(start); it->Valid() && cnt > 0 && reqContinue; it->Next(), cnt--)
    {
        reqContinue = func(it->key(), it->value());
    }
}

void KeyDB::seekRange(const leveldb::Slice &start, const leveldb::Slice &end, std::function<void(const leveldb::Slice &key, const leveldb::Slice &val)> func) // NOLINT: function object should be passed by value (C++11 §25.1/10)
{
    LOG_TRA("");

    std::unique_ptr<leveldb::Iterator> it(db_ptr_->NewIterator(leveldb::ReadOptions()));
    for (it->Seek(start); it->Valid() && (it->key().compare(end) <= 0); it->Next())
    {
        func(it->key(), it->value());
    }
}

std::map<std::string, std::string> KeyDB::getPrefix(const leveldb::Slice &prefix)
{
    std::map<std::string, std::string> results{};
    std::unique_ptr<leveldb::Iterator> it(db_ptr_->NewIterator(leveldb::ReadOptions()));
    size_t keySize = prefix.size();
    for (it->Seek(prefix); it->Valid() && it->key().starts_with(prefix); it->Next())
    {
        leveldb::Slice key = it->key();
        results[key.ToString().substr(keySize)] = it->value().ToString();
    }

    return results;
}

void KeyDB::seekPrefix(const leveldb::Slice &prefix,
                       std::function<void(const leveldb::Slice &key, const leveldb::Slice &val)> func, uint32_t limit) // NOLINT: function object should be passed by value (C++11 §25.1/10)
{
    uint32_t cnt = limit == 0 ? MAX_LOOP_CNT : limit;

    std::unique_ptr<leveldb::Iterator> it(db_ptr_->NewIterator(leveldb::ReadOptions()));

    for (it->Seek(prefix); it->Valid() && it->key().starts_with(prefix) && cnt > 0; it->Next(), cnt--)
    {
        func(it->key(), it->value());
    }
}

// SW 관리시 사용
uint32_t KeyDB::getNewKeyIdRandom()
{
    uint32_t keyId = 0;
    while (true)
    {
        // 10으로 시작하도록 하여 RT키, HSM키와 충돌 회피
        keyId = (uint32_t)rand(); // NOLINT:Needed for now (for random keystoreId generation)
        keyId |= 1UL << 31;       // NOLINT:Need refactoring
        keyId |= 0UL << 30;       // NOLINT:Need refactoring
        if (keyId == 0)
        {
            continue;
        }

        auto it1 = key_id_set_.find(keyId);
        auto it2 = this->key_memory_.find(keyId);
        if (it1 == key_id_set_.end() && it2 == this->key_memory_.end())
        {
            break;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    key_id_set_.insert(keyId);

    return keyId;
}

// HW 관리시 사용. start 제공시 start부터 신규 키 생성
uint32_t KeyDB::getNewKeyIdSequential(size_t size, uint32_t start)
{
    uint32_t keyId = start;
    while (keyId < size + start)
    {
        auto it = key_id_set_.find(keyId);
        if (it == key_id_set_.end())
        {
            std::lock_guard<std::mutex> lock(mutex_);
            key_id_set_.insert(keyId);

            return keyId;
        }

        keyId++;
    }

    THROWLOG(SysException, "get new key failed, all spaces are being used. id: {}", keyId);
}

void KeyDB::putValue(uint32_t keyid, const OctStr &privKey) // NOLINT
{
    leveldb::Status status = db_ptr_->Put(leveldb::WriteOptions(), kDbPrefix + std::to_string(keyid), std::string(privKey.begin(), privKey.end()));
    if (!status.ok())
    {
        THROWLOG(SysException, "put key failed id : {}, {}", keyid, status.ToString());
    }
}

void KeyDB::getValue(uint32_t keyid, OctStr &privKey)
{
    auto it = this->key_memory_.find(keyid);
    if (it != this->key_memory_.end())
    {
        privKey = this->key_memory_[keyid];
    }
    else
    {
        std::string private_key_string{};
        leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), kDbPrefix + std::to_string(keyid), &private_key_string);
        if (!status.ok())
        {
            THROWLOG(SysException, "get key failed id : {}, {}", keyid, status.ToString());
        }

        privKey = OctStr{private_key_string.begin(), private_key_string.end()};
    }
}

void KeyDB::delValue(uint32_t keyid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = this->key_memory_.find(keyid);
    if (it != this->key_memory_.end())
    {
        this->key_memory_.erase(keyid);
    }
    else
    {
        key_id_set_.erase(keyid);
        leveldb::Status status = db_ptr_->Delete(leveldb::WriteOptions(), kDbPrefix + std::to_string(keyid));
        if (!status.ok())
        {
            THROWLOG(SysException, "del key failed id : {}, {}", keyid, status.ToString());
        }
    }
}

bool KeyDB::containsKey(uint32_t keyid)
{
    auto it = this->key_memory_.find(keyid);
    if (it == this->key_memory_.end())
    {
        std::string private_key_string;

        leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), kDbPrefix + std::to_string(keyid), &private_key_string);
        if (!status.ok())
        {
            return false;
        }
    }

    return true;
}

} // namespace vp