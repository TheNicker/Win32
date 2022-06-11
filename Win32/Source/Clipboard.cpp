#include <Win32/Clipboard.h>
#include <LLUtils/StringUtility.h>
#include <LLUtils/Exception.h>
#include <string_view>
#include <Windows.h>

namespace Win32
{
    void Clipboard::RegisterFormat(ClipboardFormatType format)
    {
        fListFormats.push_back(format);
    }

    ClipboardFormatType Clipboard::RegisterFormat(std::wstring format)
    {
        auto formatID = RegisterClipboardFormat(format.c_str());
        RegisterFormat(formatID);
        return formatID;
    }

    void Clipboard::SetClipboardData(ClipboardFormatType format, const LLUtils::Buffer& data)
    {
        SetClipboardData(format, data.data(), data.size());
    }

    void Clipboard::SetClipboardData(ClipboardFormatType format, const std::byte* data, size_t size)
    {
        HANDLE handle = GlobalAlloc(GHND, size);
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
        {
            // lock memory and get pointer to it
            void* buffer = GlobalLock(handle);

            if (buffer == nullptr)
                LL_EXCEPTION_SYSTEM_ERROR("Can't lock global memory.");

            memcpy(buffer, data, size);

            if (GlobalUnlock(handle) > 0 && GetLastError() != NO_ERROR)
                LL_EXCEPTION_SYSTEM_ERROR("Can't unlock global memory.");

            if (SetClipboardData(format, handle) == false)
            {
                if (GlobalFree(handle) != nullptr)
                    LL_EXCEPTION_SYSTEM_ERROR("Can't free global handle");
            }
        }
    }

    void Clipboard::SetClipboardText(const wchar_t* text)
    {
        std::wstring_view strView(text);
        SetClipboardData(CF_UNICODETEXT
            , reinterpret_cast<const std::byte*>(text)
            , (strView.length() + 1) * sizeof(wchar_t));


        std::string ansi = LLUtils::StringUtility::ConvertString<std::string>(text);

        SetClipboardData(CF_TEXT
            , reinterpret_cast<const std::byte*>(ansi.data())
            , (ansi.length() + 1) * sizeof(char));


    }

    void Clipboard::SetClipboardText(const char* text)
    {
        std::string_view strView(text);
        SetClipboardData(CF_TEXT
            , reinterpret_cast<const std::byte*>(text)
            , (strView.length() + 1) * sizeof(char));


        std::wstring unicode = LLUtils::StringUtility::ConvertString<std::wstring>(text);

        SetClipboardData(CF_UNICODETEXT
            , reinterpret_cast<const std::byte*>(unicode.data())
            , (unicode.length() + 1) * sizeof(wchar_t));

    }

    bool Clipboard::SetClipboardData(ClipboardFormatType format, HANDLE data)
    {

        if (::OpenClipboard(nullptr))
        {
            if (::SetClipboardData(format, data) == nullptr)
                LL_EXCEPTION_SYSTEM_ERROR("Unable to set clipboard data.");


            if (CloseClipboard() == FALSE)
                LL_EXCEPTION_SYSTEM_ERROR("can not close clipboard.");

            return true;

        }
        else
        {
            LL_EXCEPTION_SYSTEM_ERROR("Can not open clipboard.");
        }

        return false;
    }

    ClipboardData Clipboard::GetClipboardData()
    {
        ClipboardData result;
        ClipboardFormatType selectedFormatID{};

        for (const auto formatID : fListFormats)
        {
            if (IsClipboardFormatAvailable(formatID))
            {
                selectedFormatID = formatID;
                break;
            }
        }

        if (selectedFormatID != 0)
        {
            if (OpenClipboard(nullptr))
            {
                HANDLE hClipboard = ::GetClipboardData(selectedFormatID);

                if (!hClipboard)
                    LL_EXCEPTION(LLUtils::Exception::ErrorCode::NotImplemented, "Unsupported clipboard bitmap format type");


                if (hClipboard != nullptr && hClipboard != INVALID_HANDLE_VALUE)
                {
                    size_t size = GlobalSize(hClipboard);
                    void* clipboardBuffer = GlobalLock(hClipboard);
                    if (clipboardBuffer != nullptr)
                    {
                        auto& buffer = std::get<LLUtils::Buffer>(result);
                        buffer.Allocate(size);
                        buffer.Write(reinterpret_cast<const std::byte*>(clipboardBuffer), 0, size);
                        std::get<ClipboardFormatType>(result) = selectedFormatID;
                    }
                    GlobalUnlock(clipboardBuffer);
                }
                CloseClipboard();
            }
        }
        return result;
    }
}