#include "quad_tree_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// ============================================================
//  TreeObject
// ============================================================

void TreeObject::_bind_methods() {
    ClassDB::bind_method(D_METHOD("recreate", "obj", "team"), &TreeObject::recreate);
    ClassDB::bind_method(D_METHOD("duplicate"), &TreeObject::duplicate);
    ClassDB::bind_method(D_METHOD("update", "p_pos", "p_size"), &TreeObject::update);
    ClassDB::bind_method(D_METHOD("is_in_tree_bounds"), &TreeObject::is_in_tree_bounds);
}

TreeObject* TreeObject::recreate(Object* p_obj, int p_team) {
    obj = p_obj;
    pos = obj->call("get_pos");
    Variant size_var = obj->call("get_size");
    Vector2 size = size_var;
    radius = size.x * 0.5f;
    aabb = Rect2(pos - size * 0.5f, size);
    team = p_team;
    return this;
}

Ref<TreeObject> TreeObject::duplicate() const {
    Ref<TreeObject> t;
    t.instantiate();
    t->obj = obj;
    t->aabb = aabb;
    t->pos = pos;
    t->radius = radius;
    t->team = team;
    return t;
}

void TreeObject::update(const Vector2& p_pos, const Vector2& p_size) {
    pos = p_pos;
    radius = p_size.x * 0.5f;
    aabb = Rect2(pos - p_size * 0.5f, p_size);
}

bool TreeObject::is_in_tree_bounds() const {
    return current_quad_tree_bounds.has_point(pos);
}

// ============================================================
//  QuadTree
// ============================================================

void QuadTree::_bind_methods() {
    ClassDB::bind_method(D_METHOD("recreate", "p_region_pos", "p_bounds", "p_parent", "p_self_idx"), &QuadTree::recreate, DEFVAL(nullptr), DEFVAL(0));
    ClassDB::bind_method(D_METHOD("deactivate"), &QuadTree::deactivate);
    ClassDB::bind_method(D_METHOD("get_parent"), &QuadTree::get_parent);
    ClassDB::bind_method(D_METHOD("get_tree_obj", "obj", "last_tick_pos"), &QuadTree::get_tree_obj);
    ClassDB::bind_method(D_METHOD("insert", "tree_obj", "tile"), &QuadTree::insert);
    ClassDB::bind_method(D_METHOD("erase_at", "obj", "tile"), &QuadTree::erase_at);
    ClassDB::bind_method(D_METHOD("retrieve_rect", "area", "team", "team_except", "method"), &QuadTree::retrieve_rect);
    ClassDB::bind_method(D_METHOD("retrieve_circle", "center", "radius", "team", "team_except", "method"), &QuadTree::retrieve_circle);
    ClassDB::bind_method(D_METHOD("retrieve_ring", "center", "radius_from", "radius_to", "team", "team_except", "method"), &QuadTree::retrieve_ring);
    ClassDB::bind_method(D_METHOD("clear"), &QuadTree::clear);
    ClassDB::bind_method(D_METHOD("is_empty"), &QuadTree::is_empty);
    ClassDB::bind_method(D_METHOD("draw_debug", "canvas_item", "color"), &QuadTree::draw_debug, DEFVAL(Color(1, 1, 1)));
}

QuadTree* QuadTree::recreate(const Vector2i& p_region_pos, const Rect2i& p_bounds, QuadTree* p_parent, int p_self_idx) {
    region_pos = p_region_pos;
    bounds = p_bounds;
    bounds_center = bounds.get_center();
    parent = p_parent;
    self_idx = p_self_idx;
    return this;
}

void QuadTree::deactivate() {
    Object* m = manager;
    parent = nullptr;
    for (int i = 0; i < 4; i++) {
        if (children[i].is_valid()) {
            children[i]->deactivate();
        }
        children[i].unref();
    }
    tree_objects.clear();
    child_count = 0;
    if (m) {
        QuadTreeManagerC* mgr = Object::cast_to<QuadTreeManagerC>(m);
        if (mgr) {
            mgr->_recycle_quad_tree(this);
        }
    }
}

