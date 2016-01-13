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
// TaeAttrib2.h - header file for TaeAttrib2.cpp.  character and paragraph
// formatting classes.
//---------------------------------------------------------------------------
#ifndef TaeAttrib2H
#define TaeAttrib2H
//---------------------------------------------------------------------------
#include "TaeAttrib.h"
#pragma pack(push, 4)

#define PFM_BOX         0x04000000
#define PFM_COLLAPSED       0x01000000
#define PFM_OUTLINELEVEL     0x02000000
#define PFE_OUTLINELEVEL    (PFM_OUTLINELEVEL   >> 16)
#define PFE_COLLAPSED      (PFM_COLLAPSED     >> 16)
#define PFE_BOX          (PFM_BOX       >> 16)
#define PFE_TABLE        0x4000
//---------------------------------------------------------------------------
// TTaeTextAttributes2 class
//---------------------------------------------------------------------------
// extended font styles
enum TFontStyle2 {
  fs2AllCaps, fs2Disabled, fs2Emboss, fs2Hidden, fs2Imprint,
  fs2Outline, fs2Revised, fs2Shadow, fs2SmallCaps,
  fs2SubScript, fs2SuperScript
  };
typedef Set<TFontStyle2, fs2AllCaps, fs2SuperScript> TFontStyles2;

// extended TFont class
class PACKAGE TFont2 : public TFont
{
protected:
  TColor FBackColor;
  TFontStyles2 FStyle2;
  void __fastcall SetBackColor(TColor color);
  TFontStyles2 __fastcall GetStyle2(void);
  void __fastcall SetStyle2(TFontStyles2 style);
  DYNAMIC void __fastcall Changed(void);

public:
  __fastcall TFont2(void);
  virtual void __fastcall Assign(Classes::TPersistent* Source);

__published:
  __property TColor BackColor = { read = FBackColor, write = SetBackColor, default = clWindow};
  __property TFontStyles2 Style2 = { read = GetStyle2, write = SetStyle2, nodefault };
};

// extended TConsistentAttribute types
enum TConsistentAttribute2 {
  ca2Animation, ca2BackColor, ca2CharSet, ca2Color, ca2Face, ca2Kerning,
  ca2LcId, ca2Offset, ca2RevAuthor, ca2Size, ca2Spacing, ca2Style,
  ca2UnderlineType, ca2Weight, ca2AllCaps, ca2Bold, ca2Disabled,
  ca2Emboss, ca2Hidden, ca2Imprint, ca2Italic, ca2Link, ca2Outline,
  ca2Protected, ca2Revised, ca2Shadow, ca2SmallCaps, ca2StrikeOut,
  ca2SubScript, ca2SuperScript, ca2Underline
  };
typedef Set<TConsistentAttribute2, ca2Animation, ca2Underline> TConsistentAttributes2;

// character types for convenience
enum TCharMask {
  cmAnimation = CFM_ANIMATION, cmBackColor = CFM_BACKCOLOR,
  cmCharSet = CFM_CHARSET, cmColor = CFM_COLOR, cmFace = CFM_FACE,
  cmKerning = CFM_KERNING, cmLcId = CFM_LCID, cmOffset = CFM_OFFSET,
  cmRevAuthor = CFM_REVAUTHOR, cmSize = CFM_SIZE, cmSpacing = CFM_SPACING,
  cmStyle = CFM_STYLE, cmUnderlineType = CFM_UNDERLINETYPE,
  cmWeight = CFM_WEIGHT, cmAllCaps = CFM_ALLCAPS, cmBold = CFM_BOLD,
  cmDisabled = CFM_DISABLED, cmEmboss = CFM_EMBOSS, cmHidden = CFM_HIDDEN,
  cmImprint = CFM_IMPRINT, cmItalic = CFM_ITALIC, cmLink = CFM_LINK,
  cmOutline = CFM_OUTLINE, cmProtected = CFM_PROTECTED,
  cmRevised = CFM_REVISED, cmShadow = CFM_SHADOW, cmSmallCaps = CFM_SMALLCAPS,
  cmStrikeOut = CFM_STRIKEOUT, cmSubScript = CFM_SUBSCRIPT,
  cmSuperScript = CFM_SUPERSCRIPT, cmUnderline = CFM_UNDERLINE
  };

