/*******************************************************************************
*                         Goggles Music Manager                                *
********************************************************************************
*           Copyright (C) 2006-2010 by Sander Jansen. All Rights Reserved      *
*                               ---                                            *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU General Public License as published by         *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
* GNU General Public License for more details.                                 *
*                                                                              *
* You should have received a copy of the GNU General Public License            *
* along with this program.  If not, see http://www.gnu.org/licenses.           *
********************************************************************************/
#ifndef GMFILENAME_H
#define GMFILENAME_H

namespace GMFilename {

  enum {
    NOSPACES  			= 0x00000001,
    LOWERCASE 			= 0x00000002,
    LOWERCASE_EXTENSION	= 0x00000004
    };

  /// return FXTextCodec from given codec string
  FXbool parsecodec(const FXString & codec,FXTextCodec *& textcodec);

  FXString format(const GMTrack & track,const FXString & path,const FXString & forbidden,const FXuint & options,FXTextCodec * textcodec);

  enum {
    REPLACE_UNDERSCORE = 0x1,
    OVERWRITE = 0x2
    };

  /// Extract info from filename based on mask and store in GMTrack.
  void parse(GMTrack & track,const FXString & mask,FXuint opts);
  }

#endif

