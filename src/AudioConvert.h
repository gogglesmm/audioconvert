/*******************************************************************************
*                             Audio Converter                                  *
********************************************************************************
*           Copyright (C) 2010-2010 by Sander Jansen. All Rights Reserved      *
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
#ifndef AUDIO_CONVERT_H
#define AUDIO_CONVERT_H

class AudioTools;

#if FOXVERSION > FXVERSION(1,7,22)
class AudioConverter : public FXDirVisitor {
#else
class AudioConverter {
#endif
protected: /// Settings
  AudioTools tools;
  FXbool dryrun;        // Simulated run.
  FXbool overwrite;     // Overwrite existing file
  FXuint mode[FILE_NTYPES];
protected: // State
  FXString src_path;  // Source Path
  FXString dst_path;  // Destination Path
  FXString cur_path;  // Current Path
  FXString tmp_file;  // Temp File
  FXString cvr_file;  // cover file name
  FXTime   out_time;  // Timestamp of out file
  FXTime   start_time;  // Timestamp when we started
  FXTime   cvr_time;  // Timestamp for cover file
  FXuint   status;
protected:
  FXuint copy(const FXString & path);
  FXuint convert(FXuint from,FXuint to,const FXString & path);
  FXuint convert_recode(FXuint from,FXuint to,const FXString & path);
  FXuint convert_direct(FXuint from,FXuint to,const FXString & path);
  FXuint convert_indirect(FXuint from,FXuint to,const FXString & path);
protected:
  FXbool make_path(const FXString & path) const;
  FXbool check_destination(const FXString & src,const FXString & dst);
  FXbool update_destination(const FXString & src,const FXString & dst);

  FXbool copy_tags(const FXString & src,const FXString & dst) const;
  FXbool copy_files(const FXString & src,const FXString & dst) const;
  FXbool copy_folder_cover(FXuint from,FXuint to,const FXString & in);


  void   cleanup(const FXString &);

  void   parse_config();
  FXuint parse_mode(const FXchar*);
public:
  FXuint enter(const FXString& path);
  FXuint visit(const FXString& path);  
  FXuint leave(const FXString& path);
#if FOXVERSION < FXVERSION(1,7,22)  
  FXuint traverse(const FXString& path);
#endif
public:
  AudioConverter();

  FXint run();

  FXbool parse(int argc,FXchar * argv[]);

  virtual ~AudioConverter();
  };

#endif
