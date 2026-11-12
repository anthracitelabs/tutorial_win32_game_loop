#ifndef GAME_MATH_H
#define GAME_MATH_H

#include "math.h"

#define PI_VAL 3.14159265359f
#define RAD_TO_DEG ((float)(180.0f/PI_VAL))
#define DEG_TO_RAD ((float)(PI_VAL/180.0f))


struct v2
{
	float x;
	float y;
};

struct v3
{
	float x;
	float y;
	float z;
};

struct v4
{
	float x;
	float y;
	float z;
	float w;
};

/*
| Index | Component |
| -------- | -------- |
| 0 - 3 | First column (typically X-axis basis vector) |
| 4 – 7	| Second column (typically Y-axis basis vector) |
| 8 – 11 | Third column (typically Z-axis basis vector) |
| 12 – 15 | Fourth column (Translation components X, Y, Z) |
 */
struct m4x4
{
	float Elements[16];
};

static inline float DegreeToRadian(float Angle)
{
    float Result = Angle * DEG_TO_RAD;
    return Result;
}

static inline float RadianToDegree(float Angle)
{
    float Result = Angle * RAD_TO_DEG;
    return Result;
}

static inline float Lerp(float A, float Time, float B)
{
    return (1.0f - Time) * A + Time * B;
}

static inline float Clamp(float Min, float Value, float Max)
{
    float Result = Value;

    if (Result < Min)
    {
        Result = Min;
    }

    if (Result > Max)
    {
        Result = Max;
    }

    return Result;
}

inline int32 RoundReal32ToInt32(float input)
{
	int32 result = (int32)roundf(input);
	return result;
}

