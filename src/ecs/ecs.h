#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "component_register.h"

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
    explicit Chunk(Archetype* p_type);

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