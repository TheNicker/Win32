#include <Win32/MonitorInfo.h>
#include <algorithm>
#include <string>
#include <ShellScalingApi.h>
#include <LLUtils/StringDefs.h>
#include <LLUtils/Templates.h>

namespace Win32
{
    //---------------------------------------------------------------------
    MonitorInfo::MonitorInfo()
    {
        Refresh();
    }
    //---------------------------------------------------------------------
    void MonitorInfo::Refresh()
    {
        mDisplayDevices.clear();
        mHMonitorToDesc.clear();
        DISPLAY_DEVICE disp;
        disp.cb = sizeof(disp);
        DWORD devNum = 0;
        while (EnumDisplayDevices(nullptr, devNum++, &disp, 0) == TRUE)
        {
            if ((disp.StateFlags & DISPLAY_DEVICE_ACTIVE) == DISPLAY_DEVICE_ACTIVE) // only connected
            {
                mDisplayDevices.push_back(MonitorDesc());
                MonitorDesc& desc = mDisplayDevices.back();
                desc.DisplayInfo = disp;
                desc.DisplaySettings.dmSize = sizeof(desc.DisplaySettings.dmSize);
                EnumDisplaySettings(disp.DeviceName, ENUM_CURRENT_SETTINGS, &desc.DisplaySettings);
            }
        }
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
    }
    //---------------------------------------------------------------------
    const MonitorDesc& MonitorInfo::getMonitorInfo(size_t monitorIndex, bool allowRefresh /*= false*/)
    {
        if (monitorIndex >= mDisplayDevices.size() && allowRefresh)
            Refresh();

		return monitorIndex < mDisplayDevices.size() ? mDisplayDevices[monitorIndex] : mEmptyMonitorDesc;
    }
    //---------------------------------------------------------------------
    const MonitorDesc&  MonitorInfo::getMonitorInfo(HMONITOR hMonitor, bool allowRefresh )
    {
        if (allowRefresh)
            Refresh();

        auto it = mHMonitorToDesc.find(hMonitor);
        return it == mHMonitorToDesc.end() ? mEmptyMonitorDesc : it->second;

    }
    //---------------------------------------------------------------------
    BOOL CALLBACK MonitorInfo::MonitorEnumProc(_In_ HMONITOR hMonitor, _In_ [[maybe_unused]]  HDC hdcMonitor
        , _In_ [[maybe_unused]] LPRECT lprcMonitor, _In_ LPARAM dwData)
    {
        MonitorInfo* _this = reinterpret_cast<MonitorInfo*>(dwData);
        MONITORINFOEX monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);
        GetMonitorInfo(hMonitor, &monitorInfo);
        for (MonitorDesc& desc : _this->mDisplayDevices)
        {
            if (LLUtils::native_string_type(monitorInfo.szDevice) == LLUtils::native_string_type(desc.DisplayInfo.DeviceName))
            {
                desc.monitorInfo = monitorInfo;
                desc.handle = hMonitor;

                UINT dpix;
                UINT dpiy;
                GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
                desc.DPIx = static_cast<uint16_t>(dpix);
                desc.DPIy = static_cast<uint16_t>(dpiy);

                _this->mHMonitorToDesc.insert(std::make_pair(hMonitor, desc));
                break;
            }
        }

        return true;
    }
    //---------------------------------------------------------------------
    size_t MonitorInfo::getMonitorsCount() const
    {
        return mDisplayDevices.size();
    }

    RECT MonitorInfo::getBoundingMonitorArea()
    {
        using namespace std;
        RECT rect {};
        size_t count = getMonitorsCount();
        for (size_t i = 0; i < count; i++)
        {
            const MONITORINFOEX&  info = getMonitorInfo(i).monitorInfo;
            const RECT& monRect = info.rcMonitor;
            rect.left = min(rect.left, monRect.left);
            rect.top = min(rect.top, monRect.top);
            rect.right = max(rect.right, monRect.right);
            rect.bottom = max(rect.bottom, monRect.bottom);
        }
        return rect;
    }
}
