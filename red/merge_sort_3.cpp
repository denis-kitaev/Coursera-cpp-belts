#include "test_runner.h"

#include <iterator>
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    if (range_end - range_begin < 2) {
        return;
    }
    vector<typename RandomIt::value_type> v(
        make_move_iterator(range_begin), 
        make_move_iterator(range_end)
    );

    size_t chunk_size = v.size() / 3;

    pair<RandomIt, RandomIt> chunk1{v.begin(), v.begin() + chunk_size}, 
                             chunk2{v.begin() + chunk_size, v.begin() + 2 * chunk_size}, 
                             chunk3{v.begin() + 2 * chunk_size, v.end()};
    
    MergeSort(chunk1.first, chunk1.second);
    MergeSort(chunk2.first, chunk2.second);
    MergeSort(chunk3.first, chunk3.second);

    vector<typename RandomIt::value_type> tmp;
    tmp.reserve(chunk_size * 2);

    merge(
        make_move_iterator(chunk1.first), 
        make_move_iterator(chunk1.second), 
        make_move_iterator(chunk2.first), 
        make_move_iterator(chunk2.second),
        back_inserter(tmp)
    );

    merge(
        make_move_iterator(tmp.begin()), 
        make_move_iterator(tmp.end()),
        make_move_iterator(chunk3.first), 
        make_move_iterator(chunk3.second), 
        range_begin
    );
}

void TestIntVector() {
  vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
  MergeSort(begin(numbers), end(numbers));
  ASSERT(is_sorted(begin(numbers), end(numbers)));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestIntVector);
  return 0;
}
