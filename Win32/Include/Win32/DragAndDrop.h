#pragma once
#include <ShlObj.h>
#include "Event.h"


namespace Win32
{
    class DragAndDropTarget : public IDropTarget
    {
#pragma region IUnkown
    private:
        size_t m_cRef;
        Win32Window& fParentWindow;
    public:

        DragAndDropTarget(Win32Window& parentWindow);
        virtual ~DragAndDropTarget();
        void Detach();
        // *** IUnknown ***
        STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;

        STDMETHODIMP_(ULONG) AddRef() override;

        STDMETHODIMP_(ULONG) Release() override;

#pragma endregion
#pragma region  IDropTarget
        // *** IDropTarget ***
        STDMETHODIMP DragEnter(IDataObject* pdto,
            DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;

        STDMETHODIMP DragOver(DWORD grfKeyState,
            POINTL ptl, DWORD* pdwEffect) override;

        STDMETHODIMP DragLeave() override;

        STDMETHODIMP Drop(IDataObject* pdto, DWORD grfKeyState,
            POINTL ptl, DWORD* pdwEffect) override;
#pragma endregion


#pragma region ParseAndDispatch
    public:
        typedef std::function< bool(const EventDragDrop&) > DragDropCallback;
    private:
        typedef std::vector <DragDropCallback> DragDropCallbackCollection;
        DragDropCallbackCollection fDragDropListeners;

        void OpenFilesFromDataObject(IDataObject* pdto);
#pragma endregion 
    };
}// namespace Win32


