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
// TaeDataObjBmp.cpp - TIDataObjectBmp class (IDataObject interface) to
// support programmatic insertion of bitmaps into TTaeRichEdit objects.
//---------------------------------------------------------------------------
// Note:  Although the IDataObject interface supports transfer of arbitrary
// data types, this code is designed to support exactly one data type
// (bitmap).
//
// The code could easily be modified to support multiple data types but care
// will be required to alter logic that assumes exactly one element in the
// FormatEtc structure and the related IEnumFORMATETC interface
// (TIDataObjEnuFmtEtc below) as well as changes to the IDataObjectBmp
// interface.
//
// Note that the following code simply inserts a raw bitmap into a TaeRichEdit
// Component.  It does not insert the bitmap with any association with an
// editor (such as MS Paint).
//---------------------------------------------------------------------------
#include "TaeDataObjBmp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// FORMATETC structure containing valid formats
//
FORMATETC FormatEtc[] = {
  { CF_BITMAP, 0, DVASPECT_CONTENT, -1, TYMED_GDI }
  // additional formats could be added here; of course,
  // additional code would be required in the interface classes below...
  // for example, you could add support for metafiles using
  // { CF_METAFILEPICT, 0, DVASPECT_CONTENT, -1, TYMED_MFPICT }
  };
