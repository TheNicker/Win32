#pragma once
#include <string>
#include <vector>
#include <memory>
#include <Windows.h>
#include <ShlObj.h>
#include <LLUtils/Warnings.h>

namespace Win32
{
    enum class FileDialogType
    {
          Unspecified
        , OpenFile
        , SaveFile
    };

    using file_dialog_string_type = std::wstring;
   
    using ListFileDialogCOMDLGFilters = std::vector<COMDLG_FILTERSPEC>;
    
    using ListFileDialogFileNames = std::vector<file_dialog_string_type>;
    
    enum class FileDialogResult
    {
          Success
        , UserCanceled
        , UnknownError
    };


    /// <summary>
    /// helper class to build file dialog filters
    /// </summary>
    class FileDialogFilterBuilder
    {
    public:
        FileDialogFilterBuilder() = default;
        using ListExtensions = std::vector<file_dialog_string_type>;

        struct FileDialogFilter
        {
            file_dialog_string_type description;
            ListExtensions extensions;
        };

        using ListFileDialogFilters = std::vector<FileDialogFilter>;

        struct FileDialogFilterData
        {
            std::wstring name;
            std::wstring spec;
        };


    private:
        std::vector<FileDialogFilterData> fFiltersData;
        std::vector<COMDLG_FILTERSPEC> fDialogFilters;
    
    public:
        FileDialogFilterBuilder(const ListFileDialogFilters& filters)
        {
            fFiltersData.resize(filters.size());
            fDialogFilters.resize(filters.size());
            size_t i = 0;
            for (auto& filter : filters)
            {

                fFiltersData[i].name = filter.description;
                fDialogFilters[i].pszName = fFiltersData[i].name.c_str();

                std::wstringstream extBuffer;
                for (auto& extension : filter.extensions)
                    extBuffer << extension << L';';

                if (extBuffer.rdbuf()->in_avail() > 0)
                {
                    extBuffer.seekp(-1, std::ios_base::end);
                    extBuffer << L'\0';
                }
                fFiltersData[i].spec = extBuffer.str();
                fDialogFilters[i].pszSpec = fFiltersData[i].spec.c_str();
                i++;
            }
        }
        const std::vector<COMDLG_FILTERSPEC>& GetFilters()
        {
            return fDialogFilters;
        }
    };

    class FileDialog
    {

    public:

        static FileDialogResult Show(FileDialogType dialogType
            , const ListFileDialogCOMDLGFilters& filters
            , const file_dialog_string_type& title
            , HWND ownerWindow
            , const file_dialog_string_type& defaultExtension
            , UINT filterIndex
            , file_dialog_string_type defaultFileName
            , file_dialog_string_type& out_Filename
        )
        {
            ListFileDialogFileNames fileNames;
            FileDialogResult result = Show(dialogType, filters, title, ownerWindow, defaultExtension, filterIndex, defaultFileName, fileNames);

            if (result == FileDialogResult::Success && fileNames.empty() == false)

                out_Filename = fileNames.back();
            
            return result;
        }



        static FileDialogResult Show(FileDialogType dialogType
            , const ListFileDialogCOMDLGFilters& filters
            , const file_dialog_string_type& title
            , HWND ownerWindow
            , const file_dialog_string_type& defaultExtension
            , UINT filterIndex
            , file_dialog_string_type defaultFileName
            , ListFileDialogFileNames& out_Filenames
            )
        {

            FileDialogResult result = FileDialogResult::UnknownError;

            // CoCreate the File Open Dialog object.
            IFileDialog* pfd = nullptr;

            const CLSID& dialogClassID = dialogType == FileDialogType::OpenFile ? CLSID_FileOpenDialog :
                dialogType == FileDialogType::SaveFile ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        
            if (SUCCEEDED(hr))
            {
LLUTILS_DISABLE_WARNING_PUSH
LLUTILS_DISABLE_WARNING_LANGUAGE_EXTENSION
                hr = CoCreateInstance(dialogClassID, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
LLUTILS_DISABLE_WARNING_POP
                if (SUCCEEDED(hr))
                {
                    // Create an event handling object, and hook it up to the dialog.
                    //IFileDialogEvents* pfde = nullptr;
                   // hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
                  //  if (SUCCEEDED(hr))
                    {
                        // Hook up the event handler.
                       // DWORD dwCookie;
                        // hr = pfd->Advise(pfde, &dwCookie);
                        if (SUCCEEDED(hr))
                        {
                            // Set the options on the dialog.
                            DWORD dwFlags;

                            // Before setting, always get the options first in order not to override existing options.
                            hr = pfd->GetOptions(&dwFlags);
                            if (SUCCEEDED(hr))
                            {
                                // In this case, get shell items only for file system items.
                                hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);

                                pfd->SetTitle(title.c_str());
                                pfd->SetFileName(defaultFileName.c_str());
    
                                if (SUCCEEDED(hr))
                                {
                                    // Set the file types to display only. Notice that, this is a 1-based array.
                                    hr = pfd->SetFileTypes(static_cast<UINT>(filters.size()), filters.data());
                                    if (SUCCEEDED(hr))
                                    {
                                        // Set the selected file type index to Word Docs for this example.
                                        hr = pfd->SetFileTypeIndex(filterIndex);
                                        if (SUCCEEDED(hr))
                                        {
                                            // Set the default extension to be ".doc" file.
                                            hr = pfd->SetDefaultExtension(defaultExtension.c_str());
                                            if (SUCCEEDED(hr))
                                            {
                                                // Show the dialog
                                                hr = pfd->Show(ownerWindow);
                                                if (SUCCEEDED(hr))
                                                {
                                                    // Obtain the result, once the user clicks the 'Open' button.
                                                    // The result is an IShellItem object.
                                                    IShellItem* psiResult;
                                                    hr = pfd->GetResult(&psiResult);
                                                    if (SUCCEEDED(hr))
                                                    {
                                                        // We are just going to print out the name of the file for sample sake.
                                                        PWSTR pszFilePath = nullptr;
                                                        hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                                        if (SUCCEEDED(hr))
                                                        {
                                                            out_Filenames.push_back(pszFilePath);
                                                            CoTaskMemFree(pszFilePath);
                                                        }
                                                        psiResult->Release();
                                                        result = FileDialogResult::Success;
                                                    }
                                                }
                                                else
                                                {
                                                    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                                                        result = FileDialogResult::UserCanceled;

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            // Unhook the event handler.
            //                pfd->Unadvise(dwCookie);
                        }
                        // pfde->Release();
                    }
                    pfd->Release();
                }
            }
            CoUninitialize();
            //return hr;
            return result;
        }
    };
}