QuadTree* QuadTree::get_parent() const {
    return parent;
}

Ref<TreeObject> QuadTree::get_tree_obj(Object* obj, const Vector2& last_tick_pos) {
    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid() && tree_obj->obj == obj) {
            return tree_obj;
        }
    }

    if (child_count) {
        int id = _pos_get_index(last_tick_pos);

        if (children[id].is_valid()) {
            Ref<TreeObject> tree_obj = children[id]->get_tree_obj(obj, last_tick_pos);
            if (tree_obj.is_valid()) return tree_obj;
        }

        for (int i = 0; i < 4; i++) {
            if (i != id && children[i].is_valid()) {
                Ref<TreeObject> tree_obj = children[i]->get_tree_obj(obj, last_tick_pos);
                if (tree_obj.is_valid()) return tree_obj;
            }
        }
    }

    return Ref<TreeObject>();
}

void QuadTree::insert(const Ref<TreeObject>& tree_obj, const Vector2i& tile) {
    if (child_count) {
        int idx = _pos_get_index(tree_obj->pos);
        if (idx == -1) return;

        if (!children[idx].is_valid()) {
            QuadTreeManagerC* mgr = Object::cast_to<QuadTreeManagerC>(manager);
            if (mgr) {
                Ref<QuadTree> child = mgr->_get_deactive();
                if (child.is_valid()) {
                    child->recreate(region_pos, _get_child_rect(idx), this, idx);
                    children[idx] = child;
                    child_count++;
                }
            }
        }

        if (children[idx].is_valid()) {
            children[idx]->insert(tree_obj, tile);
        }
        return;
    }
    _add_tree_object(tree_obj);
}

void QuadTree::erase_at(Object* obj, const Vector2i& tile) {
    if (_erase_here(obj)) return;

    if (child_count) {
        int idx = _pos_get_index(Call::tile_to_pos(tile));
        if (idx != -1) {
            if (children[idx].is_valid() && children[idx]->_erase_at_internal(obj, tile)) return;
        }

        for (int i = 0; i < 4; i++) {
            if (children[i].is_valid() && children[i]->_erase_at_internal(obj, tile)) return;
        }
    }
}

bool QuadTree::_erase_at_internal(Object* obj, const Vector2i& tile) {
    if (_erase_here(obj)) return true;

    if (child_count) {
        int idx = _pos_get_index(Call::tile_to_pos(tile));
        if (idx != -1) {
            if (children[idx].is_valid() && children[idx]->_erase_at_internal(obj, tile)) return true;
        }

        for (int i = 0; i < 4; i++) {
            if (children[i].is_valid() && children[i]->_erase_at_internal(obj, tile)) return true;
        }
    }

    return false;
}

bool QuadTree::_erase_here(Object* obj) {
    bool removed = false;
    int i = 0;
    while (i < (int)tree_objects.size()) {
        if (tree_objects[i].is_valid() && tree_objects[i]->obj == obj) {
            tree_objects.erase(tree_objects.begin() + i);
            removed = true;
        } else {
            i++;
        }
    }
    if (removed) _prune_if_empty();
    return removed;
}

void QuadTree::_prune_if_empty() {
    if (!is_empty()) return;

    QuadTree* parent_ref = get_parent();
    if (parent_ref) {
        parent_ref->_delete_child(self_idx, this);
    } else {
        QuadTreeManagerC* mgr = Object::cast_to<QuadTreeManagerC>(manager);
        if (mgr) {
            mgr->_delete_leaf(this);
        }
    }
}