// character effects types for convenience
enum TCharEffects {
  ceAllCaps = CFE_ALLCAPS, ceAutoColor = CFE_AUTOCOLOR, ceBold = CFE_BOLD,
  ceDisabled = CFE_DISABLED, ceEmboss = CFE_EMBOSS, ceHidden = CFE_HIDDEN,
  ceImprint = CFE_IMPRINT, ceItalic = CFE_ITALIC, ceLink = CFE_LINK,
  ceOutline = CFE_OUTLINE, ceProtected = CFE_PROTECTED,
  ceRevised = CFE_REVISED, ceShadow = CFE_SHADOW,
  ceSmallCaps = CFE_SMALLCAPS, ceStrikeOut = CFE_STRIKEOUT,
  ceSubScript = CFE_SUBSCRIPT, ceSuperScript = CFE_SUPERSCRIPT,
  ceUnderline = CFE_UNDERLINE
  };

// underline types for RE 2.0+  note:  the actual display and print
// characteristics of underlined text are determined by the version of
// richedXX.dll being used.  the following enum lists all current
//possibilities...
enum TUnderlineType {
  utNone, utSingle, utWord, utDouble, utDotted, utDash, utDashDot,
  utDashDotDot, utWave, utThick, utHairline, utCf1Underine = 0xff
  };

// many attributes are on for part of the text, but not all, hence tsMaybe
enum TThreeState { tsNo, tsYes, tsMaybe };

// TTaeTextAttributes2 class declaration
class PACKAGE TTaeTextAttributes2 : public TTaeTextAttributes
{
protected:
  void __fastcall GetAttributes2(Richedit::TCharFormat2A &Format);
  Graphics::TColor __fastcall GetBackColor(void);
  TConsistentAttributes2 __fastcall GetConsistentAttributes2(void);
  TFontStyles2 __fastcall GetStyle2(void);
  void __fastcall SetAttributes2(Richedit::TCharFormat2A &Format);
  void __fastcall SetBackColor(Graphics::TColor Value);
  void __fastcall SetStyle2(TFontStyles2& Value);
  void __fastcall InitFormat2(Richedit::TCharFormat2A &Format);
  virtual void __fastcall AssignTo(Classes::TPersistent* Dest);

  TUnderlineType __fastcall GetUnderlineType(void);
  void __fastcall SetUnderlineType(TUnderlineType ut);

