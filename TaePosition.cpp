// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//---------------------------------------------------------------------------
// TaePosition.cpp - implementation file for saving/restoring edit-control's
// position information.
//
// Author: Scott Swift
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeRichEdit.h"
#include "TaePosition.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TTaePosition::TTaePosition(TTaeRichEdit* re)
{
  re_handle = re->Handle;
  bRestoreSel = true; // default is to restore SelRange
}
//---------------------------------------------------------------------------

TAEPOSITION __fastcall TTaePosition::GetPosition(void)
{
  TAEPOSITION pos;
  
  // get the line offset in pos.y and the x-offset into the line in pos.x
  TCharRange CharRange;
  ::SendMessage(re_handle, EM_EXGETSEL, 0, (LPARAM)&CharRange);
  
  pos.p.x = CharRange.cpMax -
                            ::SendMessage(re_handle, EM_LINEINDEX, -1, 0);
  pos.p.y = SendMessage(re_handle, EM_LINEFROMCHAR, -1, 0);
  
  pos.start = CharRange.cpMin;
  pos.end = CharRange.cpMax;  
  pos.firstVisLine = SendMessage(re_handle, EM_GETFIRSTVISIBLELINE, 0, 0);
  savePos = pos; // Save it for SetPosition to use...
  
  return pos;
}
//---------------------------------------------------------------------------

void __fastcall TTaePosition::SetPosition(TAEPOSITION pos)
{
  // Don't want to restore SelStart and SelLength if it's become "unknown",
  // so the invoking class clears this flag...
  if ( !bRestoreSel )
  {
    // Set SelStart to beginning of the line we were on... no selection
    pos.start = SendMessage(re_handle, EM_LINEINDEX, (WPARAM)savePos.p.y, 0);
    pos.end = pos.start;
  }

  TCharRange CharRange;
  CharRange.cpMax = pos.end;
  CharRange.cpMin = pos.start;
  ::SendMessage(re_handle, EM_EXSETSEL, 0, (LPARAM)&CharRange);

  int newPos = SendMessage(re_handle, EM_GETFIRSTVISIBLELINE, 0, 0);
  ::SendMessage(re_handle, EM_LINESCROLL, 0, savePos.firstVisLine-newPos);
}
//---------------------------------------------------------------------------
