#ifndef ZENGINE_CORE
#define ZENGINE_CORE

#include <cstdint>
struct ZEvent : virtual RTTI::Enable {
	RTTI_DECLARE_TYPEINFO(ZEvent);
};

float toRadians(float degrees) {
	return degrees * static_cast<float>(0.01745329251994329576923690768489);
}

float toDegrees(float radians) {
	return radians * static_cast<float>(57.295779513082320876798154814105);
}

template <typename T>
struct Vec2Impl {
	T x = 0.0f, y = 0.0f;

	Vec2Impl() = default;

	Vec2Impl(T value) : x(value), y(value) {}

	Vec2Impl(T x, T y) : x(x), y(y) {}

	Vec2Impl operator + (T value) {
		return Vec2Impl(x + value, y + value);
	}

	Vec2Impl operator + (const Vec2Impl& other) {
		return Vec2Impl(x + other.x, y + other.y);
	}

	Vec2Impl operator - (T value) {
		return Vec2Impl(x - value, y - value);
	}

	Vec2Impl operator - (const Vec2Impl& other) {
		return Vec2Impl(x - other.x, y - other.y);
	}

	void operator += (T value) {
		x += value;
		y += value;
	}

	void operator += (const Vec2Impl& other) {
		x += other.x;
		y += other.y;
	}

	void operator -= (T value) {
		x -= value;
		y -= value;
	}

	void operator -= (const Vec2Impl& other) {
		x -= other.x;
		y -= other.y;
	}
};

template <typename T>
struct Vec3Impl {
	T x = 0.0f, y = 0.0f, z = 0.0f;

	Vec3Impl() = default;

	Vec3Impl(T value) : x(value), y(value), z(value) {}

	Vec3Impl(T x, T y, T z) : x(x), y(y), z(z) {}

	Vec3Impl operator + (T value) {
		return Vec3Impl(x + value, y + value, z + value);
	}

	Vec3Impl operator + (const Vec3Impl& other) {
		return Vec3Impl(x + other.x, y + other.y, z + other.z);
	}

	Vec3Impl operator - (T value) {
		return Vec3Impl(x - value, y - value, z - value);
	}

	Vec3Impl operator - (const Vec3Impl& other) {
		return Vec3Impl(x - other.x, y - other.y, z - other.z);
	}

	void operator += (T value) {
		x += value;
		y += value;
		z += value;
	}

	void operator += (const Vec3Impl& other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator -= (T value) {
		x -= value;
		y -= value;
		z -= value;
	}

	void operator -= (const Vec3Impl& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}
};

template <typename T>
struct Vec4Impl {
	T x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	Vec4Impl() = default;

	Vec4Impl(T value) : x(value), y(value), z(value), w(value) {}

	Vec4Impl(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

	Vec4Impl operator + (T value) {
		return Vec4Impl(x + value, y + value, z + value, w + value);
	}

	Vec4Impl operator + (const Vec4Impl& other) {
		return Vec4Impl(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4Impl operator - (T value) {
		return Vec4Impl(x - value, y - value, z - value, w - value);
	}

	Vec4Impl operator - (const Vec4Impl& other) {
		return Vec4Impl(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	void operator += (T value) {
		x += value;
		y += value;
		z += value;
		w += value;
	}

	void operator += (const Vec4Impl& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	void operator -= (T value) {
		x -= value;
		y -= value;
		z -= value;
		w -= value;
	}

	void operator -= (const Vec4Impl& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
	}
};

typedef Vec2Impl<float> Vec2;
typedef Vec3Impl<float> Vec3;
typedef Vec4Impl<float> Vec4;

typedef Vec2Impl<int> IVec2;
typedef Vec3Impl<int> IVec3;
typedef Vec4Impl<int> IVec4;

typedef Vec3Impl<std::uint8_t> U8Vec3;
typedef Vec4Impl<std::uint8_t> U8Vec4;

struct ZTransform {
	Vec2 position = Vec2(0.0f);
	float rotation = 0;
	Vec2 scale = Vec2(1.0f);
};

#endif // !ZENGINE_CORE
