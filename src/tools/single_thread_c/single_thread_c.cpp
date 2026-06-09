#include "tools/single_thread_c/single_thread_c.hpp"

#include "tools/call/call.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

std::vector<Ref<WeakRef>> SingleThreadC::instances;

void SingleThreadC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "name", "skip_frame", "handler", "init_function", "thread_func"), &SingleThreadC::init, DEFVAL(Callable()), DEFVAL(Callable()));

    ClassDB::bind_static_method("SingleThreadC", D_METHOD("wait_for_all"), &SingleThreadC::wait_for_all);
    ClassDB::bind_static_method("SingleThreadC", D_METHOD("get_thread_count"), &SingleThreadC::get_thread_count);
    ClassDB::bind_static_method("SingleThreadC", D_METHOD("call_on_thread_deferred", "thread", "callable"), &SingleThreadC::call_on_thread_deferred);

    ClassDB::bind_method(D_METHOD("set_cycle_start", "method"), &SingleThreadC::set_cycle_start);
    ClassDB::bind_method(D_METHOD("set_finished", "finished"), &SingleThreadC::set_finished);
    ClassDB::bind_method(D_METHOD("stop_print_skipping"), &SingleThreadC::stop_print_skipping);

    ClassDB::bind_method(D_METHOD("is_exiting"), &SingleThreadC::is_exiting);
    ClassDB::bind_method(D_METHOD("is_workng"), &SingleThreadC::is_workng);
    ClassDB::bind_method(D_METHOD("get_process_time"), &SingleThreadC::get_process_time);
    ClassDB::bind_method(D_METHOD("get_duration_from_submitting"), &SingleThreadC::get_duration_from_submitting);
    ClassDB::bind_method(D_METHOD("get_process_time_message"), &SingleThreadC::get_process_time_message);
    ClassDB::bind_method(D_METHOD("get_rander"), &SingleThreadC::get_rander);

    ClassDB::bind_method(D_METHOD("free_sources"), &SingleThreadC::free_sources);
    ClassDB::bind_method(D_METHOD("update"), &SingleThreadC::update);
    ClassDB::bind_method(D_METHOD("try_start"), &SingleThreadC::try_start);
}

SingleThreadC::SingleThreadC() = default;

SingleThreadC::~SingleThreadC() {
    if (running.load(std::memory_order_acquire)) {
        free_sources();
    }
}

bool SingleThreadC::world_is_ready() {
    Node2D *vars = Call::get_vars();
    if (!vars) {
        return false;
    }

    return bool(vars->call("world_is_ready"));
}

void SingleThreadC::init(const String &p_name, bool p_skip_frame, const Callable &p_handler, const Callable &p_init_function, const Callable &p_thread_func) {
    name = p_name;
    skip_frame = p_skip_frame;
    handler = p_handler;
    init_function = p_init_function;

    if (p_thread_func.is_valid()) {
        UtilityFunctions::push_warning("SingleThreadC: custom thread_func is ignored in persistent thread mode");
    }

    instances.push_back(Ref<WeakRef>(UtilityFunctions::weakref(this)));

    UtilityFunctions::print("当前线程数：", get_thread_count());

    running.store(true, std::memory_order_release);

    RenderingServer *rs = RenderingServer::get_singleton();
    rs->connect("frame_pre_draw", Callable(this, "try_start"));
    frame_connected = true;

    worker_thread = std::thread(&SingleThreadC::thread_loop, this);
}

void SingleThreadC::thread_loop() {
    while (running.load(std::memory_order_acquire)) {
        {
            std::unique_lock<std::mutex> lock(wake_mutex);
            wake_cv.wait(lock, [this] {
                return wake_requested || !running.load(std::memory_order_acquire);
            });

            if (!running.load(std::memory_order_acquire)) {
                break;
            }

            wake_requested = false;
        }

        if (!init_done && init_function.is_valid()) {
            init_function.call();
            init_done = true;
        }

        const int64_t time = Time::get_singleton()->get_ticks_usec();

        handler.call();
        handle_call_deferreds();

        process_time = Time::get_singleton()->get_ticks_usec() - time;

        if (finished.is_valid()) {
            finished.call_deferred();
        }

        working.store(false, std::memory_order_release);
    }
}

