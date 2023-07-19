#pragma once
#ifndef TIPSY_ENCODER_BINARY_TO_FLOAT_H
#define TIPSY_ENCODER_BINARY_TO_FLOAT_H
/*
 * binary-to-float.h
 *
 * Low level encoding to take 3 unsigned chars and project them to a single valid float
 * and vice versa. The values used are designed for compatability with VCV Rack voltage standards.
 *
 * The encoding we use is as follows:
 *
 * An IEEE float is: lowest 23 bits are fraction, next 8 are exponent, last is sign bit.
 *
 */
#include <cstdint>

namespace tipsy
{

constexpr const unsigned char BYTE_MASK = 0xff;
constexpr const unsigned char LOW_7_MASK = 0x7f;
constexpr const unsigned char BIT_8_MASK = 0x80;
constexpr const unsigned char EXPONENT_FILL = 0x3f;

union FloatBytes
{
    unsigned char bytes[4];
    float f;

    operator float() const noexcept { return f; }

    unsigned char first() const noexcept { return bytes[0]; }
    unsigned char second() const noexcept { return bytes[1]; }
    unsigned char third() const noexcept
    {
        return (bytes[2] & LOW_7_MASK) | (bytes[3] & BIT_8_MASK);
    }

    explicit constexpr FloatBytes(float fi) noexcept : f(fi) {}

    FloatBytes() noexcept
    {
        bytes[0] = 0;
        bytes[1] = 0;
        bytes[2] = 0;
        bytes[3] = EXPONENT_FILL;
    }

    FloatBytes(unsigned char b1, unsigned char b2) noexcept
    {
        bytes[0] = b1;
        bytes[1] = b2;
        bytes[2] = 0;
        bytes[3] = EXPONENT_FILL;
    }

    FloatBytes(uint16_t value) noexcept
    {
        bytes[0] = value & BYTE_MASK;
        bytes[1] = (value >> 8) & BYTE_MASK;
        bytes[2] = 0;
        bytes[3] = EXPONENT_FILL;
    }

    // Not all uint32_t values are representable in this format
    static bool isRepresentable(uint32_t value) noexcept
    {
        return 0 == (value & (uint32_t)0xFF000000);
    }

    FloatBytes(uint32_t value) noexcept
    {
        assert(isRepresentable(value));
        bytes[0] = value & BYTE_MASK;
        bytes[1] = (value >> 8) & BYTE_MASK;
        unsigned char b3 = (value >> 16) & BYTE_MASK;
        bytes[2] = b3 & LOW_7_MASK;
        bytes[3] = (b3 & BIT_8_MASK) | EXPONENT_FILL;
    }

    constexpr FloatBytes(unsigned char b1, unsigned char b2, unsigned char b3) noexcept
        : bytes{b1, b2, (unsigned char)(b3 & LOW_7_MASK),
                (unsigned char)((b3 & BIT_8_MASK) | EXPONENT_FILL)}
    {
    }
};

inline uint16_t uint16_FromFloat(float f) noexcept
{
    auto fb = FloatBytes(f);
    assert(0 == fb.bytes[2]);
    return fb.bytes[0] | (fb.bytes[1] << 8);
}

inline uint32_t uint32_FromFloat(float f) noexcept
{
    auto fb = FloatBytes(f);
    return fb.first() | (fb.second() << 8) | (fb.third() << 16);
}

inline float threeBytesToFloat(unsigned char b1, unsigned char b2, unsigned char b3) noexcept
{
    return FloatBytes(b1, b2, b3).f;
}

inline unsigned char FirstByte(float f) noexcept { return FloatBytes(f).first(); }

inline unsigned char SecondByte(float f) noexcept { return FloatBytes(f).second(); }

inline unsigned char ThirdByte(float f) noexcept { return FloatBytes(f).third(); }

inline float minimumEncodedFloat() noexcept { return FloatBytes(255, 255, 255).f; }
inline float maximumEncodedFloat() noexcept { return FloatBytes(255, 255, 127).f; }
inline bool isValidDataEncoding(float f) noexcept
{
    return (minimumEncodedFloat() <= f) && (f <= maximumEncodedFloat());
}
} // namespace tipsy
#endif // TIPSY_ENCODER_BINARY_TO_FLOAT_H
