#include "my_multi_mesh_c.hpp"
#include "tools/call/call.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


using namespace godot;

const Transform2D MyMultiMeshC::INVALID_TRANSFORM = Transform2D(0.0, Vector2(0.0, 0.0), 0.0, Vector2(0.0, 0.0));

void MyMultiMeshC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "p_mesh", "instance_max", "use_colors", "use_custom_data", "auto_free"), &MyMultiMeshC::init, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("recreate"), &MyMultiMeshC::recreate);
    ClassDB::bind_method(D_METHOD("_recreate_deferred"), &MyMultiMeshC::_recreate_deferred);
    ClassDB::bind_method(D_METHOD("set_material", "mate"), &MyMultiMeshC::set_material);
    ClassDB::bind_method(D_METHOD("set_z", "z"), &MyMultiMeshC::set_z);
    ClassDB::bind_method(D_METHOD("set_aabb", "aabb"), &MyMultiMeshC::set_aabb);

    ClassDB::bind_method(D_METHOD("set_now_instance_transform", "trans"), &MyMultiMeshC::set_now_instance_transform);
    ClassDB::bind_method(D_METHOD("set_now_instance_color", "col"), &MyMultiMeshC::set_now_instance_color);
    ClassDB::bind_method(D_METHOD("set_now_instance_custom", "custom"), &MyMultiMeshC::set_now_instance_custom);

    ClassDB::bind_method(D_METHOD("set_instance_transform", "trans", "id"), &MyMultiMeshC::set_instance_transform);
    ClassDB::bind_method(D_METHOD("set_instance_color", "col", "id"), &MyMultiMeshC::set_instance_color);
    ClassDB::bind_method(D_METHOD("set_instance_custom", "custom", "id"), &MyMultiMeshC::set_instance_custom);

    ClassDB::bind_method(D_METHOD("set_invalid", "id"), &MyMultiMeshC::set_invalid);
    ClassDB::bind_method(D_METHOD("add_now"), &MyMultiMeshC::add_now);
    ClassDB::bind_method(D_METHOD("update", "reset_now"), &MyMultiMeshC::update, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("_auto_exit_tree"), &MyMultiMeshC::_auto_exit_tree);
    ClassDB::bind_method(D_METHOD("exit_tree"), &MyMultiMeshC::exit_tree);
    ClassDB::bind_method(D_METHOD("print_attributes"), &MyMultiMeshC::print_attributes);

    ClassDB::bind_method(D_METHOD("set_multimesh", "multimesh"), &MyMultiMeshC::set_multimesh);
    ClassDB::bind_method(D_METHOD("get_multimesh"), &MyMultiMeshC::get_multimesh);
    ClassDB::bind_method(D_METHOD("set_canvas_item", "canvas_item"), &MyMultiMeshC::set_canvas_item);
    ClassDB::bind_method(D_METHOD("get_canvas_item"), &MyMultiMeshC::get_canvas_item);
}

void MyMultiMeshC::set_multimesh(const Ref<MultiMesh>& multimesh) { _multimesh = multimesh; }
Ref<MultiMesh> MyMultiMeshC::get_multimesh() const { return _multimesh; }

void MyMultiMeshC::set_canvas_item(const RID& canvas_item) { _canvas_item = canvas_item; }
RID MyMultiMeshC::get_canvas_item() const { return _canvas_item; }

MyMultiMeshC::MyMultiMeshC() {
    _multimesh.instantiate();
    _canvas_item = RenderingServer::get_singleton()->canvas_item_create();
}

MyMultiMeshC::~MyMultiMeshC() {}

