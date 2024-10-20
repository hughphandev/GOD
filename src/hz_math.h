#ifndef HZ_MATH_H
#define HZ_MATH_H

#include "hz_types.h"
#include "hz_utils.h"

#define DEFINE_LERP(T) T Lerp(T a, T b, f32 t) { return (a + ((b - a) * t)); }
#ifndef EPSILON 
#define EPSILON 0.00001f
#endif
#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef DEG2RAD
#define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f/PI)
#endif

struct Vec2
{
  f32 x, y;
};

struct Vec2I
{
  i32 x, y;
};

struct Vec3I
{
  i32 x, y, z;
};

struct Vec3
{
  f32 x, y, z;
};

union Vec4
{
  struct
  {
    f32 x, y, z, w;
  };
  struct
  {
    f32 r, g, b, a;
  };

  f32 e[4];
};
typedef Vec4 Quaternion;
typedef Vec4 Color;

union Mat4
{
  f32 e[4][4];
  struct
  {
    Vec4 r0, r1, r2, r3;
  };
};

struct Mat4Inverse
{
  bool isExisted;
  f32 det;
  Mat4 inv;
};

DEFINE_SWAP(Vec2)
DEFINE_SWAP(Vec3)
DEFINE_SWAP(Vec4)

union Rect
{
  struct
  {
    Vec2 pos;
    Vec2 size;
  };
  struct
  {
    f32 x, y, width, height;
  };

  inline Vec2 Rect::GetMinBound()
  {
    //TODO: Test
    return { this->pos.x - (this->width / 2.0f), this->pos.y - (this->height / 2.0f) };
  }

  inline Vec2 Rect::GetMaxBound()
  {
    //TODO: Test
    return { this->pos.x + (this->width / 2.0f), this->pos.y + (this->height / 2.0f) };
  }
};

struct Box
{
  Vec3 pos;
  Vec3 size;
};

// Implementation

//TODO: Use math.h for now. Switch to platform efficient code in the future!
#include <math.h>

inline f32 Floor(f32 value)
{
  return floorf(value);
}
inline f32 Ceil(f32 value)
{
  return ceilf(value);
}

inline f32 Round(f32 value)
{
  return roundf(value);
}

inline f32 Sin(f32 value)
{
  return sinf(value);
}
inline f32 Cos(f32 value)
{
  return cosf(value);
}
inline f32 ACos(f32 value)
{
  return acosf(value);
}
inline f32 Tan(f32 value)
{
  return tanf(value);
}
inline f32 Atan2(f32 y, f32 x)
{
  return atan2f(y, x);
}
inline f32 Atan(f32 value)
{
  return atanf(value);
}

inline f32 Q_rsqrt(f32 value)
{
  long i;
  f32 x2, y;
  const f32 threeHalfs = 1.5f;

  x2 = value * 0.5f;
  y = value;
  i = *(long*)&y;
  i = 0x5f3759df - (i >> 1);
  y = *(f32*)&i;

  //NOTE: newton iteration, duplicate to increase precision!
  y = y * (threeHalfs - (x2 * y * y));

  return y;
}

inline f32 Sqr(f32 x)
{
  return (x * x);
}

inline f32 Sqrt(f32 x)
{
  return sqrtf(x);
}

inline f32 Abs(f32 value)
{
  u32 bit = *(u32*)&value;
  bit = bit & (0x7FFFFFFF);
  value = *(f32*)&bit;
  return value;
}

inline Vec2 Round(Vec2 value)
{
  return { Round(value.x), Round(value.y) };
}

inline f32 Clamp01(f32 a)
{
  if (a > 1.0f) return 1.0f;
  if (a < 0.0f) return 0.0f;
  return a;
}

inline f32 Clamp(f32 a, f32 min, f32 max)
{
  f32 result = a;
  if (a < min) result = min;
  if (a > max) result = max;
  return result;
}
inline i32 Clamp(i32 a, i32 min, i32 max)
{
  i32 result = a;
  if (a < min) result = min;
  if (a > max) result = max;
  return result;
}

