#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/weak_ref.hpp>

namespace godot {

class SingleThreadC : public RefCounted {
    GDCLASS(SingleThreadC, RefCounted);

private:
    static std::vector<Ref<WeakRef>> instances;

    String name;
    bool skip_frame = false;
    bool should_print_skipping = true;
    bool init_done = false;
    bool frame_connected = false;

    Callable handler;
    Callable init_function;
    Callable cycle_start;
    Callable finished;

    std::atomic<bool> working{false};
    std::atomic<bool> running{false};

    int64_t submit_time = 0;
    int64_t last_try_start_interval = 0;
    int64_t process_time = 0;

    Ref<RandomNumberGenerator> rander;

    std::vector<Callable> call_deferreds;
    std::vector<Callable> call_deferreds_buffer;
    std::mutex deferred_mutex;

    std::thread worker_thread;
    std::mutex wake_mutex;
    std::condition_variable wake_cv;
    bool wake_requested = false;

    static void _bind_methods();
    static bool world_is_ready();

    void thread_loop();
    void try_start();
    void handle_call_deferreds();
    void print_skipping();

public:
    SingleThreadC();
    ~SingleThreadC() override;

    void init(const String &p_name, bool p_skip_frame, const Callable &p_handler, const Callable &p_init_function = Callable(), const Callable &p_thread_func = Callable());

    static void wait_for_all();
    static int get_thread_count();
    static void call_on_thread_deferred(SingleThreadC *thread, const Callable &callable);

    Ref<SingleThreadC> set_cycle_start(const Callable &method);
    Ref<SingleThreadC> set_finished(const Callable &p_finished);
    Ref<SingleThreadC> stop_print_skipping();

    bool is_exiting() const;
    bool is_workng() const;
    int64_t get_process_time() const;
    int64_t get_duration_from_submitting() const;
    String get_process_time_message() const;
    Ref<RandomNumberGenerator> get_rander();

    void free_sources();
    void update();
};

} // namespace godot
