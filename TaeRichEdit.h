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
// TaeRichEdit.h - header file for TTaeRichEdit class.
//---------------------------------------------------------------------------
// note:  if you have trouble compiling this header, be sure that it is not
// open in the IDE.  there is a known problem compiling headers > 16k while
// open in the IDE.
//---------------------------------------------------------------------------
#ifndef TaeRichEditH
#define TaeRichEditH
//---------------------------------------------------------------------------
// NOTE: The actual version is defined by using Options->Version tab prior
// to building - then this info is stored in FRichEditVersionInfo
// and we access that property with RichEditVersionInfo. S.S.

// this "may" be used in some system header files? (S.S.)
#ifdef _RICHEDIT_VER
#undef _RICHEDIT_VER
#endif
#define _RICHEDIT_VER 0x0400

// this is used to load FRichEditDllVersion
#define RICHEDIT_DLL_VER 4

#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <StdCtrls.hpp>
#include <vcl\olectnrs.hpp>

#include "TaeCompileOpts.h"
#include "TaeAttrib2.h"
#include "TaePageLayout.h"
#include "TaeVerInfo.h"
#include "TaePosition.h"
#include "TaeRichEditStrings.h"
//---------------------------------------------------------------------------
// rich edit 3.0 constants
//---------------------------------------------------------------------------
#include "TaeRichEdit30.h"
//---------------------------------------------------------------------------
#pragma pack(push, 4)
//---------------------------------------------------------------------------
// forward declarations
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit;
class PACKAGE TTaeRichEditStrings;
class PACKAGE TIRichEditOle;
class PACKAGE TTaeTextAttributes;
class PACKAGE TTaeTextAttributes2;
class PACKAGE TTaeParaAttributes;
class PACKAGE TTaeParaAttributes2;
class PACKAGE TTaeRichEditPrint;
class PACKAGE TTaeRichEditAdvPrint;
//---------------------------------------------------------------------------
typedef NMHDR TTaeNMHdr;
typedef ENLINK TTaeENLink;
//typedef ENPROTECTED TTaeENProtected;

//typedef struct _CHANGENOTIFY {
//  DWORD dwChangeType;
//  void  *pvCookieData;
//} CHANGENOTIFY;

//typedef CHANGENOTIFY TENChangeNotify;

//typedef void __fastcall (__closure *TTaeRichEditChange)(TObject* Sender,
//  DWORD dwChangeType, void* pvCookieData, bool &AllowChange);
//typedef void __fastcall (__closure *TTaeRichEditChangeEvent)(TObject* Sender,
//  TWMNotify& Message);

typedef void __fastcall (__closure *TTaeRichEditProtectChange)(TObject* Sender,
  int StartPos, int EndPos, bool &AllowChange);
typedef void __fastcall (__closure *TTaeRichEditProtectEvent)(TObject* Sender,
  TWMNotify& Message);

typedef void __fastcall (__closure *TTaeRichEditUrlClick)(TObject* Sender,
  AnsiString urlText);
typedef void __fastcall (__closure *TTaeRichEditLinkEvent)(TObject* Sender,
  TWMNotify& Message);

typedef enum { wwtNone, wwtWindow, wwtPrinter } TWordWrapTo;

typedef int TTaeTabStops[MAX_TAB_STOPS];

enum TSearchType2 { st2WholeWord, st2MatchCase, st2Backward };
typedef Set<TSearchType2, stWholeWord, st2Backward> TSearchTypes2;

enum TWordBreakType { wbClassify = WB_CLASSIFY, wbIsDelimiter = WB_ISDELIMITER,
  wbLeft = WB_LEFT, wbLeftBreak = WB_LEFTBREAK, wbMoveWordLeft = WB_MOVEWORDLEFT,
  wbRight = WB_RIGHT, wbMoveWordRight = WB_MOVEWORDRIGHT,
  wbRightBreak = WB_RIGHTBREAK };

