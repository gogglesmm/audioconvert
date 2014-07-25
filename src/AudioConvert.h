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


class Task {
friend class TaskManager;
protected:
  pid_t pid;
protected:
  virtual FXbool execute(const FXString & cmd);
public:
  Task();

  virtual FXbool run();

  virtual FXbool done(FXbool /*success*/) { return true; }

  virtual ~Task();
  };

class TaskManager {
protected:
  FXPtrListOf<Task> tasklist;
  FXint             maxtask;
protected:
  void updateTask(pid_t pid,FXint status);
  void removeTask(FXint i);
public:
  TaskManager(FXint max=1);

  FXint getMaxTasks() const { return maxtask; }

  void setMaxTasks(FXint m) { maxtask=m; }

  void appendTask(Task*,FXbool dryrun=false);

  void wait(FXbool block=false);

  void waitall();

  ~TaskManager();
  };


class AudioConverter : public FXDirVisitor {
protected:
  TaskManager manager;
  AudioTools  tools;
protected:
  FXString     source;            // source directory
  FXString     target;            // target directory
  FXuint       mode[FILE_NTYPES]; // the conversion mode for the given file type. initialy set to none
  FXbool       dryrun;            // dry run mode
  FXbool       overwrite;         // overwrite existing files
  FXbool       nodirect;          // don't use direct converter
  FXbool       reformat;          // apply formatting
  FXbool       extractcover;      // extract cover from source
  FXuint       status;            // traverser status
  FXString     format_template;   // File format template
  FXString     format_strip;      // Characters to strip
  FXuint       format_options;    // Additional Format Options
  FXTextCodec* format_codec;      // Format encoding
protected:
  FXuint enter(const FXString & path);
  FXuint leave(const FXString & path);
  FXuint visit(const FXString & path);
protected:
  FXuint getMode(const FXchar*);
  void initConfig();
  FXbool canDirectConvert(const FXString&path,FXuint filetype);
public:
  AudioConverter();

  // Init the audioconverter and parse command line arguments
  FXbool init(FXint argc,FXchar *argv[]);

  // Get the source directory
  const FXString & getSource() const { return source; }

  // Get the target directory
  const FXString & getTarget() const { return target; }

  // Return whether to force overwriting existing files
  FXbool getOverWrite() const { return overwrite; }

  // Return whether we are dry running or not
  FXbool getDryRun() const { return dryrun; }

  // Return whether to apply formatting rules
  FXbool getFormat() const { return reformat; }

  // Return whether to extract cover from source and place in target path
  FXbool getExtractCover() const { return extractcover; }

  // Return complete command line to encode source into target
  FXString getEncoder(FXuint to,const FXString & source,const FXString & target) const { return tools.encode(to,source,target); }

  // Return complete command line to decode source into target
  FXString getDecoder(FXuint from,const FXString & source,const FXString & target) const { return tools.decode(from,source,target); }

  // Return the default file extension for the given file type
  FXString getExtension(FXuint to) const { return tools.extension(to); }

  // Return the filetype for the given path
  FXuint getFileType(const FXString & path) const;

  // Get Format Template
  FXString getFormatTemplate() const  { return format_template; }

  // Get Characters to strip
  FXString getFormatStrip() const { return format_strip; }

  // Get Format Options
  FXuint getFormatOptions() const { return format_options; }

  // Get Format Codec
  FXTextCodec* getFormatCodec() const { return format_codec; }

  // Run the audio converter
  FXint run();

  // Stop the audio converter
  void stop();
  };




#if 0



#if FOXVERSION > FXVERSION(1,7,22)
class AudioConverter : public FXDirVisitor {
#else
class AudioConverter {
#endif
protected: /// Settings
  AudioTools tools;
  FXbool dryrun;                // Simulated run.
  FXbool overwrite;             // Overwrite existing file
  FXbool rename;                // Rename output file
  FXString format_template;     // File format template
  FXString format_strip;        // Characters to strip
  FXuint   format_options;      // Additional Format Options
  FXTextCodec* format_codec;    // File format encoding
  FXuint mode[FILE_NTYPES];
protected: // State
  GMTrack  src_tag;     // Source Tag
  FXString src_root;    // Source Root Path
  FXString dst_root;    // Destination Root Path
  FXString out_path;    // Output Path (set by format_destination)
  FXString tmp_file;    // Temp File
  FXTime   out_time;    // Timestamp of out file
  FXTime   start_time;  // Timestamp when we started
  FXTime   cvr_time;    // Timestamp for cover file
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
  FXbool format_destination(const FXString & in,FXString & out,FXuint to);

  FXbool copy_tags(const FXString & src,const FXString & dst);
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
#endif
