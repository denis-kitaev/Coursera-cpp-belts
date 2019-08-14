#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <tuple>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};


class Database {
public:
    bool Put(const Record& record) {
        if (data_.count(record.id) > 0) {
            return false;
        }
        data_[record.id] = record;
        timestamp_index_.insert({record.timestamp, record.id});
        karma_index_.insert({record.karma, record.id});
        user_index_.insert({record.user, record.id});
        return true;
    }
    const Record* GetById(const string& id) const {
        auto it = data_.find(id);
        if (it == data_.end()) {
            return nullptr;
        }
        return &it->second;
    }
    bool Erase(const string& id) {
        auto it = data_.find(id);
        if (it == data_.end()) {
            return false;
        }
        
        auto& record = it->second;
        RemoveFromIndex(timestamp_index_, {record.timestamp, id});
        RemoveFromIndex(karma_index_, {record.karma, id});
        RemoveFromIndex(user_index_, {record.user, id});
        data_.erase(it);
        return true;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto range = make_pair(low, high);
        RangeByIndex(timestamp_index_, range, callback);
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto range = make_pair(low, high);
        RangeByIndex(karma_index_, range, callback);
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto range = make_pair(user, user);
        RangeByIndex(user_index_, range, callback);
    }

private:
    unordered_map<string, Record> data_;
    multimap<int, string> timestamp_index_;
    multimap<int, string> karma_index_;
    multimap<string, string> user_index_;

private:
    template<typename Key>
    void RemoveFromIndex(multimap<Key, string>& index, 
                         const pair<Key, string>& p) 
    {
        auto it = index.lower_bound(p.first);
        while (it != index.end()) {
            if (it->second == p.second) {
                index.erase(it);
            }
            it++;
        }
    };

    template<typename Key, typename Callback>
    void RangeByIndex(const multimap<Key, string>& index, 
                      const pair<Key, Key>& range, 
                      Callback& callback) const 
    {
        auto start_it = index.lower_bound(range.first);
        auto end_it = index.upper_bound(range.second);
        for (auto it = start_it; it != end_it; ++it) {
            auto dit = data_.find(it->second);
            if (dit != data_.end()) {
                if (!callback(dit->second)) {
                    break;
                }
            } else {
                continue;
            }
            
        }
    };
};


void TestRangeBoundaries() {
  const int good_karma = 1000;
  const int bad_karma = -10;

  Database db;
  db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
  db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

  int count = 0;
  db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestSameUser() {
  Database db;
  db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
  db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

  int count = 0;
  db.AllByUser("master", [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestReplacement() {
  const string final_body = "Feeling sad";

  Database db;
  db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
  db.Erase("id");
  db.Put({"id", final_body, "not-master", 1536107260, -10});

  auto record = db.GetById("id");
  ASSERT(record != nullptr);
  ASSERT_EQUAL(final_body, record->title);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestRangeBoundaries);
  RUN_TEST(tr, TestSameUser);
  RUN_TEST(tr, TestReplacement);
  return 0;
}
