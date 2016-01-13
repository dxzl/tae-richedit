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
// TaeVerInfo.cpp - implementation file for retrieving module version
// information.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeVerInfo.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
bool ModuleVersionInfo(HMODULE hModule, TModuleVersionInfo& mvi)
{
  // initialize module information structure
  ::memset(&mvi, 0, sizeof(mvi));

  // make sure that HMODULE is valid
  if ((int) hModule < HINSTANCE_ERROR) return false;

  // get name of DLL to use with GetFileVersionInfo()
  TCHAR modulePath[MAX_PATH];
  if (!::GetModuleFileName(hModule, modulePath,
    sizeof(modulePath) / sizeof(TCHAR)))
    return false;

  // get size of version information
  DWORD dummy;
  DWORD size = ::GetFileVersionInfoSize(modulePath, &dummy);
  if (!size) return false;

  // allocate a buffer for version information
  BYTE* buffer = (BYTE*) new BYTE[size];

  // get the version information
  if (!::GetFileVersionInfo(modulePath, 0, size, buffer)) {
    delete[] buffer;
    return false;
    }

  // get the static version information (we could get other information,
  // but this is all we need today)
  VS_FIXEDFILEINFO* vsInfo;
  UINT vsInfoSize;
  if (!::VerQueryValue(buffer, "\\", (LPVOID*) &vsInfo, &vsInfoSize)) {
    delete[] buffer;
    return false;
    }

  // put version information into returned structure
  mvi.Major = HIWORD(vsInfo->dwFileVersionMS);
  mvi.Minor = LOWORD(vsInfo->dwFileVersionMS);
  mvi.Release = HIWORD(vsInfo->dwFileVersionLS);
  mvi.Build = LOWORD(vsInfo->dwFileVersionLS);

  // clean up and return success
  delete[] buffer;
  return true;
}
//---------------------------------------------------------------------------
