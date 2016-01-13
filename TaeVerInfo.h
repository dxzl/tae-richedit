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
// TaeVerInfo.h - header file for TaeVerInfo.cpp.
//---------------------------------------------------------------------------
#ifndef TaeVerInfoH
#define TaeVerInfoH
//---------------------------------------------------------------------------
typedef struct tagTModuleVersionInfo {
  int Major;
  int Minor;
  int Release;
  int Build;
  } TModuleVersionInfo;

bool ModuleVersionInfo(HMODULE hModule, TModuleVersionInfo& mvi);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
