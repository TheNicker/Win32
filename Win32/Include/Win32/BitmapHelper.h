#pragma once
#include <fstream>
#include <LLUtils/PlatformUtility.h>
#include <LLUtils\Exception.h>
namespace Win32
{
    struct BitmapBuffer
    {
        const std::byte* buffer;
        uint8_t bitsPerPixel;
        uint32_t width;
        uint32_t height;
        uint32_t rowPitch;
    };

    class Bitmap;
    using BitmapSharedPtr = std::shared_ptr<Bitmap>;

    class Bitmap
    {
        struct {
            BITMAPINFOHEADER bmiHeader;
            RGBQUAD bmiColors[256];
        } fBitmapInfo = {};

    public:

        Bitmap(const BitmapBuffer& bitmapBuffer)
        {
            fBitmap = FromMemory(bitmapBuffer);

        }

        BitmapSharedPtr resize(size_t width, size_t height)
        {
            HDC dcSrc = CreateCompatibleDC(nullptr);
            SelectObject(dcSrc, fBitmap);

            std::unique_ptr<std::uint8_t[]> emptyBuffer = std::make_unique<std::uint8_t[]>( width * height * 4);
            memset(emptyBuffer.get(), 0, width * height * 4);

            BitmapBuffer buf;
            buf.bitsPerPixel = 32;
            buf.buffer = reinterpret_cast<std::byte*>(emptyBuffer.get());
            buf.width = static_cast<uint32_t>(width);
            buf.height = static_cast<uint32_t>(height);
            buf.rowPitch = static_cast < uint32_t>(4 * width);

            BitmapSharedPtr resized = std::make_shared<Bitmap>(buf);
            HDC dst = CreateCompatibleDC(nullptr);
            SelectObject(dst, resized->fBitmap);
            SetStretchBltMode(dst, STRETCH_HALFTONE);
            //SetBkMode(dst, TRANSPARENT);
            size_t finalWidth = std::min<size_t>(width, static_cast<size_t>(GetBitmapHeader().biWidth));
            size_t finalHeight = std::min<size_t>(height, static_cast<size_t>(GetBitmapHeader().biHeight));

            //blit image to the middle of the new image.
            size_t posX = (width - finalWidth) / 2;
            size_t posY = (height - finalHeight) / 2;

            StretchBlt(dst, posX, posY, static_cast<int>(finalWidth), static_cast<int>(finalHeight), dcSrc, 0, 0, fBitmapInfo.bmiHeader.biWidth, fBitmapInfo.bmiHeader.biHeight, SRCCOPY);

            DeleteDC(dcSrc);
            DeleteDC(dst);

            return resized;
        }




        Bitmap(const std::wstring& fileName)
        {
            fBitmap = FromFileAnyFormat(fileName);
        }


        ~Bitmap()
        {
            if (fBitmap != nullptr)
                DeleteObject(fBitmap);
        }



        const BITMAPINFOHEADER& GetBitmapHeader()
        {
            if (fBitmapInfo.bmiHeader.biSize == 0)
            {
                fBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

                HDC hDC = GetDC(nullptr);

                GetDIBits(hDC, fBitmap, 0, 1, nullptr, (BITMAPINFO*)&fBitmapInfo,
                    DIB_RGB_COLORS);
                ReleaseDC(nullptr, hDC);
            }
            return fBitmapInfo.bmiHeader;
        }

        static HBITMAP FromMemory(const BitmapBuffer& bitmapBuffer)
        {
            const int height = bitmapBuffer.height;
            const int width = bitmapBuffer.width;
            const int bpp = bitmapBuffer.bitsPerPixel;
            const int rowPitch = bitmapBuffer.rowPitch;

            BITMAPINFO bi{};
            
            bi.bmiHeader.biBitCount = bpp;
            bi.bmiHeader.biClrImportant = 0;
            bi.bmiHeader.biClrUsed = 0;
            bi.bmiHeader.biCompression = 0;
            bi.bmiHeader.biHeight = height;
            bi.bmiHeader.biWidth = width;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biSize = 40;
            bi.bmiHeader.biSizeImage = rowPitch * height;

            bi.bmiHeader.biXPelsPerMeter = 11806;
            bi.bmiHeader.biYPelsPerMeter = 11806;


            bi.bmiColors[0].rgbBlue = 0;
            bi.bmiColors[0].rgbGreen = 0;
            bi.bmiColors[0].rgbRed = 0;
            bi.bmiColors[0].rgbReserved = 0;

            const std::byte* pPixels = (bitmapBuffer.buffer);

            char* ppvBits;

            HBITMAP hBitmap = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, (void**)&ppvBits, nullptr, 0);
            if (SetDIBits(nullptr, hBitmap, 0, height, pPixels, &bi, DIB_RGB_COLORS) == 0)
                LL_EXCEPTION_SYSTEM_ERROR("can not set bitmap pixels");
            return hBitmap;
        }


        static HBITMAP FromFileAnyFormat(const std::wstring& filePath)
        {
            std::ifstream is;
            is.open(filePath, std::ios::binary);
            is.seekg(0, std::ios::end);
            size_t length = is.tellg();
            is.seekg(0, std::ios::beg);
            char* pBuffer = new char[length];
            is.read(pBuffer, length);
            is.close();

            auto nativeStringFilePath = LLUtils::StringUtility::ToNativeString(filePath);
        	
            HBITMAP bmp = (HBITMAP)LoadImage(GetModuleHandle(nullptr), nativeStringFilePath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            return bmp;
        }

        HBITMAP GetHBitmap() const { return fBitmap; }

    private:
        HBITMAP fBitmap = nullptr;
    };
}