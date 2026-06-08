#pragma once

#include <vector>
#include <unordered_map>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <tools/syncable/syncable.hpp>
#include <tools/call/Call.hpp>
#include <tools/consts_c/consts_c.hpp>

using namespace godot;

struct Vector2iHash {
    size_t operator()(const Vector2i& v) const {
        size_t h1 = static_cast<size_t>(v.x);
        size_t h2 = static_cast<size_t>(v.y);
        return (h1 ^ (h1 << 7) ^ (h1 >> 3)) ^ ((h2 ^ (h2 << 7) ^ (h2 >> 3)) << 1);
    }
};

class TreeObject : public RefCounted {
    GDCLASS(TreeObject, RefCounted);
public:
    Rect2 current_quad_tree_bounds;

    Object* obj = nullptr;
    Rect2 aabb;
    Vector2 pos;
    float radius = 0.0f;
    int team = 0;

    TreeObject* recreate(Object* obj, int team);
    Ref<TreeObject> duplicate() const;
    void update(const Vector2& p_pos, const Vector2& p_size);
    bool is_in_tree_bounds() const;

protected:
    static void _bind_methods();
};

class QuadTree : public RefCounted {
    GDCLASS(QuadTree, RefCounted);
public:
    Object* manager = nullptr;

    QuadTree* parent = nullptr;
    int self_idx = 0;

    Rect2 bounds;
    Vector2 bounds_center;
    Vector2i region_pos;

    std::vector<Ref<TreeObject>> tree_objects;
    Ref<QuadTree> children[4] = {};
    int child_count = 0;

    QuadTree* recreate(const Vector2i& p_region_pos, const Rect2i& p_bounds, QuadTree* p_parent = nullptr, int p_self_idx = 0);
    void deactivate();

    QuadTree* get_parent() const;
    Ref<TreeObject> get_tree_obj(Object* obj, const Vector2& last_tick_pos);
    void insert(const Ref<TreeObject>& tree_obj, const Vector2i& tile);
    void erase_at(Object* obj, const Vector2i& tile);
    bool _erase_at_internal(Object* obj, const Vector2i& tile);

    bool retrieve_rect(const Rect2& area, int team, int team_except, const Callable& method);
    bool retrieve_circle(const Vector2& center, double radius, int team, int team_except, const Callable& method);
    bool retrieve_ring(const Vector2& center, double radius_from, double radius_to, int team, int team_except, const Callable& method);

    // C++ lambda versions — avoids Callable overhead
    template<typename F>
    bool retrieve_rect_fn(const Rect2& area, int team, int team_except, F&& func) {
        if (child_count) {
            for (int i = 0; i < 4; i++) {
                if (children[i].is_valid() && children[i]->bounds.intersects(area)) {
                    if (children[i]->retrieve_rect_fn(area, team, team_except, func)) return true;
                }
            }
            return false;
        }

        for (const Ref<TreeObject>& tree_obj : tree_objects) {
            if (tree_obj.is_valid()) {
                if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && tree_obj->aabb.intersects(area)) {
                    if (func(tree_obj->obj)) return true;
                }
            }
        }
        return false;
    }

    template<typename F>
    bool retrieve_circle_fn(const Vector2& center, double radius, int team, int team_except, F&& func) {
        if (child_count) {
            for (int i = 0; i < 4; i++) {
                if (children[i].is_valid() && Call::rect_circle_intersects(children[i]->bounds, radius, center)) {
                    if (children[i]->retrieve_circle_fn(center, radius, team, team_except, func)) return true;
                }
            }
            return false;
        }

        for (const Ref<TreeObject>& tree_obj : tree_objects) {
            if (tree_obj.is_valid()) {
                if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && Call::rect_circle_intersects(tree_obj->aabb, radius, center)) {
                    if (func(tree_obj->obj)) return true;
                }
            }
        }
        return false;
    }

    template<typename F>
    bool retrieve_ring_fn(const Vector2& center, double radius_from, double radius_to, int team, int team_except, F&& func) {
        if (child_count) {
            for (int i = 0; i < 4; i++) {
                if (children[i].is_valid() && Call::rect_ring_intersects(children[i]->bounds, radius_from, radius_to, center)) {
                    if (children[i]->retrieve_ring_fn(center, radius_from, radius_to, team, team_except, func)) return true;
                }
            }
            return false;
        }

        for (const Ref<TreeObject>& tree_obj : tree_objects) {
            if (tree_obj.is_valid()) {
                if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && Call::rect_ring_intersects(tree_obj->aabb, radius_from, radius_to, center)) {
                    if (func(tree_obj->obj)) return true;
                }
            }
        }
        return false;
    }

    void clear();
    bool is_empty() const;
    void draw_debug(RID canvas_item, const Color& color);