//---------------------------------------------------------------------------
// TTaeRichEdit class
//---------------------------------------------------------------------------
class PACKAGE TTaeRichEdit : public TCustomMemo
{
public:
// static class members
static const AnsiString Untitled;

// TTaeRichEdit is a very friendly class...
friend class TTaeTextAttributes;
friend class TTaeTextAttributes2;
friend class TTaeParaAttributes;
friend class TTaeRichEditStrings;
friend class TTaeRichEditPrint;
friend class TTaePrintDialog;
friend class TTaePreviewForm;
friend class TTaePreviewWindow;
friend class TTaeRichEditAdvPrint;
friend class TTaePosition;

typedef TTaeRichEditAdvPrint TTaePrint;
//typedef TTaeRichEditPrint TTaePrint;

typedef TTaeTextAttributes2 TTextAttrib;
typedef TTaeParaAttributes2 TParaAttrib;
typedef TFont2 TTaeFont;

//------------------------------------
// data members
//
protected:
  TFont2* FFont2;
  bool FOleSupport;
  bool FPrintSupport;
  bool FSelectionBar;
  bool FInsertMode;
  int FUndoLimit;
  int FUndoLimitActual;
  bool FAutoUrlDetect;
  bool FAutoWordSelect;
  TNotifyEvent FOnInsertModeChange;
  TStrings* FTaeRichEditStrings;
  int FLibHandle;
  TIRichEditOle* FRichEditOle;
  TMemoryStream* FMemStream;
  bool FModified;
  bool FHideScrollBars;
  TTextAttrib* FSelAttributes;
  TTextAttrib* FDefAttributes;
  TParaAttrib* FParagraph;
  Classes::TAlignment FOldParaAlignment;
  int FScreenLogPixels;
  bool FHideSelection;
  Classes::TNotifyEvent FOnSelChange;
  Classes::TNotifyEvent FOnUpdateEvent;
//  TTaeRichEditChange FSaveOnChange;
  Classes::TNotifyEvent FSaveOnChange;
  TMetaClass* FDefaultConverter;
  TRichEditResizeEvent FOnResizeRequest;
  TRichEditSaveClipboard FOnSaveClipboard;
  Windows::TRect FPageRect;
  TWordWrapTo FWordWrapTo;
  TTaeTabStops FTabStops;
  int FTabCount;
  int FTabWidth; // with in chars
  bool FTransparent;
  TTaeFont* FPlainTextFont;
  TTaeFont* FRtfTextFont;
  bool FGuessFont;
  AnsiString FFileName;
  bool FEnableRedraw;
  bool FEnableNotifications;
  TTaePrint* FRichEditPrint;
  TTaeRichEditUrlClick FOnUrlClick;
  bool FAutoUrlExecute;
  int FRichEditDllVersion;
  TModuleVersionInfo* FRichEditVersionInfo;
  TModuleVersionInfo FRichEditVersionInfoData;

//  TTaeRichEditChange FOnChange;
//  TTaeRichEditChangeEvent FOnChangeEvent;

  TTaeRichEditProtectChange FOnProtectChange;
  TTaeRichEditProtectEvent FOnProtectEvent;
  TTaeRichEditProtectEvent FOnLinkEvent;
  // S.S. Added 5/2014
  long FOldLineCount, FOldLength;
  int FLockCounter, FView;
  // S.S. Added 5/2015
//  int FColumn;

  TScrollStyle FScrollBars;

  bool FOleAllowTextDrag;
  bool FOleAcceptDrop;

//------------------------------------
// property getters/setters
//
protected:
  void __fastcall SetDefAttributes(TTextAttrib* Value);
  void __fastcall SetSelAttributes(TTextAttrib* Value);

