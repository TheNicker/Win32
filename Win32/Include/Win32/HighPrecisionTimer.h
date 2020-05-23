#pragma once
#include <windows.h>
#include <LLUtils/Exception.h>
#include <LLUtils/Templates.h>

namespace Win32
{
    class HighPrecisionTimer : public LLUtils::NoCopyable
    {
    public:

        using Callback = std::function<void()>;
        HighPrecisionTimer(Callback callback)
            :
            fCallback(callback)
        {
            RegisterWindow();
        }

        ~HighPrecisionTimer()
        {
            Enable(false);
            UnregisterWindow();
            if (fTimerID != nullptr)
                DeleteTimerQueueTimer(nullptr, fTimerID, INVALID_HANDLE_VALUE);
        }

    	void UnregisterWindow()
        {
	        if (fWindowHandle != nullptr)
	        {
                DestroyWindow(fWindowHandle);
	        	UnregisterClass(CLASS_NAME, GetModuleHandle(nullptr));
	        }
        }

        void SetDelay(DWORD delay)
        {
            if (fDelay != delay)
            {
                fDelay = delay;
                if (fEnabled == true)
                {
                    Enable(false);
                    Enable(true);
                }
            }
        }
    	
        void Enable(bool enable)
        {
            if (enable != fEnabled)
            {
            	
                fEnabled = enable;

                if (fEnabled)
                {
                	if (fTimerID == nullptr)
                	{
                        if (
                            CreateTimerQueueTimer(
                                &fTimerID
                                , nullptr//_In_opt_ HANDLE              TimerQueue,
                                , OnTimer//_In_     WAITORTIMERCALLBACK Callback,
                                , reinterpret_cast<PVOID>(this) //_In_opt_ PVOID               Parameter,
                                , 0//_In_     DWORD               DueTime,
                                , fDelay//_In_     DWORD               Period,
                                , WT_EXECUTEDEFAULT//_In_     ULONG               Flags
                            ) == FALSE)
                        {
                            LL_EXCEPTION_SYSTEM_ERROR("Could not create timer");
                        }
                	}
                    else
                	{
                        if (ChangeTimerQueueTimer(nullptr, fTimerID, 0, fDelay) == FALSE)
                            LL_EXCEPTION_SYSTEM_ERROR("Could not reenable timer");
                	}
                    
                }
                else
                {
                    if (ChangeTimerQueueTimer(nullptr, fTimerID, INFINITE, fDelay) == FALSE)
                    //if (DeleteTimerQueueTimer(nullptr, fTimerID, fWaitable) == FALSE)
                    {
                        LL_EXCEPTION_SYSTEM_ERROR("Could not delete timer");

                    }
//                    fTimerID = nullptr;
                }
            }
        }

    private:
        static
            LRESULT CALLBACK WindProc(
                _In_ HWND hWnd,
                _In_ UINT Msg,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam)
        {
            switch (Msg)
            {
            case WM_USER + 1:
                reinterpret_cast<HighPrecisionTimer*>(wParam)->fCallback();
                return 0;
                break;
            default:
                return DefWindowProc(hWnd, Msg, wParam, lParam);
            }
            
        }
    	void RegisterWindow()
    	{
            WNDCLASS wc{};
            wc.lpfnWndProc = WindProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = CLASS_NAME;
            RegisterClass(&wc);


            // Create the window.

            fWindowHandle
                = CreateWindowEx(
                    0,                              // Optional window styles.
                    CLASS_NAME,                     // Window class
                    nullptr,    // Window text
                    WS_OVERLAPPEDWINDOW,            // Window style

                    // Size and position
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                    nullptr,       // Parent window    
                    nullptr,       // Menu
                    GetModuleHandle(nullptr),  // Instance handle
                    nullptr        // Additional application data
                );
    	}
        static VOID CALLBACK OnTimer(
            _In_ PVOID   lpParameter,
            _In_ BOOLEAN TimerOrWaitFired
        )
        {
            auto _this = reinterpret_cast<HighPrecisionTimer*>(lpParameter);
            HWND windowHandle = _this->fWindowHandle;
            SendMessage(windowHandle, WM_USER + 1, reinterpret_cast<WPARAM>(_this), 0);
        }

    private:
        static inline const LLUtils::native_char_type CLASS_NAME[] = LLUTILS_TEXT("Win32.HighPrecisionTimerWindow");
        HWND fWindowHandle = nullptr;
        bool fEnabled = false;
        HANDLE fTimerID = nullptr;
        Callback fCallback;
        DWORD fDelay = 50;
    };
}