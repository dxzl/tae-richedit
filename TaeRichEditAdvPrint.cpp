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
// TaeRichEditAdvPrint.cpp - implementation file for TaeRichEditAdvPrint.cpp
// (advanced print and print preview support).
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeRichEditAdvPrint.h"
#include "TaeParser.h"
#include "TaeRichEdit.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static TTaeParser Parser("mm/dd/yy", "hh:nn:ss");
static TTaePageRects PageRects;
static TTaePageRects DrawRects;
static TTaePageRects FirstPageRects;
static TTaePageRects FirstDrawRects;
//---------------------------------------------------------------------------
// TTaeRichEditAdvPrint constructor
//
__fastcall TTaeRichEditAdvPrint::TTaeRichEditAdvPrint(TTaeRichEdit* Owner) :
  TTaeRichEditPrint(Owner), FHeader(new TTaeHeaderText),
  FFooter(new TTaeHeaderText), FFirstHeader(new TTaeHeaderText),
  FFirstFooter(new TTaeHeaderText), FInsideMargin(72), FBorderWidth(1),
  FUseFirstHeader(false), FNeedToCalcDrawRects(false)
{
}
//---------------------------------------------------------------------------
// TTaeRichEditAdvPrint destructor
//
__fastcall TTaeRichEditAdvPrint::~TTaeRichEditAdvPrint()
{
  if (FHeader) delete FHeader;
  if (FFooter) delete FFooter;
  if (FFirstHeader) delete FFirstHeader;
  if (FFirstFooter) delete FFirstFooter;
}
//---------------------------------------------------------------------------
// set header or footer text and recalculate the header, footer, and body
// page rectangles
//
void __fastcall TTaeRichEditAdvPrint::SetAHeader(TTaeHeaderText* headerFooter,
  TTaeHeaderText* newHeaderFooter)
{
  *headerFooter = *newHeaderFooter;
  this->CalcRects();
}
//---------------------------------------------------------------------------
// fetch the rendering rect for the TTaeRichEdit control; this overrides
// TTaeRichEditPrint::GetRendRect(); this function works in TWIPs
//
TRect __fastcall TTaeRichEditAdvPrint::GetRendRect(HDC hdc, int page)
{
  if (page > 1 || !FUseFirstHeader)
    return PageRects.BodyRender;

  return FirstPageRects.BodyRender;
}
//---------------------------------------------------------------------------
// draw the margins for TTaeRichEdit preview; this function works in TWIPs
//
void TTaeRichEditAdvPrint::DrawMargins(HDC hdc, int page, TRect margins)
{
  // could have different margins here, but width should be same for all pages
  TTaeRichEditPrint::DrawMargins(hdc, page, margins);
}
//---------------------------------------------------------------------------
// calculate the various rectangles and lines; all calculations are in TWIPs;
// the border lines will always be drawn to TTaeRichEdit::FRendRect
//
// Override...
void TTaeRichEditAdvPrint::CalcRects()
{
  // Set FRendRect, inherited from TTaeRichEditPrint, based on FRendDC
  TTaeRichEditPrint::CalcRects();

// THIS MUST BE WHERE IT IS GETTING ERROR IN THE WYSIWYG HEIGHT
  CalcRectsForPage(FFirstHeader, FFirstFooter, FirstPageRects, FBorders, FRendRect);
  CalcRectsForPage(FHeader, FFooter, PageRects, FBorders, FRendRect);
}
//---------------------------------------------------------------------------
// convert the various rectangles to target units
//
void TTaeRichEditAdvPrint::CalcDrawRects(HDC hdc)
{
  DrawRects.HeaderBorder = TwipsToTargetRect(hdc, PageRects.HeaderBorder);
  DrawRects.BodyBorder = TwipsToTargetRect(hdc, PageRects.BodyBorder);
  DrawRects.FooterBorder = TwipsToTargetRect(hdc, PageRects.FooterBorder);

  DrawRects.HeaderRender = TwipsToTargetRect(hdc, PageRects.HeaderRender);
  DrawRects.BodyRender = TwipsToTargetRect(hdc, PageRects.BodyRender);
  DrawRects.FooterRender = TwipsToTargetRect(hdc, PageRects.FooterRender);

  if (FUseFirstHeader)
  {
    FirstDrawRects.HeaderBorder = TwipsToTargetRect(hdc, FirstPageRects.HeaderBorder);
    FirstDrawRects.BodyBorder = TwipsToTargetRect(hdc, FirstPageRects.BodyBorder);
    FirstDrawRects.FooterBorder = TwipsToTargetRect(hdc, FirstPageRects.FooterBorder);

    FirstDrawRects.HeaderRender = TwipsToTargetRect(hdc, FirstPageRects.HeaderRender);
    FirstDrawRects.BodyRender = TwipsToTargetRect(hdc, FirstPageRects.BodyRender);
    FirstDrawRects.FooterRender = TwipsToTargetRect(hdc, FirstPageRects.FooterRender);
  }
}
//---------------------------------------------------------------------------
// calculate the various lines and rectangles for a page -- big and ugly...
//
void TTaeRichEditAdvPrint::CalcRectsForPage(TTaeHeaderText* header,
  TTaeHeaderText* footer,  TTaePageRects& pageRects,
    TTaeBorderLines borderLines, TRect renderRect)
{
  int vHeader = GetTextHeight(header);
  int vFooter = GetTextHeight(footer);

  // start with empty rects for all rects with renderRect left & right margins
  TRect r;  // empty??? (S.S. reply NO! need TRect r = {0};)
  r.Left = renderRect.Left;
  r.Right = renderRect.Right;
  r.Top = 0;
  r.Bottom = 0;

  pageRects.HeaderBorder = r;
  pageRects.HeaderRender = r;
  pageRects.BodyBorder = r;
  pageRects.BodyRender = r;
  pageRects.FooterBorder = r;
  pageRects.FooterRender = r;

  // if there are *any* left borders, then we want *all* rendering
  // margins to indent
  TTaeBorderLines noBorders;
  TTaeBorderLines allLeftBorders;
  allLeftBorders << blBodyLeft;

  if (vHeader)
    allLeftBorders << blHeaderLeft;

  if (vFooter)
    allLeftBorders << blFooterLeft;

  if (borderLines * allLeftBorders != noBorders)
  {
    pageRects.FooterRender.Left += FInsideMargin + FBorderWidth;
    pageRects.HeaderRender.Left = pageRects.FooterRender.Left;
    pageRects.BodyRender.Left = pageRects.FooterRender.Left;
  };

  // similarly, if there are *any* right borders, then we want *all* rendering
  // margins to indent
  TTaeBorderLines allRightBorders;
  allRightBorders << blBodyRight;

  if (vHeader)
    allRightBorders << blHeaderRight;

  if (vFooter)
    allRightBorders << blFooterRight;

  if (borderLines * allRightBorders != noBorders)
  {
    pageRects.FooterRender.Right -= FInsideMargin + FBorderWidth;
    pageRects.HeaderRender.Right = pageRects.FooterRender.Right;
    pageRects.BodyRender.Right = pageRects.FooterRender.Right;
  }

  // adjust header/footer render and border rects
  if (vHeader)
  {
    pageRects.HeaderRender.Bottom = vHeader;

    // adjust vertically for borders
    if (borderLines.Contains(blHeaderTop))
      ::OffsetRect((RECT*) &pageRects.HeaderRender, 0,
        FInsideMargin + FBorderWidth);

    pageRects.HeaderBorder.Bottom =
            pageRects.HeaderRender.Bottom + FInsideMargin;
  }

  if (vFooter)
  {    // like header, only negative offsets
    pageRects.FooterRender.Top = -vFooter;

    // adjust vertically for borders
    if (borderLines.Contains(blFooterBottom))
      ::OffsetRect((RECT*) &pageRects.FooterRender, 0, -FInsideMargin - FBorderWidth);

    pageRects.FooterBorder.Top = pageRects.FooterRender.Top - FInsideMargin;
  }

  // move header to top of rendering area
  int vHeaderMove = renderRect.Top;
  ::OffsetRect((RECT*) &pageRects.HeaderBorder, 0, vHeaderMove);
  ::OffsetRect((RECT*) &pageRects.HeaderRender, 0, vHeaderMove);

  // move footer to bottom of rendering area
  int vFooterMove = renderRect.Bottom - pageRects.FooterBorder.Bottom;
  ::OffsetRect((RECT*) &pageRects.FooterBorder, 0, vFooterMove);
  ::OffsetRect((RECT*) &pageRects.FooterRender, 0, vFooterMove);

  // adjust body to be within remaining vertical area
  pageRects.BodyRender.Top = pageRects.HeaderBorder.Bottom;
  pageRects.BodyBorder.Top = pageRects.HeaderBorder.Bottom;

  if (borderLines.Contains(blHeaderBottom) || borderLines.Contains(blBodyTop))
    pageRects.BodyRender.Top += FInsideMargin + FBorderWidth;

  pageRects.BodyRender.Bottom = pageRects.FooterBorder.Top;
  pageRects.BodyBorder.Bottom = pageRects.FooterBorder.Top;

  if (borderLines.Contains(blFooterTop) || borderLines.Contains(blBodyBottom))
    pageRects.BodyRender.Bottom -= FInsideMargin + FBorderWidth;
}
//---------------------------------------------------------------------------
// get the height of text for a header or footer.  the only way I could find
// to accurately measure the height of the header or footer line(s) was to
// actually draw the text on a compatible device context.  not pretty, but
// it finally works.
//
int TTaeRichEditAdvPrint::GetTextHeight(TTaeHeaderText* header)
{
  int height = 0;

  if (header->GetText() == "||")
    return 0;

  // create a compatible DC -- do not need a bitmap???
  HDC hdc = ::CreateCompatibleDC(Printer()->Handle);

  // save the DC state
  ::SaveDC(hdc);

  // create a rect that matches the printer
  int rWidth = ::GetDeviceCaps(hdc, HORZRES);
  int rHeight = ::GetDeviceCaps(hdc, VERTRES);
  TRect rect(0x0000, 0x0000, rWidth, rHeight);

//  // create a really big rect for bounds
//  TRect rect(0x0000, 0x0000, 0x1fff, 0x1fff);

  // create a compatible bitmap
  HBITMAP hBmp = ::CreateCompatibleBitmap(hdc, rect.Width(), rect.Height());

  // if good...
  if (hBmp)
  {
    // select bitmap into DC
    ::SelectObject(hdc, hBmp);

    // call DrawHeader to get the height
    height = DrawHeader(hdc, header, rect);
  }

  // restore the DC, delete bitmap and DC, and return height
  ::RestoreDC(hdc, -1);

  if (hBmp)
    ::DeleteObject(hBmp);

  ::DeleteDC(hdc);

  return height;
}
//---------------------------------------------------------------------------
// parse and render a header or footer.  return height of text (used only by
// GetTextHeight()).
//
int TTaeRichEditAdvPrint::DrawHeader(HDC hdc, TTaeHeaderText* header,
  TRect rect)
{
  // quick out if rect empty (should only happen if text empty in GetTextHeight())
  if (rect.Top == rect.Bottom) return 0;

  // get the three parts of the header and remove trailing "\r\n"
  AnsiString LeftText = Parser.Parse(header->Left->Text).TrimRight();
  AnsiString CenterText = Parser.Parse(header->Center->Text).TrimRight();
  AnsiString RightText = Parser.Parse(header->Right->Text).TrimRight();

  // save dc
  ::SaveDC(hdc);

  // select font into dc (took a while to figure out -- may not be only/best way)
  TFont* font = new TFont;
  font->Assign(header->Font);
  HFONT hOldFont;
  if (Printer()->Printing) {
    Printer()->Canvas->Font->Assign(font);
    hOldFont = ::SelectObject(hdc, Printer()->Canvas->Font->Handle);
    }
  else hOldFont = ::SelectObject(hdc, font->Handle);

  // set up params for DrawTextEx()
  DRAWTEXTPARAMS dtp;
  dtp.cbSize = sizeof(DRAWTEXTPARAMS);
  dtp.iTabLength = FRichEdit->FTabWidth;
  dtp.iLeftMargin = 0;
  dtp.iRightMargin = 0;
  dtp.uiLengthDrawn = 0;

  // draw left text (returns zero on fail -- ignore???)
  int vHtLeft = ::DrawTextEx(hdc, LeftText.c_str(), -1, (RECT*) &rect,
    DT_LEFT | DT_TABSTOP | DT_NOPREFIX | DT_EXPANDTABS | DT_TOP, &dtp);

  // draw center text
  int vHtCenter = ::DrawTextEx(hdc, CenterText.c_str(), -1, (RECT*) &rect,
    DT_CENTER | DT_TABSTOP | DT_NOPREFIX | DT_EXPANDTABS | DT_TOP, &dtp);

  // draw right text
  int vHtRight = ::DrawTextEx(hdc, RightText.c_str(), -1, (RECT*) &rect,
    DT_RIGHT | DT_TABSTOP | DT_NOPREFIX | DT_EXPANDTABS | DT_TOP, &dtp);

  // get vertical pixels per inch
  int vPPI = Printer()->Printing ? Printer()->Canvas->Font->PixelsPerInch :
    font->PixelsPerInch;

  // restore dc and delete temporary font
  ::SelectObject(hdc, hOldFont);  // unnecessary???
  ::RestoreDC(hdc, -1);
  delete font;

  // prepare to return the maximum height
  int height = max(max(vHtLeft, vHtCenter), vHtRight);

  // convert the height into twips and return
  height = ::MulDiv(height, 1440, vPPI);
  return height;
}
//---------------------------------------------------------------------------
// draw a border line
//
inline void DrawLine(HDC hdc, int fromX, int fromY, int toX, int toY)
{
  ::MoveToEx(hdc, fromX, fromY, NULL);
  LineTo(hdc, toX, toY);
}
//---------------------------------------------------------------------------
// draw border lines
//
void TTaeRichEditAdvPrint::DrawBorders(HDC hdc, TTaePageRects& pageRects,
  TTaeBorderLines lines)
{
  // create empty set for comparison
  TTaeBorderLines noLines;

  // quick out if no borders
  if (lines == noLines)
    return;

  // save dc
  ::SaveDC(hdc);

  // select pen of appropriate width
  int borderWidthInPixels = ::MulDiv(FBorderWidth,
                  ::GetDeviceCaps(hdc, LOGPIXELSX), 1440);

  HPEN pen = ::CreatePen(PS_SOLID, borderWidthInPixels, COLORREF(0x00000000));
  
  ::SelectObject(hdc, pen);

  // draw lines
  bool hasHeader = pageRects.HeaderBorder.Bottom > pageRects.HeaderBorder.Top;
  bool hasFooter = pageRects.FooterBorder.Bottom > pageRects.FooterBorder.Top;

  if (hasHeader)
  {
    if (lines.Contains(blHeaderTop))
      DrawLine(hdc, pageRects.HeaderBorder.Left, pageRects.HeaderBorder.Top,
        pageRects.HeaderBorder.Right, pageRects.HeaderBorder.Top);
    if (lines.Contains(blHeaderLeft))
      DrawLine(hdc, pageRects.HeaderBorder.Left, pageRects.HeaderBorder.Top,
        pageRects.HeaderBorder.Left, pageRects.HeaderBorder.Bottom);
    if (lines.Contains(blHeaderRight))
      DrawLine(hdc, pageRects.HeaderBorder.Right, pageRects.HeaderBorder.Top,
        pageRects.HeaderBorder.Right, pageRects.HeaderBorder.Bottom);
  }

  if (lines.Contains(blHeaderBottom) || lines.Contains(blBodyTop))
    DrawLine(hdc, pageRects.BodyBorder.Left, pageRects.BodyBorder.Top,

      pageRects.BodyBorder.Right, pageRects.BodyBorder.Top);
  if (lines.Contains(blBodyBottom) || lines.Contains(blFooterTop))
    DrawLine(hdc, pageRects.FooterBorder.Left, pageRects.FooterBorder.Top,

      pageRects.FooterBorder.Right, pageRects.FooterBorder.Top);
  if (lines.Contains(blBodyLeft))
    DrawLine(hdc, pageRects.BodyBorder.Left, pageRects.BodyBorder.Top,
      pageRects.BodyBorder.Left, pageRects.BodyBorder.Bottom);

  if (lines.Contains(blBodyRight))
    DrawLine(hdc, pageRects.BodyBorder.Right, pageRects.BodyBorder.Top,
      pageRects.BodyBorder.Right, pageRects.BodyBorder.Bottom);

  if (hasFooter)
  {
    if (lines.Contains(blFooterBottom))
      DrawLine(hdc, pageRects.FooterBorder.Left, pageRects.FooterBorder.Bottom,
        pageRects.FooterBorder.Right, pageRects.FooterBorder.Bottom);
    if (lines.Contains(blFooterLeft))
      DrawLine(hdc, pageRects.FooterBorder.Left, pageRects.FooterBorder.Top,
        pageRects.FooterBorder.Left, pageRects.FooterBorder.Bottom);
    if (lines.Contains(blFooterRight))
      DrawLine(hdc, pageRects.FooterBorder.Right, pageRects.FooterBorder.Top,
        pageRects.FooterBorder.Right, pageRects.FooterBorder.Bottom);
  }

  // restore dc and delete pen
  ::RestoreDC(hdc, -1);
  DeleteObject(pen);
}
//---------------------------------------------------------------------------
// override TTaeRichEditPrint::StartPrint() to calculate page rects and
// set unchanging parser substitution values
//
void TTaeRichEditAdvPrint::StartPrint(void)
{
  FNeedToCalcDrawRects = true;
  this->CalcRects(); // From Printer's HDC
  TTaeRichEditPrint::StartPrint();

  Parser.SetName(FRichEdit->FileName);
  Parser.SetPages(PageCount);
  Parser.SetNow();
}
//---------------------------------------------------------------------------
// override TTaeRichEditPrint::AfterPage() to recalculate page rects, if
// necesssary, set the parser page value, and draw headers and footers
//
void TTaeRichEditAdvPrint::AfterPage(HDC hdc, int page)
{
  // calculate drawing rectangles (note -- redundant if dimensions unchanged?)
  if (FNeedToCalcDrawRects)
  {
    CalcDrawRects(hdc);
    FNeedToCalcDrawRects = false;
  }

  // set the page value for headers/footers
  Parser.SetPage(page);

  // draw borders and header/footer
  if (page > 1 || !FUseFirstHeader)
  {
    DrawHeader(hdc, FHeader, DrawRects.HeaderRender);
    DrawHeader(hdc, FFooter, DrawRects.FooterRender);
    DrawBorders(hdc, DrawRects, FBorders);
  }
  else
  {
    DrawHeader(hdc, FFirstHeader, FirstDrawRects.HeaderRender);
    DrawHeader(hdc, FFirstFooter, FirstDrawRects.FooterRender);
    DrawBorders(hdc, FirstDrawRects, FBorders);
  }

  // call inherited
  TTaeRichEditPrint::AfterPage(hdc, page);
}
//---------------------------------------------------------------------------
// set PageStyle property
// 
void TTaeRichEditAdvPrint::SetPageStyle(TTaePageStyle& style)
{
  Header = style.FHeader;
  Footer = style.FFooter;
  FirstHeader = style.FFirstHeader;
  FirstFooter = style.FFirstFooter;
  UseFirstHeaderFooter = style.FDifferentFirstPage;
  Borders = style.FBorderLines;
  BorderWidth = ::MulDiv(style.FBorderWidth, 1440, 1000);
  InsideMargin = ::MulDiv(style.FInsideMargin, 1440, 1000);
}
//---------------------------------------------------------------------------
