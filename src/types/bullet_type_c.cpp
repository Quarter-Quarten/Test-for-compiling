#include "bullet_type_c.hpp"
#include "tools/call/call.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

BulletTypeC::BulletTypeC() {
}

bool BulletTypeC::has_trail() {
    return _trail_len > 0;
}

float BulletTypeC::get_real_speed() {
    Object* vars = Call::get_vars();
    if (!vars) return _speed;
    Variant d = vars->get("d");
    return _speed * (d.get_type() == Variant::FLOAT ? float(d) : 1.0f);
}

float BulletTypeC::get_hit_damage(Object *b) {
    if (!b) return 0.0f;
    float b_damage = b->get("damage");
    Vector2 b_vel = b->get("vel");
    float b_damage_multi = b->get("damage_multi");

    return (b_damage + (b_vel.length() / _speed) * _cutting_damage) * b_damage_multi;
}


void BulletTypeC::draw_introduction_c(Object *bar) {
    if (!bar) return;
    bar->call("add_level");
    TypeC::draw_introduction_c(bar);
    draw_itdct(bar);
    bar->call("reduce_level");
}

void BulletTypeC::draw_itdct(Object *bar) {
    if (!bar) return;
    Object* vars = Call::get_vars();
    if (!vars) return;
    Variant bundle = vars->get("bundle");

    if (bundle.get_type() == Variant::OBJECT) {
        Object* b = bundle;
        if (b) {
            bar->call("draw_text", String(b->get("speed")) + String::num(_speed));
            if (_damage > 0.0f)
                bar->call("draw_text", String(b->get("damage")) + " " + String::num(_damage));
            if (_cutting_damage > 0.0f)
                bar->call("draw_text", String(b->get("cutting_damage")) + " " + String::num(_cutting_damage));
            if (_splash_damage > 0.0f && _splash_range > 0.0f)
                bar->call("draw_text", String(b->get("splash_damage")) + " " + String::num(_splash_damage));
            if (_crit_chance > 0.0f)
                bar->call("draw_text", String(b->get("crit_chance")) + " " + String::num(_crit_chance));
        }
    }
}

