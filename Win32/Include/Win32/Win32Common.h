#pragma once

namespace Win32
{
    struct WinMessage
    {
        HWND hWnd;
        UINT message;
        WPARAM wParam;
        LPARAM lParam;
    };

}
