#include "dh_c.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void DHC::_bind_methods() {
    // Instance methods
    ClassDB::bind_method(D_METHOD("write", "method"), &DHC::write);
    ClassDB::bind_method(D_METHOD("_ensure_capacity", "required_size"), &DHC::_ensure_capacity);

    ClassDB::bind_method(D_METHOD("w_str", "s"), &DHC::w_str);
    ClassDB::bind_method(D_METHOD("w_byte_array", "a"), &DHC::w_byte_array);
    ClassDB::bind_method(D_METHOD("w_s8", "n"), &DHC::w_s8);
    ClassDB::bind_method(D_METHOD("w_s16", "n"), &DHC::w_s16);
    ClassDB::bind_method(D_METHOD("w_s32", "n"), &DHC::w_s32);
    ClassDB::bind_method(D_METHOD("w_s64", "n"), &DHC::w_s64);
    ClassDB::bind_method(D_METHOD("w_u8", "n"), &DHC::w_u8);
    ClassDB::bind_method(D_METHOD("w_u16", "n"), &DHC::w_u16);
    ClassDB::bind_method(D_METHOD("w_u32", "n"), &DHC::w_u32);
    ClassDB::bind_method(D_METHOD("w_u64", "n"), &DHC::w_u64);
    ClassDB::bind_method(D_METHOD("w_f16", "n"), &DHC::w_f16);
    ClassDB::bind_method(D_METHOD("w_f32", "n"), &DHC::w_f32);
    ClassDB::bind_method(D_METHOD("w_f64", "n"), &DHC::w_f64);
    ClassDB::bind_method(D_METHOD("w_col", "c"), &DHC::w_col);
    ClassDB::bind_method(D_METHOD("add_write_pos", "byte_count"), &DHC::add_write_pos);
    ClassDB::bind_method(D_METHOD("w_adaptive_u", "value", "from_bits"), &DHC::w_adaptive_u, DEFVAL(8));

    ClassDB::bind_method(D_METHOD("read", "data", "method"), &DHC::read);
    ClassDB::bind_method(D_METHOD("is_reached_end"), &DHC::is_reached_end);

    ClassDB::bind_method(D_METHOD("r_str"), &DHC::r_str);
    ClassDB::bind_method(D_METHOD("r_byte_array"), &DHC::r_byte_array);
    ClassDB::bind_method(D_METHOD("r_s8"), &DHC::r_s8);
    ClassDB::bind_method(D_METHOD("r_s16"), &DHC::r_s16);
    ClassDB::bind_method(D_METHOD("r_s32"), &DHC::r_s32);
    ClassDB::bind_method(D_METHOD("r_s64"), &DHC::r_s64);
    ClassDB::bind_method(D_METHOD("r_u8"), &DHC::r_u8);
    ClassDB::bind_method(D_METHOD("r_u16"), &DHC::r_u16);
    ClassDB::bind_method(D_METHOD("r_u32"), &DHC::r_u32);
    ClassDB::bind_method(D_METHOD("r_u64"), &DHC::r_u64);
    ClassDB::bind_method(D_METHOD("r_f16"), &DHC::r_f16);
    ClassDB::bind_method(D_METHOD("r_f32"), &DHC::r_f32);
    ClassDB::bind_method(D_METHOD("r_f64"), &DHC::r_f64);
    ClassDB::bind_method(D_METHOD("r_col"), &DHC::r_col);
    ClassDB::bind_method(D_METHOD("add_read_pos", "byte_count"), &DHC::add_read_pos);
    ClassDB::bind_method(D_METHOD("r_adaptive_u", "start_bits"), &DHC::r_adaptive_u, DEFVAL(8));
}

DHC::DHC() {
}

DHC::~DHC() {
}

// ---------- Write System ----------

void DHC::_ensure_capacity(int required_size) {
    if (required_size > arr.size()) {
        int new_capacity = Math::max<int>(required_size, arr.size() * 2);
        arr.resize(new_capacity);
    }
}

