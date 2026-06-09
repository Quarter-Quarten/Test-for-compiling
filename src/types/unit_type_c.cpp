#include "unit_type_c.hpp"

#include "tools/call/call.hpp"
#include "tools/consts_c/consts_c.hpp"

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cmath>

using namespace godot;

UnitTypeC::UnitTypeC() {
}

// ==================== Step 1: 简单辅助函数 ====================

bool UnitTypeC::is_slime_c() {
    return _speed == Math_INF && _rotate_speed == 0.0;
}

void UnitTypeC::slime_c() {
    _speed = Math_INF;
    _rotate_speed = 0.0;
}

void UnitTypeC::fly_c(double h) {
    _flying = true;
    _height = h;
}

double UnitTypeC::get_max_speed_c(Object* u) {
    double speed_multi = u->get("speed_multi");
    return _speed * speed_multi * 60.0;
}

double UnitTypeC::damage_handle_c(Object* u, double damage, double armor_pierce) {
    return Math::max(damage - Math::max(get_real_armor_c(u) - armor_pierce, 0.0), 0.0);
}

double UnitTypeC::get_real_armor_c(Object* u) {
    return _armor;
}

// ==================== Step 2: 简单更新 ====================

void UnitTypeC::update_heal_c(Object* u, double delta) {
    if (_heal_amount <= 0.0) return;
    double health = u->get("health");
    if (health < _health) {
        u->call("healed", _heal_amount * delta, Variant(), false, false);
    }
}

void UnitTypeC::update_attributes_c(Object* u, double delta) {
    // height lerp
    const double HEIGHT_LERP_SPEED = 72.0;
    double height = u->get("height");
    double to_height = u->get("to_height");
    if (!Math::is_equal_approx(height, to_height)) {
        u->set("height", Math::lerp(height, to_height, 1.0 - std::exp(-delta * HEIGHT_LERP_SPEED)));
    }

    // invincible (flash handled in GDScript via TimeController)
    double invincible_time = u->get("invincible_time");
    if (invincible_time > 0.0) {
        u->set("invincible_time", invincible_time - delta);
    }

    // damage flash
    double flash_timer = u->get("flash_timer");
    if (flash_timer > 0.0) {
        flash_timer = Math::max(flash_timer - delta, 0.0);
        u->set("flash_timer", flash_timer);
        Color flash_color = u->get("flash_color");
        Color modulate = Color(1, 1, 1).lerp(flash_color, flash_timer / _flash_time);
        u->set("modulate", modulate);
    } else {
        u->set("modulate", Color(1, 1, 1));
    }
}

void UnitTypeC::update_data_c(Object* u) {
    Object* world = Call::get_world();
    if (!world) return;
    Object* units_ctrl = world->get("units");
    if (units_ctrl) {
        units_ctrl->call("update_data", u);
    }
    u->set("last_tick_pos", u->get("global_position"));
}

// ==================== Step 3: 移动物理 ====================

