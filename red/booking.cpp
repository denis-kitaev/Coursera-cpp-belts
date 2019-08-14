#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;


struct Booking {
    int time;
    int client_id;
    int room_count;
};

bool operator < (const Booking& lhs, const Booking& rhs) {
    return lhs.time < rhs.time;
}

class BookingManager {
public:
    void Book(int time, const string& hotel_name, int client_id, int room_count) {
        Booking b{time, client_id, room_count};
        hotels_[hotel_name].push_back(b);
        current_time_ = time;
    }
    size_t GetClients(const string& hotel_name) {
        if (hotels_.count(hotel_name) == 0) {
            return 0;
        }
        Booking fake{current_time_ - 86400, 0, 0};
        auto& bookings = hotels_.at(hotel_name);
        auto it = upper_bound(bookings.begin(), bookings.end(), fake, 
                    [](const Booking& lhs, const Booking& rhs){
                        return lhs.time < rhs.time;
                    });
        
        set<int> client_ids;
        while (it != bookings.end()) {
            client_ids.insert(it->client_id);
            ++it;
        }

        return client_ids.size();
    }
    size_t GetRooms(const string& hotel_name) {
        if (hotels_.count(hotel_name) == 0) {
            return 0;
        }
        Booking fake{current_time_ - 86400, 0, 0};
        auto& bookings = hotels_.at(hotel_name);
        auto it = upper_bound(bookings.begin(), bookings.end(), fake, 
                    [](const Booking& lhs, const Booking& rhs){
                        return lhs.time < rhs.time;
                    });
        
        size_t total_rooms = 0;
        while (it != bookings.end()) {
            total_rooms += it->room_count;
            ++it;
        }

        return total_rooms;
    }

private:
    map<string, vector<Booking>> hotels_;
    int current_time_;

private:
    void RemoveOld(const string& hotel_name) {
        Booking fake{current_time_ - 86400, 0, 0};
        auto& bookings = hotels_.at(hotel_name);
        auto it = upper_bound(bookings.begin(), bookings.end(), fake, 
                    [](const Booking& lhs, const Booking& rhs){
                        return lhs.time < rhs.time;
                    });
        hotels_[hotel_name] = {it, bookings.end()};
    }
};


int main() {
    BookingManager manager;

    int q;
    cin >> q;
    for (int i = 0; i < q; ++i) {
        string cmd, hotel_name;
        cin >> cmd;
        if (cmd == "BOOK") {
            int time, client_id, room_count;
            cin >> time >> hotel_name >> client_id >> room_count;
            manager.Book(time, hotel_name, client_id, room_count);
        } else if (cmd == "CLIENTS") {
            cin >> hotel_name;
            cout << manager.GetClients(hotel_name) << '\n';
        } else if (cmd == "ROOMS") {
            cin >> hotel_name;
            cout << manager.GetRooms(hotel_name) << '\n';
        }
    }
    return 0;
}
