#pragma once
#include <kernel/cpp/type-traits.hpp>
#include <kernel/cpp/utility.hpp>
#include <kernel/cpp/allocator.hpp>

template <typename T, Allocator Alloc = Mallocator>
class UniquePtr {
public:
    UniquePtr() = default;
    UniquePtr(T* ptr, Alloc& alloc): allocator(alloc) {
        data = Blk{ .ptr = (void*)ptr, .size = sizeof(T) };
    }
    explicit UniquePtr(T* ptr) {
        data = Blk{ .ptr = (void*)ptr, .size = sizeof(T) };
        allocator = Alloc{};
    }

    ~UniquePtr() { reset(); }

    // Move operations
    UniquePtr(UniquePtr&& rhs) { data.ptr = rhs.release(); }
    UniquePtr& operator=(UniquePtr&& rhs) {
        reset(rhs.release());
        return *this;
    }

    // Delete copy ones
    UniquePtr(const UniquePtr& lhs) = delete;
    UniquePtr& operator=(const UniquePtr& lhs) = delete;

    T* release() noexcept {
        auto oldPtr = (T*)data.ptr;
        data.ptr = nullptr;
        return oldPtr;
    }

    void reset(T* newPtr = nullptr) noexcept {
        auto oldPtr = (T*)data.ptr;
        data.ptr = newPtr;
        if (oldPtr) {
            oldPtr->~T();
            allocator.deallocate(Blk{ oldPtr, sizeof(T) });
        }
    }

    // Operator overloads
    explicit operator bool() const noexcept { return data.ptr != nullptr; }
    const T* operator->() const noexcept { return get(); }
    const T& operator*() const noexcept {
        assert(data.ptr != nullptr);
        return *get();
    }

    T* operator->()  noexcept { return get(); }
    T& operator*()  noexcept {
        assert(data.ptr != nullptr);
        return *get();
    }

    T* get() const noexcept { return (T*)data.ptr; }

private:
    Blk data;
    Alloc allocator;
};

template <typename T, Allocator Alloc>
class UniquePtr<T[], Alloc> {
public:
    UniquePtr() = default;
    UniquePtr(Blk blk, Alloc& alloc): data(blk), allocator(alloc) {}
    explicit UniquePtr(Blk blk): data(blk), allocator(Alloc{}) {}

    ~UniquePtr() { reset(); }

    // Move operations
    UniquePtr(UniquePtr&& rhs): data(rhs.release()) {}
    UniquePtr& operator=(UniquePtr&& rhs) {
        reset(rhs.release());
        return *this;
    }

    Blk release() noexcept {
        auto oldData = data;
        data = Blk{};
        return oldData;
    }

    void reset(Blk newData = Blk{}) noexcept {
        auto oldData = data;
        data = newData;
        if (oldData.ptr) {
            int len = oldData.size / sizeof(T);
            for (int i = len - 1; i >= 0; i--) {
                ((T*)oldData.ptr)->~T();
            }
            allocator.deallocate(oldData);
        }
    }

    // Operator overloads
    explicit operator bool() const noexcept { return data.ptr != nullptr; }
    T* operator->() const noexcept { return get(); }
    T& operator*() const noexcept {
        assert(data.ptr != nullptr);
        return *get();
    }
    T& operator[](size_t index) { return get()[index]; }
    const T& operator[](size_t index) const { return get()[index]; }

    T* get() const noexcept { return (T*)data.ptr; }

private:
    Blk data;
    Alloc allocator;
};

template <typename T, typename... Args, Allocator Alloc = Mallocator>
    requires(!is_array_v<T>)
UniquePtr<T> makeUnique(Args&&... args) {
    Alloc allocator{};
    auto blk = allocator.allocate(sizeof(T));
    return UniquePtr<T>(new T(forward<Args>(args)...), allocator);
}

template <typename T, typename... Args, Allocator Alloc>
    requires(!is_array_v<T>)
UniquePtr<T> makeUnique(Alloc allocator, Args&&... args) {
    auto blk = allocator.allocate(sizeof(T));
    return UniquePtr<T>(new T(forward<Args>(args)...), allocator);
}

template <typename T, Allocator Alloc = Mallocator>
    requires(is_array_v<T>)
UniquePtr<T> makeUnique(size_t num) {
    Alloc allocator{};
    using baseT = remove_extent_t<T>;
    Blk blk = allocator.allocate(sizeof(baseT) * num);
    for (int i = 0; i < num; i++) {
        baseT* ptr = ((baseT*)blk.ptr) + i;
        new (ptr) baseT();
    }
    return UniquePtr<T>(blk, allocator);
}

template <typename T, Allocator Alloc = Mallocator>
    requires(is_array_v<T>)
UniquePtr<T> makeUnique(Alloc allocator, size_t num) {
    using baseT = remove_extent_t<T>;
    Blk blk = allocator.allocate(sizeof(baseT) * num);
    for (int i = 0; i < num; i++) {
        baseT* ptr = ((baseT*)blk.ptr) + i;
        new (ptr) baseT();
    }
    return UniquePtr<T>(blk, allocator);
}

