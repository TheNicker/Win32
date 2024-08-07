#pragma once
#include <fstream>
#include <LLUtils/PlatformUtility.h>
#include <LLUtils/FileHelper.h>
#include <LLUtils/Exception.h>
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

        BitmapSharedPtr resize(int width, int height, uint8_t background = 0)
        {
            HDC dcSrc = CreateCompatibleDC(nullptr);
            SelectObject(dcSrc, fBitmap);

            const auto& header = GetBitmapHeader();
            const uint32_t rowPitch = LLUtils::Utility::Align<uint32_t>(header.biBitCount * width / CHAR_BIT, sizeof(DWORD));
            const uint32_t pixelsDataSize = rowPitch * width;

            std::unique_ptr<std::uint8_t[]> emptyBuffer = std::make_unique<std::uint8_t[]>(pixelsDataSize);
            memset(emptyBuffer.get(), background, pixelsDataSize);

            BitmapBuffer buf;
            buf.bitsPerPixel = static_cast<uint8_t>(header.biBitCount);
            buf.buffer = reinterpret_cast<std::byte*>(emptyBuffer.get());
            buf.width = static_cast<uint32_t>(width);
            buf.height = static_cast<uint32_t>(height);
            buf.rowPitch = rowPitch;

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

            StretchBlt(dst, static_cast<int>(posX), static_cast<int>(posY), static_cast<int>(finalWidth), static_cast<int>(finalHeight), dcSrc, 0, 0, fBitmapInfo.bmiHeader.biWidth, fBitmapInfo.bmiHeader.biHeight, SRCCOPY);

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

    void SaveToFile(const std::wstring& fileName)
    {
        BITMAPFILEHEADER fileHeader{};
        fileHeader.bfType = 0x4D42; // "BM"
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


        const auto& header = GetBitmapHeader();
        size_t pixelsSize = header.biWidth * header.biBitCount / CHAR_BIT * header.biHeight;
        LLUtils::Buffer pixelsData(pixelsSize);

        HDC hDC = GetDC(nullptr);

        BITMAPINFO info{};
        info.bmiHeader = header;

         int returnedLines = GetDIBits(hDC, fBitmap, 0, header.biHeight, pixelsData.data(), &info, DIB_RGB_COLORS);

         if (returnedLines != header.biHeight)
             LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Data size mismatch");
        
        ReleaseDC(nullptr, hDC);
        fileHeader.bfSize = static_cast<DWORD>(fileHeader.bfOffBits + pixelsSize);

        LLUtils::File::WriteAllBytes(fileName, sizeof(BITMAPFILEHEADER), reinterpret_cast<std::byte*>(&fileHeader));
        LLUtils::File::WriteAllBytes(fileName, sizeof(BITMAPINFOHEADER), reinterpret_cast<const std::byte*>(&header), true);
        LLUtils::File::WriteAllBytes(fileName, pixelsSize, reinterpret_cast<const std::byte*>(pixelsData.data()), true);

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
            
            bi.bmiHeader.biBitCount = static_cast<WORD>(bpp);
            bi.bmiHeader.biHeight = height;
            bi.bmiHeader.biWidth = width;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biSize = 40;
            bi.bmiHeader.biSizeImage = rowPitch * height;

            const std::byte* pPixels = (bitmapBuffer.buffer);

            char* ppvBits;

            HBITMAP hBitmap = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, (void**)&ppvBits, nullptr, 0);
            
            if (hBitmap == nullptr)
                LL_EXCEPTION_SYSTEM_ERROR("unable to allocate bitmap");

            if (SetDIBits(nullptr, hBitmap, 0, height, pPixels, &bi, DIB_RGB_COLORS) != height)
                LL_EXCEPTION_SYSTEM_ERROR("can not set bitmap pixels");
            
            return hBitmap;
        }


        static HBITMAP FromFileAnyFormat(const std::wstring& filePath)
        {
            std::ifstream is;
            is.open(filePath.c_str(), std::ios::binary);
            is.seekg(0, std::ios::end);
            size_t length = is.tellg();
            is.seekg(0, std::ios::beg);
            
            auto pBuffer = std::make_unique<char[]>(length);
            is.read(pBuffer.get(), length);
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