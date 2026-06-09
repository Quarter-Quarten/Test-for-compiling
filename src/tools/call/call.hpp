#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <tools/consts_c/consts_c.hpp>

namespace godot {

class Call : public Object {
    GDCLASS(Call, Object);

private:
    static inline Node2D* vars = nullptr;
    static inline Node2D* world = nullptr;
    static inline Ref<Resource> setting;

    static void _bind_methods();
    static Node2D* _vars();
    static Node2D* _world();
    static Resource* _setting();

public:
    static Node2D* get_vars();
    static Node2D* get_world();
    static Resource* get_setting();

    static int x_to_tile(double x);
    static int y_to_tile(double y);
    static Vector2i pos_to_tile(Vector2 pos);
    static Vector2 tile_to_pos(Vector2i tile_pos);
    static Vector2i tile_to_region(Vector2i tile_pos);
    static Vector2i region_to_tile(Vector2i region);
    static Vector2i pos_to_region(Vector2 pos);
    static Vector2 region_to_pos(Vector2i region);
    static Vector2i pos_to_big_region(Vector2 pos);
    static Vector2i tile_to_big_region(Vector2i tile_pos);
    static Vector2i big_region_to_region(Vector2i region);
    static Vector2 region_center(Vector2i region);
    static Vector2 big_region_center(Vector2i region);
    static Rect2 tile_rect_to_global(Rect2i rect);
    static Rect2i global_rect_to_tile(Rect2 rect);
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

    // --- Iteration: lambda-friendly template + Callable overload ---

    template<typename F>
    static bool region_tiles(Vector2i r, F method) {
        Vector2i first = region_to_tile(r);
        Vector2i rs = ConstsC::get_region_size();
        for (int y = first.y; y < first.y + rs.y; y++)
            for (int x = first.x; x < first.x + rs.x; x++)
                if (method(Vector2i(x, y))) return true;
        return false;
    }
    static bool region_tiles(Vector2i r, Callable method);

    template<typename F>
    static bool big_region_regions(Vector2i r, F method) {
        Vector2i first = big_region_to_region(r);
        Vector2i bs = ConstsC::get_big_region_size();
        for (int y = first.y; y < first.y + bs.y; y++)
            for (int x = first.x; x < first.x + bs.x; x++)
                if (method(Vector2i(x, y))) return true;
        return false;
    }
    static bool big_region_regions(Vector2i r, Callable method);

    template<typename F>
    static bool circle_tiles(Vector2 pos, double radius, F method) {
        Vector2i f = pos_to_tile(pos - Vector2(radius, radius));
        Vector2 hs = ConstsC::get_half_tile_size();
        int hx = Math::ceil(radius / hs.x) + 1;
        int hy = Math::ceil(radius / hs.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i tile = f + Vector2i(x, y);
                if (rect_circle_intersects(get_tile_rect(tile), radius, pos) && method(tile))
                    return true;
            }
        return false;
    }
    static bool circle_tiles(Vector2 pos, double radius, Callable method);

    template<typename F>
    static bool circle_collision(Vector2 pos, double radius, F method) {
        Object* v = _vars();
        if (!v) return false;
        Vector2i f = pos_to_tile(pos - Vector2(radius, radius));
        int size = Math::ceil(radius / ConstsC::get_half_tile_size().x) + 1;
        Array collided;
        Dictionary inst = v->get("block_instances");
        for (int y = f.y; y < f.y + size; y++)
            for (int x = f.x; x < f.x + size; x++) {
                Vector2i tile(x, y);
                Object* b = Object::cast_to<Object>(inst.get(tile, Variant()));
                if (b && !collided.has(b)) {
                    collided.append(b);
                    if (rect_circle_intersects(b->call("get_collision_rect"), radius, pos) && method(b))
                        return true;
                }
            }
        return false;
    }
    static bool circle_collision(Vector2 pos, double radius, Callable method);

    template<typename F>
    static bool circle_regions(Vector2 pos, double radius, F method) {
        Vector2i f = pos_to_region(pos - Vector2(radius, radius));
        Vector2 h = ConstsC::get_hrrs();
        int hx = Math::ceil(radius / h.x) + 1;
        int hy = Math::ceil(radius / h.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i r = f + Vector2i(x, y);
                if (rect_circle_intersects(get_region_rect(r), radius, pos) && method(r))
                    return true;
            }
        return false;
    }
    static bool circle_regions(Vector2 pos, double radius, Callable method);

