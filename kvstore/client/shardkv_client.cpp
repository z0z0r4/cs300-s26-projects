#include "shardkv_client.hpp"

std::optional<std::string> ShardKvClient::Get(const std::string& key) {
    // Query shardcontroller for config
    auto config = this->Query();
    if (!config) return std::nullopt;

    // find responsible server in config
    std::optional<std::string> server = config->get_server(key);
    if (!server) return std::nullopt;

    return SimpleClient{*server}.Get(key);
}

bool ShardKvClient::Put(const std::string& key, const std::string& value) {
    // Query shardcontroller for config
    auto config = this->Query();
    if (!config) return false;

    // find responsible server in config, then make Put request
    std::optional<std::string> server = config->get_server(key);
    if (!server) return false;
    return SimpleClient{*server}.Put(key, value);
}

bool ShardKvClient::Append(const std::string& key, const std::string& value) {
    // Query shardcontroller for config
    auto config = this->Query();
    if (!config) return false;

    // find responsible server in config, then make Append request
    std::optional<std::string> server = config->get_server(key);
    if (!server) return false;
    return SimpleClient{*server}.Append(key, value);
}

std::optional<std::string> ShardKvClient::Delete(const std::string& key) {
    // Query shardcontroller for config
    auto config = this->Query();
    if (!config) return std::nullopt;

    // find responsible server in config, then make Delete request
    std::optional<std::string> server = config->get_server(key);
    if (!server) return std::nullopt;
    return SimpleClient{*server}.Delete(key);
}

std::optional<std::vector<std::string>> ShardKvClient::MultiGet(
    const std::vector<std::string>& keys) {
    std::vector<std::string> values;
    // values.reserve(keys.size());
    values.resize(keys.size());
    // TODO (Part B, Step 3): Implement!

    auto config = this->Query();
    if (!config) return std::nullopt;

    std::map<std::string, std::vector<size_t>> server_to_keys_idx_map;
    for (size_t key_idx = 0; key_idx < keys.size(); key_idx++) {
        auto key = keys[key_idx];
        auto server = config.value().get_server(key);

        if (!server) {
            return std::nullopt;
        }

        server_to_keys_idx_map[server.value()].push_back(key_idx);
    }

    for (auto& [server, indices] : server_to_keys_idx_map) {
        std::vector<std::string> server_keys;
        server_keys.reserve(indices.size());
        for (size_t idx : indices) {
            server_keys.push_back(keys[idx]);
        }

        auto result = SimpleClient{server}.MultiGet(server_keys);
        if (!result) return std::nullopt;

        for (size_t i = 0; i < indices.size(); i++) {
            values[indices[i]] = result.value()[i];
        }
    }

    return values;
}

bool ShardKvClient::MultiPut(const std::vector<std::string>& keys,
                             const std::vector<std::string>& values) {
    // TODO (Part B, Step 3): Implement!
    auto config = this->Query();
    if (!config) return false;

    std::map<std::string, std::vector<size_t>> server_to_keys_idx_map;
    for (size_t idx = 0; idx < keys.size(); idx++) {
        auto key = keys[idx];
        auto server = config.value().get_server(key);

        if (!server) {
            return false;
        }

        server_to_keys_idx_map[server.value()].push_back(idx);
    }

    for (auto& [server, indices] : server_to_keys_idx_map) {
        std::vector<std::string> server_keys;
        std::vector<std::string> server_values;
        server_keys.reserve(indices.size());
        server_values.reserve(indices.size());
        for (size_t idx : indices) {
            server_keys.push_back(keys[idx]);
            server_values.push_back(values[idx]);
        }

        auto result = SimpleClient{server}.MultiPut(server_keys, server_values);
        if (!result) return false;
    }
    
    return true;
}

// Shardcontroller functions
std::optional<ShardControllerConfig> ShardKvClient::Query() {
    QueryRequest req;
    if (!this->shardcontroller_conn->send_request(req)) return std::nullopt;

    std::optional<Response> res = this->shardcontroller_conn->recv_response();
    if (!res) return std::nullopt;
    if (auto* query_res = std::get_if<QueryResponse>(&*res)) {
        return query_res->config;
    }

    return std::nullopt;
}

bool ShardKvClient::Move(const std::string& dest_server,
                         const std::vector<Shard>& shards) {
    MoveRequest req{dest_server, shards};
    if (!this->shardcontroller_conn->send_request(req)) return false;

    std::optional<Response> res = this->shardcontroller_conn->recv_response();
    if (!res) return false;
    if (auto* move_res = std::get_if<MoveResponse>(&*res)) {
        (void)move_res;
        return true;
    }

    return false;
}
