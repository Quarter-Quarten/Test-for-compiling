#pragma once

#include "TypeC.hpp"

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

class UnitTypeC : public TypeC {
    GDCLASS(UnitTypeC, TypeC);
public:
    enum UnitKind {
        UNIT_KIND_UNIT = 0,
        UNIT_KIND_PLAYER = 1,
        UNIT_KIND_ENEMY = 2,
        UNIT_KIND_NPC = 3,
        UNIT_KIND_SUMMONED = 4,
    };

private:
    UnitKind _unit_kind = UNIT_KIND_UNIT;

    bool _draw_health_prog = true;
    bool _flying = false;
    bool _collide_unit = true;
    bool _draw_target_pos = false;
    bool _heal_unit = false;
    bool _is_interactive = false;

    double _damaged_invincible_time = 0.0;
    double _height = 1.0;
    double _heal_amount = 0.0;
    double _flash_time = 0.25;
    double _rotate_speed = 10.0;
    double _armor = 0.0;
    double _speed = 1.0;
    double _health = 10.0;
    double _accel = 7.5;
    double _drag = 7.5;
    double _size = 12.0;
    double _kbrst = 12.0;
    double _despawn_sound_volume = -1.0;
    double _heal_other_amount = 0.0;
    double _hit_sound_volumn = -8.0;

    Variant _ai;
    Variant _sprite;
    Variant _despawn_effect;
    Variant _despawn_sound;
    Variant _turret;
    Variant _light;
    Variant _hit_sound;

    Dictionary _spawn_units_on_death;

    Callable _on_shooting;

    Array _att_progs;

protected:
    static void _bind_methods();

public:
    UnitTypeC();

    // Bool getters/setters
    void set_draw_health_prog(bool v) { _draw_health_prog = v; }
    bool get_draw_health_prog() const { return _draw_health_prog; }
    void set_flying(bool v) { _flying = v; }
    bool get_flying() const { return _flying; }
    void set_collide_unit(bool v) { _collide_unit = v; }
    bool get_collide_unit() const { return _collide_unit; }
    void set_draw_target_pos(bool v) { _draw_target_pos = v; }
    bool get_draw_target_pos() const { return _draw_target_pos; }
    void set_heal_unit(bool v) { _heal_unit = v; }
    bool get_heal_unit() const { return _heal_unit; }
    void set_is_interactive(bool v) { _is_interactive = v; }
    bool get_is_interactive() const { return _is_interactive; }

    // Float getters/setters
    void set_damaged_invincible_time(double v) { _damaged_invincible_time = v; }
    double get_damaged_invincible_time() const { return _damaged_invincible_time; }
    void set_height(double v) { _height = v; }
    double get_height() const { return _height; }
    void set_heal_amount(double v) { _heal_amount = v; }
    double get_heal_amount() const { return _heal_amount; }
    void set_flash_time(double v) { _flash_time = v; }
    double get_flash_time() const { return _flash_time; }
    void set_rotate_speed(double v) { _rotate_speed = v; }
    double get_rotate_speed() const { return _rotate_speed; }
    void set_armor(double v) { _armor = v; }
    double get_armor() const { return _armor; }
    void set_speed(double v) { _speed = v; }
    double get_speed() const { return _speed; }
    void set_health(double v) { _health = v; }
    double get_health() const { return _health; }
    void set_accel(double v) { _accel = v; }
    double get_accel() const { return _accel; }
    void set_drag(double v) { _drag = v; }
    double get_drag() const { return _drag; }
    void set_size(double v) { _size = v; }
    double get_size() const { return _size; }
    void set_kbrst(double v) { _kbrst = v; }
    double get_kbrst() const { return _kbrst; }
    void set_despawn_sound_volume(double v) { _despawn_sound_volume = v; }
    double get_despawn_sound_volume() const { return _despawn_sound_volume; }
    void set_heal_other_amount(double v) { _heal_other_amount = v; }
    double get_heal_other_amount() const { return _heal_other_amount; }
    void set_hit_sound_volumn(double v) { _hit_sound_volumn = v; }
    double get_hit_sound_volumn() const { return _hit_sound_volumn; }

    // Variant/Object getters/setters
    void set_ai(Variant v) { _ai = v; }
    Variant get_ai() const { return _ai; }
    void set_sprite(Variant v) { _sprite = v; }
    Variant get_sprite() const { return _sprite; }
    void set_despawn_effect(Variant v) { _despawn_effect = v; }
    Variant get_despawn_effect() const { return _despawn_effect; }
    void set_despawn_sound(Variant v) { _despawn_sound = v; }
    Variant get_despawn_sound() const { return _despawn_sound; }
    void set_turret(Variant v) { _turret = v; }
    Variant get_turret() const { return _turret; }
    void set_light(Variant v) { _light = v; }
    Variant get_light() const { return _light; }
    void set_hit_sound(Variant v) { _hit_sound = v; }
    Variant get_hit_sound() const { return _hit_sound; }
    void set_spawn_units_on_death(Dictionary v) { _spawn_units_on_death = v; }
    Dictionary get_spawn_units_on_death() const { return _spawn_units_on_death; }

    // Callable getter/setter
    void set_on_shooting(Callable v) { _on_shooting = v; }
    Callable get_on_shooting() const { return _on_shooting; }

    // Array getter/setter
    void set_att_progs(Array v) { _att_progs = v; }
    Array get_att_progs() const { return _att_progs; }

    // UnitKind getter/setter
    void set_unit_kind(UnitKind v) { _unit_kind = v; }
    UnitKind get_unit_kind() const { return _unit_kind; }

    // === 由简至繁：移植自 UnitType.gd 的更新逻辑 ===

    // Step 1: 简单辅助函数
    bool is_slime_c();
    void slime_c();
    void fly_c(double h);
    double get_max_speed_c(Object* u);
    double damage_handle_c(Object* u, double damage, double armor_pierce);
    double get_real_armor_c(Object* u);

    // Step 2: 简单更新
    void update_heal_c(Object* u, double delta);
    void update_attributes_c(Object* u, double delta);
    void update_data_c(Object* u);

    // Step 3: 移动物理
    void update_move_c(Object* u, double delta);

    // Step 4: 状态效果
    bool update_status_c(Object* u, double delta);

    // Step 5: 方块碰撞
    void update_collide_block_c(Object* u, double delta);

    // Step 6: 主更新入口
    bool update_c(Object* u, double delta);
    bool tick_update_c(Object* u, double delta);
};

VARIANT_ENUM_CAST(UnitTypeC::UnitKind);