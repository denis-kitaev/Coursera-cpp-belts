#include <cstdint>
#include <algorithm>

using namespace std;


template <typename T>
class SimpleVector {
public:
    SimpleVector() = default;
    explicit SimpleVector(size_t size) {
        capacity_ = size_ = size;
        data_ = new T[capacity_];
    }
    ~SimpleVector() {
        delete[] data_;
    }

    T& operator[](size_t index) {
        return data_[index];
    }

    T* begin() {
        return data_;
    }
    T* end() {
        return data_ + size_;
    }

    size_t Size() const {
        return size_;
    }
    size_t Capacity() const {
        return capacity_;
    }
    void PushBack(const T& value) {
        ReAllocate();
        data_[size_++] = value;
    }
    void PushBack(T&& value) {
        ReAllocate();
        data_[size_++] = move(value);
    }

private:
    T* data_ = nullptr;
    size_t size_ = 0, capacity_ = 0;

    void ReAllocate() {
        if (size_ == capacity_) {
            capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            T* new_data = new T[capacity_];
            move(begin(), end(), new_data);

            delete[] data_;
            data_ = new_data;
        }
    }
};
