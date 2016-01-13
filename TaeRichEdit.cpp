// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 1999, 2000 Thin Air Enterprises and Robert Dunn.  All rights 
// reserved.  Free for non-commercial use.  Commercial use requires license 
// agreement.  See http://home.att.net/~robertdunn/Yacs.html for the most 
// current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeRichEdit.cpp - implementation file for TTaeRichEdit class.
//---------------------------------------------------------------------------
// one of the most frustrating aspects of the Borland VCL is the number of
// class members and methods unnecessarily declared as private in the C++
// headers.  oddly, Delphi does not treat the corresponding declarations
// as private in its own code.  i suppose that this is due to differences
// in the language implementations, but it is still an unacceptable
// situation which severely handicaps BCB programmers.
//
// the following is a nasty hack to get by for now.  this gets us past the
// following errors:
//
//   E2247 '_fastcall TControl::FontChanged(TObject *)' is not accessible.
//   E2247 'TCustomEdit::FMaxLength' is not accessible.
//
//   the following were fixed with Stefan Hoffmeister's (TeamB) help and no
//   longer require the ugly hack:
//   E2247 '_fastcall TCustomMemo::WMNCDestroy(TWMNoParams &)' is not accessible.
//   E2247 '_fastcall TWinControl::CMBiDiModeChanged(TMessage &)' is not accessible.
//   E2247 '_fastcall TWinControl::CMColorChanged(TMessage &)' is not accessible.
//   E2247 '_fastcall TWinControl::WMPaint(TWMPaint &)' is not accessible.
//   E2247 '_fastcall TWinControl::WMSetCursor(TWMSetCursor &)' is not accessible.
//
// you can reproduce the errors by commenting out the following #define.  if
// you find a better way to get around these errors without the ugly hack,
// please let me know.  anyway, here is the hack....
//
#define private protected
#include <vcl.h>
#pragma hdrstop
#undef private

#include <vcl\printers.hpp>
#include "TaeRichEdit.h"
#include "TaeRichEditStrings.h"
#include "TaeRichEditAdvPrint.h"
#include "TaeRichEditOle.h"
#include "TaeUtility.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma pack(push, 4)

//#define EM_SETAUTOCORRECTPROC (WM_USER+234)
//#define EM_CALLAUTOCORRECTPROC (WM_USER+255)
#define EM_SETTEXTEX      (WM_USER + 97)

// Codepages: UTF-16 is 1200, UTF-8 is 65001, ANSI is 1252
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
// Flags for the SETEXTEX data structure
#define ST_DEFAULT    0
#define ST_KEEPUNDO    1
#define ST_SELECTION  2
#define ST_NEWCHARS   4
#define ST_UNICODE    8

// For the GETEXTEX struct
#define GT_NOHIDDENTEXT  8

USERES("TaeRichEdit.dcr");
//---------------------------------------------------------------------------
const AnsiString TTaeRichEdit::Untitled = "(untitled)";
//---------------------------------------------------------------------------
// EM_SETTEXTEX info; this struct is passed in the wparam of the message
typedef struct _settextex
{
  DWORD  flags;      // Flags (see the ST_XXX defines)
  UINT  codepage;    // Code page for translation (CP_ACP for sys default,
                //  1200 for Unicode, -1 for control default)
} SETTEXTEX;
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(TTaeRichEdit *)
{
  new TTaeRichEdit((void*) NULL);
}
//---------------------------------------------------------------------------
namespace Taerichedit
{
  void __fastcall PACKAGE Register()
  {
    TComponentClass classes[1] = {__classid(TTaeRichEdit)};
    RegisterComponents("TaeRichEdit", classes, 0);
  }
}
//---------------------------------------------------------------------------
// TTaeRichEdit constructor
//
__fastcall TTaeRichEdit::TTaeRichEdit(TComponent* Owner) : TCustomMemo(Owner)
{
// Tried this to hopefully be able to receive WM_COMMAND and then EN_CHANGE
// messages - no dice! S.S.
//  Parent = (TWinControl*)((TApplication*)Owner)->MainForm->Handle;

  FFileName = Untitled;
  FLibHandle = 0;
  FModified = false;
  FOnSelChange = 0;
  FOnUpdateEvent = 0;
  FOnResizeRequest = 0;

//  FOnChange = 0;
//  FOnChangeEvent = 0;

  FOnProtectChange = 0;
  FOnProtectEvent = 0;

  FOnSaveClipboard = 0;
  FPageRect.left = FPageRect.top = FPageRect.right = FPageRect.bottom = 0;
  FOleSupport = true;
  // the default font is MS Sans Serif -- print preview is screwed by non-
  // true-type fonts
  // note: do not install OnChange handler until *after* all changes are
  // made (when this event is invoked, there must be a valid Rich Edit
  // handle -- not the case here)
  FFont2 = new TFont2();
  FFont2->Name = "Arial";
  FFont2->Size = 10;
  FFont2->OnChange = FontChanged;

  // S.S. added 5/2014
  FOldLineCount = 0;
  FOldLength = 0;
  FLockCounter = 0;
  FView = 0; // V_OFF

  // from TCustomRichEdit
  HDC DC;

  FSelAttributes = new TTextAttrib(this, atSelected);
  FDefAttributes = new TTextAttrib(this, atDefaultText);
  FParagraph = new TParaAttrib(this);
  FTaeRichEditStrings = new TTaeRichEditStrings();
  static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->FRichEdit = this;
  TabStop = true;
  Width = 185;
  Height = 89;
  AutoSize = false;
  DoubleBuffered = false;
  FHideSelection = true;
  FHideScrollBars = true;
  // get desktop window DC to calc screen pixels (does this cause a
  // DeskTop redraw???)
  DC = ::GetDC(0);
  FScreenLogPixels = ::GetDeviceCaps(DC, LOGPIXELSY);
  FDefaultConverter = __classid(TConversion);
  ::ReleaseDC(0, DC);
  FOldParaAlignment = Alignment;
  Perform(CM_PARENTBIDIMODECHANGED, 0, 0);

  // TTaeRichEdit initializations
  FRichEditOle = 0;
  FInsertMode = true;
  FSelectionBar = false;
  FUndoLimit = 100;
  FUndoLimitActual = 0;
  FAutoUrlDetect = false;
  FOnUrlClick = 0;
  FOnLinkEvent = 0;
  FAutoUrlExecute = true;
  FAutoWordSelect = false;
  FOnInsertModeChange = 0;
  FMemStream = 0;
  FWordWrapTo = wwtNone;
  FTabWidth = 8;
  FTabCount = 0;
  FTransparent = false;
  FGuessFont = true;
  FPlainTextFont = new TTaeFont();
  FPlainTextFont->Name = "Courier New";
  FPlainTextFont->Size = 10;
  FRtfTextFont = new TTaeFont();
  FRtfTextFont->Name = "Arial";
  FRtfTextFont->Size = 10;
  FGuessFont = true;
  FEnableRedraw = true;
  FEnableNotifications = true;

  FOleAllowTextDrag = false;
  FOleAcceptDrop = false;
  FRichEditVersionInfo = &FRichEditVersionInfoData;

  // S.S. added 2/1/2015
  // Set Property PrintSupport = true to call SetPrintSupport()
  FPrintSupport = false;

// Only in Windows 8
//  try { SetAutoCorrectProc(); } //S.S.
//  catch(...){}
}
//---------------------------------------------------------------------------
// TTaeRichEdit destructor
//
__fastcall TTaeRichEdit::~TTaeRichEdit(void)
{
  // TTaeRichEdit destructors
  if (FFont2) {
    delete FFont2;
    FFont2 = 0;
    }
  if (FPlainTextFont) {
    delete FPlainTextFont;
    FPlainTextFont = 0;
    }
  if (FRtfTextFont) {
    delete FRtfTextFont;
    FRtfTextFont = 0;
    }
  if (FRichEditPrint) {
    delete FRichEditPrint;
    FRichEditPrint = 0;
    }
  // from TCustomRichEdit destructor
  if (FSelAttributes) {
    delete FSelAttributes;
    FSelAttributes = 0;
    }
  if (FDefAttributes) {
    delete FDefAttributes;
    FDefAttributes = 0;
    }
  if (FParagraph) {
    delete FParagraph;
    FParagraph = 0;
    }
  if (FTaeRichEditStrings) {
    delete FTaeRichEditStrings;
    FTaeRichEditStrings = 0;
    }
  if (FMemStream) {
    delete FMemStream;
    FMemStream = 0;
    }
}
//---------------------------------------------------------------------------
// set the FTaeRichEditStrings member (converted from TStrings)
//
void __fastcall TTaeRichEdit::SetLines(TStrings* Value)
{
  static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->Assign(Value);
}
//---------------------------------------------------------------------------
// RegisterConversionFormat() - analog to the
// TRichEdit::RegisterConversionFormat() method -- parameters are identical
//
void __fastcall  TTaeRichEdit::RegisterConversionFormat(System::TMetaClass* vmt,
  const System::AnsiString AExtension, System::TMetaClass* AConversionClass)
{
  TTaeRichEditStrings::RegisterConversionFormat(vmt, AExtension,
    AConversionClass);
}
//---------------------------------------------------------------------------
// the following ensures that the new version of richedit (v2.0) is loaded
//
void __fastcall TTaeRichEdit::CreateParams(Controls::TCreateParams& Params)
{
  // modified from TCustomRichEdit
  const char RichEditModuleName[] = "MSFTEDIT.DLL";

  long int OldError;

  OldError = SetErrorMode(SEM_NOOPENFILEERRORBOX);
  FLibHandle = (int) LoadLibrary(RichEditModuleName);
  if (FLibHandle > 0 && FLibHandle < HINSTANCE_ERROR) FLibHandle = 0;

  ModuleVersionInfo((HMODULE) FLibHandle, *FRichEditVersionInfo);

  FRichEditDllVersion = RICHEDIT_DLL_VER;

  SetErrorMode(OldError);
  TCustomMemo::CreateParams(Params);
  CreateSubClass(Params, "RichEdit50W");
  Params.Style = Params.Style | (FHideScrollBars ? 0 : ES_DISABLENOSCROLL) |
   (FHideSelection ? 0 : ES_NOHIDESEL) | (FSelectionBar ? ES_SELECTIONBAR : 0);
  Params.WindowClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}
