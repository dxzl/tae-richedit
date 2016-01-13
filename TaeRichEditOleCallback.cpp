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
// class TIRichEditOle - adds basic OLE functionality to TRichEdit.  Originally
// based on code found at http://www.dystopia.fi/~janij/techinfo/richedit.htm
// and presumably written by Jani Järvinen.  Thanks, Jani.
//
// Additional code developed through examination of Borland's VCL library,
// Microsoft's MFC source code, and sample code available on Microsoft's
// developer web site.
//
// Note that this code is very experimental -- the author admits to only a
// vague familiarity with OLE and accepts no criticism of the code.  Many,
// if not most, of the interfaces return failure codes arbitrarily chosen by
// the author with no particular reason to think that the values are correct.
// In particular, no great effort has been expended looking for "memory
// leaks," and these are considered quite probable.  You have been warned.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <vcl\olectnrs.hpp>
#include "TaeRichEditOleCallback.h"
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TIRichEditOleCallback::TIRichEditOleCallback(TTaeRichEdit* richedit) : FRefCnt(0)
{
  FRichEdit = richedit;
  FAcceptDrop = false;
  FAllowTextDrag = false;
}
//---------------------------------------------------------------------------
TIRichEditOleCallback::~TIRichEditOleCallback(void)
{
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::QueryInterface(THIS_ REFIID riid,
  LPVOID FAR * lplpObj)
{
  // return S_OK if the interface is supported, E_NOINTERFACE if not.
  if (riid == IID_IUnknown)
    *lplpObj = (LPUNKNOWN) this;
  else if (riid == IID_IRichEditOleCallback)
    *lplpObj = (LPRICHEDITOLECALLBACK) this;
  else {
    *lplpObj = 0;
    return E_NOINTERFACE;
    }

  ((LPUNKNOWN) *lplpObj)->AddRef();
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) TIRichEditOleCallback::AddRef(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  return ++FRefCnt;
}
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) TIRichEditOleCallback::Release(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  --FRefCnt;
  return FRefCnt;
}
//---------------------------------------------------------------------------
// *** IRichEditOleCallback methods ***
STDMETHODIMP TIRichEditOleCallback::GetNewStorage(THIS_ LPSTORAGE FAR * lplpstg)
{
  if (!lplpstg) return E_INVALIDARG;
  *lplpstg = NULL;

  //
  // We need to create a new storage for an object to occupy.  We're going
  // to do this the easy way and just create a storage on an HGLOBAL and let
  // OLE do the management.  When it comes to saving things we'll just let
  // the RichEdit control do the work.  Keep in mind this is not efficient.
  //
  LPLOCKBYTES pLockBytes;
  HRESULT hr = ::CreateILockBytesOnHGlobal(NULL, TRUE, &pLockBytes);
  if (FAILED(hr)) return hr;

  hr = ::StgCreateDocfileOnILockBytes(pLockBytes,
    STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, lplpstg);
  pLockBytes->Release();
  return hr;
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::GetInPlaceContext(THIS_ LPOLEINPLACEFRAME FAR * lplpFrame,
  LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
  return E_NOTIMPL;  // what to return???
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::ShowContainerUI(THIS_ BOOL fShow)
{
  return E_NOTIMPL;  // what to return?
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::QueryInsertObject(THIS_ LPCLSID lpclsid,
  LPSTORAGE lpstg, LONG cp)
{
  // let richedit insert any and all objects
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::DeleteObject(THIS_ LPOLEOBJECT lpoleobj)
{
  // per doc, no return value, i.e., this is simply a notification
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::QueryAcceptData(THIS_ LPDATAOBJECT lpdataobj,
  CLIPFORMAT FAR * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
  // allow insertion on dropped file?
  if (reco == RECO_DROP && !FAcceptDrop) return E_NOTIMPL;
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::ContextSensitiveHelp(THIS_ BOOL fEnterMode)
{
  return E_NOTIMPL;  // what to return???
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::GetClipboardData(THIS_ CHARRANGE FAR * lpchrg,
  DWORD reco, LPDATAOBJECT FAR * lplpdataobj)
{
  return E_NOTIMPL;  // what to return???
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::GetDragDropEffect(THIS_ BOOL fDrag,
  DWORD grfKeyState, LPDWORD pdwEffect)
{
  // per doc, no return value, i.e., for notification only
  // note: testing shows that dragging text within the control is
  // enabled simply by returning DROPEFFECT_MOVE in pdwEffect....
  if (FAllowTextDrag) *pdwEffect = DROPEFFECT_MOVE;
  else *pdwEffect = DROPEFFECT_NONE;
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP TIRichEditOleCallback::GetContextMenu(THIS_ WORD seltype,
  LPOLEOBJECT lpoleobj, CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu)
{
  return E_NOTIMPL;  // what to return???
}
//---------------------------------------------------------------------------
