#pragma once
#include "ecs/component_register"

struct Health {
	float health;
}
REGISTER_COMPONENT(Health)