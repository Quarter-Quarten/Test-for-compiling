#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

namespace godot {

class ConstsC : public Node {
    GDCLASS(ConstsC, Node);

private:
    static void _bind_methods();

public:
    static String get_save_path();
    static String get_auto_save_path();
    static String get_setting_path();
    static String get_blueprint_path();
    static String get_internal_blueprint_path();
    static String get_uid_path();
    static String get_wall_description();
    static String get_door_description();
    static String get_blueprint_suffix();

    static int get_tile_set_separation();
    static int get_ticks_per_second();
    static real_t get_tick_time();

    static Vector2i get_tile_size_i();
    static Vector2 get_tile_size();
    static Vector2i get_half_tile_size_i();
    static Vector2 get_half_tile_size();
    static Vector2i get_region_size();
    static int get_region_tile_count();
    static Vector2 get_real_region_size();
    static Vector2 get_hrrs();
    static Vector2 get_half_real_region_size();
    static Vector2i get_big_region_size();
    static Vector2i get_big_region_tile_size();
    static Vector2 get_big_region_real_size();
    static Vector2 get_half_big_region_real_size();

    static int get_trial_rect_z();
    static int get_blood_warning_z();
    static int get_block_map_z();
    static int get_block_z();
    static int get_unit_z();
    static int get_flying_z();
    static int get_item_z();
    static int get_sampler_z();
    static int get_light_layer();
    static int get_bullet_z();
    static int get_progress_bar_z();
    static int get_fx_z();
    static int get_label_z();
    static int get_select_z();
    static int get_conversation_z();

    static int get_player_team();
    static int get_enemy_team();
    static int get_structure_team();

    static double get_sound_range();
    static double get_shake_range_min();
    static double get_ninty_deg();
    static int get_port();
    static Vector2 get_normal_scale();
    static Array get_directions4_i();
    static Array get_directions8_i();
    static double get_difficulty_min();
    static double get_difficulty_max();
    static double get_difficulty_step();
    static int get_mtu();
    static Callable get_empty_func();
};

} // namespace godot
