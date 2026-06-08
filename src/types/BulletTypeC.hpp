#pragma once

#include "TypeC.hpp"

#include <cmath>

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>

class BulletTypeC : public TypeC {
    GDCLASS(BulletTypeC, TypeC);

public:
    enum BulletKind {
        BULLET_KIND_BULLET = 0,
        BULLET_KIND_POINT_BULLET = 1,
        BULLET_KIND_LIGHTNING_BULLET = 2,
    };

private:
    bool _hidden = false;
    Callable _drawer;
    Variant _item_type;

    double _drag = 0.0;
    double _drag_percent = 1.0;
    double _shake = 0.0;

    double _damage = 0.0;
    double _splash_range = 0.0;
    double _splash_damage = 0.0;
    double _splash_knockback = 0.0;
    bool _splash_hurt_self = false;
    double _cutting_damage = 0.0;
    double _armor_pierce = 0.0;

    double _speed = 150.0;
    double _speed_scale_min = 1.0;

    double _size = 2.0;
    Vector2 _scale_from = Vector2(1, 1);
    Vector2 _scale_to = Vector2(1, 1);

    double _lifetime = 1.0;

    double _knockback = 0.0;

    int64_t _trail_len = 0;
    double _trail_width = 1.0;
    Color _trail_color_front = Color(1, 1, 1);
    Color _trail_color_back = Color(1, 1, 1);

    Color _color = Color(1, 1, 1);
    Color _color_to = Color(1, 1, 1);
    bool _color_gradient = false;

    int64_t _pierce_cap = 0;

    double _homing_power = 0.0;
    double _homing_delay = 0.0;
    double _homing_range = 60.0;

    Variant _despawn_sound;
    double _despawn_sound_volume = 0.0;

    double _inaccuracy = 0.0;

    Variant _despawn_effect;
    Variant _hit_effect;
    Variant _trail_effect;
    double _trail_effect_chance = 0.2;
    double _trail_effect_chance_to = INFINITY;
    Vector2 _trail_effect_scl_to = Vector2(1, 1);

    double _spin = 0.0;
    bool _rand_dir = true;

    Array _status;
    bool _splash_add_status = true;
    bool _hit_add_status = true;

    double _rot_offset = 0.0;

    int64_t _ammo_multi = 1;
    double _reload_speed = 1.0;
    int64_t _shots = 1;

    Variant _frag_bullet;
    int64_t _frag_bullets = 1;
    double _frag_angle = 0.0;
    double _frag_angle_random = Math_PI;
    double _frag_speed_from = 1.0;

    double _ground_unit_damage_multi = 1.0;
    double _air_unit_damage_multi = 1.0;
    double _block_damage_multi = 1.0;

    bool _collide = true;
    bool _collide_unit = true;
    bool _collide_block = true;

    bool _hit_once = false;

    bool _predict = true;
    bool _can_move = true;

    double _recoil = 0.0;
    double _crit_chance = 0.0;

    BulletKind _bullet_kind = BULLET_KIND_BULLET;

protected:
    static void _bind_methods();

public:
    BulletTypeC();

    // Bool getters/setters
    void set_hidden(bool v) { _hidden = v; }
    bool get_hidden() const { return _hidden; }
    void set_splash_hurt_self(bool v) { _splash_hurt_self = v; }
    bool get_splash_hurt_self() const { return _splash_hurt_self; }
    void set_color_gradient(bool v) { _color_gradient = v; }
    bool get_color_gradient() const { return _color_gradient; }
    void set_rand_dir(bool v) { _rand_dir = v; }
    bool get_rand_dir() const { return _rand_dir; }
    void set_splash_add_status(bool v) { _splash_add_status = v; }
    bool get_splash_add_status() const { return _splash_add_status; }
    void set_hit_add_status(bool v) { _hit_add_status = v; }
    bool get_hit_add_status() const { return _hit_add_status; }
    void set_collide(bool v) { _collide = v; }
    bool get_collide() const { return _collide; }
    void set_collide_unit(bool v) { _collide_unit = v; }
    bool get_collide_unit() const { return _collide_unit; }
    void set_collide_block(bool v) { _collide_block = v; }
    bool get_collide_block() const { return _collide_block; }
    void set_hit_once(bool v) { _hit_once = v; }
    bool get_hit_once() const { return _hit_once; }
    void set_predict(bool v) { _predict = v; }
    bool get_predict() const { return _predict; }
    void set_can_move(bool v) { _can_move = v; }
    bool get_can_move() const { return _can_move; }

