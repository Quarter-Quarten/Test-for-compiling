#include "ecs/ecs_world.hpp"
#include "ecs/systems.hpp"
#include "tools/call/call.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/math.hpp>
#include <cstdlib>
#include <cassert>

using namespace godot;

namespace ecs {
    ECSWorld::ECSWorld() {
    	global::init_global(this);
    }

    ECSWorld::~ECSWorld() {
        if (trail_canvas_rid_.is_valid()) {
            RenderingServer::get_singleton()->free_rid(trail_canvas_rid_);
        }
    }

    void ECSWorld::_exit_tree() {
    }

    void ECSWorld::init(
        QuadTreeManagerC* quad_tree, MyMultiMeshC* bullet_drawer,
        MyMultiMeshC* effect_circle, MyMultiMeshC* effect_rect,
        MyMultiMeshC* effect_triangle, MyMultiMeshC* point_lights
    ) {
        world.set<QuadTreeComp>({ quad_tree });
        world.set<BulletDrawer>({ bullet_drawer });
        world.set<EffectCircleDrawer>({ effect_circle });
        world.set<EffectRectDrawer>({ effect_rect });
        world.set<EffectTriangleDrawer>({ effect_triangle });
        world.set<PointLightDrawer>({ point_lights });

        int thread_count = static_cast<int>(Call::get_setting()->get("process_thread_count").call("get_value"));
        if (thread_count > 1) world.set_threads(thread_count - 1);  // 创建 thread_count-1 个 worker 线程，总计 thread_count 个线程
    }
    void ECSWorld::_ready() {
        if (Engine::get_singleton()->is_editor_hint())
            return;
        register_components(world);
        register_systems(world, this);

        trail_canvas_rid_ = RenderingServer::get_singleton()->canvas_item_create();
        unit_canvas_rid_ = RenderingServer::get_singleton()->canvas_item_create();
        Node2D* vars_node = Call::get_vars();
        if (vars_node) {
            RenderingServer::get_singleton()->canvas_item_set_parent(trail_canvas_rid_, vars_node->get_canvas_item());
            RenderingServer::get_singleton()->canvas_item_set_z_index(trail_canvas_rid_, ConstsC::get_bullet_z());
            
            RenderingServer::get_singleton()->canvas_item_set_parent(unit_canvas_rid_, vars_node->get_canvas_item());
            RenderingServer::get_singleton()->canvas_item_set_z_index(unit_canvas_rid_, ConstsC::get_unit_z());
        }
        world.set<TrailDrawer>({ trail_canvas_rid_ });
        world.set<UnitDrawer>({ unit_canvas_rid_ });
    }

    void ECSWorld::update(double delta) {
        if (!world) return;

        int timer = Time::get_singleton()->get_ticks_usec();
        ecs_progress(world, delta);
        last_process_duration_ = Time::get_singleton()->get_ticks_usec() - timer;
    }

    int ECSWorld::get_last_process_duration() const {
        return last_process_duration_;
    }

    void ECSWorld::create_bullet(
        const Vector2& position, const Vector2& velocity,
        float damage_multi, int team,
        const Ref<BulletTypeC>& bullet_type
    ) {
        assert(bullet_type.is_valid());

        flecs::entity e = world.entity();
        e.insert([&](Position& pos, LastPosition& last_pos, Velocity& vel, 
                     Rotation& rot, BulletOrigin& origin,
                     Lifetime& lifetime, ColorComp& color, Team& team_c,
                     SizeValue& size, BulletDamage& damage, BulletCollision& collision) { 
            pos = {position};
            last_pos = {position};
            vel = {velocity};
            rot = {velocity.angle()};
            origin = {position};
            
            lifetime = {static_cast<float>(bullet_type->get_lifetime()), 0.0f};
            color = {bullet_type->get_color()};
            team_c = {team};
            
            size = {static_cast<float>(bullet_type->get_size())};
            
            damage = {
                static_cast<float>(bullet_type->get_damage()),
                damage_multi,
                static_cast<float>(bullet_type->get_cutting_damage()),
                static_cast<float>(bullet_type->get_armor_pierce())
            };
            
            collision = {
                bullet_type->get_collide(),
                bullet_type->get_collide(),
                static_cast<int>(bullet_type->get_pierce_cap()),
                {}
            };
        });

        float spin = static_cast<float>(bullet_type->get_spin());
        if (spin) e.set<RotateSpeed>({spin});

        float drag = static_cast<float>(bullet_type->get_drag());
        float drag_percent = static_cast<float>(bullet_type->get_drag_percent());
        if (drag != 0.0f || drag_percent != 1.0f) {
            e.set<Acceleration>({-drag, drag_percent});
        }

        float homing_power = static_cast<float>(bullet_type->get_homing_power());
        float homing_range = static_cast<float>(bullet_type->get_homing_range());
        if (homing_power && homing_range) {
            e.set<HomingPower>({homing_power})
             .set<HomingRange>({homing_range});
            float homing_delay = static_cast<float>(bullet_type->get_homing_delay());
            if (homing_delay) e.set<HomingDelay>({homing_delay});
        }

        if (bullet_type->get_scale_from() != Vector2(1, 1) || bullet_type->get_scale_to() != Vector2(1, 1)) {
            e.set<ScaleVector>({bullet_type->get_scale_from()});
            e.set<ScaleVectorFromTo>({
            	bullet_type->get_scale_from(),
            	bullet_type->get_scale_to()
            });
        }

        if (bullet_type->has_trail()) {
            Trail t;
            t.max_len = Math::min(static_cast<int>(bullet_type->get_trail_len()), Trail::MAX_POINTS);
            t.max_len = Math::max(t.max_len, 2);
            t.max_width = static_cast<float>(bullet_type->get_trail_width());
            t.front_color = bullet_type->get_trail_color_front();
            t.back_color = bullet_type->get_trail_color_back();
            t.points[0] = position;
            t.now_len = 1;
            t.next_point = 1;
            e.set<Trail>(std::move(t));
        }

        BulletTypeComp btc;
        btc.speed        = static_cast<float>(bullet_type->get_speed());
        btc.bullet_type  = bullet_type.ptr();
        e.set<BulletTypeComp>(btc);

        switch (bullet_type->get_bullet_kind())
        {
        case BulletTypeC::BULLET_KIND_BULLET:
            break;
        case BulletTypeC::BULLET_KIND_LIGHTNING_BULLET:
            e.add<LightingBullet>();
            break;
        case BulletTypeC::BULLET_KIND_POINT_BULLET:
            e.add<PointBullet>();
            break;
        
        default:
            break;
        }
    }

