#pragma once
#include "ecs/components.hpp"
#include "types/bullet_type_c.hpp"
#include "types/unit_type_c.hpp"
#include <ecs/lib/flecs.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <vector>

using namespace godot;

namespace ecs {
    class ECSWorld : public Node {
        GDCLASS(ECSWorld, Node);
    public:
        ECSWorld();
        ~ECSWorld();

    void init(
        QuadTreeManagerC* quad_tree, MyMultiMeshC* bullet_drawer,
        MyMultiMeshC* effect_circle, MyMultiMeshC* effect_rect,
        MyMultiMeshC* effect_triangle, MyMultiMeshC* point_lights
    );

    void _ready() override;
    void _exit_tree() override;
    void update(double delta);

    int get_last_process_duration() const;

    void create_bullet(
        const Vector2& position, const Vector2& velocity,
        float damage_multi, int team,
        const Ref<BulletTypeC>& bullet_type
    );

    void create_unit(const Vector2& position, int team, float rotation, const Ref<UnitTypeC>& unit_type);

    void create_particle(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone,
        int shape_type
    );

    void create_torus_particle(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone,
        float width_from, float width_to
    );

    protected:
        static void _bind_methods();
    private:
        flecs::world world;
        int last_process_duration_;
        RID trail_canvas_rid_;
        
        IDPool<int> particle_ids_;
    };
}