  long __fastcall GetTextLength(void); // S.S. 5/28/15
  void __fastcall SetLines(TStrings* Value);
  HIDESBASE void __fastcall SetHideSelection(bool Value);
  void __fastcall SetHideScrollBars(bool Value);
  void __fastcall SetSelectionBar(bool value);
  void __fastcall SetInsertMode(bool value);
  int __fastcall GetUndoLimit(void);
  void __fastcall SetUndoLimit(int value);
  void __fastcall SetAutoUrlDetect(bool value);
  void __fastcall SetAutoWordSelect(bool value);
  bool __fastcall GetCanRedo(void);
  AnsiString __fastcall GetUndoTypeString(void);
  AnsiString __fastcall GetRedoTypeString(void);
  bool __fastcall GetPlainText(void);
  void __fastcall SetPlainText(bool value);
  void __fastcall SetOleSupport(bool Value);

// S.S. added 2/1/15
  void __fastcall SetPrintSupport(bool Value);
// S.S. added 6/7/15
  WideString __fastcall GetTextW(void);
  WideString __fastcall GetSelTextW(void);
  void __fastcall SetSelTextW(WideString wSelText);
  void __fastcall SetTextW(WideString wText);

  AnsiString __fastcall GetSelTextRtf(void);
  void __fastcall SetSelTextRtf(AnsiString rtfText);

  void __fastcall SetFont2(TFont2* font);
  void __fastcall SetWordWrapTo(TWordWrapTo wwtType);
  void __fastcall SetRichEditStrings(Classes::TStrings* Value);
  virtual TPoint __fastcall GetCaretPos(void);
  virtual void __fastcall SetCaretPos(TPoint Value);
  virtual TPoint __fastcall GetScrollPos(void);
  virtual void __fastcall SetScrollPos(TPoint Value);
  virtual int __fastcall GetSelLength(void);
  virtual void __fastcall SetSelLength(int Value);
  virtual int __fastcall GetSelStart(void);
  virtual void __fastcall SetSelStart(int Value);
  long __fastcall GetLine(void);
  void __fastcall SetLine(long Value);
  long __fastcall GetColumn(void);
  void __fastcall SetColumn(long column);
  void __fastcall SetOnUrlClick(TNotifyEvent value);
  void __fastcall SetEnableRedraw(bool enable);
  void __fastcall SetEnableNotifications(bool enable);

  TTaeHeaderText* __fastcall GetHeader(void);
  void __fastcall SetHeader(TTaeHeaderText* header);
  TTaeHeaderText* __fastcall GetFooter(void);
  void __fastcall SetFooter(TTaeHeaderText* footer);
  TTaeHeaderText* __fastcall GetFirstHeader(void);
  void __fastcall SetFirstHeader(TTaeHeaderText* firstHeader);
  TTaeHeaderText* __fastcall GetFirstFooter(void);
  void __fastcall SetFirstFooter(TTaeHeaderText* firstFooter);
  bool __fastcall GetFirstPageDifferent(void);
  void __fastcall SetFirstPageDifferent(bool different);
  TTaeBorderLines __fastcall GetBorderLines(void);
  void __fastcall SetBorderLines(TTaeBorderLines borderLines);
  int __fastcall GetInsideMargin(void);
  void __fastcall SetInsideMargin(int marginTwips);
  int __fastcall GetBorderWidth(void);
  void __fastcall SetBorderWidth(int borderWidthTwips);
  void __fastcall SetTabWidth(int tabWidth);
  void __fastcall SetTabCount(int tabCount);
  AnsiString __fastcall GetFileTitle(void);

