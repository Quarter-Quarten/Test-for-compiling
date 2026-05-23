#include "ecs/ECS.h"
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

