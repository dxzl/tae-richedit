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
// TaeRichEditConv.cpp - implementation file for Microsoft RTF converter
// support classes.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeRichEditConv.h"
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

const DWORD BUFFSIZE = 0x4000;  // 16k buffer (arbitrary)

// callback type - converter calls whenever it needs to return a buffer of RTF
// cchBuff - count of bytes in buffer
// nPercent - percent of completion (ranges 0 to 100)
// returns a negative value to cancel processing or zero to continue
typedef long (PASCAL *PFN_RTF_OUT)(long cchBuff, long nPercent);

// callback type - converter calls whenever it needs to get a buffer of RTF
// rgfOptions
// returns a negative value to cancel processing or zero to continue
// note: to get percent completion, the caller must call RegisterApp with
// the fRegAppPctComp bit set and pass a handle to a PCVT structure at the
// start of the ghBuff
typedef long (PASCAL *PFN_RTF_IN)(long rgfOptions, long /* 0 (unused) */);

// percent completion structure - see PFN_RTF above
typedef struct {
  short cbpcft;    // size of this structure
  short wVersion;    // structure version number
  short wPctApp;    // current % complete per application
  short wPctConvtr;  // current % complete per converter
  } PCVT;

// initialize converter (required) - this is the absolute first thing called
// hWnd = top-level client window of caller; focus gets returned to this window
// szModule = caller's module name; used by converter to modify its behavior
//     for different calling applications
// returns non-zero on success, zero on failure
typedef long PASCAL InitConverter32(HANDLE hWnd, char *szModule);

// terminate the converter (optional)
typedef void PASCAL UninitConverter(void);

// check the format of a file (required)
// ghszFile - handle to string with fully qualified file path to be checked
// ghszClass - if file format is recognized, returns handle to class name
// returns 1 (fceTrue) on success; any other value indicates failure
typedef FCE PASCAL IsFormatCorrect32(HANDLE ghszFile, HANDLE ghszClass);

// read binary file or docfile stream and convert to RTF stream (required)
// ghszFile - name of file to convert or NULL if input comes from embedding
// pstgForeign - pointer to an open OLE 2 IStorage (caller responsible for
//    releasing after processing) of the embedding or NULL for file
// ghBuff - handle to destination buffer for RTF
// ghszClass - string to return the write class name to the caller; NULL if
//    caller does not care
// ghszSubset - string identifying the subset of the file to process; meaning
//    is determined by converter; for example, could be an Excel named range
// lpfnOut - pointer to callback that converter calls whenever ready to
//    return a chunk of RTF to the caller (see PFN_RTF above)
// returns zero on success; if cancelled in lpfnOut, the return value from
//    lpfnOut is returned by ForeignToRtf32
typedef FCE PASCAL ForeignToRtf32(HANDLE ghszFile, void *pstgForeign,
  HANDLE ghBuff, HANDLE ghszClass, HANDLE ghszSubset, PFN_RTF_OUT lpfnOut);

// read RTF stream and convert to binary file or docfile stream (optional!)
// ghszFile - name of destination file or NULL for embedding
// pstgForeign - IStorage interface for embedding or NULL for file
// ghBuff - buffer from which the converter will read the RTF
// ghshClass - string with class name for desired format
// lpfnIn - callback that converter calls whenever it needs more RTF to
//     convert
// returns zero on success
typedef FCE PASCAL RtfToForeign32(HANDLE ghszFile, void *pstgForeign,
  HANDLE ghBuff, HANDLE ghshClass, PFN_RTF_IN lpfnIn);

// parse free-form options from caller and return converter's free-form
// options to caller (strongly recommended)
// lFlags - see fRegAppxxx flags below
// lpFuture - pointer to a list of application preferences; NULL is ok
typedef HGLOBAL PASCAL RegisterApp(unsigned long lFlags, void FAR *lpFuture);

// fetch a text description of a converter error code (optional)
// fce - error code previously returned by the converter
// lpszError - buffer into which the error description is to be copied
// cb - size of lpszError (bytes)
// returns zero if buffer is too small or fce is unrecognized; otherwise,
// returns number of bytes copied into buffer
typedef long PASCAL CchFetchLpszError(long fce, char FAR *lpszError, long cb);