//---------------------------------------------------------------------------
// do stuff when creating/recreating window handle
//
void __fastcall TTaeRichEdit::CreateWnd(void)
{
  // from TCustomRichEdit - this actually creates the Rich Edit window.
  // it also does some things that, perhaps, we do not want or need.  for
  // example, it sets the maximum text length.  unfortunately, it appears
  // to do this *after* the control's text is set to the Caption property.
  // if, indeed, I am reading this correctly, and if the EM_EXLIMITTEXT
  // message *must* be sent while the control is empty, this is probably
  // the source of the many reports of problems setting the maximum text
  // length > 64k.  anyway, the following code works around any potential
  // problem by redoing the work.
  //
  // get the text from the control (if we are not reloading) and make it unique
  AnsiString caption;

  if (!FMemStream)
  {
    caption = Caption;
    caption.Unique();
  }

  // now create window handle
  TCustomMemo::CreateWnd();

  // TCustomRichEdit stuff
  if (SysLocale.FarEast && SysLocale.PriLangID != LANG_JAPANESE)
    Font->Charset = GetDefFontCharSet();

  // move the next line *after* loading of text???
  SetEnableNotifications(FEnableNotifications);
  ::SendMessage(Handle, EM_SETBKGNDCOLOR, 0, ColorToRGB(Color));

  // enable multi-undo/redo (returns zero on success)
#if __BORLANDC__ < 0x0550
  if (::SendMessage(Handle, EM_SETTEXTMODE, (WPARAM) Richedit::TM_MULTILEVELUNDO, 0))
#else
  if (::SendMessage(Handle, EM_SETTEXTMODE, (WPARAM) TM_MULTILEVELUNDO, 0))
#endif
    FUndoLimitActual = 1;
  else // set undo limit (sets FUndoLimitActual)
    SetUndoLimit(FUndoLimit);

  // the following enables full word justification in RE 3.0 -- should
  // have no affect on earlier versions (if this breaks RE 1.0 or RE 2.0,
  // just remove it)
  ::SendMessage(Handle, EM_SETTYPOGRAPHYOPTIONS, TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY);

  // set up ole
  if (FRichEditOle) delete FRichEditOle;
  if (FOleSupport)
  {
    FRichEditOle = new TIRichEditOle(this);
    SetOleAllowTextDrag(FOleAllowTextDrag);
    SetOleAcceptDrop(FOleAcceptDrop);
  }

  // be sure that control is really, really empty before trying to change
  // max length of text.  note that EM_EXLIMITTEXT does *not* limit the
  // size when streaming -- only when the user is typing.
  AnsiString fileName = FileName;
  SelectAll();
  Clear();
  FileName = fileName;
  int maxLength = FMaxLength;

  if (maxLength <= 0)
    maxLength = 0x7ffffff0;

  // set the max length
  ::SendMessage(Handle, EM_EXLIMITTEXT, 0, maxLength);

  // reset the Caption into the text body
  if (!FMemStream)
  {
    FMemStream = new TMemoryStream();
    bool Plain = PlainText;
    bool DesignMode = ComponentState.Contains(csDesigning);
    PlainText = DesignMode;
    FMemStream->WriteBuffer(&DesignMode, sizeof(DesignMode));
    try
    {
      FMemStream->Write(caption.c_str(), caption.Length());
      FMemStream->Position = 0;
    }
    __finally
    {
      PlainText = Plain;
    }
  }

  // set auto url detection to match property state (must be done *before*
  // setting the text or streaming it in if it is to work on the loaded text)
  SetAutoUrlDetect(FAutoUrlDetect);

  // load the text
  if (FMemStream)
  {
    bool DesignMode;
    bool Plain = PlainText;
    FMemStream->ReadBuffer(&DesignMode, sizeof(DesignMode));
    PlainText = DesignMode;
    try
    {
      static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->LoadFromStream(FMemStream);
      delete FMemStream;
      FMemStream = 0;
    }
    __finally
    {
      PlainText = Plain;
    }
  }

  Modified = FModified;

//  if (FWordWrapTo == wwtPrinter) SetWordWrapToPrinter();

  // if we have a printer, calculate rects and set wordwrap (if wrap to printer)
  if (FRichEditPrint)
  {
    FRichEditPrint->CalcRects();

    if (FWordWrapTo == wwtPrinter)
      SetWordWrapToPrinter();
  }

  if (FTabCount > 0)
  {
//    TTaeTabStops tabs;
//    GetTabStops(tabs);
//    SetTabStops(FTabCount, tabs);
    SetTabStops(); // S.S.
  }

  if (!FInsertMode)
  {
    FInsertMode = true;  // this is the starting state for RichEdits
    ToggleInsertMode(); // this will toggle shadow variable (FInsertMode)
  }

  // set auto word select to match property state
  SetAutoWordSelect(FAutoWordSelect);

  // set transparent to match property state
  SetTransparent(FTransparent);

  // move the cursor to the top of the text
  ::SendMessage(Handle, EM_SETSEL, 0, 0);
}
//---------------------------------------------------------------------------
// do stuff when destroying the window handle.  as written, this saves the
// text currently in the control in case it needs to be reloaded in
// CreateWnd().  it could be improved by adding a flag to bypass saving
// the text when it will not be needed (e.g., when shutting down).
//
void __fastcall TTaeRichEdit::DestroyWnd(void)
{
  // TCustomRichEdit stuff
  FModified = Modified;
  FMemStream = new TMemoryStream();
  bool Plain = PlainText;
  bool DesignMode = ComponentState.Contains(csDesigning);
  PlainText = DesignMode;
  FMemStream->WriteBuffer(&DesignMode, sizeof(DesignMode));
  try {
    static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->SaveToStream(FMemStream);
    FMemStream->Position = 0;
    }
  __finally {
    PlainText = Plain;
    }

  // TTaeRichEdit stuff
  if (FRichEditOle) {
    delete FRichEditOle;
    FRichEditOle = 0;
    }

  TCustomMemo::DestroyWnd();
}
//---------------------------------------------------------------------------
// I have no idea why Borland chose to release the rich edit DLL in this
// message handler instead of in the DestroyWnd() method, but here it is...
//
void __fastcall TTaeRichEdit::WMNCDestroy(TWMNCDestroy& Message)
{
  // better make sure that embedded objects are not active before unloading
  // the rich edit DLL, destroying the window handle, etc.
  if (FRichEditOle) {
    delete FRichEditOle;
    FRichEditOle = 0;
    }

  // Stefan Hostmeister (TeamB) says Delphi's inherited keyword calls
  // the inherited Dispatch method in message handlers...
  // TCustomMemo::WMNCDestroy(Message);
  TCustomMemo::Dispatch(&Message);

  // unload the rich edit DLL
  if (FLibHandle) {
    FreeLibrary((void*) FLibHandle);
    FLibHandle = 0;
    }
}
//---------------------------------------------------------------------------
// turn the selection bar on or off.  (the selection bar is a small margin
// to the left of all of the text in the control -- when it is on, placing
// the cursor in this area changes the cursor to a northeast arrow in preparation
// for selecting one or more lines.)  there is a bug somewhere in the native
// control or in the VCL that causes the control to vanish (at least, in my
// application where the control is on a tab control/tab sheet).  oddly, the
// control is still there, accepting keystrokes and processing messages....
//
void __fastcall TTaeRichEdit::SetSelectionBar(bool value)
{
  if (value)
    ::SendMessage(Handle, EM_SETOPTIONS, ECOOP_OR, ECO_SELECTIONBAR);
  else
    ::SendMessage(Handle, EM_SETOPTIONS, ECOOP_AND, ~(ECO_SELECTIONBAR));

  // work around the bug
  bool focused = Focused();
  Visible = false;
  Visible = true;
  if (focused) SetFocus();

  FSelectionBar = value;
}
//---------------------------------------------------------------------------
// note: SetInsertMode() does not trigger an OnInsertChange event
//
void __fastcall TTaeRichEdit::SetInsertMode(bool value)
{
  if (FInsertMode != value) return;
  FInsertMode = value;
  ToggleInsertMode();
}
//---------------------------------------------------------------------------
// note: ToggleInsertMode() does not trigger an OnInsertChange event --
// the FInsertMode member shadow variable gets flipped in the KeyDown()
// handler
//
void __fastcall TTaeRichEdit::ToggleInsertMode(void)
{
  // synthesize an insert keystroke (cannot use keybd_event() because
  // there is no window associated with the API function)
  if (!Handle) return;
  // save and clear the event handler
  TNotifyEvent event = FOnInsertModeChange;
  FOnInsertModeChange = 0;
  // the following was glommed from a Micro$oft VB example
  ::SendMessage(Handle, WM_KEYDOWN, VK_INSERT, 0x00510001);
  ::SendMessage(Handle, WM_KEYUP, VK_INSERT, 0xC0510001);
  // restore the event handler
  FOnInsertModeChange = event;
}
//---------------------------------------------------------------------------
// set number of undo's stored by control. returns number actually set (may
// be smaller than request).
//
void __fastcall TTaeRichEdit::SetUndoLimit(int value)
{                                         
  FUndoLimit = value;
  FUndoLimitActual = ::SendMessage(Handle, EM_SETUNDOLIMIT, value, 0);
}
//---------------------------------------------------------------------------
// get number of undo's stored by control.  we do an odd thing here... we
// want the design-mode requested undo limit to be whatever the programmer
// requested and we want that value to be stored.  however, if the programmer
// asks for the value at runtime, we want to return the true value that the
// control is using.
//
int __fastcall TTaeRichEdit::GetUndoLimit(void)
{
  if (ComponentState.Contains(csDesigning)) return FUndoLimit;
  return FUndoLimitActual;
}
//---------------------------------------------------------------------------
// turn on/off auto detection of urls
//
void __fastcall TTaeRichEdit::SetAutoUrlDetect(bool value)
{
  // set the shadow variable
  FAutoUrlDetect = value;

  // if in design mode, do not auto-detect
  if (ComponentState.Contains(csDesigning)) return;

  // tell the RE to auto detect or ignore URLs
  ::SendMessage(Handle, EM_AUTOURLDETECT, value, (LPARAM) 0);

//
// must remove this optimization for OnLinkEvent to work when AutoUrlDetect = false
//
//  // get the current event notification mask
//  unsigned int mask = ::SendMessage(Handle, EM_GETEVENTMASK, 0, 0);
//
//  // set or clear the link notification bit
//  if (value) mask |= ENM_LINK;
//  else mask &= ~(ENM_LINK);
//
//  // set the new event notification mask
//  ::SendMessage(Handle, EM_SETEVENTMASK, 0, mask);
}
//---------------------------------------------------------------------------
// ok, at first, I misunderstood what this meant.  originally, I thought that
// this meant enabling/disabling the automatic selection of the current word
// upon a double-click (and the entire line upon a triple-click).  that is
// *not* what this means.  what it means is that, if you start selecting in
// the middle of a word and drag the selection into, say, the next word, the
// beginning of the selection will be expanded on the left to include the
// entire starting word.  sheesh!
//
void __fastcall TTaeRichEdit::SetAutoWordSelect(bool value)
{
  if (value)
    ::SendMessage(Handle, EM_SETOPTIONS, ECOOP_OR, ECO_AUTOWORDSELECTION);
  else
    ::SendMessage(Handle, EM_SETOPTIONS, ECOOP_AND, ~(ECO_AUTOWORDSELECTION));
  FAutoWordSelect = value;
}
//---------------------------------------------------------------------------
// return true if can redo, false otherwise
//
bool __fastcall TTaeRichEdit::GetCanRedo(void)
{
  return ::SendMessage(Handle, EM_CANREDO, 0, 0);
}
//---------------------------------------------------------------------------
// undo the last change.  return zero on failure.
//
bool TTaeRichEdit::Undo(void)
{
// S.S. I have my own class in YahCoLoRiZe to handle Undo now so this
// is never called...
  return ::SendMessage(Handle, EM_UNDO, 0, 0);
}
//---------------------------------------------------------------------------
// redo the last undone change.  return zero on failure.
//
bool TTaeRichEdit::Redo(void)
{
  return ::SendMessage(Handle, EM_REDO, 0, 0);
}
//---------------------------------------------------------------------------
// force next change to be recorded as a separately undoable action, even if
// it would normally be grouped with the prior changes.  for example, if you
// call this after each keystroke, then each keystroke is separately undoable.
//
void TTaeRichEdit::StopGroupTyping(void)
{
  ::SendMessage(Handle, EM_STOPGROUPTYPING, 0, 0);
}
//---------------------------------------------------------------------------
// build a string of text representing the nature of the next undo/redo
//
static const char *UndoNames[] = {
  "Cannot ", " Typing", " Delete", " Drop Object", " Cut", " Paste"
  };

