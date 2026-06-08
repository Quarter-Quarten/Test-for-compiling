#include "tools/call/call.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <random>

using namespace godot;

static double s_time_point = 0.0;

// ------------------------------------------------------------------
//  Bind methods (Godot API)
// ------------------------------------------------------------------

void Call::_bind_methods() {
    ClassDB::bind_static_method("Call", D_METHOD("x_to_tile", "x"), &Call::x_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("y_to_tile", "y"), &Call::y_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_tile", "pos"), &Call::pos_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_pos", "tile_pos"), &Call::tile_to_pos);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_region", "tile_pos"), &Call::tile_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_to_tile", "region"), &Call::region_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_region", "pos"), &Call::pos_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_to_pos", "region"), &Call::region_to_pos);
    ClassDB::bind_static_method("Call", D_METHOD("tile_rect_to_global", "rect"), &Call::tile_rect_to_global);
    ClassDB::bind_static_method("Call", D_METHOD("global_rect_to_tile", "rect"), &Call::global_rect_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_big_region", "pos"), &Call::pos_to_big_region);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_big_region", "tile_pos"), &Call::tile_to_big_region);
    ClassDB::bind_static_method("Call", D_METHOD("big_region_to_region", "region"), &Call::big_region_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_center", "region"), &Call::region_center);
    ClassDB::bind_static_method("Call", D_METHOD("big_region_center", "region"), &Call::big_region_center);
    ClassDB::bind_static_method("Call", D_METHOD("get_tile_rect", "tile"), &Call::get_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_region_rect", "region"), &Call::get_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_big_region_rect", "region"), &Call::get_big_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_region_tile_rect", "region"), &Call::get_region_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_big_region_tile_rect", "region"), &Call::get_big_region_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_circle_tile_rect", "center", "radius"), &Call::get_circle_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_circle_aabb", "center", "radius"), &Call::get_circle_aabb);
    ClassDB::bind_static_method("Call", D_METHOD("get_line_tile_rect", "from", "to"), &Call::get_line_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("region_tiles", "r", "method"), static_cast<bool(*)(Vector2i, Callable)>(&Call::region_tiles));
    ClassDB::bind_static_method("Call", D_METHOD("big_region_regions", "r", "method"), static_cast<bool(*)(Vector2i, Callable)>(&Call::big_region_regions));
    ClassDB::bind_static_method("Call", D_METHOD("circle_tiles", "pos", "radius", "method"), static_cast<bool(*)(Vector2, double, Callable)>(&Call::circle_tiles));
    ClassDB::bind_static_method("Call", D_METHOD("circle_collision", "pos", "radius", "method"), static_cast<bool(*)(Vector2, double, Callable)>(&Call::circle_collision));
    ClassDB::bind_static_method("Call", D_METHOD("circle_regions", "pos", "radius", "method"), static_cast<bool(*)(Vector2, double, Callable)>(&Call::circle_regions));
    ClassDB::bind_static_method("Call", D_METHOD("circle_big_regions", "pos", "radius", "method"), static_cast<bool(*)(Vector2, double, Callable)>(&Call::circle_big_regions));
    ClassDB::bind_static_method("Call", D_METHOD("rect_tiles", "pos", "width", "height", "method"), static_cast<void(*)(Vector2, double, double, Callable)>(&Call::rect_tiles));
    ClassDB::bind_static_method("Call", D_METHOD("rect_collision", "pos", "width", "height", "method"), static_cast<void(*)(Vector2, double, double, Callable)>(&Call::rect_collision));
    ClassDB::bind_static_method("Call", D_METHOD("rect_regions", "pos", "width", "height", "method"), static_cast<void(*)(Vector2, double, double, Callable)>(&Call::rect_regions));
    ClassDB::bind_static_method("Call", D_METHOD("rect_big_regions", "pos", "width", "height", "method"), static_cast<void(*)(Vector2, double, double, Callable)>(&Call::rect_big_regions));
    ClassDB::bind_static_method("Call", D_METHOD("rect2i_outer", "l", "t", "size_x", "size_y", "width", "method", "ignore_vertex"), static_cast<void(*)(int, int, int, int, int, Callable, bool)>(&Call::rect2i_outer), DEFVAL(false));
    ClassDB::bind_static_method("Call", D_METHOD("ring_tiles", "pos", "radius_from", "radius", "method"), static_cast<bool(*)(Vector2, double, double, Callable)>(&Call::ring_tiles));
    ClassDB::bind_static_method("Call", D_METHOD("ring_collision", "pos", "radius_from", "radius", "method"), static_cast<bool(*)(Vector2, double, double, Callable)>(&Call::ring_collision));
    ClassDB::bind_static_method("Call", D_METHOD("ring_blocks", "pos", "radius_from", "radius", "method"), static_cast<bool(*)(Vector2, double, double, Callable)>(&Call::ring_blocks));
    ClassDB::bind_static_method("Call", D_METHOD("ring_regions", "pos", "radius_from", "radius", "method"), static_cast<bool(*)(Vector2, double, double, Callable)>(&Call::ring_regions));
    ClassDB::bind_static_method("Call", D_METHOD("ring_big_regions", "pos", "radius_from", "radius", "method"), static_cast<bool(*)(Vector2, double, double, Callable)>(&Call::ring_big_regions));
    ClassDB::bind_static_method("Call", D_METHOD("angle_range_transport", "angle"), &Call::angle_range_transport);
    ClassDB::bind_static_method("Call", D_METHOD("line_chunks", "from", "to", "width", "chunk_size", "method"), static_cast<bool(*)(Vector2, Vector2, double, Vector2, Callable)>(&Call::line_chunks));
    ClassDB::bind_static_method("Call", D_METHOD("line_chunks_without_width", "from", "to", "chunk_size", "method"), static_cast<bool(*)(Vector2, Vector2, Vector2, Callable)>(&Call::line_chunks_without_width));
    ClassDB::bind_static_method("Call", D_METHOD("iterate_points_in_r1_not_r2", "r1", "r2", "callback"), static_cast<void(*)(Rect2i, Rect2i, Callable)>(&Call::iterate_points_in_r1_not_r2));
    ClassDB::bind_static_method("Call", D_METHOD("rect_circle_intersects", "rect", "radius", "pos"), &Call::rect_circle_intersects);
    ClassDB::bind_static_method("Call", D_METHOD("rect_ring_intersects", "rect", "radius_from", "radius", "pos"), &Call::rect_ring_intersects);
    ClassDB::bind_static_method("Call", D_METHOD("l_c_handle", "from", "to", "width", "radius", "pos"), &Call::l_c_handle);
    ClassDB::bind_static_method("Call", D_METHOD("point_s_handle", "point", "radius", "pos", "dir", "half_angle"), &Call::point_s_handle);
    ClassDB::bind_static_method("Call", D_METHOD("screen_to_global", "screen_pos"), &Call::screen_to_global);
    ClassDB::bind_static_method("Call", D_METHOD("global_to_screen", "global_pos"), &Call::global_to_screen);
    ClassDB::bind_static_method("Call", D_METHOD("get_transform"), &Call::get_transform);
    ClassDB::bind_static_method("Call", D_METHOD("screen_left_up"), &Call::screen_left_up);
    ClassDB::bind_static_method("Call", D_METHOD("screen_global_size"), &Call::screen_global_size);
    ClassDB::bind_static_method("Call", D_METHOD("screen_first_tile"), &Call::screen_first_tile);
    ClassDB::bind_static_method("Call", D_METHOD("screen_first_region"), &Call::screen_first_region);
    ClassDB::bind_static_method("Call", D_METHOD("screen_to_global_rect"), &Call::screen_to_global_rect);
    ClassDB::bind_static_method("Call", D_METHOD("screen_tile_rect"), &Call::screen_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("screen_region_rect"), &Call::screen_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_lightness", "c"), &Call::get_lightness);
    ClassDB::bind_static_method("Call", D_METHOD("ff"), &Call::ff);
    ClassDB::bind_static_method("Call", D_METHOD("time_start"), &Call::time_start);
    ClassDB::bind_static_method("Call", D_METHOD("time_print", "extra", "restart"), &Call::time_print, DEFVAL(String()), DEFVAL(true));
}

