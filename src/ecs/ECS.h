#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/classes/object.hpp>

using namespace godot;

// ---------- 实体 ----------
struct Entity {
    uint32_t id = 0;
    uint32_t generation = 0; // 用于检测实体是否存活
    bool operator==(const Entity& other) const {
        return id == other.id && generation == other.generation;
    }
};

// ---------- 组件类型ID ----------
using ComponentTypeID = size_t;
template<typename T>
ComponentTypeID GetComponentTypeID() {
    static const ComponentTypeID id = std::hash<std::string>()(typeid(T).name());
    return id;
}

// ---------- 组件定义 ----------
struct Position {
    Vector2 value;
};
struct Velocity {
    Vector2 value;
};

// ---------- Chunk：存储固定数量实体的组件数据 ----------
static constexpr size_t CHUNK_CAPACITY = 64;

struct Archetype; // 前置声明

struct Chunk {
    Archetype* archetype = nullptr;
    size_t count = 0;                        // 当前存活的实体数
    std::vector<uint8_t> data;               // 所有组件连续存储
    std::vector<Entity> entities;            // 实体句柄，与组件一一对应

    // 根据组件类型获取其在该Chunk中的偏移（字节）
    size_t get_offset(ComponentTypeID type) const;
    // 获取第index个实体的某组件指针
    template<typename T>
    T* get_component(size_t index);
};

// ---------- Archetype：组件组合的模板 ----------
struct Archetype {
    std::vector<ComponentTypeID> component_types; // 排序后的组件ID列表
    std::vector<size_t> component_offsets;        // 在Chunk::data中的偏移
    std::vector<Chunk*> chunks;                   // 存储该原型的块链表

    // 检查该原型是否包含指定组件类型集合
    bool has_components(const std::vector<ComponentTypeID>& types) const;
};

// ---------- ECSWorld：ECS核心管理类 ----------
class ECSWorld : public Object {
public:
    ECSWorld();
    ~ECSWorld();

    // 创建一个实体，并添加指定组件（用默认值初始化）
    Entity create_entity(const std::vector<ComponentTypeID>& component_types);
    // 销毁实体
    void destroy_entity(Entity entity);

    // 获取实体组件
    template<typename T>
    T* get_component(Entity entity);

    // 获取所有匹配组件集合的原型（用于系统遍历）
    std::vector<Archetype*> query_archetypes(const std::vector<ComponentTypeID>& types);

private:
    std::vector<Entity> entity_pool;   // 实体池，索引为id
    std::vector<Archetype*> archetypes;
    std::vector<Chunk*> chunks;
    // 内部查找或创建原型
    Archetype* get_or_create_archetype(const std::vector<ComponentTypeID>& types);
    // 将实体从旧块移除（压缩移动最后一个实体）
    void remove_from_chunk(Entity entity);
};

// ---------- 移动系统 ----------
class MovementSystem {
public:
    void update(ECSWorld& world, float delta);
};