inline f32 Min(f32 a, f32 b)
{
  return a < b ? a : b;
}

inline u32 RoundToU32(f32 value)
{
  return (u32)(value + 0.5f);
}

inline f32 Max(f32 a, f32 b)
{
  return a > b ? a : b;
}

inline Vec2 V2(f32 x, f32 y)
{
  return { x, y };
}

inline Vec2 V2(i32 x, i32 y)
{
  return { (f32)x, (f32)y };
}

inline f32 Length(Vec2 v)
{
  return Sqrt(Sqr(v.x) + Sqr(v.y));
}

inline Vec2 Clamp(Vec2 a, Vec2 min, Vec2 max)
{
  Vec2 result;
  result.x = Clamp(a.x, min.x, max.x);
  result.y = Clamp(a.y, min.y, max.y);
  return result;
}

inline f32 Dot(Vec2 a, Vec2 b)
{
  return (a.x * b.x + a.y * b.y);
}

inline Vec2 operator*(Vec2 a, f32 b)
{
  return { a.x * b, a.y * b };
}
inline Vec2 operator*(f32 a, Vec2 b)
{
  return b * a;
}
inline Vec2 operator/(Vec2 a, f32 b)
{
  return { a.x / b, a.y / b };
}
inline Vec2 operator+(Vec2 a, f32 b)
{
  return { a.x + b, a.y + b };
}
inline Vec2 operator-(Vec2 a, f32 b)
{
  return { a.x - b, a.y - b };
}

inline Vec2 operator*(Vec2 a, Vec2 b)
{
  return { a.x * b.x, a.y * b.y };
}
inline Vec2 operator/(Vec2 a, Vec2 b)
{
  return { a.x / b.x, a.y / b.y };
}
inline Vec2 operator+(Vec2 a, Vec2 b)
{
  return { a.x + b.x, a.y + b.y };
}
inline Vec2 operator-(Vec2 a, Vec2 b)
{
  return { a.x - b.x, a.y - b.y };
}

inline Vec2 operator*=(Vec2& a, Vec2 b)
{
  return a = (a * b);
}
inline Vec2 operator/=(Vec2& a, Vec2 b)
{
  return a = (a / b);
}
inline Vec2 operator+=(Vec2& a, Vec2 b)
{
  return a = (a + b);
}
inline Vec2 operator-=(Vec2& a, Vec2 b)
{
  return a = (a - b);
}
inline bool operator==(Vec2& a, Vec2 b)
{
  return (a.x == b.x && a.y == b.y);
}
inline bool operator!=(Vec2 a, Vec2 b)
{
  return (a.x != b.x || a.y != b.y);
}
inline bool operator<=(Vec2 a, Vec2 b)
{
  return (a.x <= b.x && a.y <= b.y);
}
inline bool operator<(Vec2 a, Vec2 b)
{
  return (a.x < b.x && a.y < b.y);
}
inline bool operator>(Vec2 a, Vec2 b)
{
  return (a.x > b.x && a.y > b.y);
}

inline Vec2 operator*=(Vec2& a, f32 b)
{
  return a = (a * b);
}
inline Vec2 operator/=(Vec2& a, f32 b)
{
  return a = (a / a);
}
inline Vec2 operator+=(Vec2& a, f32 b)
{
  return a = (a + b);
}
inline Vec2 operator-=(Vec2 a, f32 b)
{
  return a = (a - b);
}
inline bool operator==(Vec2 a, f32 b)
{
  return (a.x == b && a.y == b);
}
inline bool operator!=(Vec2 a, f32 b)
{
  return (a.x != b || a.y != b);
}
inline bool operator<=(Vec2 a, f32 b)
{
  return (a.x <= b && a.y <= b);
}
inline bool operator<(Vec2 a, f32 b)
{
  return (a.x < b && a.y < b);
}
inline bool operator>(Vec2 a, f32 b)
{
  return (a.x > b && a.y > b);
}
inline Vec2 operator-(Vec2 a)
{
  return { -a.x, -a.y };
}

