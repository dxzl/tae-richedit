// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 1999, 2000 Thin Air Enterprises and Robert Dunn.  All rights 
// reserved.  Free for non-commercial use.  Commercial use requires license 
// agreement.  See http://home.att.net/~robertdunn/Yacs.html for the most 
// current version.
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

#include "TaeRichEditOLE.h"
#include "TaeRichEdit.h"
#include "TaeDataObjBmp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// utiltiy functions
//---------------------------------------------------------------------------
//
static void OleUIMetafilePictIconFree(HGLOBAL hMetaPict)
{
  if (!hMetaPict) return;

  LPMETAFILEPICT pMF = (LPMETAFILEPICT) GlobalLock(hMetaPict);

  if (pMF && pMF->hMF) DeleteMetaFile(pMF->hMF);
  GlobalUnlock(hMetaPict);
  GlobalFree(hMetaPict);
  return;
}

static HRESULT OleStdSwitchDisplayAspect(LPOLEOBJECT  lpOleObj,
  LPDWORD lpdwCurAspect, DWORD dwNewAspect, HGLOBAL hMetaPict,
  BOOL fDeleteOldAspect, BOOL fSetupViewAdvise, LPADVISESINK lpAdviseSink,
  LPBOOL lpfMustUpdate)
{
  LPOLECACHE      lpOleCache = NULL;
  LPVIEWOBJECT    lpViewObj = NULL;
  LPENUMSTATDATA  lpEnumStatData = NULL;
  STATDATA        StatData;
  FORMATETC       FmtEtc;
  STGMEDIUM       Medium;
  DWORD           dwAdvf;
  DWORD           dwNewConnection;
  DWORD           dwOldAspect = *lpdwCurAspect;
  HRESULT         hrErr;

  if (lpfMustUpdate) *lpfMustUpdate = FALSE;

  lpOleObj->QueryInterface(IID_IOleCache, (LPVOID*)&lpOleCache);

  // if IOleCache* is NOT available, do nothing
  if (!lpOleCache) return E_INVALIDARG;

  // Setup new cache with the new aspect
  FmtEtc.cfFormat = 0;     // whatever is needed to draw
  FmtEtc.ptd      = NULL;
  FmtEtc.dwAspect = dwNewAspect;
  FmtEtc.lindex   = -1;
  FmtEtc.tymed    = TYMED_NULL;

  /* OLE2NOTE: if we are setting up Icon aspect with a custom icon
  **    then we do not want DataAdvise notifications to ever change
  **    the contents of the data cache. thus we set up a NODATA
  **    advise connection. otherwise we set up a standard DataAdvise
  **    connection.
  */
  if (dwNewAspect == DVASPECT_ICON && hMetaPict) dwAdvf = ADVF_NODATA;
  else dwAdvf = ADVF_PRIMEFIRST;

  hrErr = lpOleCache->Cache(&FmtEtc, dwAdvf, &dwNewConnection);
  if (!SUCCEEDED(hrErr)) {
    lpOleCache->Release();
    return hrErr;
    }

  *lpdwCurAspect = dwNewAspect;

  /* OLE2NOTE: if we are setting up Icon aspect with a custom icon,
  **    then stuff the icon into the cache. otherwise the cache must
  **    be forced to be updated. set the *lpfMustUpdate flag to tell
  **    caller to force the object to Run so that the cache will be
  **    updated.
  */
  if (dwNewAspect == DVASPECT_ICON && hMetaPict) {
    FmtEtc.cfFormat = CF_METAFILEPICT;
    FmtEtc.ptd      = NULL;
    FmtEtc.dwAspect = DVASPECT_ICON;
    FmtEtc.lindex   = -1;
    FmtEtc.tymed    = TYMED_MFPICT;

    Medium.tymed          = TYMED_MFPICT;
    Medium.hGlobal        = hMetaPict;
    Medium.pUnkForRelease = NULL;

    hrErr = lpOleCache->SetData(&FmtEtc, &Medium, FALSE /* fRelease */);

    if (!SUCCEEDED(hrErr)) {
      lpOleCache->Release();
      return hrErr;
      }
    }
  else if (lpfMustUpdate) *lpfMustUpdate = TRUE;

  if (fSetupViewAdvise && lpAdviseSink) {
    /* OLE2NOTE: re-establish the ViewAdvise connection */
    lpOleObj->QueryInterface(IID_IViewObject, (LPVOID*)&lpViewObj);

    if (lpViewObj) {
      lpViewObj->SetAdvise(dwNewAspect, 0, lpAdviseSink);
      lpViewObj->Release();
      }
    }

  /* OLE2NOTE: remove any existing caches that are set up for the old
  **    display aspect. It WOULD be possible to retain the caches set
  **    up for the old aspect, but this would increase the storage
  **    space required for the object and possibly require additional
  **    overhead to maintain the unused cachaes. For these reasons the
  **    strategy to delete the previous caches is prefered. If it is a
  **    requirement to quickly switch between Icon and Content
  **    display, then it would be better to keep both aspect caches.
  */

  if (fDeleteOldAspect) {
    hrErr = lpOleCache->EnumCache(&lpEnumStatData);

    while (hrErr == NOERROR) {
      hrErr = lpEnumStatData->Next(1, &StatData, NULL);
      if (hrErr != NOERROR) break;    // DONE! no more caches.

      if (StatData.formatetc.dwAspect == dwOldAspect)
        // Remove previous cache with old aspect
        lpOleCache->Uncache(StatData. dwConnection);
      }

    if (lpEnumStatData)
      if (lpEnumStatData->Release())
        throw EOleError("OleStdSwitchDisplayAspect: Cache enumerator NOT released");
    }

  if (lpOleCache) lpOleCache->Release();

  return NOERROR;
}
//---------------------------------------------------------------------------
// TIRichEditOle
//---------------------------------------------------------------------------
TIRichEditOle::TIRichEditOle(TTaeRichEdit* richedit) : FIRichEditOle(0)
{
  // do we need to call OleInitialize(NULL)? seems to work without it...
  ::OleInitialize(0);

  FIRichEditCallback = new TIRichEditOleCallback(richedit);

  FRichEdit = richedit;
  ::SendMessage(richedit->Handle, EM_SETOLECALLBACK, 0, (LPARAM) (LPRICHEDITOLECALLBACK) FIRichEditCallback);
  if (!::SendMessage(richedit->Handle, EM_GETOLEINTERFACE, 0, (LPARAM) &FIRichEditOle))
    FIRichEditOle = 0;

  // register clipboard formats used in ole (so we'll have the clipboard
  // format IDs for ole clipboard operations (like OleUIPasteSpecial)
  CFObjectDescriptor = RegisterClipboardFormat("Object Descriptor");
  CFEmbeddedObject = RegisterClipboardFormat("Embedded Object");
  CFLinkSource = RegisterClipboardFormat("Link Source");
  CFRtf = RegisterClipboardFormat(CF_RTF);
  CFRtfNoObjs = RegisterClipboardFormat(CF_RTFNOOBJS);
  CFReTextObjs = RegisterClipboardFormat(CF_RETEXTOBJ);
}
//---------------------------------------------------------------------------
TIRichEditOle::~TIRichEditOle(void)
{
  // close any active objects unconditionally
  if (FIRichEditOle) CloseActiveObjects(false);

  // set the callback to null
  ::SendMessage(FRichEdit->Handle, EM_SETOLECALLBACK, 0, (LPARAM) 0);

  // release the interface
  if (FIRichEditOle) {
    FIRichEditOle = 0;
    }

  if (FIRichEditCallback) {
    delete FIRichEditCallback;
    FIRichEditCallback = 0;
    }

  // balance calls to OleInitialize()
  OleUninitialize();
}
//---------------------------------------------------------------------------
void TIRichEditOle::SetHostNames(AnsiString hostApp, AnsiString hostDoc)
{
  if (!FIRichEditOle) return;
  FIRichEditOle->SetHostNames(hostApp.c_str(), hostDoc.c_str());
}
//---------------------------------------------------------------------------
LPOLECLIENTSITE TIRichEditOle::GetClientSite(void)
{
  LPOLECLIENTSITE clientSite;
  if (FIRichEditOle->GetClientSite(&clientSite) != S_OK) clientSite = 0;
  return clientSite;
}
//---------------------------------------------------------------------------
LONG TIRichEditOle::GetObjectCount(void)
{
  if (!FIRichEditOle) throw EOleError("IRichEditOle interface is not valid.");
  return FIRichEditOle->GetObjectCount();
}
//---------------------------------------------------------------------------
LONG TIRichEditOle::GetLinkCount(void)
{
  if (!FIRichEditOle) throw EOleError("IRichEditOle interface is not valid.");
  return FIRichEditOle->GetLinkCount();
}
//---------------------------------------------------------------------------
bool TIRichEditOle::InsertObject(void)
{
  LPOLEOBJECT pOleObject = 0;

  // better be a valid richedit
  if (!FRichEdit) return false;

  // make sure client site is valid
  LPOLECLIENTSITE pClientSite = GetClientSite();
  if (!pClientSite) throw EOleError("IOleClientSite interface is not valid.");

  // get substorage
  LPSTORAGE pStg;
  if (FIRichEditCallback->GetNewStorage(&pStg) != S_OK) {
    pClientSite->Release();
    throw EOleError("GetNewStorage failed.");
    }

  // display the InsertObject dialog
  TCHAR buf[MAX_PATH];
  buf[0] = 0;
  OLEUIINSERTOBJECT io;

  ::memset(&io, 0, sizeof(io));
  io.cbStruct = sizeof(io);
  io.dwFlags = IOF_SHOWHELP | IOF_CREATENEWOBJECT | IOF_CREATEFILEOBJECT |
    IOF_SELECTCREATENEW | IOF_CREATELINKOBJECT;
  io.hWndOwner = Application->MainForm->Handle;
  io.lpszFile = buf;
  io.cchFile = sizeof(buf);
  io.iid = IID_IOleObject;
  io.oleRender = OLERENDER_DRAW;
  io.lpIOleClientSite = pClientSite;
  io.lpIStorage = pStg;
  io.ppvObj = (void**) &pOleObject;
  io.clsid = CLSID_NULL;

  DWORD retVal = ::OleUIInsertObject(&io);
  if (retVal != OLEUI_SUCCESS) {
    pClientSite->Release();
    pStg->Release();
    if (io.hMetaPict) ::OleUIMetafilePictIconFree(io.hMetaPict);
    if (pOleObject) pOleObject->Release();
    if (retVal == OLEUI_CANCEL) return false;
    throw EOleError("Insert Object dialog returned failure.");
    }

  // got the object
  pOleObject = (LPOLEOBJECT) *io.ppvObj;

  REOBJECT reObj;
  ::memset(&reObj, 0, sizeof(reObj));
  reObj.cbStruct = sizeof(reObj);
  reObj.cp = REO_CP_SELECTION;
  reObj.clsid = io.clsid;
  reObj.poleobj = pOleObject;
  reObj.pstg = pStg;
  reObj.polesite = pClientSite;
  reObj.dvaspect = DVASPECT_CONTENT;
  reObj.dwFlags = REO_RESIZABLE;

  if (io.dwFlags & IOF_SELECTCREATENEW) reObj.dwFlags |= REO_BLANK;

  // try to get a valid clsid
  if (::IsEqualCLSID(reObj.clsid, CLSID_NULL)) {
#ifdef UNICODE
    ::GetClassFile(buf, &reObj.clsid);
#else
    WCHAR bufFile[MAX_PATH];
    ::MultiByteToWideChar(CP_ACP, 0, buf, -1, bufFile, sizeof(bufFile));
    ::GetClassFile(bufFile, &reObj.clsid);
#endif
    }

  // display as icon?
  if (io.dwFlags & IOF_CHECKDISPLAYASICON) {
    int fUpdate;
    if (::OleStdSwitchDisplayAspect(pOleObject, &reObj.dvaspect,
      DVASPECT_ICON, io.hMetaPict, true, false, 0, &fUpdate))
      Application->MessageBox("Cannot display object as icon.",
        "Insert Object", MB_OK | MB_ICONWARNING);
    }

  // insert the object into the richedit
  if (FIRichEditOle->InsertObject(&reObj) != S_OK) {
    pClientSite->Release();
    pStg->Release();
    if (io.hMetaPict) OleUIMetafilePictIconFree(io.hMetaPict);
    if (pOleObject) pOleObject->Release();
    throw EOleError("RichEdit refused to insert object.");
    }

  // if new object, do the show verb
  if (io.dwFlags & IOF_SELECTCREATENEW) {
    // get object position
    POINT pt;
    // hey! where is REO_IOB_SELECTION documented!
    ::SendMessage(FRichEdit->Handle, EM_POSFROMCHAR, (WPARAM) &pt, REO_IOB_SELECTION);
    RECT rect = { 0, 0, 100, 100 };
    ::OffsetRect(&rect, pt.x, pt.y);
    pOleObject->DoVerb(OLEIVERB_SHOW, 0, pClientSite, 0, FRichEdit->Handle, &rect);
    }

  pClientSite->Release();
  pStg->Release();
  if (io.hMetaPict) OleUIMetafilePictIconFree(io.hMetaPict);
  if (pOleObject) pOleObject->Release();

  return true;
}
//---------------------------------------------------------------------------
bool TIRichEditOle::PasteSpecial(void)
{
  TOleUIPasteSpecial data;
  TOleUIPasteEntry formats[8];

  ::memset(&data, 0, sizeof(data));
  ::memset(&formats, 0, sizeof(formats));

  data.cbStruct = sizeof(data);
  data.hWndOwner = Application->MainForm->Handle;
  data.arrPasteEntries = formats;
  data.cPasteEntries = sizeof(formats) / sizeof(formats[0]);
  data.arrLinkTypes = &CFLinkSource;
  data.cLinkTypes = 1;

  // the following entries were devined from MS MFC code and appear to
  // be fairly standard for rich edit controls; this is basically a static
  // table and could be moved, but the overhead here is small...
  formats[0].fmtetc.cfFormat = (CLIPFORMAT) CFEmbeddedObject;
  formats[0].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[0].fmtetc.lindex = -1;
  formats[0].fmtetc.tymed = TYMED_ISTORAGE;
  formats[0].lpstrFormatName = "%s";
  formats[0].lpstrResultText = "%s";
  formats[0].dwFlags = OLEUIPASTE_PASTE | OLEUIPASTE_ENABLEICON;

  formats[1].fmtetc.cfFormat = (CLIPFORMAT) CFLinkSource;
  formats[1].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[1].fmtetc.lindex = -1;
  formats[1].fmtetc.tymed = TYMED_ISTREAM;
  formats[1].lpstrFormatName = "%s";
  formats[1].lpstrResultText = "%s";
  formats[1].dwFlags = OLEUIPASTE_LINKTYPE1 | OLEUIPASTE_ENABLEICON;

  formats[2].fmtetc.cfFormat = CF_TEXT;
  formats[2].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[2].fmtetc.lindex = -1;
  formats[2].fmtetc.tymed = TYMED_HGLOBAL;
  formats[2].lpstrFormatName = "Unformatted Text";
  formats[2].lpstrResultText = "text without any formatting";
  formats[2].dwFlags = OLEUIPASTE_PASTEONLY;

  formats[3].fmtetc.cfFormat = (CLIPFORMAT) CFRtf;
  formats[3].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[3].fmtetc.lindex = -1;
  formats[3].fmtetc.tymed = TYMED_HGLOBAL;
  formats[3].lpstrFormatName = "Formatted Text (RTF)";
  formats[3].lpstrResultText = "text with font and paragraph formatting";
  formats[3].dwFlags = OLEUIPASTE_PASTEONLY;

  formats[4].fmtetc.cfFormat = CF_ENHMETAFILE;
  formats[4].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[4].fmtetc.lindex = -1;
  formats[4].fmtetc.tymed = TYMED_ENHMF;
  formats[4].lpstrFormatName = "Picture (Enhanced Metafile)";
  formats[4].lpstrResultText = "a picture";
  formats[4].dwFlags = OLEUIPASTE_PASTEONLY;

  formats[5].fmtetc.cfFormat = CF_METAFILEPICT;
  formats[5].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[5].fmtetc.lindex = -1;
  formats[5].fmtetc.tymed = TYMED_MFPICT;
  formats[5].lpstrFormatName = "Picture (Metafile)";
  formats[5].lpstrResultText = "a picture";
  formats[5].dwFlags = OLEUIPASTE_PASTEONLY;

  formats[6].fmtetc.cfFormat = CF_DIB;
  formats[6].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[6].fmtetc.lindex = -1;
  formats[6].fmtetc.tymed = TYMED_MFPICT;
  formats[6].lpstrFormatName = "Device Independent Bitmap";
  formats[6].lpstrResultText = "a device independent bitmap";
  formats[6].dwFlags = OLEUIPASTE_PASTEONLY;

  formats[7].fmtetc.cfFormat = CF_BITMAP;
  formats[7].fmtetc.dwAspect = DVASPECT_CONTENT;
  formats[7].fmtetc.lindex = -1;
  formats[7].fmtetc.tymed = TYMED_GDI;
  formats[7].lpstrFormatName = "Bitmap";
  formats[7].lpstrResultText = "a bitmap";
  formats[7].dwFlags = OLEUIPASTE_PASTEONLY;

  DWORD retVal = OleUIPasteSpecial(&data);
  if (retVal == OLEUI_OK) {
    // apparently, richedit handles linking for us; unfortunately, some
    // objects do not embed (MS Word/Office 97 simply fails to embed,
    // although linking works...
    if (FIRichEditOle->ImportDataObject(data.lpSrcDataObj,
      formats[data.nSelectedIndex].fmtetc.cfFormat,
      (data.dwFlags & PSF_CHECKDISPLAYASICON) ? data.hMetaPict : 0) != S_OK)
      throw EOleError("RichEdit refused to paste object.");
    }

  if (data.hMetaPict) OleUIMetafilePictIconFree(data.hMetaPict);
  if (data.lpSrcDataObj) data.lpSrcDataObj->Release();
  return retVal == OLEUI_OK;
}
//---------------------------------------------------------------------------
// close any active ole objects (else servers left hanging); return false on
// cancel unconditionally if savePrompt != true (changes are lost)
//
bool TIRichEditOle::CloseActiveObjects(bool savePrompt)
{
  // if no interface, yell
  if (!FIRichEditOle) throw EOleError("RichEdit OLE interface is not valid.");

  // get the total number of objects
  int objCnt = FIRichEditOle->GetObjectCount();

  // check each object and, if active, deactivate and, if open, close
  for (int i = 0; i < objCnt; i++) {
    REOBJECT reObj;
    ::memset(&reObj, 0, sizeof(reObj));
    reObj.cbStruct = sizeof(reObj);

    // get object data
    if (FIRichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ) == S_OK) {
      // if active, kill it
      if (reObj.dwFlags & REO_INPLACEACTIVE)
        FIRichEditOle->InPlaceDeactivate();
      // if open, close it (prompting if requested)
      HRESULT hr = S_OK;
      if (reObj.dwFlags & REO_OPEN)
        hr = reObj.poleobj->Close(savePrompt ?
          OLECLOSE_PROMPTSAVE : OLECLOSE_NOSAVE);
      // release the interface
      reObj.poleobj->Release();
      // if cancelled, return false
      if (hr == OLE_E_PROMPTSAVECANCELLED) return false;
      }
    }

  return true;
}
//---------------------------------------------------------------------------
// insert a TBitmap programmatically. returns true on success or false if
// the bitmap cannot be inserted due to insufficient memory, etc.
//
bool __fastcall TIRichEditOle::InsertBitmap(Graphics::TBitmap* bmp)
{
// *** the following code is subject to revision ***
//
// note: mixed "return false" or throw exception seems incongruant.  maybe pick
// one for consistency?
//
// made some changes but, for now, make sure that failures (return false) properly
// free storage, interfaces, etc.  (I am leaving the exception code in but
// commented for now.)  probably still need to ensure that allocations are
// freed...
//
  LPSTORAGE lpStorage = 0;
  LPOLECLIENTSITE lpClientSite = 0;
  LPLOCKBYTES lpLockBytes = 0;
  SCODE sc;

  sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
//  if (sc != S_OK) throw EOleError("CreateILockBytesOnHGlobal() failed.");
  if (sc != S_OK) return false;

  sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
    STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &lpStorage);
  if (sc != S_OK) {
    lpLockBytes->Release();
    lpLockBytes = NULL;
//    throw EOleError("StgCreateDocfileOnILockBytes() failed.");
    return false;
    }

  // attempt to create the object
  if (FIRichEditOle->GetClientSite(&lpClientSite) != S_OK)
