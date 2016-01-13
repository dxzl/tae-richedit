// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------
USEFORM("LogoMain.cpp", LogoAppForm);
USERC("LogoStrs.rc");
USERES("LogoApp.res");
USEFORM("about.cpp", AboutBox);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  Application->Initialize();
  Application->CreateForm(__classid(TLogoAppForm), &LogoAppForm);
    Application->CreateForm(__classid(TAboutBox), &AboutBox);
    Application->Run();

  return 0;
}
//---------------------------------------------------------------------