// ------------------------------------------------------------------
//  Vars
// ------------------------------------------------------------------

Node2D* Call::get_vars() { return _vars(); }
Node2D* Call::_vars() {
    if (vars) return vars;

    SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
    if (!tree || !tree->get_root()) return nullptr;

    vars = Object::cast_to<Node2D>(tree->get_root()->get_node_or_null(NodePath("/root/Vars")));
    return vars;
}
Node2D* Call::get_world() { return _world(); }
Node2D* Call::_world() {
    if (world) return world;
    return Object::cast_to<Node2D>(_vars()->get("world"));
}
Resource* Call::get_setting() { return _setting(); }
Resource* Call::_setting() {
    if (setting.is_valid()) return setting.ptr();
    setting = ResourceLoader::get_singleton()->load("res://src/global/Setting.gd");
    return setting.ptr();
}

// ------------------------------------------------------------------
//  Coordinate conversion
// ------------------------------------------------------------------

int Call::x_to_tile(double x) { return int(Math::floor(x / ConstsC::get_tile_size().x)); }
int Call::y_to_tile(double y) { return int(Math::floor(y / ConstsC::get_tile_size().y)); }
Vector2i Call::pos_to_tile(Vector2 pos) { return (pos / ConstsC::get_tile_size()).floor(); }
Vector2 Call::tile_to_pos(Vector2i tile_pos) { return (Vector2(tile_pos) + Vector2(0.5, 0.5)) * ConstsC::get_tile_size(); }

