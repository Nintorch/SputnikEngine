#ifndef __VECTOR2_H__
#define __VECTOR2_H__

#include <ostream>
#include <cmath>

namespace Sputnik
{
	
#define BV_OP(op) \
		Basic_Vector& operator op ## = (const Basic_Vector& v) { \
			x op ## = v.x;\
			y op ## = v.y;\
			return *this;\
		} \
		Basic_Vector operator op (const Basic_Vector& v) const { \
			return {x op v.x, y op v.y};\
		}
	
	template<typename T>
	class Basic_Vector
	{
	public:
		T x, y;

		BV_OP(+);
		BV_OP(-);
		BV_OP(*);
		BV_OP(/);

		template<typename O>
		Basic_Vector& operator *= (O value)
		{
			x *= value;
			y *= value;
			return *this;
		}

		template<typename O>
		Basic_Vector operator * (O value) const
		{
			return { x * value, y * value };
		}

		template<typename O>
		Basic_Vector& operator /= (O value)
		{
			x = (T)(x / value);
			y = (T)(y / value);
			return *this;
		}

		template<typename O>
		Basic_Vector operator / (O value) const
		{
			return { (T)(x / value), (T)(y / value) };
		}

		Basic_Vector operator - () const
		{
			return { -x, -y };
		}

		float length() const
		{
			return std::sqrt(x * x + y * y);
		}

		Basic_Vector floor() const
		{
			return { std::floor(x), std::floor(y) };
		}
		
		Basic_Vector ceil() const
		{
			return { std::ceil(x), std::ceil(y) };
		}

		float distance_to(const Basic_Vector& v) const
		{
			return (*this - v).length();
		}

		// Rotate the vector clockwise
		Basic_Vector rotate(float degrees) const
		{
			float rad = degrees * M_PI / 180.0;
			float cos_value = cos(rad);
			float sin_value = sin(rad);

			Basic_Vector rotated_vector;
			rotated_vector.x = x * cos_value - y * sin_value;
			rotated_vector.y = x * sin_value + y * cos_value;
			return rotated_vector;
		}

		template<typename O>
		Basic_Vector<O> convert_to() const
		{
			return { (O)x, (O)y };
		}
		
		friend std::ostream& operator << (std::ostream& a, Basic_Vector b)
		{
			return a << '(' << b.x << ", " << b.y << ')';
		}
	};

	using Vector2 = Basic_Vector<float>;
	using Vector2Int = Basic_Vector<int>;
	
#undef BV_OP
}
#endif // __VECTOR2_H__