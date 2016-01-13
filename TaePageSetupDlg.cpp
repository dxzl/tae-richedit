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
// TaePageSetupDlg.cpp - implementation file for TTaePageSetupDialog class.
//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "TaePageSetupDlg.h"
#include "TaeRichEdit.h"
#include "TaeRichEditAdvPrint.h"
#include "TaeUnits.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
static inline TTaePageSetupDialog *ValidCtrCheck()
{
  return new TTaePageSetupDialog(NULL);
}
//---------------------------------------------------------------------------
__fastcall TTaePageSetupDialog::TTaePageSetupDialog(TComponent* Owner)
  : TCommonDialog(Owner)
{
  FPaperSize = new TTaePageSetupPoint(Owner);
  FMinMargins = new TTaePageSetupRect(Owner);
  FMargins = new TTaePageSetupRect(Owner);
  FPrinter = Printer();
  FUnits = psuInches1000ths;
  FOptions.Clear();
  FOptions << psoDefaultMinMargins << psoMargins;
}
//---------------------------------------------------------------------------
__fastcall TTaePageSetupDialog::~TTaePageSetupDialog()
{
  // release dynamic allocations
  delete FPaperSize;
  delete FMinMargins;
  delete FMargins;
}
//---------------------------------------------------------------------------
namespace Taepagesetupdlg
{
  void __fastcall PACKAGE Register()
  {
    TComponentClass classes[1] = {__classid(TTaePageSetupDialog)};
    RegisterComponents("TaeRichEdit", classes, 0);
  }
}
//---------------------------------------------------------------------------
bool __fastcall TTaePageSetupDialog::Execute(TTaeRichEdit* taeRichEdit)
{
  TPageSetupDlg psd;
  char device[255];
  char driver[255];
  char port[255];
  unsigned int devmode;  // help file is wrong (it says "int")

  // quick out if no rich edit
  if (!taeRichEdit || !taeRichEdit->TaePrint)
    return false;

  // set margins to values from rich edit
  if (Units == psuInches1000ths)
  {
    FMargins->Left = TwipsToInches(taeRichEdit->TaePrint->Margins.Left);
    FMargins->Top = TwipsToInches(taeRichEdit->TaePrint->Margins.Top);
    FMargins->Right = TwipsToInches(taeRichEdit->TaePrint->Margins.Right);
    FMargins->Bottom = TwipsToInches(taeRichEdit->TaePrint->Margins.Bottom);
//    FMargins->Left =
//      (TwipsToInches(taeRichEdit->TaePrint->Margins.Left) * 1000.0) + 0.5;
//    FMargins->Top =
//      (TwipsToInches(taeRichEdit->TaePrint->Margins.Top) * 1000.0) + 0.5;
//    FMargins->Right =
//      (TwipsToInches(taeRichEdit->TaePrint->Margins.Right) * 1000.0) + 0.5;
//    FMargins->Bottom =
//      (TwipsToInches(taeRichEdit->TaePrint->Margins.Bottom) * 1000.0) + 0.5;
  }
  else
  {
    FMargins->Left = TwipsToMMs(taeRichEdit->TaePrint->Margins.Left);
    FMargins->Top = TwipsToMMs(taeRichEdit->TaePrint->Margins.Top);
    FMargins->Right = TwipsToMMs(taeRichEdit->TaePrint->Margins.Right);
    FMargins->Bottom = TwipsToMMs(taeRichEdit->TaePrint->Margins.Bottom);
//    FMargins->Left =
//      (TwipsToMMs(taeRichEdit->TaePrint->Margins.Left) * 100.0) + 0.5;
//    FMargins->Top =
//      (TwipsToMMs(taeRichEdit->TaePrint->Margins.Top) * 100.0) + 0.5;
//    FMargins->Right =
//      (TwipsToMMs(taeRichEdit->TaePrint->Margins.Right) * 100.0) + 0.5;
//    FMargins->Bottom =
//      (TwipsToMMs(taeRichEdit->TaePrint->Margins.Bottom) * 100.0) + 0.5;
  }


  // get printer information
  FPrinter->GetPrinter(device, driver, port, devmode);

  // initialize the print setup dialog structure
  psd.lStructSize = sizeof(psd);
  psd.hwndOwner = Application->MainForm->Handle;
  psd.hDevMode = (HGLOBAL) devmode;
  psd.hDevNames = 0;
  psd.Flags = OptionFlags;
  psd.rtMinMargin = FMinMargins->Rect;
  psd.ptPaperSize = FPaperSize->Point;
  psd.rtMargin = FMargins->Rect;
  psd.hInstance = 0;
  psd.lCustData = 0;
  psd.lpfnPageSetupHook = 0;
  psd.lpfnPagePaintHook = 0;
  psd.lpPageSetupTemplateName = 0;
  psd.hPageSetupTemplate = 0;


  // execute the dialog (returns false on cancel)
  if (!PageSetupDlg(&psd)) return false;

  // get the device, driver, and port from the returned data
  DEVNAMES* dn = 0;
  if (psd.hDevNames) dn = (DEVNAMES*) ::GlobalLock(psd.hDevNames);
  char *pdriver = (char*) ((char*) dn + dn->wDriverOffset);
  char *pdevice = (char*) ((char*) dn + dn->wDeviceOffset);
  char *pport = (char*) ((char*) dn + dn->wOutputOffset);

  // set the printer to the device, driver, port, and device mode returned
  FPrinter->SetPrinter(pdevice, pdriver, pport, (int) psd.hDevMode);

  // TPrinter takes care of freeing the hDevMode global memory (or
  // so I believe and very much hope) but we still have to free
  // the device names memory
  ::GlobalUnlock(psd.hDevNames);
  ::GlobalFree(psd.hDevNames);

  // save the returned values back into the properties
  OptionFlags = psd.Flags;
  FMinMargins->Rect = psd.rtMinMargin;
  FPaperSize->Point = psd.ptPaperSize;
  FMargins->Rect = psd.rtMargin;

  // set the rich edit margins
  if (Units == psuInches1000ths)
  {
    taeRichEdit->TaePrint->Margins.Left = InchesToTwips(FMargins->Left);
    taeRichEdit->TaePrint->Margins.Top = InchesToTwips(FMargins->Top);
    taeRichEdit->TaePrint->Margins.Right = InchesToTwips(FMargins->Right);
    taeRichEdit->TaePrint->Margins.Bottom = InchesToTwips(FMargins->Bottom);
//    taeRichEdit->TaePrint->Margins.Left =
//      (InchesToTwips(FMargins->Left) / 1000.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Top =
//      (InchesToTwips(FMargins->Top) / 1000.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Right =
//      (InchesToTwips(FMargins->Right) / 1000.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Bottom =
//      (InchesToTwips(FMargins->Bottom) / 1000.0) + 0.5;
  }
  else
  {
    taeRichEdit->TaePrint->Margins.Left = MMsToTwips(FMargins->Left);
    taeRichEdit->TaePrint->Margins.Top = MMsToTwips(FMargins->Top);
    taeRichEdit->TaePrint->Margins.Right = MMsToTwips(FMargins->Right);
    taeRichEdit->TaePrint->Margins.Bottom = MMsToTwips(FMargins->Bottom);
//    taeRichEdit->TaePrint->Margins.Left =
//      (MMsToTwips(FMargins->Left) / 100.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Top =
//      (MMsToTwips(FMargins->Top) / 100.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Right =
//      (MMsToTwips(FMargins->Right) / 100.0) + 0.5;
//    taeRichEdit->TaePrint->Margins.Bottom =
//      (MMsToTwips(FMargins->Bottom) / 100.0) + 0.5;
  }

  // return success
  return true;
}
//---------------------------------------------------------------------------
int __fastcall TTaePageSetupDialog::GetOptionFlags(void)
{
  int flags = 0;
  if (FOptions.Contains(psoDefaultMinMargins)) flags |= PSD_DEFAULTMINMARGINS;
  if (FOptions.Contains(psoDisableMargins)) flags |= PSD_DISABLEMARGINS;
  if (FOptions.Contains(psoDisableOrientation)) flags |= PSD_DISABLEORIENTATION;
  if (FOptions.Contains(psoDisablePagePainting)) flags |= PSD_DISABLEPAGEPAINTING;
  if (FOptions.Contains(psoDisablePaper)) flags |= PSD_DISABLEPAPER;
  if (FOptions.Contains(psoDisablePrinter)) flags |= PSD_DISABLEPRINTER;
  if (FOptions.Contains(psoMargins)) flags |= PSD_MARGINS;
  if (FOptions.Contains(psoMinMargins)) flags |= PSD_MINMARGINS;
  if (FOptions.Contains(psoNoWarning)) flags |= PSD_NOWARNING;
  if (FOptions.Contains(psoReturnDefault)) flags |= PSD_RETURNDEFAULT;
  if (FOptions.Contains(psoShowHelp)) flags |= PSD_SHOWHELP;
  if (FUnits == psuMMs100ths) flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
  if (FUnits == psuInches1000ths) flags |= PSD_INTHOUSANDTHSOFINCHES;
  return flags;
}
//---------------------------------------------------------------------------
void __fastcall TTaePageSetupDialog::SetOptionFlags(int flags)
{
  FOptions.Clear();
  FUnits = psuInches1000ths;
  if (flags & PSD_DEFAULTMINMARGINS) FOptions << psoDefaultMinMargins;
  if (flags & PSD_DISABLEMARGINS) FOptions << psoDisableMargins;
  if (flags & PSD_DISABLEORIENTATION) FOptions << psoDisableOrientation;
  if (flags & PSD_DISABLEPAGEPAINTING) FOptions << psoDisablePagePainting;
  if (flags & PSD_DISABLEPAPER) FOptions << psoDisablePaper;
  if (flags & PSD_DISABLEPRINTER) FOptions << psoDisablePrinter;
  if (flags & PSD_MARGINS) FOptions << psoMargins;
  if (flags & PSD_MINMARGINS) FOptions << psoMinMargins;
  if (flags & PSD_NOWARNING) FOptions << psoNoWarning;
  if (flags & PSD_RETURNDEFAULT) FOptions << psoReturnDefault;
  if (flags & PSD_SHOWHELP) FOptions << psoShowHelp;
  if (flags & PSD_INHUNDREDTHSOFMILLIMETERS) FUnits = psuMMs100ths;
  if (flags & PSD_INTHOUSANDTHSOFINCHES) FUnits = psuInches1000ths;
}
//---------------------------------------------------------------------------
