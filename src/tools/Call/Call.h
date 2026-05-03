#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <tools/ConstsC/ConstsC.h>

namespace godot {

class Call : public Object {
    GDCLASS(Call, Object);

private:
    static Object* vars;

    static void _bind_methods();
    static Object *_vars();

public:
    static int x_to_tile(double x);
    static int y_to_tile(double y);
    static Vector2i pos_to_tile(Vector2 pos);
    static Vector2 tile_to_pos(Vector2i tile_pos);
    static Vector2i tile_to_region(Vector2i tile_pos);
    static Vector2i region_to_tile(Vector2i region);
    static Vector2i pos_to_region(Vector2 pos);
    static Vector2 region_to_pos(Vector2i region);
    static Rect2 tile_rect_to_global(Rect2i rect);
    static Rect2i global_rect_to_tile(Rect2 rect);
    static Vector2i pos_to_big_region(Vector2 pos);
    static Vector2i tile_to_big_region(Vector2i tile_pos);
    static Vector2i big_region_to_region(Vector2i region);
    static Vector2 region_center(Vector2i region);
    static Vector2 big_region_center(Vector2i region);
    static Rect2 get_tile_rect(Vector2i tile);
    static Rect2 get_region_rect(Vector2i region);
    static Rect2 get_big_region_rect(Vector2i region);
    static Rect2i get_region_tile_rect(Vector2i region);
    static Rect2i get_big_region_tile_rect(Vector2i region);
    static Rect2i get_circle_tile_rect(Vector2 center, double radius);
    static Rect2 get_circle_aabb(Vector2 center, double radius);
    static Rect2i get_line_tile_rect(Vector2 from, Vector2 to);

    static bool clear_block(Vector2i tile, bool kill = true, bool direct_write = false);
    static void set_block_data(Vector2i tile, PackedByteArray data);

    static bool region_tiles(Vector2i r, Callable method);
    static bool big_region_regions(Vector2i r, Callable method);
    static bool circle_tiles(Vector2 pos, double radius, Callable method);
    static bool circle_collision(Vector2 pos, double radius, Callable method);
    static bool circle_regions(Vector2 pos, double radius, Callable method);
    static bool circle_big_regions(Vector2 pos, double radius, Callable method);
    static void rect_tiles(Vector2 pos, double width, double height, Callable method);
    static void rect_collision(Vector2 pos, double width, double height, Callable method);
    static void rect_regions(Vector2 pos, double width, double height, Callable method);
    static void rect_big_regions(Vector2 pos, double width, double height, Callable method);
    static void rect2i_outer(int l, int t, int size_x, int size_y, int width, Callable method, bool ignore_vertex = false);
    static bool ring_tiles(Vector2 pos, double radius_from, double radius, Callable method);
    static bool ring_collision(Vector2 pos, double radius_from, double radius, Callable method);
    static bool ring_blocks(Vector2 pos, double radius_from, double radius, Callable method);
    static bool ring_regions(Vector2 pos, double radius_from, double radius, Callable method);
    static bool ring_big_regions(Vector2 pos, double radius_from, double radius, Callable method);
    static double angle_range_transport(double angle);
    static bool line_chunks(Vector2 from, Vector2 to, double width, Vector2 chunk_size, Callable method);
    static bool line_chunks_without_width(Vector2 from, Vector2 to, Vector2 chunk_size, Callable method);
    static void iterate_points_in_r1_not_r2(Rect2i r1, Rect2i r2, Callable callback);
    static bool rect_circle_intersects(Rect2 rect, double radius, Vector2 pos);
    static bool rect_ring_intersects(Rect2 rect, double radius_from, double radius, Vector2 pos);
    static bool l_c_handle(Vector2 from, Vector2 to, double width, double radius, Vector2 pos);
    static bool point_s_handle(Vector2 point, double radius, Vector2 pos, double dir, double half_angle);
    static Vector2 screen_to_global(Vector2 screen_pos);
    static Vector2 global_to_screen(Vector2 global_pos);
    static Transform2D get_transform();
    static Vector2 screen_left_up();
    static Vector2 screen_global_size();
    static Vector2i screen_first_tile();
    static Vector2i screen_first_region();
    static Rect2 screen_to_global_rect();
    static Rect2i screen_tile_rect();
    static Rect2i screen_region_rect();
    static double get_lightness(Color c);
    static void ff();
    static void time_start();
    static void time_print(String extra = "", bool restart = true);
};

} // namespace godot
