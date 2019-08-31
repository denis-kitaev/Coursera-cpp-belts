#include <memory>
#include <iostream>


using namespace std;


struct Actor {
    Actor() {
        cout << "Ctor" << endl;
    }
    ~Actor() {
        cout << "Dtor" << endl;
    }
};


int main () {
    auto ptr = new Actor;
    auto s1 = shared_ptr<Actor>(ptr);
    auto s2 = shared_ptr<Actor>(ptr);
    return 0;
}