Vector2i Call::tile_to_region(Vector2i tile_pos) { return (Vector2(tile_pos) / Vector2(ConstsC::get_region_size())).floor(); }
Vector2i Call::region_to_tile(Vector2i region) { return region * ConstsC::get_region_size(); }
Vector2i Call::pos_to_region(Vector2 pos) { return (pos / ConstsC::get_real_region_size()).floor(); }
Vector2 Call::region_to_pos(Vector2i region) { return Vector2(region) * ConstsC::get_real_region_size(); }

Vector2i Call::pos_to_big_region(Vector2 pos) { return (pos / ConstsC::get_big_region_real_size()).floor(); }
Vector2i Call::tile_to_big_region(Vector2i tile_pos) { return (Vector2(tile_pos) / Vector2(ConstsC::get_big_region_tile_size())).floor(); }
Vector2i Call::big_region_to_region(Vector2i region) { return region * ConstsC::get_big_region_size(); }

Vector2 Call::region_center(Vector2i region) { return Vector2(region) * ConstsC::get_real_region_size() + ConstsC::get_hrrs(); }
Vector2 Call::big_region_center(Vector2i region) { return Vector2(region) * ConstsC::get_big_region_real_size() + ConstsC::get_half_big_region_real_size(); }

// ------------------------------------------------------------------
//  Rect conversion
// ------------------------------------------------------------------

Rect2 Call::tile_rect_to_global(Rect2i rect) {
    Vector2 ts = ConstsC::get_tile_size();
    return Rect2(rect.position.x * ts.x, rect.position.y * ts.y, rect.size.x * ts.x, rect.size.y * ts.x);
}

Rect2i Call::global_rect_to_tile(Rect2 rect) {
    Vector2i lt = pos_to_tile(rect.position);
    Vector2i rd = pos_to_tile(rect.position + rect.size);
    return Rect2i(lt, rd - lt + Vector2i(1, 1));
}

Rect2 Call::get_tile_rect(Vector2i tile) { Vector2 ts = ConstsC::get_tile_size(); return Rect2(Vector2(tile) * ts, ts); }
Rect2 Call::get_region_rect(Vector2i region) { Vector2 rs = ConstsC::get_real_region_size(); return Rect2(Vector2(region) * rs, rs); }
Rect2 Call::get_big_region_rect(Vector2i region) { Vector2 brs = ConstsC::get_big_region_real_size(); return Rect2(Vector2(region) * brs, brs); }
Rect2i Call::get_region_tile_rect(Vector2i region) { Vector2i rs = ConstsC::get_region_size(); return Rect2i(region * rs, rs); }
Rect2i Call::get_big_region_tile_rect(Vector2i region) { Vector2i brs = ConstsC::get_big_region_tile_size(); return Rect2i(region * brs, brs); }

Rect2i Call::get_circle_tile_rect(Vector2 center, double radius) {
    Vector2 ts = ConstsC::get_tile_size();
    return Rect2i(pos_to_tile(Vector2(center.x - radius, center.y - radius)),
        Vector2i(Vector2(radius * 2 + ts.x, radius * 2 + ts.y) / ts));
}

Rect2 Call::get_circle_aabb(Vector2 center, double radius) {
    return Rect2(center.x - radius, center.y - radius, radius * 2, radius * 2);
}

Rect2i Call::get_line_tile_rect(Vector2 from, Vector2 to) {
    float l = MIN(from.x, to.x), r = MAX(from.x, to.x);
    float t = MIN(from.y, to.y), b = MAX(from.y, to.y);
    Vector2 ts = ConstsC::get_tile_size();
    return Rect2i(pos_to_tile(Vector2(l, t)),
        Vector2i(int(Math::floor((r - l) / ts.x)) + 1, int(Math::floor((b - t) / ts.y)) + 1));
}

// ------------------------------------------------------------------
//  Utility — angle
// ------------------------------------------------------------------

double Call::angle_range_transport(double angle) { return Math::fmod(angle + Math_TAU, Math_TAU); }

// ------------------------------------------------------------------
//  Block operations
// ------------------------------------------------------------------