void QuadTree::_split() {
    std::vector<Ref<TreeObject>> remain;
    while (!tree_objects.empty()) {
        Ref<TreeObject> tree_obj = tree_objects.back();
        tree_objects.pop_back();

        int idx = _pos_get_index(tree_obj->pos);
        if (idx == -1) {
            remain.push_back(tree_obj);
            continue;
        }

        if (!children[idx].is_valid()) {
            QuadTreeManagerC* mgr = Object::cast_to<QuadTreeManagerC>(manager);
            if (mgr) {
                Ref<QuadTree> child = mgr->_get_deactive();
                if (child.is_valid()) {
                    child->recreate(region_pos, _get_child_rect(idx), this, idx);
                    children[idx] = child;
                    child_count++;
                }
            }
        }

        if (children[idx].is_valid()) {
            children[idx]->_add_tree_object(tree_obj);
        }
    }

    tree_objects = remain;
    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid()) {
            tree_obj->current_quad_tree_bounds = bounds;
        }
    }
}

void QuadTree::_add_tree_object(const Ref<TreeObject>& tree_obj) {
    tree_objects.push_back(tree_obj);
    tree_obj->current_quad_tree_bounds = bounds;
    if (static_cast<int>(tree_objects.size()) > QuadTreeManagerC::MAX_OBJECTS && _can_split()) {
        _split();
    }
}

Rect2 QuadTree::_get_child_rect(int idx) const {
    Vector2 half = bounds.size * 0.5f;
    switch (idx) {
        case 0: return Rect2(bounds.position, half);
        case 1: return Rect2(bounds.position + Vector2(half.x, 0), half);
        case 2: return Rect2(bounds.position + Vector2(0, half.y), half);
        case 3: return Rect2(bounds.position + half, half);
        default: return Rect2();
    }
}

int QuadTree::_pos_get_index(const Vector2& p) const {
    bool left   = p.x <= bounds_center.x;
    bool right  = p.x >= bounds_center.x;
    bool top    = p.y <= bounds_center.y;
    bool bottom = p.y >= bounds_center.y;

    if (left && top)    return 0;
    if (right && top)   return 1;
    if (left && bottom) return 2;
    if (right && bottom)return 3;
    return -1;
}

bool QuadTree::_can_split() const {
    return bounds.size.x > ConstsC::get_real_region_size().x;
}

void QuadTree::_delete_child(int idx, const Ref<QuadTree>& child) {
    if (child.is_valid()) {
        child->deactivate();
    }
    children[idx].unref();
    child_count--;

    if (is_empty()) {
        QuadTree* parent_ref = get_parent();
        if (parent_ref) {
            parent_ref->_delete_child(self_idx, this);
        } else {
            QuadTreeManagerC* mgr = Object::cast_to<QuadTreeManagerC>(manager);
            if (mgr) {
                mgr->_delete_leaf(this);
            }
        }
    }
}

bool QuadTree::retrieve_rect(const Rect2& area, int team, int team_except, const Callable& method) {
    if (child_count) {
        for (int i = 0; i < 4; i++) {
            if (children[i].is_valid() && children[i]->bounds.intersects(area)) {
                if (children[i]->retrieve_rect(area, team, team_except, method)) return true;
            }
        }
        return false;
    }

    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid()) {
            if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && tree_obj->aabb.intersects(area)) {
                if (method.call(tree_obj->obj).booleanize()) return true;
            }
        }
    }
    return false;
}

bool QuadTree::retrieve_circle(const Vector2& center, double radius, int team, int team_except, const Callable& method) {
    if (child_count) {
        for (int i = 0; i < 4; i++) {
            if (children[i].is_valid() && Call::rect_circle_intersects(children[i]->bounds, radius, center)) {
                if (children[i]->retrieve_circle(center, radius, team, team_except, method)) return true;
            }
        }
        return false;
    }

    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid()) {
            if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && Call::rect_circle_intersects(tree_obj->aabb, radius, center)) {
                if (method.call(tree_obj->obj).booleanize()) return true;
            }
        }
    }
    return false;
}