// fetch import/export class names, description strings, and file extensions
// for the converter (both optional)
// haszClass - returns a list of class names recognized by converter
// haszDescrip - returns descriptive strings corresponding to haszClass values
// haszExt - returns a list of file name extensions corresponding to haszClass
// note: these lists are single-null-terminated after each entry and double-null-
// terminated at end of list (e.g., "entry1\0entry2\0\0")
typedef void PASCAL GetReadNames(HANDLE haszClass, HANDLE haszDescrip, HANDLE haszExt);
typedef void PASCAL GetWriteNames(HANDLE haszClass, HANDLE haszDescrip, HANDLE haszExt);

// make all registry entries for this converter using the same values
// available through GetReadNames() and GetWriteNames() (optional)
typedef long PASCAL FRegisterConverter(HANDLE hkeyRoot);

// note file names passed to the converter are assumed to be in OEM character
// set unless explicitly set using RegisterApp.  (internally, the converter
// will convert to ANSI before calling Windows API.)
//===========================================================================

//---------------------------------------------------------------------------
// note: the following code assumes that only one converter will be loaded
// at a time.  it uses globals and is not designed to be thread-safe.
//---------------------------------------------------------------------------

// global pointers to DLL exports
InitConverter32* pInitConverter32 = 0;
UninitConverter* pUninitConverter = 0;
IsFormatCorrect32* pIsFormatCorrect32 = 0;
ForeignToRtf32* pForeignToRtf32 = 0;
RtfToForeign32* pRtfToForeign32 = 0;
RegisterApp* pRegisterApp = 0;
CchFetchLpszError* pCchFetchLpszError = 0;
GetReadNames* pGetReadNames = 0;
GetWriteNames* pGetWriteNames = 0;
FRegisterConverter* pFRegisterConverter = 0;

//---------------------------------------------------------------------------
// TRtfConverter class
//---------------------------------------------------------------------------
bool TRtfConverter::FSemaphore = false;

// TRtfConverter() - ctor
// hwnd - handle to the top level window that will parent any dialog boxes
//    created by the converter
// libpath - the complete path to the converter DLL
// note: do not instantiate more than one instance of this class at a time;
// this is a design concession to avoid having to code up callback functions
// that are also member functions (but not static); anyway, this is not
// really a big deal...
//
TRtfConverter::TRtfConverter(HWND hwnd, AnsiString libpath) : FLibrary(0)
{
  // make sure that there is only one instance; throw an ugly exception if not
  if (FSemaphore)
    throw Exception("Program attempted to create "
      "multiple instances of the TRtfConverter class.");
  FSemaphore = true;

  // load the library
  LoadLibrary(hwnd, libpath);
}

//---------------------------------------------------------------------------
// ~TRtfConverter() - dtor
// unload the library
//
TRtfConverter::~TRtfConverter()
{
  UnloadLibrary();
  FSemaphore = false;
}

//---------------------------------------------------------------------------
// LoadLibrary() - loads the converter DLL and retrieves the entry points; if
// unable to load and initialize the library or if required entry points are
// not exported, throws an exception
// hwnd - handle to the parent window
// libpath - full path to DLL to load
//
void TRtfConverter::LoadLibrary(HWND hwnd, AnsiString libpath)
{
  // attempt to load library
  HINSTANCE hinst = ::LoadLibrary(libpath.c_str());
  if (!hinst) throw Exception("Unable to load library");

  // library loaded ok
  FLibrary = hinst;

  // get entry points for calls
  pInitConverter32 = (InitConverter32*) ::GetProcAddress(FLibrary, "InitConverter32");
  pUninitConverter = (UninitConverter*) ::GetProcAddress(FLibrary, "UninitConverter");
  pIsFormatCorrect32 = (IsFormatCorrect32*) ::GetProcAddress(FLibrary, "IsFormatCorrect32");
  pForeignToRtf32 = (ForeignToRtf32*) ::GetProcAddress(FLibrary, "ForeignToRtf32");
  pRtfToForeign32 = (RtfToForeign32*) ::GetProcAddress(FLibrary, "RtfToForeign32");
  pRegisterApp = (RegisterApp*) ::GetProcAddress(FLibrary, "RegisterApp");
  pCchFetchLpszError = (CchFetchLpszError*) ::GetProcAddress(FLibrary, "CchFetchLpszError");
  pGetReadNames = (GetReadNames*) ::GetProcAddress(FLibrary, "GetReadNames");
  pGetWriteNames = (GetWriteNames*) ::GetProcAddress(FLibrary, "GetWriteNames");
  pFRegisterConverter = (FRegisterConverter*) ::GetProcAddress(FLibrary, "FRegisterConverter");

  // verify that required entry points are available
  if (!pInitConverter32 || !pIsFormatCorrect32 || !pForeignToRtf32) {
    UnloadLibrary();
    throw Exception("Unable to initialize library (required entry points missing)");
    }

  // initialize converter - if cannot, unload and return failure
  if (!pInitConverter32(hwnd, Application->ExeName.c_str())) {
    UnloadLibrary();
    throw Exception("Unable to initialize library (initialization failed)");
    }
}

