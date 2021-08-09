#pragma once


namespace Win32
{

    template <typename val>
    constexpr LPSTR MakeIntResourceA(val i)
    {
        return reinterpret_cast<LPSTR>(i);
    }

    template <typename val>
    constexpr LPCWSTR MakeIntResourceW(val i)
    {
        return reinterpret_cast<LPCWSTR>(i);
    }


#ifdef UNICODE
    #define MakeIntResource(VAL) MakeIntResourceW(VAL)
#else
    #define MakeIntResource(VAL) MakeIntResourceA(VAL)
#endif // !UNICODE





    struct WinMessage
    {
        HWND hWnd;
        UINT message;
        WPARAM wParam;
        LPARAM lParam;
    };

}
