#pragma once
#include "ecs/lib/flecs.h"
#include "ecs/tools/id_pool.hpp"

namespace ecs {
    class ECSWorld;
    
    
	namespace global {
		inline IDPool<int> particle_ids;
    	inline flecs::entity _player;
    	inline ECSWorld* ecs_world;
    	
	    inline void init_global(ECSWorld* p_ecs_world) {
	        particle_ids.clear();
	        ecs_world = p_ecs_world;
	    }
	}
}