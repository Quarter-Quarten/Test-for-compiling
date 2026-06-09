#include "tools/syncable/syncable.hpp"

Syncable::~Syncable() {
    for (Syncable* target : sync_to) {
        target->call_deferred("free");
    }
}

void Syncable::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_sync_to", "to"), &Syncable::add_sync_to);
    ClassDB::bind_method(D_METHOD("set_sync_from", "from"), &Syncable::set_sync_from);
    ClassDB::bind_method(D_METHOD("set_thread", "thread"), &Syncable::set_thread);
    ClassDB::bind_method(D_METHOD("add_sync_call", "callable"), &Syncable::add_sync_call);
    ClassDB::bind_method(D_METHOD("update_sync_calls"), &Syncable::update_sync_calls);

    ClassDB::bind_method(D_METHOD("each_sync_to", "callable"), &Syncable::each_sync_to);
}

void Syncable::each_sync_to(const Callable& callable) {
    for (Syncable* target : sync_to) {
        callable.call(Variant(target));
    }
}

void Syncable::add_sync_to(Syncable* to) {
    sync_to.push_back(to);
}
void Syncable::set_sync_from(Syncable* from) {
    from->add_sync_to(this);
}
void Syncable::set_thread(SingleThreadC* thread) {
    thread_ = thread;
}

void Syncable::add_sync_call(Callable callable) {
    std::lock_guard<std::mutex> lock(sync_calls_mutex_);
    sync_calls_.push_back(callable);
    
    if (!is_queueing_update_sync_) {
        SingleThreadC::call_on_thread_deferred(thread_, Callable(this, "update_sync_calls"));
        is_queueing_update_sync_ = true;
    }
}

void Syncable::update_sync_calls() {
    is_queueing_update_sync_ = false;
    
    std::unique_lock<std::mutex> lock(sync_calls_mutex_);
    std::swap(sync_calls_buffer_, sync_calls_);
    sync_calls_.clear();
    lock.unlock();
    
    for (const Callable& callable : sync_calls_buffer_) {
        if (callable.is_valid()) {
            callable.call();
        }
    }
    sync_calls_buffer_.clear();
}