    template<typename F>
    static bool circle_big_regions(Vector2 pos, double radius, F method) {
        Vector2i f = pos_to_big_region(pos - Vector2(radius, radius));
        Vector2 h = ConstsC::get_half_big_region_real_size();
        int hx = Math::ceil(radius / h.x) + 1;
        int hy = Math::ceil(radius / h.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i r = f + Vector2i(x, y);
                if (rect_circle_intersects(get_big_region_rect(r), radius, pos) && method(r))
                    return true;
            }
        return false;
    }
    static bool circle_big_regions(Vector2 pos, double radius, Callable method);

    template<typename F>
    static void rect_tiles(Vector2 pos, double width, double height, F method) {
        Vector2 tl = pos - Vector2(width * 0.5, height * 0.5);
        Vector2i st = pos_to_tile(tl);
        Vector2 ts = ConstsC::get_tile_size();
        int xc = Math::ceil(width / ts.x) + 1;
        int yc = Math::ceil(height / ts.y) + 1;
        for (int y = 0; y < yc; y++)
            for (int x = 0; x < xc; x++)
                method(st + Vector2i(x, y));
    }
    static void rect_tiles(Vector2 pos, double width, double height, Callable method);

    template<typename F>
    static void rect_collision(Vector2 pos, double width, double height, F method) {
        Object* v = _vars();
        if (!v) return;
        Rect2 rect(pos.x - width * 0.5, pos.y - height * 0.5, width, height);
        Vector2i st = pos_to_tile(rect.position);
        Vector2 ts = ConstsC::get_tile_size();
        int xc = Math::ceil(width / ts.x) + 1;
        int yc = Math::ceil(height / ts.y) + 1;
        Array collided;
        Dictionary inst = v->get("block_instances");
        for (int y = 0; y < yc; y++)
            for (int x = 0; x < xc; x++) {
                Object* b = Object::cast_to<Object>(inst.get(st + Vector2i(x, y), Variant()));
                if (b && !collided.has(b) && Rect2(b->call("get_collision_rect")).intersects(rect)) {
                    collided.append(b);
                    method(b);
                }
            }
    }
    static void rect_collision(Vector2 pos, double width, double height, Callable method);

    template<typename F>
    static void rect_regions(Vector2 pos, double width, double height, F method) {
        Vector2 tl = pos - Vector2(width * 0.5, height * 0.5);
        Vector2i st = pos_to_region(tl);
        Vector2 h = ConstsC::get_hrrs();
        int xc = Math::ceil(width / h.x) + 1;
        int yc = Math::ceil(height / h.y) + 1;
        for (int y = 0; y < yc; y++)
            for (int x = 0; x < xc; x++)
                method(st + Vector2i(x, y));
    }
    static void rect_regions(Vector2 pos, double width, double height, Callable method);

    template<typename F>
    static void rect_big_regions(Vector2 pos, double width, double height, F method) {
        Vector2 tl = pos - Vector2(width * 0.5, height * 0.5);
        Vector2i st = pos_to_big_region(tl);
        Vector2 h = ConstsC::get_half_big_region_real_size();
        int xc = Math::ceil(width / h.x) + 1;
        int yc = Math::ceil(height / h.y) + 1;
        for (int y = 0; y < yc; y++)
            for (int x = 0; x < xc; x++)
                method(st + Vector2i(x, y));
    }
    static void rect_big_regions(Vector2 pos, double width, double height, Callable method);

    template<typename F>
    static void rect2i_outer(int l, int t, int size_x, int size_y, int width, F method, bool ignore_vertex = false) {
        int ign = int(ignore_vertex);
        int r = l + size_x - 1, b = t + size_y - 1;
        int left = l - width, right = r + width;
        int top = t - width, bottom = b + width;
        for (int w = 0; w < width; w++) {
            int cl = left + w, cr = right - w;
            int ct = top + w, cb = bottom - w;
            for (int x = cl + ign; x <= cr - ign; x++) {
                method(Vector2i(x, ct));
                method(Vector2i(x, cb));
            }
            for (int y = ct + 1; y < cb; y++) {
                method(Vector2i(cl, y));
                method(Vector2i(cr, y));
            }
        }
    }
    static void rect2i_outer(int l, int t, int size_x, int size_y, int width, Callable method, bool ignore_vertex = false);

