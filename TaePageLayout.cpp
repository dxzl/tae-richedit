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
// TaePageLayout.cpp - implementation file for several classes involved
// in page formatting (headers, footers, etc.).
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaePageLayout.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// TTaeHeaderText class
//---------------------------------------------------------------------------
// TTaeHeaderText routines - these are not expected to be called often, i.e.,
// not called from within loops, and are, therefore, not intended to be
// very efficient
//---------------------------------------------------------------------------
// TTaeHeaderText constructor
//
__fastcall TTaeHeaderText::TTaeHeaderText(void)
{
  FFont = new TFont();
  FFont->Name = "Arial";
  FFont->Size = 10;
  FFont->Style = FFont->Style << fsBold;
  FLeft = new TStringList();
  FCenter = new TStringList();
  FRight = new TStringList();
}
//---------------------------------------------------------------------------
// TTaeHeaderText destructor
//
__fastcall TTaeHeaderText::~TTaeHeaderText(void)
{
  // delete font(s), if any
  if (FLeft) delete FLeft;
  if (FCenter) delete FCenter;
  if (FRight) delete FRight;
  if (FFont) delete FFont;
}
//---------------------------------------------------------------------------
// font property setter
//
void __fastcall TTaeHeaderText::SetFont(TFont* font)
{
  FFont->Assign(font);
}
//---------------------------------------------------------------------------
// scan text looking adding escapeChar before any charToEscape's not
// already preceded with escapeChar
//
static AnsiString AddCharEscapes(AnsiString text, char escapeChar, char charToEscape)
{
  // allocate a buffer twice the size of the string
  int size = text.Length() * 2;
  char* buf = new char[size + 2];
  char* pBuf = buf;

  // get char buffer from text
  char* pText = text.c_str();

  // march through pText doing copies and substitutions into pBuf
  do {
    // if this is escapeChar, copy this and fall through
    if (*pText == escapeChar) *pBuf++ = *pText++;
    // else if this is charToEscape, insert escapeChar and fall through
    else if (*pText == charToEscape) *pBuf++ = escapeChar;
    // this is the fallthrough -- copy char unconditionally
    *pBuf++ = *pText++;
    } while (*pText && (pBuf - buf) < size);

  // make sure buf is delimited (should not be necessary)
  buf[size + 1] = '\0';

  // copy to temporary and release allocated storage
  AnsiString retVal(buf);
  delete[] buf;

  // return temporary
  return retVal;
}
//---------------------------------------------------------------------------
// break into parts at '|' (unless escaped with '\')
//
void __fastcall TTaeHeaderText::SetText(AnsiString text)
{
  // clear current values
  FLeft->Clear();
  FCenter->Clear();
  FRight->Clear();

  // allocate working buffer
  int size = text.Length();
  char* buf = new char[size + 1];
  char* pBuf = buf;
  char* bBuf;
  char c;

  // copy into working buffer
  strncpy(buf, text.c_str(), size);
  buf[size] = '\0';

  // find first '|' not preceded by '\'
  bBuf = pBuf;
  while (*pBuf && *pBuf != '|') {
    if (*pBuf == '\\') {
      pBuf++;
      if (*pBuf) pBuf++;
      }
    else pBuf++;
    }

  // *pBuf == '|' or '\0'; save current char to test later and put in left text
  c = *pBuf;
  *pBuf = '\0';
  FLeft->Text = bBuf;

  // if finished, quick out
  if (!c) {
    delete[] buf;
    return;
    }

  // else advance buffer pointer and do it again
  pBuf++;
  bBuf = pBuf;
  while (*pBuf && *pBuf != '|') {
    if (*pBuf == '\\') {
      pBuf++;
      if (*pBuf) pBuf++;
      }
    else pBuf++;
    }

  // *pBuf == '|' or '\0'; save current char to test later and put in left text
  c = *pBuf;
  *pBuf = '\0';
  FCenter->Text = bBuf;

  // if finished, quick out
  if (!c) {
    delete[] buf;
    return;
    }

  // else remainder goes to right
  pBuf++;
  FRight->Text = pBuf;

  // free buffer
  delete[] buf;
}
//---------------------------------------------------------------------------
// return left, center, & right text as a single string delimited by '|';
// escapes added as needed
AnsiString __fastcall TTaeHeaderText::GetText(void)
{
return AddCharEscapes(FLeft->Text, '\\', '|') + "|" +
  AddCharEscapes(FCenter->Text, '\\', '|') + "|" +
  AddCharEscapes(FRight->Text, '\\', '|');
}
//---------------------------------------------------------------------------
// copy the list of strings
//
void __fastcall TTaeHeaderText::SetStringList(TStringList* list, TStringList* newList)
{
  list->Assign(newList);
}
//---------------------------------------------------------------------------
// TTaeHeaderText assignment operator
//
TTaeHeaderText& TTaeHeaderText::operator=(TTaeHeaderText& header)
{
  if (&header == this) return *this;
  FLeft->Assign(header.FLeft);
  FCenter->Assign(header.FCenter);
  FRight->Assign(header.FRight);
  SetFont(header.FFont);
  return *this;
}
//---------------------------------------------------------------------------
// TTaeHeaderText copy constructor
//
TTaeHeaderText::TTaeHeaderText(const TTaeHeaderText& header)
{
  FLeft = new TStringList();
  FCenter = new TStringList();
  FRight = new TStringList();
  FFont = new TFont();

  FLeft->Assign(header.FLeft);
  FCenter->Assign(header.FCenter);
  FRight->Assign(header.FRight);
  SetFont(header.FFont);
}
//---------------------------------------------------------------------------
// load TTaeHeaderText information from the registry
//
void TTaeHeaderText::LoadFromRegistry(TRegistry& reg, AnsiString name)
{
  // load text -- apparently AnsiString s = TRegistry::ReadString (or maybe
  // TRegistry::WriteString()) does not handle special characters, e.g.,
  // "\r\n", correctly -- use the left, center, and right members rather
  // than the Text property or SetText() method
  FLeft->Text = RegValueExists(reg, name + "Left").ReadString(name + "Left");
  FCenter->Text = RegValueExists(reg, name + "Center").ReadString(name + "Center");
  FRight->Text = RegValueExists(reg, name + "Right").ReadString(name + "Right");

  // load font
  ::LoadFromRegistry(reg, name + "Font", *FFont);
}
//---------------------------------------------------------------------------
// save TTaeHeaderText information to the registry
//
void TTaeHeaderText::SaveToRegistry(TRegistry& reg, AnsiString name)
{
  // save text -- apparently AnsiString s = TRegistry::ReadString (or maybe
  // TRegistry::WriteString()) does not handle special characters, e.g.,
  // "\r\n", correctly -- use the left, center, and right members rather
  // than the Text property or GetText() method
  reg.WriteString(name + "Left", FLeft->Text);
  reg.WriteString(name + "Center", FCenter->Text);
  reg.WriteString(name + "Right", FRight->Text);

  // save font
  ::SaveToRegistry(reg, name + "Font", *FFont);
}
//---------------------------------------------------------------------------
// TTaePageStyle class
//---------------------------------------------------------------------------
// TTaePageStyle constructor
//
TTaePageStyle::TTaePageStyle(void)
{
  FFirstHeader = new TTaeHeaderText();
  FFirstFooter = new TTaeHeaderText();
  FHeader = new TTaeHeaderText();
  FFooter = new TTaeHeaderText();
  FBorderWidth = 10;
  FInsideMargin = 100;
  FDifferentFirstPage = false;
}
//---------------------------------------------------------------------------
// TTaePageStyle destructor
//
TTaePageStyle::~TTaePageStyle()
{
  if (FFirstHeader) delete FFirstHeader;
  if (FFirstFooter) delete FFirstFooter;
  if (FHeader) delete FHeader;
  if (FFooter) delete FFooter;
}
//---------------------------------------------------------------------------
// TTaePageStyle assignment operator
//
TTaePageStyle& TTaePageStyle::operator=(TTaePageStyle& style)
{
  if (&style == this) return *this;

  FName = style.FName;
  *FFirstHeader = *style.FFirstHeader;
  *FFirstFooter = *style.FFirstFooter;
  *FHeader = *style.FHeader;
  *FFooter = *style.FFooter;
  FBorderWidth = style.FBorderWidth;
  FInsideMargin = style.FInsideMargin;
  FBorderLines = style.FBorderLines;
  FDifferentFirstPage = style.FDifferentFirstPage;
  return *this;
}
//---------------------------------------------------------------------------
// TTaePageStyle copy constructor
//
TTaePageStyle::TTaePageStyle(const TTaePageStyle& style)
{
  FFirstHeader = new TTaeHeaderText;
  FFirstFooter = new TTaeHeaderText;
  FHeader = new TTaeHeaderText;
  FFooter = new TTaeHeaderText;

  FName = style.FName;
  *FFirstHeader = *style.FFirstHeader;
  *FFirstFooter = *style.FFirstFooter;
  *FHeader = *style.FHeader;
  *FFooter = *style.FFooter;
  FBorderWidth = style.FBorderWidth;
  FInsideMargin = style.FInsideMargin;
  FBorderLines = style.FBorderLines;
  FDifferentFirstPage = style.FDifferentFirstPage;
}
//---------------------------------------------------------------------------
// load TTaePageStyle information from the registry
//
void TTaePageStyle::LoadFromRegistry(TRegistry& reg, AnsiString name)
{
  FName = RegValueExists(reg, name + "Name").ReadString(name + "Name");
  FFirstHeader->LoadFromRegistry(reg, name + "Header1");
  FFirstFooter->LoadFromRegistry(reg, name + "Footer1");
  FHeader->LoadFromRegistry(reg, name + "Header0");
  FFooter->LoadFromRegistry(reg, name + "Footer0");
  FBorderWidth = RegValueExists(reg, name + "BorderWidth").ReadInteger(name + "BorderWidth");

  FBorderLines.Clear();
  if (RegValueExists(reg, name + "HeaderLeft").ReadBool(name + "HeaderLeft"))
    FBorderLines << blHeaderLeft;
  if (RegValueExists(reg, name + "HeaderTop").ReadBool(name + "HeaderTop"))
    FBorderLines << blHeaderTop;
  if (RegValueExists(reg, name + "HeaderRight").ReadBool(name + "HeaderRight"))
    FBorderLines << blHeaderRight;
  if (RegValueExists(reg, name + "HeaderBottom").ReadBool(name + "HeaderBottom"))
    FBorderLines << blHeaderBottom;

  if (RegValueExists(reg, name + "BodyLeft").ReadBool(name + "BodyLeft"))
    FBorderLines << blBodyLeft;
  if (RegValueExists(reg, name + "BodyTop").ReadBool(name + "BodyTop"))
    FBorderLines << blBodyTop;
  if (RegValueExists(reg, name + "BodyRight").ReadBool(name + "BodyRight"))
    FBorderLines << blBodyRight;
  if (RegValueExists(reg, name + "BodyBottom").ReadBool(name + "BodyBottom"))
    FBorderLines << blBodyBottom;

  if (RegValueExists(reg, name + "FooterLeft").ReadBool(name + "FooterLeft"))
    FBorderLines << blFooterLeft;
  if (RegValueExists(reg, name + "FooterTop").ReadBool(name + "FooterTop"))
    FBorderLines << blFooterTop;
  if (RegValueExists(reg, name + "FooterRight").ReadBool(name + "FooterRight"))
    FBorderLines << blFooterRight;
  if (RegValueExists(reg, name + "FooterBottom").ReadBool(name + "FooterBottom"))
    FBorderLines << blFooterBottom;

  FInsideMargin = RegValueExists(reg, name + "InsideMargin").ReadInteger(name + "InsideMargin");
  FDifferentFirstPage = RegValueExists(reg, name + "Different").ReadBool(name + "Different");
}
//---------------------------------------------------------------------------
// save TTaePageStyle information to the registry
//
void TTaePageStyle::SaveToRegistry(TRegistry& reg, AnsiString name)
{
  reg.WriteString(name + "Name", FName);
  FFirstHeader->SaveToRegistry(reg, name + "Header1");
  FFirstFooter->SaveToRegistry(reg, name + "Footer1");
  FHeader->SaveToRegistry(reg, name + "Header0");
  FFooter->SaveToRegistry(reg, name + "Footer0");
  reg.WriteInteger(name + "BorderWidth", FBorderWidth);

  reg.WriteBool(name + "HeaderLeft", FBorderLines.Contains(blHeaderLeft));
  reg.WriteBool(name + "HeaderTop", FBorderLines.Contains(blHeaderTop));
  reg.WriteBool(name + "HeaderRight", FBorderLines.Contains(blHeaderRight));
  reg.WriteBool(name + "HeaderBottom", FBorderLines.Contains(blHeaderBottom));

  reg.WriteBool(name + "BodyLeft", FBorderLines.Contains(blBodyLeft));
  reg.WriteBool(name + "BodyTop", FBorderLines.Contains(blBodyTop));
  reg.WriteBool(name + "BodyRight", FBorderLines.Contains(blBodyRight));
  reg.WriteBool(name + "BodyBottom", FBorderLines.Contains(blBodyBottom));

  reg.WriteBool(name + "FooterLeft", FBorderLines.Contains(blFooterLeft));
  reg.WriteBool(name + "FooterTop", FBorderLines.Contains(blFooterTop));
  reg.WriteBool(name + "FooterRight", FBorderLines.Contains(blFooterRight));
  reg.WriteBool(name + "FooterBottom", FBorderLines.Contains(blFooterBottom));

  reg.WriteInteger(name + "InsideMargin", FInsideMargin);
  reg.WriteBool(name + "Different", FDifferentFirstPage);
}
//---------------------------------------------------------------------------
// remove TTaePageStyle information from the registry
//
void TTaePageStyle::RemoveFromRegistry(TRegistry& reg, AnsiString name)
{
  if (reg.ValueExists(name + "Name")) reg.DeleteValue(name + "Name");
  if (reg.ValueExists(name + "Header1")) reg.DeleteValue(name + "Header1");
  if (reg.ValueExists(name + "Footer1")) reg.DeleteValue(name + "Footer1");
  if (reg.ValueExists(name + "Header0")) reg.DeleteValue(name + "Header0");
  if (reg.ValueExists(name + "Footer0")) reg.DeleteValue(name + "Footer0");
  if (reg.ValueExists(name + "BorderWidth")) reg.DeleteValue(name + "BorderWidth");

  if (reg.ValueExists(name + "HeaderLeft")) reg.DeleteValue(name + "HeaderLeft");
  if (reg.ValueExists(name + "HeaderTop")) reg.DeleteValue(name + "HeaderTop");
  if (reg.ValueExists(name + "HeaderRight")) reg.DeleteValue(name + "HeaderRight");
  if (reg.ValueExists(name + "HeaderBottom")) reg.DeleteValue(name + "HeaderBottom");

  if (reg.ValueExists(name + "BodyLeft")) reg.DeleteValue(name + "BodyLeft");
  if (reg.ValueExists(name + "BodyTop")) reg.DeleteValue(name + "BodyTop");
  if (reg.ValueExists(name + "BodyRight")) reg.DeleteValue(name + "BodyRight");
  if (reg.ValueExists(name + "BodyBottom")) reg.DeleteValue(name + "BodyBottom");

  if (reg.ValueExists(name + "FooterLeft")) reg.DeleteValue(name + "FooterLeft");
  if (reg.ValueExists(name + "FooterTop")) reg.DeleteValue(name + "FooterTop");
  if (reg.ValueExists(name + "FooterRight")) reg.DeleteValue(name + "FooterRight");
  if (reg.ValueExists(name + "FooterBottom")) reg.DeleteValue(name + "FooterBottom");

  if (reg.ValueExists(name + "InsideMargin")) reg.DeleteValue(name + "InsideMargin");
  if (reg.ValueExists(name + "Different")) reg.DeleteValue(name + "Different");
}
//---------------------------------------------------------------------------
// TTaePageStyleList class - manages a list of TTaePageStyle entries
//---------------------------------------------------------------------------
// TTaePageStyleList constructor
//
TTaePageStyleList::TTaePageStyleList(void)
{
}
//---------------------------------------------------------------------------
// TTaePageStyleList destructor
//
TTaePageStyleList::~TTaePageStyleList()
{
  Clear();
}
//---------------------------------------------------------------------------
// clear the list of styles
//
void TTaePageStyleList::Clear(void)
{
// S.S. Replaced 07/14/00 *******************************************
//  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++)
//    delete *ps++;
  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++)
    delete *ps;
