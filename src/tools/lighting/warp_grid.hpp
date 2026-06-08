#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/color.hpp>

#include <algorithm>
#include <vector>
#include <cstdint>

namespace godot {

class Lighting;

static inline int _wrapi(int p_value, int p_min, int p_max) {
	const int range = p_max - p_min;
	if (range <= 0) return p_min;
	int v = (p_value - p_min) % range;
	if (v < 0) v += range;
	return p_min + v;
}

// Macro for grids with full arithmetic support
#define DECLARE_GRID_FULL(ClassSuffix, Type) \
class ClassSuffix : public RefCounted { \
	GDCLASS(ClassSuffix, RefCounted); \
private: \
	Vector2i size; \
	std::vector<Type> data; \
protected: \
	static void _bind_methods(); \
public: \
	ClassSuffix() = default; \
	inline void init_size(const Vector2i &p_size) { \
		size = p_size; \
		data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y)); \
	} \
	inline void fill(Type p_v) { std::fill(data.begin(), data.end(), p_v); } \
	inline Type get_v(int p_y, int p_x) const { \
		if (size.x <= 0 || size.y <= 0) return Type(); \
		return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))]; \
	} \
	inline void set_v(int p_y, int p_x, Type p_v) { \
		if (size.x <= 0 || size.y <= 0) return; \
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v; \
	} \
	inline void add_v(int p_y, int p_x, Type p_v) { \
		if (size.x <= 0 || size.y <= 0) return; \
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] += p_v; \
	} \
	inline void reduce_v(int p_y, int p_x, Type p_v) { \
		if (size.x <= 0 || size.y <= 0) return; \
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] -= p_v; \
	} \
	inline void mul_v(int p_y, int p_x, float p_factor) { \
		if (size.x <= 0 || size.y <= 0) return; \
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] *= p_factor; \
	} \
	inline void copy_from(Object *p_from) { \
		if (p_from == nullptr) return; \
		ClassSuffix *source = Object::cast_to<ClassSuffix>(p_from); \
		if (source == nullptr) return; \
		size = source->size; \
		data = source->data; \
	} \
	inline const std::vector<Type>& get_data() const { return data; } \
	inline std::vector<Type>& get_data() { return data; } \
	inline Vector2i get_size() const { return size; } \
	inline Type get_v_by_idx(int p_idx) const { return data[static_cast<size_t>(p_idx)]; } \
	inline void set_v_by_idx(int p_idx, Type p_v) { data[static_cast<size_t>(p_idx)] = p_v; } \
	friend class Lighting; \
};

// Macro for grids with basic support
#define DECLARE_GRID_BASIC(ClassSuffix, Type) \
class ClassSuffix : public RefCounted { \
	GDCLASS(ClassSuffix, RefCounted); \
private: \
	Vector2i size; \
	std::vector<Type> data; \
protected: \
	static void _bind_methods(); \
public: \
	ClassSuffix() = default; \
	inline void init_size(const Vector2i &p_size) { \
		size = p_size; \
		data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y)); \
	} \
	inline void fill(Type p_v) { std::fill(data.begin(), data.end(), p_v); } \
	inline Type get_v(int p_y, int p_x) const { \
		if (size.x <= 0 || size.y <= 0) return Type(); \
		return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))]; \
	} \
	inline void set_v(int p_y, int p_x, Type p_v) { \
		if (size.x <= 0 || size.y <= 0) return; \
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v; \
	} \
	inline void copy_from(Object *p_from) { \
		if (p_from == nullptr) return; \
		ClassSuffix *source = Object::cast_to<ClassSuffix>(p_from); \
		if (source == nullptr) return; \
		size = source->size; \
		data = source->data; \
	} \
	inline const std::vector<Type>& get_data() const { return data; } \
	inline std::vector<Type>& get_data() { return data; } \
	inline Vector2i get_size() const { return size; } \
	friend class Lighting; \
};

DECLARE_GRID_FULL(WrapGridFloat32, float)
DECLARE_GRID_FULL(WrapGridInt32, int32_t)
DECLARE_GRID_FULL(WrapGridVector2, Vector2)
DECLARE_GRID_FULL(WrapGridColor, Color)
DECLARE_GRID_FULL(WrapGridFloat64, double)
DECLARE_GRID_FULL(WrapGridInt64, int64_t)
DECLARE_GRID_FULL(WrapGridInt16, int16_t)
DECLARE_GRID_FULL(WrapGridFloat16, float)

DECLARE_GRID_BASIC(WrapGridRect2, Rect2)
DECLARE_GRID_BASIC(WrapGridRect2i, Rect2i)

// WrapGridVector2i needs special handling for mul_v
class WrapGridVector2i : public RefCounted {
	GDCLASS(WrapGridVector2i, RefCounted);
private:
	Vector2i size;
	std::vector<Vector2i> data;
protected:
	static void _bind_methods();
public:
	WrapGridVector2i() = default;

	inline void init_size(const Vector2i &p_size) {
		size = p_size;
		data.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y));
	}
	inline void fill(Vector2i p_v) { std::fill(data.begin(), data.end(), p_v); }
	inline Vector2i get_v(int p_y, int p_x) const {
		if (size.x <= 0 || size.y <= 0) return Vector2i();
		return data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))];
	}
	inline void set_v(int p_y, int p_x, Vector2i p_v) {
		if (size.x <= 0 || size.y <= 0) return;
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] = p_v;
	}
	inline void add_v(int p_y, int p_x, Vector2i p_v) {
		if (size.x <= 0 || size.y <= 0) return;
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] += p_v;
	}
	inline void reduce_v(int p_y, int p_x, Vector2i p_v) {
		if (size.x <= 0 || size.y <= 0) return;
		data[static_cast<size_t>(_wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x))] -= p_v;
	}
	inline void mul_v(int p_y, int p_x, float p_factor) {
		if (size.x <= 0 || size.y <= 0) return;
		int idx = _wrapi(p_y, 0, size.y) * size.x + _wrapi(p_x, 0, size.x);
		data[idx] = Vector2i(Vector2(data[idx]) * p_factor);
	}
	inline void copy_from(Object *p_from) {
		if (p_from == nullptr) return;
		WrapGridVector2i *source = Object::cast_to<WrapGridVector2i>(p_from);
		if (source == nullptr) return;
		size = source->size;
		data = source->data;
	}
	inline const std::vector<Vector2i>& get_data() const { return data; }
	inline std::vector<Vector2i>& get_data() { return data; }
	inline Vector2i get_size() const { return size; }
	friend class Lighting;
};

} // namespace godot
