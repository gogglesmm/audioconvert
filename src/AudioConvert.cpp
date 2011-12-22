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
#include "fox.h"
#include "AudioTools.h"
#include "AudioTags.h"
#include "AudioConvert.h"
#include "AudioFilename.h"

/*

  cover conversion:

    embedded -> embedded
    embedded -> folder    (extract front cover)
    folder   -> embedded


*/


FXbool gm_make_path(const FXString & path,FXuint perm=FXIO::OwnerFull|FXIO::GroupFull|FXIO::OtherFull) {
#if FOXVERSION < FXVERSION(1,7,0)
  if(!path.empty()){
    if(FXStat::isDirectory(path)) return true;
    if(gm_make_path(FXPath::upLevel(path),perm)){
      if(FXDir::create(path,perm)) return true;
      }
    }
  return false;
#else
  return FXDir::createDirectories(path,perm);
#endif
  }









enum {
  STATUS_ERROR=0,
  STATUS_OK=1
  };


AudioConverter::AudioConverter() : dryrun(false),overwrite(false),rename(false),format_codec(NULL),out_time(0) {
  mode[FILE_FLAC]=FILE_NONE;
  mode[FILE_MP3]=FILE_NONE;
  mode[FILE_OGG]=FILE_NONE;
  mode[FILE_MP4]=FILE_NONE;
  mode[FILE_MPC]=FILE_NONE;
  tmp_file=FXSystem::getTempDirectory() + PATHSEPSTRING + "audioconvert.wav";
  cvr_file="cover.jpg";
  format_strip="\'\\#~!\"$&();<>|`^*?[]/.:";
  }

AudioConverter::~AudioConverter(){
  delete format_codec;
  }


FXuint AudioConverter::parse_mode(const FXchar * cstr){
  FXString m = FXString(cstr).after('=');
  if (comparecase(m,"ogg")==0)
    return FILE_OGG;
  else if (comparecase(m,"mp3")==0)
    return FILE_MP3;
  else if (comparecase(m,"flac")==0)
    return FILE_FLAC;
  else if (comparecase(m,"mp4")==0)
    return FILE_MP4;
  else if (comparecase(m,"mpc")==0)
    return FILE_MPC;
  else if (comparecase(m,"copy")==0)
    return FILE_COPY;
  else if (comparecase(m,"none")==0 || comparecase(m,"off")==0)
    return FILE_NONE;
  else{
    fxmessage("Error: Invalid argument \"%s\"\n",cstr);
    return FILE_INVALID;
    }
  }

FXbool AudioConverter::parse(int argc,FXchar * argv[]) {
  FXint nargs=0;
  for (FXint i=1;i<argc;i++) {
    if (compare(argv[i],"--dry-run")==0 || compare(argv[i],"-n")==0){
      dryrun=true;
      tools.dryrun=true;
      }
    else if (compare(argv[i],"--ogg=",6)==0){
      if ((mode[FILE_OGG]=parse_mode(argv[i]))==FILE_INVALID)
        return false;
      }
    else if (compare(argv[i],"--mp3=",6)==0){
      if ((mode[FILE_MP3]=parse_mode(argv[i]))==FILE_INVALID)
        return false;
      }
    else if (compare(argv[i],"--flac=",7)==0){
      if ((mode[FILE_FLAC]=parse_mode(argv[i]))==FILE_INVALID)
        return false;
      }
    else if (compare(argv[i],"--mp4=",6)==0){
      if ((mode[FILE_MP4]=parse_mode(argv[i]))==FILE_INVALID)
        return false;
      }
    else if (compare(argv[i],"--mpc=",6)==0){
      if ((mode[FILE_MPC]=parse_mode(argv[i]))==FILE_INVALID)
        return false;
      }
    else if (compare(argv[i],"--all=",6)==0){
      FXuint m=parse_mode(argv[i]);
      if (m==FILE_INVALID)
        return false;

      for (FXint f=0;f<FILE_NTYPES;f++)
        mode[f]=m;
      }
    else if (compare(argv[i],"--overwrite")==0)
      overwrite=true;
    else if (compare(argv[i],"--quiet")==0 || compare(argv[i],"-q")==0)
      tools.quiet();
    else if (compare(argv[i],"--rename")==0) 
      rename=true;        
    else if (compare(argv[i],"--format=",9)==0) {
      format = FXString(argv[i]).after('=');
      }
    else if (compare(argv[i],"--format-strip=",15)==0) {
      format_strip = FXString(argv[i]).after('=');
      }
    else if (compare(argv[i],"--format-encoding=",18)==0) {
      if (!GMFilename::parsecodec(FXString(argv[i]).after('='),format_codec)) {
        fxmessage("Error: Invalid format encoding %s",argv[i]);
        return false;
        }
      }
    else {
      nargs++;

      if (nargs>2) {
        fxmessage("Error: Unexpected argument %s\n",argv[i]);
        return false;
        }

      FXString path = argv[i];
      if (!FXPath::isAbsolute(path)) {
        path=FXPath::absolute(FXSystem::getCurrentDirectory(),path);
        }

      if (nargs==1)
        src_path.adopt(path);
      else
        dst_path.adopt(path);
      }
    }

  /// Time to read the configuration file.
  parse_config();

  if (src_path.empty()) {
    fxmessage("Error: Missing argument source directory.\n");
    return false;
    }

  if (!FXStat::exists(src_path)){
    fxmessage("Error: Source directory doesn't exist.\n");
    return false;
    }

  if (!FXStat::isDirectory(src_path)){
    fxmessage("Error: source is not a directory\n");
    return false;
    }

  if (dst_path.empty()) {
    dst_path=src_path;
    }

  if (dst_path==src_path ){
    fxmessage("Destination same as source! Continue? ");
    int answer = getc(stdin);
    if (!(answer=='y' || answer=='Y')) {
      fxmessage("\n");
      return false;
      }
    fxmessage("\n");
    }

  if (FXStat::exists(dst_path) && !FXStat::isDirectory(dst_path)){
    fxmessage("Error: destination is not a directory\n");
    return false;
    }


 /// Make sure we're actually doing something
  FXbool nothing=true;
  for (FXint f=0;f<FILE_NTYPES;f++){
    if (mode[f]!=FILE_NONE) {
      nothing=false;
      break;
      }
    }
  if(nothing) {
    fxmessage("Error: No output conversion specified\n");
    return false;
    }

  return true;
  }