void UnitTypeC::update_move_c(Object* u, double delta) {
    // RPC位置插值（客户端）
    Object* vars = Call::get_vars();
    if (!vars) return;
    Object* player = vars->get("player");

    bool is_client = false;
    Object* multiplay = vars->get("multiplay");
    if (multiplay) {
        is_client = multiplay->call("is_client");
    }

    Object* units_ctrl = Call::get_world() ? Call::get_world()->get("units") : nullptr;

    if (u != player && is_client && units_ctrl) {
        double pos_rpc_rate = 20.0;
        double percent = delta * pos_rpc_rate;
        Vector2 pos = u->get("global_position");
        Vector2 rpc_pos = u->get("rpc_pos");
        pos += (rpc_pos - pos) * percent;
        u->set("global_position", pos);

        double rot = u->get("global_rotation");
        double rpc_rot = u->get("rpc_rot");
        double diff = static_cast<double>(Math::fmod(std::abs(rpc_rot - rot), Math_PI));
        // manual rotate_toward (not available in godot-cpp Math)
        {
            double d = Math::fmod(rpc_rot - rot, Math_TAU);
            if (d > Math_PI) d -= Math_TAU;
            if (d < -Math_PI) d += Math_TAU;
            rot += Math::clamp(d, -diff * percent, diff * percent);
        }
        u->set("global_rotation", Math::fmod(rot, Math_TAU));
        return;
    }

    // 最大速度
    double speed_multi = u->get("speed_multi");
    double max_speed = _speed * speed_multi * 60.0;
    double delta_60 = delta * 60.0;

    // 地板平滑
    double floor_smoothness = 0.0;
    if (!_flying && vars) {
        Object* system_ctrl = vars->get("SystemController");
        if (system_ctrl) {
            Vector2i tile = Call::pos_to_tile(Vector2(u->get("global_position")));
            Object* floor_type = system_ctrl->call("get_default_tile_floor", tile);
            if (floor_type) {
                floor_smoothness = floor_type->get("smoothness");
            }
        }
    }

    double real_accel = _accel * (1.0 - floor_smoothness) * delta_60;
    double real_drag = _drag * (1.0 - floor_smoothness) * delta_60;

    // 速度有效性检查
    Vector2 vel = u->get("vel");
    if (Math::is_nan(vel.x) || Math::is_inf(vel.x)) {
        vel = Vector2(0, 0);
    }

    double vel_len_sq = vel.length_squared();
    double vel_len = Math::sqrt(vel_len_sq);
    Vector2 vel_norm = vel_len_sq > 0.0 ? vel / vel_len : Vector2(0, 0);

    Vector2 to_pos = u->get("to_pos");
    Vector2 pos = u->get("global_position");

    // 判断是否停止
    double dst = (to_pos - pos).length();
    bool is_slime = (_speed == Math_INF && _rotate_speed == 0.0);
    bool is_stopping = (dst <= real_drag * 0.5) && !is_slime;

    if (is_stopping) {
        vel -= vel_norm * Math::min(real_drag, vel_len);
    } else {
        Vector2 last_vel = vel;
        // AI自定义移动
        bool ai_moved = false;
        if (_ai.get_type() == Variant::OBJECT) {
            Object* ai_obj = Object::cast_to<Object>(_ai);
            if (ai_obj) {
                bool use_custom = ai_obj->get("use_custom_move");
                if (use_custom) {
                    ai_obj->call("move", u, delta);
                    vel = u->get("vel");
                    ai_moved = true;
                }
            }
        }
        if (!ai_moved) {
            Vector2 dir = (to_pos - pos).normalized();
            vel = vel + dir * static_cast<real_t>(real_accel * speed_multi);
        }
        // 限速
        double last_len = last_vel.length();
        bool is_bouncing = u->call("is_bouncing");
        double limit = is_bouncing ? Math::max(last_len - real_drag, 0.0) : max_speed;
        vel = vel.limit_length(Math::max(limit, 0.0));
    }
    u->set("vel", vel);

    // 旋转
    if (vel.length_squared() > 0.0 && _rotate_speed > 0.0) {
        Object* turret = u->get("turret");
        double angle;
        if (turret) {
            Object* target = turret->get("target");
            if (target) {
                Vector2 target_pos = turret->get("target_pos");
                angle = (target_pos - pos).angle();
            } else {
                angle = vel.angle();
            }
        } else {
            angle = vel.angle();
        }
        double rot = u->get("global_rotation");
        {
            double d = Math::fmod(angle - rot, Math_TAU);
            if (d > Math_PI) d -= Math_TAU;
            if (d < -Math_PI) d += Math_TAU;
            rot += Math::clamp(d, -_rotate_speed * delta, _rotate_speed * delta);
        }
        u->set("global_rotation", Math::fmod(rot, Math_TAU));
    }

    // 最终位置更新
    u->set("global_position", pos + vel * static_cast<real_t>(delta));
}

// ==================== Step 4: 状态效果 ====================

bool UnitTypeC::update_status_c(Object* u, double delta) {
    Array status = u->get("status");
    int o = 0;
    int sc = status.size();
    for (int i = 0; i < sc; i++) {
        int sub = i - o;
        if (sub >= status.size()) break;
        Variant s_v = status[sub];
        if (s_v.get_type() == Variant::OBJECT) {
            Object* s = Object::cast_to<Object>(s_v);
            if (s) {
                bool died = s->call("update_unit", u, delta);
                if (died) {
                    this->call("remove_status", u, sub);
                    o += 1;
                }
            }
        }
        if (!u->get("type")) return true;
    }
    return !u->get("type");
}

// ==================== Step 5: 方块碰撞 ====================

