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
// TaeAttrib.h - header file for TaeAttrib.cpp  character and paragraph
// formatting classes.
//---------------------------------------------------------------------------
#ifndef TaeAttribH
#define TaeAttribH

#include <comctrls.hpp>

#pragma pack(push, 4)
class PACKAGE TTaeTextAttributes : public Classes::TPersistent
{
friend class PACKAGE TTaeRichEdit;

protected:
  TTaeRichEdit* FRichEdit;
  TAttributeType FType;
  void __fastcall GetAttributes(Richedit::TCharFormatA &Format);
  Graphics::TFontCharset __fastcall GetCharset(void);
  Graphics::TColor __fastcall GetColor(void);
  TConsistentAttributes __fastcall GetConsistentAttributes(void);
  int __fastcall GetHeight(void);
  AnsiString __fastcall GetName();
  Graphics::TFontPitch __fastcall GetPitch(void);
  bool __fastcall GetProtected(void);
  int __fastcall GetSize(void);
  Graphics::TFontStyles __fastcall GetStyle(void);
  void __fastcall SetAttributes(Richedit::TCharFormatA &Format);
  void __fastcall SetCharset(Graphics::TFontCharset Value);
  void __fastcall SetColor(Graphics::TColor Value);
  void __fastcall SetHeight(int Value);
  void __fastcall SetName(AnsiString Value);
  void __fastcall SetPitch(Graphics::TFontPitch Value);
  void __fastcall SetProtected(bool Value);
  void __fastcall SetSize(int Value);
  //  note pass by reference in the following -- see implementation for comments
  void __fastcall SetStyle(Graphics::TFontStyles& Value);

protected:
  void __fastcall InitFormat(Richedit::TCharFormatA &Format);
  virtual void __fastcall AssignTo(Classes::TPersistent* Dest);

public:
  __fastcall TTaeTextAttributes(TTaeRichEdit* AOwner, TAttributeType AttributeType);
  #pragma option push -w-inl
  inline __fastcall virtual ~TTaeTextAttributes(void) { };
  #pragma option pop
  virtual void __fastcall Assign(Classes::TPersistent* Source);

__published:
  __property Graphics::TFontCharset Charset = {read=GetCharset, write=SetCharset, nodefault};
  __property Graphics::TColor Color = {read=GetColor, write=SetColor, nodefault};
  __property TConsistentAttributes ConsistentAttributes = {read=GetConsistentAttributes, nodefault};
  __property AnsiString Name = {read=GetName, write=SetName};
  __property Graphics::TFontPitch Pitch = {read=GetPitch, write=SetPitch, nodefault};
  __property bool Protected = {read=GetProtected, write=SetProtected, nodefault};
  __property int Size = {read=GetSize, write=SetSize, nodefault};
  __property Graphics::TFontStyles Style = {read=GetStyle, write=SetStyle, nodefault};
  __property int Height = {read=GetHeight, write=SetHeight, nodefault};
};

class PACKAGE TTaeParaAttributes : public Classes::TPersistent
{
friend class PACKAGE TTaeRichEdit;

protected:
  TTaeRichEdit* FRichEdit;
  void __fastcall GetAttributes(_paraformat& Paragraph);
  Classes::TAlignment __fastcall GetAlignment(void);
  int __fastcall GetFirstIndent(void);
  int __fastcall GetLeftIndent(void);
  int __fastcall GetRightIndent(void);
  TNumberingStyle __fastcall GetNumbering(void);
  int __fastcall GetTab(Byte Index);
  int __fastcall GetTabCount(void);
  void __fastcall InitPara(_paraformat& Paragraph);
  void __fastcall SetAlignment(Classes::TAlignment Value);
  void __fastcall SetAttributes(_paraformat& Paragraph);
  void __fastcall SetFirstIndent(int Value);
  void __fastcall SetLeftIndent(int Value);
  void __fastcall SetRightIndent(int Value);
  void __fastcall SetNumbering(TNumberingStyle Value);
  void __fastcall SetTab(Byte Index, int Value);
  void __fastcall SetTabCount(int Value);

public:
  __fastcall TTaeParaAttributes(TTaeRichEdit* AOwner);
  #pragma option push -w-inl
  inline __fastcall virtual ~TTaeParaAttributes(void) { };
  #pragma option pop
  virtual void __fastcall Assign(Classes::TPersistent* Source);

__published:
  __property Classes::TAlignment Alignment = {read=GetAlignment, write=SetAlignment, nodefault};
  __property int FirstIndent = {read=GetFirstIndent, write=SetFirstIndent, nodefault};
  __property int LeftIndent = {read=GetLeftIndent, write=SetLeftIndent, nodefault};
  __property TNumberingStyle Numbering = {read=GetNumbering, write=SetNumbering, nodefault};
  __property int RightIndent = {read=GetRightIndent, write=SetRightIndent, nodefault};
  __property int Tab[Byte Index] = {read=GetTab, write=SetTab};
  __property int TabCount = {read=GetTabCount, write=SetTabCount, nodefault};
};
#pragma pack(pop)
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