inline Vec2 Abs(Vec2 v)
{
  return { Abs(v.x), Abs(v.y) };
}

inline Vec2 Perp(Vec2 v)
{
  return { -v.y, v.x };
}

inline Vec2 Normalize(Vec2 v)
{
  if (v == 0.0f) return { 0.0f, 0.0f };
  f32 rsqrtMagtitude = Q_rsqrt(Sqr(v.x) + Sqr(v.y));
  return v * rsqrtMagtitude;
}

inline bool IsApproximate(Vec2 a, Vec2 b, f32 delta = EPSILON)
{
  return Abs(a.x - b.x) < delta && Abs(a.y - b.y) < delta;
}

inline Vec3 V3(f32 x, f32 y, f32 z)
{
  return { x, y, z };
}
inline Vec3 V3(Vec2 xy, f32 z)
{
  return { xy.x, xy.y, z };
}
inline Vec3 operator-(Vec3& a)
{
  return { -a.x, -a.y, -a.z };
}

inline Vec3 operator*(Vec3 a, f32 b)
{
  return { a.x * b, a.y * b, a.z * b };
}
inline Vec3 operator*(f32 a, Vec3 b)
{
  return b * a;
}
inline Vec3 operator/(Vec3 a, f32 b)
{
  return { a.x / b, a.y / b, a.z / b };
}
inline Vec3 operator+(Vec3 a, f32 b)
{
  return { a.x + b, a.y + b, a.z + b };
}
inline Vec3 operator-(Vec3 a, f32 b)
{
  return { a.x - b, a.y - b, a.z - b };
}
inline Vec3 operator*(Vec3 a, Vec3 b)
{
  return { a.x * b.x, a.y * b.y, a.z * b.z };
}
inline Vec3 operator/(Vec3 a, Vec3 b)
{
  return { a.x / b.x, a.y / b.y, a.z / b.z };
}
inline Vec3 operator+(Vec3 a, Vec3 b)
{
  return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline Vec3 operator-(Vec3 a, Vec3 b)
{
  return { a.x - b.x, a.y - b.y, a.z - b.z };
}
inline Vec3 operator*=(Vec3& a, Vec3 b)
{
  return a = (a * b);
}
inline Vec3 operator/=(Vec3& a, Vec3 b)
{
  return a = (a / b);
}
inline Vec3 operator+=(Vec3& a, Vec3 b)
{
  return a = (a + b);
}
inline Vec3 operator-=(Vec3& a, Vec3 b)
{
  return a = (a - b);
}
inline bool operator==(Vec3 a, Vec3 b)
{
  return (a.x == b.x && a.y == b.y && a.z == b.z);
}
inline bool operator!=(Vec3 a, Vec3 b)
{
  return !(a == b);
}
inline bool operator<=(Vec3 a, Vec3 b)
{
  return (a.x <= b.x && a.y <= b.y && a.z <= b.z);
}
inline bool operator<(Vec3 a, Vec3 b)
{
  return (a.x < b.x && a.y < b.y && a.z < b.z);
}
inline bool operator>(Vec3 a, Vec3 b)
{
  return (a.x > b.x && a.y > b.y && a.z > b.z);
}

inline Vec3 operator*=(Vec3& a, f32 b)
{
  return a = (a * b);
}
inline Vec3 operator/=(Vec3& a, f32 b)
{
  return a = (a / b);
}
inline Vec3 operator+=(Vec3& a, f32 b)
{
  return a = (a + b);
}
inline Vec3 operator-=(Vec3& a, f32 b)
{
  return a = (a - b);
}
inline bool operator==(Vec3 a, f32 b)
{
  return (a.x == b && a.y == b && a.z == b);
}
inline bool operator!=(Vec3 a, f32 b)
{
  return !(a == b);
}
inline bool operator<=(Vec3 a, f32 b)
{
  return (a.x <= b && a.y <= b && a.z <= b);
}
inline bool operator<(Vec3 a, f32 b)
{
  return (a.x < b && a.y < b && a.z < b);
}
inline bool operator>(Vec3 a, f32 b)
{
  return (a.x > b && a.y > b && a.z > b);
}

inline bool IsApproximate(Vec3 a, Vec3 b, f32 delta = EPSILON)
{
  return Abs(a.x - b.x) < delta && Abs(a.y - b.y) < delta && Abs(a.z - b.z) < delta;
}

inline Vec3 Normalize(Vec3 v)
{
  if (v == 0.0f) return {};
  f32 rsqrtMagtitude = Q_rsqrt(Sqr(v.x) + Sqr(v.y) + Sqr(v.z));
  return v * rsqrtMagtitude;
}

inline f32 Length(Vec3 v)
{
  return Sqrt(Sqr(v.x) + Sqr(v.y) + Sqr(v.z));
}

inline Vec3 Clamp(Vec3 a, Vec3 min, Vec3 max)
{
  Vec3 result;
  result.x = Clamp(a.x, min.x, max.x);
  result.y = Clamp(a.y, min.y, max.y);
  result.z = Clamp(a.z, min.z, max.z);
  return result;
}

inline Vec3 Round(Vec3 value)
{
  return V3(Round(value.x), Round(value.y), Round(value.z));
}

inline Vec3 Cross(Vec3 a, Vec3 b)
{
  Vec3 result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

inline f32 Dot(Vec3 a, Vec3 b)
{
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}


Vec3 RotateBy(Vec3 v, Vec3 axis, float angle)
{
  Vec3 result = v;

  // Vector3Normalize(axis);
  float length = Sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
  if (length == 0.0f) length = 1.0f;
  float ilength = 1.0f / length;
  axis.x *= ilength;
  axis.y *= ilength;
  axis.z *= ilength;

  angle /= 2.0f;
  float a = Sin(angle);
  float b = axis.x * a;
  float c = axis.y * a;
  float d = axis.z * a;
  a = Cos(angle);
  Vec3 w = { b, c, d };

  // Vector3CrossProduct(w, v)
  Vec3 wv = { w.y * v.z - w.z * v.y, w.z * v.x - w.x * v.z, w.x * v.y - w.y * v.x };

  // Vector3CrossProduct(w, wv)
  Vec3 wwv = { w.y * wv.z - w.z * wv.y, w.z * wv.x - w.x * wv.z, w.x * wv.y - w.y * wv.x };

  // Vector3Scale(wv, 2*a)
  a *= 2;
  wv.x *= a;
  wv.y *= a;
  wv.z *= a;

  // Vector3Scale(wwv, 2)
  wwv.x *= 2;
  wwv.y *= 2;
  wwv.z *= 2;

  result.x += wv.x;
  result.y += wv.y;
  result.z += wv.z;

  result.x += wwv.x;
  result.y += wwv.y;
  result.z += wwv.z;

  return result;
}

inline Vec4 operator*(Vec4 a, f32 b)
{
  return { a.x * b, a.y * b, a.z * b, a.w * b };
}
inline Vec4 operator*(f32 a, Vec4 b)
{
  return b * a;
}
inline Vec4 operator/(Vec4 a, f32 b)
{
  return { a.x / b, a.y / b, a.z / b, a.w / b };
}
inline Vec4 operator+(Vec4 a, f32 b)
{
  return { a.x + b, a.y + b, a.z + b, a.w + b };
}
inline Vec4 operator-(Vec4 a, f32 b)
{
  return { a.x - b, a.y - b, a.z - b, a.w - b };
}

inline Vec4 operator*(Vec4 a, Vec4 b)
{
  return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}
inline Vec4 operator/(Vec4 a, Vec4 b)
{
  return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}
inline Vec4 operator+(Vec4 a, Vec4 b)
{
  return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
inline Vec4 operator-(Vec4 a, Vec4 b)
{
  return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}
inline Vec4 operator*=(Vec4& a, Vec4 b)
{
  return a = (a * b);
}
inline Vec4 operator/=(Vec4& a, Vec4 b)
{
  return a = (a / b);
}
inline Vec4 operator+=(Vec4& a, Vec4 b)
{
  return a = (a + b);
}
inline Vec4 operator-=(Vec4& a, Vec4 b)
{
  return a = (a - b);
}
inline bool operator==(Vec4 a, Vec4 b)
{
  return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}
inline bool operator!=(Vec4 a, Vec4 b)
{
  return !(a == b);
}
inline bool operator<=(Vec4 a, Vec4 b)
{
  return (a.x <= b.x && a.y <= b.y && a.z <= b.z && a.w <= b.w);
}
inline bool operator<(Vec4 a, Vec4 b)
{
  return (a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w);
}
inline bool operator>(Vec4 a, Vec4 b)
{
  return (a.x > b.x && a.y > b.y && a.z > b.z && a.w > b.w);
}

inline Vec4 operator*=(Vec4& a, f32 b)
{
  return a = (a * b);
}
inline Vec4 operator/=(Vec4& a, f32 b)
{
  return a = (a / b);
}
inline Vec4 operator+=(Vec4& a, f32 b)
{
  return a = (a + b);
}
inline Vec4 operator-=(Vec4& a, f32 b)
{
  return a = (a - b);
}
inline bool operator==(Vec4 a, f32 b)
{
  return (a.x == b && a.y == b && a.z == b && a.w == b);
}
inline bool operator!=(Vec4 a, f32 b)
{
  return !(a == b);
}
inline bool operator<=(Vec4 a, f32 b)
{
  return (a.x <= b && a.y <= b && a.z <= b && a.w <= b);
}
inline bool operator<(Vec4 a, f32 b)
{
  return (a.x < b && a.y < b && a.z < b && a.w < b);
}
inline bool operator>(Vec4 a, f32 b)
{
  return (a.x > b && a.y > b && a.z > b && a.w > b);
}
inline Vec4 operator-(Vec4& a)
{
  return { -a.x, -a.y, -a.z, -a.w };
}
inline bool IsApproximate(Vec4 a, Vec4 b, f32 delta = EPSILON)
{
  return Abs(a.x - b.x) < delta && Abs(a.y - b.y) < delta && Abs(a.z - b.z) < delta && Abs(a.w - b.w) < delta;
}
inline Vec4 Normalize(Vec4 v)
{
  if (v == 0.0f) return {};
  f32 rsqrtMagtitude = Q_rsqrt(Sqr(v.x) + Sqr(v.y) + Sqr(v.z) + Sqr(v.w));
  return v * rsqrtMagtitude;
}

inline f32 Length(Vec4 v)
{
  return Sqrt(Sqr(v.x) + Sqr(v.y) + Sqr(v.z) + Sqr(v.w));
}

inline Vec4 V4(f32 x, f32 y, f32 z, f32 w)
{
  return { x, y, z, w };
}

inline Vec4 V4(Vec3 xyz, f32 w)
{
  return { xyz.x, xyz.y, xyz.z, w };
}

u32 ToU32Color(Vec4 a)
{
  u32 red = a.x < 0.0f ? 0 : RoundToU32(a.x * 255.0f);
  u32 green = a.y < 0.0f ? 0 : RoundToU32(a.y * 255.0f);
  u32 blue = a.z < 0.0f ? 0 : RoundToU32(a.z * 255.0f);
  u32 alpha = a.w < 0.0f ? 0 : RoundToU32(a.w * 255.0f);

  return (alpha << 24) | (blue << 16) | (green << 8) | (red << 0);
}

Quaternion FromEuler(float pitch, float yaw, float roll)
{
  Quaternion result = { 0 };

  float x0 = Cos(pitch * 0.5f);
  float x1 = Sin(pitch * 0.5f);
  float y0 = Cos(yaw * 0.5f);
  float y1 = Sin(yaw * 0.5f);
  float z0 = Cos(roll * 0.5f);
  float z1 = Sin(roll * 0.5f);

  result.x = x1 * y0 * z0 - x0 * y1 * z1;
  result.y = x0 * y1 * z0 + x1 * y0 * z1;
  result.z = x0 * y0 * z1 - x1 * y1 * z0;
  result.w = x0 * y0 * z0 + x1 * y1 * z1;

  return result;
}

Quaternion FromAxisAngle(Vec3 axis, f32 angle)
{
  Quaternion output = {};
  output.w = Cos(angle / 2.0f);
  f32 c = Sin(angle / 2.0f);
  output.x = c * axis.x;
  output.y = c * axis.y;
  output.z = c * axis.z;
  return output;
}
void ToAxisAngle(Quaternion q, Vec3* outAxis, f32* outAngle)
{
  if (Abs(q.w) > 1.0f)
  {
    // QuaternionNormalize(q);
    float length = Sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (length == 0.0f) length = 1.0f;
    float ilength = 1.0f / length;

    q.x = q.x * ilength;
    q.y = q.y * ilength;
    q.z = q.z * ilength;
    q.w = q.w * ilength;
  }

  Vec3 resAxis = { 0.0f, 0.0f, 0.0f };
  float resAngle = 2.0f * Cos(q.w);
  float den = Sqrt(1.0f - q.w * q.w);

  if (den > EPSILON)
  {
    resAxis.x = q.x / den;
    resAxis.y = q.y / den;
    resAxis.z = q.z / den;
  }
  else
  {
    // This occurs when the angle is zero.
    // Not a problem: just set an arbitrary normalized axis.
    resAxis.x = 1.0f;
  }

  *outAxis = resAxis;
  *outAngle = resAngle;
}

Vec3 Rotate(Vec3 v, Quaternion q)
{
  Vec3 result = { 0 };

  result.x = v.x * (q.x * q.x + q.w * q.w - q.y * q.y - q.z * q.z) + v.y * (2 * q.x * q.y - 2 * q.w * q.z) + v.z * (2 * q.x * q.z + 2 * q.w * q.y);
  result.y = v.x * (2 * q.w * q.z + 2 * q.x * q.y) + v.y * (q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z) + v.z * (-2 * q.w * q.x + 2 * q.y * q.z);
  result.z = v.x * (-2 * q.w * q.y + 2 * q.x * q.z) + v.y * (2 * q.w * q.x + 2 * q.y * q.z) + v.z * (q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);

  return result;
}

DEFINE_LERP(f32);
DEFINE_LERP(Vec2);
DEFINE_LERP(Vec3);
DEFINE_LERP(Vec4);

//
// Matrix operation
//
inline Mat4 operator*(Mat4 a, Mat4 b)
{
  Mat4 result = {};

  for (i32 i = 0; i < 4; ++i)
  {
    for (i32 j = 0; j < 4; ++j)
    {
      for (i32 k = 0; k < 4; ++k)
      {
        result.e[i][j] += a.e[i][k] * b.e[k][j];
      }
    }
  }
  return result;
}

inline Mat4 operator+(Mat4 a, Mat4 b)
{
  Mat4 result;
  f32* e = (f32*)result.e;
  f32* e1 = (f32*)a.e;
  f32* e2 = (f32*)b.e;

  for (i32 i = 0; i < 16; ++i)
  {
    e[i] = e1[i] + e2[i];
  }
  return result;
}

inline Mat4 operator-(Mat4 a, Mat4 b)
{
  Mat4 result;
  f32* e = (f32*)result.e;
  f32* e1 = (f32*)a.e;
  f32* e2 = (f32*)b.e;

  for (i32 i = 0; i < 16; ++i)
  {
    e[i] = e1[i] - e2[i];
  }
  return result;
}

inline Mat4 operator*(Mat4 a, f32 b)
{
  Mat4 result;
  f32* e = (f32*)result.e;
  f32* e1 = (f32*)a.e;

  for (i32 i = 0; i < 16; ++i)
  {
    e[i] = e1[i] * b;
  }
  return result;
}

inline Mat4 operator*(f32 a, Mat4 b)
{
  Mat4 result;
  f32* e = (f32*)result.e;
  f32* e2 = (f32*)b.e;

  for (i32 i = 0; i < 16; ++i)
  {
    e[i] = a * e2[i];
  }
  return result;
}

inline Mat4 operator/(Mat4 a, f32 b)
{
  Mat4 result;
  f32* e = (f32*)result.e;
  f32* e1 = (f32*)a.e;

  for (i32 i = 0; i < 16; ++i)
  {
    e[i] = e1[i] / b;
  }
  return result;
}

inline Mat4 operator+(Mat4 a, f32 b)
{
  Mat4 result = a;

  for (i32 i = 0; i < 4; ++i)
  {
    result.e[i][i] += b;
  }
  return result;
}

inline Mat4 operator-(Mat4 a, f32 b)
{
  Mat4 result = a;

  for (i32 i = 0; i < 16; ++i)
  {
    result.e[i][i] -= b;
  }
  return result;
}

inline bool operator==(Mat4 a, Mat4 b)
{
  bool equal = (a.r0 == b.r0) && (a.r1 == b.r1) && (a.r2 == b.r2) && (a.r3 == b.r3);
  return equal;
}

// inline Vec4 operator*(Vec4 a, Mat4 b)
// {
//   Vec4 result = {};

//   for (i32 i = 0; i < 4; ++i)
//   {
//     for (i32 j = 0; j < 4; ++j)
//     {
//       result.e[i] += b.e[j][i] * a.e[j];
//     }
//   }
//   return result;
// }
// inline Vec4 operator*(Mat4 a, Vec4 b)
// {
//   Vec4 result = {};

//   for (i32 i = 0; i < 4; ++i)
//   {
//     for (i32 j = 0; j < 4; ++j)
//     {
//       result.e[i] += a.e[i][j] * b.e[j];
//     }
//   }
//   return result;
// }

Mat4 GetViewMatrix(Vec3 position, Vec3 direction, Vec3 worldUp) {
  Vec3 right = Normalize(Cross(worldUp, direction));
  Vec3 up = Cross(direction, right);

  Mat4 result =
  {
    {
      { right.x, right.y, right.z, -Dot(right, position) },
      { up.x, up.y, up.z, -Dot(up, position) },
      { direction.x, direction.y, direction.z, -Dot(direction, position) },
      { 0, 0, 0, 1 },
    }
  };
  return result;
}

Mat4 GetPerspectiveProjection(f32 fov, f32 aspect, f32 nPlane, f32 fPlane) {
  float f = 1.0f / Tan(fov / 2.0f);
  float delta = nPlane - fPlane;

  Mat4 result = { {
          {f / aspect, 0, 0, 0},
          {0, f, 0, 0},
          {0, 0, -fPlane / delta, (fPlane * nPlane) / delta},
          {0, 0, 1, 0}
      } };

  return result;
}

Mat4 Transpose(Mat4 mat) {
  Mat4 result = {
      {
          {mat.e[0][0], mat.e[1][0], mat.e[2][0], mat.e[3][0]},
          {mat.e[0][1], mat.e[1][1], mat.e[2][1], mat.e[3][1]},
          {mat.e[0][2], mat.e[1][2], mat.e[2][2], mat.e[3][2]},
          {mat.e[0][3], mat.e[1][3], mat.e[2][3], mat.e[3][3]}
      }
  };
  return result;
}

inline f32 Det(Mat4 a)
{
  f32* m[4] = { a.e[0], a.e[1], a.e[2], a.e[3] };
  return
    m[0][3] * m[1][2] * m[2][1] * m[3][0] - m[0][2] * m[1][3] * m[2][1] * m[3][0] -
    m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][1] * m[1][3] * m[2][2] * m[3][0] +
    m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][1] * m[1][2] * m[2][3] * m[3][0] -
    m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][2] * m[1][3] * m[2][0] * m[3][1] +
    m[0][3] * m[1][0] * m[2][2] * m[3][1] - m[0][0] * m[1][3] * m[2][2] * m[3][1] -
    m[0][2] * m[1][0] * m[2][3] * m[3][1] + m[0][0] * m[1][2] * m[2][3] * m[3][1] +
    m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][1] * m[1][3] * m[2][0] * m[3][2] -
    m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][0] * m[1][3] * m[2][1] * m[3][2] +
    m[0][1] * m[1][0] * m[2][3] * m[3][2] - m[0][0] * m[1][1] * m[2][3] * m[3][2] -
    m[0][2] * m[1][1] * m[2][0] * m[3][3] + m[0][1] * m[1][2] * m[2][0] * m[3][3] +
    m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][0] * m[1][2] * m[2][1] * m[3][3] -
    m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][0] * m[1][1] * m[2][2] * m[3][3];
}

