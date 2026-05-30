#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/color.hpp>

#include <vector>
#include <cstdint>

namespace godot {

class Lighting;

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
	void init_size(const Vector2i &p_size); \
	void fill(Type p_v); \
	Type get_v(int p_y, int p_x) const; \
	void set_v(int p_y, int p_x, Type p_v); \
	void add_v(int p_y, int p_x, Type p_v); \
	void reduce_v(int p_y, int p_x, Type p_v); \
	void mul_v(int p_y, int p_x, float p_factor); \
	void copy_from(Object *p_from); \
	const std::vector<Type>& get_data() const { return data; } \
	std::vector<Type>& get_data() { return data; } \
	Vector2i get_size() const { return size; } \
	Type get_v_by_idx(int p_idx) const; \
	void set_v_by_idx(int p_idx, Type p_v); \
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
	void init_size(const Vector2i &p_size); \
	void fill(Type p_v); \
	Type get_v(int p_y, int p_x) const; \
	void set_v(int p_y, int p_x, Type p_v); \
	void copy_from(Object *p_from); \
	const std::vector<Type>& get_data() const { return data; } \
	std::vector<Type>& get_data() { return data; } \
	Vector2i get_size() const { return size; } \
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
	void init_size(const Vector2i &p_size);
	void fill(Vector2i p_v);
	Vector2i get_v(int p_y, int p_x) const;
	void set_v(int p_y, int p_x, Vector2i p_v);
	void add_v(int p_y, int p_x, Vector2i p_v);
	void reduce_v(int p_y, int p_x, Vector2i p_v);
	void mul_v(int p_y, int p_x, float p_factor);
	void copy_from(Object *p_from);
	const std::vector<Vector2i>& get_data() const { return data; }
	std::vector<Vector2i>& get_data() { return data; }
	Vector2i get_size() const { return size; }
	friend class Lighting;
};

} // namespace godot
