// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 2000 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeRichEditConv.h - header file for TaeRichEditConv.cpp (Microsoft RTF
// converter support classes).
//---------------------------------------------------------------------------
#ifndef TaeRichEditConvH
#define TaeRichEditConvH
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit;

// ERtfConverter - exception class
class ERtfConverter : public Exception
{
public:
  int Code;
  ERtfConverter(int code, AnsiString message) :
    Code(code), Exception(message)
    {};
};

// file conversion error code typedef
typedef short FCE;

// File Conversion Errors reported by converters
#define fceTrue        1    // true (used by IsFormatCorrect32() only)
#define fceNoErr        0    // success
#define fceOpenInFileErr  (-1)  // could not open input file
#define fceReadErr      (-2)  // error during read
#define fceOpenConvErr    (-3)  // error opening conversion file (obsolete - use fceOpenInFileErr)
#define fceWriteErr      (-4)  // error during write
#define fceInvalidFile    (-5)  // invalid data in conversion file
#define fceOpenExceptErr  (-6)  // error opening exception file (obsolete - use fceOpenInFileError)
#define fceWriteExceptErr  (-7)  // error writing exception file (obsolete - use fceWriteErr)
#define fceNoMemory      (-8)  // out of memory
#define fceInvalidDoc    (-9)  // invalid document (obsolete - use fceInvalidFile)
#define fceDiskFull      (-10)  // out of space on output (obsolete - use fceWriteErr)
#define fceDocTooLarge    (-11)  // conversion document too large for target (obsolete - use fceWriteErr)
#define fceOpenOutFileErr  (-12)  // could not open output file
#define fceUserCancel    (-13)  // conversion cancelled by user
#define fceWrongFileType  (-14)  // wrong file type for this converter

// RegisterApp() input flags
#define fRegAppPctComp      0x00000001  // app is prepared to accept percent complete numbers in export
#define fRegAppNoBinary      0x00000002  // app is not prepared to accept binary-encoded picture and other data
#define fRegAppPreview      0x00000004  // converter should display no message boxes or dialogs
#define fRegAppSupportNonOem  0x00000008  // app is prepared to provide non-OEM character set filenames
#define fRegAppIndexing      0x00000010  // converter can omit non-content Rtf

//---------------------------------------------------------------------------
// note: the following code assumes that only one converter will be loaded
// at a time.  it uses globals and is not designed to be thread-safe.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// TRtfConverter class - class to manage converters - only one instance of
// this class should exist!
//---------------------------------------------------------------------------
class PACKAGE TRtfConverter
{
protected:
  static bool FSemaphore;    // ensure that only one instance exists
  HINSTANCE FLibrary;      // library DLL instance

  void LoadLibrary(HWND hwnd, AnsiString libpath);
  void UnloadLibrary(void);

public:
  TRtfConverter(HWND hwnd, AnsiString libpath);
  ~TRtfConverter();
  FCE IsFormatCorrect(AnsiString filepath);
  AnsiString GetFormatClass(AnsiString filepath);
  FCE ForeignToRtf(TTaeRichEdit* richedit, AnsiString filepath,
    AnsiString formatClass = "");
  FCE RtfToForeign(TTaeRichEdit* richedit, AnsiString filepath,
    AnsiString formatClass = "");
};

//---------------------------------------------------------------------------
// TRtfConverterList class - class to retrieve needed information from the
// Registry
//---------------------------------------------------------------------------

class PACKAGE TRtfConverterList
{
protected:
  TStringList* FRawDescription;

public:
  TStringList* LibraryPath;    // library (DLL) paths
  TStringList* Description;    // descriptive text for each library
  TStringList* FormatClass;    // list of format classes
  TStringList* Filters;      // filters for each library
  AnsiString FilterList;      // a filter string suitable for use with
                  // TOpenDialog
  TRtfConverterList(bool import);
  ~TRtfConverterList();
  void GetConverterList(HKEY regRoot, AnsiString regPath, AnsiString appName,
    bool import);
};

#endif
//---------------------------------------------------------------------------
