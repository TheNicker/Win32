#pragma once
#include <Windows.h>
#include <map>
#include <vector>
#include <LLUtils/Singleton.h>
namespace Win32
{

    struct MonitorDesc
    {
        DISPLAY_DEVICE DisplayInfo;
        DEVMODE DisplaySettings;
        MONITORINFOEX monitorInfo;
        HMONITOR handle;
        uint16_t DPIx;
        uint16_t DPIy;
    };

    class MonitorInfo  : public LLUtils::Singleton<MonitorInfo>
    {
    public:
        MonitorInfo();
        void Refresh();

        const MonitorDesc& getMonitorInfo(size_t monitorIndex, bool allowRefresh = false);
        const MonitorDesc& getMonitorInfo(HMONITOR hMonitor, bool allowRefresh = false);
        const MonitorDesc& GetPrimaryMonitor(bool allowRefresh);
        size_t getMonitorsCount() const;
        RECT getBoundingMonitorArea();
    private:
        RECT getBoundingMonitorAreaInternal();
        using MapHMonitorToDesc = std::map<HMONITOR, MonitorDesc>;
        std::vector<MonitorDesc> mDisplayDevices;
        MapHMonitorToDesc mHMonitorToDesc;
        MapHMonitorToDesc::iterator fPrimaryMonitorIterator = mHMonitorToDesc.end();
        inline static MonitorDesc mEmptyMonitorDesc { };

        bool fBoundAreaOutOfDate = true;
        RECT fBoundArea;
        static BOOL CALLBACK MonitorEnumProc(
            _In_  HMONITOR hMonitor,
            _In_  HDC hdcMonitor,
            _In_  LPRECT lprcMonitor,
            _In_  LPARAM dwData
        );
    };
}