bool Call::clear_block(Vector2i tile, bool kill, bool direct_write) {
    // todo: implement
    return false;
}

void Call::set_block_data(Vector2i tile, PackedByteArray data) {
    // todo: implement
}

// ------------------------------------------------------------------
//  Iteration — Callable delegates
//  Each wraps the Godot Callable in a lambda and forwards to the
//  template overload defined in the header.
// ------------------------------------------------------------------

bool Call::region_tiles(Vector2i r, Callable method) {
    return region_tiles(r, [&](Vector2i v) { return bool(method.call(v)); });
}

bool Call::big_region_regions(Vector2i r, Callable method) {
    return big_region_regions(r, [&](Vector2i v) { return bool(method.call(v)); });
}

bool Call::circle_tiles(Vector2 pos, double radius, Callable method) {
    return circle_tiles(pos, radius, [&](Vector2i t) { return bool(method.call(t)); });
}

bool Call::circle_collision(Vector2 pos, double radius, Callable method) {
    return circle_collision(pos, radius, [&](Object* b) { return bool(method.call(b)); });
}

bool Call::circle_regions(Vector2 pos, double radius, Callable method) {
    return circle_regions(pos, radius, [&](Vector2i r) { return bool(method.call(r)); });
}

bool Call::circle_big_regions(Vector2 pos, double radius, Callable method) {
    return circle_big_regions(pos, radius, [&](Vector2i r) { return bool(method.call(r)); });
}

void Call::rect_tiles(Vector2 pos, double width, double height, Callable method) {
    rect_tiles(pos, width, height, [&](Vector2i v) { method.call(v); });
}

void Call::rect_collision(Vector2 pos, double width, double height, Callable method) {
    rect_collision(pos, width, height, [&](Object* b) { method.call(b); });
}

void Call::rect_regions(Vector2 pos, double width, double height, Callable method) {
    rect_regions(pos, width, height, [&](Vector2i v) { method.call(v); });
}

void Call::rect_big_regions(Vector2 pos, double width, double height, Callable method) {
    rect_big_regions(pos, width, height, [&](Vector2i v) { method.call(v); });
}

void Call::rect2i_outer(int l, int t, int size_x, int size_y, int width, Callable method, bool ignore_vertex) {
    rect2i_outer(l, t, size_x, size_y, width, [&](Vector2i v) { method.call(v); }, ignore_vertex);
}

bool Call::ring_tiles(Vector2 pos, double radius_from, double radius, Callable method) {
    return ring_tiles(pos, radius_from, radius, [&](Vector2i t) { return bool(method.call(t)); });
}

bool Call::ring_collision(Vector2 pos, double radius_from, double radius, Callable method) {
    return ring_collision(pos, radius_from, radius, [&](Object* b) { return bool(method.call(b)); });
}

bool Call::ring_blocks(Vector2 pos, double radius_from, double radius, Callable method) {
    return ring_blocks(pos, radius_from, radius, [&](Object* b) { return bool(method.call(b)); });
}

bool Call::ring_regions(Vector2 pos, double radius_from, double radius, Callable method) {
    return ring_regions(pos, radius_from, radius, [&](Vector2i r) { return bool(method.call(r)); });
}

bool Call::ring_big_regions(Vector2 pos, double radius_from, double radius, Callable method) {
    return ring_big_regions(pos, radius_from, radius, [&](Vector2i r) { return bool(method.call(r)); });
}

bool Call::line_chunks(Vector2 from, Vector2 to, double width, Vector2 chunk_size, Callable method) {
    return line_chunks(from, to, width, chunk_size, [&](Vector2i t) { return bool(method.call(t)); });
}

bool Call::line_chunks_without_width(Vector2 from, Vector2 to, Vector2 chunk_size, Callable method) {
    return line_chunks_without_width(from, to, chunk_size, [&](Vector2i t) { return bool(method.call(t)); });
}

void Call::iterate_points_in_r1_not_r2(Rect2i r1, Rect2i r2, Callable callback) {
    iterate_points_in_r1_not_r2(r1, r2, [&](Vector2i v) { callback.call(v); });
}

// ------------------------------------------------------------------
//  Intersection tests
// ------------------------------------------------------------------

bool Call::rect_circle_intersects(Rect2 rect, double radius, Vector2 pos) {
    Vector2 dif = pos - rect.get_center();
    Vector2 n(Math::clamp(dif.x, -rect.size.x / 2, rect.size.x / 2),
        Math::clamp(dif.y, -rect.size.y / 2, rect.size.y / 2));
    return (dif - n).length_squared() <= radius * radius;
}

