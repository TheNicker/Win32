#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <Win32/Win32Window.h>
#include <LLUtils/UniqueIDProvider.h>
#include <LLUtils/Rect.h>

namespace Win32
{
    class NotificationIconGroup final
    {
    public:  //definitions 
        static constexpr UINT WM_PRIVATE_NOTIFICATION_CALLBACK_MESSAGE_ID = WM_USER + 1;
        using IconID = uint8_t;
    public: 
        IconID AddIcon(LPWSTR IconName, const LLUtils::native_string_type&);
        ~NotificationIconGroup();
    private: //methods
        bool OnWindowMessage(const Win32::Event* evnt);
       
    public:
        enum class NotificationIconAction { None, Select, ContextMenu};
        struct NotificationIconEventArgs
        {
            NotificationIconAction action;
            int16_t mouseX;
            int16_t mouseY;
        };
        using NotificationIconEvent = LLUtils::Event<void(NotificationIconEventArgs)>;
        NotificationIconEvent OnNotificationIconEvent;
        LLUtils::Rect<uint16_t> GetIconRect(IconID iconid);

        
    private: // member fields
        void HandleMessage(const WinMessage& message);
        
        struct NotificationIconData
        {
            IconID id;
        };
        std::map<IconID, NotificationIconData> fMapIconData;

        Win32Window fWindow;
        LLUtils::UniqueIdProvider<IconID,std::set<IconID>> fIconIdProvider{ 1 };
    };
}
