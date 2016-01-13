// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeRichEdit.h - header file for TaeRichEdit.cpp.
//---------------------------------------------------------------------------
#ifndef RichEditOleCallbackH
#define RichEditOleCallbackH

#include <vcl\comctrls.hpp>
#include <vcl\comobj.hpp>
#include <vcl\ole2.hpp>
#include <vcl\oledlg.hpp>
#include <richole.h>
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit;

class PACKAGE TIRichEditOleCallback : public IRichEditOleCallback
{
protected:
  TTaeRichEdit* FRichEdit;
  ULONG FRefCnt;
  bool FAcceptDrop;
  bool FAllowTextDrag;

public:
  // *** IUnknown methods ***
  STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj);
  STDMETHOD_(ULONG, AddRef) (THIS);
  STDMETHOD_(ULONG, Release) (THIS);

  // *** IRichEditOleCallback methods ***
  STDMETHOD(GetNewStorage) (THIS_ LPSTORAGE FAR * lplpstg);
  STDMETHOD(GetInPlaceContext) (THIS_ LPOLEINPLACEFRAME FAR * lplpFrame,
    LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
  STDMETHOD(ShowContainerUI) (THIS_ BOOL fShow);
  STDMETHOD(QueryInsertObject) (THIS_ LPCLSID lpclsid, LPSTORAGE lpstg,
    LONG cp);
  STDMETHOD(DeleteObject) (THIS_ LPOLEOBJECT lpoleobj);
  STDMETHOD(QueryAcceptData) (THIS_ LPDATAOBJECT lpdataobj,
    CLIPFORMAT FAR * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
  STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode);
  STDMETHOD(GetClipboardData) (THIS_ CHARRANGE FAR * lpchrg, DWORD reco,
    LPDATAOBJECT FAR * lplpdataobj);
  STDMETHOD(GetDragDropEffect) (THIS_ BOOL fDrag, DWORD grfKeyState,
    LPDWORD pdwEffect);
  STDMETHOD(GetContextMenu) (THIS_ WORD seltype, LPOLEOBJECT lpoleobj,
    CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu);

  TIRichEditOleCallback(TTaeRichEdit* richedit);
  TIRichEditOleCallback(HWND window);
  ~TIRichEditOleCallback(void);

  __property bool AcceptDrop = { read = FAcceptDrop, write = FAcceptDrop };
  __property bool AllowTextDrag = { read = FAllowTextDrag,
    write = FAllowTextDrag };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