PackedByteArray DHC::write(const Callable& method) {
    write_pos = 0;
    if (arr.is_empty()) {
        arr.resize(INITIAL_CAPACITY);
    }
    method.call(this);
    PackedByteArray result = arr.duplicate();
    result.resize(write_pos);
    return result;
}

void DHC::w_str(const String& s) {
    PackedByteArray buffer = s.to_utf8_buffer();
    int length = buffer.size();
    w_u32(length);
    _ensure_capacity(write_pos + length);
    arr.append_array(buffer);
    write_pos += length;
}

void DHC::w_byte_array(const PackedByteArray& a) {
    int length = a.size();
    if (length < 254) {
        _ensure_capacity(write_pos + 1 + length);
        arr.encode_u8(write_pos, length);
        write_pos += 1;
    }
    else if (length <= 65534) {
        _ensure_capacity(write_pos + 3 + length);
        arr.encode_u8(write_pos, 254);
        write_pos += 1;
        w_u16(length);
    }
    else {
        _ensure_capacity(write_pos + 5 + length);
        arr.encode_u8(write_pos, 255);
        write_pos += 1;
        w_u32(length);
    }
    arr.append_array(a);
    write_pos += length;
}

void DHC::w_s8(int64_t n) {
    _ensure_capacity(write_pos + 1);
    arr.encode_s8(write_pos, static_cast<int8_t>(n));
    write_pos += 1;
}

void DHC::w_s16(int64_t n) {
    _ensure_capacity(write_pos + 2);
    arr.encode_s16(write_pos, static_cast<int16_t>(n));
    write_pos += 2;
}

void DHC::w_s32(int64_t n) {
    _ensure_capacity(write_pos + 4);
    arr.encode_s32(write_pos, static_cast<int32_t>(n));
    write_pos += 4;
}

void DHC::w_s64(int64_t n) {
    _ensure_capacity(write_pos + 8);
    arr.encode_s64(write_pos, n);
    write_pos += 8;
}

void DHC::w_u8(int64_t n) {
    _ensure_capacity(write_pos + 1);
    arr.encode_u8(write_pos, static_cast<uint8_t>(n));
    write_pos += 1;
}

void DHC::w_u16(int64_t n) {
    _ensure_capacity(write_pos + 2);
    arr.encode_u16(write_pos, static_cast<uint16_t>(n));
    write_pos += 2;
}

void DHC::w_u32(int64_t n) {
    _ensure_capacity(write_pos + 4);
    arr.encode_u32(write_pos, static_cast<uint32_t>(n));
    write_pos += 4;
}

void DHC::w_u64(int64_t n) {
    _ensure_capacity(write_pos + 8);
    arr.encode_u64(write_pos, static_cast<uint64_t>(n));
    write_pos += 8;
}

void DHC::w_f16(double n) {
    _ensure_capacity(write_pos + 2);
    arr.encode_half(write_pos, n);
    write_pos += 2;
}

void DHC::w_f32(double n) {
    _ensure_capacity(write_pos + 4);
    arr.encode_float(write_pos, n);
    write_pos += 4;
}

void DHC::w_f64(double n) {
    _ensure_capacity(write_pos + 8);
    arr.encode_double(write_pos, n);
    write_pos += 8;
}

void DHC::w_col(const Color& c) {
    _ensure_capacity(write_pos + 4);
    uint8_t* ptr = arr.ptrw();
    ptr[write_pos] = static_cast<uint8_t>(c.r * 255);
    ptr[write_pos + 1] = static_cast<uint8_t>(c.g * 255);
    ptr[write_pos + 2] = static_cast<uint8_t>(c.b * 255);
    ptr[write_pos + 3] = static_cast<uint8_t>(c.a * 255);
    write_pos += 4;
}

void DHC::add_write_pos(int byte_count) {
    write_pos += byte_count;
}

