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
// TaeUtility.cpp - implementation file for TaeUtility.cpp.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <shlobj.h>
#include "TaeUtility.h"
#include "TaeCompileOpts.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// GetLongFileName() - intended to take a filename (which may or may not
// include a path) and convert it to a file/path in which each element of
// the path is in long name format.  this turned out to be a bit harder than
// I expected....
//
// the following was adapted from a Borland FAQ.  note that you should pass
// the full path and that the file must exist.
//
// I have some vague recollection that SHGetPathFromIDList expands to
// SHGetPathFromIDListA or SHGetPathFromIDListW (depending upon platform)
// and that at least one of these is not exported by shell.dll on some
// versions of Win9x.  however, I cannot now find the Microsoft KnowledgeBase
// article that explains this so maybe I am wrong.  if you have problems or
// insights on this, please let me know.
//
AnsiString GetLongFileName(AnsiString filename)
{
#if WORK_AROUND_SHGETPATHFROMIDLIST_BUG    // see TaeCompileOpts.h
  return filename;
#else
  // set default return value to passed value
  AnsiString longName = filename;

  // get desktop folder
  LPSHELLFOLDER psfDeskTop = 0;
  if (::SHGetDesktopFolder(&psfDeskTop) == NOERROR ) {
    // determine full characteristics of name
    WCHAR wideName[MAX_PATH];
    ::MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, wideName,
      MAX_PATH);

    // get the pidl for the short path
    ULONG chEaten = 0;
    LPITEMIDLIST pidlShellItem = 0;
    psfDeskTop->ParseDisplayName(0, 0, wideName, &chEaten,
      &pidlShellItem, 0);
    if (pidlShellItem) {
      // determine name from shell object
      char szLongName[_MAX_PATH];
      if (::SHGetPathFromIDList(pidlShellItem, szLongName ))
        longName = szLongName;

      // clean up memory
      LPMALLOC pMalloc;
      if (::SHGetMalloc(&pMalloc) == NOERROR) {
        pMalloc->Free(pidlShellItem);
        pMalloc->Release();
        }
      }

    // release resources
    psfDeskTop->Release();
    }

  return longName;
#endif
}
//---------------------------------------------------------------------------
// get the currently logged on user name from Windows
//
AnsiString GetNetUser(void)
{
  AnsiString retVal("(unknown)");
  char *user = 0;
  DWORD len = 0;

  ::WNetGetUser(NULL, user, &len);
  user = new char[(int) (len + 1)];
  if (::WNetGetUser(NULL, user, &len) == NO_ERROR) retVal = user;
  delete[] user;

  return retVal;
}
//---------------------------------------------------------------------------
// TW95FileData (WIN32_FILE_ATTRIBUTE_DATA) class
//---------------------------------------------------------------------------
TDateTime TW95FileData::ConvertDateTime(FILETIME& ft) const
{
  FILETIME ftLocal;
  SYSTEMTIME st;

  ::FileTimeToLocalFileTime(&ft, &ftLocal);  // note: failure return (0) ignored
  ::FileTimeToSystemTime(&ftLocal, &st);

  TDateTime dt(st.wYear, st.wMonth, st.wDay);
  TDateTime tm(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
  return dt + tm;
}
//---------------------------------------------------------------------------
TW95FileData::TW95FileData(AnsiString path)
{
  memset(&data_, sizeof data_, '\0');

  HANDLE hff = FindFirstFile(path.c_str(), &data_);

  if (hff == INVALID_HANDLE_VALUE) {
    AnsiString s;
    s = "FindFirstFile(\"";
    s += path;
    s += "\") failed.";
    throw Exception(s);
    }
  else FindClose(hff);
}
//---------------------------------------------------------------------------
TW95FileData::TW95FileData(WIN32_FIND_DATA& data)
{
  data_ = data;
}
//---------------------------------------------------------------------------
bool TW95FileData::operator ==(TW95FileData& data) const
{
  return (memcmp(&data, &data_, sizeof data_) == 0);
}
//---------------------------------------------------------------------------
bool TW95FileData::IsArchiveSet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsCompressedSet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsDirectorySet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsHiddenSet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsOfflineSet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsReadonlySet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsSystemSet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
}
//---------------------------------------------------------------------------
bool TW95FileData::IsTemporarySet() const
{
  return data_.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;
}
//---------------------------------------------------------------------------
AnsiString TW95FileData::FileName(void) const
{
  return data_.cFileName;
}
//---------------------------------------------------------------------------
AnsiString TW95FileData::ShortFileName() const
{
  return data_.cAlternateFileName;
}
//---------------------------------------------------------------------------
unsigned long TW95FileData::FileBytes() const
{
  return (data_.nFileSizeHigh * MAXDWORD) + data_.nFileSizeLow;
}
//---------------------------------------------------------------------------
TDateTime TW95FileData::CreationTime() const
{
  return ConvertDateTime((_FILETIME&) data_.ftCreationTime);
}
//---------------------------------------------------------------------------
TDateTime TW95FileData::LastAccessTime() const
{
  return ConvertDateTime((_FILETIME&) data_.ftLastAccessTime);
}
//---------------------------------------------------------------------------
TDateTime TW95FileData::LastWriteTime() const
{
  return ConvertDateTime((_FILETIME&) data_.ftLastWriteTime);
}
//---------------------------------------------------------------------------
