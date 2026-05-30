#include "lighting.h"
#include "tools/call/call.h"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

bool Lighting::SourceGroupKey::operator<(const SourceGroupKey &p_other) const {
    if (color.r != p_other.color.r) return color.r < p_other.color.r;
    if (color.g != p_other.color.g) return color.g < p_other.color.g;
    if (color.b != p_other.color.b) return color.b < p_other.color.b;
    if (color.a != p_other.color.a) return color.a < p_other.color.a;
    if (intensity != p_other.intensity) return intensity < p_other.intensity;
    return radius < p_other.radius;
}

void Lighting::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_source", "position", "intensity", "radius", "color"), &Lighting::add_source);
    ClassDB::bind_method(D_METHOD("erase_source", "position"), &Lighting::erase_source);
    ClassDB::bind_method(D_METHOD("erase_and_add_source", "erase_p", "add_p", "intensity", "radius", "color"), &Lighting::erase_and_add_source);
    ClassDB::bind_method(D_METHOD("add_moving_light", "position", "intensity", "radius", "color"), &Lighting::add_moving_light);
    ClassDB::bind_method(D_METHOD("erase_moving_light", "position"), &Lighting::erase_moving_light);
    ClassDB::bind_method(D_METHOD("erase_and_add_moving_light", "erase_p", "add_p", "intensity", "radius", "color"), &Lighting::erase_and_add_moving_light);
    ClassDB::bind_method(D_METHOD("block_created", "light_lose"), &Lighting::block_created);
    ClassDB::bind_method(D_METHOD("block_destroyed", "light_lose"), &Lighting::block_destroyed);
    ClassDB::bind_method(D_METHOD("refresh"), &Lighting::refresh);
    ClassDB::bind_method(D_METHOD("get_tile_light", "tile"), &Lighting::get_tile_light);
}

float Lighting::_get_lightness(const Color &p_c) {
    return p_c.r * 0.299f + p_c.g * 0.114f + p_c.b * 0.587f;
}

bool Lighting::_rect_circle_intersects(const Rect2i &p_rect, float p_radius, const Vector2i &p_center) {
    const Vector2 center = p_rect.get_center();
    const Vector2 diff = Vector2(p_center) - center;
    const Vector2 half = Vector2(p_rect.size) * 0.5f;
    const Vector2 nearest = Vector2(Math::clamp(diff.x, -half.x, half.x), Math::clamp(diff.y, -half.y, half.y));
    return (diff - nearest).length_squared() <= p_radius * p_radius;
}

Color Lighting::_apply_effects(const Color &p_color) {
    return Color(p_color.r, p_color.g * 0.9f, p_color.b * 0.9f, 1.0f);
}

Vector2i Lighting::_region_to_tile(const Vector2i &p_region, const Vector2i &p_region_size) {
    return Vector2i(p_region.x * p_region_size.x, p_region.y * p_region_size.y);
}

Color Lighting::_mix_color(const Color &p_a, const Color &p_b) const {
    Color result = p_a + p_b;
    const float denom = _get_lightness(result);
    if (denom <= 0.00001f) {
        return result;
    }
    result *= MAX(_get_lightness(p_a), _get_lightness(p_b)) / denom;
    return result;
}

void Lighting::_fetch_runtime_config() {
    SceneTree *tree = get_tree();
    if (!tree || !tree->get_root()) {
        return;
    }
    vars_node = Object::cast_to<Object>(tree->get_root()->get_node_or_null(NodePath("/root/Vars")));

    consts_script = ResourceLoader::get_singleton()->load("res://src/global/Consts.gd");
    setting_script = ResourceLoader::get_singleton()->load("res://src/global/Setting.gd");
    region_controler_script = ResourceLoader::get_singleton()->load("res://src/world/controllers/map/RegionControler.gd");
    time_controler_script = ResourceLoader::get_singleton()->load("res://src/world/controllers/TimeControler.gd");

    if (vars_node) {
        const Vector2i vars_draw_size = vars_node->get("draw_size");
        draw_size_tile = vars_node->get("draw_size_tile");
        load_size_tile = vars_node->get("load_size_tile");
        draw_size = vars_draw_size;
    }

    if (consts_script.is_valid()) {
        Dictionary c = consts_script->get_script_constant_map();
        if (c.has("region_size")) {
            region_size = c["region_size"];
        }
        if (c.has("tile_size")) {
            tile_size = c["tile_size"];
        }
        if (c.has("light_layer")) {
            light_layer = int(c["light_layer"]);
        }
    }

    // key fix: lighting computes on tile grid, must be draw_size * region_size
    draw_size = draw_size * region_size;
    if (draw_size_tile == Vector2i()) {
        draw_size_tile = draw_size;
    }

    if (setting_script.is_valid()) {
        Variant light_setting = setting_script->get("light");
        if (Object *o = Object::cast_to<Object>(light_setting)) {
            light_enabled = bool(o->call("get_value"));
        }
        int smooth_value = 2;
        Variant smooth = setting_script->get("smooth_light_strength");
        if (Object *o = Object::cast_to<Object>(smooth)) {
            smooth_value = int(o->call("get_value"));
        }
        tile_light_size = 1 << smooth_value;
    }
}

