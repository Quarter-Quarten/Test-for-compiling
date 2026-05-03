#pragma once

#include <tools/Lighting/WarpGrid.h>

#include <godot_cpp/classes/canvas_item_material.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace godot {

class Lighting : public Node {
    GDCLASS(Lighting, Node);

private:
    static constexpr float SOURCE_LIGHTNESS_MULTIPLIER = 1.25f;
    static constexpr int MAX_SHADOW_DISTANCE = 3;

    struct LightSource {
        Vector2i pos;
        float intensity = 0.0f;
        Color color = Color(0, 0, 0, 1);
        float radius = 0.0f;
    };

    struct SourceGroupKey {
        Color color;
        float intensity = 0.0f;
        float radius = 0.0f;
        bool operator<(const SourceGroupKey &p_other) const;
    };

    int tile_light_size = 1;
    Vector2i draw_size = Vector2i(); // tile count for lighting compute
    Vector2i draw_size_tile = Vector2i(); // Vars.draw_size_tile
    Vector2i load_size_tile = Vector2i();
    Vector2i region_size = Vector2i(4, 4);
    Vector2 tile_size = Vector2(16, 16);
    int light_layer = 120;
    bool light_enabled = true;

    Object *vars_node = nullptr;
    Ref<Script> consts_script;
    Ref<Script> setting_script;
    Ref<Script> region_controler_script;
    Ref<Script> time_controler_script;

    Ref<Image> src_image;
    Ref<ImageTexture> texture;

    Ref<WrapGridColor> last_light_map;
    Ref<WrapGridColor> current_light_map;
    mutable std::mutex last_light_map_mutex;

    Ref<WrapGridFloat32> light_lose_map;
    Ref<WrapGridFloat32> distance_field;
    std::vector<int> edge_points;

    std::map<Vector2i, LightSource> sources;
    std::map<Vector2i, LightSource> current_sources;
    std::vector<Vector2i> source_erase;
    std::vector<Vector2i> current_source_erase;
    mutable std::mutex source_mutex;

    std::map<Vector2i, LightSource> moving_lights;
    mutable std::mutex moving_lights_mutex;

    std::map<Vector2i, float> now_grids1;
    std::map<Vector2i, float> now_grids2;
    std::map<Vector2i, float> handled_grids;
    std::map<Vector2i, LightSource> keeping;
    std::map<SourceGroupKey, std::vector<LightSource>> category;

    RID rid;
    Ref<CanvasItemMaterial> material;
    Color last_time_color = Color(0, 0, 0, 1);

    std::thread worker_thread;
    std::condition_variable worker_cv;
    std::mutex worker_mutex;
    bool worker_running = false;
    bool worker_wakeup = false;
    std::atomic<bool> dirty = false;

    std::mutex pending_mutex;
    Ref<Image> pending_img;
    Vector2i pending_top_left = Vector2i();
    bool has_pending_submit = false;

    static int _wrapi(int p_value, int p_min, int p_max);
    static float _get_lightness(const Color &p_c);
    static bool _rect_circle_intersects(const Rect2i &p_rect, float p_radius, const Vector2i &p_center);
    static Color _apply_effects(const Color &p_color);
    static Vector2i _region_to_tile(const Vector2i &p_region, const Vector2i &p_region_size);

    Color _mix_color(const Color &p_a, const Color &p_b) const;
    void _fetch_runtime_config();
    void _init_canvas_item();
    void _start_worker();
    void _stop_worker();
    void _worker_loop();
    void _thread_function();
    void _copy_light_lose_from_vars(Ref<WrapGridFloat32> &p_out);
    Color _get_time_color() const;
    Vector2i _get_region_first() const;
    bool _is_world_initiated() const;
    void _mix_light(Ref<WrapGridColor> &p_light_map, const Vector2i &p_size, const Vector2i &p_current_top_left, const Color &p_time_color);
    void _compute_sky_shadows(Ref<WrapGridColor> &p_light_map, Ref<WrapGridFloat32> &p_light_lose_map, const Rect2i &p_draw_tile_rect);
    void _process_light_source_group(const std::vector<LightSource> &p_sources, Ref<WrapGridFloat32> &p_light_lose_map, const Color &p_color, float p_intensity, float p_radius, Ref<WrapGridColor> &p_light_map, const Vector2i &p_current_top_left);
    void _spread(Ref<WrapGridColor> &p_light_map, Ref<WrapGridFloat32> &p_light_lose_map, const Color &p_color, float p_max_radius, float p_intensity, const Vector2i &p_current_top_left);
    void _generate_tiled_texture(const Ref<Image> &p_img);
    void _submit_texture(const Vector2i &p_current_top_left);

protected:
    static void _bind_methods();

public:
    Lighting() = default;
    ~Lighting() override = default;

    void _ready() override;
    void _process(double p_delta) override;
    void _exit_tree() override;

    void add_source(const Vector2i &p_position, float p_intensity, int p_radius, const Color &p_color);
    void erase_source(const Vector2i &p_position);
    void erase_and_add_source(const Vector2i& p_erase_position, const Vector2i& p_add_position, float p_intensity, int p_radius, const Color& p_color);
    void add_moving_light(const Vector2i &p_pos, float p_intensity, int p_radius, const Color &p_color);
    void erase_moving_light(const Vector2i &p_pos);
    void erase_and_add_moving_light(const Vector2i &p_erase, const Vector2i &p_add, float p_intensity, int p_radius, const Color &p_color);
    void block_created(float p_light_lose);
    void block_destroyed(float p_light_lose);
    void refresh();
    Color get_tile_light(const Vector2i &p_tile) const;
};

} // namespace godot
