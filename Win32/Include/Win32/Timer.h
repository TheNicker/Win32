#pragma once
#include <LLUtils/UniqueIDProvider.h>
#include <LLUtils/Singleton.h>
#include <map>
#include <set>
#include <Windows.h>
#include <LLUtils/Exception.h>

namespace Win32
{
    class Timer;
    class TimerManager : public LLUtils::Singleton<TimerManager>
    {
    private:
        friend class Timer;
        using TimerIDType = size_t;
        TimerIDType RegisterTimer(const Timer& timer)
        {
            TimerIDType id = fUniqueIdProvider.Acquire();

            auto it = fMapTimerIdToTimer.emplace(id, &timer);

            if (it.second == false)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::DuplicateItem, "Timer id already registered");

            return id;
        }
        void UnRegisterTimer(TimerIDType timerID)
        {
            auto it = fMapTimerIdToTimer.find(timerID);
            if (it == fMapTimerIdToTimer.end())
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Timer id not found in data structure");

            fMapTimerIdToTimer.erase(it);
            fUniqueIdProvider.Release(timerID);
        }

        static void Timerproc(
            HWND hwnd,        // handle to window for timer messages 
            UINT message,     // WM_TIMER message 
            UINT idTimer,     // timer identifier 
            DWORD dwTime);  // current system time
        

    private:
        using UniqueIdProviderType = LLUtils::UniqueIdProvider<TimerIDType, std::set<TimerIDType>>;
        UniqueIdProviderType fUniqueIdProvider = UniqueIdProviderType(1);
        
        std::map< TimerIDType, const Timer*> fMapTimerIdToTimer;
    };

    class Timer
    {
    public:

        using Callback = std::function<void()>;

        ~Timer()
        {
            Unregister();
        }

		void SetTargetWindow(HWND hwnd)
		{
			if (hwnd == nullptr)
				LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "null window as timer target is illegal");
		
			Unregister();
			fWindowHandle = hwnd;
			SetInterval(fInterval);
		}

        uint32_t GetInterval() const
        {
            return  fInterval;
        }

        void SetInterval(uint32_t interval)
        {
            if (fInterval != interval)
            {
				Register();

                fInterval = interval;
                if (fInterval == 0)
                    ::KillTimer(fWindowHandle, fTimerID);
                else
                    ::SetTimer(fWindowHandle, fTimerID, fInterval, reinterpret_cast<TIMERPROC>(TimerManager::Timerproc));
            }
        }
        void SetCallback(Callback callback)
        {
            fCallback = callback;
        }

    private:
        friend class TimerManager;

        void Unregister()
        {
            if (fTimerID != 0)
            {
                SetInterval(0);
                TimerManager::GetSingleton().UnRegisterTimer(fTimerID);
                fTimerID = 0;
            }
        }


        void Register()
        {
			if (fTimerID == 0)
			{
				if (fWindowHandle == nullptr)
					LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Timer is not bound to a window, call 'Win32::Timer::SetTargetWindow' first");
				Unregister();
				fTimerID = TimerManager::GetSingleton().RegisterTimer(*this);
			}
        }

    private:
        Callback fCallback;
        TimerManager::TimerIDType fTimerID = 0;
        uint32_t fInterval = 0;
        HWND fWindowHandle = nullptr;

    };


    inline void TimerManager::Timerproc(
        HWND hwnd,        // handle to window for timer messages 
        UINT message,     // WM_TIMER message 
        UINT idTimer,     // timer identifier 
        DWORD dwTime)     // current system time
    {
        TimerManager& _this = TimerManager::GetSingleton();

        auto it = _this.fMapTimerIdToTimer.find(idTimer);
        if (it == _this.fMapTimerIdToTimer.end())
            LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Timer id not found in data structure");

        const Timer* timer = it->second;
        timer->fCallback();
    }
}