//---------------------------------------------------------------------------
// forward declaration of bitmap rendering function
//
HRESULT RenderBitmap(LPSTGMEDIUM pSTM, Graphics::TBitmap* bmp);
//---------------------------------------------------------------------------
// IEnumFORMATETC interface class declaration
//---------------------------------------------------------------------------
class PACKAGE TIDataObjBmpEnumFmtEtc : public IEnumFORMATETC
{
protected:
  ULONG FRefCnt;
  int FIndex;
  
public:
  // *** IUnknown methods ***
  STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj);
  STDMETHOD_(ULONG, AddRef) (THIS);
  STDMETHOD_(ULONG, Release) (THIS);

  // *** IEnumFORMATETC methods ***
  STDMETHOD(Next) (THIS_ ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
  STDMETHOD(Skip) (THIS_ ULONG celt);
  STDMETHOD(Reset) (THIS_);
  STDMETHOD(Clone) (THIS_ IEnumFORMATETC** ppEnum);

  TIDataObjBmpEnumFmtEtc(void);
  ~TIDataObjBmpEnumFmtEtc(void);
};
//---------------------------------------------------------------------------
// IEnumFORMATETC interface class implementation
//---------------------------------------------------------------------------
// TIDataObjBmpEnumFmtEtc constructor
//
TIDataObjBmpEnumFmtEtc::TIDataObjBmpEnumFmtEtc(void) : FRefCnt(0), FIndex(0)
{
}
//---------------------------------------------------------------------------
// TIDataObjBmpEnumFmtEtc destructor
//
TIDataObjBmpEnumFmtEtc::~TIDataObjBmpEnumFmtEtc(void)
{
}
//---------------------------------------------------------------------------
// QueryInterface() implementation
//
STDMETHODIMP TIDataObjBmpEnumFmtEtc::QueryInterface(THIS_ REFIID riid,
  LPVOID FAR * lplpObj)
{
  // validate arguments
  if (!lplpObj) return E_NOINTERFACE;

  // return S_OK if the interface is supported, E_NOINTERFACE if not
  // note: all of this happy type casting is presumably gratuitous
  if (riid == IID_IUnknown) *lplpObj = (LPUNKNOWN) this;
  else if (riid == IID_IEnumFORMATETC) *lplpObj = (IEnumFORMATETC*) this;
  else {
    *lplpObj = 0;
    return E_NOINTERFACE;
    }

  // interface is supported; increment reference count before returning
  // interface pointer
  ((LPUNKNOWN) *lplpObj)->AddRef();
  return S_OK;
}
//---------------------------------------------------------------------------
// AddRef() implementation
//
STDMETHODIMP_(ULONG) TIDataObjBmpEnumFmtEtc::AddRef(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  return ++FRefCnt;
}
//---------------------------------------------------------------------------
// Release() implementation
//
STDMETHODIMP_(ULONG) TIDataObjBmpEnumFmtEtc::Release(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  --FRefCnt;

  // if reference count reaches zero, delete self
  if (!FRefCnt) delete this;

  // return current reference count
  return FRefCnt;
}
//---------------------------------------------------------------------------
// Next() implementation
//
STDMETHODIMP TIDataObjBmpEnumFmtEtc::Next(THIS_ ULONG celt, FORMATETC* rgelt,
  ULONG* pceltFetched)
{
  // validate arguments
  if (!rgelt || !pceltFetched) return ResultFromScode(E_INVALIDARG);

  // we only support one format, so fail if not first call
  if (FIndex || celt) return ResultFromScode(S_FALSE);

  // first call so copy FORMATETC structure into *rgelt
  memcpy(rgelt, FormatEtc, sizeof(FormatEtc));

  // set return FORMATETC element count to one...
  if (pceltFetched) *pceltFetched = 1;

  // ... and return success
  return ResultFromScode(S_OK);
}
//---------------------------------------------------------------------------
// Skip() implementation
//
STDMETHODIMP TIDataObjBmpEnumFmtEtc::Skip(THIS_ ULONG celt)
{
  // we only have one entry so there are no entries to skip
  return ResultFromScode(S_FALSE);
}
//---------------------------------------------------------------------------
// Reset() implementation
//
STDMETHODIMP TIDataObjBmpEnumFmtEtc::Reset(THIS_)
{
  // reset index to first FormatEtc[] entry
  FIndex = 0;
  return ResultFromScode(S_OK);
}
//---------------------------------------------------------------------------
// Clone() implementation
//
STDMETHODIMP TIDataObjBmpEnumFmtEtc::Clone(THIS_ IEnumFORMATETC** ppEnum)
{
  // validate arguments
  if (!ppEnum) return ResultFromScode(E_INVALIDARG);

  // allocate new interface object; trap creation failure
  try {
    TIDataObjBmpEnumFmtEtc* pEnum = new TIDataObjBmpEnumFmtEtc();
    pEnum->FIndex = FIndex;
    *ppEnum = pEnum;
    return ResultFromScode(S_OK);
    }
  catch(...) {
    // new TIDataObjBmpEnumFmtEtc() failed, presumably because there
    // was insufficient memory; fall through to exit below
    }

  // creation failure
  return ResultFromScode(E_OUTOFMEMORY);
}
//---------------------------------------------------------------------------
// TIDataObjectBmp interfaces
//---------------------------------------------------------------------------
// TIDataObjectBmp constructor
//
TIDataObjectBmp::TIDataObjectBmp(Graphics::TBitmap* bmp) : FRefCnt(0)
{
  // copy bitmap object; we will release in destructor
  FBitmap = new Graphics::TBitmap;
  FBitmap->Assign(bmp);
}
//---------------------------------------------------------------------------
// TIDataObjectBmp destructor
//
TIDataObjectBmp::~TIDataObjectBmp(void)
{
  // release bitmap
  delete FBitmap;
}
//---------------------------------------------------------------------------
// QueryInterface() implementation
//
STDMETHODIMP TIDataObjectBmp::QueryInterface(THIS_ REFIID riid,
  LPVOID FAR * lplpObj)
{
  // validate arguments
  if (!lplpObj) return E_NOINTERFACE;

  // return S_OK if the interface is supported, E_NOINTERFACE if not
  // note: all of this happy type casting is presumably gratuitous
  if (riid == IID_IUnknown) *lplpObj = (LPUNKNOWN) this;
  else if (riid == IID_IDataObject) *lplpObj = (IDataObject*) this;
  else {
    *lplpObj = 0;
    return E_NOINTERFACE;
    }

  // interface is supported; increment reference count before returning
  // interface pointer
  ((LPUNKNOWN) *lplpObj)->AddRef();
  return S_OK;
}
//---------------------------------------------------------------------------
// AddRef() implementation
//
STDMETHODIMP_(ULONG) TIDataObjectBmp::AddRef(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  return ++FRefCnt;
}
//---------------------------------------------------------------------------
// Release() implementation
//
STDMETHODIMP_(ULONG) TIDataObjectBmp::Release(THIS)
{
  // need to add real reference counting if any caller really holds
  // on to interface pointers
  --FRefCnt;

  // if reference count reaches zero, delete self
  if (!FRefCnt) delete this;

  // return current reference count
  return FRefCnt;
}
//---------------------------------------------------------------------------
// GetData() implementation
//
STDMETHODIMP TIDataObjectBmp::GetData(THIS_ LPFORMATETC pFEIn,
  LPSTGMEDIUM pSTM)
{
  // validate arguments
  if (!pFEIn || !pSTM) return ResultFromScode(E_INVALIDARG);

  // we support only rendering content
  if (!(pFEIn->dwAspect & DVASPECT_CONTENT))
    return ResultFromScode(DATA_E_FORMATETC);

  // we support only bitmap rendering
  if (pFEIn->cfFormat != CF_BITMAP || !(pFEIn->tymed & TYMED_GDI))
    return ResultFromScode(DATA_E_FORMATETC);

  // attempt to insert bitmap and return result
  return RenderBitmap(pSTM, FBitmap);
}
//---------------------------------------------------------------------------
// GetDataHere() implementation
//
STDMETHODIMP TIDataObjectBmp::GetDataHere(THIS_ LPFORMATETC pFE,
  LPSTGMEDIUM pSTM)
{
  // not implemented
  return E_NOTIMPL;
}
//---------------------------------------------------------------------------
// QueryGetData() implementation
//
STDMETHODIMP TIDataObjectBmp::QueryGetData(THIS_ LPFORMATETC pFE)
{
  // validate arguments
  if (!pFE) return ResultFromScode(E_INVALIDARG);

  // we support only rendering content
  if (!(pFE->dwAspect & DVASPECT_CONTENT))
    return ResultFromScode(DATA_E_FORMATETC);

  // we support only bitmap rendering
  if (pFE->cfFormat != CF_BITMAP || !(pFE->tymed & TYMED_GDI))
    return ResultFromScode(S_FALSE);

  // we can do it so return success
  return NOERROR;
}
//---------------------------------------------------------------------------
// GetCanonicalFormatEtc() implementation
// Note: GetCanonicalFormatEtc() is used to determine if a standard FORMATETC
// structure is logically equivalent to one that is more complex.  In other 
// words, this interface method is used to determine if two different
// FORMATETC structures would result in the same data.  Since we support only
// one format, the formats being compared must always be equivalent (in this
// case, they must be identical).  if you add support for other formats, this
// method will change radically....
//
STDMETHODIMP TIDataObjectBmp::GetCanonicalFormatEtc(THIS_ LPFORMATETC pFEIn,
  LPFORMATETC pFEOut)
{
  // make sure that arguments are valid
  if (!pFEIn || !pFEOut) return ResultFromScode(E_INVALIDARG);

  // we only support one format, so it must be identical to itself
  pFEOut->ptd = NULL;
  return ResultFromScode(DATA_S_SAMEFORMATETC);
}
//---------------------------------------------------------------------------
// SetData() implementation
//
STDMETHODIMP TIDataObjectBmp::SetData(THIS_ LPFORMATETC pFE,
  LPSTGMEDIUM pSTM, BOOL fRelease)
{
  // not implemented
  return E_NOTIMPL;
}
//---------------------------------------------------------------------------
// EnumFormatEtc() implementation
//
STDMETHODIMP TIDataObjectBmp::EnumFormatEtc(THIS_ DWORD dwDirection,
  LPENUMFORMATETC* ppEnum)
{
  // make sure that arguments are valid
  // we support only getting the available formats (i.e., DATADIR_SET
  // is not supported)
  if (dwDirection != DATADIR_GET) {
    *ppEnum = NULL;
    return ResultFromScode(E_FAIL);
    }
  // ppEnum must not be null
  if (!ppEnum) return ResultFromScode(E_INVALIDARG);

  // create a format enumerator interface object (do not delete on exit --
  // the object will delete itself when its reference count returns to zero)
  TIDataObjBmpEnumFmtEtc* pEnumObj;
  try {
    pEnumObj = new TIDataObjBmpEnumFmtEtc();
    }
  catch (...) {
    // new failed so there must be insufficient memory
    return ResultFromScode(E_OUTOFMEMORY);
    }

  // get the enumerator interface
  IEnumFORMATETC* pIEnum = 0;
  HRESULT hr = pEnumObj->QueryInterface(IID_IEnumFORMATETC, (void**) &pIEnum);
  if (hr != S_OK) return hr;

  // return the format enumerator and a successful result code
  *ppEnum = pIEnum;
  return NOERROR;
}
//---------------------------------------------------------------------------
// DAdvise() implementation
//
STDMETHODIMP TIDataObjectBmp::DAdvise(THIS_ LPFORMATETC pFE, DWORD grfAdv,
  LPADVISESINK pAdvSink, LPDWORD pdwConnection)
{
  // we do not support change advises
  return OLE_E_ADVISENOTSUPPORTED;
}
//---------------------------------------------------------------------------
// DUnadvise() implementation
//
STDMETHODIMP TIDataObjectBmp::DUnadvise(THIS_ DWORD dwConnection)
{
  // we do not support change advises
  return E_NOTIMPL;
}
//---------------------------------------------------------------------------
// EnumDAdvise() implementation
//
STDMETHODIMP TIDataObjectBmp::EnumDAdvise(THIS_ LPENUMSTATDATA* ppEnum)
{
  // we do not support change advises
  return E_NOTIMPL;
}
//---------------------------------------------------------------------------
// RenderBitmap() - code to render a bitmap to a STGMEDIUM.  this code was
// adapted from _Inside_OLE_ by Kraig Brockschmidt
//
HRESULT RenderBitmap(LPSTGMEDIUM pSTM, Graphics::TBitmap* bmp)
{
  HBITMAP     hBmp, hBmpT;
  HDC         hDC, hDCSrc, hDCDst;

  //Get two memory DCs between which to BitBlt.
  hDC = GetDC(NULL);
  hDCSrc = CreateCompatibleDC(hDC);
  hDCDst = CreateCompatibleDC(hDC);
  ReleaseDC(NULL, hDC);

  if (NULL==hDCSrc || NULL==hDCDst) {
    if (NULL!=hDCDst) DeleteDC(hDCDst);
    if (NULL!=hDCSrc) DeleteDC(hDCSrc);
    return ResultFromScode(STG_E_MEDIUMFULL);
    }

  SelectObject(hDCSrc, bmp->Handle);

  hBmp = CreateCompatibleBitmap(hDCSrc, bmp->Width, bmp->Height);

  if (!hBmp) {
    DeleteDC(hDCDst);
    DeleteDC(hDCSrc);
    return ResultFromScode(STG_E_MEDIUMFULL);
    }

  //Copy from the source to destination
  hBmpT = (HBITMAP) SelectObject(hDCDst, hBmp);
  BitBlt(hDCDst, 0, 0, bmp->Width, bmp->Height, hDCSrc, 0, 0, SRCCOPY);
  SelectObject(hDCDst, hBmpT);

  DeleteDC(hDCDst);
  DeleteDC(hDCSrc);

  pSTM->hGlobal = (HGLOBAL) hBmp;
  pSTM->tymed = TYMED_GDI;
  pSTM->pUnkForRelease = NULL;
  return NOERROR;
}
//---------------------------------------------------------------------------
