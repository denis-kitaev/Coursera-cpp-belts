#include "test_runner.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <map>
#include <optional>
#include <unordered_set>
#include <utility>

using namespace std;


struct HttpRequest {
    string method, path, body;
    map<string, string> get_params;
};

pair<string, string> SplitBy(const string& what, const string& by) {
    size_t pos = what.find(by);
    if (by.size() < what.size() && pos < what.size() - by.size()) {
        return {what.substr(0, pos), what.substr(pos + by.size())};
    } else {
        return {what, {}};
    }
}

template<typename T>
T FromString(const string& s) {
    T x;
    istringstream is(s);
    is >> x;
    return x;
}

pair<size_t, string> ParseIdAndContent(const string& body) {
    auto [id_string, content] = SplitBy(body, " ");
    return {FromString<size_t>(id_string), content};
}

struct LastCommentInfo {
    size_t user_id, consecutive_count;
};

struct HttpHeader {
    string name, value;
};

ostream& operator<<(ostream& output, const HttpHeader& h) {
    return output << h.name << ": " << h.value;
}

bool operator==(const HttpHeader& lhs, const HttpHeader& rhs) {
    return lhs.name == rhs.name && lhs.value == rhs.value;
}

enum class HttpCode {
    Ok = 200,
    NotFound = 404,
    Found = 302,
};

class HttpResponse {
public:
    explicit HttpResponse(HttpCode code) : code_(code) {}

    HttpResponse& AddHeader(string name, string value) {
        headers_.push_back({name, value});
        return *this;
    }
    HttpResponse& SetContent(string content) {
        content_ = move(content);
        return *this;
    }
    HttpResponse& SetCode(HttpCode code) {
        code_ = code;
        return *this;
    }

    friend ostream& operator << (ostream& output, const HttpResponse& resp) {
        output << "HTTP/1.1 " << static_cast<int>(resp.code_)
               << " " << resp.GetVerboseCode() << "\n";
        for (const auto& [name, value] : resp.headers_) {
            output << name << ": " << value << '\n';
        }
        if (!resp.content_.empty()) {
            output << "Content-Length: " << resp.content_.size() << '\n';
        }
        return output << '\n' << resp.content_;
    }

private:
    HttpCode code_;
    vector<HttpHeader> headers_;
    string content_;

    string GetVerboseCode() const {
        switch (code_) {
        case HttpCode::Ok:
            return "OK";
        case HttpCode::Found:
            return "Found";
        case HttpCode::NotFound:
            return "Not found";
        default:
            return {};
        }
    }
};

class CommentServer {
private:
    vector<vector<string>> comments_;
    std::optional<LastCommentInfo> last_comment;
    unordered_set<size_t> banned_users;

    void AddUser(const HttpRequest& req, HttpResponse& resp) {
        comments_.emplace_back();
        auto user_id = to_string(comments_.size() - 1);
        resp.SetCode(HttpCode::Ok).SetContent(user_id);
    }
    void AddComment(const HttpRequest& req, HttpResponse& resp) {
        auto [user_id, comment] = ParseIdAndContent(req.body);

        if (!last_comment || last_comment->user_id != user_id) {
            last_comment = LastCommentInfo{user_id, 1};
        } else if (++last_comment->consecutive_count > 3) {
            banned_users.insert(user_id);
        }

        if (banned_users.count(user_id) == 0) {
            comments_[user_id].push_back(comment);
            resp.SetCode(HttpCode::Ok);
        } else {
            resp.SetCode(HttpCode::Found).AddHeader("Location", "/captcha");
        }
    }
    void CheckCaptcha(const HttpRequest& req, HttpResponse& resp) {
        if (auto [id, response] = ParseIdAndContent(req.body); response == "42") {
            banned_users.erase(id);
            if (last_comment && last_comment->user_id == id) {
                last_comment.reset();
            }
            resp.SetCode(HttpCode::Ok);
        } else {
            resp.SetCode(HttpCode::Found).AddHeader("Location", "/captcha");
        }
    }
    void UserComments(const HttpRequest& req, HttpResponse& resp) {
        auto user_id = FromString<size_t>(req.get_params.at("user_id"));
        string response;
        for (const string& c : comments_[user_id]) {
            response += c + '\n';
        }
        resp.SetCode(HttpCode::Ok).SetContent(response);
    }
    void Captcha(const HttpRequest& req, HttpResponse& resp) {
        resp.SetCode(HttpCode::Ok).SetContent(
            "What's the answer for The Ultimate Question " \
            "of Life, the Universe, and Everything?");
    }
public:
    HttpResponse ServeRequest(const HttpRequest& req) {
        HttpResponse resp(HttpCode::NotFound);
        if (req.method == "POST") {
            if (req.path == "/add_user") {
                AddUser(req, resp);
            } else if (req.path == "/add_comment") {
                AddComment(req, resp);
            } else if (req.path == "/checkcaptcha") {
                CheckCaptcha(req, resp);
            }
        } else if (req.method == "GET") {
            if (req.path == "/user_comments") {
                UserComments(req, resp);
            } else if (req.path == "/captcha") {
                Captcha(req, resp);
            }
        }
        return resp;
    }