void DHC::w_adaptive_u(int64_t value, int from_bits) {
    while (from_bits <= 64) {
        int64_t max_val;
        switch (from_bits) {
        case 8: max_val = 0xFF; break;
        case 16: max_val = 0xFFFF; break;
        case 32: max_val = 0xFFFFFFFF; break;
        default:
            UtilityFunctions::push_error("Invalid bits in w_adaptive_u");
            return;
        }
        if (value >= max_val && from_bits < 32) {
            switch (from_bits) {
            case 8: w_u8(max_val); break;
            case 16: w_u16(max_val); break;
            case 32: w_u32(max_val); break;
            }
            from_bits *= 2;
        }
        else {
            switch (from_bits) {
            case 8: w_u8(value); break;
            case 16: w_u16(value); break;
            case 32: w_u32(value); break;
            }
            break;
        }
    }
}

// ---------- Read System ----------

Variant DHC::read(const PackedByteArray& data, const Callable& method) {
    arr = data;
    read_pos = 0;
    return method.call(this);
}

bool DHC::is_reached_end() const {
    return read_pos >= arr.size();
}

String DHC::r_str() {
    int size = r_u32();
    PackedByteArray slice = arr.slice(read_pos, read_pos + size);
    read_pos += size;
    return slice.get_string_from_utf8();
}

PackedByteArray DHC::r_byte_array() {
    int marker = arr.decode_u8(read_pos);
    read_pos += 1;
    int size;
    if (marker < 254) {
        size = marker;
    }
    else if (marker == 254) {
        size = r_u16();
    }
    else {
        size = r_u32();
    }
    PackedByteArray ret = arr.slice(read_pos, read_pos + size);
    read_pos += size;
    return ret;
}

int64_t DHC::r_s8() {
    int8_t val = arr.decode_s8(read_pos);
    read_pos += 1;
    return val;
}

int64_t DHC::r_s16() {
    int16_t val = arr.decode_s16(read_pos);
    read_pos += 2;
    return val;
}

int64_t DHC::r_s32() {
    int32_t val = arr.decode_s32(read_pos);
    read_pos += 4;
    return val;
}

int64_t DHC::r_s64() {
    int64_t val = arr.decode_s64(read_pos);
    read_pos += 8;
    return val;
}

int64_t DHC::r_u8() {
    uint8_t val = arr.decode_u8(read_pos);
    read_pos += 1;
    return val;
}

int64_t DHC::r_u16() {
    uint16_t val = arr.decode_u16(read_pos);
    read_pos += 2;
    return val;
}

int64_t DHC::r_u32() {
    uint32_t val = arr.decode_u32(read_pos);
    read_pos += 4;
    return val;
}

int64_t DHC::r_u64() {
    uint64_t val = arr.decode_u64(read_pos);
    read_pos += 8;
    return val;
}

double DHC::r_f16() {
    real_t val = arr.decode_half(read_pos);
    read_pos += 2;
    return val;
}

double DHC::r_f32() {
    real_t val = arr.decode_float(read_pos);
    read_pos += 4;
    return val;
}

double DHC::r_f64() {
    double val = arr.decode_double(read_pos);
    read_pos += 8;
    return val;
}

Color DHC::r_col() {
    const uint8_t* ptr = arr.ptr();
    Color col(
        ptr[read_pos] / 255.0f,
        ptr[read_pos + 1] / 255.0f,
        ptr[read_pos + 2] / 255.0f,
        ptr[read_pos + 3] / 255.0f
    );
    read_pos += 4;
    return col;
}

void DHC::add_read_pos(int byte_count) {
    read_pos += byte_count;
}

int64_t DHC::r_adaptive_u(int start_bits) {
    while (start_bits <= 64) {
        int64_t val;
        switch (start_bits) {
        case 8: val = r_u8(); break;
        case 16: val = r_u16(); break;
        case 32: val = r_u32(); break;
        default:
            UtilityFunctions::push_error("Invalid bits in r_adaptive_u");
            return 0;
        }
        int64_t max_val;
        switch (start_bits) {
        case 8: max_val = 0xFF; break;
        case 16: max_val = 0xFFFF; break;
        case 32: max_val = 0xFFFFFFFF; break;
        }
        if (val == max_val && start_bits < 32) {
            start_bits *= 2;
            continue;
        }
        else {
            return val;
        }
    }
    UtilityFunctions::push_error("DHC: r_adaptive_u failed to decode");
    return 0;
}