#pragma once
#include "ecs/component_register"

#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

struct Position {
	Vector2 position;
}
REGISTER_COMPONENT(Position)