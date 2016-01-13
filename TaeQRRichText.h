// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//---------------------------------------------------------------------------
// this file implements the TaeRichEdit Component for use with Quick Reports.
// it was contributed by Herpai István [hi@si.hu].  Many thanks, Herpai!
//---------------------------------------------------------------------------
#ifndef TaeQRRichTextH
#define TaeQRRichTextH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <QuickRpt.hpp>
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
class PACKAGE TTaeQRRichText : public TQRPrintable
{
typedef Quickrpt::TQRPrintable inherited;

private:
  int LastChar;
  bool FAutoStretch;
  TTaeRichEdit* FParentRichEdit;
  TTaeRichEdit* FRichEdit;
  Classes::TAlignment __fastcall GetAlignment(void);
  Graphics::TColor __fastcall GetColor(void);
  Graphics::TFont* __fastcall GetFont(void);
  Classes::TStrings* __fastcall GetLines(void);
  virtual void __fastcall SetAlignment(Classes::TAlignment Value);
  HIDESBASE void __fastcall SetColor(Graphics::TColor Value);
  HIDESBASE void __fastcall SetFont(Graphics::TFont* Value);
  void __fastcall SetLines(Classes::TStrings* Value);
  void __fastcall SetParentRichEdit(TTaeRichEdit* Value);

protected:
  virtual void __fastcall Print(int OfsX, int OfsY);
  virtual void __fastcall SetBounds(int ALeft, int ATop, int AWidth, int AHeight);

public:
  __fastcall TTaeQRRichText(TComponent* Owner);
  __fastcall ~TTaeQRRichText();

__published:
  __property Classes::TAlignment Alignment = {read=GetAlignment, write=SetAlignment, nodefault};
  __property bool AutoStretch = {read=FAutoStretch, write=FAutoStretch, nodefault};
  __property Graphics::TColor Color = {read=GetColor, write=SetColor, nodefault};
  __property Graphics::TFont* Font = {read=GetFont, write=SetFont};
  __property Classes::TStrings* Lines = {read=GetLines, write=SetLines};
  __property TTaeRichEdit* ParentRichEdit = {read=FParentRichEdit, write=SetParentRichEdit};
};
//---------------------------------------------------------------------------
#endif
