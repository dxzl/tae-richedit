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
// TaeRichEditPrint.cpp - implementation file for TaeRichEditPrint.cpp
// (print and print preview support).
//---------------------------------------------------------------------------
// 20 * points = twips
// TWIPSPERINCH twips = 1 inch
// 120/points = pitch
// points / 12 = pica
// 1/72 inch = 1 point
// 96 DPI (dots per inch) = 1 pixel
// (can have also 120 DPI = 1.2 pixels and 144 DPI = 1.5 pixels!)
#include <vcl.h>
#pragma hdrstop

#include "TaeRichEditPrint.h"
#include "TaeRichEdit.h"
#include "TaeUnits.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// TTaeRichEditPrint constructor
//
__fastcall TTaeRichEditPrint::TTaeRichEditPrint(TTaeRichEdit* Owner) :
    FPageOffsets(), FOnBeforePage(0), FOnAfterPage(0),
      FOnStartPrint(0), FOnEndPrint(0)
{
  FInfo = GetInfo(); // Get printer's physical width and offsets, etc.

  FRendDC = Printer()->Handle;
  FRichEdit = Owner;
  FMargins.Left = 720;
  FMargins.Top = 720;
  FMargins.Right = 720;
  FMargins.Bottom = 720;

  // Scaling (fudge) factors...
  FScaleX = 9;
  FScaleY = 9;

  FRendRect = GetRendRect(FRendDC, FMargins, FRendWidth, FRendHeight);
  ::SetMapMode(FRendDC, MM_TEXT);

  FPrinterOffset.x = FPrinterOffset.y = 0;
  FPrinting = false;
  FShowMargins = false;
  FAbortPrint = false;
}
//---------------------------------------------------------------------------
// TTaeRichEditPrint destructor
//
__fastcall TTaeRichEditPrint::~TTaeRichEditPrint(void)
{
}
//---------------------------------------------------------------------------
// get printer's info into member vars
//
TTaeInfoDC __fastcall TTaeRichEditPrint::GetInfo(void)
{
  return GetInfo(Printer()->Handle);
}

