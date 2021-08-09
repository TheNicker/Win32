#pragma once
#include <Windows.h>
#include <LLUtils/Exception.h>
#include <LLUtils/Templates.h>
#include <mutex>

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

        void SetRepeatInterval(DWORD repeatInterval)
        {
            if (fRepeatInterval != repeatInterval)
            {
                fRepeatInterval = repeatInterval;
                if (fEnabled == true)
                {
                    Enable(false);
                    Enable(true);
                }
            }
        }

        void SetDueTime(DWORD dueTime)
        {
            fDueTime = dueTime;
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
                                , fDueTime//_In_     DWORD               DueTime,
                                , fRepeatInterval//_In_     DWORD               Period,
                                , WT_EXECUTEINTIMERTHREAD//_In_     ULONG               Flags
                            ) == FALSE)
                        {
                            LL_EXCEPTION_SYSTEM_ERROR("Could not create timer");
                        }
                    }
                    else
                    {
                        if (ChangeTimerQueueTimer(nullptr, fTimerID, fDueTime, fRepeatInterval) == FALSE)
                            LL_EXCEPTION_SYSTEM_ERROR("Could not reenable timer");
                    }

                }
                else
                {
                    if (ChangeTimerQueueTimer(nullptr, fTimerID, INFINITE, INFINITE) == FALSE)
                    {
                        LL_EXCEPTION_SYSTEM_ERROR("Could not delete timer");

                    }
  
                }
            }
        }
  
    private:
     
        static VOID CALLBACK OnTimer(
            _In_ PVOID   lpParameter,
            [[maybe_unused]] _In_ BOOLEAN TimerOrWaitFired
        )
        {
            reinterpret_cast<HighPrecisionTimer*>(lpParameter)->ExecuteTimerFunc();        
        }


    private:
      
        void ExecuteTimerFunc()
        {
            if (fRepeatInterval == INFINITE)
                fEnabled = false;

            SendMessage(fWindowHandle, ON_TIMER_MESSAGE, reinterpret_cast<WPARAM>(this), 0);
        }



        static
            LRESULT CALLBACK WindProc(
                _In_ HWND hWnd,
                _In_ UINT Msg,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam)
        {
            switch (Msg)
            {
            case ON_TIMER_MESSAGE:
                reinterpret_cast<HighPrecisionTimer*>(wParam)->fCallback();
                return 0;
                break;
            default:
                return DefWindowProc(hWnd, Msg, wParam, lParam);
            }

        }

#pragma region Windowed timer begin

        void CreateWindowClassOnce()
        {
            std::call_once(fCreateClassOnceFlag, []()->void
                {
                    WNDCLASS wc{};
                    wc.lpfnWndProc = WindProc;
                    wc.hInstance = GetModuleHandle(nullptr);
                    wc.lpszClassName = CLASS_NAME;
                    if (RegisterClass(&wc) == 0)
                        LL_EXCEPTION_SYSTEM_ERROR("Could not create window class");
                });
          
        }

        void RegisterWindow()
        {

            CreateWindowClassOnce();

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

        void UnregisterWindow()
        {
            if (fWindowHandle != nullptr)
            {
                DestroyWindow(fWindowHandle);
                UnregisterClass(CLASS_NAME, GetModuleHandle(nullptr));
            }
        }
#pragma region Windowed timer end
        


        static constexpr LLUtils::native_char_type CLASS_NAME[] = LLUTILS_TEXT("Win32.HighPrecisionTimerWindow");
        static constexpr UINT ON_TIMER_MESSAGE = WM_USER + 1;
        static inline std::once_flag fCreateClassOnceFlag;
        bool fEnabled = false;
        HANDLE fTimerID = nullptr;
        Callback fCallback;
        DWORD fDueTime = INFINITE;
        DWORD fRepeatInterval = INFINITE;
        HWND fWindowHandle = nullptr;
    };

    
}