//    throw EOleError("GetClientSite() failed.");
    return false;

  // create an IDataObject object and interface for the bitmap
  TIDataObjectBmp* pTIDataObjBmp = new TIDataObjectBmp(bmp);
  IDataObject* lpDataObj = 0;
  HRESULT hr = pTIDataObjBmp->QueryInterface(IID_IDataObject,
    (void**) &lpDataObj);

  if (hr != S_OK) return false;

  sc = FIRichEditOle->ImportDataObject(lpDataObj, CF_BITMAP, NULL);
//  if (sc != S_OK) EOleError("InsertObject() failed.");
  if (sc != S_OK) return false;

  // release interfaces
  if (lpDataObj) lpDataObj->Release();
  if (lpStorage) lpStorage->Release();
  if (lpClientSite) lpClientSite->Release();

  // return no errors
  return true;
}
//---------------------------------------------------------------------------
// insert a TOleContainer object programmatically.  returns true on success
// or false on failure.
//
bool __fastcall TIRichEditOle::InsertContainerObject(TOleContainer* obj,
  CLIPFORMAT fmt)
{
  // if no ole interface, fail (can this happen?)
  if (!FIRichEditOle) return false;

  // get an IOleObject interface from the TOleContainer
  IOleObject* lpOleObj = obj->OleObjectInterface;
  if (!lpOleObj) return false;

  // get an IDataObject interface from the IOleObject interface
  IDataObject* lpDataObj;
  HRESULT hr = lpOleObj->QueryInterface(IID_IDataObject, (void**) &lpDataObj);
  if (hr != S_OK) {
    lpOleObj->Release();
    return false;
    }

  // import the object (using the most
  hr = FIRichEditOle->ImportDataObject(lpDataObj, fmt, NULL);

  // release interfaces
  lpDataObj->Release();
  lpOleObj->Release();

  // return success/failure
  return hr == S_OK;
}
//---------------------------------------------------------------------------
