#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include <timeapi.h>

#include <sys/stat.h> 
#include "hz_utils.h"
#include "hz_math.h"
#include "hz_vulkan.h"
// #include "hz_d3d11.h"
#include "game.h"

typedef decltype(Init) GameInit;
typedef decltype(Update) GameUpdate;

struct Win32GameCode
{
    HMODULE gameDll;
    GameInit* InitGame;
    GameUpdate* UpdateGame;
};


Win32GameCode Win32LoadGameCode(char* dll)
{
    Win32GameCode result = {};
    result.gameDll = LoadLibraryA(dll);
    result.InitGame = (GameInit*)GetProcAddress(result.gameDll, "Init");
    result.UpdateGame = (GameUpdate*)GetProcAddress(result.gameDll, "Update");
    return result;
}

void Win32ReloadGameCode(Win32GameCode* gameCode)
{
    static long long lastEditTimeTimeStampDll;

    struct stat st;
    if (stat("game_temp.dll", &st) == 0)
    {
        long long currentEditTimeStampDll = st.st_mtime;
        if (currentEditTimeStampDll > lastEditTimeTimeStampDll)
        {
            if (gameCode->gameDll)
            {
                FreeLibrary(gameCode->gameDll);
                gameCode->gameDll = NULL;
            }

            while (!CopyFile("game_temp.dll", "game.dll", FALSE))
            {
                Sleep(10);
            }
            DeleteFile("game_temp.dll");

            *gameCode = Win32LoadGameCode("game.dll");

            lastEditTimeTimeStampDll = currentEditTimeStampDll;
        }
    }
}


inline static u64 Win32GetPerfCounter()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

inline static u64 Win32GetPerfFrequency()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}

LRESULT Win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_DESROY\n");
        }
        break;

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_CLOSE\n");
        }
        break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;

        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        }
        break;
    }
    return result;
}

static void Win32ProcessKeyboardInput(ButtonState* button, bool isDown)
{
    ASSERT(button->isDown != isDown);
    button->isDown = isDown;
    button->halfTransitionCount++;
}

