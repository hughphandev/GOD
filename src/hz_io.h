#ifndef HZ_IO_H
#define HZ_IO_H

#include "hz_types.h"
#include "hz_memory.h"

#define Win32_LEARN_AND_MEAN
#include <windows.h>

struct File
{
    void* content;
    u32 contentSize;
};


File ReadFile(const char* fileName, MemoryArena* arena)
{
    File result = {};

    HANDLE fileHandle = CreateFile(fileName, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            uint32_t fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
            result.content = (char*)PUSH_SIZE(arena, fileSize32);
            if (result.content)
            {
                DWORD bytesRead;
                if (ReadFile(fileHandle, result.content, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
                {
                    result.contentSize = bytesRead;
                }
                else
                {
                    FREE_SIZE(arena, result.content, fileSize32);
                    result = {};
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }
        CloseHandle(fileHandle);
    }
    else
    {
        // TODO: Logging
    }

    return result;
}


#endif