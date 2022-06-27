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

    enum class ClipboardResult
    {
         Success
        ,AccessDenied
        ,UnknownError
    };

    class Clipboard
    {
    public:
        void RegisterFormat(ClipboardFormatType format);
        ClipboardFormatType RegisterFormat(std::wstring format);
        ClipboardResult SetClipboardData(ClipboardFormatType format, const LLUtils::Buffer& data);
        ClipboardResult SetClipboardData(ClipboardFormatType format, const std::byte* data, size_t size);
        ClipboardResult SetClipboardText(const wchar_t* text);
        ClipboardResult SetClipboardText(const char* text);
        ClipboardResult SetClipboardData(ClipboardFormatType format, HANDLE data);
        ClipboardData GetClipboardData();

    private:
        ClipboardResult GetClipboardError() const;
        using ListFormat = std::vector<ClipboardFormatType>;
        ListFormat fListFormats;
    };
}
