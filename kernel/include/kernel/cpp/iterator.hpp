#pragma once

template <typename T> class ForwardIterator {
  public:
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;

    ForwardIterator(T* ptr) : ptr(ptr) {}

    reference operator*() { return *ptr; }
    const_reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    ForwardIterator& operator++() {
        ptr++;
        return *this;
    }
    ForwardIterator operator++(int) {
        ForwardIterator temp{ptr};
        operator++();
        return temp;
    }
    friend bool operator==(const ForwardIterator& lhs, const ForwardIterator& rhs) {
        return lhs.ptr == rhs.ptr;
    }
    friend bool operator!=(const ForwardIterator& lhs, const ForwardIterator& rhs) {
        return lhs.ptr != rhs.ptr;
    }

  private:
    T* ptr;
};
