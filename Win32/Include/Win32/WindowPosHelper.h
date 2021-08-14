#pragma once

#include <cstdint>
#include <wtypes.h>
#include <vector>
#include <LLUtils/Exception.h>

namespace Win32
{
    enum class WindowPosOp
    {
        None
        , Resize
        , Move
        , Zorder
        , UpdateFrame
        , Placement
        , AlwaysOnTop
    };


    class WindowPosHelper
    {
    public:

        static const UINT BaseFlags =
            0
            | SWP_NOMOVE
            | SWP_NOSIZE
            | SWP_NOZORDER
            | SWP_NOOWNERZORDER
            | SWP_NOACTIVATE
            | SWP_NOCOPYBITS
            | SWP_NOREPOSITION
            | SWP_NOREDRAW
            | SWP_NOSENDCHANGING
            | SWP_DEFERERASE
            ;

        static UINT GetFlagsForWindowSetPosOp(WindowPosOp op)
        {
            switch (op)
            {
            case WindowPosOp::None:
                return 0;
            case WindowPosOp::Placement:
                return BaseFlags & static_cast<uint32_t>((~(SWP_NOMOVE | SWP_NOSIZE)));
            case WindowPosOp::Move:
                return BaseFlags & static_cast<uint32_t>(~SWP_NOMOVE);
            case WindowPosOp::UpdateFrame:
                return BaseFlags | static_cast<uint32_t>(SWP_FRAMECHANGED);
            case WindowPosOp::Resize:
                return BaseFlags & static_cast<uint32_t>(~SWP_NOSIZE);
            case WindowPosOp::Zorder:
                return static_cast<uint32_t>(BaseFlags & static_cast<uint32_t>(~SWP_NOZORDER));
            case WindowPosOp::AlwaysOnTop:
                return BaseFlags & static_cast<uint32_t>(~SWP_NOZORDER);

            default:
                LL_EXCEPTION_UNEXPECTED_VALUE;
            }
        }

        static UINT ComposeFlags(std::vector<WindowPosOp> flags)
        {
            UINT result = BaseFlags;

            for (WindowPosOp op : flags)
            {
                switch (op)
                {
                case WindowPosOp::None:
                    break;
                case WindowPosOp::Placement:
                    result &= static_cast<UINT>((~(SWP_NOMOVE | SWP_NOSIZE)));
                    break;
                case WindowPosOp::Move:
                    result &= static_cast<UINT>(~SWP_NOMOVE);
                    break;
                case WindowPosOp::UpdateFrame:
                    result |= static_cast<UINT>(SWP_FRAMECHANGED);
                    break;
                case WindowPosOp::Resize:
                    result &= static_cast<UINT>(~SWP_NOSIZE);
                    break;
                case WindowPosOp::AlwaysOnTop:
                case WindowPosOp::Zorder:
                    result &= static_cast<UINT>(~SWP_NOZORDER);
                    break;

                default:
                    LL_EXCEPTION_UNEXPECTED_VALUE;
                }
            }
            return result;
        }

        static void SetAlwaysOnTop(HWND handle, bool ontop)
        {
            SetWindowPos(handle, ontop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, GetFlagsForWindowSetPosOp(WindowPosOp::AlwaysOnTop));
        }

        static void SetPosition(HWND handle, int32_t x, int32_t y)
        {
            SetWindowPos(handle, nullptr, x, y, 0, 0, GetFlagsForWindowSetPosOp(WindowPosOp::Move));
        }

        static void SetSize(HWND handle, uint32_t width, uint32_t height)
        {
            SetWindowPos(handle, nullptr, 0, 0, static_cast<int>(width), static_cast<int>(height), GetFlagsForWindowSetPosOp(WindowPosOp::Resize));
        }

        static void SetPlacement(HWND handle, int32_t x, int32_t y, uint32_t width, uint32_t height)
        {
            SetWindowPos(handle, nullptr, x, y, static_cast<int>(width), static_cast<int>(height), GetFlagsForWindowSetPosOp(WindowPosOp::Placement));
        }

        static void UpdateFrame(HWND handle)
        {
            SetWindowPos(handle, nullptr, 0, 0, 0, 0, GetFlagsForWindowSetPosOp(WindowPosOp::UpdateFrame));
        }

    };
}