//---------------------------------------------------------------------------
// UnloadLibrary() - uninitialized the library, unloads it, and clears
// globals (can you say, "paranoia?")
//
void TRtfConverter::UnloadLibrary(void)
{
  // if library loaded and pUninitConverter is exported, uninitialize the library
  if (FLibrary && pUninitConverter) (pUninitConverter)();

  // if the library is loaded, free it
  if (FLibrary) ::FreeLibrary(FLibrary);

  // clear FLibrary and globals
  FLibrary = 0;
  pInitConverter32 = 0;
  pUninitConverter = 0;
  pIsFormatCorrect32 = 0;
  pForeignToRtf32 = 0;
  pRtfToForeign32 = 0;
  pRegisterApp = 0;
  pCchFetchLpszError = 0;
  pGetReadNames = 0;
  pGetWriteNames = 0;
  pFRegisterConverter = 0;
}

//---------------------------------------------------------------------------
// GlobalAllocString() - utility function to allocate global storage and
// put a string in it
// s - string to put in global storage
//
HGLOBAL GlobalAllocString(AnsiString s)
{
  // allocate a block of storage large enough for string
  HGLOBAL hgsz = ::GlobalAlloc(GMEM_MOVEABLE, s.Length() + 1);
  if (!hgsz) return 0;

  // lock the storage and copy the string into it
  char* p = (char*) ::GlobalLock(hgsz);
  if (!p) {
    ::GlobalFree(hgsz);
    return 0;
    }
  ::lstrcpy(p, s.c_str());

  // unlock the storage and return the global handle
  ::GlobalUnlock(hgsz);
  return hgsz;
}

//---------------------------------------------------------------------------
// IsFormatCorrect() - verifies that a given file can be converted by the
// library; see also the somewhat more useful GetFormatClass() below
// filepath - file to test
// returns fceNoErr on success; all else is failure
//
FCE TRtfConverter::IsFormatCorrect(AnsiString filepath)
{
  // allocate global storage for filepath and formatclass
  HGLOBAL hgszFile, hgszClass;
  hgszFile = GlobalAllocString(filepath);
  if (!hgszFile) return fceNoMemory;
  hgszClass = ::GlobalAllocString("");
  if (!hgszClass) {
    ::GlobalFree(hgszFile);
    return fceNoMemory;
    }

  // test the file
  FCE fce = pIsFormatCorrect32(hgszFile, hgszClass);

  // free the global storage and return
  ::GlobalFree(hgszFile);
  ::GlobalFree(hgszClass);
  return fce;
}

