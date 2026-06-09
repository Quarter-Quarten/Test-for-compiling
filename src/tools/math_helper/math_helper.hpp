#pragma once

#include <godot_cpp/classes/object.hpp>   // 正确的头文件

class MathHelper : public godot::Object {
    GDCLASS(MathHelper, godot::Object);   // 通常放在 public 区域

protected:
    static double random_sign_double();
    static void _bind_methods();

public:
    static int random_sign();
    MathHelper() = default;
};