bool QuadTree::retrieve_ring(const Vector2& center, double radius_from, double radius_to, int team, int team_except, const Callable& method) {
    if (child_count) {
        for (int i = 0; i < 4; i++) {
            if (children[i].is_valid() && Call::rect_ring_intersects(children[i]->bounds, radius_from, radius_to, center)) {
                if (children[i]->retrieve_ring(center, radius_from, radius_to, team, team_except, method)) return true;
            }
        }
        return false;
    }

    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid()) {
            if ((team == -1 || tree_obj->team == team) && tree_obj->team != team_except && Call::rect_ring_intersects(tree_obj->aabb, radius_from, radius_to, center)) {
                if (method.call(tree_obj->obj).booleanize()) return true;
            }
        }
    }
    return false;
}

void QuadTree::clear() {
    deactivate();
}

bool QuadTree::is_empty() const {
    return !child_count && tree_objects.empty();
}

void QuadTree::draw_debug(RID canvas_item, const Color& color) {
    RenderingServer* rs = RenderingServer::get_singleton();
    if (!rs) return;

    rs->canvas_item_add_rect(canvas_item, bounds.grow(-1), Color(color.r, color.g, color.b, 0.1f));

    for (int i = 0; i < 4; i++) {
        if (children[i].is_valid()) {
            children[i]->draw_debug(canvas_item, color);
        }
    }

    for (const Ref<TreeObject>& tree_obj : tree_objects) {
        if (tree_obj.is_valid() && tree_obj->obj) {
            rs->canvas_item_add_rect(canvas_item, tree_obj->aabb, Color(0.0f, 0.0f, 0.0f, 0.2f));
        }
    }
}

// ============================================================
//  QuadTreeManagerC
// ============================================================

void QuadTreeManagerC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "reserve_size"), &QuadTreeManagerC::init);

    ClassDB::bind_method(D_METHOD("add", "region", "next_tile", "obj"), &QuadTreeManagerC::add);
    ClassDB::bind_method(D_METHOD("_add", "region", "next_tile", "obj"), &QuadTreeManagerC::_add);
    ClassDB::bind_method(D_METHOD("delete", "big_region", "last_tile", "obj"), &QuadTreeManagerC::delete_);
    ClassDB::bind_method(D_METHOD("update_pos", "u", "current_pos", "last_tick_pos"), &QuadTreeManagerC::update_pos);

    ClassDB::bind_method(D_METHOD("retrieve_rect", "big_region", "area", "team", "team_except", "method"), &QuadTreeManagerC::retrieve_rect);
    ClassDB::bind_method(D_METHOD("retrieve_circle", "center", "radius", "team", "team_except", "method"), &QuadTreeManagerC::retrieve_circle);
    ClassDB::bind_method(D_METHOD("retrieve_ring", "center", "radius_from", "radius_to", "team", "team_except", "method"), &QuadTreeManagerC::retrieve_ring);

    ClassDB::bind_method(D_METHOD("clear"), &QuadTreeManagerC::clear);
    ClassDB::bind_method(D_METHOD("draw_debug", "canvas_item", "color"), &QuadTreeManagerC::draw_debug, DEFVAL(Color(1, 1, 1)));
    ClassDB::bind_method(D_METHOD("_recycle_quad_tree", "quad_tree"), &QuadTreeManagerC::_recycle_quad_tree);

    // Sync handlers: each_sync_to passes target LAST
    ClassDB::bind_method(D_METHOD("_on_sync_add", "region", "next_tile", "obj", "target"), &QuadTreeManagerC::_on_sync_add);
    ClassDB::bind_method(D_METHOD("_on_sync_delete", "big_region", "last_tile", "obj", "target"), &QuadTreeManagerC::_on_sync_delete);
    ClassDB::bind_method(D_METHOD("_on_sync_clear", "target"), &QuadTreeManagerC::_on_sync_clear);
}

