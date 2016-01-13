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
// TaeUnits.h - inline functions for unit conversions
//---------------------------------------------------------------------------
#ifndef TaeUnitsH
#define TaeUnitsH
//---------------------------------------------------------------------------
// utility functions for converting units between twips, inches, millimeters,
// etc.
//---------------------------------------------------------------------------
inline double TwipsToPoints(double twips);
inline double PointsToTwips(double points);
inline double TwipsToInches(double twips);
inline double InchesToTwips(double inches);
inline double TwipsToMMs(double twips);
inline double MMsToTwips(double mms);
inline double MMsToInches(double mms);
inline double InchesToMMs(double inches);
//---------------------------------------------------------------------------
#define TWIPSPERPOINT 20.0
#define TWIPSPERINCH 1440.0
#define TWIPSPERHUNDREDTHINCH 14.4
#define MMSPERINCH 25.4

inline double TwipsToPoints(double twips)
{
  return twips / TWIPSPERPOINT;
}
inline double PointsToTwips(double points)
{
  return points * TWIPSPERPOINT;
}
inline double TwipsToInches(double twips)
{
  return twips / TWIPSPERINCH;
}
inline double InchesToTwips(double inches)
{
  return inches * TWIPSPERINCH;
}
inline double TwipsToMMs(double twips)
{
  return twips / TWIPSPERINCH * MMSPERINCH;
}
inline double MMsToTwips(double mms)
{
  return mms / MMSPERINCH * TWIPSPERINCH;
}
inline double MMsToInches(double mms)
{
  return mms / MMSPERINCH;
}
inline double InchesToMMs(double inches)
{
  return inches * MMSPERINCH;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
