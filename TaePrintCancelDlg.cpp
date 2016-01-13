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
// TaePrintCancelDlg.cpp - implementation file for TaePrintCancelDlg.cpp
// (sample print cancel dialog).  note that this dialog (form) is not yet
// complete -- there are elements that are essentially placeholders for
// future revisions.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaePrintCancelDlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTaePrintCancelDialog* TaePrintCancelDialog;
//---------------------------------------------------------------------------
__fastcall TTaePrintCancelDialog::TTaePrintCancelDialog(TComponent* Owner)
  : TForm(Owner)
{
  FOnAbortPrint = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTaePrintCancelDialog::CancelClick(TObject *Sender)
{
  if (OnAbortPrint)(OnAbortPrint)(this);
}
//---------------------------------------------------------------------------
void __fastcall TTaePrintCancelDialog::FormShow(TObject *Sender)
{
  DocNameLbl->Caption = TargetPrinter->Title;
  PrinterNameLbl->Caption = Printer()->Printers->Strings[Printer()->PrinterIndex];
  OrientationLbl->Caption = Printer()->Orientation == poPortrait ?
    "Portrait" : "Landscape";
  CopiesLbl->Caption = AnsiString(Printer()->Copies);
  PagesPrinted = 0; // use property to update progress bar...
}
//---------------------------------------------------------------------------
