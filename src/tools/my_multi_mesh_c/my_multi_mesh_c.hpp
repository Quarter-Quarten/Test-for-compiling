#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {

class MyMultiMeshC : public RefCounted {
    GDCLASS(MyMultiMeshC, RefCounted);

private:
    static const Transform2D INVALID_TRANSFORM;

    bool _auto_free = false;
    Ref<MultiMesh> _multimesh;
    RID _canvas_item;
    Ref<Material> _material;
    int _now = 0;

    void set_multimesh(const Ref<MultiMesh>& multimesh);
    Ref<MultiMesh> get_multimesh() const;
    void set_canvas_item(const RID& canvas_item);
    RID get_canvas_item() const;

protected:
    static void _bind_methods();

public:
    MyMultiMeshC();
    ~MyMultiMeshC();

    void init(const Ref<Mesh>& p_mesh, int instance_max, bool use_colors, bool use_custom_data, bool auto_free = true);
    void recreate();
    void _recreate_deferred();
    void set_material(const Ref<Material>& mate);
    void set_z(int z);
    void set_aabb(const AABB& aabb);

    void set_now_instance_transform(const Transform2D& trans);
    void set_now_instance_color(const Color& col);
    void set_now_instance_custom(const Color& custom);

    void set_instance_transform(const Transform2D& trans, int id);
    void set_instance_color(const Color& col, int id);
    void set_instance_custom(const Color& custom, int id);

    void set_invalid(int id);
    void add_now();
    void update(bool reset_now = true);
    void _auto_exit_tree();
    void exit_tree();
    void _clear_data();
    void print_attributes();
};

} // namespace godot