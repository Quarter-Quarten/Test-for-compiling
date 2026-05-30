#include "tools/math_helper/math_helper.h"
#include <random>
#include <godot_cpp/core/class_db.hpp>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 1);

double MathHelper::random_sign_double() {
	return dis(gen) == 0 ? -1.0 : 1.0;
}

int MathHelper::random_sign() {
    return dis(gen);
}

void MathHelper::_bind_methods() {
    // 绑定静态方法到 Godot
    godot::ClassDB::bind_static_method("MathHelper", godot::D_METHOD("random_sign"), &MathHelper::random_sign);
}
