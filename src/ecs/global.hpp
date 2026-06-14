#pragma once
#include "ecs/lib/flecs.h"
#include "ecs/tools/id_pool.hpp"

namespace ecs {
	namespace global {
		inline IDPool<int> particle_ids;
    	inline flecs::entity player;
	    inline void init_global() {
	        particle_ids.clear();
	        player = {};
	    }
	}
}