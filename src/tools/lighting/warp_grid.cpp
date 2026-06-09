#include "tools/lighting/warp_grid.hpp"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Implementation for FULL grids
#define IMPL_GRID_FULL_BIND(ClassSuffix) \
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
}

// Implementation for BASIC grids
#define IMPL_GRID_BASIC_BIND(ClassSuffix) \
void ClassSuffix::_bind_methods() { \
	ClassDB::bind_method(D_METHOD("init_size", "size"), &ClassSuffix::init_size); \
	ClassDB::bind_method(D_METHOD("fill", "v"), &ClassSuffix::fill); \
	ClassDB::bind_method(D_METHOD("get_v", "y", "x"), &ClassSuffix::get_v); \
	ClassDB::bind_method(D_METHOD("set_v", "y", "x", "v"), &ClassSuffix::set_v); \
	ClassDB::bind_method(D_METHOD("copy_from", "from"), &ClassSuffix::copy_from); \
}

IMPL_GRID_FULL_BIND(WrapGridFloat32)
IMPL_GRID_FULL_BIND(WrapGridInt32)
IMPL_GRID_FULL_BIND(WrapGridVector2)
IMPL_GRID_FULL_BIND(WrapGridColor)
IMPL_GRID_FULL_BIND(WrapGridFloat64)
IMPL_GRID_FULL_BIND(WrapGridInt64)
IMPL_GRID_FULL_BIND(WrapGridInt16)
IMPL_GRID_FULL_BIND(WrapGridFloat16)

IMPL_GRID_BASIC_BIND(WrapGridRect2)
IMPL_GRID_BASIC_BIND(WrapGridRect2i)

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
