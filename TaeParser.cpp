//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeParser.cpp - implementation file for routines used to substitute
// text in page headers and footers.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeUtility.h"
#include "TaeParser.h"
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// constants representing text that can/will be replaced by parser
//
const char NameOpt[] =     "{name}";
const char TitleOpt[] =    "{title}";
const char CDateOpt[] =    "{cdate}";
const char CTimeOpt[] =    "{ctime}";
const char MDateOpt[] =    "{mdate}";
const char MTimeOpt[] =    "{mtime}";
const char PageOpt[] =     "{page}";
const char PagesOpt[] =    "{pages}";
const char TodayOpt[] =    "{today}";
const char NowOpt[] =      "{now}";
const char OpenEscOpt[] =   "{{";
const char CloseEscOpt[] =  "}}";
const char UserNameOpt[] =  "{user}";
//---------------------------------------------------------------------------
// TTaeParserBase
//---------------------------------------------------------------------------
// TTaeParserBase constructor
//
TTaeParserBase::TTaeParserBase(void)
{
  FOptList = new TStringList;
  FSubstList = new TStringList;
}
//---------------------------------------------------------------------------
// TTaeParserBase destructor
//
TTaeParserBase::~TTaeParserBase()
{
  delete FOptList;
  delete FSubstList;
}
//---------------------------------------------------------------------------
// return the index of a string from FOptList
//
bool TTaeParserBase::Find(AnsiString option, int& i)
{
  i = FOptList->IndexOf(option);
  return i >= 0;
}
//---------------------------------------------------------------------------
// add a value to FOptList and a corresponding replacement value to
// FSubstList
//
void TTaeParserBase::Add(AnsiString option, AnsiString value)
{
  FOptList->Add(option);
  FSubstList->Add(value);
}
//---------------------------------------------------------------------------
// check whether FOptList has a value
//
bool TTaeParserBase::Has(AnsiString option)
{
  int i;
  return Find(option, i);
}
//---------------------------------------------------------------------------
// remove a string from both FOptList and FSubstList
//
bool TTaeParserBase::Remove(AnsiString option)
{
  int ndx;
  if (!Find(option, ndx)) return false;
  FOptList->Delete(ndx);
  FSubstList->Delete(ndx);
  return true;
}
//---------------------------------------------------------------------------
// locate a value in FOptList and change the corresponding value in
// FSubstList
//
bool TTaeParserBase::Change(AnsiString option, AnsiString newValue)
{
  int ndx;
  if (!Find(option, ndx)) return false;
  FSubstList->Strings[ndx] = newValue;
  return true;
}
//---------------------------------------------------------------------------
// check to see if the text in subStr matches the text the first characters in
// str for the length of subStr
//
bool TTaeParserBase::Compare(AnsiString str, AnsiString subStr, bool ignoreCase)
{
  AnsiString s = str.SubString(1, subStr.Length());
  if (ignoreCase) return !s.AnsiCompareIC(subStr);
  return !s.AnsiCompare(subStr);
}
//---------------------------------------------------------------------------
// locate the FOptList entry that matches the string in s; return -1 on no
// match
//
int TTaeParserBase::CompareFind(AnsiString s, bool ignoreCase)
{
  if (!s.Length()) return -1;
  int cnt = FOptList->Count;
  for (int i = 0; i < cnt; i++)
    if (Compare(s, FOptList->Strings[i], ignoreCase)) return i;
  return -1;
}
//---------------------------------------------------------------------------
// walk through a string (format) replacing text that matches FOptList
// entries with FSubstList values and return the new text string
//
AnsiString TTaeParserBase::Parse(AnsiString format, bool ignoreCase)
{
  AnsiString retVal;
  AnsiString subStr;
  int pos = 1;
  int len = format.Length();
  int ndx;

  while (pos <= len) {
    subStr = format.SubString(pos, len - pos + 1);
    ndx = CompareFind(subStr, ignoreCase);
    if (ndx < 0) {
      retVal = retVal + format.SubString(pos++, 1);
      continue;
      }
    retVal += FSubstList->Strings[ndx];
    pos += FOptList->Strings[ndx].Length();
    };

  return retVal;
}
//---------------------------------------------------------------------------
// TTaeParser - TTaeParserBase specialized for TTaeRichEdit printing
// and previewing code
//---------------------------------------------------------------------------
// TTaeParser constructor
//
TTaeParser::TTaeParser(AnsiString dateFormat, AnsiString timeFormat)
{
  FDateFormat = dateFormat;
  FTimeFormat = timeFormat;
  Add(OpenEscOpt, "{");
  Add(CloseEscOpt, "}");
  Add(NameOpt);
  Add(TitleOpt);
  Add(PageOpt, "0");
  Add(PagesOpt, "0");
  Add(CDateOpt);
  Add(CTimeOpt);
  SetCreated(TDateTime(0));
  Add(MDateOpt);
  Add(MTimeOpt);
  SetModified(TDateTime(0));
  Add(TodayOpt);
  Add(NowOpt);
  SetNow();

  Add(UserNameOpt, GetNetUser());
}
//---------------------------------------------------------------------------
// store the file name; if autoInit, then initialize other FSubstOpt values
// to rational values
//
// S.S. Note - YahCoLoRiZe now uses wide file-paths... but
// it gets the equivalent short-path (all ansi) to set tae's FileName
// property (which is really the full path)
void TTaeParser::SetName(AnsiString name, bool autoInit)
{
  // if not autoInit, simply copy name to the corresponding FSubstOpt
  // entry and return
  if (!autoInit)
  {
    Change(NameOpt, name);
    return;
  }

  // autoInit was true so save the file name and assume that it will
  // also be the file title
  AnsiString fileName = name;
  AnsiString title = name;

  // if the file name was not "(untitled)", initialize the corresponding
  // FSubstOpt values for the long file name (full path), title (file name
  // without path), creation date, last modified date, etc.
  if (name != AnsiString(TTaeRichEdit::Untitled)) // S.S. modified
  {
    bool bHaveFile = false;

    if (!FileExists(fileName))
    {
      name = GetLongFileName(fileName);

      if (FileExists(name))
      {
        fileName = name;
        bHaveFile = true;
      }
    }
    else
      bHaveFile = true;

    if (bHaveFile)
    {
      title = ExtractFileName(fileName);
      TW95FileData fileData(fileName);
      SetCreated(fileData.CreationTime());
      SetModified(fileData.LastWriteTime());
    }
  }

  Change(NameOpt, fileName);
  Change(TitleOpt, ExtractFileName(fileName));

  // we are still autoInit'ing, so set the parser's current date & time
  SetNow();
}
//---------------------------------------------------------------------------
// set the value to be substituted for "{title}"
//
void TTaeParser::SetTitle(AnsiString title)
{
  Change(TitleOpt, title);
}
//---------------------------------------------------------------------------
// set the value to be substituted for "{page}"
//
void TTaeParser::SetPage(int page)
{
  Change(PageOpt, AnsiString(page));
}
//---------------------------------------------------------------------------
// set the value to be substituted for "{pages}"
//
void TTaeParser::SetPages(int pages)
{
  Change(PagesOpt, AnsiString(pages));
}
//---------------------------------------------------------------------------
// set the values to be substituted for "{today}" and "{now}"
//
void TTaeParser::SetDateTime(AnsiString dateOpt, AnsiString timeOpt, TDateTime dateTime)
{
  Change(dateOpt, dateTime.FormatString(FDateFormat));
  Change(timeOpt, dateTime.FormatString(FTimeFormat));
}
//---------------------------------------------------------------------------
// set the values to be substituted for "{cdate}" and "{ctime}"
//
void TTaeParser::SetCreated(TDateTime dateTime)
{
  SetDateTime(CDateOpt, CTimeOpt, dateTime);
}
//---------------------------------------------------------------------------
// set the values to be substituted for "{mdate}" and "{mtime}"
//
void TTaeParser::SetModified(TDateTime dateTime)
{
  SetDateTime(MDateOpt, MTimeOpt, dateTime);
}
//---------------------------------------------------------------------------
// set the values substituted for "{today}" and "{now}" to the current date
// and time
//
void TTaeParser::SetNow(void)
{
  SetDateTime(TodayOpt, NowOpt, TDateTime::CurrentDateTime());
}
//---------------------------------------------------------------------------
