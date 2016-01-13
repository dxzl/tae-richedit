//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeRichEditAdvPrint.h - header file for TaeRichEditAdvPrint.cpp (advanced
// print and print preview functionality).
//
// note:  the TRichEditAdvPrint class works in TWIPs, not pixels or some other
// unit of measure.  the TRichEditPrint class includes methods to convert
// between TWIPs and other units.
//---------------------------------------------------------------------------
#ifndef TaeRichEditAdvPrintH
#define TaeRichEditAdvPrintH

#include "TaeRichEditPrint.h"

//---------------------------------------------------------------------------
#include "TaePageLayout.h"
//---------------------------------------------------------------------------
typedef struct tagTTaePageRects
{
  TRect HeaderBorder;
  TRect BodyBorder;
  TRect FooterBorder;
  TRect HeaderRender;
  TRect BodyRender;
  TRect FooterRender;
} TTaePageRects;

class PACKAGE TTaeRichEditAdvPrint : public TTaeRichEditPrint
{
friend class PACKAGE TTaeRichEdit;

protected:
  TTaeHeaderText* FHeader;
  TTaeHeaderText* FFooter;
  TTaeHeaderText* FFirstHeader;
  TTaeHeaderText* FFirstFooter;
  bool FUseFirstHeader;
  bool FNeedToCalcDrawRects;
  TTaeBorderLines FBorders;
  int FInsideMargin;
  int FBorderWidth;

  void __fastcall SetHeader(TTaeHeaderText* header) { SetAHeader(FHeader, header); };
  void __fastcall SetFooter(TTaeHeaderText* footer) { SetAHeader(FFooter, footer); };
  void __fastcall SetFirstHeader(TTaeHeaderText* header) { SetAHeader(FFirstHeader, header); };
  void __fastcall SetFirstFooter(TTaeHeaderText* footer) { SetAHeader(FFirstFooter, footer); };
  void __fastcall SetAHeader(TTaeHeaderText* headerFooter, TTaeHeaderText* newHeaderFooter);

  virtual TRect __fastcall GetRendRect(HDC hdc, int page);
  virtual void DrawMargins(HDC hdc, int page, TRect margins);

  virtual void CalcRects(void);
  void CalcDrawRects(HDC hdc);
  void CalcRectsForPage(TTaeHeaderText* header, TTaeHeaderText* footer,
    TTaePageRects& pageRects, TTaeBorderLines borderLines, TRect renderRect);
  int GetTextHeight(TTaeHeaderText* header);
  int DrawHeader(HDC hdc, TTaeHeaderText* header, TRect rect);
  void DrawBorders(HDC hdc, TTaePageRects& pageRects, TTaeBorderLines lines);
  virtual void StartPrint(void);
  virtual void AfterPage(HDC hdc, int page);

public:
  __fastcall TTaeRichEditAdvPrint(TTaeRichEdit* Owner);
  __fastcall ~TTaeRichEditAdvPrint();

  void SetPageStyle(TTaePageStyle& style);

__published:
  __property TTaeHeaderText* Header = { read = FHeader, write = SetHeader, nodefault };
  __property TTaeHeaderText* Footer = { read = FFooter, write = SetFooter, nodefault };
  __property TTaeHeaderText* FirstHeader = { read = FFirstHeader, write = SetFirstHeader, nodefault };
  __property TTaeHeaderText* FirstFooter = { read = FFirstFooter, write = SetFirstFooter, nodefault };
  __property bool UseFirstHeaderFooter = { read = FUseFirstHeader, write = FUseFirstHeader, default = false };
  __property TTaeBorderLines Borders = { read = FBorders, write = FBorders, nodefault };
  __property int InsideMargin = { read = FInsideMargin, write = FInsideMargin, default = 100 };
  __property int BorderWidth = { read = FBorderWidth, write = FBorderWidth, default = 1 };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
