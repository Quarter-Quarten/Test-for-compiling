#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "ecs/component_register.h"

inline constexpr size_t kChunkSize = 16384;

struct Archetype;   // 前向声明

struct Chunk {
    // 归属的 Archetype（共享的布局信息）
    Archetype* type;

    // 数据缓冲区：
    // - [0..1] 字节：entity_count (uint16_t)
    // - [2..3] 字节：capacity     (uint16_t)
    // - 剩余空间：实体数据区
    std::array<std::uint8_t, kChunkSize - sizeof(Archetype*)> data;

    // ---------- 构造函数 ----------
    explicit Chunk() = default;
    
    recreate(Archetype* p_type);

	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	Chunk(Chunk&&) = default;
	Chunk& operator=(Chunk&&) = default;

    // ---------- 公共接口 ----------
    // 实体容量与计数（只读）
    std::uint16_t entity_count() const noexcept;
    std::uint16_t capacity() const noexcept;

    // 添加一个实体，返回新实体的索引
    std::uint16_t add_entity();

    // 删除指定索引的实体（将末尾实体移动到空缺位置）
    void remove_entity(std::uint16_t index);

    // 获取实体数据区的起始指针
    void* get_entity_data(std::uint16_t index) const;

    // 获取某个实体指定组件的指针（模板版本）
    template<typename T>
    T* get_component(std::uint16_t index, comp::ComponentID id);

private:
    // ---------- 内部辅助 ----------
    // 读写头部元数据
    std::uint16_t& ref_entity_count();
    std::uint16_t& ref_capacity();
    const std::uint16_t& ref_entity_count() const;
    const std::uint16_t& ref_capacity() const;

    // 缓冲区中实体存储区的起始地址
    void* entities_begin() const;

    // 从 Archetype 获取单个实体的大小及组件偏移
    std::size_t entity_size() const;
    std::size_t component_offset(comp::ComponentID id) const;
};






// 全局Chunk管理器
inline constexpr size_t kChunkSetSize = 64;
class ChunkSet {
    std::array<Chunk, kChunkSetSize> chunks;
    size_t next_id = 0;
public:
	ChunkSet() = default;
    Chunk* get_next_chunk() {
        if (next_id >= kChunkSetSize) return nullptr;
        return &chunks[next_id++];
    }
};

class ChunkAllocator {
    std::vector<std::unique_ptr<ChunkSet>> chunk_sets_;
    std::mutex mutex_;
public:
    Chunk* get_next_chunk() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (chunk_sets_.size()) {
        	if (Chunk* c = chunk_sets_.back()->get_next_chunk()) {
        		return c
			}
        }
        
        // 没有空闲 ChunkSet，创建新的
        auto new_cs = std::make_unique<ChunkSet>();
        Chunk* c = new_cs->get_next_chunk();
        chunk_sets_.push_back(std::move(new_cs));
        return c;
    }
};

inline ChunkAllocator& get_chunk_allocator() {
    static ChunkAllocator allocator;
    return allocator;
}

inline Chunk* get_next_chunk() {
    return get_chunk_allocator().get_next_chunk();
}