AnsiString __fastcall TTaeRichEdit::BuildUndoTypeString(AnsiString type, int id)
{
  if (id >= sizeof(UndoNames) / sizeof(UndoNames[0])) id = 0;
  AnsiString s(UndoNames[id]);
  if (!id) return s + type;
  else return type + s;
}
//---------------------------------------------------------------------------
// build a string of text representing the nature of the next undo operation
//
AnsiString __fastcall TTaeRichEdit::GetUndoTypeString(void)
{
  int id = ::SendMessage(Handle, EM_GETUNDONAME, 0, 0);
  if (CanUndo && !id) return "Undo";
  return BuildUndoTypeString("Undo", id);
}
//---------------------------------------------------------------------------
// build a string of text representing the nature of the next redo operation
//
AnsiString __fastcall TTaeRichEdit::GetRedoTypeString(void)
{
  int id = ::SendMessage(Handle, EM_GETREDONAME, 0, 0);
  if (CanRedo && !id) return "Redo";
  return BuildUndoTypeString("Redo", id);
}
//---------------------------------------------------------------------------
// get the PlainText property value
//
bool __fastcall TTaeRichEdit::GetPlainText(void)
{
  return static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->PlainText;
}
//---------------------------------------------------------------------------
// change the PlainText property
//
void __fastcall TTaeRichEdit::SetPlainText(bool value)
{
  static_cast<TTaeRichEditStrings*>(FTaeRichEditStrings)->PlainText = value;
}
//---------------------------------------------------------------------------
// clear the text from the control
//
void __fastcall TTaeRichEdit::Clear(void)
{
  TCustomMemo::Clear();
  FileName = Untitled;
  Modified = false;
}
//---------------------------------------------------------------------------
// one of the following two is wrong, I think.  one needs to get the font from
// the control and assign it back to the Font property?
//
// handler for the WM_SETFONT message.
//
void __fastcall TTaeRichEdit::WMSetFont(TWMSetFont& Message)
{
  FDefAttributes->Assign(Font);
}
//---------------------------------------------------------------------------
// handler for the CM_FONTCHANGED notification
//
void __fastcall TTaeRichEdit::CMFontChanged(TMessage& Message)
{
  FDefAttributes->Assign(Font);
}
//---------------------------------------------------------------------------
// set the maximum length of text in the control.  note:  my understanding is
// that setting the max length while the control is not empty has no effect.
// if this turns out to be true, then this method should be recoded to
// recreate the window.
//
void __fastcall TTaeRichEdit::DoSetMaxLength(int Value)
{
  ::SendMessage(Handle, EM_EXLIMITTEXT, 0, Value);
}
//---------------------------------------------------------------------------
// get the length of the selected text
//
int __fastcall TTaeRichEdit::GetSelLength(void)
{
  TCharRange CharRange;
  ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM) &CharRange);

  int len = CharRange.cpMax - CharRange.cpMin;

  // Need to check for selection at end of document"no text"
  if (len == 1 && CharRange.cpMax >= this->TextLength - this->LineCount)
    len = 0;

  return len;
}
//---------------------------------------------------------------------------
// returns first and last as 0-based char-indices from a selection.
// both return char-index corresponding to the caret-position if no selection.
// (full 32-bits each) S.S.
void __fastcall TTaeRichEdit::GetSelFirstLast(long &first, long &last)
{
  ::SendMessage(Handle, EM_GETSEL, (WPARAM)&first , (LPARAM)&last);
}
//---------------------------------------------------------------------------
// sets select-zone from first and last as 0-based char-indices
// S.S.
void __fastcall TTaeRichEdit::SetSelFirstLast(long first, long last)
{
  ::SendMessage(Handle, EM_SETSEL, (WPARAM)&first , (LPARAM)&last);
}
//---------------------------------------------------------------------------
// set the length of the selection
//
void __fastcall TTaeRichEdit::SetSelLength(int Value)
{
  TCharRange CharRange;

  ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM) &CharRange);
  CharRange.cpMax = CharRange.cpMin + Value;
  ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &CharRange);
  ::SendMessage(Handle, EM_SCROLLCARET, 0, 0);
}
//---------------------------------------------------------------------------
// get the line and offset at the selection start
//
TPoint __fastcall TTaeRichEdit::GetSelStartPos(void)
{
  return GetPosByIndex(SelStart);
}
//---------------------------------------------------------------------------
// get the line and offset at the selection start
//
TPoint __fastcall TTaeRichEdit::GetSelEndPos(void)
{
  return GetPosByIndex(SelStart+SelLength);
}
//---------------------------------------------------------------------------
// get the start offset of the cursor
//
int __fastcall TTaeRichEdit::GetSelStart(void)
{
  TCharRange CharRange;

  ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM) &CharRange);
  return CharRange.cpMin;
}
//---------------------------------------------------------------------------
// move the start of the selection
//
void __fastcall TTaeRichEdit::SetSelStart(int Value)
{
  TCharRange CharRange;

  CharRange.cpMin = Value;
  CharRange.cpMax = Value;
  ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &CharRange);
}
//---------------------------------------------------------------------------
// handler for the CM_BIDIMODECHANGED message
//
void __fastcall TTaeRichEdit::CMBiDiModeChanged(TMessage& Message)
{
  TParaFormat AParagraph;

  HandleNeeded(); // we REALLY need the handle for BiDi (per TCustomRichEdit)

  // Stefan Hostmeister (TeamB) says Delphi's inherited keyword calls
  // the inherited Dispatch method in message handlers...
  // TCustomMemo::CMBiDiModeChanged(Message);
  TCustomMemo::Dispatch(&Message);

  Paragraph->GetAttributes(AParagraph);
  AParagraph.dwMask = PFM_ALIGNMENT;
  AParagraph.wAlignment = (WORD) (Alignment + 1);    // ???
  Paragraph->SetAttributes(AParagraph);
}
//---------------------------------------------------------------------------
// hide or show the scrollbars
//
void __fastcall TTaeRichEdit::SetHideScrollBars(bool Value)
{
  if (HideScrollBars == Value) return;
  FHideScrollBars = Value;
  RecreateWnd();
}
//---------------------------------------------------------------------------
// set ScrollBars
//
void __fastcall TTaeRichEdit::SetScrollBars(TScrollStyle Value)
{
  FScrollBars = Value;
  ScrollBars = FScrollBars;
}
//---------------------------------------------------------------------------
// hide or show the selection (if true, the selection highlight disappears
// when the focus is moved to another control or window; if false, the
// selection remains highlighted even if focus is removed.
//
void __fastcall TTaeRichEdit::SetHideSelection(bool Value)
{
  if (HideSelection == Value) return;
  FHideSelection = Value;
  ::SendMessage(Handle, EM_HIDESELECTION, HideSelection, (LPARAM) true);
}
//---------------------------------------------------------------------------
// set attributes for currently selected text
//
void __fastcall TTaeRichEdit::SetSelAttributes(TTextAttrib* Value)
{
  FSelAttributes->Assign(Value);
}
//---------------------------------------------------------------------------
// set the default attributes for text
//
void __fastcall TTaeRichEdit::SetDefAttributes(TTextAttrib* Value)
{
  DefAttributes->Assign(Value);
}
//---------------------------------------------------------------------------
// handler for CM_COLORCHANGED notification
//
void __fastcall TTaeRichEdit::CMColorChanged(TMessage& Message)
{
  // Stefan Hostmeister (TeamB) says Delphi's inherited keyword calls
  // the inherited Dispatch method in message handlers...
  // TCustomMemo::CMColorChanged(Message);
  TCustomMemo::Dispatch(&Message);

  ::SendMessage(Handle, EM_SETBKGNDCOLOR, 0, ColorToRGB(Color));
}
//---------------------------------------------------------------------------
// print the contents of the control -- very simple-minded.  note that
// the Caption parameter is the text that will be displayed in the print
// spooler, *not* a caption that prints on the page
//
void __fastcall TTaeRichEdit::Print(const AnsiString Caption)
{
  TFormatRange Range;
  int LastChar, MaxLen, LogX, LogY, OldMap;
  TRect SaveRect;

  ::memset(&Range, 0, sizeof(TFormatRange));
  Printer()->Title = Caption;
  Printer()->BeginDoc();
  Range.hdc = Printer()->Handle;
  Range.hdcTarget = Range.hdc;
  LogX = ::GetDeviceCaps(Range.hdc, LOGPIXELSX);
  LogY = ::GetDeviceCaps(Range.hdc, LOGPIXELSY);
  if (IsRectEmpty(&PageRect)) {
    Range.rc.right = Printer()->PageWidth * 1440 / LogX;
    Range.rc.bottom = Printer()->PageHeight * 1440 / LogY;
    }
  else {
    Range.rc.left = PageRect.Left * 1440 / LogX;
    Range.rc.top = PageRect.Top * 1440 / LogY;
    Range.rc.right = PageRect.Right * 1440 / LogX;
    Range.rc.bottom = PageRect.Bottom * 1440 / LogY;
    }
  Range.rcPage = Range.rc;
  SaveRect = Range.rc;
  LastChar = 0;
  MaxLen = this->TextLength;
  Range.chrg.cpMax = -1;
  // ensure printer DC is in text map mode
  OldMap = ::SetMapMode(Range.hdc, MM_TEXT);
  ::SendMessage(Handle, EM_FORMATRANGE, 0, 0);    // flush buffer
  try {
    do {
      Range.rc = SaveRect;
      Range.chrg.cpMin = LastChar;
      LastChar = ::SendMessage(Handle, EM_FORMATRANGE, 1, (LPARAM) &Range);
      if (LastChar < MaxLen && LastChar != -1) Printer()->NewPage();
      } while (LastChar < MaxLen && LastChar != -1);
    Printer()->EndDoc();  // should not this be moved to the __finally block?
    }
  __finally {
    ::SendMessage(Handle, EM_FORMATRANGE, 0, 0);  // flush buffer
    ::SetMapMode(Range.hdc, OldMap);       // restore previous map mode
    }
}
//---------------------------------------------------------------------------
// handler for WM_PAINT messages.  note that the Painting variable was
// a global variable in the original TCustomRichEdit code.  I changed it
// to a static local variable so that each rich edit will have its own
// copy.
void __fastcall TTaeRichEdit::WMPaint(TWMPaint& Message)
{
  static bool Painting = false;
  TRect R, R1;

  if (::GetUpdateRect(Handle, &R, true)) {
    R1 = Rect(ClientRect.Right - 3, ClientRect.Top, ClientRect.Right, ClientRect.Bottom);
    if (::IntersectRect(&R, &R, &R1)) ::InvalidateRect(Handle, &R1, true);
    }

  if (Painting) Invalidate();
  else {
    Painting = true;
    try {
      // Stefan Hostmeister (TeamB) says Delphi's inherited keyword calls
      // the inherited Dispatch method in message handlers...
      // TCustomMemo::WMPaint(Message);
      TCustomMemo::Dispatch(&Message);
      }
    __finally {
      Painting = false;
      }
    }
}
//---------------------------------------------------------------------------
// handler for WM_SETCURSOR messages
//
void __fastcall TTaeRichEdit::WMSetCursor(TWMSetCursor& Message)
{
  TPoint P;

  // Stefan Hostmeister (TeamB) says Delphi's inherited keyword calls
  // the inherited Dispatch method in message handlers...
  // TCustomMemo::WMSetCursor(Message);
  TCustomMemo::Dispatch(&Message);

  if (Message.Result) return;
  Message.Result = 1;
  ::GetCursorPos(&P);

  TSmallPoint pt = PointToSmallPoint(P);

  int htTest = Perform(WM_NCHITTEST, 0, (LPARAM) &pt);
  switch (htTest) {
    case HTVSCROLL:
    case HTHSCROLL:
      ::SetCursor(Screen->Cursors[crArrow]);
      break;
    case HTCLIENT:
      ::SetCursor(Screen->Cursors[crIBeam]);
      break;
    }
}
//---------------------------------------------------------------------------
// handler for CN_NOTIFY messages
//
// the following structures and typedefs are already defined and here only as
// reference....
//
// struct TWMNotify {
//    unsigned Msg;
//    int IDCtrl;
//    tagNMHDR *NMHdr;
//    int Result;
//     };
//
//  typedef struct tagNMHDR {
//    HWND hwndFrom;
//    UINT idFrom;
//    UINT code;
//    } NMHDR;
//
//  typedef struct _enlink {
//    NMHDR nmhdr;
//    UINT msg;
//    WPARAM wParam;
//    LPARAM lParam;
//    CHARRANGE chrg;
//    } ENLINK;
//
//  typedef struct _enprotected {
//    NMHDR nmhdr;
//    UINT msg;
//    WPARAM wParam;
//    LPARAM lParam;
//    CHARRANGE chrg;
//    } ENPROTECTED;
//
void __fastcall TTaeRichEdit::CNNotify(TWMNotify& Message)
{
  switch (Message.NMHdr->code)
  {
    case EN_UPDATE: // Don't forget to set the ENM_UPDATE flag!
      UpdateEvent();
//      ::SendMessage(Handle, EM_EXGETSEL, &FColumn, NULL); // S.S. 5/2015
      break;

    case EN_SELCHANGE:
      SelectionChange();
    break;

    case EN_REQUESTRESIZE:
      RequestSize(((TReqSize*) Message.NMHdr)->rc);
    break;

    case EN_SAVECLIPBOARD:
      if (!SaveClipboard(((TENSaveClipboard*) Message.NMHdr)->cObjectCount,
        ((TENSaveClipboard*) Message.NMHdr)->cch))
        Message.Result = 1;    // do not save
    break;

    // note: the following code handles EN_PROTECTED notifications and
    // passes them to the OnProtectEvent (first) and OnProtectedChange
    // (second) handlers.
    case EN_PROTECTED:
      // note: the OnProtectEvent handler should set the return value in
      // Message.Result to 0 to continue processing the message
      if (FOnProtectEvent)
      {
        FOnProtectEvent(this, Message);
        if (!FOnProtectChange) return;
      }

      if (!ProtectChange(((TENProtected*) Message.NMHdr)->chrg.cpMin,
                            ((TENProtected*) Message.NMHdr)->chrg.cpMax))
        Message.Result = 1;    // do not allow change
    break;

// can't get this to fire (S.S.)
//    case EN_CHANGE: // Get the CHANGENOTIFY Struct
//      // note: the OnChangeEvent handler should set the return value in
//      // Message.Result to 0 to continue processing the message
//      if (FOnChangeEvent)
//      {
//        FOnChangeEvent(this, Message);
//        if (!FOnChangeEvent) return;
//      }
//
//      if (!EditChange(((TENChangeNotify*) Message.NMHdr)->dwChangeType,
//                            ((TENChangeNotify*) Message.NMHdr)->pvCookieData))
//        Message.Result = 1;    // do not allow change
//      break;

    // note: the following code handles EN_LINK notifications and
    // passes them to the OnLinkEvent (first) and OnUrlClick (second)
    // handlers.
    case EN_LINK:
      // S.S. This is called when you even hover the mouse over a link!!!

      // note: the OnLinkEvent handler should set the return value in
      // Message.Result to 0 to continue processing the message
      if (FOnLinkEvent)
      {
        FOnLinkEvent(this, Message);

        if (!FOnUrlClick)
          return;
      }

      // is this a left button down notification that we will handle?
      TENLink& enLink = *((TENLink*) Message.NMHdr);

      if (enLink.msg == WM_LBUTTONDOWN && (FAutoUrlExecute || FOnUrlClick))
      {
        // select the text
        ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &enLink.chrg);

        // S.S. MODIFIED 8/2013 to handle wide characters!!!!!!!!!!!

        int len = enLink.chrg.cpMax - enLink.chrg.cpMin + 1;

        // allocate a wide-char buffer for the text
        wchar_t* buf = new wchar_t[len];

        // get the text
        ::SendMessage(Handle, EM_GETSELTEXT, 0, (LPARAM)buf);

        buf[len-1] = '\0';

        // copy text into a WideString
        WideString linkText(buf);

        delete [] buf;

//        linkText.Unique();

        // if an OnUrlClick handler is installed, call it
        if (FOnUrlClick)
          FOnUrlClick(this, linkText);

        // if the AutoUrlExecute property is (still) set, execute the link
        if (FAutoUrlExecute)
          ExecuteUrl(this, linkText);

        // return non-zero to tell the RE that we handled this message
        Message.Result = 1;

        return;
      }
    break;
  }
}
//---------------------------------------------------------------------------
// save the contents of the clipboard
//
bool __fastcall TTaeRichEdit::SaveClipboard(int NumObj, int NumChars)
{
  bool Result = true;

  if (FOnSaveClipboard) FOnSaveClipboard(this, NumObj, NumChars, Result);
  return Result;
}
//---------------------------------------------------------------------------
// handle an attempt to change protected text
//
bool __fastcall TTaeRichEdit::ProtectChange(int StartPos, int EndPos)
{
  bool Result = false;

  if (FOnProtectChange) FOnProtectChange(this, StartPos, EndPos, Result);
  return Result;
}
//---------------------------------------------------------------------------
// handle change event
//
//bool __fastcall TTaeRichEdit::EditChange(DWORD dwChangeType, void* pvCookieData)
//{
//  bool Result = false;
//
//  if (FOnChange) FOnChange(this, dwChangeType, pvCookieData, Result);
//  return Result;
//}
//---------------------------------------------------------------------------
// selection has changed.  call OnSelectionChange handler.
//
void __fastcall TTaeRichEdit::SelectionChange(void)
{
  if (FOnSelChange) FOnSelChange(this);
}
//---------------------------------------------------------------------------
// update event.  call OnUpdateEvent handler.
//
void __fastcall TTaeRichEdit::UpdateEvent(void)
{
  if (FOnUpdateEvent) FOnUpdateEvent(this);
}
//---------------------------------------------------------------------------
// a EM_REQUESTRESIZE event has occurred.  call handler.
//
void __fastcall TTaeRichEdit::RequestSize(const Windows::TRect& Rect)
{
  if (FOnResizeRequest) FOnResizeRequest(this, Rect);
}
//---------------------------------------------------------------------------
// locate text in the control
//
/*
int __fastcall TTaeRichEdit::FindText(AnsiString text, int StartPos,
  int Length, TSearchTypes2 searchOptions)
{
#if __BORLANDC__ < 0x0550
  FINDTEXTEX ft;
#else
  Richedit::FINDTEXTEX ft;
#endif

  ft.chrg.cpMin = StartPos;
  ft.chrg.cpMax = ft.chrg.cpMin + Length;

  /////////// S.S. Modified 5/2014 /////////////////////
  int destSize = text.Length()*sizeof(wchar_t) + sizeof(wchar_t);
  wchar_t* destBuf = new wchar_t[destSize];
  ft.lpstrText = (char *)StringToWideChar(text, destBuf, destSize);
  ///////////////////////////////////////////////////////

  WPARAM flags = 0;
  if (!searchOptions.Contains(st2Backward)) flags |= FR_DOWN;    // and this is documented WHERE???
  if (searchOptions.Contains(st2MatchCase)) flags |= FR_MATCHCASE;  // apparently could use FR_MATCHCASE
  if (searchOptions.Contains(st2WholeWord)) flags |= FR_WHOLEWORD;  // apparently could use FR_WHOLEWORD

  ft.lpstrText = text.c_str();

  int pos = ::SendMessage(Handle, EM_FINDTEXT, flags, (LPARAM) &ft);

  delete [] destBuf; // S.S. 5/2014

  return pos;
}
*/
//---------------------------------------------------------------------------
// locate text in the control
//
//FR_DOWN
//If set, the operation searches from the end of the current selection to the
// end of the document. If not set, the operation searches from the end of the
// current selection to the beginning of the document.
//FR_MATCHALEFHAMZA
//By default, Arabic and Hebrew alefs with different accents are all matched by
// the alef character. Set this flag if you want the search to differentiate
// between alefs with different accents.
//FR_MATCHCASE
//If set, the search operation is case-sensitive. If not set, the search
// operation is case-insensitive.
//FR_MATCHDIAC
//By default, Arabic and Hebrew diacritical marks are ignored. Set this flag
// if you want the search operation to consider diacritical marks.
//FR_MATCHKASHIDA
//By default, Arabic and Hebrew kashidas are ignored. Set this flag if you want
// the search operation to consider kashidas.
//FR_WHOLEWORD
//If set, the operation searches only for whole words that match the search
// string. If not set, the operation also searches for word fragments that
// match the search string.
int __fastcall TTaeRichEdit::FindTextW(const WideString SearchStr, int StartPos,
  int Length, TSearchTypes Options)
{
  FINDTEXTW Find;
  int Flags = 0;

  Find.chrg.cpMin = StartPos;
  Find.chrg.cpMax = Find.chrg.cpMin + Length;

  if (Options.Contains(stWholeWord)) Flags |= FT_WHOLEWORD;
  if (Options.Contains(stMatchCase)) Flags |= FT_MATCHCASE;

  // have to cast to char* but it's realy wchar_t*
  Find.lpstrText = SearchStr.c_bstr();

  return (::SendMessageW(Handle, EM_FINDTEXTW, Flags, (LPARAM) &Find));
}

