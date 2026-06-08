#pragma once
#include "tools/my_multi_mesh_c/my_multi_mesh_c.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/color.hpp>
#include <vector>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

using namespace godot;

class ParticleBase : public RefCounted {
    GDCLASS(ParticleBase, RefCounted);
public:
    ParticleBase() = default;
    ParticleBase(int p_count_max, Node* p_controller);
    virtual ~ParticleBase() = default;
    void init(int p_count_max, Node* p_controller);
    void update(double delta, MyMultiMeshC* multimesh);
protected:
    Node* controller;

    int count_max = 0;
    Vector2 light_radius_mul = Vector2(2.0, 2.0);
    float light_strength = 0.3;

    std::vector<Vector2> positions;
    std::vector<Vector2> scales;
    std::vector<float> lifetimes;
    std::vector<float> times;

    static void _bind_methods();

    int create_particle(Vector2 pos, float lifetime, Vector2 size);
    virtual void _update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) = 0;
private:
    std::vector<short> free_indexes;
    std::vector<short> active_indexes;

    inline int get_free_idx() {
        if (free_indexes.empty()) {
            UtilityFunctions::printerr("No free particle slots!");
            return -1;
        }
        int idx = free_indexes.back();
        free_indexes.pop_back();
        return idx;
    }
    inline void delete_particle(int idx) {
        free_indexes.push_back(idx);
    }
};

class ParticlesC : public ParticleBase {
    GDCLASS(ParticlesC, ParticleBase);
protected:
    std::vector<Color> colors;
    std::vector<Color> color_tos;
    std::vector<float> color_eases;
    
    std::vector<Vector2> pos_tos;
    std::vector<float> pos_eases;
    
    std::vector<float> rotations;

    std::vector<Vector2> scale_tos;
    std::vector<float> scale_eases;

    static void _bind_methods();
    void _update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) override;

    void _add_light(Transform2D transform, const Color& color);
public:
    ParticlesC() = default;
    ParticlesC(int p_count_max, Node* p_controller);
    ~ParticlesC() = default;
    void init(int p_count_max, Node* p_controller);

    void create(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone
    );
};

class ParticleTorusC : public ParticlesC {
    GDCLASS(ParticleTorusC, ParticlesC);
protected:
    std::vector<float> width_froms;
    std::vector<float> width_tos;

    static void _bind_methods();
    void _update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) override;
public:
    ParticleTorusC() = default;
    ParticleTorusC(int p_count_max, Node* p_controller);
    ~ParticleTorusC() = default;
    void init(int p_count_max, Node* p_controller);

    void create(
        Vector2 pos, float rota, float length, float len_rand, float pos_ease,
        Color col_from, Color col_to, float col_ease,
        float lifetime, int count,
        Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
        bool radiate, float radiate_angle, float cone,
        float width_from = 0.0, float width_to = 0.0
    );
};













// class WorldLabelsC : public ParticleBase {
//     GDCLASS(WorldLabelsC, ParticleBase);
// protected:
//     const static float DISAPPEAR_TIME = 0.5;

//     std::vector<Vector2> pos_tos;

//     static void _bind_methods();
// public:
//     WorldLabelsC() = default;
//     WorldLabelsC(int p_count_max, Node* p_controller);
//     ~WorldLabelsC() = default;
//     void init(int p_count_max, Node* p_controller);
// };



// class DamageLabelsC : public WorldLabelsC {
//     GDCLASS(DamageLabelsC, WorldLabelsC);
// private:
//     static PackedColorArray colors;

//     const static float POS_EASE = 0.3;
//     const static float LIFETIME = 1.0;
//     const static Vector2 BASIC_SCALE = Vector2(35.0, 35.0);
//     char get_color_sign(float damage, bool crit, bool is_player, bool is_mana);
// protected:
//     std::vector<float> damages;
//     std::vector<char> color_signs;

//     static void _bind_methods();
//     void _update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) override;
// public:
//     DamageLabelsC() = default;
//     DamageLabelsC(int p_count_max, Node* p_controller, const PackedColorArray& p_colors);
//     ~DamageLabelsC() = default;
//     void init(int p_count_max, Node* p_controller, const PackedColorArray& p_colors);

//     void create(float damage, Vector2 position, bool crit, bool is_player = false, bool is_mana = false);
// };