static void Win32ProcessPendingMessage(GameState* gameState)
{
    gameState->input.dMouse = {};
    for (int i = 0; i < MAX_BUTTON; ++i)
    {
        gameState->input.buttons[i].halfTransitionCount = 0;
    }

    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        switch (msg.message)
        {
            case WM_QUIT:
            {
                gameState->running = false;
            }
            break;
            case WM_INPUT:
            {
                UINT dwSize;

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                LPBYTE lpb = new BYTE[dwSize];

                if (GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                    OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

                RAWINPUT* raw = (RAWINPUT*)lpb;
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    gameState->input.dMouse.x += raw->data.mouse.lLastX;
                    gameState->input.dMouse.y += raw->data.mouse.lLastY;
                }
                delete[] lpb;
            } break;
            case WM_MOUSEMOVE:
            {
                gameState->input.mouse.x = GET_X_LPARAM(msg.lParam);
                gameState->input.mouse.y = GET_Y_LPARAM(msg.lParam);

                gameState->input.mouseButtonState[0] = (msg.wParam & MK_LBUTTON) != 0;
                gameState->input.mouseButtonState[1] = (msg.wParam & MK_RBUTTON) != 0;
                gameState->input.mouseButtonState[2] = (msg.wParam & MK_MBUTTON) != 0;
                gameState->input.mouseButtonState[3] = (msg.wParam & MK_XBUTTON1) != 0;
                gameState->input.mouseButtonState[4] = (msg.wParam & MK_XBUTTON2) != 0;
            }
            case WM_MOUSEWHEEL:
            {
                gameState->input.dMouse.z = GET_WHEEL_DELTA_WPARAM(msg.wParam);
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                bool isDown = ((msg.lParam & (1 << 31)) == 0);

                if (isDown != wasDown)
                {
                    if (msg.wParam == 'W')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.up, isDown);
                    }
                    if (msg.wParam == 'S')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.down, isDown);
                    }
                    if (msg.wParam == 'A')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.left, isDown);
                    }
                    if (msg.wParam == 'D')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.right, isDown);
                    }
                    if (msg.wParam == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.escape, isDown);
                    }
                    if (msg.wParam == VK_SPACE)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.space, isDown);
                    }
                    if (msg.wParam == VK_F1)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.f1, isDown);
                    }
                    if (msg.wParam == VK_F3)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.f3, isDown);
                    }
                    if (isDown)
                    {
                        bool32 altKey = (msg.lParam & (1 << 29));
                        if (altKey && msg.wParam == VK_F4)
                        {
                            PostQuitMessage(0);
                        }
                    }
                }
            }
            break;
            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            break;
        }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    GameMemory gameMemory;
    size_t persistantArenaSize = MEGABYTES(64);
    size_t transientArenaSize = MEGABYTES(64);
    size_t pushBufferSize = MEGABYTES(4);
    void* memory = VirtualAlloc(0, persistantArenaSize + transientArenaSize + pushBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    InitMemoryArena(&gameMemory.persistantArena, persistantArenaSize, memory);
    InitMemoryArena(&gameMemory.transientArena, transientArenaSize, (u8*)memory + persistantArenaSize);
    GameState* gameState = PUSH_TYPE(&gameMemory.persistantArena, GameState);

    RenderGroup* renderGroup = PUSH_TYPE(&gameMemory.persistantArena, RenderGroup);
    InitMemoryArena(&renderGroup->pushBuffer, pushBufferSize, (u8*)memory + persistantArenaSize + transientArenaSize);

    gameState->width = 1280;
    gameState->height = 720;

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    //  windowClass.hIcon;
    windowClass.lpszClassName = "GOD";

    u64 perfFrequency = Win32GetPerfFrequency();

    if (RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, windowClass.lpszClassName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, gameState->width, gameState->height, 0, 0, instance, 0);


        if (windowHandle)
        {
            RAWINPUTDEVICE rawInputDevices[1];
            rawInputDevices[0].usUsagePage = 0x01;  // Mouse
            rawInputDevices[0].usUsage = 0x02;      // Mouse
            rawInputDevices[0].dwFlags = 0;
            rawInputDevices[0].hwndTarget = windowHandle;
            RegisterRawInputDevices(rawInputDevices, 1, sizeof(RAWINPUTDEVICE));

            Renderer renderer = {};
            renderGroup->renderer = &renderer;
            // D3D11InitScene(gameState->width, gameState->height, renderGroup, &gameMemory.persistantArena, windowHandle);
            HZVKInit(renderGroup->renderer, instance, windowHandle, "GOD", &gameMemory.persistantArena);

            Win32GameCode gameCode = Win32LoadGameCode("game.dll");

            gameState->api.UpdateMesh = {}; //Win32UpdateMesh;
            gameState->api.UploadModel = {}; //Win32UploadModel;
            gameState->api.UploadTexture = {}; //Win32UploadTexture;
            gameCode.InitGame(gameState, renderGroup, &gameMemory);

            bool timeIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;
            u64 lastPerfCounter = Win32GetPerfCounter();
            while (gameState->running)
            {
                Win32ReloadGameCode(&gameCode);
                Win32ProcessPendingMessage(gameState);

                ResetMemoryArena(&gameMemory.transientArena);
                ResetMemoryArena(&renderGroup->pushBuffer);
                gameCode.UpdateGame(gameState, renderGroup, &gameMemory);

                RECT gameRect;
                GetWindowRect(windowHandle, &gameRect);
                if (gameState->lockCursor) SetCursorPos((gameRect.left + gameRect.right) / 2, (gameRect.top + gameRect.bottom) / 2);
                ShowCursor(gameState->showCursor);

                // D3D11RenderOutput(renderGroup);
                HZVKRenderOutput(renderGroup);

                f32 elapsed = (f32)(Win32GetPerfCounter() - lastPerfCounter) / perfFrequency;

                if (elapsed < gameState->dt)
                {
                    if (timeIsGranular)
                    {
                        Sleep((DWORD)((gameState->dt - elapsed) * 1000));
                    }
                    while (elapsed < gameState->dt)
                    {
                        elapsed = (f32)(Win32GetPerfCounter() - lastPerfCounter) / perfFrequency;
                    }
                }
                else
                {
                    //TODO: Missed framerate!
                    LOG_WARNING("Missed");
                }
                // LOGINFO("%.2fms\n", elapsed * 1000);
                lastPerfCounter = Win32GetPerfCounter();
                gameState->t += gameState->dt;
            }
            CloseWindow(windowHandle);
        }
        else
        {
            //TODO: Logging!
        }
    }
    else
    {
        //TODO: Logging!
    }

    return 0;
}
