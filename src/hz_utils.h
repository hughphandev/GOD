#ifndef HZ_UTILS_H
#define HZ_UTILS_H

#include "hz_types.h"

//TODO: remove maybe!
#include "string.h"
#include "stdio.h"

#define LAMBDA(return_type, function_body) \
({ \
      return_type __fn__ function_body \
          __fn__; \
})

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if SLOW
#define ASSERT(Expression) if(!(Expression)) {*(int *)0 = 0;}
inline u32 SafeTruncateUInt64(u64 value);
#else
#define ASSERT(Expression) 
#endif

#define INVALID_CODE_PATH ASSERT(!"Invalid code path!")
#define INVALID_DEFAULT_CASE default:{ASSERT(!"Invalid code path!");} 
#define INVALID_VALUE 0xffffffff

#define MEMORY_TO_FILE_ADDRESS(base, target, type) target = (type*)((u64)target - (u64)base);
#define FILE_TO_MEMORY_ADDRESS(base, target, type) target = (type*)((u64)target + (u64)base);


inline u32 SafeTruncateUInt64(u64 value)
{
  // TODO: Define maximum value
  ASSERT(value < 0xFFFFFFFF);
  return (u32)value;
}

//NOTE: non-pointer type only
#define DEFINE_SWAP(T)  void Swap(T* l, T* r){ T temp = *l; *l = *r; *r = temp; }

DEFINE_SWAP(f32)
DEFINE_SWAP(s32)

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

void MemSet(void* mem, int val, size_t size)
{
  memset(mem, val, size);
}

inline s32 StrToI(char* str, char** pStr)
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

template <typename T>
inline int FindFirstIndex(T* c, T target, int len)
{
  for (int i = 0; i < len; ++i)
  {
    if (c[i] == target) return i;
  }
  return INVALID_VALUE;
}

template <typename T>
inline int FindLastIndex(const T* c, T target, int len)
{
  for (int i = len - 1; i >= 0; ++i)
  {
    if (c[i] == target) return i;
  }
  return INVALID_VALUE;
}

inline int FindLastIndex(const char* c, char target)
{
  int result = INVALID_VALUE;
  size_t len = strlen(c);
  for (size_t i = 0; i < len; ++i)
  {
    if (c[i] == target) result = (int)i;
  }
  return result;
}

inline char* Copy(char* des, const char* src)
{
  return strcpy(des, src);
}

inline char* Copy(char* des, const char* src, int count)
{
  return strncpy(des, src, count);
}

#ifdef DEBUG
#define LOGINFO(string, ...) fprintf(stdout, string, __VA_ARGS__);
#define LOGWARNING(string, ...) fprintf(stdout, string, __VA_ARGS__);
#define LOGERROR(string, ...) fprintf(stderr, string, __VA_ARGS__);
#elif
#define LOGINFO(string, ...)
#define LOGWARNING(string, ...) 
#define LOGERROR(string, ...) 
#endif


#endif