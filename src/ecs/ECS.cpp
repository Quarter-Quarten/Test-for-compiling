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
