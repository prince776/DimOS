#pragma once
#include <kernel/cpp/type-traits.hpp>
#include <kernel/cpp/utility.hpp>

template <typename T>
class UniquePtr {
public:
    UniquePtr() = default;
    explicit UniquePtr(T* ptr): ptr(ptr) {}

    ~UniquePtr() { reset(); }

    // Move operations
    UniquePtr(UniquePtr&& rhs): ptr(rhs.release()) {}
    UniquePtr& operator=(UniquePtr&& rhs) {
        reset(rhs.release());
        return *this;
    }

    T* release() noexcept {
        auto oldPtr = ptr;
        ptr = nullptr;
        return oldPtr;
    }

    void reset(T* newPtr = nullptr) noexcept {
        auto oldPtr = ptr;
        ptr = newPtr;
        if (oldPtr) {
            delete oldPtr;
        }
    }

    // Operator overloads
    explicit operator bool() const noexcept { return ptr != nullptr; }
    T* operator->() const noexcept { return get(); }
    T& operator*() const noexcept {
        assert(ptr != nullptr);
        return *get();
    }

    T* get() const noexcept { return ptr; }

private:
    T* ptr = nullptr;
};

template <typename T>
class UniquePtr<T[]> {
public:
    UniquePtr() = default;
    explicit UniquePtr(T* ptr): ptr(ptr) {}

    ~UniquePtr() { reset(); }

    // Move operations
    UniquePtr(UniquePtr&& rhs): ptr(rhs.release()) {}
    UniquePtr& operator=(UniquePtr&& rhs) {
        reset(rhs.release());
        return *this;
    }

    T* release() noexcept {
        auto oldPtr = ptr;
        ptr = nullptr;
        return oldPtr;
    }

    void reset(T* newPtr = nullptr) noexcept {
        auto oldPtr = ptr;
        ptr = newPtr;
        if (oldPtr) {
            delete[] oldPtr;
        }
    }

    // Operator overloads
    explicit operator bool() const noexcept { return ptr != nullptr; }
    T* operator->() const noexcept { return get(); }
    T& operator*() const noexcept {
        assert(ptr != nullptr);
        return *get();
    }
    T& operator[](size_t index) { return ptr[index]; }
    const T& operator[](size_t index) const { return ptr[index]; }

    T* get() const noexcept { return ptr; }

private:
    T* ptr = nullptr;
};

template <typename T, typename... Args>
    requires(!is_array_v<T>)
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
    requires(is_array_v<T>)
UniquePtr<T> makeUnique(size_t num) {
    return UniquePtr<T>(new remove_extent_t<T>[num]());
}
