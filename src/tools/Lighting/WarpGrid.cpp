#include "tools/Lighting/WarpGrid.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

#include <algorithm>

using namespace godot;

static int _wrapi(int p_value, int p_min, int p_max) {
	const int range = p_max - p_min;
	if (range <= 0) return p_min;
	int v = (p_value - p_min) % range;
	if (v < 0) v += range;
	return p_min + v;
}

// Implementation Macro for FULL grids
#define IMPL_GRID_FULL(ClassSuffix, Type) \
void ClassSuffix::_bind_methods() { \
	ClassDB::bind_method(D_METHOD("init_size", "size"), &ClassSuffix::init_size); \
	ClassDB::bind_method(D_METHOD("fill", "v"), &ClassSuffix::fill); \
	ClassDB::bind_method(D_METHOD("get_v", "y", "x"), &ClassSuffix::get_v); \
	ClassDB::bind_method(D_METHOD("set_v", "y", "x", "v"), &ClassSuffix::set_v); \
	ClassDB::bind_method(D_METHOD("add_v", "y", "x", "addition"), &ClassSuffix::add_v); \
	ClassDB::bind_method(D_METHOD("reduce_v", "y", "x", "reduction"), &ClassSuffix::reduce_v); \
	ClassDB::bind_method(D_METHOD("mul_v", "y", "x", "factor"), &ClassSuffix::mul_v); \
	ClassDB::bind_method(D_METHOD("get_v_by_idx", "idx"), &ClassSuffix::get_v_by_idx); \
	ClassDB::bind_method(D_METHOD("set_v_by_idx", "idx", "v"), &ClassSuffix::set_v_by_idx); \
	ClassDB::bind_method(D_METHOD("copy_from", "from"), &ClassSuffix::copy_from); \
} \
void ClassSuffix::init_size(const Vector2i &p_size) { \
	size = p_size; \
	data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y)); \
} \
void ClassSuffix::fill(Type p_v) { \
	std::fill(data.begin(), data.end(), p_v); \
} \
Type ClassSuffix::get_v(int p_y, int p_x) const { \
	if (size.x <= 0 || size.y <= 0) return Type(); \
	return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))]; \
} \
void ClassSuffix::set_v(int p_y, int p_x, Type p_v) { \
	if (size.x <= 0 || size.y <= 0) return; \
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v; \
} \
void ClassSuffix::add_v(int p_y, int p_x, Type p_v) { \
	if (size.x <= 0 || size.y <= 0) return; \
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] += p_v; \
} \
void ClassSuffix::reduce_v(int p_y, int p_x, Type p_v) { \
	if (size.x <= 0 || size.y <= 0) return; \
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] -= p_v; \
} \
void ClassSuffix::mul_v(int p_y, int p_x, float p_factor) { \
	if (size.x <= 0 || size.y <= 0) return; \
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] *= p_factor; \
} \
Type ClassSuffix::get_v_by_idx(int p_idx) const { \
	return data[static_cast<size_t>(p_idx)]; \
} \
void ClassSuffix::set_v_by_idx(int p_idx, Type p_v) { \
	data[static_cast<size_t>(p_idx)] = p_v; \
} \
void ClassSuffix::copy_from(Object *p_from) { \
	if (p_from == nullptr) return; \
	ClassSuffix *source = Object::cast_to<ClassSuffix>(p_from); \
	if (source == nullptr) return; \
	size = source->size; \
	data = source->data; \
}

