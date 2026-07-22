#include "concurrent_kvstore.hpp"

#include <mutex>
#include <optional>
#include <set>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    auto key = req->key;
    auto bucket_index = store.bucket(key);
    store.bucket_locks[bucket_index].lock_shared();
    auto result = store.getIfExists(bucket_index, key);
    if (result.has_value()) {
        res->value = result->value;
        store.bucket_locks[bucket_index].unlock_shared();
        return true;
    }
    store.bucket_locks[bucket_index].unlock_shared();
    return false;
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    auto key = req->key;
    auto value = req->value;
    auto bucket_index = store.bucket(key);
    store.bucket_locks[bucket_index].lock();
    store.insertItem(bucket_index, key, value);
    store.bucket_locks[bucket_index].unlock();
    return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    auto key = req->key;
    auto value = req->value;
    auto bucket_index = store.bucket(key);
    std::string appended_value;

    store.bucket_locks[bucket_index].lock();

    auto result = store.getIfExists(bucket_index, key);
    if (result.has_value()) {
        appended_value = result->value + value;
    } else {
        appended_value = value;
    }

    store.insertItem(bucket_index, key, appended_value);

    store.bucket_locks[bucket_index].unlock();
    return true;
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    auto key = req->key;
    auto bucket_index = store.bucket(key);

    store.bucket_locks[bucket_index].lock();

    auto get_result = store.getIfExists(bucket_index, key);
    if (get_result.has_value()) {
        res->value = get_result->value;
    }

    auto remove_result = store.removeItem(bucket_index, key);
    store.bucket_locks[bucket_index].unlock();
    return remove_result;
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    std::set<size_t> locked_buckets_idx;

    // 对于 1->3 / 3->1 这种循环死锁，如果已知要加哪些锁，可以统一一次性加，那么让所有操作都按一个固定顺序加，就可以避免死锁
    // set 自带排序
    for (size_t i = 0; i < req->keys.size(); i++) {
        auto key = req->keys[i];
        auto bucket_index = store.bucket(key);
        locked_buckets_idx.insert(bucket_index);
    }

    for (auto &lock_idx: locked_buckets_idx) {
        store.bucket_locks[lock_idx].lock_shared();
    }

    for (size_t i = 0; i < req->keys.size(); i++) {
        auto key = req->keys[i];
        auto bucket_index = store.bucket(key);
        auto result = store.getIfExists(bucket_index, key);

        if (result.has_value()) {
            res->values.push_back(result->value);
        } else {
            for (auto& lock_idx : locked_buckets_idx) {
                store.bucket_locks[lock_idx].unlock_shared();
            }
            return false;
        }
    }
    for (auto& lock_idx : locked_buckets_idx) {
        store.bucket_locks[lock_idx].unlock_shared();
    }
    return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
    // TODO (Part A, Step 3 and Step 4): Implement!
    if (req->keys.size() != req->values.size()) {
        return false;
    }

    std::set<size_t> locked_buckets_idx;

    for (size_t i = 0; i < req->keys.size(); i++) {
        auto key = req->keys[i];
        auto bucket_index = store.bucket(key);
        locked_buckets_idx.insert(bucket_index);
    }

    for (auto &lock_idx: locked_buckets_idx) {
        store.bucket_locks[lock_idx].lock();
    }

    for (size_t i = 0; i < req->keys.size(); i++) {
        auto key = req->keys[i];
        auto value = req->values[i];
        auto bucket_index = store.bucket(key);
        store.insertItem(bucket_index, key, value);
    }

    for (auto& lock_idx : locked_buckets_idx) {
        store.bucket_locks[lock_idx].unlock();
    }
    return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
    // TODO (Part A, Step 3 and Step 4): Implement!
    for (size_t i = 0; i < store.buckets.size(); i++) {
        store.bucket_locks[i].lock_shared();
    }
    std::vector<std::string> all_keys;
    for (auto& bucket : store.buckets) {
        for (auto& [key, val] : bucket) {
            all_keys.emplace_back(key);
        }
    }
    for (size_t i = 0; i < store.buckets.size(); i++) {
        store.bucket_locks[i].unlock_shared();
    }
    return all_keys;
}