void Lighting::_ready() {
    _fetch_runtime_config();

    src_image = Image::create(draw_size_tile.x, draw_size_tile.y, false, Image::FORMAT_RGBAH);
    texture.instantiate();
    material.instantiate();
    rid = RenderingServer::get_singleton()->canvas_item_create();

    current_light_map.instantiate();
    current_light_map->init_size(draw_size);
    last_light_map.instantiate();
    last_light_map->init_size(draw_size);
    light_lose_map.instantiate();
    light_lose_map->init_size(load_size_tile);
    distance_field.instantiate();
    distance_field->init_size(load_size_tile);

    _init_canvas_item();
    refresh();
    _start_worker();
}

void Lighting::_init_canvas_item() {
    if (!light_enabled || !vars_node) {
        return;
    }
    Variant canvas = vars_node->call("get_canvas");
    if (canvas.get_type() != Variant::RID) {
        return;
    }
    RenderingServer *rs = RenderingServer::get_singleton();
    rs->canvas_item_set_parent(rid, canvas);
    rs->canvas_item_set_z_index(rid, light_layer);
    material->set_blend_mode(CanvasItemMaterial::BLEND_MODE_MUL);
    rs->canvas_item_set_material(rid, material->get_rid());
    rs->canvas_item_set_modulate(rid, Color(0, 0, 0, 0));
    rs->canvas_item_add_texture_rect(rid, Rect2(Vector2(), Vector2(draw_size) * tile_size), texture->get_rid());
}

void Lighting::_start_worker() {
    std::lock_guard<std::mutex> lock(worker_mutex);
    worker_running = true;
    worker_thread = std::thread(&Lighting::_worker_loop, this);
}

