#include "tools/predict/predict.hpp"
#include <cmath>
#include <godot_cpp/core/class_db.hpp>

Vector2 Predict::intercept_c(const Vector2 &from, const Vector2 &to, const Vector2 &e_vel, double bs, bool predict, double predict_factor) {
    if (e_vel == Vector2() || !predict) {
        return to;
    }

    Vector2 d = to - from;

    double A = e_vel.length_squared() - bs * bs;
    double B = 2.0 * d.dot(e_vel);
    double C = d.length_squared();

    double discriminant = B * B - 4.0 * A * C;

    if (discriminant < 0.0) {
        return to;
    }

    double t1 = (A != 0.0) ? (-B + sqrt(discriminant)) / (2.0 * A) : 0.0;
    double t2 = (A != 0.0) ? (-B - sqrt(discriminant)) / (2.0 * A) : 0.0;

    double t;
    if (t1 > 0.0) {
        t = t1;
    } else if (t2 > 0.0) {
        t = t2;
    } else {
        return to;
    }

    return to + e_vel * t * predict_factor;
}

void Predict::_bind_methods() {
    ClassDB::bind_static_method("Predict", D_METHOD("intercept", "from", "to", "e_vel", "bullet_speed", "predict", "predict_factor"), &Predict::intercept_c, DEFVAL(true), DEFVAL(1.0));
}
