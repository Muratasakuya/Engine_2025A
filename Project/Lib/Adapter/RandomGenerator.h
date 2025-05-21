#pragma once

//============================================================================*/
//	include
//============================================================================*/
#include <Lib/MathUtils/Vector3.h>
#include <Lib/MathUtils/Vector4.h>

// c++
#include <random>
#include <type_traits>

//============================================================================*/
//	RandomGenerator class
//============================================================================*/
class RandomGenerator {
public:
	//========================================================================*/
	//	public Methods
	//========================================================================*/

	template <typename T>
	static T Generate(T min, T max);

	// Vector3
	static Vector3 Generate(const Vector3& min, const Vector3& max);
	// Color
	static Color Generate(const Color& min, const Color& max);
};

template<typename T>
inline T RandomGenerator::Generate(T min, T max) {

	T minValue = min;
	T maxValue = max;
	// どちらが大きいか比較して、小さい方をminに入れる
	if (minValue > maxValue) {

		// minがmaxより大きい場合、minとmaxを入れ替える
		minValue = max;
		maxValue = min;
	}
	static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

	static std::random_device rd;
	static std::mt19937 gen(rd());

	if constexpr (std::is_integral<T>::value) {

		std::uniform_int_distribution<T> dist(minValue, maxValue);
		return dist(gen);
	} else if constexpr (std::is_floating_point<T>::value) {

		std::uniform_real_distribution<T> dist(minValue, maxValue);
		return dist(gen);
	}
}