//---------------------------------------------------------------------------
// GetFormatClass() - tests a file to see if it is a recognized format and, if
// so, returns the format class; throws an exception if not
// filepath - file to test
//
AnsiString TRtfConverter::GetFormatClass(AnsiString filepath)
{
  // allocate global storage for filepath and format class
  HGLOBAL hgszFile, hgszClass;
  hgszFile = GlobalAllocString(filepath);
  if (!hgszFile) throw ERtfConverter(fceNoMemory, "Insufficient memory");
  hgszClass = ::GlobalAllocString("");
  if (!hgszClass) {
    ::GlobalFree(hgszFile);
    throw ERtfConverter(fceNoMemory, "Insufficient memory");
    }

  // test the file
  FCE fce = pIsFormatCorrect32(hgszFile, hgszClass);

  // on failure, free global storage and throw exception
  if (fce != fceTrue) {
    ::GlobalFree(hgszFile);
    ::GlobalFree(hgszClass);
    AnsiString msg("Format invalid");
    if (pCchFetchLpszError) {
      char buf[256];  // arbitrary -- if not long enough, empty string returned
      if (pCchFetchLpszError(fce, buf, sizeof(buf) / sizeof(buf[0])))
        msg = AnsiString(buf);
      }
    throw ERtfConverter(fce, msg);
    }

  // copy the class name into local storage and free global storage
  char* p = (char*) ::GlobalLock(hgszClass);
  AnsiString sClass(p);
  ::GlobalUnlock(hgszClass);
  ::GlobalFree(hgszFile);
  ::GlobalFree(hgszClass);

  // return format class name
  return sClass;
}

//---------------------------------------------------------------------------
// more globals...
//
//bool TRtfConverter::FSemaphore = false;
HGLOBAL ghBuff = 0;
TMemoryStream* mstream = 0;

//---------------------------------------------------------------------------
// RtfOut - global function callback used by ForeignToRtf() below; when called,
// copies the specified number of bytes into the global memory stream
// cchBuff - count of bytes to copy
// returns 0 on success; all else is failure
//
long PASCAL RtfOut(long cchBuff, long /*nPercent - we do not use */)
{
  long retval = 0;  // assume success
  char* p = (char*) ::GlobalLock(ghBuff);
  if (!p) retval = fceNoMemory;
  else {
    try {
      mstream->Write(p, cchBuff);
      }
    catch (...) {
      retval = fceNoMemory; // what to return???
      }
    ::GlobalUnlock(ghBuff);
    }
  return retval;
}

//---------------------------------------------------------------------------
// ForeignToRtf() - import a file into a Rich Edit control; note that
// converter library implementers are required to support import (but
// not export)
// richedit - pointer to a TRichEdit to be imported to
// filepath - file to be imported into the TRichEdit
// formatClass - an optional format class to use for the import
// returns an fceXXX code; fceNoErr is success
//
FCE TRtfConverter::ForeignToRtf(TTaeRichEdit* richedit, AnsiString filepath,
  AnsiString formatClass)
{
  // create a temporary stream to hold incoming RTF
  try {
    mstream = new TMemoryStream();
    }
  catch (...) {
    return fceNoMemory;
    }

  // create global handles for ghszFile, ghszClass, & ghszSubset and
  // allocate a working buffer
  HGLOBAL ghszFile, ghszClass, ghszSubset;
  ghszFile = ::GlobalAllocString(filepath);
  ghszClass = ::GlobalAllocString(formatClass);
  ghszSubset = ::GlobalAllocString("");
  ghBuff = ::GlobalAlloc(GHND, BUFFSIZE /* arbitrary buffer size */);
  if (!ghszFile || !ghszClass || !ghszSubset || !ghBuff) {
    if (ghszFile) ::GlobalFree(ghszFile);
    if (ghszClass) ::GlobalFree(ghszClass);
    if (ghszSubset) ::GlobalFree(ghszSubset);
    if (ghBuff) ::GlobalFree(ghBuff);
    ghBuff = 0;
    return fceNoMemory;
    }

  // import RTF
  FCE fce = pForeignToRtf32(ghszFile, 0, ghBuff, ghszClass, ghszSubset, RtfOut);

  // free storage
  ::GlobalFree(ghszFile);
  ::GlobalFree(ghszClass);
  ::GlobalFree(ghszSubset);
  ::GlobalFree(ghBuff);
  ghBuff = 0;

  // copy the stream into the rich edit control
  mstream->Position = 0;
  richedit->Lines->LoadFromStream(mstream);

  // free the memory stream
  delete mstream;
  mstream = 0;

  // and return results
  return fce;
}