void AudioConverter::parse_config() {
  const FXchar * config_file = "audioconvert/config.rc";

  FXString xdg_config_dirs = FXSystem::getEnvironment("XDG_CONFIG_DIRS");
  if (xdg_config_dirs.empty()) xdg_config_dirs="/etc/xdg";

  FXString xdg_config_home = FXSystem::getEnvironment("XDG_CONFIG_HOME");
  if (xdg_config_home.empty()) xdg_config_home="$HOME/.config";

  FXString config = FXPath::search(xdg_config_home,config_file);
  if (config.empty()) config = FXPath::search(xdg_config_dirs,config_file);

  if (!config.empty()){
    fxmessage("Found config: %s\n",config.text());
    tools.load_rc(config);
    }
  else {
    config = FXPath::absolute(FXPath::expand(xdg_config_home),config_file);
    if (!gm_make_path(FXPath::directory(config))) return;
    fxmessage("Creating config: %s\n",config.text());
    tools.init_rc(config);
    }
  }


#if FOXVERSION < FXVERSION(1,7,22)
FXuint AudioConverter::traverse(const FXString & path) {
  FXString * dirs=NULL;
  FXString * files=NULL;
  FXuint code;
  FXint no;

  code=enter(path);
  if (code!=STATUS_OK) return code;

  no = FXDir::listFiles(dirs,path,"*",FXDir::NoFiles|FXDir::NoParent);
  if (no) {
    for (FXint i=0;i<no;i++) {
      code = traverse(path+PATHSEPSTRING+dirs[i]);
      if (code!=STATUS_OK) {
        delete [] dirs;
        return code;
        }
      }
    delete [] dirs;
    }

  no = FXDir::listFiles(files,path,"*",FXDir::NoDirs);
  if (no) {
    for (FXint i=0;i<no;i++) {
      code=visit(path+PATHSEPSTRING+files[i]);
      if (code!=STATUS_OK) {
        delete [] files;
        return code;
        }
      }
    delete [] files;
    }

  return leave(path);
  }

#endif




FXint AudioConverter::run() {
  FXASSERT(!src_path.empty());
  FXASSERT(!dst_path.empty());
  status=STATUS_OK;
  start_time=FXThread::time();
  if (traverse(src_path)==0)
    return -1;
  else
    return 0;
  }

FXuint AudioConverter::enter(const FXString & path) {
  if (path!=src_path)
    cur_path = FXPath::absolute(dst_path,FXPath::relative(src_path,path));
  else
    cur_path = dst_path;

  if (!cvr_file.empty())
    cvr_time = FXStat::modified(cur_path+PATHSEPSTRING+cvr_file);

  return status;
  }

FXuint AudioConverter::leave(const FXString &) {
  return status;
  }

