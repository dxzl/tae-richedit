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
// TaeRichEdit30.h - header file for Rich Edit 3.0 declarations and constants.
//---------------------------------------------------------------------------
#ifndef TaeRichEdit30H
#define TaeRichEdit30H

// BCB5 header files include RE 3.0 declarations so we need not define them
#if __BORLANDC__ < 0x0550

// note that the following is taken almost directly from the Microsoft Platform SDK.  
// it is presented here under the concept of the "fair use doctrine" of copyright law
// which allows such things as copying an exact quote from Webster's Dictionary as part
// of some clearly larger endevour.  If the Microsoft lawyers find this to be in violation
// of some part of the MS licensing agreement, then this code may be immediately withdrawn.
//

#define EM_SETTYPOGRAPHYOPTIONS  (WM_USER + 202)
#define EM_GETTYPOGRAPHYOPTIONS  (WM_USER + 203)
#define  TO_ADVANCEDTYPOGRAPHY  1
#define  TO_SIMPLELINEBREAK    2
// outline mode
#define EM_OUTLINE         (WM_USER + 220)
// outline mode wparam values
#define EMO_EXIT         0
#define EMO_ENTER         1
#define EMO_PROMOTE       2
#define EMO_EXPAND         3
#define EMO_MOVESELECTION     4
#define EMO_GETVIEWMODE     5
// EMO_EXPAND options
#define EMO_EXPANDSELECTION   0
#define EMO_EXPANDDOCUMENT     1

// S.S. backwards
//#define VM_NORMAL         2
//#define VM_OUTLINE         4
#define VM_NORMAL         4
#define VM_OUTLINE         2

// notification & mask
#define EN_PARAGRAPHEXPANDED   0x070d
#define ENM_PARAGRAPHEXPANDED   0x00000020
// change font size of current selection by wparam
#define EM_SETFONTSIZE       (WM_USER + 223)
#define EM_GETZOOM         (WM_USER + 224)
#define EM_SETZOOM         (WM_USER + 225)
// get/set scroll position
#define EM_GETSCROLLPOS     (WM_USER + 221)
#define EM_SETSCROLLPOS      (WM_USER + 222)

#define PFM_BOX         0x04000000
#define PFM_COLLAPSED       0x01000000
#define PFM_OUTLINELEVEL     0x02000000
#define PFE_OUTLINELEVEL    (PFM_OUTLINELEVEL   >> 16)
#define PFE_COLLAPSED      (PFM_COLLAPSED     >> 16)
#define PFE_BOX          (PFM_BOX       >> 16)
#define PFE_TABLE        0x4000

#endif // #if __BORLANDC__ < 0x0550

#endif
//---------------------------------------------------------------------------
