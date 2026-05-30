#pragma once

#include <memory>
#include <cstring>   // memcpy
#include <cassert>

#include "ecs/ecs.h"

inline constexpr size_t kChunkSize = 16384;

struct Archetype;    // 前向声明

struct Chunk {
    Archetype* archetype;
    // 缓冲区：前 sizeof(uint32_t)*2 存放 count / capacity，之后为实体数据
    std::array<uint8_t, kChunkSize - sizeof(Archetype*)> data;

    Chunk(Archetype& p_archetype);

    // 实体数量
    uint32_t count() const {
        return *reinterpret_cast<const uint32_t*>(data.data());
    }

    // 最大容量
    uint32_t capacity() const {
        return *(reinterpret_cast<const uint32_t*>(data.data()) + 1);
    }

    // 获取实体数据起始指针（所有组件连续存放）
    void* get_entity_data(uint32_t index) {
        assert(index < count());
        const size_t header_size = 2 * sizeof(uint32_t);
        return data.data() + header_size + index * archetype_entity_size();
    }

    // 获取某个实体的指定组件
    template<typename T>
    T* get_component(uint32_t index, comp::ComponentID id) {
        void* entity_ptr = get_entity_data(index);
        size_t offset = archetype_component_offset(id);
        return reinterpret_cast<T*>(static_cast<uint8_t*>(entity_ptr) + offset);
    }

    // 添加实体，返回新实体索引
    uint32_t add_entity();

    // 删除实体（将末尾实体移动到 index 处）
    void remove_entity(uint32_t index);

private:
    // 辅助：从 Archetype 获取实体大小与组件偏移
    size_t archetype_entity_size() const;
    size_t archetype_component_offset(comp::ComponentID id) const;

    // 读写元数据
    uint32_t& ref_count() {
        return *reinterpret_cast<uint32_t*>(data.data());
    }
    uint32_t& ref_capacity() {
        return *(reinterpret_cast<uint32_t*>(data.data()) + 1);
    }
};





struct Archetype {
    std::bitset<comp::kMaxComponentCount> component_mask;
    std::vector<std::unique_ptr<Chunk>> chunks;

    // 所含组件的 ID 列表（按 ID 排序）
    std::vector<comp::ComponentID> component_ids;
    // 每个组件在实体内的偏移量
    std::array<size_t, comp::kMaxComponentCount> component_offsets{};
    // 单个实体总大小（已对齐）
    size_t entity_size = 0;

    Archetype() = default;

    // 根据 component_mask 初始化布局信息
    void initialize_layout() {
        // 收集已设置的组件 ID
        for (size_t i = 0; i < comp::kMaxComponentCount; ++i) {
            if (component_mask.test(i)) {
                component_ids.push_back(static_cast<comp::ComponentID>(i));
            }
        }
        // 计算偏移与总大小
        size_t current_offset = 0;
        size_t max_align = 1;
        for (comp::ComponentID id : component_ids) {
            const auto& meta = comp::ComponentRegister::get_meta(id);
            // 对齐
            size_t aligned = (current_offset + meta.align - 1) & ~(meta.align - 1);
            component_offsets[id] = aligned;
            current_offset = aligned + meta.size;
            if (meta.align > max_align) max_align = meta.align;
        }
        // 整体对齐
        entity_size = (current_offset + max_align - 1) & ~(max_align - 1);
    }

    // 创建一个新实体（添加到有空闲的 Chunk，或新建 Chunk）
    std::pair<Chunk*, uint32_t> create_entity() {
        // 寻找第一个未满的 Chunk
        for (auto& chunk : chunks) {
            if (chunk->count() < chunk->capacity()) {
                uint32_t idx = chunk->add_entity();
                return {chunk.get(), idx};
            }
        }
        // 所有 Chunk 均满，新建一个
        auto new_chunk = std::make_unique<Chunk>(*this);
        Chunk* ptr = new_chunk.get();
        chunks.push_back(std::move(new_chunk));
        uint32_t idx = ptr->add_entity();
        return {ptr, idx};
    }

    // 销毁一个实体（需要知道其所在 Chunk 与索引）
    void destroy_entity(Chunk* chunk, uint32_t index) {
        chunk->remove_entity(index);
        // 若 Chunk 变空，可选择是否回收（此处保留空 Chunk 以便重用）
    }
};

// ===== Chunk 成员函数实现 =====

Chunk::Chunk(Archetype& p_archetype) : archetype(&p_archetype) {
    // 计算实体容量
    const size_t header_size = 2 * sizeof(uint32_t);          // count + capacity
    size_t available = data.size() - header_size;
    size_t ent_size = p_archetype.entity_size;
    uint32_t cap = static_cast<uint32_t>(available / ent_size);
    
    // 初始化元数据
    ref_count() = 0;
    ref_capacity() = cap;
}

uint32_t Chunk::add_entity() {
    assert(count() < capacity());
    uint32_t index = ref_count();
    void* entity_ptr = get_entity_data(index);

    // placement‑new 构造所有组件
    for (comp::ComponentID id : archetype->component_ids) {
        const auto& meta = comp::ComponentRegister::get_meta(id);
        size_t offset = archetype_component_offset(id);
        uint8_t* comp_ptr = static_cast<uint8_t*>(entity_ptr) + offset;
        meta.constructor(comp_ptr);   // 调用默认构造函数
    }

    ref_count()++;
    return index;
}

void Chunk::remove_entity(uint32_t index) {
    assert(index < count());
    uint32_t last = count() - 1;

    // 析构待删除实体的所有组件
    void* entity_ptr = get_entity_data(index);
    for (comp::ComponentID id : archetype->component_ids) {
        const auto& meta = comp::ComponentRegister::get_meta(id);
        size_t offset = archetype_component_offset(id);
        uint8_t* comp_ptr = static_cast<uint8_t*>(entity_ptr) + offset;
        meta.destructor(comp_ptr);
    }

    // 如果删除的不是最后一个实体，将最后一个实体整体移动到空位
    if (index != last) {
        void* last_entity_ptr = get_entity_data(last);
        memcpy(entity_ptr, last_entity_ptr, archetype->entity_size);
    }

    ref_count()--;
}

size_t Chunk::archetype_entity_size() const {
    return archetype->entity_size;
}

size_t Chunk::archetype_component_offset(comp::ComponentID id) const {
    return archetype->component_offsets[id];
}