    template<typename F>
    static bool ring_tiles(Vector2 pos, double rf, double r, F method) {
        Vector2i f = pos_to_tile(pos - Vector2(r, r));
        Vector2 hs = ConstsC::get_half_tile_size();
        int hx = Math::ceil(r / hs.x) + 1;
        int hy = Math::ceil(r / hs.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i t = f + Vector2i(x, y);
                if (rect_ring_intersects(get_tile_rect(t), rf, r, pos) && method(t))
                    return true;
            }
        return false;
    }
    static bool ring_tiles(Vector2 pos, double radius_from, double radius, Callable method);

    template<typename F>
    static bool ring_collision(Vector2 pos, double rf, double r, F method) {
        Object* v = _vars();
        if (!v) return false;
        Vector2i f = pos_to_tile(pos - Vector2(r, r));
        Vector2 hs = ConstsC::get_half_tile_size();
        Array collided;
        Dictionary inst = v->get("block_instances");
        int hx = Math::ceil(r / hs.x) + 1;
        int hy = Math::ceil(r / hs.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i t = f + Vector2i(x, y);
                Object* b = Object::cast_to<Object>(inst.get(t, Variant()));
                if (b && !collided.has(b)) {
                    collided.append(b);
                    if (rect_ring_intersects(b->call("get_collision_rect"), rf, r, pos) && method(b))
                        return true;
                }
            }
        return false;
    }
    static bool ring_collision(Vector2 pos, double radius_from, double radius, Callable method);

    template<typename F>
    static bool ring_blocks(Vector2 pos, double rf, double r, F method) {
        Object* v = _vars();
        if (!v) return false;
        Vector2i f = pos_to_tile(pos - Vector2(r, r));
        Vector2 hs = ConstsC::get_half_tile_size();
        Dictionary inst = v->get("block_instances");
        int hx = Math::ceil(r / hs.x) + 1;
        int hy = Math::ceil(r / hs.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i t = f + Vector2i(x, y);
                Object* b = Object::cast_to<Object>(inst.get(t, Variant()));
                if (b && Vector2i(b->get("pos")) == t) {
                    if (rect_ring_intersects(b->call("get_collision_rect"), rf, r, pos) && method(b))
                        return true;
                }
            }
        return false;
    }
    static bool ring_blocks(Vector2 pos, double radius_from, double radius, Callable method);

    template<typename F>
    static bool ring_regions(Vector2 pos, double rf, double r, F method) {
        Vector2i f = pos_to_region(pos - Vector2(r, r));
        Vector2 h = ConstsC::get_hrrs();
        int hx = Math::ceil(r / h.x) + 1;
        int hy = Math::ceil(r / h.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i rg = f + Vector2i(x, y);
                if (rect_ring_intersects(get_region_rect(rg), rf, r, pos) && method(rg))
                    return true;
            }
        return false;
    }
    static bool ring_regions(Vector2 pos, double radius_from, double radius, Callable method);

    template<typename F>
    static bool ring_big_regions(Vector2 pos, double rf, double r, F method) {
        Vector2i f = pos_to_big_region(pos - Vector2(r, r));
        Vector2 h = ConstsC::get_half_big_region_real_size();
        int hx = Math::ceil(r / h.x) + 1;
        int hy = Math::ceil(r / h.y) + 1;
        for (int y = 0; y < hy; y++)
            for (int x = 0; x < hx; x++) {
                Vector2i rg = f + Vector2i(x, y);
                if (rect_ring_intersects(get_big_region_rect(rg), rf, r, pos) && method(rg))
                    return true;
            }
        return false;
    }
    static bool ring_big_regions(Vector2 pos, double radius_from, double radius, Callable method);

    static double angle_range_transport(double angle);