  TThreeState __fastcall GetFontStyle2(DWORD effect, DWORD mask);
  void __fastcall SetFontStyle2(DWORD effect, DWORD mask, TThreeState state);
  TThreeState __fastcall GetBold2(void);
  void __fastcall SetBold2(TThreeState state);
  TThreeState __fastcall GetItalic2(void);
  void __fastcall SetItalic2(TThreeState state);
  TThreeState __fastcall GetUnderline2(void);
  void __fastcall SetUnderline2(TThreeState state);
  TThreeState __fastcall GetStrikeOut2(void);
  void __fastcall SetStrikeOut2(TThreeState state);
  TThreeState __fastcall GetSuperScript(void);
  void __fastcall SetSuperScript(TThreeState state);
  TThreeState __fastcall GetSubScript(void);
  void __fastcall SetSubScript(TThreeState state);
  TThreeState __fastcall GetLink(void);
  void __fastcall SetLink(TThreeState);
  TThreeState __fastcall GetProtected2(void);
  void __fastcall SetProtected2(TThreeState);

public:
  __fastcall TTaeTextAttributes2(TTaeRichEdit* AOwner, TAttributeType AttributeType);
  __fastcall virtual ~TTaeTextAttributes2(void) { };
  virtual void __fastcall Assign(Classes::TPersistent* Source);
  __property TConsistentAttributes2 ConsistentAttributes2 = { read = GetConsistentAttributes2, nodefault};

__published:
  __property Graphics::TColor BackColor = { read = GetBackColor, write = SetBackColor, nodefault};
  __property TFontStyles2 Style2 = { read = GetStyle2, write = SetStyle2, nodefault};
  __property TUnderlineType UnderlineType = { read = GetUnderlineType, write = SetUnderlineType, nodefault };
  __property TThreeState Bold2 = { read = GetBold2, write = SetBold2, nodefault };
  __property TThreeState Italic2 = { read = GetItalic2, write = SetItalic2, nodefault };
  __property TThreeState Underline2 = { read = GetUnderline2, write = SetUnderline2, nodefault };
  __property TThreeState StrikeOut2 = { read = GetStrikeOut2, write = SetStrikeOut2, nodefault };
  __property TThreeState SuperScript = { read = GetSuperScript, write = SetSuperScript, nodefault };
  __property TThreeState SubScript = { read = GetSubScript, write = SetSubScript, nodefault };
  __property TThreeState Link = { read = GetLink, write = SetLink, nodefault};
  __property TThreeState Protected2 = { read = GetProtected2, write = SetProtected2, nodefault};
};
//---------------------------------------------------------------------------
// TTaeParaAttributes2 class
//---------------------------------------------------------------------------
// PARAFORMAT2 dwMask flags - these flags specify which structure elements or
// effects flags are valid.  note that the EM_GETPARAFORMAT message returns
// these flags for those attributes that are consistent for all currently
// selected text.
//
enum TParaMask {
  // general flags (see other PARAFORMAT2 structure members)
  pmAlignment = PFM_ALIGNMENT, pmBorder = PFM_BORDER,
  pmLineSpacing = PFM_LINESPACING, pmNumbering = PFM_NUMBERING,
  pmNumberingStart = PFM_NUMBERINGSTART, pmNumberingStyle = PFM_NUMBERINGSTYLE,
  pmNumberingTab = PFM_NUMBERINGTAB, pmOffset = PFM_OFFSET,
  pmOffsetIndent = PFM_OFFSETINDENT, pmRightIndent = PFM_RIGHTINDENT,
  pmShading = PFM_SHADING, pmSpaceAfter = PFM_SPACEAFTER,
  pmSpaceBefore = PFM_SPACEBEFORE, pmStartIndent = PFM_STARTINDENT,
  pmStyle = PFM_STYLE, pmTabStops = PFM_TABSTOPS,
  // wReserved or wEffects flags
  pmNoHyphen = PFM_DONOTHYPHEN, pmKeepTogether = PFM_KEEP,
  pmKeepNext = PFM_KEEPNEXT, pmNoLineNumber = PFM_NOLINENUMBER,
  pmNoWidowControl = PFM_NOWIDOWCONTROL, pmPageBefore = PFM_PAGEBREAKBEFORE,
  pmRtlPara = PFM_RTLPARA, pmSideBySide = PFM_SIDEBYSIDE, pmTable = PFM_TABLE,
  pmBox = PFM_BOX, pmCollapsed = PFM_COLLAPSED,
  pmOutlineLevel = PFM_OUTLINELEVEL
  };

enum TParaEffects {
  peDoNotHyphen = PFE_DONOTHYPHEN, peKeepTogether = PFE_KEEP,
  peKeepNext = PFE_KEEPNEXT, peNoLineNumber = PFE_NOLINENUMBER,
  peNoWidowControl = PFE_NOWIDOWCONTROL,
  pePageBreakBefore = PFE_PAGEBREAKBEFORE, peRtlPara = PFE_RTLPARA,
  peSideBySide = PFE_SIDEBYSIDE, peTable = PFE_TABLE
  };

