#pragma once

#include <cstdint>
#include <vector>
#include <bitset>
#include <array>
#include <memory>
#include <cstring>   // memcpy
#include <cassert>
#include "component_register.h"

inline constexpr size_t kChunkSize = 16384;

struct Archetype;    // 前向声明

struct Chunk;