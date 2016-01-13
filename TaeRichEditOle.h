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
// TaeRichEditOle.h - header file for TaeRichEditOle.cpp.
//---------------------------------------------------------------------------
#ifndef RichEditOLEH
#define RichEditOLEH

#include <vcl\comctrls.hpp>
#include <vcl\comobj.hpp>
#include <vcl\ole2.hpp>
#include <vcl\oledlg.hpp>
#include <richole.h>
#include <vcl\olectnrs.hpp>
#include "TaeRichEditOleCallback.h"
//---------------------------------------------------------------------------
class PACKAGE TIRichEditOleCallback;

class PACKAGE TIRichEditOle
{
protected:
  TTaeRichEdit* FRichEdit;
  IRichEditOle* FIRichEditOle;
  TIRichEditOleCallback* FIRichEditCallback;

  LONG GetObjectCount(void);
  LONG GetLinkCount(void);

  UINT CFObjectDescriptor;
  UINT CFEmbeddedObject;
  UINT CFLinkSource;
  UINT CFRtf;
  UINT CFRtfNoObjs;
  UINT CFReTextObjs;

  TOleForm* FFrameForm;

public:
  TIRichEditOle(TTaeRichEdit* richedit);
  ~TIRichEditOle(void);
  void SetHostNames(AnsiString hostApp, AnsiString hostDoc);
  bool InsertObject(void);
  bool PasteSpecial(void);
  bool CloseActiveObjects(bool savePrompt);

  LPOLECLIENTSITE GetClientSite(void);

  __property LONG ObjectCount = { read = GetObjectCount };
  __property LONG LinkCount = { read = GetLinkCount };

  __property TIRichEditOleCallback* OleCallback = { read = FIRichEditCallback };

  // use the following to insert a bitmap programmatically
  bool __fastcall InsertBitmap(Graphics::TBitmap* bmp);
  // use the following to insert the contents of a TOleContainer programmatically
  bool __fastcall InsertContainerObject(TOleContainer* obj, CLIPFORMAT fmt = 0);
};
#endif
//---------------------------------------------------------------------------
