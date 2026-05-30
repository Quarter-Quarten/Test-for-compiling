#pragma once
#include "ecs/component_register"

#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

struct Velocity {
	Vector2 velocity;
}
REGISTER_COMPONENT(Velocity)