//---------------------------------------------------------------------------
// RtfIn - global function callback used by RtfToForeignTo() below; when called,
// copies the specified number of bytes into the global memory stream
// parameters - not used in this implementation
// returns number of bytes of RTF to be processed on success; zero indicates
// end of RTF data; fceUserCancel indicates user cancelled; other fceXXX
// codes should be self-explanatory...
//
long PASCAL RtfIn(long /* rgfOptions - not implemented here */, long /* not used */)
{
  long bytes;  // assume no more to write
  char* p = (char*) ::GlobalLock(ghBuff);
  if (!p) return fceNoMemory;
  else {
    try {
      bytes = mstream->Read(p, BUFFSIZE);
      }
    catch (...) {
      bytes = fceWriteErr;
      }
    ::GlobalUnlock(ghBuff);
    }
  return bytes;
}

//---------------------------------------------------------------------------
// RtfToForeign() - export a file from a Rich Edit control; note that
// converter library implementers are not required to support export
// richedit - pointer to a TRichEdit to be exported from
// filepath - file to be exported to
// formatClass - an optional format class to use for the import
// returns an fceXXX code; fceNoErr is success
//
// optional - may not exist
FCE TRtfConverter::RtfToForeign(TTaeRichEdit* richedit, AnsiString filepath,
  AnsiString formatClass)
{
  // quick out if not implemented
  if (!pRtfToForeign32) return fceWrongFileType;  // what to return???

  // create a temporary stream to hold incoming RTF
  try {
    mstream = new TMemoryStream();
    }
  catch (...) {
    return fceNoMemory;
    }

  // fill the stream from the Rich Edit control
  try {
    richedit->Lines->SaveToStream(mstream);
    }
  catch (...) {
    delete mstream;
    mstream = 0;
    return fceNoMemory;
    }

  // reposition stream to start
  mstream->Position = 0;

  // create global handles for ghszFile, ghszClass, & ghszSubset and
  // allocate a working buffer
  HGLOBAL ghszFile, ghszClass;
  ghszFile = ::GlobalAllocString(filepath);
  ghszClass = ::GlobalAllocString(formatClass);
  ghBuff = ::GlobalAlloc(GHND, BUFFSIZE /* arbitrary buffer size */);
  if (!ghszFile || !ghszClass || !ghBuff) {
    if (ghszFile) ::GlobalFree(ghszFile);
    if (ghszClass) ::GlobalFree(ghszClass);
    if (ghBuff) ::GlobalFree(ghBuff);
    ghBuff = 0;
    return fceNoMemory;
    }

  // export RTF
  FCE fce = pRtfToForeign32(ghszFile, 0, ghBuff, ghszClass, RtfIn);

  // free storage
  ::GlobalFree(ghszFile);
  ::GlobalFree(ghszClass);
  ::GlobalFree(ghBuff);
  ghBuff = 0;

  // free the memory stream
  delete mstream;
  mstream = 0;

  // and return results
  return fce;
}

//---------------------------------------------------------------------------
// TRtfConverterList class - support class to get lists of available
// converter paths, descriptive names, and recognized extensions from
// the Registry
//---------------------------------------------------------------------------
// NextExt() - quick and dirty utility function to extract an extension from
// a char[]
// s - string of extensions, space-delimited and null-terminated
//
AnsiString NextExt(char*& s)
{
  while (*s && *s == ' ') s++;
  if (!*s) return "";
  char *p = s + 1;
  while (*p && *p != ' ') p++;
  AnsiString str(s, (unsigned char) (p - s));
  s = p;
  return str;
}