void SingleThreadC::try_start() {
    if (!world_is_ready()) {
        return;
    }

    last_try_start_interval = Time::get_singleton()->get_ticks_usec() - submit_time;

    bool expected = false;
    if (working.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        submit_time = Time::get_singleton()->get_ticks_usec();

        if (cycle_start.is_valid()) {
            cycle_start.call();
        }

        {
            std::lock_guard<std::mutex> lock(wake_mutex);
            wake_requested = true;
        }
        wake_cv.notify_one();
    } else {
        print_skipping();
    }
}

void SingleThreadC::handle_call_deferreds() {
    {
        std::lock_guard<std::mutex> lock(deferred_mutex);
        call_deferreds_buffer.swap(call_deferreds);
    }

    for (const Callable &c : call_deferreds_buffer) {
        c.call();
    }
    call_deferreds_buffer.clear();
}

void SingleThreadC::print_skipping() {
    ERR_FAIL_COND(name.is_empty());

    if (!should_print_skipping) {
        return;
    }

    const double fps = Engine::get_singleton()->get_frames_per_second();
    const double frame_ms = fps > 0.0 ? 1.0 / fps * 1000.0 : 0.0;

    (
            name, " Skipping ",
            process_time * 0.001, "/",
            get_duration_from_submitting() * 0.001, "/",
            last_try_start_interval * 0.001, "/",
            frame_ms);
}

void SingleThreadC::wait_for_all() {
}

int SingleThreadC::get_thread_count() {
    int cnt = 0;
    for (const Ref<WeakRef> &wref : instances) {
        if (wref.is_valid() && wref->get_ref().get_type() != Variant::NIL) {
            cnt += 1;
        }
    }
    return cnt;
}

void SingleThreadC::call_on_thread_deferred(SingleThreadC *thread, const Callable &callable) {
    if (thread) {
        std::lock_guard<std::mutex> lock(thread->deferred_mutex);
        thread->call_deferreds.push_back(callable);
    } else {
        callable.call_deferred();
    }
}

Ref<SingleThreadC> SingleThreadC::set_cycle_start(const Callable &method) {
    cycle_start = method;
    return Ref<SingleThreadC>(this);
}

Ref<SingleThreadC> SingleThreadC::set_finished(const Callable &p_finished) {
    finished = p_finished;
    return Ref<SingleThreadC>(this);
}

Ref<SingleThreadC> SingleThreadC::stop_print_skipping() {
    should_print_skipping = false;
    return Ref<SingleThreadC>(this);
}

bool SingleThreadC::is_exiting() const {
    return !running.load(std::memory_order_acquire);
}

bool SingleThreadC::is_workng() const {
    return working.load(std::memory_order_acquire);
}

int64_t SingleThreadC::get_process_time() const {
    return process_time;
}

int64_t SingleThreadC::get_duration_from_submitting() const {
    return Time::get_singleton()->get_ticks_usec() - submit_time;
}

String SingleThreadC::get_process_time_message() const {
    return name + String::num(process_time * 0.001) + "ms";
}

Ref<RandomNumberGenerator> SingleThreadC::get_rander() {
    if (!rander.is_valid()) {
        rander.instantiate();
    }
    return rander;
}

void SingleThreadC::free_sources() {
    running.store(false, std::memory_order_release);

    {
        std::lock_guard<std::mutex> lock(wake_mutex);
        wake_requested = true;
    }
    wake_cv.notify_one();

    if (worker_thread.joinable()) {
        worker_thread.join();
    }

    for (int i = static_cast<int>(instances.size()) - 1; i >= 0; i--) {
        Object *inst = instances[i].is_valid() ? Object::cast_to<Object>(instances[i]->get_ref()) : nullptr;
        if (inst == this || inst == nullptr) {
            instances.erase(instances.begin() + i);
        }
    }

    if (frame_connected) {
        RenderingServer::get_singleton()->disconnect("frame_pre_draw", Callable(this, "try_start"));
        frame_connected = false;
    }
}

void SingleThreadC::update() {
}
