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
// TaeUtility.h - header file for TaeUtility.cpp.
//---------------------------------------------------------------------------
#ifndef TaeUtilityH
#define TaeUtilityH
//---------------------------------------------------------------------------
AnsiString GetLongFileName(AnsiString filename);
AnsiString GetNetUser(void);
//---------------------------------------------------------------------------
class PACKAGE TW95FileData {
protected:
  WIN32_FIND_DATA data_;
  TDateTime ConvertDateTime(FILETIME& ft) const;

public:
  TW95FileData(AnsiString path);
  TW95FileData(WIN32_FIND_DATA& data);
  bool operator ==(TW95FileData& data) const;
  bool IsArchiveSet() const;
  bool IsCompressedSet() const;
  bool IsDirectorySet() const;
  bool IsHiddenSet() const;
  bool IsOfflineSet() const;
  bool IsReadonlySet() const;
  bool IsSystemSet() const;
  bool IsTemporarySet() const;
  AnsiString FileName(void) const;
  AnsiString ShortFileName() const;
  unsigned long FileBytes() const;
  TDateTime CreationTime() const;
  TDateTime LastAccessTime() const;
  TDateTime LastWriteTime() const;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
