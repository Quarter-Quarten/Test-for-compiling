#pragma once

#include "ecs/chunk.h"

#include <bitset>

struct Archetype {
	std::bitset<kMaxComponentCount> component_mask;
	std::vector<ComponentMeta*> components;
	
	size_t entity_stride;
	size_t capacity;
	
	// Chunk本体在连续数组中 Chunk对齐为16kb
	std::vector<Chunk*> chunks;
	
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
	
	
};
