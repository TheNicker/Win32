#pragma once
#include <string>
#include <vector>
#include <memory>
#include <LLUtils/Buffer.h>

typedef void* HANDLE;

namespace Win32
{
    using ClipboardFormatType = std::uint32_t;
    using ClipboardData = std::tuple<ClipboardFormatType, LLUtils::Buffer>;

    class Clipboard
    {
    public:
        void RegisterFormat(ClipboardFormatType format);
        ClipboardFormatType RegisterFormat(std::wstring format);
        void SetClipboardData(ClipboardFormatType format, const LLUtils::Buffer& data);
        void SetClipboardData(ClipboardFormatType format, const std::byte* data, size_t size);
        void SetClipboardText(const wchar_t* text);
        void SetClipboardText(const char* text);
        bool SetClipboardData(ClipboardFormatType format, HANDLE data);
        ClipboardData GetClipboardData();

    private:
        using ListFormat = std::vector<ClipboardFormatType>;
        ListFormat fListFormats;
    };
}
