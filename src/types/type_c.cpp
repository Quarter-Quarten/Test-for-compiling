#include "type_c.hpp"

TypeC::TypeC() {
}

void TypeC::init(const Callable &p_method) {
    if (p_method.is_valid()) {
        p_method.call(this);
    }
}

void TypeC::after_init_c() {
}

void TypeC::draw_introduction_c(Object *bar) {
    if (!bar) return;
    bar->call("draw_text", _name, bar->get("name_size"));
    bar->call("draw_text", _description, bar->get("d_size"));
    bar->call("y_space");
}

void TypeC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_index", "index"), &TypeC::set_index);
    ClassDB::bind_method(D_METHOD("get_index"), &TypeC::get_index);
    ClassDB::add_property("TypeC", PropertyInfo(Variant::INT, "index"), "set_index", "get_index");

    ClassDB::bind_method(D_METHOD("set_name", "name"), &TypeC::set_name);
    ClassDB::bind_method(D_METHOD("get_name"), &TypeC::get_name);
    ClassDB::add_property("TypeC", PropertyInfo(Variant::STRING_NAME, "name"), "set_name", "get_name");

    ClassDB::bind_method(D_METHOD("set_description", "description"), &TypeC::set_description);
    ClassDB::bind_method(D_METHOD("get_description"), &TypeC::get_description);
    ClassDB::add_property("TypeC", PropertyInfo(Variant::STRING, "description"), "set_description", "get_description");

    ClassDB::bind_method(D_METHOD("init", "method"), &TypeC::init);
    ClassDB::bind_method(D_METHOD("after_init_c"), &TypeC::after_init_c);
    // ClassDB::add_virtual_method(get_class_static(), MethodInfo("_after_init"), Vector<StringName>());

    ClassDB::bind_method(D_METHOD("draw_introduction_c", "bar"), &TypeC::draw_introduction_c);
    // ClassDB::add_virtual_method(get_class_static(), MethodInfo("_draw_introduction", PropertyInfo(Variant::OBJECT, "bar")), ::godot::snarray("bar"));
}
