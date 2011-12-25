/*******************************************************************************
*                         Goggles Music Manager                                *
********************************************************************************
*           Copyright (C) 2006-2011 by Sander Jansen. All Rights Reserved      *
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

/// Album number consists of a disc number + track number
#define GMALBUMNO(disc,track) ((((FX::FXuint)(track))&0xffff) | (((FX::FXuint)(disc))<<16))

/// Get the disc number from a album no.
#define GMDISCNO(s) ((FX::FXushort)(((s)>>16)&0xffff))

/// Get the track number from a album no.
#define GMTRACKNO(s) ((FX::FXushort)((s)&0xffff))

class GMTrack{
public:
  FXString      title;
  FXString      artist;
  FXString      album;
  FXString      album_artist;
  FXString      composer;
  FXString      conductor;
  FXStringList  tags;
  FXint         year;
  FXint 	    no;
  FXuint        rating;
public:
  GMTrack();

  /// Clear the track
  void clear();

  /// Adopt from track
  void adopt(GMTrack &);

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
  FXuchar*  data;
  FXuval    len;
  FXString  description;
  FXuint    type;
public:
  GMCover();
  GMCover(const void * data, FXuval len,FXuint t=GMCover::Other,const FXString & label=FXString::null,FXbool owned=false);
  ~GMCover();

  /// Return file extension for image.
  FXString fileExtension() const;

  FXbool save(const FXString & path);
public:
  static FXint fromTag(const FXString & mrl,GMCoverList & list);

  static GMCover * fromTag(const FXString & mrl);

  static GMCover * fromPath(const FXString & mrl);

  static GMCover * fromFile(const FXString & file);
  };

#endif
