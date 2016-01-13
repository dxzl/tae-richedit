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
// TaeAttrib2.cpp - implementation file for character and paragraph 
// formatting classes.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeAttrib2.h"
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma pack(push, 4)
//---------------------------------------------------------------------------
// TFont2 class
//---------------------------------------------------------------------------
// TFont2 constructor
//
__fastcall TFont2::TFont2(void) : TFont()
{
  FBackColor = clWindow;
  FStyle2.Clear();
}
//---------------------------------------------------------------------------
// set the background color for the text (RE 2.0+)
//
void __fastcall TFont2::SetBackColor(TColor color)
{
  FBackColor = color;
  Changed();
}
//---------------------------------------------------------------------------
// fetch TFont2 styles
//
TFontStyles2 __fastcall TFont2::GetStyle2(void)
{
  return FStyle2;
}
//---------------------------------------------------------------------------
// set TFont2 styles
//
void __fastcall TFont2::SetStyle2(TFontStyles2 style)
{
  FStyle2 = style;
  Changed();
}
//---------------------------------------------------------------------------
// notify TFont that TFont2 has changed something...
// things fell apart at some point without this (GPFs, etc.)  may not really
// be needed
//
void __fastcall TFont2::Changed(void)
{
  TFont::Changed();
}
//---------------------------------------------------------------------------
// assign font characteristics from another object
//
void __fastcall TFont2::Assign(Classes::TPersistent* Source)
{
  TFont2* f2 = dynamic_cast<TFont2*>(Source);
  if (f2) {
    TFont::Assign((TFont*) f2);
    BackColor = f2->BackColor;
    Style2 = f2->Style2;
    return;
    }

  TFont::Assign(Source);
}
//---------------------------------------------------------------------------
// TTaeTextAttributes2 class
//---------------------------------------------------------------------------
// TTaeTextAttributes2 constructor
//
__fastcall TTaeTextAttributes2::TTaeTextAttributes2(TTaeRichEdit* AOwner,
  TAttributeType AttributeType) : TTaeTextAttributes(AOwner, AttributeType)
{
}
//---------------------------------------------------------------------------
// initialize a CHARFORMAT2 structure
//
void __fastcall TTaeTextAttributes2::InitFormat2(Richedit::TCharFormat2A &Format)
{
  ::memset(&Format, 0, sizeof(Format));
  Format.cbSize = sizeof(Format);
}
//---------------------------------------------------------------------------
// get character attributes that are consistent throughout text range
//
TConsistentAttributes2 __fastcall TTaeTextAttributes2::GetConsistentAttributes2(void)
{
  TCharFormat2A Format;
  TConsistentAttributes2 Result;

  if (FRichEdit->Handle && FType == atSelected) {
    InitFormat2(Format);
    ::SendMessage(FRichEdit->Handle, EM_GETCHARFORMAT,
      (WPARAM) FType == atSelected, (LPARAM) &Format);

    if (Format.dwMask & CFM_ANIMATION)    Result << ca2Animation;
    if (Format.dwMask & CFM_BACKCOLOR)    Result << ca2BackColor;
    if (Format.dwMask & CFM_CHARSET)    Result << ca2CharSet;
    if (Format.dwMask & CFM_COLOR)      Result << ca2Color;
    if (Format.dwMask & CFM_FACE)      Result << ca2Face;
    if (Format.dwMask & CFM_KERNING)    Result << ca2Kerning;
    if (Format.dwMask & CFM_LCID)      Result << ca2LcId;
    if (Format.dwMask & CFM_OFFSET)      Result << ca2Offset;
    if (Format.dwMask & CFM_REVAUTHOR)    Result << ca2RevAuthor;
    if (Format.dwMask & CFM_SIZE)      Result << ca2Size;
    if (Format.dwMask & CFM_SPACING)    Result << ca2Spacing;
    if (Format.dwMask & CFM_STYLE)      Result << ca2Style;
    if (Format.dwMask & CFM_UNDERLINETYPE)  Result << ca2UnderlineType;
    if (Format.dwMask & CFM_WEIGHT)      Result << ca2Weight;
    if (Format.dwMask & CFM_ALLCAPS)    Result << ca2AllCaps;
    if (Format.dwMask & CFM_BOLD)      Result << ca2Bold;
    if (Format.dwMask & CFM_DISABLED)    Result << ca2Disabled;
    if (Format.dwMask & CFM_EMBOSS)      Result << ca2Emboss;
    if (Format.dwMask & CFM_HIDDEN)      Result << ca2Hidden;
    if (Format.dwMask & CFM_IMPRINT)    Result << ca2Imprint;
    if (Format.dwMask & CFM_ITALIC)      Result << ca2Italic;
    if (Format.dwMask & CFM_LINK)      Result << ca2Link;
    if (Format.dwMask & CFM_OUTLINE)    Result << ca2Outline;
    if (Format.dwMask & CFM_PROTECTED)    Result << ca2Protected;
    if (Format.dwMask & CFM_REVISED)    Result << ca2Revised;
    if (Format.dwMask & CFM_SHADOW)      Result << ca2Shadow;
    if (Format.dwMask & CFM_SMALLCAPS)    Result << ca2SmallCaps;
    if (Format.dwMask & CFM_STRIKEOUT)    Result << ca2StrikeOut;
    if (Format.dwMask & (CFM_SUBSCRIPT))  Result << ca2SubScript;
    if (Format.dwMask & (CFM_SUPERSCRIPT))  Result << ca2SuperScript;
    if (Format.dwMask & CFM_UNDERLINE)    Result << ca2Underline;
    }
  return Result;
}
//---------------------------------------------------------------------------
// get character attributes for first character of text
//
void __fastcall TTaeTextAttributes2::GetAttributes2(Richedit::TCharFormat2A &Format)
{
  InitFormat2(Format);
  if (FRichEdit->Handle) ::SendMessage(FRichEdit->Handle, EM_GETCHARFORMAT,
    (WPARAM) FType == atSelected, (LPARAM) &Format);
}
//---------------------------------------------------------------------------
// set character attributes for text
//
void __fastcall TTaeTextAttributes2::SetAttributes2(Richedit::TCharFormat2A &Format)
{
  int Flag = (FType == atSelected) ? SCF_SELECTION : 0;

  if (FRichEdit->Handle) ::SendMessage(FRichEdit->Handle, EM_SETCHARFORMAT,
    (WPARAM) Flag, (LPARAM) &Format);
}
//---------------------------------------------------------------------------
// get the background color for the first character of text
//
Graphics::TColor __fastcall TTaeTextAttributes2::GetBackColor(void)
{
  TCharFormat2A Format;

  GetAttributes2(Format);
  if (Format.dwEffects & CFE_AUTOBACKCOLOR) return clWindow;
  return TColor(Format.crBackColor);
}
//---------------------------------------------------------------------------
// set the background color for the text
//
void __fastcall TTaeTextAttributes2::SetBackColor(Graphics::TColor Value)
{
  TCharFormat2A Format;

  InitFormat2(Format);
  Format.dwMask = CFM_BACKCOLOR;
  if (Value == clWindow) Format.dwEffects = CFE_AUTOBACKCOLOR;
  else Format.crTextColor = ColorToRGB(Value);
  Format.crBackColor = ColorToRGB(Value);
  SetAttributes2(Format);
}
//---------------------------------------------------------------------------
// get the TFontStyles2 attributes of the first character of the text
//
TFontStyles2 __fastcall TTaeTextAttributes2::GetStyle2(void)
{
  TCharFormat2A Format;

  TFontStyles2 Result;
  GetAttributes2(Format);
  if (Format.dwEffects & CFE_ALLCAPS)   Result << fs2AllCaps;
  if (Format.dwEffects & CFE_DISABLED)   Result << fs2Disabled;
  if (Format.dwEffects & CFE_EMBOSS)     Result << fs2Emboss;
  if (Format.dwEffects & CFE_HIDDEN)     Result << fs2Hidden;
  if (Format.dwEffects & CFE_IMPRINT)   Result << fs2Imprint;
  if (Format.dwEffects & CFE_OUTLINE)   Result << fs2Outline;
  if (Format.dwEffects & CFE_REVISED)   Result << fs2Revised;
  if (Format.dwEffects & CFE_SHADOW)     Result << fs2Shadow;
  if (Format.dwEffects & CFE_SMALLCAPS)   Result << fs2SmallCaps;
  if (Format.dwEffects & CFE_SUBSCRIPT)   Result << fs2SubScript;
  if (Format.dwEffects & CFE_SUPERSCRIPT)  Result << fs2SuperScript;

  return Result;
}
//---------------------------------------------------------------------------
// set the TFontStyles2 attributes of the text
//
void __fastcall TTaeTextAttributes2::SetStyle2(TFontStyles2& Value)
{
  TCharFormat2A Format;

  InitFormat2(Format);
  Format.dwMask = CFE_ALLCAPS | CFE_DISABLED | CFE_EMBOSS |
    CFE_HIDDEN | CFE_IMPRINT | CFE_OUTLINE | CFE_REVISED |
    CFE_SHADOW | CFE_SMALLCAPS | CFE_SUBSCRIPT |
    CFE_SUPERSCRIPT;

  if (Value.Contains(fs2AllCaps))    Format.dwEffects |= CFE_ALLCAPS;
  if (Value.Contains(fs2Disabled))  Format.dwEffects |= CFE_DISABLED;
  if (Value.Contains(fs2Emboss))    Format.dwEffects |= CFE_EMBOSS;
  if (Value.Contains(fs2Hidden))    Format.dwEffects |= CFE_HIDDEN;
  if (Value.Contains(fs2Imprint))    Format.dwEffects |= CFE_IMPRINT;
  if (Value.Contains(fs2Outline))    Format.dwEffects |= CFE_OUTLINE;
  if (Value.Contains(fs2Revised))    Format.dwEffects |= CFE_REVISED;
  if (Value.Contains(fs2Shadow))    Format.dwEffects |= CFE_SHADOW;
  if (Value.Contains(fs2SmallCaps))  Format.dwEffects |= CFE_SMALLCAPS;
  if (Value.Contains(fs2SubScript))  Format.dwEffects |= CFE_SUBSCRIPT;
  if (Value.Contains(fs2SuperScript))  Format.dwEffects |= CFE_SUPERSCRIPT;

  SetAttributes2(Format);
}
//---------------------------------------------------------------------------
// copy the text attributes of the object to this text
//
void __fastcall TTaeTextAttributes2::Assign(Classes::TPersistent* Source)
{
  // call inherited first
  TTaeTextAttributes::Assign(Source);

  // are we assigning from TTaeTextAttributes2?
  TTaeTextAttributes2* ta = dynamic_cast<TTaeTextAttributes2*>(Source);
  if (ta) {
    BackColor = ta->BackColor;
    Style2 = ta->Style2;
    Link = ta->Link;
    return;
    }

  // are we assigning from a TFont2?
  TFont2* f2 = dynamic_cast<TFont2*>(Source);
  if (f2) {
    BackColor = f2->BackColor;
    Style2 = f2->Style2;
    return;
    }
}
//---------------------------------------------------------------------------
// copy the character attributes of this text to another object
//
void __fastcall TTaeTextAttributes2::AssignTo(Classes::TPersistent* Dest)
{
  // call inherited
  TTaeTextAttributes::AssignTo(Dest);

  // are we assigning to a TTaeTextAttributes2?
  TTaeTextAttributes2* ta = dynamic_cast<TTaeTextAttributes2*>(Dest);
  if (ta) {
    ta->BackColor = BackColor;
    ta->Link = Link;
    ta->Style2 = Style2;
    return;
    }

  // are we assigning to a TFont2?
  TFont2* f2 = dynamic_cast<TFont2*>(Dest);
  if (f2) {
    f2->BackColor = BackColor;
    f2->Style2 = Style2;
    return;
    }
}
//---------------------------------------------------------------------------
// get the type of underline used for the first character of the text
//
TUnderlineType __fastcall TTaeTextAttributes2::GetUnderlineType(void)
{
  TCharFormat2A Format;

  GetAttributes2(Format);
  return (TUnderlineType) Format.bUnderlineType;
}
//---------------------------------------------------------------------------
// set the type of underline used for the text
//
void __fastcall TTaeTextAttributes2::SetUnderlineType(TUnderlineType ut)
{
  TCharFormat2A Format;

  InitFormat2(Format);
  Format.dwMask = CFM_UNDERLINETYPE;
  Format.bUnderlineType = ut;

  SetAttributes2(Format);
}
//---------------------------------------------------------------------------
// get individual character style attributes (RE 2.0+) -- if inconsistent,
// return tsMaybe
//
TThreeState __fastcall TTaeTextAttributes2::GetFontStyle2(DWORD effect, DWORD mask)
{
  TCharFormat2A Format;

  GetAttributes2(Format);
  if (!(Format.dwMask & mask)) return tsMaybe;
  if (Format.dwEffects & effect) return tsYes;
  return tsNo;
}
//---------------------------------------------------------------------------
// set text character style attributes (RE 2.0+)
//
void __fastcall TTaeTextAttributes2::SetFontStyle2(DWORD effect, DWORD mask,
  TThreeState state)
{
  if (state == tsMaybe) return;    // do nothing

  TCharFormat2A Format;

  InitFormat2(Format);
  Format.dwMask = mask;
  if (state == tsYes) Format.dwEffects |= effect;

  SetAttributes2(Format);
}
//---------------------------------------------------------------------------
// (for convenience) get bold attribute for first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetBold2(void)
{
  return GetFontStyle2(CFE_BOLD, CFM_BOLD);
}
//---------------------------------------------------------------------------
// (for convenience) set bold attribute for text
//
void __fastcall TTaeTextAttributes2::SetBold2(TThreeState state)
{
  SetFontStyle2(CFE_BOLD, CFM_BOLD, state);
}
//---------------------------------------------------------------------------
// (for convenience) get italic attribute for first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetItalic2(void)
{
  return GetFontStyle2(CFE_ITALIC, CFM_ITALIC);
}
//---------------------------------------------------------------------------
// (for convenience) set italic attribute for text
//
void __fastcall TTaeTextAttributes2::SetItalic2(TThreeState state)
{
  SetFontStyle2(CFE_ITALIC, CFM_ITALIC, state);
}
//---------------------------------------------------------------------------
// (for convenience) get simple underline attribute (RE 1.0) for first
// character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetUnderline2(void)
{
  return GetFontStyle2(CFE_UNDERLINE, CFM_UNDERLINE);
}
//---------------------------------------------------------------------------
// (for convenience) set simple underline attribute (RE 1.0) for text
//
void __fastcall TTaeTextAttributes2::SetUnderline2(TThreeState state)
{
  SetFontStyle2(CFE_UNDERLINE, CFM_UNDERLINE, state);
}
//---------------------------------------------------------------------------
// (for convenience) get strike-out attribute for first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetStrikeOut2(void)
{
  return GetFontStyle2(CFE_STRIKEOUT, CFM_STRIKEOUT);
}
//---------------------------------------------------------------------------
// (for convenience) set strike-out attribute for text
//
void __fastcall TTaeTextAttributes2::SetStrikeOut2(TThreeState state)
{
  SetFontStyle2(CFE_STRIKEOUT, CFM_STRIKEOUT, state);
}
//---------------------------------------------------------------------------
// (RE 2.0+) return superscript attribute of first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetSuperScript(void)
{
  return GetFontStyle2(CFE_SUPERSCRIPT, CFM_SUPERSCRIPT);
}
//---------------------------------------------------------------------------
// (RE 2.0+) set superscript attribute of text
//
void __fastcall TTaeTextAttributes2::SetSuperScript(TThreeState state)
{
  SetFontStyle2(CFE_SUPERSCRIPT, CFM_SUPERSCRIPT, state);
}
//---------------------------------------------------------------------------
// (RE 2.0+) return sufscript attribute of first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetSubScript(void)
{
  return GetFontStyle2(CFE_SUBSCRIPT, CFM_SUBSCRIPT);
}
//---------------------------------------------------------------------------
// (RE 2.0+) set subscript attribute of text
//
void __fastcall TTaeTextAttributes2::SetSubScript(TThreeState state)
{
  SetFontStyle2(CFE_SUBSCRIPT, CFM_SUBSCRIPT, state);
}
//---------------------------------------------------------------------------
// get Link attribute of first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetLink(void)
{
  return GetFontStyle2(CFE_LINK, CFM_LINK);
}
//---------------------------------------------------------------------------
// set Link attribute of text
//
void __fastcall TTaeTextAttributes2::SetLink(TThreeState state)
{
  SetFontStyle2(CFE_LINK, CFM_LINK, state);
}
//---------------------------------------------------------------------------
// fetch Protected attribute of first character of text
//
TThreeState __fastcall TTaeTextAttributes2::GetProtected2(void)
{
  return GetFontStyle2(CFE_PROTECTED, CFM_PROTECTED);
}
//---------------------------------------------------------------------------
// set Protected attribute of text
//
void __fastcall TTaeTextAttributes2::SetProtected2(TThreeState state)
{
  SetFontStyle2(CFE_PROTECTED, CFM_PROTECTED, state);
}
//---------------------------------------------------------------------------
// TTaeParaAttributes2 class
//---------------------------------------------------------------------------
// TTaeParaAttributes2 class constructor
//
__fastcall TTaeParaAttributes2::TTaeParaAttributes2(TTaeRichEdit* AOwner) :
  TTaeParaAttributes(AOwner)
{
}
//---------------------------------------------------------------------------
// initialize PARAFORMAT2 structure
//
void TTaeParaAttributes2::InitFormat2(PARAFORMAT2& format)
{
  ::memset(&format, 0, sizeof(PARAFORMAT2));
  format.cbSize = sizeof(PARAFORMAT2);
}
//---------------------------------------------------------------------------
// get paragraph attributes
//
void TTaeParaAttributes2::GetFormat2(PARAFORMAT2& format)
{
  InitFormat2(format);
  if (FRichEdit->Handle)
    ::SendMessage(FRichEdit->Handle, EM_GETPARAFORMAT, 0, (LPARAM) &format);
}
//---------------------------------------------------------------------------
// set paragraph attributes
//
void TTaeParaAttributes2::SetFormat2(PARAFORMAT2& format)
{
  if (FRichEdit->Handle)
    ::SendMessage(FRichEdit->Handle, EM_SETPARAFORMAT, 0, (LPARAM) &format);
}
//---------------------------------------------------------------------------
// return a set identifying paragraph attributes that are consistently applied
// throughout all paragraphs in text
//
TParaConsistentAttributes2 __fastcall TTaeParaAttributes2::GetConsistentAttributes(void)
{
  TParaConsistentAttributes2 pca;
  PARAFORMAT2 format;

  GetFormat2(format);
  DWORD mask = format.dwMask;

  if (mask & PFM_ALIGNMENT)    pca << pca2Alignment;
  if (mask & PFM_BORDER)      pca << pca2Border;
  if (mask & PFM_LINESPACING)    pca << pca2LineSpacing;
  if (mask & PFM_NUMBERING)    pca << pca2Numbering;
  if (mask & PFM_NUMBERINGSTART)  pca << pca2NumberingStart;
  if (mask & PFM_NUMBERINGSTYLE)  pca << pca2NumberingStyle;
  if (mask & PFM_NUMBERINGTAB)  pca << pca2NumberingTab;
  if (mask & PFM_OFFSET)      pca << pca2Offset;
  if (mask & PFM_OFFSETINDENT)  pca << pca2OffsetIndent;
  if (mask & PFM_RIGHTINDENT)    pca << pca2RightIndent;
  if (mask & PFM_SHADING)      pca << pca2Shading;
  if (mask & PFM_SPACEAFTER)    pca << pca2SpaceAfter;
  if (mask & PFM_SPACEBEFORE)    pca << pca2SpaceBefore;
  if (mask & PFM_STARTINDENT)    pca << pca2StartIndent;
  if (mask & PFM_STYLE)      pca << pca2Style;
  if (mask & PFM_TABSTOPS)    pca << pca2TabStops;
  if (mask & PFM_DONOTHYPHEN)    pca << pca2NoHyphen;
  if (mask & PFM_KEEP)      pca << pca2KeepTogether;
  if (mask & PFM_KEEPNEXT)    pca << pca2KeepNext;
  if (mask & PFM_NOLINENUMBER)  pca << pca2NoLineNumber;
  if (mask & PFM_NOWIDOWCONTROL)  pca << pca2NoWidowControl;
  if (mask & PFM_PAGEBREAKBEFORE)  pca << pca2PageBefore;
  if (mask & PFM_RTLPARA)      pca << pca2RtlPara;
  if (mask & PFM_SIDEBYSIDE)    pca << pca2SideBySide;
  if (mask & PFM_TABLE)      pca << pca2Table;
  if (mask & PFM_BOX)        pca << pca2Box;
  if (mask & PFM_COLLAPSED)    pca << pca2Collapsed;
  if (mask & PFM_OUTLINELEVEL)  pca << pca2OutlineLevel;

  return pca;
}
//---------------------------------------------------------------------------
// return the paragraph alignment for the first paragraph
//
TAlignment2 __fastcall TTaeParaAttributes2::GetAlignment2(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);

  return (TAlignment2) format.wAlignment;
}
//---------------------------------------------------------------------------
// set the paragraph alignment for the text
//
void __fastcall TTaeParaAttributes2::SetAlignment2(TAlignment2 alignment)
{
  PARAFORMAT2 format;

  InitFormat2(format);
  format.dwMask = PFM_ALIGNMENT;
  format.wAlignment = alignment;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// fetch the paragraph numbering attributes for the first paragraph
//
TNumberingType2 __fastcall TTaeParaAttributes2::GetNumbering2(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);

  return (TNumberingType2) format.wNumbering;
}
//---------------------------------------------------------------------------
// set the paragraph numbering attributes for the text
//
void __fastcall TTaeParaAttributes2::SetNumbering2(TNumberingType2 numberingType)
{
  PARAFORMAT2 format;

  InitFormat2(format);
  format.dwMask = PFM_NUMBERING;
  format.wNumbering = (WORD) numberingType;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the space before the first paragraph in twips
//
int __fastcall TTaeParaAttributes2::GetSpaceBefore(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);

  return format.dwMask & PFM_SPACEBEFORE ? format.dySpaceBefore : 0;
}
//---------------------------------------------------------------------------
// set the space before the paragraph(s) in twips
//
void __fastcall TTaeParaAttributes2::SetSpaceBefore(int twips)
{
  PARAFORMAT2 format;

  InitFormat2(format);
  format.dySpaceBefore = twips;
  format.dwMask = PFM_SPACEBEFORE;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the space after the first paragraph in twips
//
int __fastcall TTaeParaAttributes2::GetSpaceAfter(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);

  return format.dwMask & PFM_SPACEAFTER ? format.dySpaceAfter : 0;
}
//---------------------------------------------------------------------------
// set the space after the paragraph(s) in twips
//
void __fastcall TTaeParaAttributes2::SetSpaceAfter(int twips)
{
  PARAFORMAT2 format;

  InitFormat2(format);
  format.dySpaceAfter = twips;
  format.dwMask = PFM_SPACEAFTER;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// return line spacing for the first paragraph
//
TLineSpacingRule __fastcall TTaeParaAttributes2::GetLineSpacing(void)
{
  PARAFORMAT2 format;

  GetFormat2(format);
  return (TLineSpacingRule) format.dwMask;
}
//---------------------------------------------------------------------------
// set spacing within the paragraph(s)
//
void __fastcall TTaeParaAttributes2::SetLineSpacing(TLineSpacingRule lineSpacing)
{
  PARAFORMAT2 format;

  InitFormat2(format);
  format.bLineSpacingRule = lineSpacing;

  SetFormat2(format);
}
//---------------------------------------------------------------------------
// return the position of the next tab position from the attributes of the
// first paragraph of text
//
int __fastcall TTaeParaAttributes2::NextTab(int fromPos)
{
  PARAFORMAT2 format;

  GetFormat2(format);

  // note: we have a valid set of tabstops -- they may, however,
  // be inconsistent since multiple paragraphs may be selected....
  //
  // if only 1 tabstop, return a multiple of that value
  if (format.cTabCount < 2) {
    // note: the "+" outside of the parenthesis is an attempt to force
    // the compiler to not optimize away the expression....  this was
    // documented in BC++ and I *hope* that it still applies....
    return +((fromPos + format.rgxTabs[0]) / format.rgxTabs[0]) *
      format.rgxTabs[0];
    }

  // we will allow an unlimited number of tabstops -- by taking the last two
  // valid tabstops and adding that difference to the last valid tabstop
  // until we reach a value greater than the requested position...
  if (fromPos > format.rgxTabs[format.cTabCount - 1]) {
    int tabBase = format.rgxTabs[format.cTabCount - 1];
    int tabIncr = tabBase - format.rgxTabs[format.cTabCount - 2];
    int diffBase = fromPos - tabBase;
    int nextPos = +((diffBase + tabIncr) / tabIncr) * tabIncr;
    nextPos += tabBase;
    return nextPos;
    }
  // else we find the tab position that follows the passed position parameter
  else for (int i = 0; i < format.cTabCount; i++)
    if (format.rgxTabs[i] > fromPos) return format.rgxTabs[i];

  // if all else fails, return zero
  return 0;
}
//---------------------------------------------------------------------------
// return the position of the prior tab position from the attributes of the
// first paragraph of text
//
int __fastcall TTaeParaAttributes2::PriorTab(int fromPos)
{
  PARAFORMAT2 format;

  GetFormat2(format);

  if (fromPos < 1) return 0;

  // note: we have a valid set of tabstops -- they may, however,
  // be inconsistent since multiple paragraphs may be selected....
  //
  // if only 1 tabstop, return a multiple of that value
  if (format.cTabCount < 2) {
    // note: the "+" outside of the parenthesis is an attempt to force
    // the compiler to not optimize away the expression....  this was
    // documented in BC++ and I *hope* that it still applies....
    return +((fromPos - 1) / format.rgxTabs[0]) * format.rgxTabs[0];
    }

  // we will allow an unlimited number of tabstops -- by taking the last two
  // valid tabstops and adding that difference to the last valid tabstop
  // until we reach a value less than the requested position...
  if (fromPos > format.rgxTabs[format.cTabCount - 1]) {
    int tabBase = format.rgxTabs[format.cTabCount - 1];
    int tabIncr = tabBase - format.rgxTabs[format.cTabCount - 2];
    int diffBase = fromPos - tabBase;
    int nextPos = +((diffBase - 1) / tabIncr) * tabIncr;
    nextPos += tabBase;
    return nextPos;
    }

  // otherwise we find the tab position that follows the passed position parameter
  int pos = 0;
  for (int i = 0; i < format.cTabCount; i++) {
    if (format.rgxTabs[i] >= fromPos) break;
    pos = format.rgxTabs[i];
    }
  return pos;
}
//---------------------------------------------------------------------------
// get the first line indent (in twips) for the first paragraph of the text
// for the first line of the paragraph(s) from the left margin
//
int __fastcall TTaeParaAttributes2::GetFirstIndent2(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return format.dxStartIndent;
}
//---------------------------------------------------------------------------
// set the first line indent (in twips) from the left margin
//
void __fastcall TTaeParaAttributes2::SetFirstIndent2(int twips)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_STARTINDENT;
  format.dxStartIndent = twips;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the paragraph indent (in twips) for the first paragraph of the text
// for the first line of the paragraph(s) from the left margin
//
int __fastcall TTaeParaAttributes2::GetLeftIndent2(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return format.dxOffset;
}
//---------------------------------------------------------------------------
// set the paragraph indent (in twips) from the left margin
//
void __fastcall TTaeParaAttributes2::SetLeftIndent2(int twips)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_OFFSET;
  format.dxOffset = twips;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// return the indent from the right margin (in twips)
//
int __fastcall TTaeParaAttributes2::GetRightIndent2(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return format.dxRightIndent;
}
//---------------------------------------------------------------------------
// set the indent from the right margin (in twips)
//
void __fastcall TTaeParaAttributes2::SetRightIndent2(int twips)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_RIGHTINDENT;
  format.dxRightIndent = twips;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the "page break before" format of the first paragraph
//
bool __fastcall TTaeParaAttributes2::GetPageBreakBefore(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return format.wEffects & PFE_PAGEBREAKBEFORE;
}
//---------------------------------------------------------------------------
// set the "page break before" format of the paragraph(s)
//
// note: I added this code to determine if the doc was correct.  sadly, it was.
// that is, setting the page break before attribute on a paragraph has no
// effect upon the rendering in a RE control.  however, if you save the RTF,
// the page break will appear within Word97....
//
void __fastcall TTaeParaAttributes2::SetPageBreakBefore(bool breakBefore)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_PAGEBREAKBEFORE;
  format.wEffects = breakBefore ? (WORD) PFE_PAGEBREAKBEFORE : (WORD) 0;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// return the "numbering start" attribute of the first paragraph (RE 3.0
// partially supports; RE 2.x- does not support)
//
int __fastcall TTaeParaAttributes2::GetNumberingStart(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return format.wNumberingStart;
}
//---------------------------------------------------------------------------
// set the "numbering start" attribute of the paragraph(s) (RE 3.0
// partially supports; RE 2.x- does not support)
//
void __fastcall TTaeParaAttributes2::SetNumberingStart(int start)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_NUMBERINGSTART;
  format.wNumberingStart = (WORD) start;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the "numbering style" attribute of the first paragraph (RE 3.0
// partially supports; RE 2.x- does not support)
//
TNumberingStyle2 __fastcall TTaeParaAttributes2::GetNumberingStyle(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return (TNumberingStyle2) format.wNumberingStyle;
}
//---------------------------------------------------------------------------
// set the "numbering style" attribute of the paragraph(s) (RE 3.0
// partially supports; RE 2.x- does not support)
//
void __fastcall TTaeParaAttributes2::SetNumberingStyle(TNumberingStyle2 style)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  format.dwMask = PFM_NUMBERINGSTYLE | PFM_NUMBERING;
  format.wNumberingStyle = style;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
// get the "skip numbering" attribute of the first paragraph (not supported
// by RE 1.0 - 3.0?)
//
bool __fastcall TTaeParaAttributes2::GetSkipNumbering(void)
{
  PARAFORMAT2 format;
  GetFormat2(format);
  return (format.dwMask & PFM_NOLINENUMBER) &&
    (format.wEffects & PFE_NOLINENUMBER);
}
//---------------------------------------------------------------------------
// set the "skip numbering" attribute of the paragraph(s) (not supported
// by RE 1.0 - 3.0?)
//
void __fastcall TTaeParaAttributes2::SetSkipNumbering(bool skip)
{
  PARAFORMAT2 format;
  InitFormat2(format);
  format.dwMask = PFM_NOLINENUMBER;
  if (skip) format.wEffects = PFE_NOLINENUMBER;
  SetFormat2(format);
}
//---------------------------------------------------------------------------
#pragma pack(pop)
//---------------------------------------------------------------------------