void UnitTypeC::update_collide_block_c(Object* u, double delta) {
    double u_size_half = _size;
    Vector2 last_pos = u->get("last_pos");
    Vector2 pos = u->get("global_position");
    Vector2 half_size_2d = Vector2(u_size_half, u_size_half);

    Rect2i last_tile_rect = Call::global_rect_to_tile(Rect2(last_pos - half_size_2d, half_size_2d * 2.0));
    Rect2i next_tile_rect = Call::global_rect_to_tile(Rect2(pos - half_size_2d, half_size_2d * 2.0));

    Object* vars = Call::get_vars();
    Dictionary block_instances;
    if (vars) {
        block_instances = vars->get("block_instances");
    }

    if (last_tile_rect != next_tile_rect) {
        // 进入的新tile
        Call::iterate_points_in_r1_not_r2(next_tile_rect, last_tile_rect, [&](Vector2i tile) {
            if (!block_instances.has(tile)) return;
            Object* b = Object::cast_to<Object>(block_instances[tile]);
            if (!b) return;
            bool can_collide = b->call("collide_unit");
            if (!can_collide) return;
            u->call("add_colliding", tile, b);
        });
        // 离开的旧tile
        Call::iterate_points_in_r1_not_r2(last_tile_rect, next_tile_rect, [&](Vector2i tile) {
            u->call("erase_colliding", tile);
        });
    }

    // 碰撞解析
    Dictionary colliding = u->get("colliding");
    Array keys = colliding.keys();
    for (int i = 0; i < keys.size(); i++) {
        Vector2i tile = keys[i];
        Variant b_v = colliding[tile];
        Object* b = Object::cast_to<Object>(b_v);
        if (!b || b->is_queued_for_deletion()) {
            u->call("erase_colliding", tile);
            continue;
        }
        bool b_dead = b->call("dead");
        if (b_dead) {
            u->call("erase_colliding", tile);
            continue;
        }
        bool can_collide = b->call("collide_unit");
        if (!can_collide) {
            u->call("erase_colliding", tile);
            continue;
        }
        Rect2 tile_rect = b->call("get_tile_rect");
        if (!next_tile_rect.intersects(tile_rect)) {
            u->call("erase_colliding", tile);
            continue;
        }

        Rect2 collision_rect = b->call("get_collision_rect");
        if (!Call::rect_circle_intersects(collision_rect, _size, pos)) continue;

        const double DISTANCE = 0.0;
        Object* block_type = Object::cast_to<Object>(b->get("type"));
        if (!block_type) continue;

        Vector2 collision_size = collision_rect.size * 0.5;
        Vector2 center = collision_rect.get_center();
        Vector2 abs_d = (center - pos).abs();
        Vector2 vel = u->get("vel");

        this->call("collide_block_call_deferred", u, b, vel);

        bool side_x = abs_d.x <= collision_size.x;
        bool side_y = abs_d.y <= collision_size.y;

        if (side_x || side_y) {
            double smoothness = block_type->get("smoothness");
            if (abs_d.x > abs_d.y) {
                // 左或右面碰撞
                double dir_x = Math::sign(pos.x - center.x);
                if (dir_x == -Math::sign(vel.x)) {
                    vel.y = Math::lerp(static_cast<double>(vel.y), 0.0, 1.0 - std::pow(smoothness, delta * 10.0));
                    vel.x = 0.0;
                    pos.x = (collision_size.x + _size + DISTANCE) * dir_x + center.x;
                }
            } else {
                // 上或下面碰撞
                double dir_y = Math::sign(pos.y - center.y);
                if (dir_y == -Math::sign(vel.y)) {
                    vel.x = Math::lerp(static_cast<double>(vel.x), 0.0, 1.0 - std::pow(smoothness, delta * 10.0));
                    vel.y = 0.0;
                    pos.y = (collision_size.y + _size + DISTANCE) * dir_y + center.y;
                }
            }
            u->set("vel", vel);
            u->set("global_position", pos);
        } else {
            // 边角碰撞
            Vector2 point = center + collision_size * (pos - center).sign();
            pos = ((pos - point).normalized() * (_size + DISTANCE)) + point;
            u->set("global_position", pos);
        }
    }
}

// ==================== Step 6: 主更新入口 ====================

