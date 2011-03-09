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
#ifndef GM_TRACK_H
#define GM_TRACK_H

class GMTrack{
public:
  FXString title;
  FXString artist;
  FXString album;
  FXString album_artist;
  FXString composer;
  FXString conductor;
  FXStringList tags;
  FXint    year;
  FXint 	 no;
  FXdouble album_gain;
  FXdouble album_peak;
  FXdouble track_gain;
  FXdouble track_peak;
public:
  GMTrack();

  /// Get track number
  FXushort getTrackNumber() const { return (FXushort)(no&0xffff); }

  /// Set Track Number
  void setTrackNumber(FXushort track) { no|=((FXuint)track)&0xffff; }

  /// Set Disc Number
  void setDiscNumber(FXushort disc) { no|=((FXuint)disc)<<16; }

  /// Get disc number
  FXushort getDiscNumber() const { return (FXushort)(no>>16); }

  /// Load from tag in given filename. Note that mrl is not set
  FXbool loadTag(const FXString & filename);

  /// Save to tag in given filename. Note that mrl is not set
  FXbool saveTag(const FXString & filename,FXuint opts=0);

  void clear();
  };

class GMCover;
typedef FXArray<GMCover*> GMCoverList;

class GMCover {
public:
  enum {
    Other             =  0,
    FileIcon          =  1,
    OtherFileIcon     =  2,
    FrontCover        =  3,
    BackCover         =  4,
    Leaflet           =  5,
    Media             =  6,
    LeadArtist        =  7,
    Artist            =  8,
    Conductor         =  9,
    Band              = 10,
    Composer          = 11,
    Lyricist          = 12,
    RecordingLocation = 13,
    DuringRecoding    = 14,
    DuringPerformance = 15,
    ScreenCapture     = 16,
    Fish              = 17,
    Illustration      = 18,
    BandLogo          = 19,
    PublisherLogo     = 20
    };
public:
  FXuchar*   data;
  FXival    data_size;
  FXuint    type;
  FXString  description;
  FXString  mime;
public:
  GMCover();
  GMCover(const FXuchar * d,FXival sz,const FXString & mimetype,FXuint t=GMCover::Other,const FXString & label=FXString::null);
  ~GMCover();
public:
  static FXint fromTag(const FXString & mrl,GMCoverList & list);
  
  void save(const FXString & filename);
  };


#endif