// With *************************************************************
  FPageStyles.erase(FPageStyles.begin(), FPageStyles.end());
  FCurrentStyleName = "";
}
//---------------------------------------------------------------------------
// add a style to the list
//
void TTaePageStyleList::Add(TTaePageStyle& style)
{
  TTaePageStyle* ps = new TTaePageStyle(style);
  if (!FPageStyles.size()) FCurrentStyleName = ps->FName;
  FPageStyles.push_back(ps);
}
//---------------------------------------------------------------------------
// locate a style by name
//
TTaePageStyle* TTaePageStyleList::Find(AnsiString styleName)
{
  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++)
    if (!(*ps)->FName.AnsiCompare(styleName)) return *ps;
  return 0;
}
//---------------------------------------------------------------------------
// change a style; optionally add the style if not found
//
void TTaePageStyleList::Change(TTaePageStyle& style, bool add)
{
  TTaePageStyle* ps = Find(style.FName);
  if (ps) *ps = style;
  else Add(style);
}
//---------------------------------------------------------------------------
// delete a style from the list
//
void TTaePageStyleList::Delete(AnsiString styleName)
{
  // need to change to select another style and prevent deleting all...
  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++) {
    if (!(*ps)->FName.AnsiCompare(styleName)) {
      delete *ps;
      FPageStyles.erase(ps);
      return;
      }
    }
}
//---------------------------------------------------------------------------
// determine if a named style is in the list
//
bool TTaePageStyleList::StyleExists(AnsiString styleName)
{
  if (!FPageStyles.size()) return false;
  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++)
    if (!(*ps)->FName.AnsiCompare(styleName)) return true;
  return false;
}
//---------------------------------------------------------------------------
// get the number of styles in the list
//
int TTaePageStyleList::Count(void)
{
  return FPageStyles.size();
}
//---------------------------------------------------------------------------
// get the style name of the nth entry
//
AnsiString TTaePageStyleList::StyleName(int index)
{
  if (index >= Count()) return "";
  return FPageStyles[index]->FName;
}
//---------------------------------------------------------------------------
// load a list of styles from the registry
//
void TTaePageStyleList::LoadFromRegistry(TRegistry& reg, AnsiString name)
{
  int cnt = RegValueExists(reg, name + "Count").ReadInteger(name + "Count");
  TTaePageStyle style;

  for (int i = 0; i < cnt; i++) {
    style.LoadFromRegistry(reg, name + AnsiString(i));
    Add(style);
    }

  CurrentStyleName = RegValueExists(reg, name + "CurrentStyle").ReadString( name + "CurrentStyle");
}
//---------------------------------------------------------------------------
// save the list of styles to the registry
//
void TTaePageStyleList::SaveToRegistry(TRegistry& reg, AnsiString name)
{
  // save the list of styles
  int i = 0;
  for (TTaePageStyles::iterator ps = FPageStyles.begin(); ps < FPageStyles.end(); ps++)
    (*ps)->SaveToRegistry(reg, name + AnsiString(i++));

  // delete any stray entries
  int prevCnt = 0;
  if (reg.ValueExists(name + "Count")) prevCnt = reg.ReadInteger(name + "Count");
  while (i < prevCnt) TTaePageStyle::RemoveFromRegistry(reg, name + AnsiString(i++));

  // write the new count and style
  reg.WriteInteger(name + "Count", FPageStyles.size());
  reg.WriteString(name + "CurrentStyle", FCurrentStyleName);
}
//---------------------------------------------------------------------------
