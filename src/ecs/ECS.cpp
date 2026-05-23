#include "tools/ecs/ecs.h"
#include <algorithm>
#include <stdexcept>

// ---------- Chunk 实现 ----------
size_t Chunk::get_offset(ComponentTypeID type) const {
    const auto& offsets = archetype->component_offsets;
    const auto& types = archetype->component_types;
    for (size_t i = 0; i < types.size(); ++i) {
        if (types[i] == type)
            return offsets[i];
    }
    return SIZE_MAX; // 未找到
}

template<typename T>
T* Chunk::get_component(size_t index) {
    size_t offset = get_offset(GetComponentTypeID<T>());
    if (offset == SIZE_MAX) return nullptr;
    return reinterpret_cast<T*>(data.data() + index * archetype->component_offsets.back() + offset);
}
// 显式实例化常用类型（避免链接错误）
template Position* Chunk::get_component<Position>(size_t);
template Velocity* Chunk::get_component<Velocity>(size_t);

// ---------- Archetype 实现 ----------
bool Archetype::has_components(const std::vector<ComponentTypeID>& types) const {
    for (auto t : types) {
        if (std::find(component_types.begin(), component_types.end(), t) == component_types.end())
            return false;
    }
    return true;
}

// ---------- ECSWorld 实现 ----------
ECSWorld::ECSWorld() {}
ECSWorld::~ECSWorld() {
    for (auto c : chunks) delete c;
    for (auto a : archetypes) delete a;
}

Archetype* ECSWorld::get_or_create_archetype(const std::vector<ComponentTypeID>& types) {
    // 排序保证原型唯一性
    auto sorted_types = types;
    std::sort(sorted_types.begin(), sorted_types.end());
    for (auto a : archetypes) {
        if (a->component_types == sorted_types)
            return a;
    }
    // 新建原型
    Archetype* arch = new Archetype();
    arch->component_types = sorted_types;
    size_t total_size = 0;
    // 计算每个组件的大小及偏移
    for (auto type : sorted_types) {
        arch->component_offsets.push_back(total_size);
        // 这里硬编码了组件大小，可以改为用map存储
        if (type == GetComponentTypeID<Position>()) total_size += sizeof(Position);
        else if (type == GetComponentTypeID<Velocity>()) total_size += sizeof(Velocity);
        else throw std::runtime_error("Unknown component type");
    }
    archetypes.push_back(arch);
    return arch;
}

Entity ECSWorld::create_entity(const std::vector<ComponentTypeID>& component_types) {
    Archetype* arch = get_or_create_archetype(component_types);
    // 寻找有空位的Chunk
    Chunk* target = nullptr;
    for (auto c : arch->chunks) {
        if (c->count < CHUNK_CAPACITY) {
            target = c;
            break;
        }
    }
    if (!target) {
        target = new Chunk();
        target->archetype = arch;
        size_t component_data_size = 0;
        // 最后一个偏移 + 最后一个组件大小 = 每个实体的总组件数据大小
        component_data_size = arch->component_offsets.back() +
            (arch->component_types.back() == GetComponentTypeID<Position>() ? sizeof(Position) : sizeof(Velocity));
        target->data.resize(CHUNK_CAPACITY * component_data_size, 0);
        target->entities.reserve(CHUNK_CAPACITY);
        arch->chunks.push_back(target);
        chunks.push_back(target);
    }

    // 分配实体ID
    Entity entity;
    if (entity_pool.empty() || entity_pool.back().id == UINT32_MAX) {
        entity.id = entity_pool.size();
        entity.generation = 1;
        entity_pool.push_back(entity);
    } else {
        // 简单起见，不重用ID，直接 push
        entity.id = entity_pool.size();
        entity.generation = 1;
        entity_pool.push_back(entity);
    }

    size_t row = target->count++;
    target->entities.push_back(entity);
    // 默认初始化组件（可扩展为从参数传入）
    Position* pos = target->get_component<Position>(row);
    if (pos) new (pos) Position();
    Velocity* vel = target->get_component<Velocity>(row);
    if (vel) new (vel) Velocity();

    return entity;
}

void ECSWorld::destroy_entity(Entity entity) {
    // 实际项目中应从Chunk移除，这里简化略过
}

template<typename T>
T* ECSWorld::get_component(Entity entity) {
    // 遍历所有原型和块查找（实际应缓存实体位置）
    for (auto arch : archetypes) {
        for (auto chunk : arch->chunks) {
            for (size_t i = 0; i < chunk->count; ++i) {
                if (chunk->entities[i] == entity) {
                    return chunk->get_component<T>(i);
                }
            }
        }
    }
    return nullptr;
}
template Position* ECSWorld::get_component<Position>(Entity);
template Velocity* ECSWorld::get_component<Velocity>(Entity);

std::vector<Archetype*> ECSWorld::query_archetypes(const std::vector<ComponentTypeID>& types) {
    std::vector<Archetype*> result;
    for (auto a : archetypes) {
        if (a->has_components(types))
            result.push_back(a);
    }
    return result;
}

// ---------- MovementSystem 实现 ----------
void MovementSystem::update(ECSWorld& world, float delta) {
    std::vector<ComponentTypeID> query = { GetComponentTypeID<Position>(), GetComponentTypeID<Velocity>() };
    auto archetypes = world.query_archetypes(query);
    for (auto arch : archetypes) {
        for (auto chunk : arch->chunks) {
            for (size_t i = 0; i < chunk->count; ++i) {
                Position* pos = chunk->get_component<Position>(i);
                Velocity* vel = chunk->get_component<Velocity>(i);
                if (pos && vel) {
                    pos->value += vel->value * delta;
                }
            }
        }
    }
}