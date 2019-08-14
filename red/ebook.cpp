#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <set>

using namespace std;


class ReadingManager {
public:
    void Read(int user_id, int page_count) {
        if (user_to_page_.count(user_id) != 0) {
            int last_page_count = user_to_page_[user_id];
            if (page_to_users_[last_page_count].size() == 1) {
                page_to_users_.erase(last_page_count);
            } else {
                page_to_users_[last_page_count].erase(user_id);
            }
        }
        user_to_page_[user_id] = page_count;
        page_to_users_[page_count].insert(user_id);
    }

    double Cheer(int user_id) const {
        if (user_to_page_.count(user_id) == 0) {
            return 0;
        }
        if (user_to_page_.size() == 1) {
            return 1;
        }
        int page_count = user_to_page_.at(user_id);
        int count = 0;
        for (const auto& item : page_to_users_) {
            if (item.first >= page_count) {
                break;
            }
            count += item.second.size();
        }

        return count * 1.0 / (user_to_page_.size() - 1);
    }

private:
    map<int, int> user_to_page_;
    map<int, set<int>> page_to_users_;
};


int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  ReadingManager manager;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int user_id;
    cin >> user_id;

    if (query_type == "READ") {
      int page_count;
      cin >> page_count;
      manager.Read(user_id, page_count);
    } else if (query_type == "CHEER") {
      cout << setprecision(6) << manager.Cheer(user_id) << "\n";
    }
  }

  return 0;
}