FXbool AudioConverter::make_path(const FXString & path) const{
  if (dryrun)
    fxmessage("makepath: %s\n",path.text());
  else
    return gm_make_path(path);

  return true;
  }

void AudioConverter::cleanup(const FXString & out) {
  if (FXStat::modified(out)>out_time) {
    fxmessage("remove %s\n",out.text());
    FXFile::remove(out);
    }
  out_time=0;
  }

FXbool AudioConverter::update_destination(const FXString & in,const FXString & out){
  if (in!=out && FXStat::exists(out)) {
    out_time = FXStat::modified(out);
    if (overwrite || FXStat::modified(in)>out_time)
      return true;
    else {
      fxmessage("Existing: %s\n",out.text());
      return false;
      }
    }
  else {
    out_time=0;
    }
  return true;
  }

FXbool AudioConverter::check_destination(const FXString & in,const FXString & out){
  if (in==out) {
    fxmessage("Error: source and destination are the same %s\n",out.text());
    return false;
    }
  return true;
  }


FXbool AudioConverter::format_destination(const FXString & in,FXString & out,FXuint to) {
  if (rename) {

    if (!src_tag.loadTag(in)) {
      fxmessage("Error: failed to load tag from file %s\n",in.text());  
      return false;
      }

    FXString path = FXPath::expand(format);

    // Make sure its absolute
    if (!FXPath::isAbsolute(path))
        path = FXPath::absolute(cur_path,path);

    // Simplify
    path = FXPath::simplify(path);

    // Format name based on tag
    out  = GMFilename::format(src_tag,path,format_strip,format_options,format_codec);

    // Append file extension
    if (to==FILE_COPY)
      out += "." + FXPath::extension(in);
    else
      out += tools.extension(to);  

    }
  else if (to==FILE_COPY) {
    out = cur_path + PATHSEPSTRING + FXPath::name(in);
    }
  else {
    out = cur_path + PATHSEPSTRING + FXPath::title(in) + tools.extension(to);
    }
 return true;
 }


FXuint AudioConverter::convert_recode(FXuint,FXuint to,const FXString & in) {
  FXString out;

  // Get the destination filename
  if (!format_destination(in,out,to)) return STATUS_ERROR;

  /// Check if destination exists and needs updating
  if (!update_destination(in,out)) return STATUS_OK;

  fxmessage("convert (recode):\n");
  fxmessage("  from: %s\n",in.text());
  fxmessage("    to: %s\n",out.text());

  /// Make sure destination exists
  if (!make_path(cur_path)) return STATUS_ERROR;

  /// convert
  if (!tools.encode(to,in,out)) return STATUS_ERROR;

  return STATUS_OK;
  }


FXuint AudioConverter::convert_direct(FXuint from,FXuint to,const FXString & in) {
  FXString out;

  // Get the destination filename
  if (!format_destination(in,out,to)) return STATUS_ERROR;

  /// Make sure in and out are not the same
  if (!check_destination(in,out)) return STATUS_ERROR;

  /// Experimental
  // if (from==FILE_FLAC && to==FILE_OGG)
  //  copy_folder_cover(from,to,in);

  /// Check if destination exists and needs updating
  if (!update_destination(in,out)) return STATUS_OK;

  fxmessage("convert (direct):\n");
  fxmessage("  from: %s\n",in.text());
  fxmessage("    to: %s\n",out.text());

  /// Make sure destination exists
  if (!make_path(cur_path)) return STATUS_ERROR;

  /// convert
  if (!tools.encode(to,in,out)) return STATUS_ERROR;

  /// Tags
  if ((from!=to) && !(from==FILE_FLAC && to==FILE_OGG)) {
    if (!copy_tags(in,out)) return STATUS_ERROR;
    }

  return STATUS_OK;
  }


FXuint AudioConverter::convert_indirect(FXuint from,FXuint to,const FXString & in) {
  FXString out;

  // Get the destination filename
  if (!format_destination(in,out,to)) return STATUS_ERROR;

  /// Make sure in and out are not the same
  if (!check_destination(in,out)) return STATUS_ERROR;

  /// Bail out if out exists
  if (!update_destination(in,out)) return STATUS_OK;

  fxmessage("convert (indirect):\n");
  fxmessage("  from: %s\n",in.text());
  fxmessage("   via: %s\n",tmp_file.text());
  fxmessage("    to: %s\n",out.text());

  /// convert to wav
  if (!tools.decode(from,in,tmp_file)) goto error;

  /// Make sure destination exists
  if (!make_path(cur_path)) goto error;

  /// convert
  if (!tools.encode(to,tmp_file,out)) goto error;

  /// Tags
  if (!copy_tags(in,out)) goto error;

  /// Remove Temp File
  FXFile::remove(tmp_file);
  return STATUS_OK;
error:
  FXFile::remove(tmp_file);
  cleanup(out);
  return STATUS_ERROR;
  }



