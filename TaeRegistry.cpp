//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaeRegistry.cpp - implementation file for routines for storing and
// loading registry information.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TaeRegistry.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// load TFont information from the registry
//
void LoadFromRegistry(TRegistry& reg, AnsiString name, TFont& font)
{
  font.Name = RegValueExists(reg, name + "Name").ReadString(name + "Name");
  font.Height = RegValueExists(reg, name + "Height").ReadInteger(name + "Height");

  TFontStyles fs;  // need because working directly with font.Style doesn't work...
  if (RegValueExists(reg, name + "Bold").ReadBool(name + "Bold"))
    fs << fsBold;
  if (RegValueExists(reg, name + "Italic").ReadBool(name + "Italic"))
    fs << fsItalic;
  if (RegValueExists(reg, name + "Underline").ReadBool(name + "Underline"))
    fs << fsUnderline;
  if (RegValueExists(reg, name + "StrikeOut").ReadBool(name + "StrikeOut"))
    fs << fsStrikeOut;

  font.Style = fs;
  font.Pitch = (TFontPitch) RegValueExists(reg, name + "Pitch").ReadInteger(name + "Pitch");
  font.Color = (TColor) RegValueExists(reg, name + "Color").ReadInteger(name + "Color");
}
//---------------------------------------------------------------------------
// save TFont information to the registry
//
void SaveToRegistry(TRegistry& reg, AnsiString name, TFont& font)
{
  reg.WriteString(name + "Name", font.Name);
  reg.WriteInteger(name + "Height", font.Height);

  // note font.Style.contains(fsXXX), does not, for some reason,
  // write anything but zeros -- added "? true : false" to overcome
  reg.WriteBool(name + "Bold", font.Style.Contains(fsBold) ? true : false);
  reg.WriteBool(name + "Italic", font.Style.Contains(fsItalic) ? true : false);
  reg.WriteBool(name + "Underline", font.Style.Contains(fsUnderline) ? true : false);
  reg.WriteBool(name + "StrikeOut", font.Style.Contains(fsStrikeOut) ? true : false);

  reg.WriteInteger(name + "Pitch", (int) font.Pitch);
  reg.WriteInteger(name + "Color", (int) font.Color);
}
//---------------------------------------------------------------------------
// load TPrinter information from the registry
//
void LoadFromRegistry(TRegistry& reg, AnsiString name, TPrinter& printer)
{
  HGLOBAL hDevMode = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(DEVMODE));
  void* pDevMode = ::GlobalLock(hDevMode);

  RegValueExists(reg, name + "Device");
  AnsiString device = reg.ReadString(name + "Device");
  RegValueExists(reg, name + "Driver");
  AnsiString driver = reg.ReadString(name + "Driver");
  RegValueExists(reg, name + "Port");
  AnsiString port = reg.ReadString(name + "Port");
  RegValueExists(reg, name + "Mode");
  reg.ReadBinaryData(name + "Mode", pDevMode, sizeof(DEVMODE));
  ::GlobalUnlock(hDevMode);
  printer.SetPrinter(device.c_str(), driver.c_str(), port.c_str(), (int) hDevMode);
}
//---------------------------------------------------------------------------
// save TPrinter information to the registry
//
void SaveToRegistry(TRegistry& reg, AnsiString name, TPrinter& printer)
{
  char device[255];
  char driver[255];
  char port[255];
  unsigned int hDevMode;

  // get printer information
  printer.GetPrinter(device, driver, port, hDevMode);
  reg.WriteString(name + "Device", device);
  reg.WriteString(name + "Driver", driver);
  reg.WriteString(name + "Port", port);
  reg.WriteBinaryData(name + "Mode", ::GlobalLock((HGLOBAL) hDevMode),
    sizeof(DEVMODE));
  ::GlobalUnlock((HGLOBAL) hDevMode);
}
//---------------------------------------------------------------------------
// load a list of strings from the registry
//
void LoadFromRegistry(TRegistry& reg, AnsiString name, TStringList& list)
{
  // clear any existing strings
  list.Clear();

  // get count
  int count = RegValueExists(reg, name + "Count").ReadInteger(name + "Count");

  // load strings in reverse order
  AnsiString s;
  for (int i = 0; i < count; i++) {
    s = name + "Item" + AnsiString(i);
    list.Add(RegValueExists(reg, s).ReadString(s));
    }
}
//---------------------------------------------------------------------------
// save a list of strings to the registry
//
void SaveToRegistry(TRegistry& reg, AnsiString name, TStringList& list)
{
  // save existing count
  int oldCnt = 0;
  if (reg.ValueExists(name + "Count")) oldCnt = reg.ReadInteger(name + "Count");

  // write new count and strings
  reg.WriteInteger(name + "Count", list.Count);
  for (int i = 0; i < list.Count; i++)
    reg.WriteString(name + "Item" + AnsiString(i), list.Strings[i]);

  // delete any entries from prior, longer list
  for (int i = list.Count; i < oldCnt; i++)
    reg.DeleteValue(name + "Item" + AnsiString(i));
}
//---------------------------------------------------------------------------
