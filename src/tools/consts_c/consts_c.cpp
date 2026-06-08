#include "tools/consts_c/consts_c.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void ConstsC::_bind_methods() {
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_save_path"), &ConstsC::get_save_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_auto_save_path"), &ConstsC::get_auto_save_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_setting_path"), &ConstsC::get_setting_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_blueprint_path"), &ConstsC::get_blueprint_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_internal_blueprint_path"), &ConstsC::get_internal_blueprint_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_uid_path"), &ConstsC::get_uid_path);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_tile_size_i"), &ConstsC::get_tile_size_i);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_tile_size"), &ConstsC::get_tile_size);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_region_size"), &ConstsC::get_region_size);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_directions4_i"), &ConstsC::get_directions4_i);
    ClassDB::bind_static_method("ConstsC", D_METHOD("get_light_layer"), &ConstsC::get_light_layer);
}

String ConstsC::get_save_path() { return "user://saves"; }
String ConstsC::get_auto_save_path() { return "user://auto_save"; }
String ConstsC::get_setting_path() { return "user://userSetting.bin"; }
String ConstsC::get_blueprint_path() { return "res://blueprints"; }
String ConstsC::get_internal_blueprint_path() { return "res://internalBlueprints"; }
String ConstsC::get_uid_path() { return "user://uid.bin"; }
String ConstsC::get_wall_description() { return "阻挡单位和子弹"; }
String ConstsC::get_door_description() { return "可开关"; }
String ConstsC::get_blueprint_suffix() { return ".blp"; }

int ConstsC::get_tile_set_separation() { return 0; }
int ConstsC::get_ticks_per_second() { return 30; }
real_t ConstsC::get_tick_time() { return 1.0 / 30.0; }

Vector2i ConstsC::get_tile_size_i() { return Vector2i(16, 16); }
Vector2 ConstsC::get_tile_size() { return Vector2(16, 16); }
Vector2i ConstsC::get_half_tile_size_i() { return Vector2i(8, 8); }
Vector2 ConstsC::get_half_tile_size() { return Vector2(8, 8); }
Vector2i ConstsC::get_region_size() { return Vector2i(4, 4); }
int ConstsC::get_region_tile_count() { return 16; }
Vector2 ConstsC::get_real_region_size() { return Vector2(64, 64); }
Vector2 ConstsC::get_hrrs() { return Vector2(32, 32); }
Vector2 ConstsC::get_half_real_region_size() { return Vector2(32, 32); }
Vector2i ConstsC::get_big_region_size() { return Vector2i(4, 4); }
Vector2i ConstsC::get_big_region_tile_size() { return Vector2i(16, 16); }
Vector2 ConstsC::get_big_region_real_size() { return Vector2(256, 256); }
Vector2 ConstsC::get_half_big_region_real_size() { return Vector2(128, 128); }

int ConstsC::get_trial_rect_z() { return -20; }
int ConstsC::get_blood_warning_z() { return -10; }
int ConstsC::get_block_map_z() { return -1; }
int ConstsC::get_block_z() { return 0; }
int ConstsC::get_unit_z() { return 60; }
int ConstsC::get_flying_z() { return 90; }
int ConstsC::get_item_z() { return 100; }
int ConstsC::get_sampler_z() { return 130; }
int ConstsC::get_light_layer() { return 120; }
int ConstsC::get_bullet_z() { return 130; }
int ConstsC::get_progress_bar_z() { return 130; }
int ConstsC::get_fx_z() { return 150; }
int ConstsC::get_label_z() { return 200; }
int ConstsC::get_select_z() { return 500; }
int ConstsC::get_conversation_z() { return 600; }

int ConstsC::get_player_team() { return 1; }
int ConstsC::get_enemy_team() { return 2; }
int ConstsC::get_structure_team() { return 3; }

double ConstsC::get_sound_range() { return 640.0; }
double ConstsC::get_shake_range_min() { return 64.0; }
double ConstsC::get_ninty_deg() { return Math::deg_to_rad(90.0); }
int ConstsC::get_port() { return 11454; }
Vector2 ConstsC::get_normal_scale() { return Vector2(1, 1); }

Array ConstsC::get_directions4_i() {
    Array arr;
    arr.append(Vector2i(-1, 0));
    arr.append(Vector2i(1, 0));
    arr.append(Vector2i(0, -1));
    arr.append(Vector2i(0, 1));
    return arr;
}

Array ConstsC::get_directions8_i() {
    Array arr = get_directions4_i();
    arr.append(Vector2i(1, 1));
    arr.append(Vector2i(1, -1));
    arr.append(Vector2i(-1, 1));
    arr.append(Vector2i(-1, -1));
    return arr;
}

double ConstsC::get_difficulty_min() { return 1.0; }
double ConstsC::get_difficulty_max() { return 2.5; }
double ConstsC::get_difficulty_step() { return 0.5; }
int ConstsC::get_mtu() { return 1392; }
Callable ConstsC::get_empty_func() { return Callable(); }