void QuadTreeManagerC::init(int p_reserve_size) {
    reserve_size = p_reserve_size;
    for (int i = 0; i < reserve_size; i++) {
        _add_deactive();
    }
}

void QuadTreeManagerC::_add_deactive() {
    Ref<QuadTree> qt;
    qt.instantiate();
    qt->manager = this;
    qt->recreate(Vector2i(), Rect2i());
    deactive_trees.push_back(qt);
}

Ref<QuadTree> QuadTreeManagerC::_get_deactive() {
    ERR_FAIL_COND_V_MSG(deactive_trees.empty(), Ref<QuadTree>(), "QuadTreeManagerC: no deactive trees available");
    Ref<QuadTree> qt = deactive_trees.back();
    deactive_trees.pop_back();
    return qt;
}

// ------------------------------------------------------------------
//  add / _add / _add_internal
// ------------------------------------------------------------------

void QuadTreeManagerC::add(const Vector2i& region, const Vector2i& next_tile, Object* obj) {
    if (obj->call("dead").booleanize()) return;
    _add(region, next_tile, obj);
}

void QuadTreeManagerC::_add(const Vector2i& region, const Vector2i& next_tile, Object* obj) {
    Ref<TreeObject> tree_obj;
    tree_obj.instantiate();
    tree_obj->recreate(obj, static_cast<int>(obj->get(StringName("team"))));

    _add_internal(region, next_tile, tree_obj);

    each_sync_to(Callable(this, "_on_sync_add").bind(Variant(region), Variant(next_tile), Variant(obj)));
}

void QuadTreeManagerC::_add_internal(const Vector2i& region, const Vector2i& next_tile, const Ref<TreeObject>& tree_obj) {
    auto it = tree.find(region);
    Ref<QuadTree> quad_tree;
    if (it != tree.end() && it->second.is_valid()) {
        quad_tree = it->second;
    } else {
        quad_tree = _get_deactive();
        if (quad_tree.is_valid()) {
            quad_tree->recreate(region, Call::get_big_region_rect(region));
            tree[region] = quad_tree;
        }
    }

    if (quad_tree.is_valid()) {
        quad_tree->insert(tree_obj, next_tile);
    }
}

// ------------------------------------------------------------------
//  delete
// ------------------------------------------------------------------

void QuadTreeManagerC::delete_(const Vector2i& big_region, const Vector2i& last_tile, Object* obj) {
    auto it = tree.find(big_region);
    if (it != tree.end() && it->second.is_valid()) {
        it->second->erase_at(obj, last_tile);
    }

    each_sync_to(Callable(this, "_on_sync_delete").bind(Variant(big_region), Variant(last_tile), Variant(obj)));
}

// ------------------------------------------------------------------
//  update_pos
// ------------------------------------------------------------------

void QuadTreeManagerC::update_pos(Object* u, const Vector2& current_pos, const Vector2& last_tick_pos) {
    if (current_pos == last_tick_pos) return;

    Vector2i last_region = Call::pos_to_big_region(last_tick_pos);
    Vector2i last_tile = Call::pos_to_tile(last_tick_pos);
    Vector2i next_tile = Call::pos_to_tile(current_pos);

    Variant p_pos_var = u->call("get_pos");
    Vector2 p_pos = p_pos_var;
    Variant p_size_var = u->call("get_size");
    Vector2 p_size = p_size_var;

    _update_pos(u, last_tick_pos, last_region, last_tile, next_tile, p_pos, p_size);
}