// extended consistent attributes
enum TParaConsistentAttribute2 {
  pca2Alignment = PFM_ALIGNMENT, pca2Border = PFM_BORDER,
  pca2LineSpacing = PFM_LINESPACING, pca2Numbering = PFM_NUMBERING,
  pca2NumberingStart = PFM_NUMBERINGSTART, pca2NumberingStyle = PFM_NUMBERINGSTYLE,
  pca2NumberingTab = PFM_NUMBERINGTAB, pca2Offset = PFM_OFFSET,
  pca2OffsetIndent = PFM_OFFSETINDENT, pca2RightIndent = PFM_RIGHTINDENT,
  pca2Shading = PFM_SHADING, pca2SpaceAfter = PFM_SPACEAFTER,
  pca2SpaceBefore = PFM_SPACEBEFORE, pca2StartIndent = PFM_STARTINDENT,
  pca2Style = PFM_STYLE, pca2TabStops = PFM_TABSTOPS,
  pca2NoHyphen = PFM_DONOTHYPHEN, pca2KeepTogether = PFM_KEEP,
  pca2KeepNext = PFM_KEEPNEXT, pca2NoLineNumber = PFM_NOLINENUMBER,
  pca2NoWidowControl = PFM_NOWIDOWCONTROL, pca2PageBefore = PFM_PAGEBREAKBEFORE,
  pca2RtlPara = PFM_RTLPARA, pca2SideBySide = PFM_SIDEBYSIDE,
  pca2Table = PFM_TABLE, pca2Box = PFM_BOX, pca2Collapsed = PFM_COLLAPSED,
  pca2OutlineLevel = PFM_OUTLINELEVEL
  };
typedef Set<TParaConsistentAttribute2, pmAlignment, pmRtlPara>
  TParaConsistentAttributes2;

// equivalents for PARAFORMAT2... (Rich Edit 2.0+).  note that many attributes
// may not be functional....

// paragraph alignment flags
enum TAlignment2 {
  ta2LeftJustify = PFA_LEFT, ta2Center = PFA_CENTER,
  ta2RightJustify = PFA_RIGHT, ta2Justified = 4, ta2FullInterLetter = 5,
  ta2FullScaled = 6, ta2FullGlyphs = 7, ta2SnapGrid = 8
  };

// paragraph tab alignment flags
enum TParaTabType {
  pttNormal, pttCenter, pttRight, pttDecimal, pttWordBarTab
  };

// paragraph tab leader types
enum TTabLeader {
  tlNone, tlDotted, tlDashed, tlUnderlined, tlThick, tlDouble
  };

// paragraph line spacing rules
enum TLineSpacingRule {
  lsSingle, lsOnePointFive, lsDouble, lsLineSpacingLimited,
  lsLineSpacingUnlimited, lsSingleDiv20
  };

// paragraph shading style; bits 0-3 contain style; 4-7 contain foreground color
// index; 8-11 contain background color index.  used for wShadingStyle.
enum TShadingPattern {
  spNone, spDarkHorz, spDarkVert, spDarkDownDiag, spDarkUpDiag, spDarkGrid,
  spDarkTrellis, spLightHorz, spLightVert, spLightDownDiag, spLightUpDiag,
  spLightGrid, spLightTrellis
  };

// paragraph shading colors
enum TShadingColor {
  scBlack, scBlue, scCyan, scGreen, scMagenta, scRed, scYellow, scWhite,
  scDarkBlue,  scDarkCyan, scDarkGreen, scDarkMagenta, scDarkRed, scDarkYellow,
  scDarkGray, scLightGray
  };

// enum TNumberingStyle2 { ns2None, ns2Bullet, ns2Indeterminate };
// note that numbering type and numbering style have changed meaning...
enum TNumberingType2 {
  nt2None, nt2Bullet, nt2Arabic, nt2LcLetter, nt2UcLetter, nt2LcRoman,
  nt2UcRoman, nt2SpecialSequence
  //, nt2Indeterminate
  };

// numbering styles (RE 3.0)
enum TNumberingStyle2 {
  ns2RightParen, ns2Period, ns2Parens, ns2None,
  ns2RightParenRoman = 0x8000, ns2PeriodRoman, ns2ParensRoman, ns2NoneRoman
  };

// paragraph border location, style, and color.  bits 0-7 specify location,
// 8-11 specify style, and 12-15 specify the color index (use TShadingColor
// enum above).  used with wBorders.
enum TBordersLocation {
  blLeft, blRight, blTop, blBottom, blInside, blOutside, blAutoColor
  };

