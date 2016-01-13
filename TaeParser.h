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
// TaeParser.h - header file for TaeParser.cpp (routines used to substitute
// text in page headers and footers).
//---------------------------------------------------------------------------
#ifndef TaeParserH
#define TaeParserH

#include <vcl/sysdefs.h>
#include <vcl/dstring.h>

//---------------------------------------------------------------------------
// TTaeParserBase - base class for text substitution parser
//
class PACKAGE TTaeParserBase {
protected:
  TStringList* FOptList;
  TStringList* FSubstList;

  bool Find(AnsiString option, int& i);

public:
  TTaeParserBase(void);
  ~TTaeParserBase();
  void Add(AnsiString option, AnsiString value = "");
  bool Has(AnsiString option);
  bool Remove(AnsiString option);
  bool Change(AnsiString option, AnsiString newValue);
  bool Compare(AnsiString str, AnsiString subStr, bool ignoreCase);
  int CompareFind(AnsiString s, bool ignoreCase);
  virtual AnsiString Parse(AnsiString format, bool ignoreCase = true);
};

// TTaeParser class - main text substitution parser class
class PACKAGE TTaeParser : public TTaeParserBase
{
protected:
  AnsiString FDateFormat;
  AnsiString FTimeFormat;

  void SetDateTime(AnsiString dateOpt, AnsiString timeOpt, TDateTime dateTime);

public:
  TTaeParser(AnsiString dateFormat, AnsiString timeFormat);
  void SetName(AnsiString name, bool autoInit = true);
  void SetTitle(AnsiString title);
  void SetPage(int page);
  void SetPages(int pages);
  void SetCreated(TDateTime dateTime);
  void SetModified(TDateTime dateTime);
  void SetNow(void);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
