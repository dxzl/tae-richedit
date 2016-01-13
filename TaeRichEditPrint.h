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
// TaeRichEditPrint.h - header file for TaeRichEditPrint.cpp (print and
// print preview support).
//
// note:  the TRichEditPrint class works in TWIPs, not pixels or some other
// unit of measure.  the class includes methods to convert between TWIPs and
// other units. 
//---------------------------------------------------------------------------
#ifndef TaeRichEditPrintH
#define TaeRichEditPrintH

#include <vcl\ComCtrls.hpp>
#include <vcl\printers.hpp>
#include <vector>
#include <values.h>

//---------------------------------------------------------------------------
//typedef enum _Process_DPI_Awareness {
//  Process_DPI_Unaware            = 0,
//  Process_System_DPI_Aware       = 1,
//  Process_Per_Monitor_DPI_Aware  = 2
//} Process_DPI_Awareness;

typedef struct tagTTaePageOffset
{
  long int Start;
  long int End;
} TTaePageOffsets;

typedef struct tagTTaeInfoDC
{
  long int xLogPix;
  long int yLogPix;
  long int xPhysPage;
  long int yPhysPage;
  long int xMinMargin;
  long int yMinMargin;
} TTaeInfoDC;

typedef struct tagTPageOffset
{
  long int Start;
  long int End;
  RECT rendRect;
} TPageOffsets;

typedef TTaePageOffsets TTaePageRange;

typedef void __fastcall (__closure *TOnPageEvent)(TObject* Sender, HDC hdc, int page);

class PACKAGE TTaeRichEditPrint : public TPersistent
{
friend class PACKAGE TTaeRichEdit;

protected:
  TTaeRichEdit* FRichEdit;  // the rich edit control associated with this instance
  TRect FMargins;        // requested margins in twips
  TRect FRendRect;      // render rect in twips
  TRect FPageRect;      // page rect in twips
  TPoint FPrinterOffset;    // offset of printed page in printer device units
  std::vector<TTaePageOffsets> FPageOffsets;
  HDC FRendDC;        // rendering hdc
  bool FPrinting, FShowMargins;
  int FRendWidth, FRendHeight; // in device units (pixels)
  int FScaleX, FScaleY;
  bool FAbortPrint;

  TTaeInfoDC FInfo;

  virtual void CalcRects(void);
  virtual TRect __fastcall GetRendRect(HDC hdc, int page);
  TRect __fastcall GetRendRect(HDC hdc, TRect rMargins, int &w, int &h);
  virtual long int FormatRange(TFormatRange& formatRange, bool render);
  virtual void ClearRenderingCache(void);
  void __fastcall SetMargins(TRect margins);
  HDC GetTargDC(void);
  HDC __fastcall CreatePrinterDC(void);
  void __fastcall SetRendDC(HDC rendDC);
  void __fastcall SetShowMargins(bool showMargins);
  TTaeInfoDC __fastcall GetInfo(void);
  TTaeInfoDC __fastcall GetInfo(HDC hdc);
  int __fastcall GetPageCount(void);
//  int __fastcall GetPrinterPageCount(void);
  virtual void DrawMargins(HDC hdc, int page, TRect margins);
  CHARRANGE __fastcall GetSelRange(void);
  int __fastcall GetSelStartPage(void);
  int __fastcall GetSelEndPage(void);
  TOnPageEvent FOnBeforePage;
  TOnPageEvent FOnAfterPage;
  TNotifyEvent FOnStartPrint;
  TNotifyEvent FOnEndPrint;
  virtual void BeforePage(HDC hdc, int page);
  virtual void AfterPage(HDC hdc, int page);
  virtual void StartPrint(void);
  virtual void EndPrint(void);

public:
  __fastcall TTaeRichEditPrint(TTaeRichEdit* Owner);
  __fastcall ~TTaeRichEditPrint(void);
  virtual bool RenderPage(HDC hdc, int w, int h, int page);
  virtual bool PaginateTo(int page);
  ::TTaePageRange __fastcall GetOffsetPages(CHARRANGE chrg);
  ::TTaePageRange __fastcall GetOffsetPages(int begOffset = 1, int endOffset = MAXINT);

  virtual void BeginRender(int maxPages);
  virtual void EndRender(void);
  bool PrintToPrinter(WideString spoolTitle = "", int startPage = 1,
    int endPage = MAXINT, int copies = 1, bool collate = false);

  TRect TwipsToRenderRect(TRect rect);
  TRect TwipsToTargetRect(HDC hdc, TRect rect);

  __property TRect Margins = { read = FMargins, write = SetMargins, nodefault };
  __property HDC RendDC = { read = FRendDC, write = SetRendDC, nodefault };
  __property TRect TargetRendRect = { read = FRendRect };
  __property TRect TargetPageRect = { read = FPageRect };
  __property int PageCount = { read = GetPageCount };
//  __property int PrinterPageCount = { read = GetPrinterPageCount };
  __property int RendWidth = { read = FRendWidth, write = FRendWidth, nodefault };
  __property int RendHeight = { read = FRendHeight, write = FRendHeight, nodefault };
  __property int SelStartPage = { read = GetSelStartPage };
  __property int SelEndPage = { read = GetSelEndPage };
  __property int ScaleX = { read = FScaleX, write = FScaleX };
  __property int ScaleY = { read = FScaleY, write = FScaleY };
  __property bool AbortPrint = { read = FAbortPrint, write = FAbortPrint, nodefault };
  __property bool Printing = { read = FPrinting };

__published:
  __property bool ShowMargins = { read = FShowMargins, write = SetShowMargins, nodefault };
  __property TOnPageEvent OnBeforePage = { read = FOnBeforePage, write = FOnBeforePage, nodefault };
  __property TOnPageEvent OnAfterPage = { read = FOnAfterPage, write = FOnAfterPage, nodefault };
  __property TNotifyEvent OnStartPrint = { read = FOnStartPrint, write = FOnStartPrint, nodefault };
  __property TNotifyEvent OnEndPrint = { read = FOnEndPrint, write = FOnEndPrint, nodefault };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