inline Mat4Inverse Inverse(Mat4 a)
{
  Mat4Inverse result = {};
  f32 inv[16];
  i32 i;
  f32* m = (f32*)a.e;

  inv[0] = m[5] * m[10] * m[15] -
    m[5] * m[11] * m[14] -
    m[9] * m[6] * m[15] +
    m[9] * m[7] * m[14] +
    m[13] * m[6] * m[11] -
    m[13] * m[7] * m[10];

  inv[4] = -m[4] * m[10] * m[15] +
    m[4] * m[11] * m[14] +
    m[8] * m[6] * m[15] -
    m[8] * m[7] * m[14] -
    m[12] * m[6] * m[11] +
    m[12] * m[7] * m[10];

  inv[8] = m[4] * m[9] * m[15] -
    m[4] * m[11] * m[13] -
    m[8] * m[5] * m[15] +
    m[8] * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];

  inv[12] = -m[4] * m[9] * m[14] +
    m[4] * m[10] * m[13] +
    m[8] * m[5] * m[14] -
    m[8] * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];

  inv[1] = -m[1] * m[10] * m[15] +
    m[1] * m[11] * m[14] +
    m[9] * m[2] * m[15] -
    m[9] * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];

  inv[5] = m[0] * m[10] * m[15] -
    m[0] * m[11] * m[14] -
    m[8] * m[2] * m[15] +
    m[8] * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];

  inv[9] = -m[0] * m[9] * m[15] +
    m[0] * m[11] * m[13] +
    m[8] * m[1] * m[15] -
    m[8] * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];

  inv[13] = m[0] * m[9] * m[14] -
    m[0] * m[10] * m[13] -
    m[8] * m[1] * m[14] +
    m[8] * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];

  inv[2] = m[1] * m[6] * m[15] -
    m[1] * m[7] * m[14] -
    m[5] * m[2] * m[15] +
    m[5] * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];

  inv[6] = -m[0] * m[6] * m[15] +
    m[0] * m[7] * m[14] +
    m[4] * m[2] * m[15] -
    m[4] * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];

  inv[10] = m[0] * m[5] * m[15] -
    m[0] * m[7] * m[13] -
    m[4] * m[1] * m[15] +
    m[4] * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];

  inv[14] = -m[0] * m[5] * m[14] +
    m[0] * m[6] * m[13] +
    m[4] * m[1] * m[14] -
    m[4] * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];

  result.det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (result.det == 0)
    return result;

  f32 invDet = 1.0f / result.det;
  result.isExisted = true;

  for (i = 0; i < 16; i++)
    ((f32*)result.inv.e)[i] = inv[i] * invDet;

  return result;
}

#endif
