#pragma once

#include <vector>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <mutex>
#include <tools/single_thread_c/single_thread_c.hpp>

using namespace godot;

// 跨线程同步支持 每个线程创建一个实例并由一个线程同步到其它线程
class Syncable : public Object {
    GDCLASS(Syncable, Object);
public:
    Syncable() = default;
    ~Syncable();

    void each_sync_to(const Callable& callable);
protected:
    // 初始化
    void add_sync_to(Syncable* to);
    void set_sync_from(Syncable* from);
    void set_thread(SingleThreadC* thread);

    // 同步调用
    void add_sync_call(Callable callable);
    void update_sync_calls();
    
    static void _bind_methods();

private:
    std::vector<Syncable*> sync_to;

    std::vector<Callable> sync_calls_;
    std::vector<Callable> sync_calls_buffer_;

    std::mutex sync_calls_mutex_;

    bool is_queueing_update_sync_ = false;
    SingleThreadC* thread_;
};

