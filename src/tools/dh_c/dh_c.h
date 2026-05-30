#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {
    class DHC : public RefCounted {
        GDCLASS(DHC, RefCounted);

    private:
        int write_pos = 0;
        int read_pos = 0;
        PackedByteArray arr;

        static const int INITIAL_CAPACITY = 128;

    protected:
        static void _bind_methods();

    public:
        DHC();
        ~DHC();

        // Write system
        PackedByteArray write(const Callable& method);
        void _ensure_capacity(int required_size);

        // Write methods
        void w_str(const String& s);
        void w_byte_array(const PackedByteArray& a);
        void w_s8(int64_t n);
        void w_s16(int64_t n);
        void w_s32(int64_t n);
        void w_s64(int64_t n);
        void w_u8(int64_t n);
        void w_u16(int64_t n);
        void w_u32(int64_t n);
        void w_u64(int64_t n);
        void w_f16(double n);
        void w_f32(double n);
        void w_f64(double n);
        void w_col(const Color& c);
        void add_write_pos(int byte_count);
        void w_adaptive_u(int64_t value, int from_bits = 8);

        // Read system
        Variant read(const PackedByteArray& data, const Callable& method);
        bool is_reached_end() const;

        // Read methods
        String r_str();
        PackedByteArray r_byte_array();
        int64_t r_s8();
        int64_t r_s16();
        int64_t r_s32();
        int64_t r_s64();
        int64_t r_u8();
        int64_t r_u16();
        int64_t r_u32();
        int64_t r_u64();
        double r_f16();
        double r_f32();
        double r_f64();
        Color r_col();
        void add_read_pos(int byte_count);
        int64_t r_adaptive_u(int start_bits = 8);
    };

} // namespace godot