  bool __fastcall GetOleAllowTextDrag(void);
  void __fastcall SetOleAllowTextDrag(bool allow);
  bool __fastcall GetOleAcceptDrop(void);
  void __fastcall SetOleAcceptDrop(bool accept);

//------------------------------------
// protected methods
//
protected:
//  static int CALLBACK AutoCorrectProc(LANGID langid, const WCHAR *pszBefore,
//              WCHAR *pszAfter, LONG cchAfter, LONG *pcchReplaced);
  static DWORD CALLBACK StreamInCallback(DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG FAR *pcb);
  static DWORD CALLBACK StreamOutCallback(DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG FAR *pcb);
  void __fastcall ToggleInsertMode(void);
  virtual void __fastcall CreateParams(Controls::TCreateParams& Params);
  virtual void __fastcall CreateWnd(void);
  virtual void __fastcall DestroyWnd(void);
  AnsiString __fastcall BuildUndoTypeString(AnsiString type, int id);
  bool __fastcall ProtectChange(int StartPos, int EndPos);
//  bool __fastcall EditChange(DWORD dwChangeType, void* pvCookieData);
  bool __fastcall SaveClipboard(int NumObj, int NumChars);
  virtual void __fastcall RequestSize(const Windows::TRect &Rect);
  DYNAMIC void __fastcall SelectionChange(void);
  DYNAMIC void __fastcall UpdateEvent(void);
  virtual void __fastcall DoSetMaxLength(int Value);
  AnsiString __fastcall GetRtf(bool selectionOnly = true);
  virtual long __fastcall GetLineCount(void);
  void __fastcall PutRtf(AnsiString rtfText, bool replaceSelection = true);
  void SetWordWrapToPrinter(void);

  HIDESBASE MESSAGE void __fastcall WMNCDestroy(Messages::TWMNoParams &Message);
  HIDESBASE MESSAGE void __fastcall CMBiDiModeChanged(Messages::TMessage &Message);
  HIDESBASE MESSAGE void __fastcall CMFontChanged(Messages::TMessage &Message);
  HIDESBASE MESSAGE void __fastcall CMColorChanged(Messages::TMessage &Message);
//  HIDESBASE MESSAGE void __fastcall CMTextChanged(TMessage& Message);
//  HIDESBASE MESSAGE void __fastcall WMCommand(Messages::TWMCommand &Message);
  MESSAGE void __fastcall CNNotify(Messages::TWMNotify &Message);
  HIDESBASE MESSAGE void __fastcall WMSetCursor(Messages::TWMSetCursor &Message);
  HIDESBASE MESSAGE void __fastcall WMPaint(Messages::TWMPaint &Message);
  HIDESBASE MESSAGE void __fastcall WMSetFont(Messages::TWMSetFont &Message);
  DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);

  // many days were wasted before I realized that I had to add the following
  // message maps.  perhaps it was obvious, but I incorrectly assumed that
  // the message handlers would be used if declared in the base....
  BEGIN_MESSAGE_MAP
    VCL_MESSAGE_HANDLER(WM_NCDESTROY, TWMNCDestroy, WMNCDestroy)
    VCL_MESSAGE_HANDLER(CM_BIDIMODECHANGED, TMessage, CMBiDiModeChanged)
    VCL_MESSAGE_HANDLER(CM_FONTCHANGED, TMessage, CMFontChanged)
    VCL_MESSAGE_HANDLER(CM_COLORCHANGED, TMessage, CMColorChanged)
//    VCL_MESSAGE_HANDLER(CM_TEXTCHANGED, TMessage, CMTextChanged)
//    VCL_MESSAGE_HANDLER(WM_COMMAND, TWMCommand, WMCommand)
    VCL_MESSAGE_HANDLER(CN_NOTIFY, TWMNotify, CNNotify)
    VCL_MESSAGE_HANDLER(WM_SETCURSOR, TWMSetCursor, WMSetCursor)
    VCL_MESSAGE_HANDLER(WM_PAINT, TWMPaint, WMPaint)
    VCL_MESSAGE_HANDLER(WM_SETFONT, TWMSetFont, WMSetFont)
  END_MESSAGE_MAP(TCustomMemo)