// Keep this for backward compatibility with old versions of YahCoLoRiZe
int __fastcall TTaeRichEdit::FindText(const String SearchStr, int StartPos,
  int Length, TSearchTypes Options)
{
  return FindTextW(WideString(SearchStr), StartPos, Length, Options);
}
//---------------------------------------------------------------------------
// enhanced search function -- supports backwards searching.
// note: although RE 1.0 supports the EM_FINDTEXTEX message, it always
// searches forward.
//
bool __fastcall TTaeRichEdit::FindTextExW(WideString text, int startPos,
  TSearchTypes2 searchOptions, int& foundPos, int& foundLength)
{
  FINDTEXTEXW ft;

  /////////// S.S. Modified 5/2014 and 6/2015 /////////////////////
  ft.lpstrText = text.c_bstr();
  ft.chrg.cpMin = startPos;

  if (foundLength == -1)
    ft.chrg.cpMax = -1; // Search all...
  else
  {
    if (searchOptions.Contains(st2Backward))
      ft.chrg.cpMax = startPos-foundLength;
    else
      ft.chrg.cpMax = startPos+foundLength;
  }

  // This is returned with the range of text found
  ft.chrgText.cpMin = -1;
  ft.chrgText.cpMax = -1;

  ///////////////////////////////////////////////////////

  WPARAM flags = 0;
  if (!searchOptions.Contains(st2Backward)) flags |= FR_DOWN;    // and this is documented WHERE???
  if (searchOptions.Contains(st2MatchCase)) flags |= FR_MATCHCASE;  // apparently could use FR_MATCHCASE
  if (searchOptions.Contains(st2WholeWord)) flags |= FR_WHOLEWORD;  // apparently could use FR_WHOLEWORD

  int pos = ::SendMessageW(Handle, EM_FINDTEXTEXW, flags, (LPARAM) &ft);

  if (pos != -1)
  {
    foundPos = ft.chrgText.cpMin;
    foundLength = ft.chrgText.cpMax - foundPos;
  }

  return pos != -1;
}

// Keep this for backward compatibility with old versions of YahCoLoRiZe
bool __fastcall TTaeRichEdit::FindTextEx(String text, int startPos,
  TSearchTypes2 searchOptions, int& foundPos, int& foundLength)
{
  return FindTextExW(WideString(text), startPos, searchOptions,
                                                  foundPos, foundLength);
}
//---------------------------------------------------------------------------
// note: GetTextLen() is not virtual in base -- not sure what happens if
// you do not call this using a properly cast'd pointer to a TTaeRichEdit.
// some of the VCL uses this method and I am not sure whether this version
// actually gets called.
//
// S.S. 07/17/00: (It never gets called)
///* Flags for the GETTEXTLENGTHEX data structure              */
//#define GTL_DEFAULT    0  /* do the default (return # of chars)    */
//#define GTL_USECRLF    1  /* compute answer using CRLFs for paragraphs*/
//#define GTL_PRECISE    2  /* compute a precise answer          */
//#define GTL_CLOSE     4  /* fast computation of a "close" answer    */
//#define GTL_NUMCHARS  8  /* return the number of characters      */
//#define GTL_NUMBYTES  16  /* return the number of _bytes_        */
//
// /* EM_GETTEXTLENGTHEX info; this struct is passed in the wparam of the msg */
// typedef struct _gettextlengthex
// {
//  DWORD  flags;      /* flags (see GTL_XXX defines)        */
//  UINT  codepage;    /* code page for translation (CP_ACP for default,
//                 1200 for Unicode              */
// } GETTEXTLENGTHEX;
//
// Note: to make GetTextLen() work, I had to call it like this:
//  int iSize = static_cast<TTaeRichEdit*>(TaeRichEdit1)->GetTextLen();
//

//int __fastcall TTaeRichEdit::GetTextLen(void)
//{
//  GETTEXTLENGTHEX gtlx = { GTL_PRECISE, CP_ACP };
//  return ::SendMessage(Handle, EM_GETTEXTLENGTHEX, (WPARAM) &gtlx, 0);
//}

