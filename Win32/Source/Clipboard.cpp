#include <Win32/Clipboard.h>
#include <LLUtils/StringUtility.h>
#include <LLUtils/StopWatch.h>
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

    ClipboardResult Clipboard::SetClipboardData(ClipboardFormatType format, const LLUtils::Buffer& data)
    {
        return SetClipboardData(format, data.data(), data.size());
    }

    ClipboardResult Clipboard::SetClipboardData(ClipboardFormatType format, const std::byte* data, size_t size)
    {
        HANDLE handle = GlobalAlloc(GHND, size);
        ClipboardResult result = ClipboardResult::UnknownError;
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
        {
            // lock memory and get pointer to it
            void* buffer = GlobalLock(handle);

            if (buffer == nullptr)
                LL_EXCEPTION_SYSTEM_ERROR("Can't lock global memory.");

            memcpy(buffer, data, size);

            if (GlobalUnlock(handle) > 0 && GetLastError() != NO_ERROR)
                LL_EXCEPTION_SYSTEM_ERROR("Can't unlock global memory.");

            result = SetClipboardData(format, handle);

            if (result != ClipboardResult::Success)
            {
                // If cannot set clipboard data, free handle.
                if (GlobalFree(handle) != nullptr)
                    LL_EXCEPTION_SYSTEM_ERROR("Can't free global handle");
            }
        }
        return result;
    }

    ClipboardResult Clipboard::SetClipboardText(const wchar_t* text)
    {
        std::wstring_view strView(text);
        auto result = SetClipboardData(CF_UNICODETEXT
            , reinterpret_cast<const std::byte*>(text)
            , (strView.length() + 1) * sizeof(wchar_t));


        if (result == ClipboardResult::Success)
        {

            std::string ansi = LLUtils::StringUtility::ConvertString<std::string>(text);

            result = SetClipboardData(CF_TEXT
                , reinterpret_cast<const std::byte*>(ansi.data())
                , (ansi.length() + 1) * sizeof(char));
        }


        return result;

    }

    ClipboardResult Clipboard::SetClipboardText(const char* text)
    {
        return SetClipboardText(LLUtils::StringUtility::ToWString(text).c_str());
    }

    ClipboardResult Clipboard::GetClipboardError() const
    {
        auto lastError = GetLastError();
        switch (lastError)
        {
        case ERROR_ACCESS_DENIED:
            return ClipboardResult::AccessDenied;
            break;
        default:
            return ClipboardResult::UnknownError;
        }
    }
    

    ClipboardResult Clipboard::SetClipboardData(ClipboardFormatType format, HANDLE data)
    {
        /*constexpr auto openClipboardSpinTime = 3000; 
        LLUtils::StopWatch stopwatch(true);*/

        ClipboardResult result = ClipboardResult::UnknownError;
        BOOL clipboardResult;
        //If can not open clipboard, try spinning for 'openClipboardSpinTime' 
        /*while ( (clipboardResult = ::OpenClipboard(nullptr)) == 0 || stopwatch.GetElapsedTimeInteger(LLUtils::StopWatch::TimeUnit::Milliseconds) >= openClipboardSpinTime)
        {
            std::this_thread::yield();
        }*/

        clipboardResult = ::OpenClipboard(nullptr);

        if (clipboardResult == 0)
        {
            result = GetClipboardError();
        }
        else
        {
            if (::SetClipboardData(format, data) != nullptr)
            {
                if (CloseClipboard() == FALSE)
                    LL_EXCEPTION_SYSTEM_ERROR("can not close clipboard.");

                result = ClipboardResult::Success;
            }
            else
            {
                result = GetClipboardError();
            }
        }

        return result;

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