#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

class Predict : public Object {
    GDCLASS(Predict, Object);

protected:
    static void _bind_methods();

public:
    static Vector2 intercept_c(const Vector2 &from, const Vector2 &to, const Vector2 &e_vel, double bs, bool predict = true, double predict_factor = 1.0);
    Predict() = default;
};