//------------------------------------
// public methods
//
public:
  #pragma option push -w-inl
  __fastcall virtual TTaeRichEdit(Classes::TComponent* AOwner);
  #pragma option pop
  #pragma option push -w-inl
  __fastcall virtual ~TTaeRichEdit(void);
  #pragma option pop
  #pragma option push -w-inl
  __fastcall TTaeRichEdit(HWND ParentWindow) : TCustomMemo(ParentWindow) { };
  #pragma option pop

  static void __fastcall RegisterConversionFormat(System::TMetaClass* vmt,
    const System::AnsiString AExtension, System::TMetaClass* AConversionClass);
  virtual void __fastcall Clear(void);
//  int __fastcall FindText(AnsiString text, int StartPos,
//                    int Length, TSearchTypes2 searchOptions);
  int __fastcall FindTextW(const WideString SearchStr, int StartPos,
                                              int Length, TSearchTypes Options);
  int __fastcall FindText(const String SearchStr, int StartPos,
                                              int Length, TSearchTypes Options);
  bool __fastcall FindTextExW(WideString text, int startPos,
                    TSearchTypes2 searchOptions, int& foundPos, int& length);
  bool __fastcall FindTextEx(String text, int startPos,
                    TSearchTypes2 searchOptions, int& foundPos, int& length);
  virtual void __fastcall Print(const AnsiString Caption);

  // S.S. 07/17/00 GetTextLen() can't be overridden, but we can
  // make a better version of it using function overloading!
//  int __fastcall GetTextLen(DWORD flags);
//  virtual int __fastcall GetTextLen(void);  // note: not virtual in base!
  virtual long __fastcall GetTextBufW(wchar_t* Buffer, long BufSize);
  virtual long __fastcall GetSelTextBufW(wchar_t* Buffer, long BufSize);
  virtual WideString __fastcall GetStringW(long LineIndex);
  virtual void __fastcall SetStringW(WideString wText, long LineIndex);

  virtual TPoint __fastcall GetCharCoordinates(long Index);
  virtual long __fastcall GetLineByIndex(long Index);
  virtual TPoint __fastcall GetPosByIndex(long Index);
  virtual TPoint __fastcall GetSelStartPos(void);
  virtual TPoint __fastcall GetSelEndPos(void);
  virtual long __fastcall GetColumnByIndex(long Index);
  void __fastcall SetScrollBars(TScrollStyle Value);
//  void __fastcall SetAutoCorrectProc(void);
  virtual long __fastcall GetCharIdx(long Line);
  virtual void __fastcall SetSelFirstLast(long first, long last );
  virtual void __fastcall GetSelFirstLast(long &first, long &last );
// S.S. added 10/7/15
  virtual int __fastcall GetTabWidthTwips(void);
  virtual int __fastcall GetTabWidthTwips(int tabWidth, TFont2* font);
  // End S.S. mods

  bool Undo(void);
  bool Redo(void);
  void StopGroupTyping(void);
  void SetOleHostNames(AnsiString hostApp, AnsiString hostDoc);
  bool CloseActiveOleObjects(bool savePrompt);
  bool __fastcall InsertObject(void);
  bool __fastcall PasteSpecial(void);
  void __fastcall PasteSpecialFromClipboard(char* format = CF_RTF) {
    PasteSpecialFromClipboard((unsigned int) format);
    };
  void __fastcall PasteSpecialFromClipboard(unsigned int format);
  bool CanPasteFormat(char* format) { return CanPasteFormat((unsigned int) format); };
  bool CanPasteFormat(unsigned int format);
  static bool IsRtfFile(AnsiString filepath);
  static bool IsLikelyBinaryFile(AnsiString filepath);
  void __fastcall CopyToStream(TStream* stream, bool selectionOnly = true,
    bool plainText = false, bool noObjects = false, bool plainRtf = false);
  void __fastcall PasteFromStream(TStream* stream, bool selectionOnly = true,
    bool plainText = false, bool plainRtf = false);
  void __fastcall CopyRtfTo(TTaeRichEdit& toRichEdit, bool selectionOnly = true,
    bool replaceSelection = true);
  void ScrollToCaret(void);
  int __fastcall GetTabStops(TTaeTabStops& tabStops);
  void __fastcall SetTabStops(bool entireDocument = true); // S.S. (must set FTabWidth first!)
  void __fastcall SetTabStops(int tabCount, TTaeTabStops &tabStops);
  void __fastcall SetTabStops(int tabStopTwips);
  void __fastcall SetTabStops(int tabWidth, TFont2* font, bool entireDocument = true);
  void __fastcall SetTransparent(bool transparent);
  void __fastcall ExecuteUrl(TObject* Sender, WideString urlText);
  int FindWordBreak(TWordBreakType wbType, int startPos);
  int FindWordBreak(TWordBreakType wbType) { return FindWordBreak(wbType, SelStart); };
  void FindWord(TCharRange& chrg, int startPos, bool select = false);
  void FindWord(TCharRange& chrg, bool select = false) { FindWord(chrg, SelStart, select); };
  int SelectWord(int startPos);
  int SelectWord(void) { return SelectWord(SelStart); };

  // use the following to insert a bitmap programmatically
  bool __fastcall InsertBitmap(Graphics::TBitmap* bmp);
  // use the following to insert the contents of a TOleContainer programmatically
  bool __fastcall InsertContainerObject(TOleContainer* obj, CLIPFORMAT fmt = 0);

  // use the following for faster access to the control's text (this
  // should be much faster than the SelText property...)
  WideString __fastcall GetRangeW(long startPos, long endPos);
  wchar_t* __fastcall GetRangeBufW(long startPos, long endPos);