void BulletTypeC::_bind_methods() {
    // Bool properties
    ClassDB::bind_method(D_METHOD("set_hidden", "v"), &BulletTypeC::set_hidden);
    ClassDB::bind_method(D_METHOD("get_hidden"), &BulletTypeC::get_hidden);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "hidden"), "set_hidden", "get_hidden");

    ClassDB::bind_method(D_METHOD("set_splash_hurt_self", "v"), &BulletTypeC::set_splash_hurt_self);
    ClassDB::bind_method(D_METHOD("get_splash_hurt_self"), &BulletTypeC::get_splash_hurt_self);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "splash_hurt_self"), "set_splash_hurt_self", "get_splash_hurt_self");

    ClassDB::bind_method(D_METHOD("set_color_gradient", "v"), &BulletTypeC::set_color_gradient);
    ClassDB::bind_method(D_METHOD("get_color_gradient"), &BulletTypeC::get_color_gradient);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "color_gradient"), "set_color_gradient", "get_color_gradient");

    ClassDB::bind_method(D_METHOD("set_rand_dir", "v"), &BulletTypeC::set_rand_dir);
    ClassDB::bind_method(D_METHOD("get_rand_dir"), &BulletTypeC::get_rand_dir);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "rand_dir"), "set_rand_dir", "get_rand_dir");

    ClassDB::bind_method(D_METHOD("set_splash_add_status", "v"), &BulletTypeC::set_splash_add_status);
    ClassDB::bind_method(D_METHOD("get_splash_add_status"), &BulletTypeC::get_splash_add_status);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "splash_add_status"), "set_splash_add_status", "get_splash_add_status");

    ClassDB::bind_method(D_METHOD("set_hit_add_status", "v"), &BulletTypeC::set_hit_add_status);
    ClassDB::bind_method(D_METHOD("get_hit_add_status"), &BulletTypeC::get_hit_add_status);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "hit_add_status"), "set_hit_add_status", "get_hit_add_status");

    ClassDB::bind_method(D_METHOD("set_collide", "v"), &BulletTypeC::set_collide);
    ClassDB::bind_method(D_METHOD("get_collide"), &BulletTypeC::get_collide);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "collide"), "set_collide", "get_collide");

    ClassDB::bind_method(D_METHOD("set_collide_unit", "v"), &BulletTypeC::set_collide_unit);
    ClassDB::bind_method(D_METHOD("get_collide_unit"), &BulletTypeC::get_collide_unit);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "collide_unit"), "set_collide_unit", "get_collide_unit");

    ClassDB::bind_method(D_METHOD("set_collide_block", "v"), &BulletTypeC::set_collide_block);
    ClassDB::bind_method(D_METHOD("get_collide_block"), &BulletTypeC::get_collide_block);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "collide_block"), "set_collide_block", "get_collide_block");

    ClassDB::bind_method(D_METHOD("set_hit_once", "v"), &BulletTypeC::set_hit_once);
    ClassDB::bind_method(D_METHOD("get_hit_once"), &BulletTypeC::get_hit_once);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "hit_once"), "set_hit_once", "get_hit_once");

    ClassDB::bind_method(D_METHOD("set_predict", "v"), &BulletTypeC::set_predict);
    ClassDB::bind_method(D_METHOD("get_predict"), &BulletTypeC::get_predict);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "predict"), "set_predict", "get_predict");

    ClassDB::bind_method(D_METHOD("set_can_move", "v"), &BulletTypeC::set_can_move);
    ClassDB::bind_method(D_METHOD("get_can_move"), &BulletTypeC::get_can_move);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::BOOL, "can_move"), "set_can_move", "get_can_move");

    // Int properties
    ClassDB::bind_method(D_METHOD("set_trail_len", "v"), &BulletTypeC::set_trail_len);
    ClassDB::bind_method(D_METHOD("get_trail_len"), &BulletTypeC::get_trail_len);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "trail_len"), "set_trail_len", "get_trail_len");

    ClassDB::bind_method(D_METHOD("set_pierce_cap", "v"), &BulletTypeC::set_pierce_cap);
    ClassDB::bind_method(D_METHOD("get_pierce_cap"), &BulletTypeC::get_pierce_cap);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "pierce_cap"), "set_pierce_cap", "get_pierce_cap");

    ClassDB::bind_method(D_METHOD("set_ammo_multi", "v"), &BulletTypeC::set_ammo_multi);
    ClassDB::bind_method(D_METHOD("get_ammo_multi"), &BulletTypeC::get_ammo_multi);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "ammo_multi"), "set_ammo_multi", "get_ammo_multi");

    ClassDB::bind_method(D_METHOD("set_shots", "v"), &BulletTypeC::set_shots);
    ClassDB::bind_method(D_METHOD("get_shots"), &BulletTypeC::get_shots);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "shots"), "set_shots", "get_shots");

    ClassDB::bind_method(D_METHOD("set_frag_bullets", "v"), &BulletTypeC::set_frag_bullets);
    ClassDB::bind_method(D_METHOD("get_frag_bullets"), &BulletTypeC::get_frag_bullets);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "frag_bullets"), "set_frag_bullets", "get_frag_bullets");

    ClassDB::bind_method(D_METHOD("set_bullet_kind", "v"), &BulletTypeC::set_bullet_kind);
    ClassDB::bind_method(D_METHOD("get_bullet_kind"), &BulletTypeC::get_bullet_kind);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "bullet_kind", PROPERTY_HINT_ENUM, "Bullet:0,PointBullet:1,LightningBullet:2"), "set_bullet_kind", "get_bullet_kind");
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::INT, "bullet_kind"), "set_bullet_kind", "get_bullet_kind");

    // Float properties
    ClassDB::bind_method(D_METHOD("set_drag", "v"), &BulletTypeC::set_drag);
    ClassDB::bind_method(D_METHOD("get_drag"), &BulletTypeC::get_drag);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "drag"), "set_drag", "get_drag");

    ClassDB::bind_method(D_METHOD("set_drag_percent", "v"), &BulletTypeC::set_drag_percent);
    ClassDB::bind_method(D_METHOD("get_drag_percent"), &BulletTypeC::get_drag_percent);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "drag_percent"), "set_drag_percent", "get_drag_percent");

    ClassDB::bind_method(D_METHOD("set_shake", "v"), &BulletTypeC::set_shake);
    ClassDB::bind_method(D_METHOD("get_shake"), &BulletTypeC::get_shake);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "shake"), "set_shake", "get_shake");

    ClassDB::bind_method(D_METHOD("set_damage", "v"), &BulletTypeC::set_damage);
    ClassDB::bind_method(D_METHOD("get_damage"), &BulletTypeC::get_damage);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "damage"), "set_damage", "get_damage");

    ClassDB::bind_method(D_METHOD("set_splash_range", "v"), &BulletTypeC::set_splash_range);
    ClassDB::bind_method(D_METHOD("get_splash_range"), &BulletTypeC::get_splash_range);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "splash_range"), "set_splash_range", "get_splash_range");

    ClassDB::bind_method(D_METHOD("set_splash_damage", "v"), &BulletTypeC::set_splash_damage);
    ClassDB::bind_method(D_METHOD("get_splash_damage"), &BulletTypeC::get_splash_damage);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "splash_damage"), "set_splash_damage", "get_splash_damage");

    ClassDB::bind_method(D_METHOD("set_splash_knockback", "v"), &BulletTypeC::set_splash_knockback);
    ClassDB::bind_method(D_METHOD("get_splash_knockback"), &BulletTypeC::get_splash_knockback);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "splash_knockback"), "set_splash_knockback", "get_splash_knockback");

    ClassDB::bind_method(D_METHOD("set_cutting_damage", "v"), &BulletTypeC::set_cutting_damage);
    ClassDB::bind_method(D_METHOD("get_cutting_damage"), &BulletTypeC::get_cutting_damage);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "cutting_damage"), "set_cutting_damage", "get_cutting_damage");

    ClassDB::bind_method(D_METHOD("set_armor_pierce", "v"), &BulletTypeC::set_armor_pierce);
    ClassDB::bind_method(D_METHOD("get_armor_pierce"), &BulletTypeC::get_armor_pierce);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "armor_pierce"), "set_armor_pierce", "get_armor_pierce");

    ClassDB::bind_method(D_METHOD("set_speed", "v"), &BulletTypeC::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &BulletTypeC::get_speed);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_speed_scale_min", "v"), &BulletTypeC::set_speed_scale_min);
    ClassDB::bind_method(D_METHOD("get_speed_scale_min"), &BulletTypeC::get_speed_scale_min);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "speed_scale_min"), "set_speed_scale_min", "get_speed_scale_min");

    ClassDB::bind_method(D_METHOD("set_size", "v"), &BulletTypeC::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &BulletTypeC::get_size);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "size"), "set_size", "get_size");

    ClassDB::bind_method(D_METHOD("set_lifetime", "v"), &BulletTypeC::set_lifetime);
    ClassDB::bind_method(D_METHOD("get_lifetime"), &BulletTypeC::get_lifetime);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "lifetime"), "set_lifetime", "get_lifetime");

    ClassDB::bind_method(D_METHOD("set_knockback", "v"), &BulletTypeC::set_knockback);
    ClassDB::bind_method(D_METHOD("get_knockback"), &BulletTypeC::get_knockback);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "knockback"), "set_knockback", "get_knockback");

    ClassDB::bind_method(D_METHOD("set_trail_width", "v"), &BulletTypeC::set_trail_width);
    ClassDB::bind_method(D_METHOD("get_trail_width"), &BulletTypeC::get_trail_width);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "trail_width"), "set_trail_width", "get_trail_width");

    ClassDB::bind_method(D_METHOD("set_homing_power", "v"), &BulletTypeC::set_homing_power);
    ClassDB::bind_method(D_METHOD("get_homing_power"), &BulletTypeC::get_homing_power);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "homing_power"), "set_homing_power", "get_homing_power");

    ClassDB::bind_method(D_METHOD("set_homing_delay", "v"), &BulletTypeC::set_homing_delay);
    ClassDB::bind_method(D_METHOD("get_homing_delay"), &BulletTypeC::get_homing_delay);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "homing_delay"), "set_homing_delay", "get_homing_delay");

    ClassDB::bind_method(D_METHOD("set_homing_range", "v"), &BulletTypeC::set_homing_range);
    ClassDB::bind_method(D_METHOD("get_homing_range"), &BulletTypeC::get_homing_range);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "homing_range"), "set_homing_range", "get_homing_range");

    ClassDB::bind_method(D_METHOD("set_despawn_sound_volume", "v"), &BulletTypeC::set_despawn_sound_volume);
    ClassDB::bind_method(D_METHOD("get_despawn_sound_volume"), &BulletTypeC::get_despawn_sound_volume);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "despawn_sound_volume"), "set_despawn_sound_volume", "get_despawn_sound_volume");

    ClassDB::bind_method(D_METHOD("set_inaccuracy", "v"), &BulletTypeC::set_inaccuracy);
    ClassDB::bind_method(D_METHOD("get_inaccuracy"), &BulletTypeC::get_inaccuracy);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "inaccuracy"), "set_inaccuracy", "get_inaccuracy");

    ClassDB::bind_method(D_METHOD("set_trail_effect_chance", "v"), &BulletTypeC::set_trail_effect_chance);
    ClassDB::bind_method(D_METHOD("get_trail_effect_chance"), &BulletTypeC::get_trail_effect_chance);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "trail_effect_chance"), "set_trail_effect_chance", "get_trail_effect_chance");

    ClassDB::bind_method(D_METHOD("set_trail_effect_chance_to", "v"), &BulletTypeC::set_trail_effect_chance_to);
    ClassDB::bind_method(D_METHOD("get_trail_effect_chance_to"), &BulletTypeC::get_trail_effect_chance_to);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "trail_effect_chance_to"), "set_trail_effect_chance_to", "get_trail_effect_chance_to");

    ClassDB::bind_method(D_METHOD("set_spin", "v"), &BulletTypeC::set_spin);
    ClassDB::bind_method(D_METHOD("get_spin"), &BulletTypeC::get_spin);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "spin"), "set_spin", "get_spin");

    ClassDB::bind_method(D_METHOD("set_rot_offset", "v"), &BulletTypeC::set_rot_offset);
    ClassDB::bind_method(D_METHOD("get_rot_offset"), &BulletTypeC::get_rot_offset);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "rot_offset"), "set_rot_offset", "get_rot_offset");

    ClassDB::bind_method(D_METHOD("set_reload_speed", "v"), &BulletTypeC::set_reload_speed);
    ClassDB::bind_method(D_METHOD("get_reload_speed"), &BulletTypeC::get_reload_speed);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "reload_speed"), "set_reload_speed", "get_reload_speed");

    ClassDB::bind_method(D_METHOD("set_frag_angle", "v"), &BulletTypeC::set_frag_angle);
    ClassDB::bind_method(D_METHOD("get_frag_angle"), &BulletTypeC::get_frag_angle);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "frag_angle"), "set_frag_angle", "get_frag_angle");

    ClassDB::bind_method(D_METHOD("set_frag_angle_random", "v"), &BulletTypeC::set_frag_angle_random);
    ClassDB::bind_method(D_METHOD("get_frag_angle_random"), &BulletTypeC::get_frag_angle_random);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "frag_angle_random"), "set_frag_angle_random", "get_frag_angle_random");

    ClassDB::bind_method(D_METHOD("set_frag_speed_from", "v"), &BulletTypeC::set_frag_speed_from);
    ClassDB::bind_method(D_METHOD("get_frag_speed_from"), &BulletTypeC::get_frag_speed_from);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "frag_speed_from"), "set_frag_speed_from", "get_frag_speed_from");

    ClassDB::bind_method(D_METHOD("set_ground_unit_damage_multi", "v"), &BulletTypeC::set_ground_unit_damage_multi);
    ClassDB::bind_method(D_METHOD("get_ground_unit_damage_multi"), &BulletTypeC::get_ground_unit_damage_multi);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "ground_unit_damage_multi"), "set_ground_unit_damage_multi", "get_ground_unit_damage_multi");

    ClassDB::bind_method(D_METHOD("set_air_unit_damage_multi", "v"), &BulletTypeC::set_air_unit_damage_multi);
    ClassDB::bind_method(D_METHOD("get_air_unit_damage_multi"), &BulletTypeC::get_air_unit_damage_multi);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "air_unit_damage_multi"), "set_air_unit_damage_multi", "get_air_unit_damage_multi");

    ClassDB::bind_method(D_METHOD("set_block_damage_multi", "v"), &BulletTypeC::set_block_damage_multi);
    ClassDB::bind_method(D_METHOD("get_block_damage_multi"), &BulletTypeC::get_block_damage_multi);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "block_damage_multi"), "set_block_damage_multi", "get_block_damage_multi");

    ClassDB::bind_method(D_METHOD("set_recoil", "v"), &BulletTypeC::set_recoil);
    ClassDB::bind_method(D_METHOD("get_recoil"), &BulletTypeC::get_recoil);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "recoil"), "set_recoil", "get_recoil");

    ClassDB::bind_method(D_METHOD("set_crit_chance", "v"), &BulletTypeC::set_crit_chance);
    ClassDB::bind_method(D_METHOD("get_crit_chance"), &BulletTypeC::get_crit_chance);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::FLOAT, "crit_chance"), "set_crit_chance", "get_crit_chance");

    // Vector2 properties
    ClassDB::bind_method(D_METHOD("set_scale_from", "v"), &BulletTypeC::set_scale_from);
    ClassDB::bind_method(D_METHOD("get_scale_from"), &BulletTypeC::get_scale_from);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::VECTOR2, "scale_from"), "set_scale_from", "get_scale_from");

    ClassDB::bind_method(D_METHOD("set_scale_to", "v"), &BulletTypeC::set_scale_to);
    ClassDB::bind_method(D_METHOD("get_scale_to"), &BulletTypeC::get_scale_to);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::VECTOR2, "scale_to"), "set_scale_to", "get_scale_to");

    ClassDB::bind_method(D_METHOD("set_trail_effect_scl_to", "v"), &BulletTypeC::set_trail_effect_scl_to);
    ClassDB::bind_method(D_METHOD("get_trail_effect_scl_to"), &BulletTypeC::get_trail_effect_scl_to);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::VECTOR2, "trail_effect_scl_to"), "set_trail_effect_scl_to", "get_trail_effect_scl_to");

    // Color properties
    ClassDB::bind_method(D_METHOD("set_trail_color_front", "v"), &BulletTypeC::set_trail_color_front);
    ClassDB::bind_method(D_METHOD("get_trail_color_front"), &BulletTypeC::get_trail_color_front);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::COLOR, "trail_color_front"), "set_trail_color_front", "get_trail_color_front");

    ClassDB::bind_method(D_METHOD("set_trail_color_back", "v"), &BulletTypeC::set_trail_color_back);
    ClassDB::bind_method(D_METHOD("get_trail_color_back"), &BulletTypeC::get_trail_color_back);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::COLOR, "trail_color_back"), "set_trail_color_back", "get_trail_color_back");

    ClassDB::bind_method(D_METHOD("set_color", "v"), &BulletTypeC::set_color);
    ClassDB::bind_method(D_METHOD("get_color"), &BulletTypeC::get_color);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

    ClassDB::bind_method(D_METHOD("set_color_to", "v"), &BulletTypeC::set_color_to);
    ClassDB::bind_method(D_METHOD("get_color_to"), &BulletTypeC::get_color_to);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::COLOR, "color_to"), "set_color_to", "get_color_to");

    // Object/Other properties
    ClassDB::bind_method(D_METHOD("set_drawer", "v"), &BulletTypeC::set_drawer);
    ClassDB::bind_method(D_METHOD("get_drawer"), &BulletTypeC::get_drawer);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::CALLABLE, "drawer"), "set_drawer", "get_drawer");

    ClassDB::bind_method(D_METHOD("set_item_type", "v"), &BulletTypeC::set_item_type);
    ClassDB::bind_method(D_METHOD("get_item_type"), &BulletTypeC::get_item_type);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "item_type", PROPERTY_HINT_RESOURCE_TYPE, "ItemType"), "set_item_type", "get_item_type");

    ClassDB::bind_method(D_METHOD("set_despawn_sound", "v"), &BulletTypeC::set_despawn_sound);
    ClassDB::bind_method(D_METHOD("get_despawn_sound"), &BulletTypeC::get_despawn_sound);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "despawn_sound", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_despawn_sound", "get_despawn_sound");

    ClassDB::bind_method(D_METHOD("set_despawn_effect", "v"), &BulletTypeC::set_despawn_effect);
    ClassDB::bind_method(D_METHOD("get_despawn_effect"), &BulletTypeC::get_despawn_effect);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "despawn_effect"), "set_despawn_effect", "get_despawn_effect");

    ClassDB::bind_method(D_METHOD("set_hit_effect", "v"), &BulletTypeC::set_hit_effect);
    ClassDB::bind_method(D_METHOD("get_hit_effect"), &BulletTypeC::get_hit_effect);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "hit_effect"), "set_hit_effect", "get_hit_effect");

    ClassDB::bind_method(D_METHOD("set_trail_effect", "v"), &BulletTypeC::set_trail_effect);
    ClassDB::bind_method(D_METHOD("get_trail_effect"), &BulletTypeC::get_trail_effect);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "trail_effect"), "set_trail_effect", "get_trail_effect");

    ClassDB::bind_method(D_METHOD("set_frag_bullet", "v"), &BulletTypeC::set_frag_bullet);
    ClassDB::bind_method(D_METHOD("get_frag_bullet"), &BulletTypeC::get_frag_bullet);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::OBJECT, "frag_bullet", PROPERTY_HINT_RESOURCE_TYPE, "BulletType"), "set_frag_bullet", "get_frag_bullet");

    ClassDB::bind_method(D_METHOD("set_status", "v"), &BulletTypeC::set_status);
    ClassDB::bind_method(D_METHOD("get_status"), &BulletTypeC::get_status);
    ClassDB::add_property("BulletTypeC", PropertyInfo(Variant::ARRAY, "status"), "set_status", "get_status");

    // Methods
    ClassDB::bind_method(D_METHOD("has_trail"), &BulletTypeC::has_trail);
    ClassDB::bind_method(D_METHOD("get_real_speed"), &BulletTypeC::get_real_speed);
    ClassDB::bind_method(D_METHOD("get_hit_damage", "b"), &BulletTypeC::get_hit_damage);
    ClassDB::bind_method(D_METHOD("draw_introduction_c", "bar"), &BulletTypeC::draw_introduction_c);
    ClassDB::bind_method(D_METHOD("draw_itdct", "bar"), &BulletTypeC::draw_itdct);

    // BulletKind enum constants
    BIND_ENUM_CONSTANT(BULLET_KIND_BULLET);
    BIND_ENUM_CONSTANT(BULLET_KIND_POINT_BULLET);
    BIND_ENUM_CONSTANT(BULLET_KIND_LIGHTNING_BULLET);
}
