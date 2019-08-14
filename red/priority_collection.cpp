#include "test_runner.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <map>
#include <list>
#include <utility>
#include <vector>
#include <deque>

using namespace std;


struct Node {
    size_t id;
    int priority = 0;
};

bool operator < (const Node& lhs, const Node& rhs) {
    if (lhs.priority < rhs.priority) {
        return true;
    } else if (lhs.priority > rhs.priority) {
        return false;
    } else {
        return lhs.id < rhs.id;
    }
};


template <typename T>
class PriorityCollection {
public:
    using Id = size_t;

    struct Ref {
        T value;
        typename set<Node>::iterator set_it;
    };

    PriorityCollection() {
        refs_.reserve(1'000'000);
    }

    Id Add(T object) {
        Id id = refs_.size();
        auto p = sorted_.insert({id});
        refs_.push_back({move(object), p.first});
        return id;
    }

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
             IdOutputIt ids_begin)
    {
        auto output_it = ids_begin;
        for (auto it = range_begin; it != range_end; ++it) {
            *output_it = Add(move(*it));
            ++output_it;
        }
    }

    bool IsValid(Id id) const {
        return id < refs_.size() && refs_[id].set_it != sorted_.end();
    }

    const T& Get(Id id) const {
        return refs_[id].value;
    }

    void Promote(Id id) {
        auto& ref = refs_[id];
        int priority = ref.set_it->priority;
        sorted_.erase(ref.set_it);
        auto p = sorted_.insert({id, ++priority});
        ref.set_it = p.first;
    }

    pair<const T&, int> GetMax() const {
        auto it = prev(sorted_.end());
        return {refs_[it->id].value, it->priority};
    }

    pair<T, int> PopMax() {
        auto it = prev(sorted_.end());
        auto p = make_pair(move(refs_[it->id].value), it->priority);
        sorted_.erase(it);
        refs_[it->id] = {T(), sorted_.end()};
        return p;
    }

private:
    vector<Ref> refs_;
    set<Node> sorted_;
};


class StringNonCopyable : public string {
public:
    using string::string;
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }
    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}


struct S {
    string str;
    int p = 0;
};

bool operator < (const S& lhs, const S& rhs) {
    return lhs.p < rhs.p;
};

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);
}
