#pragma once

#include <stack>
#include <mutex>
#include <godot_cpp/classes/ref.hpp>

using namespace godot;

// T is the type of id
template <typename T>
class IDPool {
    std::mutex mtx_;
    std::stack<T> pool_;
    T next_id_ = 0;
public:
    T acquire() {
        std::lock_guard lock(mtx_);
        if (pool_.empty()) return next_id_++;
        T id = pool_.top();
        pool_.pop();
        return id;
    }
    void release(T id) {
        std::lock_guard lock(mtx_);
        pool_.push(id);
    }
    void clear() {
        std::lock_guard lock(mtx_);
        pool_ = {};
    }
};

// for godot
#define IDPOOL(T) \
class IDPool##T : public RefCounted { \
    GDCLASS(IDPool##T, RefCounted) \
    std::mutex mtx_; \
    std::stack<T> pool_; \
    T next_id_ = 0; \
public: \
    IDPool##T() = default; \
    ~IDPool##T() = default; \
    T acquire() { \
        std::lock_guard lock(mtx_); \
        if (pool_.empty()) return next_id_++; \
        T id = pool_.top(); \
        pool_.pop(); \
        return id; \
    } \
    void release(T id) { \
        std::lock_guard lock(mtx_); \
        pool_.push(id); \
    } \
    void clear() { \
        std::lock_guard lock(mtx_); \
        pool_ = {}; \
    } \
protected: \
    static void _bind_methods() { \
        ClassDB::bind_method(D_METHOD("acquire"), &IDPool##T::acquire); \
        ClassDB::bind_method(D_METHOD("release", "id"), &IDPool##T::release); \
        ClassDB::bind_method(D_METHOD("clear"), &IDPool##T::clear); \
    } \
};

IDPOOL(uint8_t)
IDPOOL(uint16_t)
IDPOOL(uint32_t)