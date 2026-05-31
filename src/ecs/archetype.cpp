#include "ecs/archetype.h"

Archetype::Archetype(
	std::bitset<kMaxComponentCount> p_component_mask,
	std::vector<ComponentMeta*> p_components
) : component_mask(p_component_mask), components(p_components) {

}

void add_entity() {

}
void remove_entity(std::uint16_t chunk_index, std::uint16_t index) {

}


