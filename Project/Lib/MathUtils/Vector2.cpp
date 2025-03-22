#include "Vector2.h"

//============================================================================*/
//	Vector2 classMethods
//============================================================================*/

Vector2 Vector2::operator+(const Vector2& other) const {
	return Vector2(x + other.x, y + other.y);
}
Vector2 Vector2::operator-(const Vector2& other) const {
	return Vector2(x - other.x, y - other.y);
}
Vector2 Vector2::operator*(const Vector2& other) const {
	return Vector2(x * other.x, y * other.y);
}
Vector2 Vector2::operator/(const Vector2& other) const {
	return Vector2(x / other.x, y / other.y);
}

Vector2& Vector2::operator+=(const Vector2& v) {
	x += v.x;
	y += v.y;
	return *this;
}
Vector2& Vector2::operator-=(const Vector2& v) {
	x += v.x;
	y += v.y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& v) {
	x *= v.x;
	y *= v.y;
	return *this;
}

Vector2& Vector2::operator/=(const Vector2& v) {
	x /= v.x;
	y /= v.y;
	return *this;
}

Vector2 Vector2::operator*(float scalar) const {
	return Vector2(x * scalar, y * scalar);
}
Vector2 operator*(float scalar, const Vector2& v) {
	return Vector2(v.x * scalar, v.y * scalar);
}
Vector2 Vector2::operator/(float scalar) const {
	return Vector2(x / scalar, y / scalar);
}
Vector2 operator/(float scalar, const Vector2& v) {
	return Vector2(v.x / scalar, v.y / scalar);
}

Vector2& Vector2::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	return *this;
}

bool Vector2::operator==(const Vector2& other) const {
	return x == other.x && y == other.y;
}
bool Vector2::operator!=(const Vector2& other) const {
	return !(*this == other);
}

Json Vector2::ToJson() const {
	return Json{ {"x", x}, {"y", y} };
}

Vector2 Vector2::FromJson(const Json& data) {

	Vector2 v{};
	if (data.contains("x") && data.contains("y")) {
		v.x = data["x"].get<float>();
		v.y = data["y"].get<float>();
	}
	return v;
}

void Vector2::Init() {

	this->x = 0.0f;
	this->y = 0.0f;
}

float Vector2::Length() const {
	return std::sqrtf(x * x + y * y);
}

Vector2 Vector2::Normalize() const {
	float length = this->Length();
	if (length == 0.0f) {
		return Vector2(0.0f, 0.0f);
	}
	return Vector2(x / length, y / length);
}

Vector2 Vector2::AnyInit(float value) {

	return Vector2(value, value);
}