TTaeInfoDC __fastcall TTaeRichEditPrint::GetInfo(HDC hdc)
{
  TTaeInfoDC info;

  // S.S. Note: .NET uses hundredths-of-an-inch units!
  info.xLogPix = ::GetDeviceCaps(hdc, LOGPIXELSX); // Pixels per inch (DPI)
  info.yLogPix = ::GetDeviceCaps(hdc, LOGPIXELSY);
  info.xMinMargin = ::GetDeviceCaps(hdc, PHYSICALOFFSETX);
  info.yMinMargin = ::GetDeviceCaps(hdc, PHYSICALOFFSETY);
  info.xPhysPage = ::GetDeviceCaps(hdc, PHYSICALWIDTH); // in twips
  info.yPhysPage = ::GetDeviceCaps(hdc, PHYSICALHEIGHT);

  return info;
}
//---------------------------------------------------------------------------
// set Margins property; note: may set margins to something other than requested
//
void __fastcall TTaeRichEditPrint::SetMargins(TRect margins)
{
  // set margins
  FMargins = margins;

  // recalc page and render rectangles
  CalcRects();

  // if wrapping to printer, force wrap to recalculate width
  if (FRichEdit->FWordWrapTo == wwtPrinter)
    FRichEdit->SetWordWrapTo(wwtPrinter);
}
//---------------------------------------------------------------------------
// set the rendering dc -- this is generally the display dc and it must
// remain valid indefinitely
//
// This is the property setter for RendDC which is set in TaePrintDlg.cpp
void __fastcall TTaeRichEditPrint::SetRendDC(HDC rendDC)
{
  if (rendDC == FRendDC)
    return;

  // if printing, restart
  bool printing = FPrinting;

  if (printing)
    EndRender(); // clears FPrinting flag

  FRendDC = rendDC;

  if (printing)
    BeginRender(MAXINT); // calls EndRender() then PaginateTo()
  else
  {
    FPageOffsets.erase(FPageOffsets.begin(), FPageOffsets.end());
    ClearRenderingCache();
    CalcRects();
  }
}
//---------------------------------------------------------------------------
// BeginRender() must be called before rendering to a printer or window;
// there must be a valid printer & rendering dc; generally the rendering
// dc is the screen dc -- dc must not be destroyed between BeginRender()
// and EndRender() calls
//
void TTaeRichEditPrint::BeginRender(int maxPages)
{
  // if already printing, end it before beginning again
  if (FPrinting)
    EndRender();

  // set flags
  FPrinting = true;

  // clear format info
  ClearRenderingCache();
  FPageOffsets.erase(FPageOffsets.begin(), FPageOffsets.end());

  // recalculate rectangles
  CalcRects();

  // call virtual StartPrint()
  StartPrint();

  // page to end (so count is valid) and save it
  PaginateTo(maxPages);
}
//---------------------------------------------------------------------------
// EndRender() must be called when rendering/printing is complete
//
void TTaeRichEditPrint::EndRender(void)
{
  // call virtual EndPrint()
  EndPrint();

  FPageOffsets.erase(FPageOffsets.begin(), FPageOffsets.end());
  ClearRenderingCache();
  FPrinting = false;
}
//---------------------------------------------------------------------------
// set flag to indicate need to draw a rectangle around the margin rectangle
//
void __fastcall TTaeRichEditPrint::SetShowMargins(bool showMargins)
{
  FShowMargins = showMargins;
}
//---------------------------------------------------------------------------
// get the selection range
//
CHARRANGE __fastcall TTaeRichEditPrint::GetSelRange(void)
{
  CHARRANGE chrg;
  ::SendMessage(FRichEdit->Handle, EM_EXGETSEL, 0, (LPARAM) &chrg);
  return chrg;
}
//---------------------------------------------------------------------------
// get the page number corresponding to the start of the selection range
//
int __fastcall TTaeRichEditPrint::GetSelStartPage(void)
{
  return GetOffsetPages(GetSelRange()).Start;
}
//---------------------------------------------------------------------------
// get the page number corresponding to the end of the selection range
//
int __fastcall TTaeRichEditPrint::GetSelEndPage(void)
{
  return GetOffsetPages(GetSelRange()).End;
}
//---------------------------------------------------------------------------
// get the pages corresponding to a range
//
TTaePageOffsets __fastcall TTaeRichEditPrint::GetOffsetPages(CHARRANGE chrg)
{
  return GetOffsetPages(chrg.cpMin, chrg.cpMax);
}
//---------------------------------------------------------------------------
// get the page count for render DC
//
int __fastcall TTaeRichEditPrint::GetPageCount(void)
{
  // Set page's FRendRect using printer's device-context
  CalcRects();

  // clear the page table and cached format info if not already doing printer
  FPageOffsets.erase(FPageOffsets.begin(), FPageOffsets.end());

  // format to infinity and save page count
  PaginateTo(MAXINT);
  int pages = FPageOffsets.size();

  // and return the page count
  return pages;
}
//---------------------------------------------------------------------------
// Get the page numbers corresponding to a selection range
// (First call GetPageCount() to populate FPageOffsets)
::TTaePageRange __fastcall TTaeRichEditPrint::GetOffsetPages(int begOffset,
                                                                int endOffset)
{
  TTaePageOffsets po;
  po.Start = po.End = -1;  // default invalid

  if (FPageOffsets.size() <= 0)
  {
    ShowMessage("GetOffsetPages(): Need to call GetPageCount() first!");
    return po;
  }

  // find selection...
  for (int i = 0; i < (int) FPageOffsets.size(); i++)
  {
    if (begOffset >= FPageOffsets[i].Start && begOffset <= FPageOffsets[i].End)
      po.Start = i;

    if (endOffset >= FPageOffsets[i].Start && endOffset <= FPageOffsets[i].End)
      po.End = i;

    if (po.Start != -1 && po.End != -1)
      break;
  }

  // handle returned -1s (negative ones)
  if (begOffset < 0)
    po.Start = 1;

  if (endOffset < 0)
    po.End = FPageOffsets.size() - 1;

  // return result
  return po;
}
//---------------------------------------------------------------------------
// paginate to a given page; used to paginate for a window only (not for
// actual printing)
//
bool TTaeRichEditPrint::PaginateTo(int page)
{
  // get # of pages already added to table; if enough, return ok;
  // if already past end of data, fail
  int currPage = FPageOffsets.size();
  if (currPage >= page) return true;

  // set up the starting page
  TTaePageOffsets po;
  if (currPage)
    po.Start = FPageOffsets[currPage-1].End + 1;
  else
    po.Start = 0;

  // initialize the formatting data
  TFormatRange fr;
  fr.chrg.cpMin = po.Start;
  fr.chrg.cpMax = po.End;

  GETTEXTLENGTHEX gtlx = { GTL_PRECISE, CP_ACP };
  int lastOffset = ::SendMessage(FRichEdit->Handle,
                              EM_GETTEXTLENGTHEX, (WPARAM) &gtlx, 0);
//  int lastOffset = FRichEdit->TextLength;

  // Note: hdcTarget contains the HDC to format for, which is usually the same
  // as the HDC specified by hdc but can be different. For example, if you
  // create a print preview module, hdc is the HDC of the window in which the
  // output is viewed, and hdcTarget is the HDC for the printer.

  fr.hdc = Printer()->Handle;
  fr.hdcTarget = 0;

  ClearRenderingCache();

  ::SaveDC(fr.hdc);
  ::SetMapMode(fr.hdc, MM_TEXT);

  // could/should use rendering page rect?
  // (S.S. tried that - made no difference...)
  fr.rcPage = FPageRect;

  int PrevEnd = -1;

  // loop through data to format and build page offset table through
  // requested page
  do
  {
    fr.rc = GetRendRect(fr.hdc, ++currPage);

    po.Start = fr.chrg.cpMin;

    fr.chrg.cpMin = FormatRange(fr, false); // measure only

    if (fr.chrg.cpMin != -1)
    {
      po.End = fr.chrg.cpMin-1;

      // This will filter out blank pages caused by a manual page break
      // coinciding with an automatic one...
      if (po.End > PrevEnd+1)
        FPageOffsets.push_back(po);

      PrevEnd = po.End;
    }

  } while (fr.chrg.cpMin >= 0 && fr.chrg.cpMin < lastOffset && currPage < page);

  // restore the dc
  ::RestoreDC(fr.hdc, -1);

  ClearRenderingCache();

  // return success if we made it to the requested page
  return (currPage == page) ? true : false;
}
//---------------------------------------------------------------------------
// compute # pages to print
//
/*
int __fastcall TTaeRichEditPrint::GetPrinterPageCount(void)
{
  std::vector<TPageOffsets> FPageOffsets;

  TPageOffsets po;
  po.Start = 0;

  TFormatRange fr;
  fr.chrg.cpMin = po.Start;
  fr.chrg.cpMax = -1;

  // Note: hdcTarget contains the HDC to format for, which is usually the same
  // as the HDC specified by hdc but can be different. For example, if you
  // create a print preview module, hdc is the HDC of the window in which the
  // output is viewed, and hdcTarget is the HDC for the printer.

  // An HDC for the device to render to, if EM_FORMATRANGE is being used to
  // send the output to a device.
  fr.hdc = Printer()->Handle;

  // An HDC for the target device to format for.
  fr.hdcTarget = 0;

  GETTEXTLENGTHEX gtlx = { GTL_PRECISE, CP_ACP };
  int lastOffset = ::SendMessage(FRichEdit->Handle, EM_GETTEXTLENGTHEX,
                                                            (WPARAM) &gtlx, 0);
//  int lastOffset = FRichEdit->TextLength;

  ClearRenderingCache();

  ::SaveDC(fr.hdc);
  ::SetMapMode(fr.hdc, MM_TEXT);

  int xOffset = -FInfo.xMinMargin + (TWIPSPERINCH*FInfo.xLogPix);
  int yOffset = -FInfo.yMinMargin + (TWIPSPERINCH*FInfo.yLogPix);

  ::SetViewportOrgEx(fr.hdc, xOffset, yOffset, NULL);

  // The entire area of a page on the rendering device.
  // Units are measured in twips.
  fr.rcPage = FPageRect;

  int PrevEnd = -1;
  int page = 0;

  do {
        // The area within the rcPage rectangle to render to.
        // Units are measured in twips.
        fr.rc = GetRendRect(fr.hdc, ++page);;

        po.Start = fr.chrg.cpMin;

        fr.chrg.cpMin = FormatRange(fr, false);

        if (fr.chrg.cpMin != -1)
        {
          po.End = fr.chrg.cpMin-1;

          // This will filter out blank pages caused by a manual page break
          // coinciding with an automatic one...
          if (po.End > PrevEnd+1)
            FPageOffsets.push_back(po);

          PrevEnd = po.End;
        }

  } while (fr.chrg.cpMin >= 0 && fr.chrg.cpMin < lastOffset);

  return FPageOffsets.size();
}
*/
//---------------------------------------------------------------------------
// PrintToPrinter() actually renders to the printer; this routine does NOT
// set the printing flag, etc., and caller should not call any TTaeRichEditPrint
// routines while this function is processing.  caller is responsible for
// supplying a print cancel dialog and calling Printer()->Abort() if
// the end-user cancels the print.  normally, the caller would gather all
// printing information, show his cancel dialog, disable all other forms,
// use the OnEndPrint event to close his cancel dialog, and use the
// OnBeforePage/OnAfterPage events to update any status information in
// the cancel dialog.  if this method is used, the only place where the
// caller could screw up is by calling some TTaeRichEditPrint function or
// property with side-effects from the cancel dialog....
//
// more notes:  I ran into an interesting problem... if I cancelled the
// printing from a cancel dialog invoked from the main form, the
// program would simply disappear (don't know if it closed or simply
// terminated).  no messages.  no nothing.  so I added a printer variable
// (ptpPrinter below) and initialized it to the state of FPrinter.
// now things seem to work as advertised.
//
// S.S. a good reference:
//        https://msdn.microsoft.com/en-us/library/ms996492.aspx
//
// Called from TaePrintDialog.cpp from the Execute method
bool TTaeRichEditPrint::PrintToPrinter(WideString spoolTitle, int startPage,
                                        int endPage, int copies, bool collate)
{
  if (FPageOffsets.size() <= 0)
  {
    ShowMessage("PrintToPrinter(): Need to call GetPageCount() first!");
    return true;
  }

  FInfo = GetInfo(); // refresh constants

  // S.S. I checked and this DOES call the CalcRects in TaeRichEditAdvPrint.cpp
  // (which is what we want because it will call its base CalcRects in this
  // file first then calc the rects for the header and footer.)

  // make sure layout is correct (was a problem if printed before previewed)
  CalcRects();

  // clear abort flag
  FAbortPrint = false;

  // set printer spool title and  set up to print
  Printer()->Title = spoolTitle; // Can't handle the WideString now! (S.S.)
  Printer()->BeginDoc(); // called before to ensure handle valid

// need to debug this idea (to go unicode) - look at end of this file for a way
// of getting a new dc to the printer... don't have time now S.S.

//  DOCINFOW di;
//  di.cbSize = sizeof(DOCINFOW);
//  di.lpszDocName = spoolTitle.c_bstr();
//  di.lpszOutput = NULL; // optional output file-name
  // DI_APPBANDING	Applications that use banding should set this flag
  // for optimal performance during printing.
  // DI_ROPS_READ_DESTINATION	The application will use raster operations that
  // involve reading from the destination surface.
//  di.lpszDatatype = 0;
//  ::StartDocW(Printer()->Handle, (DOCINFOW*)&di); // S.S. allows unicode

  bool nothingPrinted = true;
  int trips = 1;
  int dups = 1;

  // get the length of the text in the control (see treatise above)
  GETTEXTLENGTHEX gtlx = { GTL_PRECISE, CP_ACP };
  int lastOffset = ::SendMessage(FRichEdit->Handle,
                              EM_GETTEXTLENGTHEX, (WPARAM) &gtlx, 0);
//  int lastOffset = FRichEdit->TextLength;

  // if not collating, we'll simply print each page "copies" times;
  // otherwise, we have to do the whole dance "copies" times, printing
  // each page once on each pass
  if (collate)
    trips = copies;
  else
    dups = copies;

  TFormatRange fr;

  // Note: hdcTarget contains the HDC to format for, which is usually the same
  // as the HDC specified by hdc but can be different. For example, if you
  // create a print preview module, hdc is the HDC of the window in which the
  // output is viewed, and hdcTarget is the HDC for the printer.
  fr.hdc = Printer()->Handle;
  fr.hdcTarget = 0;

  fr.rcPage = FPageRect;

  ClearRenderingCache();

  // loop through the printing
  for (int i = 0; i < trips; i++)
  {
    // for this pass
    int page = 0;
    int currOffset = 0;

    // loop through each page in range
    do
    {
      // for each page...
      fr.chrg.cpMin = FPageOffsets[page].Start;
      fr.chrg.cpMax = FPageOffsets[page].End;
//      fr.chrg.cpMin = currOffset;
//      fr.chrg.cpMax = -1;

      if (++page > (int)FPageOffsets.size())
        break;

      // set up format range
      fr.rc = GetRendRect(fr.hdc, page);

      // if page is in range
      if (page >= startPage && page <= endPage)
      {
        // if this is the first page, tell printer to begin doc and
        // give caller a chance to do OnStartPrint stuff
        if (nothingPrinted)
        {
          nothingPrinted = false;
          StartPrint();
        }
        else
          Printer()->NewPage();

        // print each page "dups" times
        for (int i = 0; i < dups; i++)
        {
          // save the callers print dc and set to known state
          ::SaveDC(fr.hdc);
          ::SetMapMode(fr.hdc, MM_TEXT);  //???
          // apparently RE recognizes the unprintable area of
          // the page and simply moves everything???
          // S.S. with this commented out, the printed page is shifted down
          // 40 twips (around 3/4 line) and right two char-widths...
          ::SetViewportOrgEx(fr.hdc, -FPrinterOffset.x,
                                          -FPrinterOffset.y, NULL);

          // tell printer to do new page and give caller a chance
          // to do BeforePage() stuff -- save dc in case BeforePage()
          // screws it up
          BeforePage(fr.hdc, page);

          // render the page to the printer
          currOffset = FormatRange(fr, true);

          // restore the dc and give caller a chance to do
          // AfterPage() -- don't bother saving dc since we
          // don't care if AfterPage() screws it up
          AfterPage(fr.hdc, page);
          ::RestoreDC(fr.hdc, -1);
        }
      }
      // page was not in range so format it without rendering
      else
        currOffset = FormatRange(fr, false);

      // continue if more to do and not aborted
    } while (currOffset < lastOffset && page < endPage && !FAbortPrint);
  }

  // give caller a chance to handle end of printing and tell printer we're
  // through
  EndPrint();
  if (FAbortPrint || nothingPrinted)
  {
    if (!Printer()->Aborted)
      Printer()->Abort();
  }
  else
    Printer()->EndDoc();

  // reset page offsets & clear cached format info
  FPageOffsets.erase(FPageOffsets.begin(), FPageOffsets.end());
  ClearRenderingCache();

  // return failure if aborted
  return FAbortPrint;
}
//---------------------------------------------------------------------------
// render requested page to the passed dc (should be a window);
// paginate if needed
//
// S.S. This gets called from TPreviewFrm.cpp in YahCoLoRiZe, from the Paint()
// method of class PACKAGE TTaePreviewWindow : public TPanel. It also sets
// FRendWidth and FRendHeight (via properties) to the TPanel's ClientWidth
// and ClientHeight. GetRendRect then converts FRendWidth and FRendHeight to
// twips and returns a TRect with margins taken into account.
// FPageRect is set elsewhere by CalcRects(HDC hdc).
bool TTaeRichEditPrint::RenderPage(HDC hdc, int w, int h, int page)
{
  // sanity check
  if (page < 1) return false;

  FInfo = GetInfo(); // refresh constants for printer

  // S.S. added this - it's used in a lot of places and should be set
  // to our WYSIWYG window until we actually render to the printer
  SetRendDC(hdc);

  // try to get page; fail if does not exist
  if (!PaginateTo(page))
    return false;

  // get page offsets
  TTaePageOffsets po = FPageOffsets[page - 1];

  // initialize formatting info
  TFormatRange fr;

  // Note: hdcTarget contains the HDC to format for, which is usually the same
  // as the HDC specified by hdc but can be different. For example, if you
  // create a print preview module, hdc is the HDC of the window in which the
  // output is viewed, and hdcTarget is the HDC for the printer.
  fr.hdc = hdc;
  fr.hdcTarget = Printer()->Handle;

  // Calls overridden version in TaeRichEditAdvPrint.cpp!
  // fr.rc is the area within the rcPage rectangle to render to.
  // fr.rcPage is the entire area of a page on the rendering device.
  // Units are measured in twips.
  fr.rc = GetRendRect(fr.hdc, page); // 15120 twips
  fr.rcPage = FPageRect; // 15840 twips
// this works but so does the above - I suppose ::ScaleWindowExtEx (below)
// computes the same numbers regardless...
//  fr.rc = TwipsToRenderRect(GetRendRect(fr.hdc, page));
//  fr.rcPage = TwipsToTargetRect(fr.hdc, FPageRect);
  fr.chrg.cpMin = po.Start;
  fr.chrg.cpMax = po.End;

  // save the dc settings
  ::SaveDC(fr.hdc);

  // set the dcs to a known state
  ::SetMapMode(Printer()->Handle, MM_TEXT);    // needed?

  // Process_DPI_Unaware            = 0
  // Process_System_DPI_Aware       = 1
  // Process_Per_Monitor_DPI_Aware  = 2
// this may be needed (S.S.) but it's a real pain and only on win 7 and up!
// and it needs a different DLL than User32
//  PROCESS_DPI_AWARENESS dpi = { 2 };
//  ::SetProcessDpiAwareness(&dpi);

  // MM_TEXT Each logical unit is mapped to one device pixel. Positive x
  //  is to the right; positive y is down.
  //
  // MM_ANSIOTROPIC Logical units are mapped to arbitrary units with
  //  arbitrarily scaled axes. Use the SetWindowExtEx and SetViewportExtEx
  //  functions to specify the units, orientation, and scaling.
  //
  // S.S. this is what shrinks the text to fit into the viewport
  // (Can't OR the flags!)
  ::SetMapMode(fr.hdc, MM_TEXT);
  ::SetMapMode(fr.hdc, MM_ANISOTROPIC);

  // set window extent to width/height of printer
//ShowMessage("xPhysPage: " + String(FInfo.xPhysPage) + " yPhysPage:  " + String(FInfo.yPhysPage));
  // FInfo.xPhysPage = 5100, FInfo.yPhysPage = 6600 pixels (device units)
  ::SetWindowExtEx(fr.hdc, FInfo.xPhysPage, FInfo.yPhysPage, NULL);

  // Printer:
  //ShowMessage(String(FInfo.yPhysPage - info.yPhysPage)); // 6600 twips
  //ShowMessage(String(FInfo.yMinMargin)); // 40
  //ShowMessage(String(FInfo.yLogPix)); // 600
  //
  // Screen:
  //ShowMessage(String(info.yMinMargin)); // 0
  //ShowMessage(String(info.yLogPix)); // 96 pixels (dots) per inch
  //ShowMessage(String(GetDeviceCaps(hdc, VERTRES))); // 1200 height display adaptor
  //ShowMessage(String(GetDeviceCaps(hdc, HORZRES))); // 1600 width display-adaptor
  //ShowMessage(String(GetDeviceCaps(hdc, ASPECTY))); // 36 height of device pixel
  //ShowMessage(String(GetDeviceCaps(hdc, ASPECTX))); // 36 width of device pixel
  //ShowMessage(String(FRichEdit->FRtfTextFont->Height)); // -13
  //ShowMessage(String(FRichEdit->FRtfTextFont->Pitch)); // 2
  //ShowMessage(String(FRichEdit->FRtfTextFont->Size)); // 10 points
  //ShowMessage(String(GetDeviceCaps(Printer()->Handle, PHYSICALHEIGHT))); // 6600
  //ShowMessage(String(GetDeviceCaps(Printer()->Handle, VERTRES))); // 6260
  //ShowMessage(String(FPageRect.Bottom)); // 15840
//  int yFactor = MulDiv(GetDeviceCaps(hdc, VERTRES), FInfo.yLogPix,
//           GetDeviceCaps(Printer()->Handle, VERTRES)) - info.yLogPix;
  //ShowMessage(String(yFactor)); // 19

//  int xFactor = MulDiv(GetDeviceCaps(hdc, HORZRES), FInfo.xLogPix,
//           GetDeviceCaps(Printer()->Handle, HORZRES)) - info.xLogPix;
  //ShowMessage(String(xFactor)); // 104

//  ShowMessage(String(fr.rc.bottom)); //
//  ShowMessage(String(info.yPhysPage)); //

  TTaeInfoDC info = GetInfo(hdc);

  // scale to device units of passed hdc
//  ::ScaleWindowExtEx(fr.hdc, info.xLogPix, FInfo.xLogPix,
//                            info.yLogPix, FInfo.yLogPix, NULL);
  ::ScaleWindowExtEx(fr.hdc, info.xLogPix + FScaleX, FInfo.xLogPix,
                            info.yLogPix + FScaleY, FInfo.yLogPix, NULL);
//  FInfo.yPhysPage
//  ShowMessage(String(test.cy/(info.yLogPix + FScaleY)) + ", " + String(fr.rc.bottom/(info.yLogPix + FScaleY)));

  // set viewport to width/height of rendering area (previously set
  // by caller to match the drawing window)
  ::SetViewportExtEx(fr.hdc, w, h, NULL);

  // give user a chance to do stuff before page rendering
  // save/restore dc in case BeforePage() screws it up
  ::SaveDC(fr.hdc);
  BeforePage(fr.hdc, page);
  ::RestoreDC(fr.hdc, -1);

  // render the range
  FormatRange(fr, true);

  // S.S. do we need to ClearRenderingCache() to avoid memory leak???????
  ClearRenderingCache(); // seems not to hurt...

  // draw margins, if needed
  if (FShowMargins)
    DrawMargins(fr.hdc, page , FMargins);

  // give user a chance to do stuff after rendering page
  // don't save/restore dc because we don't care if AfterPage() screws it up
  AfterPage(fr.hdc /*FRendDC*/, page);

  // restore the dc to the caller's state
  ::RestoreDC(fr.hdc, -1);

  // and we made it
  return true;
}
//---------------------------------------------------------------------------
// override CalcRects() to set descendant header/footers.
// returns false if render rect does not actually match margins
// (if, for example, the requested margins are wider than the printer
// can handle).
//
// S.S. This IS overriden in TaeRichEditAdvPrint.cpp!
// This will SET FRendRect!
void TTaeRichEditPrint::CalcRects(void)
{
  // calculate the size of the printed page in twips
  FPageRect.Left = 0;
  FPageRect.Top = 0;
  FPageRect.Right = MulDiv(FInfo.xPhysPage, TWIPSPERINCH, FInfo.xLogPix);
  FPageRect.Bottom = MulDiv(FInfo.yPhysPage, TWIPSPERINCH, FInfo.yLogPix);

  // calculate printable area of page in twips (assume offsets are equally
  // spaced from each side)
  TRect printableRect;
  printableRect.Left = MulDiv(FInfo.xMinMargin, TWIPSPERINCH, FInfo.xLogPix);
  printableRect.Top = MulDiv(FInfo.yMinMargin, TWIPSPERINCH, FInfo.yLogPix);
  printableRect.Right = FPageRect.Right - printableRect.Left;
  printableRect.Bottom = FPageRect.Bottom - printableRect.Top;

  // calculate a margin rectangle in twips
  TRect margRect;
  margRect = FPageRect;
  margRect.Left += FMargins.Left;
  margRect.Top += FMargins.Top;
  margRect.Right -= FMargins.Right;
  margRect.Bottom -= FMargins.Bottom;

  // force margins to be within printable area
  margRect.Left = max(printableRect.Left, margRect.Left);
  margRect.Top = max(printableRect.Top, margRect.Top);
  margRect.Right = min(printableRect.Right, margRect.Right);
  margRect.Bottom = min(printableRect.Bottom, margRect.Bottom);

  // if margins leave < 1 inch, set back to printable page (sorry)
  if (margRect.Right - margRect.Left < TWIPSPERINCH ||
    margRect.Bottom - margRect.Top < TWIPSPERINCH)
    margRect = printableRect;

  // set margins to the new value
  FMargins.Left = margRect.Left;
  FMargins.Top = margRect.Top;
  FMargins.Right = FPageRect.Right - margRect.Right;
  FMargins.Bottom = FPageRect.Bottom - margRect.Bottom;

  // set the rendering rectangle to the margin rect
  FRendRect.Top = FMargins.Top;
  FRendRect.Left = FMargins.Left;
  FRendRect.Right = FPageRect.Right - FMargins.Right;
  FRendRect.Bottom = FPageRect.Bottom - FMargins.Bottom;

  // set the top/left printable point (not used internally?)
  FPrinterOffset.x = FInfo.xMinMargin;
  FPrinterOffset.y = FInfo.yMinMargin; // showing up as "40"
}
//---------------------------------------------------------------------------
// GetRendRect() is intended to be OVERRIDDEN by derived classes -- it should work
// like this: any derived class that changes the area actually that the
// TTaeRichEditPrint actually renders to should return the rendering area for
// the TTaeRichEditPrint control.  this allows, say, a derived class to print
// headers and footers by returning the smaller rect actually used by
// the TTaeRichEditPrint control here.  this virtual function (GetRenderRect) is
// called by every routine that actually needs the true rendering area for
// the TRichEdit control.  note that there is no way for SetWordWrapToPrinter()
// to handle pages of different widths so applications that require pages
// to have different rendering widths should not support SetWordWrapToPrinter().
//
// S.S. NOTE - This gets overridden in TaeRichEditAdvPrint.cpp!!!!!
TRect __fastcall TTaeRichEditPrint::GetRendRect(HDC hdc, int /*page*/)
{
  FRendRect = GetRendRect(hdc, FMargins, FRendWidth, FRendHeight);
  return FRendRect;
}
//---------------------------------------------------------------------------
// For my dell printer this returns PHYSICALHEIGHT = 6600 device units (pixels?)
// iHeight = 15840 twips
TRect __fastcall TTaeRichEditPrint::GetRendRect(HDC hdc, TRect rMargins,
                                                          int &w, int &h)
{
  TTaeInfoDC info = GetInfo(hdc);

  // Get width and height in twips
  w = ::MulDiv(info.xPhysPage, TWIPSPERINCH, info.xLogPix);
  h = ::MulDiv(info.yPhysPage, TWIPSPERINCH, info.yLogPix);

  TRect r;

  // Margins default to 720 twips for Left, Right Top and Bottom
  r.Left = rMargins.Left;
  r.Top = rMargins.Top;
  r.Right = w - rMargins.Right;
  r.Bottom = h - rMargins.Bottom; // 15094
  return r;
}
//---------------------------------------------------------------------------
// FormatRange() sends an EM_FORMATRANGE message (second version resets)
//
// Set render false if you just want to measure the text!
long int TTaeRichEditPrint::FormatRange(TFormatRange& formatRange, bool render)
{
  return ::SendMessage(FRichEdit->Handle, EM_FORMATRANGE, (WPARAM) render,
                                                    (LPARAM) &formatRange);
}
//---------------------------------------------------------------------------
// clear the Rich Edit rendering cache
//
void TTaeRichEditPrint::ClearRenderingCache(void)
{
  ::SendMessage(FRichEdit->Handle, EM_FORMATRANGE, (WPARAM) 0, (LPARAM) 0);
}
//---------------------------------------------------------------------------
// draw margins on a device context; note that the page parameter is not
// used by this implementation but that derived classes may use it
//
void TTaeRichEditPrint::DrawMargins(HDC hdc, int /*page*/, TRect margins)
{
  // save the dc
  ::SaveDC(hdc);

  // dc to known state (device units)
  ::SetMapMode(hdc, MM_TEXT);
  ::SetViewportOrgEx(hdc, 0, 0, NULL);

  // calculate the drawing rectangle in device units
  TRect r = margins;
  r.Right = FPageRect.Right - margins.Right;
  r.Bottom = FPageRect.Bottom - margins.Bottom;
  r = TwipsToRenderRect(r);

  // draw the rect
  ::DrawFocusRect(hdc, (RECT*) &r);

  // and restore the dc
  ::RestoreDC(hdc, -1);
}
//---------------------------------------------------------------------------
// these event handler callers are virtual and can be overridden
//
// BeforePage() - called before each page is rendered
//
void TTaeRichEditPrint::BeforePage(HDC hdc, int page)
{
  if (FOnBeforePage) (FOnBeforePage)(this, hdc, page);
}
//---------------------------------------------------------------------------
// AfterPage() - called after each page is rendered but before NewPage() is
// called
//
void TTaeRichEditPrint::AfterPage(HDC hdc, int page)
{
  if (FOnAfterPage) (FOnAfterPage)(this, hdc, page);
}
//---------------------------------------------------------------------------
// StartPrint() - called before the first page is rendered
//
void TTaeRichEditPrint::StartPrint(void)
{
  if (FOnStartPrint) (FOnStartPrint)(this);
}
//---------------------------------------------------------------------------
// EndPrint() - called after the last page is rendered
//
void TTaeRichEditPrint::EndPrint(void)
{
  if (FOnEndPrint) (FOnEndPrint)(this);
}
//---------------------------------------------------------------------------
// convert rectangle from twips to printer units
//
TRect TTaeRichEditPrint::TwipsToRenderRect(TRect rect)
{
  TRect r;
  r.Left = ::MulDiv(rect.Left, FRendWidth, FPageRect.Right);
  r.Top = ::MulDiv(rect.Top, FRendHeight, FPageRect.Bottom);
  r.Right = ::MulDiv(rect.Right, FRendWidth, FPageRect.Right);
  r.Bottom = ::MulDiv(rect.Bottom, FRendHeight, FPageRect.Bottom);

  return r;
}
//---------------------------------------------------------------------------
// convert rectangle from twips to target device units
//
TRect TTaeRichEditPrint::TwipsToTargetRect(HDC hdc, TRect rect)
{
  TTaeInfoDC info = GetInfo(hdc);
  TRect r;
  r.Left = ::MulDiv(rect.Left, info.xLogPix, TWIPSPERINCH);
  r.Top = ::MulDiv(rect.Top, info.yLogPix, TWIPSPERINCH);
  r.Right = ::MulDiv(rect.Right, info.xLogPix, TWIPSPERINCH);
  r.Bottom = ::MulDiv(rect.Bottom, info.yLogPix, TWIPSPERINCH);

  return r;
}
//---------------------------------------------------------------------------
// S.S. useful for creating a new printer DC independant of
// the one the VCL creates...
/*
HDC __fastcall TTaeRichEditPrint::CreatePrinterDC(void)
{
    int printerIndex = Printer()->PrinterIndex;

    if (printerIndex < 0)
      Printer()->PrinterIndex = -1; // set to default

    printerIndex = Printer()->PrinterIndex;

    if (printerIndex < 0)
      return 0; // give up

    HDC hdcPrint;
    HANDLE  hPrinter = NULL;
    PRINTER_INFO_2  *pPrinterData;
    BYTE  pdBuffer[16384];

    DWORD  cbBuf = sizeof (pdBuffer);
    DWORD  cbNeeded = 0;
    pPrinterData = (PRINTER_INFO_2 *)&pdBuffer[0];

    // get the default printer name
    String sPrinter = Printer()->Printers->Strings[printerIndex];

    if (sPrinter.IsEmpty()) return 0;

    // open the default printer and get hPrinter
    if (!OpenPrinter(sPrinter.c_str(), &hPrinter, NULL)) return 0;

    // get the printer port name
    bool bRet = GetPrinter(hPrinter, 2, &pdBuffer[0], cbBuf, &cbNeeded);

    // this handle is no longer needed
    ClosePrinter(hPrinter);

    if (!bRet) return 0;

    // create the Print DC
    hdcPrint = CreateDC("WINSPOOL", sPrinter.c_str(),
                             pPrinterData->pPortName, NULL);

//    if (hdcPrint)
//    {
//        // Print a test page that contains the string
//        // "PRINTER TEST" in the upper left corner.
//
//        Escape(hdcPrint, STARTDOC, 8, "Test-Doc", NULL);
//        TextOut(hdcPrint, 50, 50, _T("PRINTER TEST"), 12);
//        Escape(hdcPrint, NEWFRAME, 0, NULL, NULL);
//        Escape(hdcPrint, ENDDOC, 0, NULL, NULL);

        // Delete the printer DC.
//        DeleteDC(hdcPrint);
//    }

    return hdcPrint;
}
*/
//---------------------------------------------------------------------------

