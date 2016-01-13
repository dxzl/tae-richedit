// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//---------------------------------------------------------------------------
// this file implements the TaeRichEdit Component for use with Quick Reports.
// it was contributed by Herpai István [hi@si.hu].  Many thanks, Herpai!
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeQRRichText.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TTaeQRRichText *)
{
  new TTaeQRRichText(NULL);
}
//---------------------------------------------------------------------------
namespace Taeqrrichtext
{
  void __fastcall PACKAGE Register()
  {
     TComponentClass classes[1] = {__classid(TTaeQRRichText)};
     RegisterComponents("Thin Air", classes, 0);
  }
}
//---------------------------------------------------------------------------
__fastcall TTaeQRRichText::TTaeQRRichText(TComponent* Owner) : inherited(Owner){
    FRichEdit=new TTaeRichEdit(this);
    FRichEdit->Parent=this;
    FRichEdit->BorderStyle=Forms::bsNone;
    AutoStretch=false;
    PrintFinished=true;
    Width=100;
    Height=100;
}
__fastcall TTaeQRRichText::~TTaeQRRichText(){
    delete FRichEdit;
}
Classes:: TAlignment __fastcall TTaeQRRichText::GetAlignment(){return FRichEdit->Alignment;}
Graphics::TColor     __fastcall TTaeQRRichText::GetColor()    {return FRichEdit->Color;}
Graphics::TFont*     __fastcall TTaeQRRichText::GetFont()     {return FRichEdit->Font;}
Classes:: TStrings*  __fastcall TTaeQRRichText::GetLines()    {return FRichEdit->Lines;}
void __fastcall TTaeQRRichText::SetAlignment(Classes::TAlignment Value){FRichEdit->Alignment=Value;}
void __fastcall TTaeQRRichText::SetColor    (Graphics::TColor Value){   FRichEdit->Color    =Value;}
void __fastcall TTaeQRRichText::SetFont     (Graphics::TFont* Value){   FRichEdit->Font     =(TFont2*)Value;}
void __fastcall TTaeQRRichText::SetParentRichEdit(TTaeRichEdit* Value){
    FParentRichEdit=Value;
    if(Value!=NULL) FRichEdit->Lines=Value->Lines;
}
void __fastcall TTaeQRRichText::SetLines(Classes::TStrings* Value){
    FRichEdit->Lines=Value;
    if(FParentRichEdit!=NULL) FParentRichEdit->Lines=Value;
}
void __fastcall TTaeQRRichText::SetBounds(int ALeft, int ATop, int AWidth, int AHeight){
    inherited::SetBounds(ALeft, ATop, AWidth, AHeight);
    if(FRichEdit!=NULL) FRichEdit->SetBounds(1, 1, AWidth - 2, AHeight - 2);
}
void __fastcall TTaeQRRichText::Print(int OfsX, int OfsY){
    TFormatRange        Range;
    int                 LogX,LogY,TextLength,OldMapMode;
    bool                HasExpanded;
    TTaeRichEdit*       ARichEdit;
    Extended            Expanded;

    ARichEdit=ParentRichEdit==NULL?FRichEdit:ParentRichEdit;
    {
        register i=sizeof(TFormatRange);
        register char *c=(char *) &Range;
        while(i--) *c++=0;
    }
    Range.hdc= ParentReport->QRPrinter->Canvas->Handle;
    Range.hdcTarget= Range.hdc;
    LogX= GetDeviceCaps(Range.hdc, LOGPIXELSX);
    LogY= GetDeviceCaps(Range.hdc, LOGPIXELSY);
    Range.rc=Rect(QRPrinter->XPos(OfsX + Size->Left) * 1440 / LogX,
                  QRPrinter->YPos(OfsY + Size->Top) * 1440 / LogY,
                  QRPrinter->XPos(OfsX + Size->Width + Size->Left) * 1440 / LogX,
                  QRPrinter->YPos(OfsY + Size->Height + Size->Top) * 1440 / LogY);
    Range.rcPage=Rect(0,0,
                      QRPrinter->XSize(QRPrinter->PaperWidth) * 1440 / LogX,
                      QRPrinter->YSize(QRPrinter->PaperLength) * 1440 / LogY);

    if(PrintFinished) LastChar= 0;
    HasExpanded=false;
    Expanded=0;
    TextLength=ARichEdit->GetTextLen();
    Range.chrg.cpMax= -1;
    Range.chrg.cpMin= LastChar;
    OldMapMode= SetMapMode(Range.hdc, MM_TEXT);

    SendMessage(ARichEdit->Handle, EM_FORMATRANGE,0,0);

    LastChar= SendMessage(ARichEdit->Handle, EM_FORMATRANGE, 0, (long)&Range);
    if(LastChar<TextLength&&AutoStretch&&TextLength>0&&Parent->InheritsFrom(__classid(TQRCustomBand))){
        PrintFinished=false;
        while(LastChar<=TextLength&&((TQRCustomBand*)Parent)->CanExpand(50)){
          ((TQRCustomBand*)Parent)->ExpandBand(50,Expanded,HasExpanded);
          Range.rc.bottom = QRPrinter->YPos(OfsY+Size->Top+Size->Height+Expanded) * 1440 / LogY;
          LastChar= SendMessage(ARichEdit->Handle, EM_FORMATRANGE, 0, (long)&Range);
        }
        LastChar= SendMessage(ARichEdit->Handle, EM_FORMATRANGE, 1, (long)&Range);
        if(LastChar>=TextLength||LastChar==-1){
            LastChar=TextLength;
            PrintFinished=true;
        }
    } else {
        LastChar= SendMessage(ARichEdit->Handle, EM_FORMATRANGE, 1, (long)&Range);
        PrintFinished=true;
        inherited::Print(OfsX,OfsY);
    }
    SetMapMode(ParentReport->QRPrinter->Canvas->Handle, OldMapMode);
    if(PrintFinished)
      SendMessage(ARichEdit->Handle, EM_FORMATRANGE, 0, 0);
}
//---------------------------------------------------------------------------