void QuadTreeManagerC::_update_pos(Object* u, const Vector2& last_tick_pos, const Vector2i& last_region, const Vector2i& last_tile, const Vector2i& next_tile, const Vector2& p_pos, const Vector2& p_size) {
    auto it = tree.find(last_region);
    if (it == tree.end() || !it->second.is_valid()) return;

    Ref<QuadTree> last_quad_tree = it->second;
    Ref<TreeObject> last_tree_obj = last_quad_tree->get_tree_obj(u, last_tick_pos);
    if (last_tree_obj.is_null()) return;

    last_tree_obj->update(p_pos, p_size);
    if (!last_tree_obj->is_in_tree_bounds()) {
        delete_(last_region, last_tile, u);
        _add(Call::tile_to_big_region(next_tile), next_tile, u);
    }
}

// ------------------------------------------------------------------
//  Retrieval
// ------------------------------------------------------------------

bool QuadTreeManagerC::retrieve_rect(const Vector2i& big_region, const Rect2& area, int team, int team_except, const Callable& method) {
    auto it = tree.find(big_region);
    if (it != tree.end() && it->second.is_valid()) {
        return it->second->retrieve_rect(area, team, team_except, method);
    }
    return false;
}

bool QuadTreeManagerC::retrieve_circle(const Vector2& center, double radius, int team, int team_except, const Callable& method) {
    return Call::circle_big_regions(center, radius, [&](Vector2i region) -> bool {
        auto it = tree.find(region);
        if (it != tree.end() && it->second.is_valid()) {
            return it->second->retrieve_circle(center, radius, team, team_except, method);
        }
        return false;
    });
}

bool QuadTreeManagerC::retrieve_ring(const Vector2& center, double radius_from, double radius_to, int team, int team_except, const Callable& method) {
    return Call::ring_big_regions(center, radius_from, radius_to, [&](Vector2i region) -> bool {
        auto it = tree.find(region);
        if (it != tree.end() && it->second.is_valid()) {
            return it->second->retrieve_ring(center, radius_from, radius_to, team, team_except, method);
        }
        return false;
    });
}

// ------------------------------------------------------------------
//  Clear / debug
// ------------------------------------------------------------------

void QuadTreeManagerC::clear() {
    for (auto& pair : tree) {
        if (pair.second.is_valid()) {
            pair.second->deactivate();
        }
    }
    tree.clear();
    deactive_trees.clear();

    each_sync_to(Callable(this, "_on_sync_clear"));
}

void QuadTreeManagerC::_delete_leaf(const Ref<QuadTree>& quad_tree) {
    quad_tree->deactivate();
    tree.erase(quad_tree->region_pos);
}

void QuadTreeManagerC::_recycle_quad_tree(const Ref<QuadTree>& quad_tree) {
    deactive_trees.push_back(quad_tree);
}

void QuadTreeManagerC::draw_debug(RID canvas_item, const Color& color) {
    for (auto& pair : tree) {
        if (pair.second.is_valid()) {
            pair.second->draw_debug(canvas_item, color);
        }
    }
}

// ------------------------------------------------------------------
//  Sync propagation handlers
//  each_sync_to passes Object* target as the LAST arg (after bound args)
// ------------------------------------------------------------------

void QuadTreeManagerC::_on_sync_add(const Vector2i& region, const Vector2i& next_tile, Object* obj, Object* target) {
    QuadTreeManagerC* to = Object::cast_to<QuadTreeManagerC>(target);
    if (!to) return;
    to->add_sync_call(Callable(to, "_add").bind(Variant(region), Variant(next_tile), Variant(obj)));
}

void QuadTreeManagerC::_on_sync_delete(const Vector2i& big_region, const Vector2i& last_tile, Object* obj, Object* target) {
    QuadTreeManagerC* to = Object::cast_to<QuadTreeManagerC>(target);
    if (!to) return;
    to->add_sync_call(Callable(to, "delete").bind(Variant(big_region), Variant(last_tile), Variant(obj)));
}

void QuadTreeManagerC::_on_sync_clear(Object* target) {
    QuadTreeManagerC* to = Object::cast_to<QuadTreeManagerC>(target);
    if (!to) return;
    to->add_sync_call(Callable(to, "clear"));
}