void Lighting::_stop_worker() {
    {
        std::lock_guard<std::mutex> lock(worker_mutex);
        worker_running = false;
        worker_wakeup = true;
    }
    worker_cv.notify_all();
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void Lighting::_worker_loop() {
    while (true) {
        std::unique_lock<std::mutex> lock(worker_mutex);
        worker_cv.wait(lock, [this]() { return worker_wakeup; });
        worker_wakeup = false;
        const bool running = worker_running;
        lock.unlock();
        if (!running) {
            return;
        }
        _thread_function();
    }
}

bool Lighting::_is_world_initiated() const {
    if (!vars_node) return false;
    Variant world = vars_node->get("world");
    Object *world_obj = Object::cast_to<Object>(world);
    if (!world_obj) return false;
    return bool(world_obj->get("initiated"));
}

Color Lighting::_get_time_color() const {
    if (!time_controler_script.is_valid()) return Color(0, 0, 0, 1);
    Variant v = time_controler_script->call("get_now_color");
    return v.get_type() == Variant::COLOR ? (Color)v : Color(0, 0, 0, 1);
}

Vector2i Lighting::_get_region_first() const {
    if (!region_controler_script.is_valid()) return Vector2i();
    Variant first = region_controler_script->get("first");
    Object *first_obj = Object::cast_to<Object>(first);
    if (!first_obj) return Vector2i();
    Variant v = first_obj->call("get_v");
    return v.get_type() == Variant::VECTOR2I ? (Vector2i)v : Vector2i();
}

void Lighting::_copy_light_lose_from_vars(Ref<WrapGridFloat32> &p_out) {
    if (!vars_node) return;
    p_out->init_size(load_size_tile);
    vars_node->call("duplicate_light_lose_map", p_out);
}

void Lighting::_process(double) {
    if (!_is_world_initiated()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(worker_mutex);
        if (!worker_running) return;
        worker_wakeup = true;
    }
    worker_cv.notify_one();

    Ref<Image> img;
    Vector2i top_left;
    bool pending = false;
    {
        std::lock_guard<std::mutex> lock(pending_mutex);
        if (has_pending_submit) {
            img = pending_img;
            top_left = pending_top_left;
            pending_img.unref();
            has_pending_submit = false;
            pending = true;
        }
    }
    if (pending && img.is_valid()) {
        _generate_tiled_texture(img);
        _submit_texture(top_left);
    }
}

void Lighting::_exit_tree() {
    _stop_worker();
    if (rid.is_valid()) {
        RenderingServer::get_singleton()->free_rid(rid);
    }
}

void Lighting::add_source(const Vector2i &p_position, float p_intensity, int p_radius, const Color &p_color) {
    std::lock_guard<std::mutex> lock(source_mutex);
    LightSource s;
    s.pos = p_position;
    s.intensity = p_intensity * SOURCE_LIGHTNESS_MULTIPLIER;
    s.color = p_color;
    s.radius = float(p_radius);
    sources[p_position] = s;
    dirty.store(true);
}

void Lighting::erase_source(const Vector2i &p_position) {
    std::lock_guard<std::mutex> lock(source_mutex);
    source_erase.push_back(p_position);
    dirty.store(true);
}

void Lighting::erase_and_add_source(const Vector2i& p_erase_position, const Vector2i& p_add_position, float p_intensity, int p_radius, const Color& p_color) {
    erase_source(p_erase_position);
	add_source(p_add_position, p_intensity, p_radius, p_color);
}

void Lighting::add_moving_light(const Vector2i &p_pos, float p_intensity, int p_radius, const Color &p_color) {
    std::lock_guard<std::mutex> lock(moving_lights_mutex);
    LightSource s;
    s.pos = p_pos;
    s.intensity = p_intensity * SOURCE_LIGHTNESS_MULTIPLIER;
    s.color = p_color;
    s.radius = float(p_radius);
    moving_lights[p_pos] = s;
    dirty.store(true);
}

void Lighting::erase_moving_light(const Vector2i &p_pos) {
    std::lock_guard<std::mutex> lock(moving_lights_mutex);
    moving_lights.erase(p_pos);
    dirty.store(true);
}

void Lighting::erase_and_add_moving_light(const Vector2i &p_erase, const Vector2i &p_add, float p_intensity, int p_radius, const Color &p_color) {
    erase_moving_light(p_erase);
    add_moving_light(p_add, p_intensity, p_radius, p_color);
}

void Lighting::block_created(float p_light_lose) {
    if (p_light_lose != 0.0f) dirty.store(true);
}

void Lighting::block_destroyed(float p_light_lose) {
    if (p_light_lose != 0.0f) dirty.store(true);
}

void Lighting::refresh() {
    dirty.store(true);
}

Color Lighting::get_tile_light(const Vector2i &p_tile) const {
    std::lock_guard<std::mutex> lock(last_light_map_mutex);
    if (!last_light_map.is_valid() || last_light_map->get_size() == Vector2i()) return Color(0, 0, 0, 1);
    return last_light_map->get_v(p_tile.y, p_tile.x);
}

void Lighting::_thread_function() {
    const Vector2i first = _get_region_first();
    const Color time_color = _get_time_color();
    const Vector2i center_offset = vars_node ? (Vector2i(vars_node->get("load_size")) - Vector2i(vars_node->get("draw_size"))) / 2 : Vector2i();
    const Vector2i current_top_left = _region_to_tile(first + center_offset, region_size);

    bool is_dirty = dirty.exchange(false);
    if (last_time_color != time_color) {
        last_time_color = time_color;
        is_dirty = true;
    }
    if (!is_dirty) return;

    Call::time_start();
    {
        std::lock_guard<std::mutex> lock(source_mutex);
        std::swap(current_sources, sources);
        std::swap(current_source_erase, source_erase);
    }

    _copy_light_lose_from_vars(light_lose_map);
    Call::time_print("copy");
    Ref<WrapGridColor> &light_map = current_light_map;
    light_map->fill(Color(0, 0, 0, 1));

    Call::time_print("fill");

    const Rect2i draw_tile_rect(current_top_left, draw_size);
    const Rect2i load_tile_rect(_region_to_tile(first, region_size), load_size_tile);

    for (const Vector2i &tile : current_source_erase) current_sources.erase(tile);
    current_source_erase.clear();

    keeping.clear();
    category.clear();
    for (auto &it : current_sources) {
        LightSource source = it.second;
        {
            std::lock_guard<std::mutex> lock(moving_lights_mutex);
            auto ml_it = moving_lights.find(it.first);
            if (ml_it != moving_lights.end()) {
                const LightSource &m_src = ml_it->second;
                bool color_same = source.color.is_equal_approx(m_src.color);
                if (color_same) {
                    if (m_src.intensity > source.intensity) {
                        source = m_src;
                    }
                } else {
                    source.color = _mix_color(source.color, m_src.color);
                    source.intensity = MAX(source.intensity, m_src.intensity);
                    source.radius = MAX(source.radius, m_src.radius);
                }
            }
        }
        if (!_rect_circle_intersects(load_tile_rect, source.radius, source.pos)) continue;
        keeping[it.first] = source;
        SourceGroupKey key{ source.color, source.intensity, source.radius };
        category[key].push_back(source);
    }
    {
        std::lock_guard<std::mutex> lock(moving_lights_mutex);
        for (auto &ml : moving_lights) {
            if (current_sources.find(ml.first) != current_sources.end()) continue;
            const LightSource &m_src = ml.second;
            if (!_rect_circle_intersects(load_tile_rect, m_src.radius, m_src.pos)) continue;
            SourceGroupKey key{ m_src.color, m_src.intensity, m_src.radius };
            category[key].push_back(m_src);
        }
    }

    for (auto &it : category) {
        const SourceGroupKey &k = it.first;
        _process_light_source_group(it.second, light_lose_map, k.color, k.intensity, k.radius, light_map, current_top_left);
    }

    Call::time_print("process");

    _mix_light(light_map, draw_size, current_top_left, time_color);
    _compute_sky_shadows(light_map, light_lose_map, draw_tile_rect);

    Call::time_print("mix&shadow");

    {
        std::lock_guard<std::mutex> lock(last_light_map_mutex);
        last_light_map->copy_from(light_map.ptr());
    }
    {
        std::lock_guard<std::mutex> lock(source_mutex);
        current_sources.insert(sources.begin(), sources.end());
        sources.clear();
        std::swap(current_sources, sources);
    }

    if (src_image.is_valid()) {
        for (int y = 0; y < draw_size.y; y++) {
            for (int x = 0; x < draw_size.x; x++) {
                const Color c = light_map->get_v(y + current_top_left.y, x + current_top_left.x);
                src_image->set_pixel(x, y, Lighting::_apply_effects(c));
            }
        }
    }
    Ref<Image> img = src_image->duplicate();
    img->resize(draw_size.x * tile_light_size, draw_size.y * tile_light_size, Image::INTERPOLATE_BILINEAR);
    std::lock_guard<std::mutex> lock(pending_mutex);
    pending_img = img;
    pending_top_left = current_top_left;
    has_pending_submit = true;

    Call::time_print("update image");
}

void Lighting::_process_light_source_group(const std::vector<LightSource> &p_sources, Ref<WrapGridFloat32> &p_light_lose_map, const Color &p_color, float p_intensity, float p_radius, Ref<WrapGridColor> &p_light_map, const Vector2i &p_current_top_left) {
    now_grids1.clear();
    now_grids2.clear();
    const Color source_color = p_color * p_intensity * SOURCE_LIGHTNESS_MULTIPLIER;
    for (const LightSource &source : p_sources) {
        const Vector2i source_pos = source.pos;
        const Vector2i check_pos = source_pos - p_current_top_left;
        if (draw_size.y > check_pos.y && draw_size.x > check_pos.x && check_pos.y >= 0 && check_pos.x >= 0) {
            p_light_map->set_v(source_pos.y, source_pos.x, _mix_color(p_light_map->get_v(source_pos.y, source_pos.x), source_color));
        }
        now_grids1[source_pos] = p_radius;
    }
    handled_grids.clear();
    while (!now_grids1.empty()) {
        _spread(p_light_map, p_light_lose_map, p_color, p_radius, p_intensity, p_current_top_left);
        now_grids1.clear();
        std::swap(now_grids1, now_grids2);
    }
}

void Lighting::_spread(Ref<WrapGridColor> &p_light_map, Ref<WrapGridFloat32> &p_light_lose_map, const Color &p_color, float p_max_radius, float p_intensity, const Vector2i &p_current_top_left) {
    const float color_h = p_color.get_h();
    const Vector2i dirs[4] = { Vector2i(-1, 0), Vector2i(1, 0), Vector2i(0, -1), Vector2i(0, 1) };
    for (auto &it : now_grids1) {
        const Vector2i tile = it.first;
        const float radius = it.second;
        const Vector2i check_pos = tile - p_current_top_left;
        if (draw_size.y <= check_pos.y || draw_size.x <= check_pos.x || check_pos.x < 0 || check_pos.y < 0) continue;
        const float lose = p_light_lose_map->get_v(tile.y, tile.x);
        const float next_radius = radius - 1.0f - lose * 5.0f;
        if (next_radius <= 0.0f) continue;
        const float now_intensity = (next_radius / p_max_radius) * p_intensity;
        const Color add = p_color * now_intensity;
        for (const Vector2i &dir : dirs) {
            const Vector2i next = tile + dir;
            const auto hg = handled_grids.find(next);
            if (hg != handled_grids.end() && p_intensity <= hg->second) continue;
            handled_grids[next] = now_intensity;
            const Color base = p_light_map->get_v(next.y, next.x);
            if (Math::is_equal_approx(base.get_h(), color_h) && _get_lightness(base) >= now_intensity) continue;
            now_grids2[next] = next_radius;
            p_light_map->set_v(next.y, next.x, _mix_color(base, add));
        }
    }
}

void Lighting::_mix_light(Ref<WrapGridColor> &p_light_map, const Vector2i &p_size, const Vector2i &p_current_top_left, const Color &p_time_color) {
    for (int y = p_current_top_left.y; y < p_current_top_left.y + p_size.y; y++) {
        for (int x = p_current_top_left.x; x < p_current_top_left.x + p_size.x; x++) {
            p_light_map->set_v(y, x, _mix_color(p_light_map->get_v(y, x), p_time_color));
        }
    }
}

void Lighting::_compute_sky_shadows(Ref<WrapGridColor> &p_light_map, Ref<WrapGridFloat32> &p_light_lose_map, const Rect2i &p_draw_tile_rect) {
    const Vector2i size = p_draw_tile_rect.size;
    const Vector2i tl = p_draw_tile_rect.position;
    edge_points.clear();
    for (int y = tl.y; y < tl.y + size.y; y++) {
        for (int x = tl.x; x < tl.x + size.x; x++) {
            if (p_light_lose_map->get_v(y, x) <= 0.0f &&
                (p_light_lose_map->get_v(y + 1, x) > 0.0f || p_light_lose_map->get_v(y - 1, x) > 0.0f || p_light_lose_map->get_v(y, x + 1) > 0.0f || p_light_lose_map->get_v(y, x - 1) > 0.0f)) {
                edge_points.push_back(x);
                edge_points.push_back(y);
            }
        }
    }
    std::vector<float> &df_data = distance_field->get_data();
    const std::vector<float> &ll_data = p_light_lose_map->get_data();
    for (size_t i = 0; i < df_data.size(); i++) {
        df_data[i] = ll_data[i] != 0.0f ? float(MAX_SHADOW_DISTANCE) : -1.0f;
    }
    const Vector2i dirs[4] = { Vector2i(-1, 0), Vector2i(1, 0), Vector2i(0, -1), Vector2i(0, 1) };
    int i = 0;
    while (i < static_cast<int>(edge_points.size())) {
        const Vector2i current(edge_points[size_t(i)], edge_points[size_t(i + 1)]);
        i += 2;
        const float dist = distance_field->get_v(current.y, current.x);
        if (dist >= float(MAX_SHADOW_DISTANCE - 1)) continue;
        for (const Vector2i &dir : dirs) {
            const Vector2i next = current + dir;
            if (!p_draw_tile_rect.has_point(next)) continue;
            if (distance_field->get_v(next.y, next.x) == float(MAX_SHADOW_DISTANCE)) {
                distance_field->set_v(next.y, next.x, dist + 1.0f);
                edge_points.push_back(next.x);
                edge_points.push_back(next.y);
            }
        }
    }
    for (int y = tl.y; y < tl.y + size.y; y++) {
        for (int x = tl.x; x < tl.x + size.x; x++) {
            const float dist = distance_field->get_v(y, x);
            if (dist <= 0.0f) continue;
            float shadow = 0.0f;
            if (dist < float(MAX_SHADOW_DISTANCE)) {
                const float t = 1.0f - (dist / float(MAX_SHADOW_DISTANCE));
                shadow = 1.0f - Math::exp(-3.0f * t * t);
            }
            const Color c = p_light_map->get_v(y, x);
            p_light_map->set_v(y, x, Color(c.r * shadow, c.g * shadow, c.b * shadow, 1.0f));
        }
    }
}

void Lighting::_generate_tiled_texture(const Ref<Image> &p_img) {
    if (texture.is_null() || p_img.is_null()) return;
    if (texture->get_size() != Vector2i()) texture->update(p_img);
    else texture->set_image(p_img);
}

void Lighting::_submit_texture(const Vector2i &p_current_top_left) {
    if (!rid.is_valid()) return;
    RenderingServer *rs = RenderingServer::get_singleton();
    //UtilityFunctions::print(tile_size);
    rs->canvas_item_set_modulate(rid, Color(1, 1, 1, 1));
    rs->canvas_item_set_transform(rid, Transform2D(0.0, Vector2(p_current_top_left) * tile_size));
}