// S.S. 07/17/00: Allow above flags to work
// The old call will work as designed above, but the
// caller can now specify flags to control how
// GetTextLen() operates by our overloading of the function
//
// GTL_DEFAULT Returns the number of characters. This is the default (counts a cr/lf as one char! - S.S.).
// GTL_USECRLF Computes the answer by using CR/LFs at the end of paragraphs.
// GTL_PRECISE Computes a precise answer. This approach could necessitate a conversion and thereby take longer.
//   This flag cannot be used with the GTL_CLOSE flag. E_INVALIDARG will be returned if both are used.
// GTL_CLOSE Computes an approximate (close) answer. It is obtained quickly and can be used to set the buffer size.
//   This flag cannot be used with the GTL_PRECISE flag. E_INVALIDARG will be returned if both are used.
// GTL_NUMCHARS Returns the number of characters. This flag cannot be used with the GTL_NUMBYTES flag. E_INVALIDARG
//   will be returned if both are used.
// GTL_NUMBYTES Returns the number of bytes. This flag cannot be used with the GTL_NUMCHARS flag. E_INVALIDARG
//   will be returned if both are used.
//int __fastcall TTaeRichEdit::GetTextLen(DWORD flags)
//{
//  GETTEXTLENGTHEX gtlx = { flags, CP_ACP };
//  return ::SendMessage(Handle, EM_GETTEXTLENGTHEX, (WPARAM) &gtlx, 0);
//}
//---------------------------------------------------------------------------
// set the default font attributes
//
void __fastcall TTaeRichEdit::SetFont2(TFont2* font)
{
  FFont2->Assign(font);
  FDefAttributes->Assign(font);
}
//---------------------------------------------------------------------------
// get screen position from characterscroll position
//
TPoint __fastcall TTaeRichEdit::GetCharCoordinates(long Index)
// S.S. 4/2014 Send the 0-based index of the character (SelStart)
// Returns the x,y coordinates relative to the upper-left of the edit-control
// (in screen units)
{
  POINTL pt;
  ::SendMessage(Handle, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)Index);
  TPoint newPt;
  newPt.x = pt.x;
  newPt.y = pt.y;
  return newPt;
}
//---------------------------------------------------------------------------
// get the current SelLine (in y) and corrected SelStart (in x) at the caret
//
TPoint __fastcall TTaeRichEdit::GetCaretPos(void)
{
  CHARRANGE CharRange;
  TPoint pt;

  ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM)&CharRange);
  pt.x = CharRange.cpMax;
  pt.y = GetLineByIndex(pt.x);
  pt.x -= ::SendMessage(Handle, EM_LINEINDEX, -1, 0);
  return pt;
}
//---------------------------------------------------------------------------
// get the current Line (in y) and corrected Column (in x) at Index
//
TPoint __fastcall TTaeRichEdit::GetPosByIndex(long Index)
{
  long x = 0;
  long y = 0;

  if (Index >= 0)
  {
    // Get the line at the index
    if ((y = ::SendMessage(Handle, EM_EXLINEFROMCHAR, 0, (LPARAM)Index)) >= 0)
      // Column = Index - index at the start of the line
      x = Index - ::SendMessage(Handle, EM_LINEINDEX, (WPARAM)y, 0);
  }
  
  TPoint pt;
  pt.x = x;
  pt.y = y;

  return pt;
}
//---------------------------------------------------------------------------
// set caret to line y and index x in p
//
void __fastcall TTaeRichEdit::SetCaretPos(TPoint p)
{
  // first get the char-index at the beginning of line p.y and add p.x
  long idx = ::SendMessage(Handle, EM_LINEINDEX, p.y, 0) + p.x;
  ::SendMessage(Handle, EM_SETSEL, idx, idx); // set the caret, no selection
}
//---------------------------------------------------------------------------
// get scroll position
//
TPoint __fastcall TTaeRichEdit::GetScrollPos(void)
// S.S. 8/2013
{
  TPoint pt;
  ::SendMessage(Handle, EM_GETSCROLLPOS, 0, (LPARAM)&pt);
  return pt;
}
//---------------------------------------------------------------------------
// set scroll position
//
void __fastcall TTaeRichEdit::SetScrollPos(TPoint pt)
// S.S. 8/2013
{
  ::SendMessage(Handle, EM_SETSCROLLPOS, 0, (LPARAM)&pt);
}
//---------------------------------------------------------------------------
// get the column number of the cursor
//
long __fastcall TTaeRichEdit::GetColumn(void)
{
  CHARRANGE chrg;
  ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM) &chrg);
  return chrg.cpMin - ::SendMessage(Handle, EM_LINEINDEX, (WPARAM)-1, 0); // Changed to -1 5/2015
}
//---------------------------------------------------------------------------
// get the column at a specific character index
//
long __fastcall TTaeRichEdit::GetColumnByIndex(long Index)
{
  long retVal = 0;

  if (Index >= 0)
  {      
    // Get the line at the index
    long lineIdx = ::SendMessage(Handle, EM_EXLINEFROMCHAR, 0, (LPARAM)Index);

    if (lineIdx >= 0)
    {
      // Get index at the start of the line
      long charIdx = ::SendMessage(Handle, EM_LINEINDEX, (WPARAM)lineIdx, 0);

      retVal = Index - charIdx;
    }
  }
  // Subtract the index at the start of the line from Value to get the column
  return retVal;
}
//---------------------------------------------------------------------------
// get the line number at the caret OR at the start of the selection
//
long __fastcall TTaeRichEdit::GetLine(void)
{
  return GetLineByIndex(-1);
}
//---------------------------------------------------------------------------
// get the line number given a character-index
//
long __fastcall TTaeRichEdit::GetLineByIndex(long Index)
{
  return ::SendMessage(Handle, EM_EXLINEFROMCHAR, 0, (LPARAM)Index);
}
//---------------------------------------------------------------------------
// move the cursor to a specific line
//
void __fastcall TTaeRichEdit::SetLine(long Value)
{
  CHARRANGE chrg;
  chrg.cpMin = chrg.cpMax = ::SendMessage(Handle, EM_LINEINDEX,
                                                     (WPARAM) Value, 0);
  if (chrg.cpMin == -1)  // past end of text so find last line
    chrg.cpMin = chrg.cpMax = TextLength; // Property that calls GetTextLength() S.S.
  ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &chrg);
}
//---------------------------------------------------------------------------
// get the character's index at Line
//
long int __fastcall TTaeRichEdit::GetCharIdx(long Line)
{
  return ::SendMessage(Handle, EM_LINEINDEX, (WPARAM) Line, 0);
}
//---------------------------------------------------------------------------
// move the cursor to a specific column on the current line
//
void __fastcall TTaeRichEdit::SetColumn(long column)
{
  CHARRANGE chrg;
  long currLine = GetLine();
  long ndx = ::SendMessage(Handle, EM_LINEINDEX, (WPARAM) currLine, 0);
  long cols = ::SendMessage(Handle, EM_LINEINDEX, (WPARAM) currLine + 1, 0) -
    ndx - 1;
  if (column > cols) column = cols;
  chrg.cpMin = chrg.cpMax = ndx + column;
  ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &chrg);
}
//---------------------------------------------------------------------------
// install or remove OLE support
//
void __fastcall TTaeRichEdit::SetOleSupport(bool Value)
{
  if (Value != FOleSupport) {
    FOleSupport = Value;
    if (Value) {
      FRichEditOle = new TIRichEditOle(this);
      SetOleAllowTextDrag(FOleAllowTextDrag);
      SetOleAcceptDrop(FOleAcceptDrop);
      return;
      }
    if (FRichEditOle) {
      delete FRichEditOle;
      FRichEditOle = 0;
      }
    }
}
//---------------------------------------------------------------------------
// install or remove Print support
//
// S.S. added 2/1/15
void __fastcall TTaeRichEdit::SetPrintSupport(bool Value)
{
  if (Value == true)
  {
    if (FRichEditPrint == NULL)
    {
      // Create TTaeEditAdvPrint
      // (via typedef TTaeRichEditAdvPrint TTaePrint; in TaeRichEdit.h!)
      try { FRichEditPrint = new TTaePrint(this); }
      catch (...) {}
    }
  }
  else if (FRichEditPrint != NULL)
  {
    try {
      delete FRichEditPrint;
      FRichEditPrint = NULL;
    }
    catch (...) {}
  }

  FPrintSupport = (FRichEditPrint == NULL) ? false : true;
}
//---------------------------------------------------------------------------
// set the application and document name displayed in OLE editing windows
//
void TTaeRichEdit::SetOleHostNames(AnsiString hostApp, AnsiString hostDoc)
{
  if (!FOleSupport || !FRichEditOle) return;
  FRichEditOle->SetHostNames(hostApp, hostDoc);
}
//---------------------------------------------------------------------------
// close all active OLE objects
//
bool TTaeRichEdit::CloseActiveOleObjects(bool savePrompt)
{
  if (!FRichEditOle) return false;
  return FRichEditOle->CloseActiveObjects(savePrompt);
}
//---------------------------------------------------------------------------
// insert a new object from the Insert Object dialog
//
bool __fastcall TTaeRichEdit::InsertObject(void)
{
  if (!FOleSupport || !FRichEditOle) return false;
  return FRichEditOle->InsertObject();
}
//---------------------------------------------------------------------------
// paste from the clipboard using the Paste Special dialog
//
bool __fastcall TTaeRichEdit::PasteSpecial(void)
{
  if (!FOleSupport || !FRichEditOle) return false;
  return FRichEditOle->PasteSpecial();
}
//---------------------------------------------------------------------------
// paste from the clipboard in a specific format
//
void __fastcall TTaeRichEdit::PasteSpecialFromClipboard(unsigned int format)
{
  if (!FRichEditOle) return;
  if (PlainText) format = CF_TEXT;
  ::SendMessage(Handle, EM_PASTESPECIAL, (WPARAM) format, 0);
}
//---------------------------------------------------------------------------
// test to see if a particular format can be pasted into the control
//
bool TTaeRichEdit::CanPasteFormat(unsigned int format)
{
  if (PlainText && format != CF_TEXT) return false;
  return ::SendMessage(Handle, EM_CANPASTE, (WPARAM) format, 0);
}
//---------------------------------------------------------------------------
// the following four functions get and set the state of OLE drag/drop
// flags.  note that these property getters/setters (and the related
// properties) should not be relied upon to determine the current ability
// of the control to handle the OLE activity since it will return the
// shadow variable when OLE support is not enabled.  certainly more logical
// behavior can be achieved using the current design-time/run-time state,
// but this seems good enough for now....
//---------------------------------------------------------------------------
// get the OleAllowTextDrag property
//
bool __fastcall TTaeRichEdit::GetOleAllowTextDrag(void)
{
  if (!FOleSupport || !FRichEditOle || !FRichEditOle->OleCallback)
    return FOleAllowTextDrag;
  return FRichEditOle->OleCallback->AllowTextDrag;
}
//---------------------------------------------------------------------------
// set the OleAllowTextDrag property (see above)
//
void __fastcall TTaeRichEdit::SetOleAllowTextDrag(bool allow)
{
  FOleAllowTextDrag = allow;
  if (!FOleSupport || !FRichEditOle || !FRichEditOle->OleCallback) return;
  FRichEditOle->OleCallback->AllowTextDrag = allow;
}
//---------------------------------------------------------------------------
// get the OleAcceptDrop property (see above)
//
bool __fastcall TTaeRichEdit::GetOleAcceptDrop(void)
{
  if (!FOleSupport || !FRichEditOle || !FRichEditOle->OleCallback)
    return FOleAcceptDrop;
  return FRichEditOle->OleCallback->AcceptDrop;
}
//---------------------------------------------------------------------------
// set the OleAcceptDrop property (see above)
//
void __fastcall TTaeRichEdit::SetOleAcceptDrop(bool accept)
{
  FOleAcceptDrop = accept;
  if (!FOleSupport || !FRichEditOle || !FRichEditOle->OleCallback) return;
  FRichEditOle->OleCallback->AcceptDrop = accept;
}
//---------------------------------------------------------------------------
// check to see if file is likely to be an RTF file based on standard
// RTF header ("\rtf")
//
bool TTaeRichEdit::IsRtfFile(AnsiString filepath)
{
  static char idBytes[5] = "{\\rtf";  // note:  not null-terminated
  char buf[sizeof(idBytes)];
  int fileHandle;
  int bytesRead;
  bool result = false;

  try {
    fileHandle = FileOpen(filepath, fmOpenRead | fmShareDenyWrite);
    bytesRead = FileRead(fileHandle, buf, sizeof(buf));
    FileClose(fileHandle);
    if (bytesRead == sizeof(buf)) result = !memcmp(idBytes, buf, sizeof(buf));
    }
  catch (...) {
    if (fileHandle) FileClose(fileHandle);
    }
  return result;
}
//---------------------------------------------------------------------------
// check if a file is likely to be a binary file -- arbitrarily considered
// binary if has a null byte in first 8192 bytes of file
//
bool TTaeRichEdit::IsLikelyBinaryFile(AnsiString filepath)
{
  char buf[8192];
  ::memset(buf, 'z', sizeof(buf));

  int fileHandle;
  bool result = false;

  try {
    fileHandle = FileOpen(filepath, fmOpenRead | fmShareDenyWrite);
    FileRead(fileHandle, buf, sizeof(buf));
    FileClose(fileHandle);
    result = memchr(buf, '\0', sizeof(buf));
    }
  catch (...) {
    if (fileHandle) FileClose(fileHandle);
    }
  return result;
}
//---------------------------------------------------------------------------
// scroll window to display the caret (useful if cursor is programmatically
// moved when the control does not have focus)
//
void TTaeRichEdit::ScrollToCaret(void)
{
  ::SendMessage(Handle, EM_SCROLLCARET, 0, 0L);
}
//---------------------------------------------------------------------------
// track insert/overstrike state
//
void __fastcall TTaeRichEdit::KeyDown(Word &Key, Classes::TShiftState Shift)
{
  TCustomMemo::KeyDown(Key, Shift);

  TShiftState noShiftKeys;
  if (Key == VK_INSERT && Shift == noShiftKeys) {
    FInsertMode = !FInsertMode;
    if (FOnInsertModeChange) FOnInsertModeChange(this);
    }
}
//---------------------------------------------------------------------------
// set word wrap to none, window, or printer page -- note that there is no way
// for SetWordWrapToPrinter() to handle pages of different widths so
// applications that require pages to have different rendering widths should
// not support SetWordWrapToPrinter()
//
void TTaeRichEdit::SetWordWrapToPrinter(void)
{
  if (FRichEditPrint)
  {
     TRect rect = FRichEditPrint->FRendRect;
     ::SendMessage(Handle, EM_SETTARGETDEVICE, (WPARAM) Printer()->Handle, (LPARAM) (rect.Right - rect.Left));
  }
}
//---------------------------------------------------------------------------
// set word wrap to none, window, or printer page
//
void __fastcall TTaeRichEdit::SetWordWrapTo(TWordWrapTo wwtType)
{
  // if trying to wrap to printer width, must have a printer...
  if (wwtType == wwtPrinter && !Printer()->Handle) wwtType = wwtNone;

  // if wrapping to printer width, set the wrapping mode, and redraw
  if (wwtType == wwtPrinter) {
    SetWordWrapToPrinter();
    }
  // otherwise, clear the wrapping mode
  else{
    // ok, where is *THIS* documented???
    // S.S. (i had to set 1 to Disable word-wrap...
//    ::SendMessage(Handle, EM_SETTARGETDEVICE, 0, wwtType == wwtWindow ? 0 : -1);
// S.S.
    ::SendMessage(Handle, EM_SETTARGETDEVICE, 0, wwtType == wwtWindow ? 0 : 1);
    }

  // only need horizontal scroll bars if wrapping to window
  if (FScrollBars == ssNone)
    ScrollBars = ssNone;
  else if (FScrollBars == ssVertical)
    ScrollBars = ssVertical;
  else if (FScrollBars == ssHorizontal)
    ScrollBars = (wwtType == wwtWindow) ? ssVertical : ssNone;
  else if (FScrollBars == ssBoth)
    ScrollBars = (wwtType == wwtWindow) ? ssVertical : ssBoth;

  FWordWrapTo = wwtType;
}
//---------------------------------------------------------------------------
// get the printing header line(s)
//
TTaeHeaderText* __fastcall TTaeRichEdit::GetHeader(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->Header;
  else
    return 0;
}
//---------------------------------------------------------------------------
// set the printing header line(s)
//
void __fastcall TTaeRichEdit::SetHeader(TTaeHeaderText* header)
{
  if (FRichEditPrint)
     FRichEditPrint->Header = header;
}
//---------------------------------------------------------------------------
// get the printing footer line(s)
//
TTaeHeaderText* __fastcall TTaeRichEdit::GetFooter(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->Footer;
  else
    return 0;
}
//---------------------------------------------------------------------------
// set the printing footer line(s)
//
void __fastcall TTaeRichEdit::SetFooter(TTaeHeaderText* footer)
{
  if (FRichEditPrint)
     FRichEditPrint->Footer = footer;
}
//---------------------------------------------------------------------------
// get the printing header line(s) used if first page is different
//
TTaeHeaderText* __fastcall TTaeRichEdit::GetFirstHeader(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->FirstHeader;
  else
    return 0;
}
//---------------------------------------------------------------------------
// set the printing header line(s) used if first page is different
//
void __fastcall TTaeRichEdit::SetFirstHeader(TTaeHeaderText* firstHeader)
{
  if (FRichEditPrint)
     FRichEditPrint->FirstHeader = firstHeader;
}
//---------------------------------------------------------------------------
// get the printing footer line(s) used if first page is different
//
TTaeHeaderText* __fastcall TTaeRichEdit::GetFirstFooter(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->FirstFooter;
  else
    return NULL;
}
//---------------------------------------------------------------------------
// set the printing footer line(s) used if first page is different
//
void __fastcall TTaeRichEdit::SetFirstFooter(TTaeHeaderText* firstFooter)
{
  if (FRichEditPrint)
     FRichEditPrint->FirstFooter = firstFooter;
}
//---------------------------------------------------------------------------
// get the FirstPageDifferent property value
//
bool __fastcall TTaeRichEdit::GetFirstPageDifferent(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->UseFirstHeaderFooter;
  else
    return false;
}
//---------------------------------------------------------------------------
// set the FirstPageDifferent property value
//
void __fastcall TTaeRichEdit::SetFirstPageDifferent(bool different)
{
  if (FRichEditPrint)
     FRichEditPrint->UseFirstHeaderFooter = different;
}
//---------------------------------------------------------------------------
// get the BorderLines property value
//
TTaeBorderLines __fastcall TTaeRichEdit::GetBorderLines(void)
{
  TTaeBorderLines Temp;

  if (FRichEditPrint)
     return FRichEditPrint->Borders;
  else
     return Temp;
}
//---------------------------------------------------------------------------
// set the BorderLines property value
//
void __fastcall TTaeRichEdit::SetBorderLines(TTaeBorderLines borderLines)
{
  if (FRichEditPrint)
     FRichEditPrint->Borders = borderLines;
}
//---------------------------------------------------------------------------
// get the InsideMargin propery value
//
int __fastcall TTaeRichEdit::GetInsideMargin(void)
{
  if (FRichEditPrint)
     return FRichEditPrint->InsideMargin;
  else
    return 0;
}
//---------------------------------------------------------------------------
// set the InsideMargin property value
//
void __fastcall TTaeRichEdit::SetInsideMargin(int marginTwips)
{
  if (FRichEditPrint)
     FRichEditPrint->InsideMargin = marginTwips;
}
//---------------------------------------------------------------------------
// get the BorderWidth property value
//
int __fastcall TTaeRichEdit::GetBorderWidth(void)
{
  if (FRichEditPrint)
    return FRichEditPrint->BorderWidth;
  else
    return 0;
}
//---------------------------------------------------------------------------
// set the BorderWidth property value
//
void __fastcall TTaeRichEdit::SetBorderWidth(int borderWidthTwips)
{
  if (FRichEditPrint)
     FRichEditPrint->BorderWidth = borderWidthTwips;
}
//#endif // ADV_PRINT_SUPPORT
//---------------------------------------------------------------------------
// the following functions get and set tabstops -- maybe should move to
// the paragraph property?
//
int __fastcall TTaeRichEdit::GetTabStops(TTaeTabStops &tabStops)
{
  for (int i = 0; i < MAX_TAB_STOPS; i++)
    tabStops[i] = FTabStops[i];
  
  return FTabCount;
}
//---------------------------------------------------------------------------
// Property setter for TabCount (S.S.)
// -1 is MAX_TAB_COUNT
void __fastcall TTaeRichEdit::SetTabCount(int tabCount)
{
  if (FTabCount != tabCount)
  {
    FTabCount = tabCount < 0 ? MAX_TAB_STOPS : tabCount;
	
    if (FTabCount > 0)
      SetTabStops();
  }
}
//---------------------------------------------------------------------------
// Property setter for TabWidth (S.S.)
void __fastcall TTaeRichEdit::SetTabWidth(int tabWidth)
{
  if (FTabWidth != tabWidth)
  {
    FTabWidth = tabWidth;
    
    if (FTabCount > 0)
      SetTabStops();
  }
}
//---------------------------------------------------------------------------
void __fastcall TTaeRichEdit::SetTabStops(bool entireDocument)
{
  SetTabStops(this->FTabWidth, this->Font, entireDocument);
}
//---------------------------------------------------------------------------
void __fastcall TTaeRichEdit::SetTabStops(int tabWidth, TFont2* font, bool entireDocument)
{
  int tabSizeTwips = GetTabWidthTwips(tabWidth, font);
  
  if (tabSizeTwips)
  {
    // hide and save selection
    CHARRANGE chrg, chrgAll;
    ::SendMessage(Handle, EM_EXGETSEL, 0, (LPARAM) &chrg);

    // if entireDocument, set selection to all
    if (entireDocument)
    {
      chrgAll.cpMin = 0;
      chrgAll.cpMax = -1;
      ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &chrgAll);
    }

    // set tab size
    SetTabStops(abs(tabSizeTwips)); // no error on failure...

    // restore selection
    ::SendMessage(Handle, EM_EXSETSEL, 0, (LPARAM) &chrg);
  }
}
//---------------------------------------------------------------------------
// Set tab-stops equal-size and in twips
void __fastcall TTaeRichEdit::SetTabStops(int tabStopTwips)
{
  TTaeTabStops tabs;
  
  int ii;
  
  for (ii = 0; ii < FTabCount && ii < MAX_TAB_STOPS; ii++)
    tabs[ii] = (ii + 1) * tabStopTwips;

  SetTabStops(ii, tabs);
}
//---------------------------------------------------------------------------
// Set X tab-stops via an array
void __fastcall TTaeRichEdit::SetTabStops(int tabCount, TTaeTabStops &tabStops)
{
  PARAFORMAT2 pfmt;
  pfmt.cbSize = sizeof(pfmt);

  pfmt.dwMask = PFM_TABSTOPS;

  int ii;
  
  for (ii = 0; ii < tabCount && ii < MAX_TAB_STOPS; ii++) {
    pfmt.rgxTabs[ii] = tabStops[ii];
    FTabStops[ii] = tabStops[ii];
    }

  pfmt.cTabCount = (SHORT)ii;
  FTabCount = ii;

  if (::IsWindow(WindowHandle))
    ::SendMessage(Handle, EM_SETPARAFORMAT, 0, (LPARAM) &pfmt);
}
//---------------------------------------------------------------------------
// Pass in the font and tab width in chars
int __fastcall TTaeRichEdit::GetTabWidthTwips(void)
{
  return GetTabWidthTwips(FTabWidth, FFont2);
}