//------------------------------------
// public properties
//
public:
  __property TMetaClass* DefaultConverter = {read=FDefaultConverter, write=FDefaultConverter};
  __property Windows::TRect PageRect = {read=FPageRect, write=FPageRect};
  __property TTextAttrib* DefAttributes = {read=FDefAttributes, write=SetDefAttributes};
  __property TTextAttrib* SelAttributes = {read=FSelAttributes, write=SetSelAttributes};
  __property TParaAttrib* Paragraph = {read=FParagraph};
// S.S. Added 8/2013
  __property TPoint ScrollPos = { read = GetScrollPos, write = SetScrollPos, nodefault };
// S.S. Added 4/19/2014
  __property TPoint CaretPos = { read = GetCaretPos, write = SetCaretPos, nodefault };
  __property long Line = { read = GetLine, write = SetLine, nodefault };
//  __property unsigned int Column = { read = FColumn, write = SetColumn, nodefault };
  __property long Column = { read = GetColumn, write = SetColumn, nodefault };
  __property AnsiString FileName = { read = FFileName, write = FFileName, nodefault };
  __property AnsiString FileTitle = { read = GetFileTitle, nodefault };
  __property TIRichEditOle* OleInterface = { read = FRichEditOle, nodefault };  // volatile!
  __property bool EnableRedraw = { read = FEnableRedraw, write = SetEnableRedraw, default = true, stored = false };
  __property bool EnableNotifications = { read = FEnableNotifications, write = SetEnableNotifications, default = true, stored = false };

// S.S. I use these to comput the change in text-length and line-count
// in the YahCoLoRiZe OnChange handler
  __property long OldLineCount = { read = FOldLineCount, write = FOldLineCount, nodefault };
  __property long OldLength = { read = FOldLength, write = FOldLength, nodefault };
  __property int LockCounter = { read = FLockCounter, write = FLockCounter, nodefault };
  __property int View = { read = FView, write = FView, nodefault };
  __property long TextLength = { read = GetTextLength }; // S.S. 5/28/15

  // note: the SelText property is actually implemented in TCustomEdit and
  // gets reflected back down into the VCL TRichEdit class with a few
  // indirections through TRichEditStrings.  the following functions are added
  // to add RTF string support directly in this class.  (the alternative was
  // to recode yet another VCL class -- TCusomEdit -- into BCB.  Frankly,
  // I am more than tired of recursing this project back class after class...
  // there are six more classes to go before this becomes a total rewrite
  // of everything from TPersistent down....)
