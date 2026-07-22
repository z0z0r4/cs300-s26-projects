#include "common/config.hpp"

#include <sstream>

#include "common/color.hpp"
#include "common/utils.hpp"

std::string ShardControllerConfig::print() {
    std::stringstream ss;
    ss << "Shardcontroller configuration: \n";
    for (auto&& [server, shards] : this->server_to_shards) {
        ss << "- " << server << ": ";
        for (auto&& s : shards) {
            ss << s;
            if (s != shards.back()) ss << ", ";
        }
        ss << '\n';
    }
    return ss.str();
}

std::optional<std::string> ShardControllerConfig::get_server(
    const std::string& key) {
    std::string key_uppercase = to_upper(key);
    // TODO (Part B, Step 2): Implement!
    // You should use key_uppercase (instead of key) in your implementation
    for (auto& [server, shards] : server_to_shards) {
        for (auto& shard : shards) {
            auto granularity = shard.granularity();
            std::string truncated_key;
            if (key_uppercase.size() < granularity) {
                truncated_key =
                    key_uppercase +
                    std::string("0", granularity - key_uppercase.size());
            } else if (key_uppercase.size() > granularity) {
                truncated_key = key_uppercase.substr(0, granularity);
            } else {
                truncated_key = key_uppercase;
            }

            if (truncated_key >= shard.lower && truncated_key <= shard.upper) {
                return server;
            }
        }
    }

    cerr_color(
        RED,
        "Shardcontroller config does not contain any server responsible for "
        "the key ",
        key);
    return std::nullopt;
}
