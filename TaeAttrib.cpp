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
// TaeAttrib.cpp - implementation file for character and paragraph
// formatting classes.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeAttrib.h"
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma pack(push, 4)
//---------------------------------------------------------------------------
// TTaeTextAttributes
//---------------------------------------------------------------------------
// TTaeTextAttributes constructor
//
__fastcall TTaeTextAttributes::TTaeTextAttributes(TTaeRichEdit* AOwner,
  TAttributeType AttributeType) : TPersistent()
{
  FRichEdit = AOwner;
  FType = AttributeType;
}
//---------------------------------------------------------------------------
// initialize a CHARFORMAT structure
//
void __fastcall TTaeTextAttributes::InitFormat(TCharFormat& Format)
{
  ::memset(&Format, 0, sizeof(TCharFormat));
  Format.cbSize = sizeof(TCharFormat);
}
//---------------------------------------------------------------------------
// Fetch the CHARFORMAT attributes and return a set that indicates which
// attributes are consistent throughout the range of text
//
TConsistentAttributes __fastcall TTaeTextAttributes::GetConsistentAttributes(void)
{
  TCharFormat Format;

  TConsistentAttributes Result;
  Result.Clear();        // necessary?
  if (FRichEdit->HandleAllocated() && FType == atSelected) {
    InitFormat(Format);
    ::SendMessage(FRichEdit->Handle, EM_GETCHARFORMAT,
      (WPARAM) FType == atSelected, (LPARAM) &Format);
    if (Format.dwMask & CFM_BOLD) Result = Result << caBold;
    if (Format.dwMask & CFM_COLOR) Result = Result << caColor;
    if (Format.dwMask & CFM_FACE) Result = Result << caFace;
    if (Format.dwMask & CFM_ITALIC) Result = Result << caItalic;
    if (Format.dwMask & CFM_SIZE) Result = Result << caSize;
    if (Format.dwMask & CFM_STRIKEOUT) Result = Result << caStrikeOut;
    if (Format.dwMask & CFM_UNDERLINE) Result = Result << caUnderline;
    if (Format.dwMask & CFM_PROTECTED) Result = Result << caProtected;
    }

  return Result;
}
//---------------------------------------------------------------------------
// fetch the character attributes for the first character of the range
//
void __fastcall TTaeTextAttributes::GetAttributes(TCharFormat& Format)
{
  InitFormat(Format);
  if (!FRichEdit->HandleAllocated()) return;
  SendMessage(FRichEdit->Handle, EM_GETCHARFORMAT,
    (WPARAM) FType == atSelected, (LPARAM) &Format);
}
//---------------------------------------------------------------------------
// set the character attributes of a range of text
//
void __fastcall TTaeTextAttributes::SetAttributes(TCharFormat& Format)
{
  int Flag = (FType == atSelected) ? SCF_SELECTION : 0;
  if (FRichEdit->HandleAllocated())
    ::SendMessage(FRichEdit->Handle, EM_SETCHARFORMAT, Flag, (LPARAM) &Format);
}
//---------------------------------------------------------------------------
// fetch the character set of the font for the first character of the text
//
TFontCharset __fastcall TTaeTextAttributes::GetCharset(void)
{
  TCharFormat Format;
  GetAttributes(Format);
  return(Format.bCharSet);
}
//---------------------------------------------------------------------------
// set the character set of the font
//
void __fastcall TTaeTextAttributes::SetCharset(TFontCharset Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_CHARSET;
  Format.bCharSet = Value;
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// get the protected state of the first character of the text
//
bool __fastcall TTaeTextAttributes::GetProtected(void)
{
  TCharFormat Format;

  GetAttributes(Format);
  return Format.dwEffects & CFE_PROTECTED;
}
//---------------------------------------------------------------------------
// set or clear the protected state of the text
//
void __fastcall TTaeTextAttributes::SetProtected(bool Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_PROTECTED;
  if (Value) Format.dwEffects = CFE_PROTECTED;
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// get the text color of the first character of the text
//
TColor __fastcall TTaeTextAttributes::GetColor(void)
{
  TCharFormat Format;

  GetAttributes(Format);
  return (Format.dwEffects & CFE_AUTOCOLOR) ?
    clWindowText : TColor(Format.crTextColor);
}
//---------------------------------------------------------------------------
// set the text color to the value (or auto-color if the value is the
// same as normal text)
//
void __fastcall TTaeTextAttributes::SetColor(TColor Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_COLOR;
  if (Value == clWindowText) Format.dwEffects = CFE_AUTOCOLOR;
  else Format.crTextColor = ColorToRGB(Value);
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// return the name of the text font for the first character of the text
//
TFontName __fastcall TTaeTextAttributes::GetName(void)
{
  TCharFormat Format;

  GetAttributes(Format);
  return Format.szFaceName;
}
//---------------------------------------------------------------------------
// set the text font by font name
//
void __fastcall TTaeTextAttributes::SetName(TFontName Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_FACE;

  ::strncpy(Format.szFaceName, Value.c_str(), LF_FACESIZE);
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// get the style attributes of the first character of the text
//
TFontStyles __fastcall TTaeTextAttributes::GetStyle(void)
{
  TCharFormat Format;
  TFontStyles Result;

  GetAttributes(Format);
  if (Format.dwEffects & CFE_BOLD) Result = Result << fsBold;
  if (Format.dwEffects & CFE_ITALIC) Result = Result << fsItalic;
  if (Format.dwEffects & CFE_UNDERLINE) Result = Result << fsUnderline;
  if (Format.dwEffects & CFE_STRIKEOUT) Result = Result << fsStrikeOut;
  return Result;
}
//---------------------------------------------------------------------------
// set the style attributes of the text
//
// warning:  I spent days trying to isolate a problem that turned out to be
// in this method.  the problem was with the original signature,
// SetStyle(TFontStyles Value).  Sets (TFontStyles is of type Set)
// do not always correctly pass unless passed by reference?
//
// note that this is an inconsistent problem.  elsewhere in this project,
// passing sets by value works just fine.  it may have something to do with
// the way TFont works (TFont manages the font resource in a rather odd way).
//
void __fastcall TTaeTextAttributes::SetStyle(TFontStyles& Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
  if (Value.Contains(fsBold)) Format.dwEffects |= CFE_BOLD;
  if (Value.Contains(fsItalic)) Format.dwEffects |= CFE_ITALIC;
  if (Value.Contains(fsUnderline)) Format.dwEffects |= CFE_UNDERLINE;
  if (Value.Contains(fsStrikeOut)) Format.dwEffects |= CFE_STRIKEOUT;
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// get the text size in points for the first character in the range
//
int __fastcall TTaeTextAttributes::GetSize(void)
{
  TCharFormat Format;

  GetAttributes(Format);
  return Format.yHeight / 20;
}
//---------------------------------------------------------------------------
// set the text size in points
//
void __fastcall TTaeTextAttributes::SetSize(int Value)
{
  TCharFormat Format;

  InitFormat(Format);
  Format.dwMask = CFM_SIZE;
  Format.yHeight = Value * 20;
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// get the text size in pixels for the first character in the text
//
int __fastcall TTaeTextAttributes::GetHeight(void)
{
  return ::MulDiv(Size, FRichEdit->FScreenLogPixels, 72);
}
//---------------------------------------------------------------------------
// set the text size in pixels
//
void __fastcall TTaeTextAttributes::SetHeight(int Value)
{
  Size = ::MulDiv(Value, 72, FRichEdit->FScreenLogPixels);
}
//---------------------------------------------------------------------------
// get the font pitch (default, fixed, or variable) for the first character
// in the text
//
TFontPitch __fastcall TTaeTextAttributes::GetPitch(void)
{
  TCharFormat Format;

  GetAttributes(Format);
  switch (Format.bPitchAndFamily & 0x03) {
    case DEFAULT_PITCH:
      return(fpDefault);
    case VARIABLE_PITCH:
      return fpVariable;
    case FIXED_PITCH:
      return fpFixed;
    }
  return fpDefault;
}
//---------------------------------------------------------------------------
// set the font pitch (default, fixed, or variable)
//
void __fastcall TTaeTextAttributes::SetPitch(TFontPitch Value)
{
  TCharFormat Format;

  InitFormat(Format);
  switch (Value) {
    case fpVariable:
      Format.bPitchAndFamily = VARIABLE_PITCH;
      break;
    case fpFixed:
      Format.bPitchAndFamily = FIXED_PITCH;
      break;
    default:
      Format.bPitchAndFamily = DEFAULT_PITCH;
      break;
    }
  SetAttributes(Format);
}
//---------------------------------------------------------------------------
// copy the font/text attributes from another object
//
void __fastcall TTaeTextAttributes::Assign(TPersistent* Source)
{
  TFont* font = dynamic_cast<TFont*>(Source);
  if (font) {
    Color = font->Color;
    Name = font->Name;
    Charset = font->Charset;
    Style = font->Style;
    Size = font->Size;
    Pitch = font->Pitch;
    return;
    }

  TTaeTextAttributes* attrib = dynamic_cast<TTaeTextAttributes*>(Source);
  if (attrib) {
    Color = attrib->Color;
    Name = attrib->Name;
    Charset = attrib->Charset;
    Style = attrib->Style;
    Pitch = attrib->Pitch;
    return;
    }

  TPersistent::Assign(Source);
}
//---------------------------------------------------------------------------
// copy the font/text attributes to another object
//
void __fastcall TTaeTextAttributes::AssignTo(TPersistent* Dest)
{
  TFont* font = dynamic_cast<TFont*>(Dest);
  if (font) {
    font->Color = Color;
    font->Name = Name;
    font->Charset = Charset;
    font->Style = Style;
    font->Size = Size;
    font->Pitch = Pitch;
    return;
    }

  TTaeTextAttributes* attrib = dynamic_cast<TTaeTextAttributes*>(Dest);
  if (attrib) {
    attrib->Color = Color;
    attrib->Name = Name;
    attrib->Charset = Charset;
    attrib->Style = Style;
    attrib->Pitch = Pitch;
    return;
    }

  TPersistent::AssignTo(Dest);
}
//---------------------------------------------------------------------------
// TTaeParaAttributes
//---------------------------------------------------------------------------
// TTaeParaAttributes constructor
//
__fastcall TTaeParaAttributes::TTaeParaAttributes(TTaeRichEdit* AOwner) :
  TPersistent()
{
  FRichEdit = AOwner;
}
//---------------------------------------------------------------------------
// initialize a PARAFORMAT structure
//
void __fastcall TTaeParaAttributes::InitPara(TParaFormat& Paragraph)
{
  ::memset(&Paragraph, 0, sizeof(TParaFormat));
  Paragraph.cbSize = sizeof(TParaFormat);
}
//---------------------------------------------------------------------------
// fetch the paragraph attributes of the first paragraph of the text
//
void __fastcall TTaeParaAttributes::GetAttributes(TParaFormat& Paragraph)
{
  InitPara(Paragraph);
  if (FRichEdit->HandleAllocated())
    ::SendMessage(FRichEdit->Handle, EM_GETPARAFORMAT, 0, (LPARAM) &Paragraph);
}
//---------------------------------------------------------------------------
// set the paragraph attributes of the text
//
// thanks to Pete Fraser for a bug report and detailed fix!
//
void __fastcall TTaeParaAttributes::SetAttributes(TParaFormat& Paragraph)
{
  FRichEdit->HandleNeeded(); // we REALLY need the handle for BiDi
  if (!FRichEdit->HandleAllocated()) return;
  if (FRichEdit->UseRightToLeftAlignment()) {
    if (Paragraph.wAlignment == PFA_LEFT)
      Paragraph.wAlignment = PFA_RIGHT;
    else if (Paragraph.wAlignment == PFA_RIGHT)
      Paragraph.wAlignment = PFA_LEFT;
    }
  ::SendMessage(FRichEdit->Handle, EM_SETPARAFORMAT, 0, (LPARAM) &Paragraph);
}
//---------------------------------------------------------------------------
// get the paragraph alignment for the first paragraph of the text
//
TAlignment __fastcall TTaeParaAttributes::GetAlignment(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return TAlignment(Paragraph.wAlignment - 1);
}
//---------------------------------------------------------------------------
// set the paragraph alignment for the text
//
void __fastcall TTaeParaAttributes::SetAlignment(TAlignment Value)
{
  TParaFormat Paragraph;

  InitPara(Paragraph);
  Paragraph.dwMask = PFM_ALIGNMENT;
  Paragraph.wAlignment = (WORD) (Value + 1);
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// return the numbering style for the first paragraph of the text
//
TNumberingStyle __fastcall TTaeParaAttributes::GetNumbering(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return TNumberingStyle(Paragraph.wNumbering);
}
//---------------------------------------------------------------------------
// set the numbering style for the paragraphs
//
void __fastcall TTaeParaAttributes::SetNumbering(TNumberingStyle Value)
{
  TParaFormat Paragraph;

  switch (Value) {
    case nsBullet:
      if (LeftIndent < 10) LeftIndent = 10;
      break;
    case nsNone:
      LeftIndent = 0;
      break;
    }

  InitPara(Paragraph);
  Paragraph.dwMask = PFM_NUMBERING;
  Paragraph.wNumbering = Value;

  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// get the first line indent (in points?) for the first paragraph of the text
// for the first line of the paragraph(s) from the left margin
//
int __fastcall TTaeParaAttributes::GetFirstIndent(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return Paragraph.dxStartIndent / 20;
}
//---------------------------------------------------------------------------
// set the first line indent (in points?) from the left margin
//
void __fastcall TTaeParaAttributes::SetFirstIndent(int Value)
{
  TParaFormat Paragraph;

  InitPara(Paragraph);
  Paragraph.dwMask = PFM_STARTINDENT;
  Paragraph.dxStartIndent = Value * 20;
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// get the paragraph indent (in points?) for the first paragraph of the text
// for the first line of the paragraph(s) from the left margin
//
int __fastcall TTaeParaAttributes::GetLeftIndent(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return Paragraph.dxOffset / 20;
}
//---------------------------------------------------------------------------
// set the paragraph indent (in points?) from the left margin
//
void __fastcall TTaeParaAttributes::SetLeftIndent(int Value)
{
  TParaFormat Paragraph;

  InitPara(Paragraph);
  Paragraph.dwMask = PFM_OFFSET;
  Paragraph.dxOffset = Value * 20;
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// return the indent from the right margin (in points?)
//
int __fastcall TTaeParaAttributes::GetRightIndent(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return(Paragraph.dxRightIndent /20);
}
//---------------------------------------------------------------------------
// set the indent from the right margin (in points?)
//
void __fastcall TTaeParaAttributes::SetRightIndent(int Value)
{
  TParaFormat Paragraph;

  InitPara(Paragraph);
  Paragraph.dwMask = PFM_RIGHTINDENT;
  Paragraph.dxRightIndent = Value * 20;
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// get the nth tab position of the first paragraph in points (?)
//
int __fastcall TTaeParaAttributes::GetTab(BYTE Index)
{
  TParaFormat Paragraph;
  GetAttributes(Paragraph);
  return Paragraph.rgxTabs[Index] / 20;
}
//---------------------------------------------------------------------------
// set the nth tab position of the paragraph(s) in points (?)
//
void __fastcall TTaeParaAttributes::SetTab(BYTE Index, int Value)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  Paragraph.rgxTabs[Index] = Value * 20;
  Paragraph.dwMask = PFM_TABSTOPS;
  if (Paragraph.cTabCount < Index) Paragraph.cTabCount = Index;
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// return the number of tabs currently defined/set
//
int __fastcall TTaeParaAttributes::GetTabCount(void)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  return Paragraph.cTabCount;
}
//---------------------------------------------------------------------------
// set the number of tabs recognized by the control
//
void __fastcall TTaeParaAttributes::SetTabCount(int Value)
{
  TParaFormat Paragraph;

  GetAttributes(Paragraph);
  Paragraph.dwMask = PFM_TABSTOPS;
  Paragraph.cTabCount = (char) Value;
  SetAttributes(Paragraph);
}
//---------------------------------------------------------------------------
// copy the paragraph attributes from the source to the paragraph(s)
//
void __fastcall TTaeParaAttributes::Assign(TPersistent* Source)
{
  TTaeParaAttributes* attrib = dynamic_cast<TTaeParaAttributes*>(Source);
  if (attrib) {
    Alignment = attrib->Alignment;
    FirstIndent = attrib->FirstIndent;
    LeftIndent = attrib->LeftIndent;
    RightIndent = attrib->RightIndent;
    Numbering = attrib->Numbering;
    for (int I = 0; I < MAX_TAB_STOPS; I++)
      Tab[(SHORT) I] = (SHORT) attrib->Tab[(SHORT) I];
    return;
    }

  TPersistent::Assign(Source);
}
#pragma pack(pop)
//---------------------------------------------------------------------------