//---------------------------------------------------------------------------
// TRtfConverterList - ctor does all the work...
// import - specifies whether to build a list of import (true) converters or
//    export (false) converters
//
TRtfConverterList::TRtfConverterList(bool import)
{
  // allocate string lists
  LibraryPath = new TStringList();
  Description = new TStringList();
  Description->Sorted = true;
  Description->Duplicates = dupAccept;
  FRawDescription = new TStringList();
  FormatClass = new TStringList();
  Filters = new TStringList();

  // get converter list for each known location
  GetConverterList(HKEY_LOCAL_MACHINE,
    "\\Software\\Microsoft\\Shared Tools\\Text Converters\\", "",
    import);
  GetConverterList(HKEY_CURRENT_USER,
    "\\Software\\Microsoft\\Word\\7.0\\Text Converters\\", "Word 95",
    import);
  GetConverterList(HKEY_CURRENT_USER,
    "\\Software\\Microsoft\\Office\\8.0\\Word\\Text Converters\\",
    "Word 97", import);

  // and build a filter list suitable for using with TOpenDialog::Filter
  for (int i = 0; i < Description->Count; i++) {
    if (i) FilterList += "|";
    FilterList += Description->Strings[i] + " (" + Filters->Strings[i] +
      ")|" + Filters->Strings[i];
    }
}
//---------------------------------------------------------------------------
TRtfConverterList::~TRtfConverterList()
{
  if (LibraryPath) delete LibraryPath;
  if (Description) delete Description;
  if (FRawDescription) delete FRawDescription;
  if (FormatClass) delete FormatClass;
  if (Filters) delete Filters;
}
//---------------------------------------------------------------------------
void TRtfConverterList::GetConverterList(HKEY regRoot, AnsiString regPath,
  AnsiString appName, bool import)
{
  // allocate string list for subkeys
  TStringList* subKeys = new TStringList();

  // Registry key path and data value names
  AnsiString sRegPath = regPath;
  AnsiString sName("Name");
  AnsiString sExt("Extensions");
  AnsiString sPath("Path");

  // modify Registry key path for import or export
  if (import) sRegPath += AnsiString("Import");
  else sRegPath += AnsiString("Export");

  // allocate a Registry object
  TRegistry* reg = new TRegistry();

  try {
    // open Registry key and get subkeys
    reg->RootKey = regRoot;
    reg->OpenKey(sRegPath, false);
    reg->GetKeyNames(subKeys);
    reg->CloseKey();

    // for each subkey
    for (int i = 0; i < subKeys->Count; i++) {
      AnsiString currKey, name, ext, path;

      // append it to the import/export key
      currKey = sRegPath + AnsiString("\\") + subKeys->Strings[i];

      // open that key and retrieve "Name," "Path", & "Extensions" values
      try {
        reg->OpenKey(currKey, false);
        name = reg->ReadString(sName);
        path = reg->ReadString(sPath);
        AnsiString tempExt = reg->ReadString(sExt);

        // extensions are returned as a space-delimited, null-terminated
        // string; parse extensions and format as filters
        char* s = tempExt.c_str();
        while (*s) {
          AnsiString temp = NextExt(s);
          if (!temp.Length()) continue;
          if (ext.Length()) ext += ";";
          ext += AnsiString("*.") + temp;
          }
        }
      // catch errors and continue
      catch (...) {}

      // close the subkey
      reg->CloseKey();

      // duplicates are possible -- look through the descriptions
      // and, if a match is found, compare the library paths, extensions,
      // and format classes... if all are the same as the existing entry,
      // then skip this one.
      bool skipIt = false;
      for (int j = 0; j < FRawDescription->Count; j++)
        if (!(FRawDescription->Strings[j].AnsiCompareIC(name)) &&
          !(LibraryPath->Strings[j].AnsiCompareIC(path)) &&
          !(FormatClass->Strings[j].AnsiCompareIC(subKeys->Strings[i])) &&
          !(Filters->Strings[j].AnsiCompareIC(ext)))
          skipIt = true;

      // and add the values to the string lists
      if (!skipIt && name.Length() && ext.Length() && path.Length()) {
        AnsiString rawName = name;
        if (appName.Length()) name += AnsiString(" - ") + appName;
        int ndx = Description->Add(name);
        FRawDescription->Insert(ndx, rawName);
        LibraryPath->Insert(ndx, path);
        FormatClass->Insert(ndx, subKeys->Strings[i]);
        Filters->Insert(ndx, ext);
        }
      }
    }
  // ignore errors
  catch (...) {}

  // free local storage
  delete reg;
  delete subKeys;
}
//---------------------------------------------------------------------------