inline v2 operator+(v2 a, v2 b)
{
	v2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

inline v2 operator+=(v2 &a, v2 b)
{
	a = a + b;
	return a;
}

inline v2 operator-(v2 a)
{
	v2 result;
	result.x = -a.x;
	result.y = -a.y;
	return result;
}

inline v2 operator-(v2 a, v2 b)
{
	v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

inline v2 operator-=(v2 &a, v2 b)
{
  a = a - b;
  return a;
}

inline v2 operator*(float a, v2 b)
{
  v2 result = { a * b.x, a * b.y };
  return result;
}

inline v2 operator*(v2 b, float a)
{
  v2 result = a * b;
  return result;
}

inline v2 operator*=(v2 &a, float b)
{
  a = b * a;
  return a;
}

inline v3 operator+(v3 a, v3 b)
{
	v3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

inline v3 operator+=(v3 &a, v3 b)
{
	a = a + b;
	return a;
}

inline v3 operator-(v3 a)
{
	v3 result;
	result.x = -a.x;
	result.y = -a.y;
	result.z = -a.z;
	return result;
}

inline m4x4 operator*(m4x4 a, m4x4 b)
{
	m4x4 result;
	result.Elements[0] = a.Elements[0] * b.Elements[0] + a.Elements[4] * b.Elements[1] + a.Elements[8] * b.Elements[2] + a.Elements[12] * b.Elements[3];
	result.Elements[1] = a.Elements[1] * b.Elements[0] + a.Elements[5] * b.Elements[1] + a.Elements[9] * b.Elements[2] + a.Elements[13] * b.Elements[3];
	result.Elements[2] = a.Elements[2] * b.Elements[0] + a.Elements[6] * b.Elements[1] + a.Elements[10] * b.Elements[2] + a.Elements[14] * b.Elements[3];
	result.Elements[3] = a.Elements[3] * b.Elements[0] + a.Elements[7] * b.Elements[1] + a.Elements[11] * b.Elements[2] + a.Elements[15] * b.Elements[3];
	
	result.Elements[4] = a.Elements[0] * b.Elements[4] + a.Elements[4] * b.Elements[5] + a.Elements[8] * b.Elements[6] + a.Elements[12] * b.Elements[7];
	result.Elements[5] = a.Elements[1] * b.Elements[4] + a.Elements[5] * b.Elements[5] + a.Elements[9] * b.Elements[6] + a.Elements[13] * b.Elements[7];
	result.Elements[6] = a.Elements[2] * b.Elements[4] + a.Elements[6] * b.Elements[5] + a.Elements[10] * b.Elements[6] + a.Elements[14] * b.Elements[7];
	result.Elements[7] = a.Elements[3] * b.Elements[4] + a.Elements[7] * b.Elements[5] + a.Elements[11] * b.Elements[6] + a.Elements[15] * b.Elements[7];
	
	result.Elements[8] = a.Elements[0] * b.Elements[8] + a.Elements[4] * b.Elements[9] + a.Elements[8] * b.Elements[10] + a.Elements[12] * b.Elements[11];
	result.Elements[9] = a.Elements[1] * b.Elements[8] + a.Elements[5] * b.Elements[9] + a.Elements[9] * b.Elements[10] + a.Elements[13] * b.Elements[11];
	result.Elements[10] = a.Elements[2] * b.Elements[8] + a.Elements[6] * b.Elements[9] + a.Elements[10] * b.Elements[10] + a.Elements[14] * b.Elements[11];
	result.Elements[11] = a.Elements[3] * b.Elements[8] + a.Elements[7] * b.Elements[9] + a.Elements[11] * b.Elements[10] + a.Elements[15] * b.Elements[11];

	result.Elements[12] = a.Elements[0] * b.Elements[12] + a.Elements[4] * b.Elements[13] + a.Elements[8] * b.Elements[14] + a.Elements[12] * b.Elements[15];
	result.Elements[13] = a.Elements[1] * b.Elements[12] + a.Elements[5] * b.Elements[13] + a.Elements[9] * b.Elements[14] + a.Elements[13] * b.Elements[15];
	result.Elements[14] = a.Elements[2] * b.Elements[12] + a.Elements[6] * b.Elements[13] + a.Elements[10] * b.Elements[14] + a.Elements[14] * b.Elements[15];
	result.Elements[15] = a.Elements[3] * b.Elements[12] + a.Elements[7] * b.Elements[13] + a.Elements[11] * b.Elements[14] + a.Elements[15] * b.Elements[15];

	return result;
}

inline v3 operator-(v3 a, v3 b)
{
	v3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

inline v3 operator-=(v3 &a, v3 b)
{
  a = a - b;
  return a;
}

inline v3 operator*(float a, v3 b)
{
  v3 result = { a * b.x, a * b.y, a * b.z };
  return result;
}

inline v3 operator*(v3 b, float a)
{
  v3 result = a * b;
  return result;
}

inline v3 operator*=(v3 &a, float b)
{
  a = b * a;
  return a;
}

inline float Inner(v2 a, v2 b)
{
  float result = a.x * b.x + a.y * b.y;
  return result;
}

inline float LengthSq(v2 a)
{
  float result = Inner(a, a);
  return result;
}

inline float SquareRoot(float value)
{
  float result = sqrtf(value);
  return result;
}

inline v2 Normalize(v2 A)
{
    float InnerProduct = A.x*A.x + A.y*A.y;
    float Length = SquareRoot(InnerProduct);
    
    v2 Result = {};
    if(Length > 0.0001f)
    {
        // Multiply by the reciprocal to save execution cycles
        float InvLength = 1.0f / Length;
        Result.x = A.x * InvLength;
        Result.y = A.y * InvLength;
    }
    
    return(Result);
}

inline float Square(float val)
{
	float result;
	result = val * val;
	return result;
}

inline v3 normalize(v3 v)
{
	float Length = SquareRoot(Square(v.x) + Square(v.y) + Square(v.z));
	v3 Result = {0};
	if(Length > 0.0001f)
	{
		Result.x = v.x * (1.0f / Length);
		Result.y = v.y * (1.0f / Length);
		Result.z = v.z * (1.0f / Length);
	}

	return Result;
}

inline float dot(v3 a, v3 b)
{
	float Result = 0.0f;
	Result = a.x * b.x + a.y * b.y + a.z * b.z;
	return Result;
}

inline v3 cross(v3 a, v3 b)
{
	v3 Result = {0};
	Result.x = a.y * b.z - a.z * b.y;
    Result.y = a.z * b.x - a.x * b.z;
    Result.z = a.x * b.y - a.y * b.x;
	return Result;
}

// column-major matrix multiplication
inline v4 Multiply(m4x4 Matrix, v4 Vector)
{
	v4 Result;

    Result.x = (Matrix.Elements[0] * Vector.x) + (Matrix.Elements[4] * Vector.y) + (Matrix.Elements[8] * Vector.z) + (Matrix.Elements[12] * Vector.w);
    Result.y = (Matrix.Elements[1] * Vector.x) + (Matrix.Elements[5] * Vector.y) + (Matrix.Elements[9] * Vector.z) + (Matrix.Elements[13] * Vector.w);
    Result.z = (Matrix.Elements[2] * Vector.x) + (Matrix.Elements[6] * Vector.y) + (Matrix.Elements[10] * Vector.z) + (Matrix.Elements[14] * Vector.w);
    Result.w = (Matrix.Elements[3] * Vector.x) + (Matrix.Elements[7] * Vector.y) + (Matrix.Elements[11] * Vector.z) + (Matrix.Elements[15] * Vector.w);

    return Result;
}

/*
| Index | Component |
| -------- | -------- |
| 0 - 3 | First column (typically X-axis basis vector) |
| 4 – 7	| Second column (typically Y-axis basis vector) |
| 8 – 11 | Third column (typically Z-axis basis vector) |
| 12 – 15 | Fourth column (Translation components X, Y, Z) |
 */
inline m4x4 Multiply(m4x4 A, m4x4 B)
{
	m4x4 Result;
	// Iterate through every column of Matrix B
    for (int Col = 0; Col < 4; ++Col)
    {
        // Compute the resulting column elements 
        // by taking the dot product of A's rows with B's column
        Result.Elements[Col * 4 + 0] = (A.Elements[0] * B.Elements[Col * 4 + 0]) + (A.Elements[4] * B.Elements[Col * 4 + 1]) + (A.Elements[8] * B.Elements[Col * 4 + 2]) + (A.Elements[12] * B.Elements[Col * 4 + 3]);
        Result.Elements[Col * 4 + 1] = (A.Elements[1] * B.Elements[Col * 4 + 0]) + (A.Elements[5] * B.Elements[Col * 4 + 1]) + (A.Elements[9] * B.Elements[Col * 4 + 2]) + (A.Elements[13] * B.Elements[Col * 4 + 3]);
        Result.Elements[Col * 4 + 2] = (A.Elements[2] * B.Elements[Col * 4 + 0]) + (A.Elements[6] * B.Elements[Col * 4 + 1]) + (A.Elements[10] * B.Elements[Col * 4 + 2]) + (A.Elements[14] * B.Elements[Col * 4 + 3]);
        Result.Elements[Col * 4 + 3] = (A.Elements[3] * B.Elements[Col * 4 + 0]) + (A.Elements[7] * B.Elements[Col * 4 + 1]) + (A.Elements[11] * B.Elements[Col * 4 + 2]) + (A.Elements[15] * B.Elements[Col * 4 + 3]);
    }
	return Result;
}

inline m4x4 Identity()
{
	m4x4 Result  = { 1.0f, 0.0f, 0.0f, 0.0f,	//| 0 - 3 | First column (typically X-axis basis vector)
			0.0f, 1.0f, 0.0f, 0.0f,		//| 4 – 7	| Second column (typically Y-axis basis vector)
			0.0f, 0.0f, 1.0f, 0.0f,		//| 8 – 11 | Third column (typically Z-axis basis vector)
			0.0f, 0.0f, 0.0f, 1.0f};	//| 12 – 15 | Fourth column (Translation components X, Y, Z)
	return Result;
}

inline m4x4 LookAt(v3 eye, v3 center, v3 up)
{
	// 1. Calculate the forward/direction vector (pointing from target to eye for right-handed)
    v3 f = normalize(center - eye);
    
    // 2. Calculate the right vector
    v3 s = normalize(cross(f, normalize(up)));
    
    // 3. Recalculate the orthogonal up vector
    v3 u = cross(s, f);

    // Initialize an identity matrix
	m4x4 Result;// = Identity();

    // Rotation component (transposed orientation)
    Result.Elements[0] = s.x;
    Result.Elements[1] = s.y;
    Result.Elements[2] = s.z;
    Result.Elements[3] = 0.0f;
    
    Result.Elements[4] = u.x;
    Result.Elements[5] = u.y;
    Result.Elements[6] = u.z;
    Result.Elements[7] = 0.0f;
    
    Result.Elements[8] = -f.x;
    Result.Elements[9] = -f.y;
    Result.Elements[10] = -f.z;
    Result.Elements[11] = 0.0f;

    // Translation component (-eye position dot-producted with the basis vectors)
    Result.Elements[12] = -dot(s, eye);
    Result.Elements[13] = -dot(u, eye);
    Result.Elements[14] =  dot(f, eye);
    Result.Elements[15] = 1.0f;
	return Result;
}

inline m4x4 Perspective(float fovy_degrees, float aspect, float zNear, float zFar)
{
	float fovy_rad = DegreeToRadian(fovy_degrees);
	float const tanHalfFovy = tan(fovy_rad / 2.0f);

    m4x4 Result = {0};
    
    Result.Elements[0] = 1.0f / (aspect * tanHalfFovy);
    Result.Elements[5] = 1.0f / (tanHalfFovy);
    Result.Elements[10] = -(zFar + zNear) / (zFar - zNear);
    Result.Elements[11] = -1.0f;
    Result.Elements[14] = -(2.0f * zFar * zNear) / (zFar - zNear);

	return Result;
}

#endif