//  __property AnsiString SelText = { read = GetSelText, write = SetSelText, nodefault };
  __property int SelLength = { read = GetSelLength, write = SetSelLength, nodefault };
  __property int SelStart = { read = GetSelStart, write = SetSelStart, nodefault };
  __property AnsiString SelTextRtf = { read = GetSelTextRtf, write = SetSelTextRtf };
  __property WideString SelTextW = { read = GetSelTextW, write = SetSelTextW };
  __property WideString TextW = { read = GetTextW, write = SetTextW };

  __property TTaePrint* TaePrint = { read = FRichEditPrint, default = 0, stored = false };
  __property int RichEditDllVersion = { read = FRichEditDllVersion, stored = false };
  __property TModuleVersionInfo* RichEditVersionInfo = { read = FRichEditVersionInfo,  stored = false };

//  __property TTaeRichEditChange SaveOnChange = { read = FSaveOnChange, write = FSaveOnChange, default = 0 };
  __property Classes::TNotifyEvent SaveOnChange = { read = FSaveOnChange, write = FSaveOnChange, default = 0 };
//------------------------------------
// published properties
//
__published:
  __property TStrings* Lines  = { read = FTaeRichEditStrings, write = SetLines, nodefault };
  __property bool HideSelection = {read=FHideSelection, write=SetHideSelection, default = true};
  __property bool HideScrollBars = {read=FHideScrollBars, write=SetHideScrollBars, default = true};
  __property bool SelectionBar  = { read = FSelectionBar, write = SetSelectionBar, default = false };
  __property bool InsertMode  = { read = FInsertMode, write = SetInsertMode, default = true };
  __property int UndoLimit  = { read = GetUndoLimit, write = SetUndoLimit, default = 100 };
  __property bool AutoUrlDetect  = { read = FAutoUrlDetect, write = SetAutoUrlDetect, default = false };
  __property bool AutoWordSelect  = { read = FAutoWordSelect, write = SetAutoWordSelect, default = false };
  __property bool CanRedo  = { read = GetCanRedo, stored = false };
  __property AnsiString UndoTypeString  = { read = GetUndoTypeString, stored = false };
  __property AnsiString RedoTypeString  = { read = GetRedoTypeString, stored = false };
  __property bool PlainText  = { read = GetPlainText, write = SetPlainText, default = false };
  __property bool OleSupport = { read = FOleSupport, write = SetOleSupport, default = true };
// S.S. added 2/1/15
  __property bool PrintSupport = { read = FPrintSupport, write = SetPrintSupport, nodefault };
  __property TFont2* Font = { read = FFont2, write = SetFont2, nodefault };
  __property TWordWrapTo WordWrapMode = { read = FWordWrapTo, write = SetWordWrapTo, default = wwtNone };
  __property TRichEditSaveClipboard OnSaveClipboard = { read = FOnSaveClipboard, write = FOnSaveClipboard, default = 0 };
  __property Classes::TNotifyEvent OnSelectionChange = { read = FOnSelChange, write = FOnSelChange, default = 0 };
  __property Classes::TNotifyEvent OnUpdateEvent = { read = FOnUpdateEvent, write = FOnUpdateEvent, default = 0 };