FXuint AudioConverter::convert(FXuint from,FXuint to,const FXString & in) {
  if (from==to) {
    if (tools.encoder_supports(to,from))
      return convert_recode(from,to,in);
    }
  else if (tools.encoder_supports(to,from))
    return convert_direct(from,to,in);
  else
    return convert_indirect(from,to,in);
  return STATUS_ERROR;
  }


FXuint AudioConverter::copy(const FXString & in) {
  FXString out = cur_path + PATHSEPSTRING + FXPath::name(in);

  /// Bail out if out exists
  if (!update_destination(in,out)) return STATUS_OK;

  fxmessage("copy:\n");
  fxmessage("  from: %s\n",in.text());
  fxmessage("    to: %s\n",out.text());

  /// Make sure destination exists
  if (!make_path(cur_path)) return STATUS_ERROR;

  /// Copy
  if (!copy_files(in,out)) return STATUS_ERROR;

  return STATUS_OK;
  }


FXbool AudioConverter::copy_files(const FXString & in,const FXString & out) const {
  if (dryrun)
    fxmessage("copy '%s' to '%s'\n",in.text(),out.text());
  else
    return FXFile::copyFiles(in,out,true);

  return true;
  }

FXbool AudioConverter::copy_tags(const FXString & in,const FXString & out) {
  GMTrack dst_tag;
  if (dryrun) {
    fxmessage("copy tags\n");
    }
  else {
    // In case of renames, we already loaded the tag
    if (!rename) 
        src_tag.loadTag(in);

    dst_tag.loadTag(out);
    if (src_tag.title!=dst_tag.title){
      src_tag.saveTag(out);
      }
    }
  return true;
  }


FXbool AudioConverter::copy_folder_cover(FXuint from,FXuint to,const FXString & in) {
  if (dryrun) {
    fxmessage("copy folder cover\n");
    return true;
    }
  else {
    FXString out = cur_path + PATHSEPSTRING + cvr_file;
    GMCoverList covers;
    if (!cvr_file.empty() && !FXStat::exists(out) && GMCover::fromTag(in,covers))  {
      fxmessage("Creating %ld %ld, %s\n",cvr_time,start_time,out.text());

      /// Make sure destination exists
      if (!make_path(cur_path)) return false;

      for (FXint i=0;i<covers.no();i++) {
        if (covers[i]->type==GMCover::FrontCover) {
          covers[i]->save(out);
//          cvr_time=FXStat::modified(out);
          return true;
          }
        }
      for (FXint i=0;i<covers.no();i++) {
        if (covers[i]->type==GMCover::Other) {
          covers[i]->save(out);
//          cvr_time=FXStat::modified(out);
          return true;
          }
        }
      }
    }
  return true;
  }


FXuint AudioConverter::visit(const FXString & path) {
  FXString extension = FXPath::extension(path);
  FXuint dst_file,src_file;

  /// Prevent from converting files that are the output of our own conversion
  /// Probably not needed..
  if (FXStat::modified(path)>start_time){
    fxmessage("Source may have been generated by us. Skipping %s\n",path.text());
    return status;
    }

  if (comparecase(extension,"flac")==0 || comparecase(extension,"oga")==0)
    src_file = FILE_FLAC;
  else if (comparecase(extension,"mp3")==0)
    src_file = FILE_MP3;
  else if (comparecase(extension,"ogg")==0)
    src_file = FILE_OGG;
  else if (comparecase(extension,"mp4")==0 || comparecase(extension,"m4a")==0)
    src_file = FILE_MP4;
  else if (comparecase(extension,"mpc")==0)
    src_file = FILE_MPC;
  else
    src_file = FILE_INVALID;

  /// Extension not recognized.
  if (src_file==FILE_INVALID){
    return status;
    }

  dst_file=mode[src_file];
  switch(dst_file){
    case FILE_FLAC:
    case FILE_MP3 :
    case FILE_OGG :
    case FILE_MP4 :
    case FILE_MPC : status=convert(src_file,dst_file,path); break;
    case FILE_COPY: status=copy(path); break;
    case FILE_NONE: break;
    default       : fxmessage("Unexpected destination target: %d\n",dst_file); status=STATUS_ERROR; break;
    }
  return status;
  }

