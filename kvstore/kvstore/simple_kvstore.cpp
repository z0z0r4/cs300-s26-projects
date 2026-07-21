#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    auto key = req->key;
    if (kv_store_.find(key) != kv_store_.end()) {
        auto value = kv_store_[key];
        res->value = value;
        global_mutex_.unlock();
        return true;
    }
    global_mutex_.unlock();
    return false;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    auto key = req->key;
    auto value = req->value;
    kv_store_[key] = value;
    global_mutex_.unlock();
    return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    auto key = req->key;
    auto value = req->value;
    std::string appended_value;
    if (kv_store_.find(key) != kv_store_.end()) {
        auto current_value = kv_store_[key];
        appended_value = current_value + value;
    } else {
        appended_value = value;
    }
    kv_store_[key] = appended_value;
    global_mutex_.unlock();
    return true;
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    auto key = req->key;
    if (kv_store_.find(key) != kv_store_.end()) {
        auto value = kv_store_[key];
        res->value = value;
        kv_store_.erase(key);
        global_mutex_.unlock();
        return true;
    }
    global_mutex_.unlock();
    return false;
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    for (auto& key : req->keys) {
        if (kv_store_.find(key) != kv_store_.end()) {
            auto value = kv_store_[key];
            res->values.push_back(value);
        } else {
            global_mutex_.unlock();
            return false;
        }
    }
    global_mutex_.unlock();
    return true;
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
    // TODO (Part A, Step 1 and Step 2): Implement!
    if (req->keys.size() != req->values.size()) {
      return false;
    }

    global_mutex_.lock();
    
    for (size_t i = 0; i < req->keys.size(); i++) {
      auto key = req->keys[i];
      auto value = req->values[i];
      kv_store_[key] = value;
    }
    global_mutex_.unlock();
    return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
    // TODO (Part A, Step 1 and Step 2): Implement!
    global_mutex_.lock();
    std::vector<std::string> all_keys;
    all_keys.reserve(kv_store_.size());
    for (auto &kv : kv_store_) {
      auto key = kv.first;
      all_keys.emplace_back(key);
    }
    global_mutex_.unlock();
    return all_keys;
}
