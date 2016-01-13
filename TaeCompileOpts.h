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
// TaeCompileOpts.h - project compile options.
//---------------------------------------------------------------------------
#ifndef TaeCompileOptsH
#define TaeCompileOptsH
//---------------------------------------------------------------------------

// use the following #define to work around Windows95B SHGetPathFromIDList()
// missing export.  the problem is characterized by clean compiles that,
// when installing in the IDE, generate "a device connected to the system
// is not functioning" or other strange errors.  the workaround causes
// GetLongFileName() function to simply return whatever path is passed to
// it rather than properly converting the value to a long filename.  perhaps
// later I will have time to write a functional workaround.  in the meantime,
// set this value to true if you have problems installing the component on
// Win95 systems.
#define WORK_AROUND_SHGETPATHFROMIDLIST_BUG false

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