    // Int getters/setters
    void set_trail_len(int64_t v) { _trail_len = v; }
    int64_t get_trail_len() const { return _trail_len; }
    void set_pierce_cap(int64_t v) { _pierce_cap = v; }
    int64_t get_pierce_cap() const { return _pierce_cap; }
    void set_ammo_multi(int64_t v) { _ammo_multi = v; }
    int64_t get_ammo_multi() const { return _ammo_multi; }
    void set_shots(int64_t v) { _shots = v; }
    int64_t get_shots() const { return _shots; }
    void set_frag_bullets(int64_t v) { _frag_bullets = v; }
    int64_t get_frag_bullets() const { return _frag_bullets; }

    // Float getters/setters
    void set_drag(double v) { _drag = v; }
    double get_drag() const { return _drag; }
    void set_drag_percent(double v) { _drag_percent = v; }
    double get_drag_percent() const { return _drag_percent; }
    void set_shake(double v) { _shake = v; }
    double get_shake() const { return _shake; }
    void set_damage(double v) { _damage = v; }
    double get_damage() const { return _damage; }
    void set_splash_range(double v) { _splash_range = v; }
    double get_splash_range() const { return _splash_range; }
    void set_splash_damage(double v) { _splash_damage = v; }
    double get_splash_damage() const { return _splash_damage; }
    void set_splash_knockback(double v) { _splash_knockback = v; }
    double get_splash_knockback() const { return _splash_knockback; }
    void set_cutting_damage(double v) { _cutting_damage = v; }
    double get_cutting_damage() const { return _cutting_damage; }
    void set_armor_pierce(double v) { _armor_pierce = v; }
    double get_armor_pierce() const { return _armor_pierce; }
    void set_speed(double v) { _speed = v; }
    double get_speed() const { return _speed; }
    void set_speed_scale_min(double v) { _speed_scale_min = v; }
    double get_speed_scale_min() const { return _speed_scale_min; }
    void set_size(double v) { _size = v; }
    double get_size() const { return _size; }
    void set_lifetime(double v) { _lifetime = v; }
    double get_lifetime() const { return _lifetime; }
    void set_knockback(double v) { _knockback = v; }
    double get_knockback() const { return _knockback; }
    void set_trail_width(double v) { _trail_width = v; }
    double get_trail_width() const { return _trail_width; }
    void set_homing_power(double v) { _homing_power = v; }
    double get_homing_power() const { return _homing_power; }
    void set_homing_delay(double v) { _homing_delay = v; }
    double get_homing_delay() const { return _homing_delay; }
    void set_homing_range(double v) { _homing_range = v; }
    double get_homing_range() const { return _homing_range; }
    void set_despawn_sound_volume(double v) { _despawn_sound_volume = v; }
    double get_despawn_sound_volume() const { return _despawn_sound_volume; }
    void set_inaccuracy(double v) { _inaccuracy = v; }
    double get_inaccuracy() const { return _inaccuracy; }
    void set_trail_effect_chance(double v) { _trail_effect_chance = v; }
    double get_trail_effect_chance() const { return _trail_effect_chance; }
    void set_trail_effect_chance_to(double v) { _trail_effect_chance_to = v; }
    double get_trail_effect_chance_to() const { return _trail_effect_chance_to; }
    void set_spin(double v) { _spin = v; }
    double get_spin() const { return _spin; }
    void set_rot_offset(double v) { _rot_offset = v; }
    double get_rot_offset() const { return _rot_offset; }
    void set_reload_speed(double v) { _reload_speed = v; }
    double get_reload_speed() const { return _reload_speed; }
    void set_frag_angle(double v) { _frag_angle = v; }
    double get_frag_angle() const { return _frag_angle; }
    void set_frag_angle_random(double v) { _frag_angle_random = v; }
    double get_frag_angle_random() const { return _frag_angle_random; }
    void set_frag_speed_from(double v) { _frag_speed_from = v; }
    double get_frag_speed_from() const { return _frag_speed_from; }
    void set_ground_unit_damage_multi(double v) { _ground_unit_damage_multi = v; }
    double get_ground_unit_damage_multi() const { return _ground_unit_damage_multi; }
    void set_air_unit_damage_multi(double v) { _air_unit_damage_multi = v; }
    double get_air_unit_damage_multi() const { return _air_unit_damage_multi; }
    void set_block_damage_multi(double v) { _block_damage_multi = v; }
    double get_block_damage_multi() const { return _block_damage_multi; }
    void set_recoil(double v) { _recoil = v; }
    double get_recoil() const { return _recoil; }
    void set_crit_chance(double v) { _crit_chance = v; }
    double get_crit_chance() const { return _crit_chance; }