//  __property TTaeRichEditChange OnChange = { read = FOnChange, write = FOnChange, default = 0 };
//  __property TTaeRichEditChangeEvent OnChangeEvent = { read = FOnChangeEvent, write = FOnChangeEvent, default = 0 };

  __property TTaeRichEditProtectChange OnProtectChange = { read = FOnProtectChange, write = FOnProtectChange, default = 0 };
  __property TTaeRichEditProtectEvent OnProtectEvent = { read = FOnProtectEvent, write = FOnProtectEvent, default = 0 };

  __property TRichEditResizeEvent OnResizeRequest = { read = FOnResizeRequest, write = FOnResizeRequest, default = 0 };
  __property TNotifyEvent OnInsertModeChange  = { read = FOnInsertModeChange, write = FOnInsertModeChange, default = 0 };
  __property bool Transparent = { read = FTransparent, write = SetTransparent, default = false };
  __property TTaeFont* DefaultTextFileFont = { read = FPlainTextFont, write = FPlainTextFont, nodefault };
  __property TTaeFont* DefaultRtfFileFont = { read = FRtfTextFont, write = FRtfTextFont, nodefault };
  __property bool UseDefaultFileFont = { read = FGuessFont, write = FGuessFont, default = true };
  __property TTaeRichEditUrlClick OnUrlClick = { read = FOnUrlClick, write = FOnUrlClick,  default = 0 };
  __property bool AutoUrlExecute = { read = FAutoUrlExecute, write = FAutoUrlExecute, default = true };
  __property TTaeRichEditProtectEvent OnLinkEvent = { read = FOnLinkEvent, write = FOnLinkEvent, default = 0 };
  __property TTaeHeaderText* PageHeader = { read = GetHeader, write = SetHeader, nodefault };
  __property TTaeHeaderText* PageFooter = { read = GetFooter, write = SetFooter, nodefault };
  __property TTaeHeaderText* PageFirstHeader = { read = GetFirstHeader, write = SetFirstHeader, nodefault };
  __property TTaeHeaderText* PageFirstFooter = { read = GetFirstFooter, write = SetFirstFooter, nodefault };
  __property bool PageFirstDifferent = { read = GetFirstPageDifferent, write = SetFirstPageDifferent, nodefault };
  __property TTaeBorderLines PageBorderLines = { read = GetBorderLines, write = SetBorderLines, nodefault };
  __property int PageInsideMargin = { read = GetInsideMargin, write = SetInsideMargin, nodefault };
  __property int PageBorderWidth = { read = GetBorderWidth, write = SetBorderWidth, nodefault };
  __property int TabWidth = { read = FTabWidth, write = SetTabWidth, nodefault };
  __property int TabCount = { read = FTabCount, write = SetTabCount, nodefault };
  __property long LineCount = { read = GetLineCount };
  __property bool OleAllowTextDrag = { read = GetOleAllowTextDrag, write = SetOleAllowTextDrag, nodefault };
  __property bool OleAcceptDrop = { read = GetOleAcceptDrop, write = SetOleAcceptDrop, nodefault };
//------------------------------------
// publish inherited properties
//
  __property Align;
  __property Alignment;
  __property Anchors;
  __property BiDiMode;
  __property BorderStyle;
  __property BorderWidth;
  __property CanUndo;
  __property Color;
  __property Ctl3D;
  __property DragCursor;
  __property DragKind;
  __property DragMode;
  __property Enabled;
  __property ImeMode;
  __property ImeName;
  __property Constraints;
  __property MaxLength;
  __property ParentBiDiMode;
  __property ParentColor;
  __property ParentCtl3D;
  __property ParentFont;
  __property ParentShowHint;
  __property PopupMenu;
  __property ReadOnly;
  __property ScrollBars;
  __property ShowHint;
  __property TabOrder;
  __property TabStop;
  __property Visible;
  __property WantTabs;
  __property WantReturns;
  __property WordWrap;
  __property OnChange;
  __property OnDragDrop;
  __property OnDragOver;
  __property OnEndDock;
  __property OnEndDrag;
  __property OnEnter;
  __property OnExit;
  __property OnKeyDown;
  __property OnKeyPress;
  __property OnKeyUp;
  __property OnMouseDown;
  __property OnMouseMove;
  __property OnMouseUp;
  __property OnMouseWheel;
  __property OnMouseWheelDown;
  __property OnMouseWheelUp;
  __property OnStartDock;
  __property OnStartDrag;
};
#pragma pack(pop)
#endif
//---------------------------------------------------------------------------