bool Call::rect_ring_intersects(Rect2 rect, double rf, double r, Vector2 pos) {
    Vector2 dif = pos - rect.get_center();
    Vector2 n(Math::clamp(dif.x, -rect.size.x / 2, rect.size.x / 2),
        Math::clamp(dif.y, -rect.size.y / 2, rect.size.y / 2));
    Vector2 f(Math::clamp(-dif.x, -rect.size.x / 2, rect.size.x / 2),
        Math::clamp(-dif.y, -rect.size.y / 2, rect.size.y / 2));
    return (dif - n).length_squared() <= r * r && (dif - f).length_squared() >= rf * rf;
}

bool Call::l_c_handle(Vector2 from, Vector2 to, double width, double radius, Vector2 pos) {
    double dst = (from - pos).length();
    double angle = (to - from).angle() - (pos - from).angle();
    if (Math::cos(angle) * dst <= (from - to).length())
        return Math::sin(angle) * dst <= width + radius;
    return dst <= radius || (to - pos).length() <= radius;
}

bool Call::point_s_handle(Vector2 point, double radius, Vector2 pos, double dir, double half_angle) {
    return (point - pos).length_squared() <= radius * radius
        && Math::abs((point - pos).angle() - dir) <= half_angle;
}

// ------------------------------------------------------------------
//  Screen / camera
// ------------------------------------------------------------------

Vector2 Call::screen_to_global(Vector2 screen_pos) {
    Object* v = _vars();
    if (!v) return Vector2();
    Object* world = Object::cast_to<Object>(v->get("world"));
    Object* camera = world ? Object::cast_to<Object>(world->get("camera")) : nullptr;
    Vector2 zoom = camera ? Vector2(camera->get("zoom")) : Vector2(1, 1);
    return screen_pos / zoom + screen_left_up();
}

Vector2 Call::global_to_screen(Vector2 global_pos) {
    return get_transform().xform(global_pos);
}

Transform2D Call::get_transform() {
    Object* v = _vars();
    if (!v) return Transform2D();
    Object* vp = Object::cast_to<Object>(v->call("get_viewport"));
    return vp ? Transform2D(vp->get("canvas_transform")) : Transform2D();
}

Vector2 Call::screen_left_up() {
    Object* v = _vars();
    if (!v) return Vector2();
    Object* world = Object::cast_to<Object>(v->get("world"));
    Object* camera = world ? Object::cast_to<Object>(world->get("camera")) : nullptr;
    Vector2 gp = camera ? Vector2(camera->get("global_position")) : Vector2();
    return gp - screen_global_size() / 2.0;
}

Vector2 Call::screen_global_size() {
    Object* v = _vars();
    if (!v) return Vector2();
    Object* world = Object::cast_to<Object>(v->get("world"));
    Object* camera = world ? Object::cast_to<Object>(world->get("camera")) : nullptr;
    Vector2 zoom = camera ? Vector2(camera->get("zoom")) : Vector2(1, 1);
    Vector2 screen = Vector2(v->get("screen_size"));
    return screen / zoom;
}

Vector2i Call::screen_first_tile() { return pos_to_tile(screen_left_up()); }
Vector2i Call::screen_first_region() { return pos_to_region(screen_left_up()); }

Rect2 Call::screen_to_global_rect() { return Rect2(screen_left_up(), screen_global_size()); }

Rect2i Call::screen_tile_rect() {
    return Rect2i(screen_first_tile(),
        Vector2i((screen_global_size() / ConstsC::get_tile_size()).ceil()) + Vector2i(1, 1));
}

Rect2i Call::screen_region_rect() {
    return Rect2i(screen_first_region(),
        Vector2i((screen_global_size() / ConstsC::get_real_region_size()).ceil()) + Vector2i(1, 1));
}

// ------------------------------------------------------------------
//  Utility
// ------------------------------------------------------------------

double Call::get_lightness(Color c) noexcept { return c.r * 0.299 + c.g * 0.114 + c.b * 0.587; }
void Call::ff() {}

void Call::time_start() {
    UtilityFunctions::print("-----------------------------");
    s_time_point = Time::get_singleton()->get_ticks_usec();
}

void Call::time_print(String extra, bool restart) {
    double now = Time::get_singleton()->get_ticks_usec();
    UtilityFunctions::print(now - s_time_point, "us", "_", extra);
    if (restart) s_time_point = now;
}

float Call::randf_range(float from, float to) {
    if (from > to) return from;

    thread_local static std::mt19937 engine(std::random_device{}());
    std::uniform_real_distribution<float> dist(from, to);
    return dist(engine);
}
