#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>

using namespace godot;

class TypeC : public RefCounted {
    GDCLASS(TypeC, RefCounted);

private:
    int64_t _index = -1;
    StringName _name;
    String _description = String("介绍");

protected:
    static void _bind_methods();

public:
    TypeC();

    void set_index(int64_t p_index) { _index = p_index; }
    int64_t get_index() const { return _index; }

    void set_name(const StringName &p_name) { _name = p_name; }
    StringName get_name() const { return _name; }

    void set_description(const String &p_description) { _description = p_description; }
    String get_description() const { return _description; }

    void init(const Callable &p_method);
    virtual void after_init_c();
    virtual void draw_introduction_c(Object *bar);
};