bool UnitTypeC::update_c(Object* u, double delta) {
    u->set("last_pos", u->get("global_position"));

    update_move_c(u, delta);
    update_attributes_c(u, delta);

    // 碰撞
    Object* vars = Call::get_vars();
    bool collision_enabled = true;
    if (vars) {
        Object* setting_obj = Object::cast_to<Object>(vars->get("setting"));
        if (setting_obj) {
            Object* unit_collision = Object::cast_to<Object>(setting_obj->get("unit_collision"));
            if (unit_collision) {
                collision_enabled = unit_collision->call("get_value");
            }
        }
    }

    if (collision_enabled) {
        if (!_flying) {
            bool only_sprite = u->get("is_only_sprite");
            if (!only_sprite) {
                update_collide_block_c(u, delta);
            }
        }
        this->call("update_collide_unit", u);
        if (u->call("dead")) return true;
    }

    // turret
    Object* turret = u->get("turret");
    if (turret) {
        turret->call("update", delta);
    }

    bool only_sprite = u->get("is_only_sprite");
    if (only_sprite) return false;

    if (update_status_c(u, delta)) return true;

    return false;
}

bool UnitTypeC::tick_update_c(Object* u, double delta) {
    update_data_c(u);

    // height
    if (is_slime_c()) {
        Vector2 vel = u->get("vel");
        double h = Math::sqrt(vel.length()) + _height;
        u->set("to_height", h);
    } else {
        u->set("to_height", _height);
    }

    bool only_sprite = u->get("is_only_sprite");
    if (only_sprite) return false;

    // AI
    if (_ai.get_type() == Variant::OBJECT) {
        Object* ai_obj = Object::cast_to<Object>(_ai);
        if (ai_obj) {
            ai_obj->call("update", u, delta);
        }
    }

    // multipliers
    u->set("health_multi", this->call("get_health_multi", u));
    u->set("damage_multi", this->call("get_damage_multi", u));
    u->set("speed_multi", this->call("get_speed_multi", u));
    u->set("reload_speed_multi", this->call("get_reload_speed_multi", u));

    // heal
    if (_heal_amount > 0.0) {
        update_heal_c(u, delta);
    }

    // light
    Object* light = u->get("light");
    if (light) {
        light->call("tick_update", delta);
    }

    return false;
}

