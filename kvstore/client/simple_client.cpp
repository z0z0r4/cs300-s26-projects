#include "simple_client.hpp"

std::optional<std::string> SimpleClient::Get(const std::string& key) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return std::nullopt;
    }

    GetRequest req{key};
    if (!conn->send_request(req)) return std::nullopt;

    std::optional<Response> res = conn->recv_response();
    if (!res) return std::nullopt;
    if (auto* get_res = std::get_if<GetResponse>(&*res)) {
        return get_res->value;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW, "Failed to Get value from server: ", error_res->msg);
    }

    return std::nullopt;
}

bool SimpleClient::Put(const std::string& key, const std::string& value) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return false;
    }

    PutRequest req{key, value};
    if (!conn->send_request(req)) return false;

    std::optional<Response> res = conn->recv_response();
    if (!res) return false;
    if (auto* put_res = std::get_if<PutResponse>(&*res)) {
        (void)put_res;
        return true;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW, "Failed to Put value to server: ", error_res->msg);
    }

    return false;
}

bool SimpleClient::Append(const std::string& key, const std::string& value) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return false;
    }

    AppendRequest req{key, value};
    if (!conn->send_request(req)) return false;

    std::optional<Response> res = conn->recv_response();
    if (!res) return false;
    if (auto* append_res = std::get_if<AppendResponse>(&*res)) {
        (void)append_res;
        return true;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW,
                   "Failed to Append value to server: ", error_res->msg);
    }

    return false;
}

std::optional<std::string> SimpleClient::Delete(const std::string& key) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return std::nullopt;
    }

    DeleteRequest req{key};
    if (!conn->send_request(req)) return std::nullopt;

    std::optional<Response> res = conn->recv_response();
    if (!res) return std::nullopt;
    if (auto* delete_res = std::get_if<DeleteResponse>(&*res)) {
        return delete_res->value;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW,
                   "Failed to Delete value on server: ", error_res->msg);
    }

    return std::nullopt;
}

std::optional<std::vector<std::string>> SimpleClient::MultiGet(
    const std::vector<std::string>& keys) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return std::nullopt;
    }

    MultiGetRequest req{keys};
    if (!conn->send_request(req)) return std::nullopt;

    std::optional<Response> res = conn->recv_response();
    if (!res) return std::nullopt;
    if (auto* multiget_res = std::get_if<MultiGetResponse>(&*res)) {
        return multiget_res->values;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW,
                   "Failed to MultiGet values on server: ", error_res->msg);
    }

    return std::nullopt;
}

bool SimpleClient::MultiPut(const std::vector<std::string>& keys,
                            const std::vector<std::string>& values) {
    std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
    if (!conn) {
        cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
                   '.');
        return false;
    }

    MultiPutRequest req{keys, values};
    if (!conn->send_request(req)) return false;

    std::optional<Response> res = conn->recv_response();
    if (!res) return false;
    if (auto* multiput_res = std::get_if<MultiPutResponse>(&*res)) {
        (void)multiput_res;
        return true;
    } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
        cerr_color(YELLOW,
                   "Failed to MultiPut values on server: ", error_res->msg);
    }

    return false;
}

bool SimpleClient::GDPRDelete(const std::string& user) {
    // TODO: Write your GDPR deletion code here!
    // You can invoke operations directly on the client object, like so:
    //
    // std::string key("user_1_posts");
    // std::optional<std::string> posts = Get(key);
    // ...
    //
    // Assume the `user` argument is a user ID such as "user_1".

    std::string user_posts_key = user + "_posts";
    std::optional<std::string> posts_list_str = Get(user_posts_key);

    if (posts_list_str.has_value() && !posts_list_str->empty()) {
        // 解析以逗号分隔的 post_id 列表
        std::stringstream ss(*posts_list_str);
        std::string post_id;

        while (std::getline(ss, post_id, ',')) {
            // 去除可能的前后空格
            post_id.erase(0, post_id.find_first_not_of(" \t"));
            post_id.erase(post_id.find_last_not_of(" \t") + 1);

            if (post_id.empty()) continue;

            Delete(post_id + "_replies");

            Delete(post_id);
        }
    }

    Delete(user_posts_key);

    Delete(user);

    return true;
}
