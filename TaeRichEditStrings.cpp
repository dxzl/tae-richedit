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
// TaeRichEditStrings.cpp - implementation file for the TTaeRichEditStrings
// class.
//---------------------------------------------------------------------------
// need to add to all functions -- test for valid RE handle???
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeRichEditStrings.h"
#include "TaeRichEdit.h"
#include "TaeAdjLineBrks.h"
#include "TaeUtility.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma pack(push, 4)
//---------------------------------------------------------------------------

const int ReadError = 0x0001;
const int WriteError = 0x0002;
const int NoError = 0x0000;

typedef struct tagTSelection {
  int StartPos;
  int EndPos;
  } TSelection;

TConversionFormat RTFConversionFormat;
TConversionFormat TextConversionFormat;

PConversionFormat ConversionFormatList = &TextConversionFormat;
//---------------------------------------------------------------------------
// helper function to create and initialize a TMetaClass instance
//
TObject* CreateMetaClassObject(TMetaClass* metaClass)
{
  int objSize = TObject::InstanceSize(metaClass);
  TObject* obj = (TObject*) new char[objSize];
  TObject::InitInstance(metaClass, obj);
  return obj;
}
//---------------------------------------------------------------------------
// EM_STREAMIN callback
//
DWORD CALLBACK StreamSave(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
{
  PRichEditStreamInfo StreamInfo;

  DWORD Result = NoError;
  StreamInfo = (PRichEditStreamInfo) dwCookie;

  try {
    *pcb = 0;
    if (StreamInfo->Converter)
      *pcb = StreamInfo->Converter->ConvertWriteStream(StreamInfo->Stream,
        (char*) pbBuff, cb);
    }
  catch (...) {
    Result = WriteError;
    }
  return Result;
}
//---------------------------------------------------------------------------
// EM_STEAMOUT callback
//
DWORD CALLBACK StreamLoad(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
{
  LPBYTE Buffer, pBuff;
  PRichEditStreamInfo StreamInfo;

  DWORD Result = NoError;
  StreamInfo = (PRichEditStreamInfo) dwCookie;
  Buffer = new unsigned char[cb + 1];

  try {
    cb /= 2;
    *pcb = 0;
    pBuff = Buffer + cb;
    try {
      if (StreamInfo->Converter)
        *pcb = StreamInfo->Converter->ConvertReadStream(StreamInfo->Stream,
          (char*) pBuff, cb);
      if (*pcb > 0) {
        pBuff[*pcb] = 0;
        if (pBuff[*pcb - 1] == '\r') pBuff[*pcb - 1] = '\0';
        *pcb = TaeAdjustLineBreaks((unsigned char*) Buffer,
                                                   (unsigned char*) pBuff);
        ::memmove(pbBuff, Buffer, *pcb);
        }
      }
    catch (...) {
      Result = ReadError;
      }
    }
  __finally {
    delete[] Buffer;
    }

  return Result;
}
//--------------------------------------------------------------------------
// TTaeRichEditStrings constructor
//
__fastcall TTaeRichEditStrings::TTaeRichEditStrings(void) : TStrings()
{
  // initialize class data
  FRichEdit = 0;
  FPlainText = false;
  FConverter = 0;

  // we cannot initialize the conversion format table statically, so we
  // do it here....
  // note that, like the VCL implementation, once you have registered a
  // conversion format, it is never deleted (so memory leaks).  if you
  // register a conversion more than once, the most recently registered
  // version will be used.  however, like the VCL implementation, older
  // conversion formats which match the same file extension are never
  // deleted....
  // this code assumes that, if RTFConversionFormat.ConversionClass is
  // not null, then the initialization has already occurred and need not
  // be repeated.
  if (!RTFConversionFormat.ConversionClass) {
    RTFConversionFormat.ConversionClass = __classid(TConversion);
    RTFConversionFormat.Extension = "rtf";
    RTFConversionFormat.Next = 0;
    TextConversionFormat.ConversionClass = __classid(TConversion);
    TextConversionFormat.Extension = "txt";
    TextConversionFormat.Next = &RTFConversionFormat;
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings destructor
//
__fastcall TTaeRichEditStrings::~TTaeRichEditStrings(void)
{
  if (FConverter) delete FConverter;
  FConverter = 0;
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::EnableChange() member
//
void TTaeRichEditStrings::EnableChange(const bool Value)
{
  int EventMask;

  EventMask = ::SendMessage(FRichEdit->Handle, EM_GETEVENTMASK, 0, 0);
  if (Value) EventMask |= ENM_CHANGE;
  else EventMask &= ~ENM_CHANGE;

  ::SendMessage(FRichEdit->Handle, EM_SETEVENTMASK, 0, EventMask);
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::Get() member
//
AnsiString __fastcall TTaeRichEditStrings::Get(int Index)
{
  union {
    WORD maxBytes;
    char text[STRINGBUFSIZE];
    } buf;
  int L;

  buf.maxBytes = (WORD) (sizeof(buf.text) - 1);
  L = ::SendMessage(FRichEdit->Handle, EM_GETLINE, Index, (LPARAM) &buf.text);
  if (L && (buf.text[L - 1] == 0x0d || buf.text[L - 1] == 0x0a)) L--;
  if (L && (buf.text[L - 1] == 0x0d || buf.text[L - 1] == 0x0a)) L--;
  buf.text[L] = '\0';
  AnsiString s = buf.text;
  return s;
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::GetCount() member
//
int __fastcall TTaeRichEditStrings::GetCount(void)
{
  // determine lines based upon accurate information returned by newer
  // WinAPI calls as adjusted for the compiled RE version...
  int NumLines = ::SendMessage(FRichEdit->Handle, EM_EXLINEFROMCHAR, 0,
                                              FRichEdit->GetTextLen()) + 1;

  int CharIndex = ::SendMessage(FRichEdit->Handle, EM_LINEINDEX,
                                                      NumLines-1, 0);

  if (!::SendMessage(FRichEdit->Handle, EM_LINELENGTH, CharIndex, 0))
    NumLines--;

  return NumLines;
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::Put() member
//
void __fastcall TTaeRichEditStrings::Put(int Index, const AnsiString S)
{
  TCharRange Selection;

  if (Index >= 0) {
    Selection.cpMin = ::SendMessage(FRichEdit->Handle, EM_LINEINDEX, Index, 0);
    if (Selection.cpMin != -1) {
      Selection.cpMax = Selection.cpMin +
        ::SendMessage(FRichEdit->Handle, EM_LINELENGTH, Selection.cpMin, 0);
      ::SendMessage(FRichEdit->Handle, EM_EXSETSEL, 0, (LPARAM) &Selection);
      ::SendMessage(FRichEdit->Handle, EM_REPLACESEL, 0, (LPARAM) S.c_str());
      }
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::SetUpdateState() member
//
void __fastcall TTaeRichEditStrings::SetUpdateState(bool Updating)
{
  if (FRichEdit->Showing)
    ::SendMessage(FRichEdit->Handle, WM_SETREDRAW, !Updating, 0);
  if (!Updating) {
    FRichEdit->Refresh();
    FRichEdit->Perform(CM_TEXTCHANGED, 0, 0);
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::SetTextStr() member
//
void __fastcall TTaeRichEditStrings::SetTextStr(const AnsiString Value)
{
  EnableChange(False);
  try {
    TStrings::SetTextStr(Value);
    }
  __finally {
    EnableChange(True);
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::Clear() member
//
void __fastcall TTaeRichEditStrings::Clear(void)
{
  FRichEdit->Clear();
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::AddStrings() member
//
void __fastcall TTaeRichEditStrings::AddStrings(TStrings* Strings)
{
  TNotifyEvent SelChange;

  // disable OnSelectionChange temporarily
  SelChange = FRichEdit->OnSelectionChange;
  FRichEdit->OnSelectionChange = 0;

  // try to add strings to TStrings
  try {
    TStrings::AddStrings(Strings);
    }

  // restore OnSelectionChange
  __finally {
    FRichEdit->OnSelectionChange = SelChange;
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::Delete() member
//
void __fastcall TTaeRichEditStrings::Delete(int Index)
{
  static const char* Empty = "";
  TCharRange Selection;

  // argh!  return silently on invalid index
  if (Index < 0) return;
  Selection.cpMin = ::SendMessage(FRichEdit->Handle, EM_LINEINDEX, Index, 0);
  if (Selection.cpMin != -1) {
    Selection.cpMax = SendMessage(FRichEdit->Handle, EM_LINEINDEX, Index + 1, 0);
    if (Selection.cpMax == -1) Selection.cpMax = Selection.cpMin +
        ::SendMessage(FRichEdit->Handle, EM_LINELENGTH, Selection.cpMin, 0);
    ::SendMessage(FRichEdit->Handle, EM_EXSETSEL, 0, (LPARAM) &Selection);
    ::SendMessage(FRichEdit->Handle, EM_REPLACESEL, 0, (LPARAM) Empty);
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::Insert() member (and the primary reason that this
// class exists)
//
void __fastcall TTaeRichEditStrings::Insert(int Index, const AnsiString S)
{
  int L;
  TCharRange Selection;
  AnsiString Str;

  if (Index >= 0) {
    Selection.cpMin = ::SendMessage(FRichEdit->Handle, EM_LINEINDEX, Index, 0);
    if (Selection.cpMin >= 0) Str = S + "\r\n";
    else {
      Selection.cpMin =
        ::SendMessage(FRichEdit->Handle, EM_LINEINDEX, Index - 1, 0);
      // if we asked to insert beyond eof, simply return!?!
      // should we not insert at eof anyway?
      if (Selection.cpMin < 0) return;
      L = ::SendMessage(FRichEdit->Handle, EM_LINELENGTH, Selection.cpMin, 0);
      if (!L) return;    // now, this I do not get.  again we return....
      Selection.cpMin +=  L;
      Str = "\r\n" + S;
      }
    Selection.cpMax = Selection.cpMin;
    ::SendMessage(FRichEdit->Handle, EM_EXSETSEL, 0, (LPARAM) &Selection);
    ::SendMessage(FRichEdit->Handle, EM_REPLACESEL, 0, (LPARAM) Str.c_str());
//    // need to fix the following for RE 1.0 vs RE 2.0+
//    if (FRichEdit->SelStart != (Selection.cpMax + Str.Length()))
//      throw EOutOfResources("Insufficient memory for operation.");
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::LoadFromFile() member
//
void __fastcall TTaeRichEditStrings::LoadFromFile(const AnsiString FileName)
{
  AnsiString fileName = GetLongFileName(FileName);

  if (FRichEdit->FGuessFont) {
    TTaeRichEdit::TTaeFont* font;
    if (FRichEdit->IsRtfFile(fileName)) font = FRichEdit->FRtfTextFont;
    else font = FRichEdit->FPlainTextFont;
    FRichEdit->DefAttributes->Color = font->Color;
    FRichEdit->DefAttributes->Name = font->Name;
    FRichEdit->DefAttributes->Pitch = font->Pitch;
    FRichEdit->DefAttributes->Size = font->Size;
    FRichEdit->DefAttributes->Style = font->Style;
    }

  AnsiString Ext;
  PConversionFormat Convert;

  Ext = AnsiLowerCaseFileName(ExtractFileExt(FileName));
  Ext.Delete(1, 1);
  Convert = ConversionFormatList;
  while (Convert) {
    if (Convert->Extension.AnsiCompare(Ext)) Convert = Convert->Next;
    else break;
    }
  if (!Convert) Convert = &TextConversionFormat;
  if (!FConverter) FConverter =
    (TConversion*) CreateMetaClassObject(Convert->ConversionClass);

  try {
    TStrings::LoadFromFile(fileName);
    }
  catch (...) {
    delete FConverter;
    FConverter = 0;
    throw EOutOfResources("Load Failure");
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::LoadFromStream() member
//
void __fastcall TTaeRichEditStrings::LoadFromStream(TStream* Stream)
{
  TEditStream EditStream;
  int Position;
  int TextType;
  TRichEditStreamInfo StreamInfo;
  TConversion* Converter;

  StreamInfo.Stream = Stream;
  if (FConverter) Converter = FConverter;
  else Converter = (TConversion*) CreateMetaClassObject(FRichEdit->DefaultConverter);
  StreamInfo.Converter = Converter;

  try {
    EditStream.dwCookie = (unsigned long) &StreamInfo;
    EditStream.pfnCallback = StreamLoad;
    EditStream.dwError = 0;
    Position = Stream->Position;
    TextType = PlainText ? SF_TEXT : SF_RTF;
    ::SendMessage(FRichEdit->Handle, EM_STREAMIN, TextType, (LPARAM) &EditStream);
    if (TextType == SF_RTF && EditStream.dwError) {
      Stream->Position = Position;
      TextType = PlainText ? SF_RTF : SF_TEXT;  // ???
      ::SendMessage(FRichEdit->Handle, EM_STREAMIN, TextType, (LPARAM) &EditStream);
      if (EditStream.dwError) throw EOutOfResources("Load Failure");
      }
    }
  __finally {
    if (!FConverter) delete Converter;
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::SaveToFile() member
//
void __fastcall TTaeRichEditStrings::SaveToFile(const AnsiString FileName)
{
  AnsiString fileName = GetLongFileName(FileName);
  AnsiString Ext;
  PConversionFormat Convert;

  Ext = AnsiLowerCaseFileName(ExtractFileExt(FileName));
  Ext.Delete(1, 1);
  Convert = ConversionFormatList;
  while (Convert) {
    if (Convert->Extension.AnsiCompare(Ext)) Convert = Convert->Next;
    else break;
    }
  if (!Convert) Convert = &TextConversionFormat;
  if (!FConverter) FConverter =
    (TConversion*) CreateMetaClassObject(Convert->ConversionClass);

  try {
    TStrings::SaveToFile(fileName);
    }
  catch (...) {
    delete FConverter;
    FConverter = 0;
    throw EOutOfResources("Save failure");
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::SaveToStream() member
//
void __fastcall TTaeRichEditStrings::SaveToStream(TStream* Stream)
{
  TEditStream EditStream;
  int TextType;
  TRichEditStreamInfo StreamInfo;
  TConversion* Converter;

  if (FConverter) Converter = FConverter;
  else Converter = (TConversion*) CreateMetaClassObject(FRichEdit->DefaultConverter);
  StreamInfo.Stream = Stream;
  StreamInfo.Converter = Converter;
  try {
    EditStream.dwCookie = (unsigned long) &StreamInfo;
    EditStream.pfnCallback = StreamSave;
    EditStream.dwError = 0;
    TextType = PlainText ? SF_TEXT : SF_RTF;
    ::SendMessage(FRichEdit->Handle, EM_STREAMOUT, TextType, (LPARAM) &EditStream);
    if (EditStream.dwError) throw EOutOfResources("Save failure");
    }
  __finally {
    if (!FConverter) delete Converter;
    }
}
//---------------------------------------------------------------------------
// TTaeRichEditStrings::RegeisterConversionFormat() member
//
// (note: the conversion format functions are both sticky and brain-dead.)
//
void __fastcall  TTaeRichEditStrings::RegisterConversionFormat(System::TMetaClass* vmt,
  const System::AnsiString AExtension, System::TMetaClass* AConversionClass)
{
  TConversionFormat* newRec = new TConversionFormat();
  newRec->ConversionClass = AConversionClass;
  newRec->Extension = AExtension;
  newRec->Next = ConversionFormatList;
  ConversionFormatList = newRec;
}
//---------------------------------------------------------------------------
#pragma pack(pop)
//---------------------------------------------------------------------------