void UnitTypeC::_bind_methods() {
    // Bool properties
    ClassDB::bind_method(D_METHOD("set_draw_health_prog", "v"), &UnitTypeC::set_draw_health_prog);
    ClassDB::bind_method(D_METHOD("get_draw_health_prog"), &UnitTypeC::get_draw_health_prog);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "draw_health_prog"), "set_draw_health_prog", "get_draw_health_prog");

    ClassDB::bind_method(D_METHOD("set_flying", "v"), &UnitTypeC::set_flying);
    ClassDB::bind_method(D_METHOD("get_flying"), &UnitTypeC::get_flying);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "flying"), "set_flying", "get_flying");

    ClassDB::bind_method(D_METHOD("set_collide_unit", "v"), &UnitTypeC::set_collide_unit);
    ClassDB::bind_method(D_METHOD("get_collide_unit"), &UnitTypeC::get_collide_unit);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "collide_unit"), "set_collide_unit", "get_collide_unit");

    ClassDB::bind_method(D_METHOD("set_draw_target_pos", "v"), &UnitTypeC::set_draw_target_pos);
    ClassDB::bind_method(D_METHOD("get_draw_target_pos"), &UnitTypeC::get_draw_target_pos);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "draw_target_pos"), "set_draw_target_pos", "get_draw_target_pos");

    ClassDB::bind_method(D_METHOD("set_heal_unit", "v"), &UnitTypeC::set_heal_unit);
    ClassDB::bind_method(D_METHOD("get_heal_unit"), &UnitTypeC::get_heal_unit);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "heal_unit"), "set_heal_unit", "get_heal_unit");

    ClassDB::bind_method(D_METHOD("set_is_interactive", "v"), &UnitTypeC::set_is_interactive);
    ClassDB::bind_method(D_METHOD("get_is_interactive"), &UnitTypeC::get_is_interactive);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::BOOL, "is_interactive"), "set_is_interactive", "get_is_interactive");

    // Float properties
    ClassDB::bind_method(D_METHOD("set_damaged_invincible_time", "v"), &UnitTypeC::set_damaged_invincible_time);
    ClassDB::bind_method(D_METHOD("get_damaged_invincible_time"), &UnitTypeC::get_damaged_invincible_time);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "damaged_invincible_time"), "set_damaged_invincible_time", "get_damaged_invincible_time");

    ClassDB::bind_method(D_METHOD("set_height", "v"), &UnitTypeC::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &UnitTypeC::get_height);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "height"), "set_height", "get_height");

    ClassDB::bind_method(D_METHOD("set_heal_amount", "v"), &UnitTypeC::set_heal_amount);
    ClassDB::bind_method(D_METHOD("get_heal_amount"), &UnitTypeC::get_heal_amount);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "heal_amount"), "set_heal_amount", "get_heal_amount");

    ClassDB::bind_method(D_METHOD("set_flash_time", "v"), &UnitTypeC::set_flash_time);
    ClassDB::bind_method(D_METHOD("get_flash_time"), &UnitTypeC::get_flash_time);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "flash_time"), "set_flash_time", "get_flash_time");

    ClassDB::bind_method(D_METHOD("set_rotate_speed", "v"), &UnitTypeC::set_rotate_speed);
    ClassDB::bind_method(D_METHOD("get_rotate_speed"), &UnitTypeC::get_rotate_speed);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "rotate_speed"), "set_rotate_speed", "get_rotate_speed");

    ClassDB::bind_method(D_METHOD("set_armor", "v"), &UnitTypeC::set_armor);
    ClassDB::bind_method(D_METHOD("get_armor"), &UnitTypeC::get_armor);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "armor"), "set_armor", "get_armor");

    ClassDB::bind_method(D_METHOD("set_speed", "v"), &UnitTypeC::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &UnitTypeC::get_speed);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_health", "v"), &UnitTypeC::set_health);
    ClassDB::bind_method(D_METHOD("get_health"), &UnitTypeC::get_health);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "health"), "set_health", "get_health");

    ClassDB::bind_method(D_METHOD("set_accel", "v"), &UnitTypeC::set_accel);
    ClassDB::bind_method(D_METHOD("get_accel"), &UnitTypeC::get_accel);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "accel"), "set_accel", "get_accel");

    ClassDB::bind_method(D_METHOD("set_drag", "v"), &UnitTypeC::set_drag);
    ClassDB::bind_method(D_METHOD("get_drag"), &UnitTypeC::get_drag);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "drag"), "set_drag", "get_drag");

    ClassDB::bind_method(D_METHOD("set_size", "v"), &UnitTypeC::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &UnitTypeC::get_size);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "size"), "set_size", "get_size");

    ClassDB::bind_method(D_METHOD("set_kbrst", "v"), &UnitTypeC::set_kbrst);
    ClassDB::bind_method(D_METHOD("get_kbrst"), &UnitTypeC::get_kbrst);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "kbrst"), "set_kbrst", "get_kbrst");

    ClassDB::bind_method(D_METHOD("set_despawn_sound_volume", "v"), &UnitTypeC::set_despawn_sound_volume);
    ClassDB::bind_method(D_METHOD("get_despawn_sound_volume"), &UnitTypeC::get_despawn_sound_volume);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "despawn_sound_volume"), "set_despawn_sound_volume", "get_despawn_sound_volume");

    ClassDB::bind_method(D_METHOD("set_heal_other_amount", "v"), &UnitTypeC::set_heal_other_amount);
    ClassDB::bind_method(D_METHOD("get_heal_other_amount"), &UnitTypeC::get_heal_other_amount);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "heal_other_amount"), "set_heal_other_amount", "get_heal_other_amount");

    ClassDB::bind_method(D_METHOD("set_hit_sound_volumn", "v"), &UnitTypeC::set_hit_sound_volumn);
    ClassDB::bind_method(D_METHOD("get_hit_sound_volumn"), &UnitTypeC::get_hit_sound_volumn);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::FLOAT, "hit_sound_volumn"), "set_hit_sound_volumn", "get_hit_sound_volumn");

    // Object/Variant properties
    ClassDB::bind_method(D_METHOD("set_ai", "v"), &UnitTypeC::set_ai);
    ClassDB::bind_method(D_METHOD("get_ai"), &UnitTypeC::get_ai);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "ai", PROPERTY_HINT_RESOURCE_TYPE, "UnitAI"), "set_ai", "get_ai");

    ClassDB::bind_method(D_METHOD("set_sprite", "v"), &UnitTypeC::set_sprite);
    ClassDB::bind_method(D_METHOD("get_sprite"), &UnitTypeC::get_sprite);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "sprite", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_sprite", "get_sprite");

    ClassDB::bind_method(D_METHOD("set_despawn_effect", "v"), &UnitTypeC::set_despawn_effect);
    ClassDB::bind_method(D_METHOD("get_despawn_effect"), &UnitTypeC::get_despawn_effect);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "despawn_effect", PROPERTY_HINT_RESOURCE_TYPE, "EffectType"), "set_despawn_effect", "get_despawn_effect");

    ClassDB::bind_method(D_METHOD("set_despawn_sound", "v"), &UnitTypeC::set_despawn_sound);
    ClassDB::bind_method(D_METHOD("get_despawn_sound"), &UnitTypeC::get_despawn_sound);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "despawn_sound", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_despawn_sound", "get_despawn_sound");

    ClassDB::bind_method(D_METHOD("set_turret", "v"), &UnitTypeC::set_turret);
    ClassDB::bind_method(D_METHOD("get_turret"), &UnitTypeC::get_turret);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "turret", PROPERTY_HINT_RESOURCE_TYPE, "TurretComponent"), "set_turret", "get_turret");

    ClassDB::bind_method(D_METHOD("set_light", "v"), &UnitTypeC::set_light);
    ClassDB::bind_method(D_METHOD("get_light"), &UnitTypeC::get_light);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "light", PROPERTY_HINT_RESOURCE_TYPE, "LightComponent"), "set_light", "get_light");

    ClassDB::bind_method(D_METHOD("set_hit_sound", "v"), &UnitTypeC::set_hit_sound);
    ClassDB::bind_method(D_METHOD("get_hit_sound"), &UnitTypeC::get_hit_sound);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::OBJECT, "hit_sound", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_hit_sound", "get_hit_sound");

    ClassDB::bind_method(D_METHOD("set_spawn_units_on_death", "v"), &UnitTypeC::set_spawn_units_on_death);
    ClassDB::bind_method(D_METHOD("get_spawn_units_on_death"), &UnitTypeC::get_spawn_units_on_death);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::DICTIONARY, "spawn_units_on_death"), "set_spawn_units_on_death", "get_spawn_units_on_death");

    // Callable property
    ClassDB::bind_method(D_METHOD("set_on_shooting", "v"), &UnitTypeC::set_on_shooting);
    ClassDB::bind_method(D_METHOD("get_on_shooting"), &UnitTypeC::get_on_shooting);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::CALLABLE, "on_shooting"), "set_on_shooting", "get_on_shooting");

    // Array property
    ClassDB::bind_method(D_METHOD("set_att_progs", "v"), &UnitTypeC::set_att_progs);
    ClassDB::bind_method(D_METHOD("get_att_progs"), &UnitTypeC::get_att_progs);
    ClassDB::add_property("UnitTypeC", PropertyInfo(Variant::ARRAY, "att_progs"), "set_att_progs", "get_att_progs");

    // Step 1: 简单辅助函数
    ClassDB::bind_method(D_METHOD("is_slime_c"), &UnitTypeC::is_slime_c);
    ClassDB::bind_method(D_METHOD("slime_c"), &UnitTypeC::slime_c);
    ClassDB::bind_method(D_METHOD("fly_c", "h"), &UnitTypeC::fly_c);
    ClassDB::bind_method(D_METHOD("get_max_speed_c", "u"), &UnitTypeC::get_max_speed_c);
    ClassDB::bind_method(D_METHOD("damage_handle_c", "u", "damage", "armor_pierce"), &UnitTypeC::damage_handle_c);
    ClassDB::bind_method(D_METHOD("get_real_armor_c", "u"), &UnitTypeC::get_real_armor_c);

    // Step 2: 简单更新
    ClassDB::bind_method(D_METHOD("update_heal_c", "u", "delta"), &UnitTypeC::update_heal_c);
    ClassDB::bind_method(D_METHOD("update_attributes_c", "u", "delta"), &UnitTypeC::update_attributes_c);
    ClassDB::bind_method(D_METHOD("update_data_c", "u"), &UnitTypeC::update_data_c);

    // Step 3: 移动物理
    ClassDB::bind_method(D_METHOD("update_move_c", "u", "delta"), &UnitTypeC::update_move_c);

    // Step 4: 状态效果
    ClassDB::bind_method(D_METHOD("update_status_c", "u", "delta"), &UnitTypeC::update_status_c);

    // Step 5: 方块碰撞
    ClassDB::bind_method(D_METHOD("update_collide_block_c", "u", "delta"), &UnitTypeC::update_collide_block_c);

    // Step 6: 主更新入口
    ClassDB::bind_method(D_METHOD("update_c", "u", "delta"), &UnitTypeC::update_c);
    ClassDB::bind_method(D_METHOD("tick_update_c", "u", "delta"), &UnitTypeC::tick_update_c);
}
