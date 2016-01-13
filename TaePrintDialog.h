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
// TaePrintDialog.h - header file for TTaeRichEdit (class-specific print dialog).
//---------------------------------------------------------------------------
#ifndef TaePrintDialogH
#define TaePrintDialogH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>

#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
// declaration for the TOnPageEvent type
typedef void __fastcall (__closure *TOnPageEvent)(TObject* Sender, HDC hdc,
  int page);

// forward declaration for the TTaePrintCancelDialog class
class PACKAGE TTaePrintCancelDialog;

// TTaePrintDialog class declaration -- this is a specialization of the
// VCL TPrintDialog component that initializes the print dialog with page
// ranges (start and end pages) that are correct for the associated
// TTaeRichEdit control
//
class PACKAGE TTaePrintDialog : public TPrintDialog
{
  private:
    WideString FTitle;

protected:
  TTaeRichEdit* FRichEdit;
  TTaeRichEdit::TTaePrint* FRichEditPrint;
  TOnPageEvent FOnAfterPage;
  TNotifyEvent FOnEndPrint;
  TTaePrintCancelDialog* FPrintCancelDlg;

  void __fastcall PageEnded(TObject* Sender, HDC hdc, int page);
  void __fastcall PrintingEnded(TObject* Sender);
  void __fastcall AbortPrint(TObject* Sender);
  virtual void __fastcall Notification(TComponent* AComponent,
    TOperation Operation);

public:
  __fastcall TTaePrintDialog(TTaeRichEdit* Owner);
  __fastcall ~TTaePrintDialog(void);
  virtual bool __fastcall TaeExecute(WideString wTitle = "");

__published:
  __property TTaeRichEdit* TaeRichEdit = { read = FRichEdit, write = FRichEdit,
    default = 0 };
  __property WideString Title = { read = FTitle, write = FTitle };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
