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
// TaePageSetupDlg.h - header file for TTaePageSetupDialog class.
//---------------------------------------------------------------------------
#ifndef pagesetupdlgH
#define pagesetupdlgH

#include <vcl\SysUtils.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\printers.hpp>
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit;

typedef enum { psoDefaultMinMargins, psoDisableMargins, psoDisableOrientation,
  psoDisablePagePainting, psoDisablePaper, psoDisablePrinter, psoMargins,
  psoMinMargins, psoNoWarning, psoReturnDefault, psoShowHelp
  } TTaePageSetupDialogOptions;

typedef Set<TTaePageSetupDialogOptions, psoDefaultMinMargins, psoShowHelp> TTaePageSetupOptionsSet;

typedef enum { psuMMs100ths, psuInches1000ths } TTaePageSetupDialogUnits;

class PACKAGE TTaePageSetupRect : public TPersistent
{
private:
protected:
  TRect r;

    __fastcall int GetLeft(void) { return r.Left; };
  __fastcall void SetLeft(int left) { r.Left = left; };
  __fastcall int GetTop(void) { return r.Top; };
  __fastcall void SetTop(int top) { r.Top = top; };
  __fastcall int GetRight(void) { return r.Right; };
  __fastcall void SetRight(int right) { r.Right = right; };
  __fastcall int GetBottom(void) { return r.Bottom; };
  __fastcall void SetBottom(int bottom) { r.Bottom = bottom; };

public:
  __fastcall TTaePageSetupRect(TComponent* Owner) { r.Left = 0; Top = 0; Right = 0; Bottom = 0; };
  __fastcall TTaePageSetupRect(TRect& rect) { r = rect; };
  __property TRect Rect = { read = r, write = r, nodefault };

__published:
  __property int Left = { read = GetLeft, write = SetLeft, default = 0 };
  __property int Top = { read = GetTop, write = SetTop, default = 0 };
  __property int Right = { read = GetRight, write = SetRight, default = 0 };
  __property int Bottom = { read = GetBottom, write = SetBottom, default = 0 };
};

class PACKAGE TTaePageSetupPoint : public TPersistent
{
private:
protected:
  TPoint p;

public:
  __fastcall TTaePageSetupPoint(TComponent* Owner) { p.x = p.y = 0; };
  __property TPoint Point = { read = p, write = p, nodefault };

__published:
  __property long int Width = { read = p.x, write = p.x, default = 0 };
  __property long int Height = { read = p.y, write = p.y, default = 0 };
};

class PACKAGE TTaePageSetupDialog : public TCommonDialog
{
private:
protected:
  TTaePageSetupPoint* FPaperSize;
  TTaePageSetupRect* FMinMargins;
  TTaePageSetupRect* FMargins;
  TTaePageSetupOptionsSet FOptions;
  TTaePageSetupDialogUnits FUnits;
  TPrinter* FPrinter;

  int __fastcall GetOptionFlags(void);
  void __fastcall SetOptionFlags(int flags);
    bool __fastcall Execute(void) { return false; };  // hide this declaration

public:
  __fastcall TTaePageSetupDialog(TComponent* Owner);
  __fastcall ~TTaePageSetupDialog();
  bool __fastcall Execute(TTaeRichEdit* taeRichEdit);
  __property TPrinter* TargetPrinter = { read = FPrinter, write = FPrinter,
    nodefault };
  __property int OptionFlags = { read = GetOptionFlags, write = SetOptionFlags };
  __property TTaePageSetupPoint* PaperSize = { read = FPaperSize };

__published:
  __property TTaePageSetupDialogUnits Units = { read = FUnits, write = FUnits,
    default = psuInches1000ths };
  __property TTaePageSetupRect* MinMargins = { read = FMinMargins, write = MinMargins };
//  __property TTaePageSetupRect* Margins = { read = FMargins, write = FMargins };
  __property TTaePageSetupOptionsSet Options = { read = FOptions, write = FOptions };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