    void ServeRequest(const HttpRequest& req, ostream& os) {
        os << ServeRequest(req);
    }
};


struct ParsedResponse {
    int code;
    vector<HttpHeader> headers;
    string content;
};

istream& operator >>(istream& input, ParsedResponse& r) {
    string line;
    getline(input, line);

    {
        istringstream code_input(line);
        string dummy;
        code_input >> dummy >> r.code;
    }

    size_t content_length = 0;

    r.headers.clear();
    while (getline(input, line) && !line.empty()) {
        if (auto [name, value] = SplitBy(line, ": "); name == "Content-Length") {
            istringstream length_input(value);
            length_input >> content_length;
        } else {
            r.headers.push_back( {std::move(name), std::move(value)});
        }
    }

    r.content.resize(content_length);
    input.read(r.content.data(), r.content.size());
    return input;
}

void Test(CommentServer& srv, const HttpRequest& request, const ParsedResponse& expected) {
    stringstream ss;
    srv.ServeRequest(request, ss);
    ParsedResponse resp;
    ss >> resp;
    ASSERT_EQUAL(resp.code, expected.code);
    ASSERT_EQUAL(resp.headers, expected.headers);
    ASSERT_EQUAL(resp.content, expected.content);
}

template <typename CommentServer>
void TestServer() {
    CommentServer cs;

    const ParsedResponse ok{200};
    const ParsedResponse redirect_to_captcha{302, {{"Location", "/captcha"}}, {}};
    const ParsedResponse not_found{404};

    Test(cs, {"POST", "/add_user"}, {200, {}, "0"});
    Test(cs, {"POST", "/add_user"}, {200, {}, "1"});
    Test(cs, {"POST", "/add_comment", "0 Hello"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Hi"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Enlarge"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);
    Test(cs, {"POST", "/add_comment", "0 What are you selling?"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);
    Test(
        cs,
        {"GET", "/user_comments", "", {{"user_id", "0"}}},
        {200, {}, "Hello\nWhat are you selling?\n"}
    );
    Test(
        cs,
        {"GET", "/user_comments", "", {{"user_id", "1"}}},
        {200, {}, "Hi\nBuy my goods\nEnlarge\n"}
    );
    Test(
        cs,
        {"GET", "/captcha"},
        {200, {}, {"What's the answer for The Ultimate Question of Life, the Universe, and Everything?"}}
    );
    Test(cs, {"POST", "/checkcaptcha", "1 24"}, redirect_to_captcha);
    Test(cs, {"POST", "/checkcaptcha", "1 42"}, ok);
    Test(cs, {"POST", "/add_comment", "1 Sorry! No spam any more"}, ok);
    Test(
        cs,
        {"GET", "/user_comments", "", {{"user_id", "1"}}},
        {200, {}, "Hi\nBuy my goods\nEnlarge\nSorry! No spam any more\n"}
    );

    Test(cs, {"GET", "/user_commntes"}, not_found);
    Test(cs, {"POST", "/add_uesr"}, not_found);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestServer<CommentServer>);
}