#ifndef HZ_UTILS_H
#define HZ_UTILS_H

#include "crt.h"
#include "hz_types.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if SLOW
#define ASSERT(Expression) if(!(Expression)) {*(int *)0 = 0;}
inline u32 SafeTruncateUInt64(u64 value);
#else
#define ASSERT(Expression) 
#endif

#define INVALID_CODE_PATH ASSERT(!"Invalid code path!")
#define INVALID_DEFAULT_CASE default:{ASSERT(!"Invalid code path!");} 

inline u32 SafeTruncateUInt64(u64 value)
{
  // TODO: Define maximum value
  ASSERT(value < 0xFFFFFFFF);
  return (u32)value;
}

//NOTE: non-pointer type only
#define DEFINE_SWAP(T)  void Swap(T* l, T* r){ T temp = *l; *l = *r; *r = temp; }

DEFINE_SWAP(f32)
DEFINE_SWAP(i32)

f32 Min(f32* value, int count)
{
  ASSERT(count > 0);

  f32 result = value[0];
  for (int i = 0; i < count; ++i)
  {
    if (result > value[i])
    {
      result = value[i];
    }
  }
  return result;
}

f32 Max(f32* value, int count)
{
  ASSERT(count > 0);

  f32 result = value[0];
  for (int i = 0; i < count; ++i)
  {
    if (result < value[i])
    {
      result = value[i];
    }
  }
  return result;
}


void Memcpy(void* dest, void* src, size_t size)
{
  memcpy(dest, src, size);
}

void ZeroSize(void* mem, size_t size)
{
  memset(mem, 0, size);
}

inline i32 StrToI(char* str, char** pStr)
{
  int sign = 1;
  int result = 0;
  while (*str == '-')
  {
    sign *= -1;
    ++str;
  }
  while (*str >= '0' && *str <= '9')
  {
    result = result * 10 + (*str++ - '0');
  }
  return result * sign;
}


inline size_t CountSubString(char* input, char* subString)
{
  size_t result = 0;
  size_t strLen = strlen(subString);
  for (char* c = input; *c != '\0'; ++c)
  {
    if (strncmp(c, subString, strLen) == 0)
    {
      ++result;
      c += strLen - 1;
    }
  }
  return result;
}

inline char* Skip(char* c, char skip)
{
  while (*c == skip) ++c;
  return c;
}
inline char* SkipUntil(char* c, char until)
{
  while (*c != until) ++c;
  return c;
}
#endif