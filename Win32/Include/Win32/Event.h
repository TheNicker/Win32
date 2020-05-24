#pragma once
#include <functional>
#include <vector>
#include "Win32Common.h"
#include <LLUtils/StringDefs.h>

namespace Win32
{
        class Win32Window;
        class Event
        {
        public:
            Win32Window* window = nullptr;
            virtual ~Event() {}
        };

        class EventWinMessage : public Event
        {
        public:
            WinMessage message = {};
        };

    

        typedef std::function< bool(const Event*) > EventCallback;
        typedef std::vector <EventCallback> EventCallbackCollection;
        

        class EventDragDrop : public Event
        {
            
        };


        class EventDdragDropFile : public EventDragDrop
        {
        public:
            LLUtils::native_string_type fileName;
        };
}
