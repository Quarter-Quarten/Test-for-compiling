#pragma once

#include "ecs/chunk.h"

#include <bitset>

class Archetype final {
public:
	size_t entity_stride;
	size_t capacity;

	Archetype(
		std::bitset<kMaxComponentCount> p_component_mask,
		std::vector<ComponentMeta*> p_components
	);
	
	Archetype(const Archetype&) = delete;
	Archetype& operator=(const Archetype&) = delete;
	Archetype(Archetype&&) = default;
	Archetype& operator=(Archetype&&) = default;
	
	// 添加一个实体
    void add_entity();
    // 删除指定索引的实体
    void remove_entity(std::uint16_t chunk_index, std::uint16_t index);

private:
	// 位标记 一位的0/1表示该Archetype是否有该组件 用于检索
	std::bitset<kMaxComponentCount> component_mask;
	// 紧密数组 所有的组件
	std::vector<ComponentMeta*> components;

	// Chunk中的数据以SoA排布方式存储 此数组缓存每种组件的起始偏移
	std::vector<uint16_t> offsets;
	
	// Chunk本体在全局连续数组中 Chunk对齐为16kb
	std::vector<Chunk*> chunks;
};
