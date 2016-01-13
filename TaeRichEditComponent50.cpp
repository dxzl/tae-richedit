// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
#include <vcl.h>
#pragma hdrstop
USERES("TaeRichEditComponent50.res");
USEUNIT("TaeRichEdit.cpp");
USERES("TaeRichEdit.dcr");
USEUNIT("TaeRichEditStrings.cpp");
USEUNIT("TaeAttrib.cpp");
USEUNIT("TaeAttrib2.cpp");
USEUNIT("TaeRichEditOleCallback.cpp");
USEUNIT("TaeRichEditOle.cpp");
USEUNIT("TaeAdjLineBrks.cpp");
USEUNIT("TaeRichEditPrint.cpp");
USEUNIT("TaeRichEditAdvPrint.cpp");
USEUNIT("TaeUtility.cpp");
USEUNIT("TaeParser.cpp");
USEUNIT("TaePageLayout.cpp");
USEUNIT("TaeRegistry.cpp");
USEUNIT("TaePrintDialog.cpp");
USEFORM("TaePrintCancelDlg.cpp", TaePrintCancelDialog);
USEUNIT("TaePageSetupDlg.cpp");
USERES("TaePageSetupDlg.dcr");
USEUNIT("TaeVerInfo.cpp");
USEUNIT("TaeRichEditConv.cpp");
USEUNIT("TaeDataObjBmp.cpp");
USEPACKAGE("vcl50.bpi");
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Package source.
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
  return 1;
}
//---------------------------------------------------------------------------
