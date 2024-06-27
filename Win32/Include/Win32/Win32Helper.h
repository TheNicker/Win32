#pragma once
#include <Windows.h>
#include <LLUtils/Point.h>
#include <LLUtils/StringUtility.h>
#include <string>

namespace Win32
{
    class Win32Helper
    {
    public:
        static bool IsKeyPressed(DWORD virtualKey)
        {
            return (GetKeyState(static_cast<int>(virtualKey)) & static_cast<USHORT>(0x8000)) != 0;
        }

        static bool IsKeyToggled(DWORD virtualKey)
        {
            return (GetKeyState(static_cast<int>(virtualKey)) & static_cast<USHORT>(0x0001)) != 0;
        }

        static bool ProcessApplicationMessage()
        {
            bool shouldQuit = false;
            MSG msg;
            while ((PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) == TRUE)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                    shouldQuit = true;
            }
            return shouldQuit;
        }

        static void MessageLoop()
        {
            MSG msg;
            BOOL bRet;
            while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
            {
                if (bRet == -1)
                {
                    // handle the error and possibly exit
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }


        static POINT GetMouseCursorPosition(HWND handle)
        {
            POINT clientMousePos = GetMouseCursorPosition();
            ScreenToClient(handle, &clientMousePos);
            return clientMousePos;
        }

        static POINT GetMouseCursorPosition()
        {
            POINT mousePos;
            GetCursorPos(&mousePos);
            return mousePos;
        }

        static void MoveMouse(LLUtils::PointI32 point)
        {
            POINT mousePos;
            ::GetCursorPos(&mousePos);
            ::SetCursorPos(mousePos.x + point.x, mousePos.y + point.y);
        }

        static SIZE GetRectSize(const RECT& rect)
        {
            return{ rect.right - rect.left , rect.bottom - rect.top };
        }


        static LLUtils::native_string_type OpenFile(HWND ownerWindow)
        {
            LLUtils::native_char_type filename[MAX_PATH]{};
            OPENFILENAME ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = ownerWindow;  // If you have a window to center over, put its HANDLE here
            ofn.lpstrFilter = LLUTILS_TEXT("Any File\0*.*\0");
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = LLUTILS_TEXT("Open a file");
            ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

            return GetOpenFileName(&ofn) == TRUE ? LLUtils::native_string_type(filename) : LLUtils::native_string_type();
        }
    	static void BrowseToFile(LLUtils::native_string_type filename)
        {
        	struct  Deletor
        	{
                LPITEMIDLIST pidl = nullptr;
        		~Deletor()
        		{
                    ILFree(pidl);
        		}
                
        	} deletor;

        	deletor.pidl = ILCreateFromPath(filename.c_str());
            if (deletor.pidl != nullptr)
                SHOpenFolderAndSelectItems(deletor.pidl, 0, nullptr, 0);
        }
    	
    };
}

