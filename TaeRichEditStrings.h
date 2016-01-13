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
// TaeRichEditStrings.h - header file for the TTaeRichEditStrings class.
//---------------------------------------------------------------------------
#ifndef TaeRichEditStringsH
#define TaeRichEditStringsH
#include <comctrls.hpp>
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit;

#pragma pack(push, 4)

// the VCL hard-codes 4k buffers in various parts of the TRichEditStrings
// class.  change the following to set the buffer size.  warning:  AnsiString
// may also have such limits, in which case large buffers here will not solve
// the problem.  I do not think this is the case, but I do not really know.
//
//#define STRINGBUFSIZE (0x8000) /* 32k */
#define STRINGBUFSIZE (0x1000) /* 4k */

class PACKAGE TTaeRichEditStrings : public TStrings
{
friend class PACKAGE TTaeRichEdit;

protected:
  TTaeRichEdit* FRichEdit;
  bool FPlainText;
  TConversion* FConverter;

  void EnableChange(const bool Value);
  virtual AnsiString __fastcall Get(int Index);
  virtual int __fastcall GetCount(void);
  virtual void __fastcall Put(int Index, const AnsiString S);
  virtual void __fastcall SetUpdateState(bool Updating);
  virtual void __fastcall SetTextStr(const AnsiString Value);

public:
  __fastcall TTaeRichEditStrings(void);
  virtual __fastcall ~TTaeRichEditStrings(void);
  virtual void __fastcall Clear(void);
  virtual void __fastcall AddStrings(TStrings* Strings);
  virtual void __fastcall Delete(int Index);
  virtual void __fastcall Insert(int Index, const AnsiString S);
  virtual void __fastcall LoadFromFile(const AnsiString FileName);
  virtual void __fastcall LoadFromStream(TStream* Stream);
  virtual void __fastcall SaveToFile(const AnsiString FileName);
  virtual void __fastcall SaveToStream(TStream* Stream);
  static void __fastcall RegisterConversionFormat(System::TMetaClass* vmt,
    const System::AnsiString AExtension, System::TMetaClass* AConversionClass);

  __property bool PlainText = { read = FPlainText, write = FPlainText,
    nodefault };
};
#pragma pack(pop)
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