    // BulletKind getter/setter
    void set_bullet_kind(BulletKind v) { _bullet_kind = v; }
    BulletKind get_bullet_kind() const { return _bullet_kind; }

    // Vector2 getters/setters
    void set_scale_from(Vector2 v) { _scale_from = v; }
    Vector2 get_scale_from() const { return _scale_from; }
    void set_scale_to(Vector2 v) { _scale_to = v; }
    Vector2 get_scale_to() const { return _scale_to; }
    void set_trail_effect_scl_to(Vector2 v) { _trail_effect_scl_to = v; }
    Vector2 get_trail_effect_scl_to() const { return _trail_effect_scl_to; }

    // Color getters/setters
    void set_trail_color_front(Color v) { _trail_color_front = v; }
    Color get_trail_color_front() const { return _trail_color_front; }
    void set_trail_color_back(Color v) { _trail_color_back = v; }
    Color get_trail_color_back() const { return _trail_color_back; }
    void set_color(Color v) { _color = v; }
    Color get_color() const { return _color; }
    void set_color_to(Color v) { _color_to = v; }
    Color get_color_to() const { return _color_to; }

    // Object/Other getters/setters
    void set_drawer(Callable v) { _drawer = v; }
    Callable get_drawer() const { return _drawer; }
    void set_item_type(Variant v) { _item_type = v; }
    Variant get_item_type() const { return _item_type; }
    void set_despawn_sound(Variant v) { _despawn_sound = v; }
    Variant get_despawn_sound() const { return _despawn_sound; }
    void set_despawn_effect(Variant v) { _despawn_effect = v; }
    Variant get_despawn_effect() const { return _despawn_effect; }
    void set_hit_effect(Variant v) { _hit_effect = v; }
    Variant get_hit_effect() const { return _hit_effect; }
    void set_trail_effect(Variant v) { _trail_effect = v; }
    Variant get_trail_effect() const { return _trail_effect; }
    void set_frag_bullet(Variant v) { _frag_bullet = v; }
    Variant get_frag_bullet() const { return _frag_bullet; }
    void set_status(Array v) { _status = v; }
    Array get_status() const { return _status; }

    // Methods
    bool has_trail();
    float get_real_speed();
    float get_hit_damage(Object *b);
    void draw_introduction_c(Object *bar) override;
    void draw_itdct(Object *bar);
};

VARIANT_ENUM_CAST(BulletTypeC::BulletKind);