    template<typename F>
    static bool line_chunks(Vector2 from, Vector2 to, double width, Vector2 chunk_size, F method) {
        PackedVector2Array handled;
        auto call_unique = [&](Vector2i tile) -> bool {
            if (!handled.has(tile)) {
                handled.push_back(tile);
                return bool(method(tile));
            }
            return false;
        };
        double angle = (to - from).angle();
        Vector2 o1 = Vector2::from_angle(angle + Math_PI / 2.0) * width / 2.0;
        Vector2 o2 = Vector2::from_angle(angle - Math_PI / 2.0) * width / 2.0;

        Vector2 from1 = from + o1;
        Vector2 to1 = to + o1;
        Vector2i ft1(int(Math::floor(from1.x / chunk_size.x)), int(Math::floor(from1.y / chunk_size.y)));
        Vector2i tt1(int(Math::floor(to1.x / chunk_size.x)), int(Math::floor(to1.y / chunk_size.y)));
        int x0 = ft1.x, y0 = ft1.y, x1 = tt1.x, y1 = tt1.y;
        int dx = Math::abs(x1 - x0), dy = Math::abs(y1 - y0);
        int sx = x0 > x1 ? -1 : 1, sy = y0 > y1 ? -1 : 1, err = dx - dy;
        while (true) {
            if (call_unique(Vector2i(x0, y0))) return true;
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }

        Vector2 from2 = from + o2;
        Vector2 to2 = to + o2;
        Vector2i ft2(int(Math::floor(from2.x / chunk_size.x)), int(Math::floor(from2.y / chunk_size.y)));
        Vector2i tt2(int(Math::floor(to2.x / chunk_size.x)), int(Math::floor(to2.y / chunk_size.y)));
        x0 = ft2.x; y0 = ft2.y; x1 = tt2.x; y1 = tt2.y;
        dx = Math::abs(x1 - x0); dy = Math::abs(y1 - y0);
        sx = x0 > x1 ? -1 : 1; sy = y0 > y1 ? -1 : 1; err = dx - dy;
        while (true) {
            if (call_unique(Vector2i(x0, y0))) return true;
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
        return false;
    }
    static bool line_chunks(Vector2 from, Vector2 to, double width, Vector2 chunk_size, Callable method);

    template<typename F>
    static bool line_chunks_without_width(Vector2 from, Vector2 to, Vector2 chunk_size, F method) {
        Vector2i ft(int(Math::floor(from.x / chunk_size.x)), int(Math::floor(from.y / chunk_size.y)));
        Vector2i tt(int(Math::floor(to.x / chunk_size.x)), int(Math::floor(to.y / chunk_size.y)));
        int x0 = ft.x, y0 = ft.y, x1 = tt.x, y1 = tt.y;
        int dx = Math::abs(x1 - x0), dy = Math::abs(y1 - y0);
        int sx = x0 > x1 ? -1 : 1, sy = y0 > y1 ? -1 : 1, err = dx - dy;
        while (true) {
            if (method(Vector2i(x0, y0))) return true;
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
        return false;
    }
    static bool line_chunks_without_width(Vector2 from, Vector2 to, Vector2 chunk_size, Callable method);

    template<typename F>
    static void iterate_points_in_r1_not_r2(Rect2i r1, Rect2i r2, F cb) {
        int x1 = r1.position.x, y1 = r1.position.y;
        int x1e = x1 + r1.size.x - 1, y1e = y1 + r1.size.y - 1;
        if (r1.size.x <= 0 || r1.size.y <= 0) return;
        int x2 = r2.position.x, y2 = r2.position.y;
        int x2e = x2 + r2.size.x - 1, y2e = y2 + r2.size.y - 1;
        int ix1 = MAX(x1, x2), iy1 = MAX(y1, y2);
        int ix2 = MIN(x1e, x2e), iy2 = MIN(y1e, y2e);
        if (ix1 > ix2 || iy1 > iy2) {
            for (int y = y1; y <= y1e; y++)
                for (int x = x1; x <= x1e; x++)
                    cb(Vector2i(x, y));
            return;
        }
        if (iy1 > y1)
            for (int y = y1; y < iy1; y++)
                for (int x = x1; x <= x1e; x++)
                    cb(Vector2i(x, y));
        if (iy2 < y1e)
            for (int y = iy2 + 1; y <= y1e; y++)
                for (int x = x1; x <= x1e; x++)
                    cb(Vector2i(x, y));
        if (ix1 > x1)
            for (int y = iy1; y <= iy2; y++)
                for (int x = x1; x < ix1; x++)
                    cb(Vector2i(x, y));
        if (ix2 < x1e)
            for (int y = iy1; y <= iy2; y++)
                for (int x = ix2 + 1; x <= x1e; x++)
                    cb(Vector2i(x, y));
    }
    static void iterate_points_in_r1_not_r2(Rect2i r1, Rect2i r2, Callable callback);

    // --- Intersection tests ---

    static bool rect_circle_intersects(Rect2 rect, double radius, Vector2 pos);
    static bool rect_ring_intersects(Rect2 rect, double radius_from, double radius, Vector2 pos);
    static bool l_c_handle(Vector2 from, Vector2 to, double width, double radius, Vector2 pos);
    static bool point_s_handle(Vector2 point, double radius, Vector2 pos, double dir, double half_angle);

    // --- Screen / camera ---

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

    // --- Utility ---

    static double get_lightness(Color c) noexcept;
    static void ff();
    static void time_start();
    static void time_print(String extra = "", bool restart = true);
    static float randf_range(float min, float max);
};

} // namespace godot