// Implementation Macro for BASIC grids
#define IMPL_GRID_BASIC(ClassSuffix, Type) \
void ClassSuffix::_bind_methods() { \
	ClassDB::bind_method(D_METHOD("init_size", "size"), &ClassSuffix::init_size); \
	ClassDB::bind_method(D_METHOD("fill", "v"), &ClassSuffix::fill); \
	ClassDB::bind_method(D_METHOD("get_v", "y", "x"), &ClassSuffix::get_v); \
	ClassDB::bind_method(D_METHOD("set_v", "y", "x", "v"), &ClassSuffix::set_v); \
	ClassDB::bind_method(D_METHOD("copy_from", "from"), &ClassSuffix::copy_from); \
} \
void ClassSuffix::init_size(const Vector2i &p_size) { \
	size = p_size; \
	data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y)); \
} \
void ClassSuffix::fill(Type p_v) { \
	std::fill(data.begin(), data.end(), p_v); \
} \
Type ClassSuffix::get_v(int p_y, int p_x) const { \
	if (size.x <= 0 || size.y <= 0) return Type(); \
	return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))]; \
} \
void ClassSuffix::set_v(int p_y, int p_x, Type p_v) { \
	if (size.x <= 0 || size.y <= 0) return; \
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v; \
} \
void ClassSuffix::copy_from(Object *p_from) { \
	if (p_from == nullptr) return; \
	ClassSuffix *source = Object::cast_to<ClassSuffix>(p_from); \
	if (source == nullptr) return; \
	size = source->size; \
	data = source->data; \
}

IMPL_GRID_FULL(WrapGridFloat32, float)
IMPL_GRID_FULL(WrapGridInt32, int32_t)
IMPL_GRID_FULL(WrapGridVector2, Vector2)
IMPL_GRID_FULL(WrapGridColor, Color)
IMPL_GRID_FULL(WrapGridFloat64, double)
IMPL_GRID_FULL(WrapGridInt64, int64_t)
IMPL_GRID_FULL(WrapGridInt16, int16_t)
IMPL_GRID_FULL(WrapGridFloat16, float)

IMPL_GRID_BASIC(WrapGridRect2, Rect2)
IMPL_GRID_BASIC(WrapGridRect2i, Rect2i)

// Special Implementation for WrapGridVector2i
void WrapGridVector2i::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_size", "size"), &WrapGridVector2i::init_size);
	ClassDB::bind_method(D_METHOD("fill", "v"), &WrapGridVector2i::fill);
	ClassDB::bind_method(D_METHOD("get_v", "y", "x"), &WrapGridVector2i::get_v);
	ClassDB::bind_method(D_METHOD("set_v", "y", "x", "v"), &WrapGridVector2i::set_v);
	ClassDB::bind_method(D_METHOD("add_v", "y", "x", "addition"), &WrapGridVector2i::add_v);
	ClassDB::bind_method(D_METHOD("reduce_v", "y", "x", "reduction"), &WrapGridVector2i::reduce_v);
	ClassDB::bind_method(D_METHOD("mul_v", "y", "x", "factor"), &WrapGridVector2i::mul_v);
	ClassDB::bind_method(D_METHOD("copy_from", "from"), &WrapGridVector2i::copy_from);
}

void WrapGridVector2i::init_size(const Vector2i &p_size) {
	size = p_size;
	data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y));
}
void WrapGridVector2i::fill(Vector2i p_v) {
	std::fill(data.begin(), data.end(), p_v);
}
Vector2i WrapGridVector2i::get_v(int p_y, int p_x) const {
	if (size.x <= 0 || size.y <= 0) return Vector2i();
	return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))];
}
void WrapGridVector2i::set_v(int p_y, int p_x, Vector2i p_v) {
	if (size.x <= 0 || size.y <= 0) return;
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v;
}
void WrapGridVector2i::add_v(int p_y, int p_x, Vector2i p_v) {
	if (size.x <= 0 || size.y <= 0) return;
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] += p_v;
}
void WrapGridVector2i::reduce_v(int p_y, int p_x, Vector2i p_v) {
	if (size.x <= 0 || size.y <= 0) return;
	data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] -= p_v;
}
void WrapGridVector2i::mul_v(int p_y, int p_x, float p_factor) {
	if (size.x <= 0 || size.y <= 0) return;
	int idx = _wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x);
	data[idx] = Vector2i(Vector2(data[idx]) * p_factor);
}
void WrapGridVector2i::copy_from(Object *p_from) {
	if (p_from == nullptr) return;
	WrapGridVector2i *source = Object::cast_to<WrapGridVector2i>(p_from);
	if (source == nullptr) return;
	size = source->size;
	data = source->data;
}