int __fastcall TTaeRichEdit::GetTabWidthTwips(int tabWidth, TFont2* font)
{
  HDC hdc = ::GetDC(Handle);
  ::SaveDC(hdc);

  ::SetMapMode(hdc, MM_TEXT);

  TFont2* afont = new TFont2();
  afont->Assign(font);
  ::SelectObject(hdc, afont->Handle);

  AnsiString s = AnsiString::StringOfChar('X', tabWidth);
  SIZE sz;
  int cx = GetDeviceCaps(hdc, LOGPIXELSX);
  
  int tabSizeTwips;
  
  if (::GetTextExtentPoint32(hdc, s.c_str(), tabWidth, &sz))
    tabSizeTwips = ::MulDiv(sz.cx, 1440, cx);
  else
    tabSizeTwips = 0;

  ::RestoreDC(hdc, -1);
  ::ReleaseDC(Handle, hdc);

  delete afont;

  return tabSizeTwips;
}
//---------------------------------------------------------------------------
// set the Transparent property (transparent background)
//
void __fastcall TTaeRichEdit::SetTransparent(bool transparent)
{
  FTransparent = transparent;
  LONG exStyle = ::GetWindowLong(Handle, GWL_EXSTYLE);
  if (transparent) {
    exStyle |= WS_EX_TRANSPARENT;
    Brush->Style = bsClear;
    }
  else {
    exStyle &= ~(WS_EX_TRANSPARENT);
    Brush->Style = bsSolid;
    }
  ::SetWindowLong(Handle, GWL_EXSTYLE, exStyle);
}
//---------------------------------------------------------------------------
// set the autocorrect procedure (use for Win 8 only i think...)
//
//void __fastcall TTaeRichEdit::SetAutoCorrectProc(void)
//{
//  ::SendMessage(Handle, EM_SETAUTOCORRECTPROC, (WPARAM)&AutoCorrectProc, 0);
//}
//---------------------------------------------------------------------------
// enable or disable screen redraws
//
void __fastcall TTaeRichEdit::SetEnableRedraw(bool enable)
{
  ::SendMessage(Handle, WM_SETREDRAW, enable, 0);
  FEnableRedraw = enable;
}
//---------------------------------------------------------------------------
// enable or disable event handling -  useful to disable certain events from
// firing while syntax highlighting, for example.  (this completely disables
// the events so that the control never generates the internal notifications
// and, thereby, eliminates execution of some additional code beyond setting
// each OnXXX handler to null.)  in other words, if you are changing text
// or text attributes programmatically, use this to speed things up as much
// as possible.
//
//ENM_CHANGE
//  Sends EN_CHANGE notifications.
//ENM_CLIPFORMAT
//  Sends EN_CLIPFORMAT notifications.
//ENM_CORRECTTEXT
//  Sends EN_CORRECTTEXT notifications.
//ENM_DRAGDROPDONE
//  Sends EN_DRAGDROPDONE notifications.
//ENM_DROPFILES
//  Sends EN_DROPFILES notifications.
//ENM_IMECHANGE
//  Microsoft Rich Edit 1.0 only: Sends EN_IMECHANGE notifications when the IME
//  conversion status has changed. Only for Asian-language versions of the
//  operating system.
//ENM_KEYEVENTS
//  Sends EN_MSGFILTER notifications for keyboard events.
//ENM_LINK
//  Rich Edit 2.0 and later: Sends EN_LINK notifications when the mouse pointer
//  is over text that has the CFE_LINK and one of several mouse actions is
//  performed.
//ENM_LOWFIRTF
//  Sends EN_LOWFIRTF notifications.
//ENM_MOUSEEVENTS
//  Sends EN_MSGFILTER notifications for mouse events.
//ENM_OBJECTPOSITIONS
//  Sends EN_OBJECTPOSITIONS notifications.
//ENM_PARAGRAPHEXPANDED
//  Sends EN_PARAGRAPHEXPANDED notifications.
//ENM_PROTECTED
//  Sends EN_PROTECTED notifications.
//ENM_REQUESTRESIZE
//  Sends EN_REQUESTRESIZE notifications.
//ENM_SCROLL
//  Sends EN_HSCROLL and EN_VSCROLL notifications.
//ENM_SCROLLEVENTS
//  Sends EN_MSGFILTER notifications for mouse wheel events.
//ENM_SELCHANGE
//  Sends EN_SELCHANGE notifications.
//  This notification code is sent when the caret position changes and no text is
//  selected (the selection is empty). The caret position can change when the user
//  clicks the mouse, types, or presses an arrow key.
//ENM_UPDATE
//  Sends EN_UPDATE notifications.
//  Rich Edit 2.0 and later: this flag is ignored and the EN_UPDATE notifications
//   are always sent. However, if Rich Edit 3.0 emulates Microsoft Rich Edit 1.0,
//   you must use this flag to send EN_UPDATE notifications.
//
//Remarks:
//  The default event mask is ENM_NONE in which case no notifications are sent
//  to the parent window. You can retrieve and set the event mask for a rich
//  edit control by using the EM_GETEVENTMASK and EM_SETEVENTMASK messages.
void __fastcall TTaeRichEdit::SetEnableNotifications(bool enable)
{
  // preserve other events
  unsigned long mask = ::SendMessage(Handle, EM_GETEVENTMASK, 0, 0);

  if (enable)  mask |= ENM_CHANGE | ENM_SELCHANGE | ENM_REQUESTRESIZE |
    ENM_PROTECTED | ENM_LINK | ENM_CORRECTTEXT | ENM_UPDATE;
  else mask &= ~(ENM_CHANGE | ENM_SELCHANGE | ENM_REQUESTRESIZE |
    ENM_PROTECTED | ENM_LINK | ENM_CORRECTTEXT | ENM_UPDATE);

  ::SendMessage(Handle, EM_SETEVENTMASK, 0, (LPARAM) mask);
  FEnableNotifications = enable;
}
//---------------------------------------------------------------------------
// execute a URL or any other program or document which has a program
// association.  for example, ExecuteUrl(this, "http://www.something.com")
// will start the default browser (it must be installed -- no error checking)
// and try to load the www.something.com site.  ExecuteUrl(this, "wordpad.exe")
// will try to load and run WordPad.exe.  ExecuteUrl(this, "somedoc.txt")
// will run NotePad.exe (or whatever is associated with *.txt files) and
// load somedoc.txt into the program.
//
void __fastcall TTaeRichEdit::ExecuteUrl(TObject* Sender, WideString urlText)
{
  // execute the link
  ::ShellExecuteW(NULL, L"open", urlText.c_bstr(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TTaeRichEdit::GetFileTitle(void)
{
  return ExtractFileName(FFileName);
}
//---------------------------------------------------------------------------
// FindWordBreak() - locates word breaks.  returns zero-based character index
// except for wbClassify (returns character class and word break flags) and
// wbIsDelimiter (returns true if delimiter or false otherwise).
//
// the following table summarizes the result of my tests:
//
// if at              msg          does this
// -------------------------------- ------------------- --------------------------------------
// Left edge of word        Move left      Moves to right edge of preceding word
// ""                Move left break    ""
// ""                Move left word    Moves to left edge of preceding word
// ""                Move right      Does not move
// ""                Move right break  ""
// ""                Move right word    Moves to left edge of next word
//
// Middle of word          Move left      Moves to left edge of current word
// ""                Move left break    Moves to right edge of preceding word
// ""                Move left word    Moves to left edge of current word
// ""                Move right      Moves to left edge of next word
// ""                Move right break  Moves to right edge of current word
// ""                Move right word    Moves to left edge of next word
//
// Right edge of word        Move left      Does not move
// ""                Move left break    ""
// ""                Move left word    Moves to left edge of current word
// ""                Move right      Moves to left edge of next word
// ""                Move right break  ""
// ""                Move right word    ""
//
// note that punctuation is a special case of the above.  your mileage will
// probably vary so, if you really, really need this to work "right" (whatever
// your definition of "right" is), then test carefully.
//
// note that this method does not change the current selection.
//
int TTaeRichEdit::FindWordBreak(TWordBreakType wbType, int startPos)
{
  return ::SendMessage(Handle, EM_FINDWORDBREAK, (WPARAM) wbType,
    (LPARAM) startPos);
}
//---------------------------------------------------------------------------
// find the range of text that constitues a "word" at the position "startPos".
// this method does not change the current selection.
//
void TTaeRichEdit::FindWord(TCharRange& chrg, int startPos, bool select)
{
  // move word right
  int tempPos = FindWordBreak(wbMoveWordRight, startPos);

  // if we did not move, then we were at eof?
  if (tempPos == startPos) {
    MessageBeep(0);
    return;
    }

  // otherwise, move back to end of word at selection
  int leftPos = FindWordBreak(wbMoveWordLeft, tempPos);
  int rightPos = FindWordBreak(wbLeft, tempPos);

  // if the right edge of the word is followed by punctuation, then
  // the wbLeft moves us back to the start of the word.  in that
  // case, use the tempPos value...
  if (rightPos == leftPos) rightPos = tempPos;

  // odd behavior if word followed by space followed by punctuation...
  // in that case, rightPos is one (or more?) characters before the
  // right edge of the word.  try to compensate...
  tempPos = FindWordBreak(wbLeftBreak, tempPos);
  if (tempPos > rightPos) rightPos = tempPos;

  // if we got this correct, we should now be able to unambiguously
  // select the word...
  if (select) {
    SelStart = leftPos;
    SelLength = rightPos - leftPos;
    }

  // set return values
  chrg.cpMin = leftPos;
  chrg.cpMax = rightPos;
}
//---------------------------------------------------------------------------
// select the "word" at startPos.  returns the starting offset to the text.
//
int TTaeRichEdit::SelectWord(int startPos)
{
  TCharRange chrg;
  FindWord(chrg, startPos, true);
  return chrg.cpMin;
}
//---------------------------------------------------------------------------
// InsertBitmap() - use this method to insert a TBitmap programmatically.
// returns true on success or false if this TTaeRichEdit instance does not
// support OLE or if the bitmap cannot be inserted due to insufficient
// memory, etc.
//
bool __fastcall TTaeRichEdit::InsertBitmap(Graphics::TBitmap* bmp)
{
  // fail if no OLE support
  if (!OleSupport || !OleInterface) return false;

  // attempt to insert bitmap
  return OleInterface->InsertBitmap(bmp);
}
//---------------------------------------------------------------------------
// InsertContainerObject() - use this method to insert the contents of a
// TOleContainer into the Rich Edit control.  returns true on success or
// false on failure.
//
bool __fastcall TTaeRichEdit::InsertContainerObject(TOleContainer* obj,
  CLIPFORMAT fmt)
{
  if (!FOleSupport || !FRichEditOle) return false;
  return FRichEditOle->InsertContainerObject(obj, fmt);
}
//---------------------------------------------------------------------------
// S.S. Added 5/28/15
//
//---------------------------------------------------------------------------
// This counts cr/lfs as 2 chars each! (SelStart and SelLength count newlines
// as 1 char)
long __fastcall TTaeRichEdit::GetTextLength(void)
{
  return ::SendMessageW(Handle, WM_GETTEXTLENGTH, 0, 0);
}
//---------------------------------------------------------------------------
// S.S. Added 5/28/15
//
WideString __fastcall TTaeRichEdit::GetStringW(long LineIndex)
{
  // Get char-index from LineIndex
  long CharIndex = ::SendMessage(Handle, EM_LINEINDEX, (WPARAM)LineIndex, 0);

  // Get length of line at CharIndex
  DWORD len = ::SendMessageW(Handle, EM_LINELENGTH, CharIndex, 0);

  if (!len) return "";

  WideString wText;

  // leave more than enough room for size or null-terminator...
  wchar_t* buf = new wchar_t[len + sizeof(DWORD) + sizeof(wchar_t)];

  ZeroMemory(buf, sizeof(DWORD) + sizeof(wchar_t));
  *((DWORD*)buf) = len; // put # chars to copy at top of buffer

  if (len == (DWORD)::SendMessageW(Handle, EM_GETLINE, LineIndex, (LPARAM)buf))
  {
    // Strip any cr/lf chars
    if (len && (buf[len - 1] == 0x0d || buf[len - 1] == 0x0a)) len--;
    if (len && (buf[len - 1] == 0x0d || buf[len - 1] == 0x0a)) len--;

    buf[len] = '\0'; // terminate

    wText = WideString(buf);
  }

  delete [] buf;

  return wText;
}
//---------------------------------------------------------------------------
// S.S. Added 11/16/15 fixed 11/21/2015
//
void __fastcall TTaeRichEdit::SetStringW(WideString wText, long LineIndex)
{
  // Get char-index from LineIndex
  long iStart = ::SendMessage(Handle, EM_LINEINDEX, (WPARAM)LineIndex, 0);
  long lineLen = ::SendMessageW(Handle, EM_LINELENGTH, iStart, 0);
  SetSelFirstLast(iStart, iStart+lineLen);
  ::SendMessageW(Handle, EM_REPLACESEL, FALSE, (LPARAM)wText.c_bstr());
}
//---------------------------------------------------------------------------
// S.S. Added 11/16/15
//
long __fastcall TTaeRichEdit::GetLineCount(void)
{
  return ::SendMessageW(Handle, EM_GETLINECOUNT, 0, 0);
}
//---------------------------------------------------------------------------
// get the text as an WideString (plaintext)
//
WideString __fastcall TTaeRichEdit::GetTextW(void)
{
  WideString sResult;
  wchar_t* pBuf = NULL;

  try
  {
    pBuf = this->GetRangeBufW(0, -1);
  }
  __finally
  {
    if (pBuf != NULL)
    {
      sResult = WideString(pBuf);
      delete [] pBuf;
    }
  }

  return sResult;
}
//---------------------------------------------------------------------------
// set the text as an WideString (plaintext)
//
void __fastcall TTaeRichEdit::SetTextW(WideString wText)
{
  SETTEXTEX st;
  st.flags = ST_UNICODE;
  st.codepage = 1200; // UTF-16 (UTF-8 is 65001, ANSI is 1252)

  ::SendMessage(Handle, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)wText.c_bstr());
}
//---------------------------------------------------------------------------
// get the text from the control (as plaintext).
//
// Returns the number of wide-chars copied not including the null;
long __fastcall TTaeRichEdit::GetTextBufW(wchar_t* Buffer, long BufSize)
{
  if (Buffer == NULL || BufSize < (long)sizeof(wchar_t))
    return 0;

  long MaxChars = BufSize/2; // how many wide-chars can this buffer hold?

  // Only room for the null?
  if (MaxChars <= 1)
  {
    Buffer[0] = '\0';
    return 0;
  }

  // have room for at least one wide-char...
  WideString S;
  long BytesToCopy;

  S = GetTextW();
  BytesToCopy = min(S.Length()*(long)sizeof(wchar_t),
                                      (MaxChars-1)*(long)sizeof(wchar_t));
  ::memcpy(Buffer, S.c_bstr(), BytesToCopy);
  Buffer[BytesToCopy/2] = '\0';
  return (BytesToCopy/2)-1;
}
//---------------------------------------------------------------------------
// get the selected text as an WideString (plaintext) - if SelLength is 0,
// this function returns the character at the caret!
//
WideString __fastcall TTaeRichEdit::GetSelTextW(void)
{
  WideString sResult;
  wchar_t* pBuf = NULL;

  try
  {
    pBuf = this->GetRangeBufW(-1, 0);
  }
  __finally
  {
    if (pBuf != NULL)
    {
      sResult = WideString(pBuf);
      delete [] pBuf;
    }
  }

  return sResult;
}
//---------------------------------------------------------------------------
// replace the selected text
//
void __fastcall TTaeRichEdit::SetSelTextW(WideString wSelText)
{
  SETTEXTEX st;
  st.flags = ST_SELECTION | ST_UNICODE;
  st.codepage = 1200; // UTF-16 (UTF-8 is 65001, ANSI is 1252)

  ::SendMessage(Handle, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)wSelText.c_bstr());
//  ::SendMessage(Handle, EM_REPLACESEL, FALSE, (LPARAM)wSelText.c_bstr());
}
//---------------------------------------------------------------------------
// get the selected text from the control (as plaintext).
//
// Returns the number of wide-chars copied not including the null;
long __fastcall TTaeRichEdit::GetSelTextBufW(wchar_t* Buffer, long BufSize)
{
  if (Buffer == NULL || BufSize < (long)sizeof(wchar_t))
    return 0;

  long MaxChars = BufSize/2; // how many wide-chars can this buffer hold?

  // Only room for the null?
  if (MaxChars <= 1)
  {
    Buffer[0] = '\0';
    return 0;
  }

  // have room for at least one wide-char...
  WideString S;
  long BytesToCopy;

  S = GetSelTextW();
  BytesToCopy = min(S.Length()*(long)sizeof(wchar_t), (MaxChars-1)*(long)sizeof(wchar_t));
  ::memcpy(Buffer, S.c_bstr(), BytesToCopy);
  Buffer[BytesToCopy/2] = '\0';
  return (BytesToCopy/2)-1;
}
//---------------------------------------------------------------------------
// GetRangeW() - use this method for faster access to the control's text
// (this should be much faster than the SelText property...)
//
WideString __fastcall TTaeRichEdit::GetRangeW(long startPos, long endPos)
{
  WideString sResult;
  wchar_t* pBuf = GetRangeBufW(startPos, endPos);
  WideString Result(pBuf);
  delete[] pBuf;
  return Result;
}
//---------------------------------------------------------------------------
// GetTextRangeW() - use this method for faster access to the control's text
// (this should be much faster than the SelTextW property...)
// (by S.S. 2015)
//
// Set startPos 0 and endPos -1 to return all text.
// Set startPos -1 and endPos 0 to return selected text or char at the caret
// if nothing is selected.
wchar_t* __fastcall TTaeRichEdit::GetRangeBufW(long startPos, long endPos)
{
  if (endPos < 0) // get all text
  {
    startPos = 0;
    endPos = this->TextLength;
  }
  else if (startPos < 0) // get selected text
  {
    int selStart = this->SelStart;
    startPos = selStart;
    endPos = selStart + this->SelLength;
  }

  int dwNum = endPos-startPos;
  
  wchar_t* pwText = new wchar_t[dwNum+1];
  
  if (pwText)
  {
    pwText[dwNum] = '\0';

    if (dwNum > 0)
    {
      TEXTRANGEW chrg = { { startPos,  endPos },  pwText };
      // get the text
      ::SendMessageW(Handle, EM_GETTEXTRANGE, 0, (LPARAM) &chrg);
    }
  }

  // return wchar_t string (can be just 1 null char! Never should be
  // a NULL pointer!)
  return pwText;
}
//---------------------------------------------------------------------------
// get the currently selected text complete with RTF codes
//
AnsiString __fastcall TTaeRichEdit::GetSelTextRtf(void)
{
  return GetRtf(true);
}
//---------------------------------------------------------------------------
// copy RTF encoded text into an AnsiString. note that this function is not
// particularly efficient, but I do not know of any way to determine in
// advance how large a buffer to allocate, so I have to use a stream as
// an intermediate buffer.  I would suggest that you not use this to get
// large blocks of text; there may be size limitations in AnsiString, also.
//
//   selectionOnly - copy only selected text into string; if false, copy
//     entire contents of control into string (not recommended for very
//     large files)
//
AnsiString __fastcall TTaeRichEdit::GetRtf(bool selectionOnly)
{
  // create a memory stream and get the text
  TMemoryStream* memStream = new TMemoryStream();
  CopyToStream(memStream, selectionOnly);
  memStream->Seek(0, soFromBeginning);

  // allocate a buffer of sufficient size and copy the rtf into it
  char* buf = new char[memStream->Size];
  memStream->ReadBuffer(buf, memStream->Size);

  // discard stream
  delete memStream;

  // insert text into an AnsiString, free buffer, and return
  AnsiString s(buf);
  delete[] buf;
  return s;
}
//---------------------------------------------------------------------------
// replace the current selection with RTF encoded text.  note that the
// rtfText value must be equivalent to a *complete* RTF file, i.e., it
// must include the prefixing "{\rtf" and the final closing "}".  if this
// is unclear, use GetSelTextRtf or the SelTextRtf property to fetch
// various selections of text from the control and look at the returned
// values.
//
void __fastcall TTaeRichEdit::SetSelTextRtf(AnsiString rtfText)
{
  PutRtf(rtfText);
}
//---------------------------------------------------------------------------
// copy RTF encoded text from an AnsiString into the control. note that this
// function is not particularly efficient -- it could be made more efficient
// by writing a version of StreamInCallback() modified to work directly with
// a buffer.  if you are using this for large blocks of text, consider
// writing such a version yourself.
//
//   replaceSelection - replace selected text (if selection empty, inserts
//     text; if false, replaces entire contents of control)
//
// note: inserting invalid RTF codes may not raise an exception -- my tests
// indicate that the control simply interprets the RTF as best it can....
//
// replaceSelection defaults true
void __fastcall TTaeRichEdit::PutRtf(AnsiString rtfText, bool replaceSelection)
{
  // create a memory stream and put the text into it
  TMemoryStream* memStream = new TMemoryStream();
  memStream->WriteBuffer(rtfText.c_str(), rtfText.Length() + 1);

  // rewind stream and insert into control
  memStream->Seek(0, soFromBeginning);
  PasteFromStream(memStream, replaceSelection);

  // discard stream
  delete memStream;
}
//---------------------------------------------------------------------------
// insert text from a stream into the control
//
//   stream - exisiting stream; note that stream is not rewound prior to
//     inserting the text into the control
//   selectionOnly - replace current selection (if selection empty, inserts
//      text; if false, replaces entire contents of control)
//   plainText - convert to plain text
//   plainRtf - ignore language-specific RTF codes
//
// note: if you CopyToStream(..., plainText = true...) and then paste back
//   from the stream with PasteFromStream(..., plainText = false...), the
//   WinAPI will return an error condition -- that is, you must paste valid
//   RTF when calling this function with plainText = false.
//
// defaults: true, false, false
void __fastcall TTaeRichEdit::PasteFromStream(TStream* stream, bool selectionOnly,
  bool plainText, bool plainRtf)
{
  TEditStream editStream;
  WPARAM format = 0;

  if (selectionOnly) format |= SFF_SELECTION;
  if (plainRtf) format |= SFF_PLAINRTF;
  format |= (plainText) ? SF_TEXT : SF_RTF; // SF_UNICODE can be combined with SF_TEXT

  editStream.dwCookie = (DWORD) stream;
  editStream.dwError = 0;
  editStream.pfnCallback = StreamInCallback;

  ::SendMessage(Handle, EM_STREAMIN, format, (LPARAM) &editStream);

  if (editStream.dwError) throw EOutOfResources("Failed to load stream.");
}
//---------------------------------------------------------------------------
// copy RTF from one TTaeRichEdit control to another
//
//   toRichEdit - TTaeRichEdit into which to insert the text
//   selectionOnly - copy only selected text from source
//   replaceSelection - replace selected text in destination (if selection
//   empty, inserts text; if false, replaces entire contents of control)
//
// selectionOnly and replaceSelection default true
void __fastcall TTaeRichEdit::CopyRtfTo(TTaeRichEdit& toRichEdit,
  bool selectionOnly, bool replaceSelection)
{
  TMemoryStream* memStream = new TMemoryStream();

  CopyToStream(memStream, selectionOnly);

  memStream->Seek(0, soFromBeginning);
  toRichEdit.PasteFromStream(memStream, replaceSelection);

  delete memStream;
}
//---------------------------------------------------------------------------
// copy the text to a stream
//
//   stream - existing TStream (usually a TMemoryStream)
//   selectionOnly - copy only selected text
//   plainText - convert to plain text
//   noObjects - insert spaces in place of OLE objects
//   plainRtf - ignore language-specific RTF codes
//
// note: text is appended to the stream at the current stream position
//
void __fastcall TTaeRichEdit::CopyToStream(TStream* stream, bool selectionOnly,
  bool plainText, bool noObjects, bool plainRtf)
{
  TEditStream editStream;
  WPARAM format = 0;

  if (selectionOnly) format |= SFF_SELECTION;
  if (plainRtf) format |= SFF_PLAINRTF;
  if (plainText) format |= (noObjects) ? SF_TEXT : SF_TEXTIZED;
  else format |= (noObjects) ? SF_RTFNOOBJS : SF_RTF;

  editStream.dwCookie = (DWORD) stream;
  editStream.dwError = 0;
  editStream.pfnCallback = StreamOutCallback;

  ::SendMessage(Handle, EM_STREAMOUT, format, (LPARAM) &editStream);

  if (editStream.dwError) throw EOutOfResources("Failed to save stream.");
}
//---------------------------------------------------------------------------
// WinAPI callback for auto-correction S.S.
//
//int WINAPI AutoCorrectProc(
//  LANGID langid,
//  const WCHAR *pszBefore,
//  WCHAR *pszAfter,
//  LONG cchAfter,
//  LONG *pcchReplaced
//);
//langid
//Type: LANGID
//Language ID that identifies the autocorrect file to use for automatic
// correcting.
//pszBefore
//Type: const WCHAR*
//Autocorrect candidate string.
//pszAfter
//Type: WCHAR*
//Resulting autocorrect string, if the return value is not ATP_NOCHANGE.
//cchAfter
//Type: LONG
//Count of characters in pszAfter.
//pcchReplaced
//Type: LONG*
//Count of trailing characters in pszBefore to replace with pszAfter.
//Return value
//Type: int
//Returns one or more of the following values.
//ATP_NOCHANGE 0 No change.
//ATP_CHANGE 1 Change but don’t replace most delimiters, and don’t replace
// a span of unchanged trailing characters (preserves their formatting).
//ATP_NODELIMITER 2 Change but don’t replace a span of unchanged trailing
// characters.
//ATP_REPLACEALLTEXT 4 Replace trailing characters even if they are not
// changed (uses the same formatting for the entire replacement string).
//int CALLBACK TTaeRichEdit::AutoCorrectProc(LANGID langid, const WCHAR *pszBefore,
//           WCHAR *pszAfter, LONG cchAfter, LONG *pcchReplaced)
//{
//  return 0;
//}
//---------------------------------------------------------------------------
// WinAPI callback for streaming text into the control
//
DWORD CALLBACK TTaeRichEdit::StreamInCallback(DWORD dwCookie, LPBYTE pbBuff,
  LONG cb, LONG FAR *pcb)
{
  TStream* stream = reinterpret_cast<TStream*>(dwCookie);

  try
  {
    *pcb = stream->Read(pbBuff, cb);
    return 0;
  }
  catch (EReadError& e)
  {
    *pcb = 0;
    return 1;
  }
}
//---------------------------------------------------------------------------
// WinAPI callback for streaming text out of the control
//
DWORD CALLBACK TTaeRichEdit::StreamOutCallback(DWORD dwCookie, LPBYTE pbBuff,
  LONG cb, LONG FAR *pcb)
{
  TStream* stream = reinterpret_cast<TStream*>(dwCookie);

  try
    {
    *pcb = stream->Write(pbBuff, cb);
    return 0;
    }
  catch (EWriteError& e)
    {
    *pcb = 0;
    return 1;
    }
}

#pragma pack(pop)
//---------------------------------------------------------------------------