enum TBordersStyle {
  bsNone = 0x0080, bs3FourthsPt, bsOneAndHalfPt, bsTwoAndOneFourthPt,
  bsThreePt, bsFourAndHalfPt, bsSixPoint, bs3FourthsPtDouble,
  bsOneAndHalfPtDouble, bsTwoAndOneFourthPtDouble, bs3FourthsPtGray,
  bs3FourthsPtGrayDashed
  };

// TTaeParaAttributes2 class declaration
class PACKAGE TTaeParaAttributes2 : public TTaeParaAttributes
{
protected:
  TParaConsistentAttributes2 __fastcall GetConsistentAttributes(void);

  void InitFormat2(PARAFORMAT2& format);
  void GetFormat2(PARAFORMAT2& format);
  void SetFormat2(PARAFORMAT2& format);

  TAlignment2 __fastcall GetAlignment2(void);
  void __fastcall SetAlignment2(TAlignment2 alignment);
  TNumberingType2 __fastcall GetNumbering2(void);
  void __fastcall SetNumbering2(TNumberingType2 numberingType);
  int __fastcall GetSpaceBefore(void);
  void __fastcall SetSpaceBefore(int twips);
  int __fastcall GetSpaceAfter(void);
  void __fastcall SetSpaceAfter(int twips);
  TLineSpacingRule __fastcall GetLineSpacing(void);
  void __fastcall SetLineSpacing(TLineSpacingRule lineSpacing);
  int __fastcall GetFirstIndent2(void);
  void __fastcall SetFirstIndent2(int twips);
  int __fastcall GetLeftIndent2(void);
  void __fastcall SetLeftIndent2(int twips);
  int __fastcall GetRightIndent2(void);
  void __fastcall SetRightIndent2(int twips);
  bool __fastcall GetPageBreakBefore(void);
  void __fastcall SetPageBreakBefore(bool breakBefore);
  int __fastcall GetNumberingStart(void);
  void __fastcall SetNumberingStart(int start);
  TNumberingStyle2 __fastcall GetNumberingStyle(void);
  void __fastcall SetNumberingStyle(TNumberingStyle2 style);
  bool __fastcall GetSkipNumbering(void);
  void __fastcall SetSkipNumbering(bool skip);

public:
  __fastcall TTaeParaAttributes2(TTaeRichEdit* AOwner);
  __fastcall virtual ~TTaeParaAttributes2(void) { };

  int __fastcall NextTab(int fromPos);
  int __fastcall PriorTab(int fromPos);

  __property TParaConsistentAttributes2 ConsistentAttributes =
    { read = GetConsistentAttributes, nodefault };

__published:
  __property TAlignment2 Alignment2 = { read = GetAlignment2, write = SetAlignment2, nodefault, stored = false };
  __property TNumberingType2 Numbering2 = { read = GetNumbering2, write = SetNumbering2, nodefault, stored = false };
  __property int SpaceBefore = { read = GetSpaceBefore, write = SetSpaceBefore, nodefault, stored = false };
  __property int SpaceAfter = { read = GetSpaceAfter, write = SetSpaceAfter, nodefault, stored = false };
  __property TLineSpacingRule LineSpacing = { read = GetLineSpacing, write = SetLineSpacing, nodefault, stored = false };
  __property int FirstIndent2 = { read = GetFirstIndent2, write = SetFirstIndent2, nodefault, stored = false };
  __property int LeftIndent2 = { read = GetLeftIndent2, write = SetLeftIndent2, nodefault, stored = false };
  __property int RightIndent2 = { read = GetRightIndent2, write = SetRightIndent2, nodefault, stored = false };
  __property bool PageBreakBefore = { read = GetPageBreakBefore, write = SetPageBreakBefore, nodefault, stored = false };
  __property int NumberingStart = { read = GetNumberingStart, write = SetNumberingStart, nodefault, stored = false };
  __property TNumberingStyle2 NumberingStyle = { read = GetNumberingStyle, write = SetNumberingStyle, nodefault, stored = false };
  __property bool SkipNumbering = { read = GetSkipNumbering, write = SetSkipNumbering, nodefault, stored = false };
};
#pragma pack(pop)
#endif
//---------------------------------------------------------------------------
