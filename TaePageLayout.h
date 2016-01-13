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
// TaePageLayout.h - header file for TaePageLayout (classes involved
// in page formatting (headers, footers, etc.).
//---------------------------------------------------------------------------
#ifndef TaePageLayoutH
#define TaePageLayoutH

#include "TaeRegistry.h"

#include <vector>
//---------------------------------------------------------------------------
// TTaeHeaderText class -- used for headers *and * footers
//
class PACKAGE TTaeHeaderText : public TPersistent {
protected:
//public:
  TStringList* FLeft;
  TStringList* FCenter;
  TStringList* FRight;
  TFont* FFont;  // TTaeHeaderText owns only the font it creates -- others are TFont::Assign()'ed

  void __fastcall SetText(AnsiString text);
  void __fastcall SetLeft(TStringList* stringList) { SetStringList(FLeft, stringList); };
  void __fastcall SetCenter(TStringList* stringList) { SetStringList(FCenter, stringList); };
  void __fastcall SetRight(TStringList* stringList) { SetStringList(FRight, stringList); };
  void __fastcall SetStringList(TStringList* list, TStringList* newList);
  void __fastcall SetFont(TFont* font);

public:
  __fastcall TTaeHeaderText(void);
  virtual __fastcall ~TTaeHeaderText(void);
  __property AnsiString Text = { read = GetText, write = SetText };

  // assignment operator
  TTaeHeaderText& operator=(TTaeHeaderText& header);
  // copy constructor
  TTaeHeaderText(const TTaeHeaderText& header);
  AnsiString __fastcall GetText(void);

  void LoadFromRegistry(TRegistry& reg, AnsiString name);
  void SaveToRegistry(TRegistry&reg, AnsiString name);

__published:
  __property TStringList* Left = { read = FLeft, write = SetLeft, nodefault };
  __property TStringList* Center = { read = FCenter, write = SetCenter, nodefault };
  __property TStringList* Right = { read = FRight, write = SetRight, nodefault };
  __property TFont* Font = { read = FFont, write = SetFont, nodefault };
};

// TTaeBorderLine enum -- used to specify borders for headers, footers
// and body text
typedef enum { blHeaderLeft, blHeaderTop, blHeaderRight, blHeaderBottom,
  blBodyLeft, blBodyTop, blBodyRight, blBodyBottom,
  blFooterLeft, blFooterTop, blFooterRight, blFooterBottom
  } TTaeBorderLine;
typedef Set<TTaeBorderLine, blHeaderLeft, blFooterBottom> TTaeBorderLines;

// TTaePageStyle -- aggregates header & footer text, border styles, and
// margins for pages.  used to define attributes for first page and
// all remaining pages.
class PACKAGE TTaePageStyle {
protected:
public:
  AnsiString FName;
  TTaeHeaderText* FFirstHeader;
  TTaeHeaderText* FFirstFooter;
  TTaeHeaderText* FHeader;
  TTaeHeaderText* FFooter;
  TTaeBorderLines FBorderLines;
  int FBorderWidth;
  int FInsideMargin;
  bool FDifferentFirstPage;

  TTaePageStyle(void);
  ~TTaePageStyle();

  // assignment operator
  TTaePageStyle& operator=(TTaePageStyle& style);
  // copy constructor
  TTaePageStyle(const TTaePageStyle& style);

  void LoadFromRegistry(TRegistry& reg, AnsiString name);
  void SaveToRegistry(TRegistry& reg, AnsiString name);
  static void RemoveFromRegistry(TRegistry& reg, AnsiString name);
};

// TTaePageStyles list typedef -- used to store a list of program-defined or
// user-defined page styles.  typically presented to users for selection of
// style to use for the current document, for example.
typedef std::vector<TTaePageStyle*> TTaePageStyles;

// TTaePageStyleList class -- manages a list of page styles
class PACKAGE TTaePageStyleList {
protected:
  AnsiString FCurrentStyleName;
  TTaePageStyles FPageStyles;

  TTaePageStyle* GetCurrentStyle(void) { return Find(FCurrentStyleName); };
  void SetCurrentStyleName(AnsiString name) {
    if (name == "") return;
    if (Find(name)) FCurrentStyleName = name;
    };

public:
  TTaePageStyleList(void);
  ~TTaePageStyleList();
  void Clear(void);
  void Add(TTaePageStyle& style);
  TTaePageStyle* Find(AnsiString styleName);
  void Change(TTaePageStyle& style, bool add = false);
  void Delete(AnsiString styleName);
  bool StyleExists(AnsiString styleName);
  int Count(void);
  AnsiString StyleName(int index);

  void LoadFromRegistry(TRegistry& reg, AnsiString name);
  void SaveToRegistry(TRegistry& reg, AnsiString name);

  __property AnsiString CurrentStyleName = { read = FCurrentStyleName,
    write = SetCurrentStyleName };
  __property TTaePageStyle* CurrentStyle = { read = GetCurrentStyle };
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