void MyMultiMeshC::init(const Ref<Mesh>& p_mesh, int instance_max, bool use_colors, bool use_custom_data, bool auto_free) {
    _auto_free = auto_free;
    recreate();

    // MultiMesh 配置
    _multimesh->set_use_colors(use_colors);
    _multimesh->set_use_custom_data(use_custom_data);
    _multimesh->set_mesh(p_mesh);
    _multimesh->set_transform_format(MultiMesh::TRANSFORM_2D);
    _multimesh->set_instance_count(instance_max);

    // Note: Assuming Consts.MAX_31B and MAX_32B are defined elsewhere, using placeholders
    set_aabb(AABB(Vector3(-1000000.0, -1000000.0, 0.0), Vector3(2000000.0, 2000000.0, 0.0)));
    RenderingServer::get_singleton()->canvas_item_add_multimesh(_canvas_item, _multimesh->get_rid());
}

void MyMultiMeshC::recreate() {
    call_deferred("_recreate_deferred");
}

void MyMultiMeshC::_recreate_deferred() {
    Node2D* const vars_node = Call::get_vars();
    RID canvas = vars_node->get_canvas_item();
    RenderingServer::get_singleton()->canvas_item_set_parent(_canvas_item, canvas);
    vars_node->connect("on_pre_snece_changed", Callable(this, "_auto_exit_tree"));
}

void MyMultiMeshC::set_material(const Ref<Material>& mate) {
    _material = mate;
    RenderingServer::get_singleton()->canvas_item_set_material(_canvas_item, mate->get_rid());
}

void MyMultiMeshC::set_z(int z) {
    RenderingServer::get_singleton()->canvas_item_set_z_index(_canvas_item, z);
}

void MyMultiMeshC::set_aabb(const AABB& aabb) {
    _multimesh->set_custom_aabb(aabb);
}

void MyMultiMeshC::set_now_instance_transform(const Transform2D& trans) {
    _multimesh->set_instance_transform_2d(_now, trans);
}
void MyMultiMeshC::set_now_instance_color(const Color& col) {
    _multimesh->set_instance_color(_now, col);
}
void MyMultiMeshC::set_now_instance_custom(const Color& custom) {
    _multimesh->set_instance_custom_data(_now, custom);
}

void MyMultiMeshC::set_instance_transform(const Transform2D& trans, int id) {
    if (_now <= id) _now = id + 1;
    _multimesh->set_instance_transform_2d(id, trans);
}
void MyMultiMeshC::set_instance_color(const Color& col, int id) {
    if (_now <= id) _now = id + 1;
    _multimesh->set_instance_color(id, col);
}
void MyMultiMeshC::set_instance_custom(const Color& custom, int id) {
    if (_now <= id) _now = id + 1;
    _multimesh->set_instance_custom_data(id, custom);
}

void MyMultiMeshC::set_invalid(int id) {
    _multimesh->set_instance_transform_2d(id, INVALID_TRANSFORM);
}

void MyMultiMeshC::add_now() {
    _now += 1;
}

void MyMultiMeshC::update(bool reset_now) {
    if (_multimesh->get_visible_instance_count() != _now) {
        _multimesh->set_visible_instance_count(_now);
    }
    if (reset_now) {
        _now = 0;
    }
}

void MyMultiMeshC::_auto_exit_tree() {
    exit_tree();
    Call::get_vars()->disconnect("on_pre_snece_changed", Callable(this, "_auto_exit_tree"));
    // Assuming Vars.on_pre_scene_changed.disconnect is handled elsewhere
}

void MyMultiMeshC::exit_tree() {
    RenderingServer::get_singleton()->canvas_item_set_parent(_canvas_item, RID());
    _clear_data();
}

void MyMultiMeshC::_clear_data() {
    if (_multimesh->is_using_colors()) {
        for (int i = 0; i < _multimesh->get_instance_count(); ++i) {
            _multimesh->set_instance_color(i, Color());
        }
    } else if (_multimesh->is_using_custom_data()) {
        for (int i = 0; i < _multimesh->get_instance_count(); ++i) {
            _multimesh->set_instance_custom_data(i, Color());
        }
    }

    // 外部
    Callable(this, "_clear_data").call();
}

void MyMultiMeshC::print_attributes() {
    UtilityFunctions::print(_now);
    UtilityFunctions::print(String::num(_multimesh->get_visible_instance_count()) + "_" + String::num(_multimesh->get_instance_count()));
}