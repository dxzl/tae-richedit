// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 2000 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeDataObjBmp.h - header file for TaeDataObjBmpInt.cpp
//---------------------------------------------------------------------------
#ifndef TaeDataObjBmpH
#define TaeDataObjBmpH

#include <vcl\olectnrs.hpp>
//---------------------------------------------------------------------------
class PACKAGE TIDataObjectBmp : public IDataObject
{
protected:
  int FRefCnt;
  Graphics::TBitmap* FBitmap;

public:
  // *** IUnknown methods ***
  STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj);
  STDMETHOD_(ULONG, AddRef) (THIS);
  STDMETHOD_(ULONG, Release) (THIS);

  // *** IDataObject methods ***
  STDMETHOD(GetData) (THIS_ LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
  STDMETHOD(GetDataHere) (THIS_ LPFORMATETC pFE, LPSTGMEDIUM pSTM);
  STDMETHOD(QueryGetData) (THIS_ LPFORMATETC pFE);
  STDMETHOD(GetCanonicalFormatEtc) (THIS_ LPFORMATETC pFEIn, LPFORMATETC pFEOut);
  STDMETHOD(SetData) (THIS_ LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease);
  STDMETHOD(EnumFormatEtc) (THIS_ DWORD dwDirection, LPENUMFORMATETC* ppEnum);
  STDMETHOD(DAdvise) (THIS_ LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink, LPDWORD pdwConnection);
  STDMETHOD(DUnadvise) (THIS_ DWORD dwConnection);
  STDMETHOD(EnumDAdvise) (THIS_ LPENUMSTATDATA* ppEnum);

  TIDataObjectBmp(Graphics::TBitmap* bmp);
  ~TIDataObjectBmp(void);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