protected:
    static void _bind_methods();

private:
    bool _erase_here(Object* obj);
    void _prune_if_empty();
    void _split();
    void _add_tree_object(const Ref<TreeObject>& tree_obj);
    Rect2 _get_child_rect(int idx) const;
    int _pos_get_index(const Vector2& p) const;
    bool _can_split() const;
    void _delete_child(int idx, const Ref<QuadTree>& child);
};

class QuadTreeManagerC : public Syncable {
    GDCLASS(QuadTreeManagerC, Syncable);
public:
    static constexpr int MAX_OBJECTS = 6;

    std::unordered_map<Vector2i, Ref<QuadTree>, Vector2iHash> tree;
    std::vector<Ref<QuadTree>> deactive_trees;
    int reserve_size = 0;

    // Init
    void init(int p_reserve_size);
    void _add_deactive();
    Ref<QuadTree> _get_deactive();

    // Public API — work + sync propagation
    void add(const Vector2i& region, const Vector2i& next_tile, Object* obj);
    void _add(const Vector2i& region, const Vector2i& next_tile, Object* obj);
    void delete_(const Vector2i& big_region, const Vector2i& last_tile, Object* obj);
    void update_pos(Object* u, const Vector2& current_pos, const Vector2& last_tick_pos);

    // Retrieval
    bool retrieve_rect(const Vector2i& big_region, const Rect2& area, int team, int team_except, const Callable& method);
    bool retrieve_circle(const Vector2& center, double radius, int team, int team_except, const Callable& method);
    bool retrieve_ring(const Vector2& center, double radius_from, double radius_to, int team, int team_except, const Callable& method);

    // C++ lambda versions — avoids Callable overhead
    template<typename F>
    bool retrieve_rect_fn(const Vector2i& big_region, const Rect2& area, int team, int team_except, F&& func) {
        auto it = tree.find(big_region);
        if (it != tree.end() && it->second.is_valid()) {
            return it->second->retrieve_rect_fn(area, team, team_except, func);
        }
        return false;
    }

    template<typename F>
    bool retrieve_circle_fn(const Vector2& center, double radius, int team, int team_except, F&& func) {
        return Call::circle_big_regions(center, radius, [&](Vector2i region) -> bool {
            auto it = tree.find(region);
            if (it != tree.end() && it->second.is_valid()) {
                return it->second->retrieve_circle_fn(center, radius, team, team_except, func);
            }
            return false;
        });
    }

    template<typename F>
    bool retrieve_ring_fn(const Vector2& center, double radius_from, double radius_to, int team, int team_except, F&& func) {
        return Call::ring_big_regions(center, radius_from, radius_to, [&](Vector2i region) -> bool {
            auto it = tree.find(region);
            if (it != tree.end() && it->second.is_valid()) {
                return it->second->retrieve_ring_fn(center, radius_from, radius_to, team, team_except, func);
            }
            return false;
        });
    }

    // Clear / debug
    void clear();
    void _delete_leaf(const Ref<QuadTree>& quad_tree);
    void _recycle_quad_tree(const Ref<QuadTree>& quad_tree);
    void draw_debug(RID canvas_item, const Color& color);

    // Sync propagation handlers — each_sync_to passes (Object* target) as LAST arg
    void _on_sync_add(const Vector2i& region, const Vector2i& next_tile, Object* obj, Object* target);
    void _on_sync_delete(const Vector2i& big_region, const Vector2i& last_tile, Object* obj, Object* target);
    void _on_sync_clear(Object* target);

protected:
    static void _bind_methods();

private:
    void _add_internal(const Vector2i& region, const Vector2i& next_tile, const Ref<TreeObject>& tree_obj);
    void _update_pos(Object* u, const Vector2& last_tick_pos, const Vector2i& last_region, const Vector2i& last_tile, const Vector2i& next_tile, const Vector2& p_pos, const Vector2& p_size);
};