    void ECSWorld::create_unit(const Vector2& position, int team, float rotation, const Ref<UnitTypeC>& unit_type) {
        assert(unit_type.is_valid());
        flecs::entity e = world.entity();
        e.insert([&](Position& pos, LastPosition& last_pos, MoveToPosition& move_to, Velocity& vel,
                     Rotation& rot, UnitTypeComp& utc,
                     ColorComp& color, Team& team_c,
                     SizeValue& size) { 
            pos = {position};
            last_pos = {position};
            move_to = {position};
            vel = {};
            rot = {rotation};
            
            
            color = {Color(1, 1, 1, 1)};
            team_c = {team};
            
            utc = {unit_type.ptr()};
            
            size = {static_cast<float>(unit_type->get_size())};
        });

        switch (unit_type->get_unit_kind())
        {
        case UnitTypeC::UNIT_KIND_UNIT:
            break;
        case UnitTypeC::UNIT_KIND_PLAYER:
            e.add<Player>();
            break;
        case UnitTypeC::UNIT_KIND_ENEMY:
            e.add<Enemy>();
            break;
        case UnitTypeC::UNIT_KIND_NPC:
            e.add<NPC>();
            break;
        case UnitTypeC::UNIT_KIND_SUMMONED:
            e.add<Summoned>();
            break;
        
        default:
            break;
        }
    }



    void ECSWorld::create_particle(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone,
        int shape_type
    ) {
        for (int i = 0; i < count; i++) {
            flecs::entity e = world.entity()
                .set<Lifetime>({lifetime, 0.0f});

            float p_rot = (radiate ? i * radiate_angle : Math::deg_to_rad(Call::randf_range(-cone, cone))) + rota;
            Vector2 p_scale = scale * Call::randf_range(scale_rand_min, 1.0f);
            Vector2 pos_to = Vector2::from_angle(p_rot) * (Call::randf_range(-len_rand, len_rand) + length) + pos;

            e.set<ParticleInfo>({
                pos, pos_to, pos_ease,
                col_from, col_to, col_ease,
                p_scale, scale_to, scale_ease,
                p_rot, lifetime, 0
            });

            if (shape_type == 0) e.add<CircleParticle>();
            else if (shape_type == 1) e.add<TriangleParticle>();
            else if (shape_type == 2) e.add<RectParticle>();
        }
    }

    void ECSWorld::create_torus_particle(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone,
        float width_from, float width_to
    ) {
        for (int i = 0; i < count; i++) {
            flecs::entity e = world.entity()
                .set<Lifetime>({lifetime, 0.0f})
                .add<TorusParticle>();

            float p_rot = (radiate ? i * radiate_angle : Math::deg_to_rad(Call::randf_range(-cone, cone))) + rota;
            Vector2 p_scale = scale * Call::randf_range(scale_rand_min, 1.0f);
            Vector2 pos_to = Vector2::from_angle(p_rot) * (Call::randf_range(-len_rand, len_rand) + length) + pos;

            e.set<ParticleInfo>({
                pos, pos_to, pos_ease,
                col_from, col_to, col_ease,
                p_scale, scale_to, scale_ease,
                p_rot, lifetime, 0
            });

            e.set<TorusParticleInfo>({ width_from, width_to });
        }
    }


    void ECSWorld::_bind_methods() {
        ClassDB::bind_method(D_METHOD("create_bullet", "position", "velocity", "damage_multi", "team", "bullet_type"), &ECSWorld::create_bullet);
        ClassDB::bind_method(D_METHOD("create_unit", "position", "team", "rotation", "unit_type"), &ECSWorld::create_unit);
        
        ClassDB::bind_method(D_METHOD("init", "quad_tree", "bullet_drawer", "effect_circle", "effect_rect", "effect_triangle", "point_lights"), &ECSWorld::init);
        
        ClassDB::bind_method(D_METHOD("get_last_process_duration"), &ECSWorld::get_last_process_duration);
        
        ClassDB::bind_method(D_METHOD("update", "delta"), &ECSWorld::update);
        
        ClassDB::bind_method(D_METHOD("create_particle", "pos", "rota", "length", "len_rand", "pos_ease", "col_from", "col_to", "col_ease", "lifetime", "count", "scale", "scale_rand_min", "scale_to", "scale_ease", "radiate", "radiate_angle", "cone", "shape_type"), &ECSWorld::create_particle);
        ClassDB::bind_method(D_METHOD("create_torus_particle", "pos", "rota", "length", "len_rand", "pos_ease", "col_from", "col_to", "col_ease", "lifetime", "count", "scale", "scale_rand_min", "scale_to", "scale_ease", "radiate", "radiate_angle", "cone", "width_from", "width_to"), &ECSWorld::create_torus_particle);
    }

} // namespace ecs
