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
// TaePrintCancelDlg.h - header file for TaePrintCancelDlg.cpp (sample print
// cancel dialog).
//---------------------------------------------------------------------------
#ifndef TaePrintCancelDlgH
#define TaePrintCancelDlgH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>

#include <vcl\printers.hpp>
#include <vcl\ExtCtrls.hpp>
//---------------------------------------------------------------------------
class PACKAGE TTaePrintCancelDialog : public TForm
{
__published:  // IDE-managed Components
  TProgressBar *Gauge;
  TButton *Cancel;
  TPanel *Panel1;
  TLabel *Label9;
  TLabel *Label10;
  TLabel *Label11;
  TLabel *Label12;
  TLabel *Label13;
  TLabel *Label14;
  TLabel *DocNameLbl;
  TLabel *PrinterNameLbl;
  TLabel *FormInfoLbl;
  TLabel *OrientationLbl;
  TLabel *PrintRangeLbl;
  TLabel *PagesPrintedLbl;
  TLabel *Label21;
  TLabel *PagesToPrintLbl;
  TLabel *Label1;
  TLabel *CopiesLbl;
  TLabel *Label2;
  TLabel *DocFileLbl;
  TLabel *Label3;
  TLabel *CollateLbl;
  void __fastcall CancelClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);

private:  // User declarations

protected:
  AnsiString GetDocName(void) { return DocNameLbl->Caption; };
  void SetDocName(AnsiString name) { DocNameLbl->Caption = name; };
  AnsiString GetDocFile(void) { return DocFileLbl->Caption; };
  void SetDocFile(AnsiString name) { DocFileLbl->Caption = name; };
  AnsiString GetFormInfo(void) { return FormInfoLbl->Caption; };
  void SetFormInfo(AnsiString info) { FormInfoLbl->Caption = info; };
  AnsiString GetPrintRange(void) { return PrintRangeLbl->Caption; };
  void SetPrintRange(AnsiString range) { PrintRangeLbl->Caption = range; };
  int FPagesPrinted;
  int FPagesToPrint;
  TPrinter* FTargetPrinter;
  void SetPagesPrinted(int pages) {
    FPagesPrinted = pages;
    PagesPrintedLbl->Caption = AnsiString(pages);
    Gauge->Position = (TProgressRange) (pages / FPagesToPrint);
    };
  void SetPagesToPrint(int pages) {
    if (pages < 1) pages = 1;
    FPagesToPrint = pages;
    PagesToPrintLbl->Caption = AnsiString(pages);
    Gauge->Max = (TProgressRange) pages;
    };
  TNotifyEvent FOnAbortPrint;

public:    // User declarations
  __fastcall TTaePrintCancelDialog(TComponent* Owner);
  __property AnsiString DocName = { read = GetDocName, write = SetDocName };
  __property AnsiString DocFile = { read = GetDocFile, write = SetDocFile };
  __property AnsiString FormInfo = { read = GetFormInfo, write = SetFormInfo };
  __property AnsiString PrintRange = { read = GetPrintRange, write = SetPrintRange };
  __property int PagesPrinted = { read = FPagesPrinted, write = SetPagesPrinted };
  __property int PagesToPrint = { read = FPagesToPrint, write = SetPagesToPrint };

  __property TPrinter* TargetPrinter = { read = FTargetPrinter, write = FTargetPrinter };
  __property TNotifyEvent OnAbortPrint = { read = FOnAbortPrint, write = FOnAbortPrint, nodefault };
};  
//---------------------------------------------------------------------------
extern PACKAGE TTaePrintCancelDialog *TaePrintCancelDialog;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
