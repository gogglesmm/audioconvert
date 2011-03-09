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


static const FXchar * filetype_extensions[]={
  ".flac",
  ".mp3",
  ".ogg",
  ".m4a",
  ".mpc"
  };


ToolConfig::ToolConfig() : section(NULL),quiet(NULL),types(0) {}

AudioTools::AudioTools() : dryrun(false) {

  tools[FLAC_DECODER].bin       = "flac --decode --force";
  tools[FLAC_DECODER].options   = "";
  tools[FLAC_DECODER].quiet     = "--silent";
  tools[FLAC_DECODER].section   = "flac_decoder";

  tools[FLAC_ENCODER].bin     = "flac --force";
  tools[FLAC_ENCODER].options = "--verify --best";
  tools[FLAC_ENCODER].quiet   = "--silent";
  tools[FLAC_ENCODER].section = "flac_encoder";
  tools[FLAC_ENCODER].types   = FILE_FLAC_BIT;

  tools[MP3_DECODER].bin     = "lame --decode";
  tools[MP3_DECODER].options = "";
  tools[MP3_DECODER].quiet   = "--quiet";
  tools[MP3_DECODER].section = "mp3_decoder";

  tools[MP3_ENCODER].bin     = "lame";
  tools[MP3_ENCODER].options = "--preset extreme";
  tools[MP3_ENCODER].quiet   = "--quiet";
  tools[MP3_ENCODER].section = "mp3_encoder";

  tools[OGG_DECODER].bin     = "oggdec";
  tools[OGG_DECODER].options = "";
  tools[OGG_DECODER].quiet   = "--quiet";
  tools[OGG_DECODER].section = "ogg_decoder";

  tools[OGG_ENCODER].bin     = "oggenc";
  tools[OGG_ENCODER].options = "-q 6";
  tools[OGG_ENCODER].quiet   = "--quiet";
  tools[OGG_ENCODER].section = "ogg_encoder";
  tools[OGG_ENCODER].types   = FILE_FLAC_BIT;

  tools[MP4_DECODER].bin     = "faad";
  tools[MP4_DECODER].options = "";
  tools[MP4_DECODER].quiet   = "--quiet";
  tools[MP4_DECODER].section = "mp4_decoder";

  tools[MP4_ENCODER].bin     = "faac";
  tools[MP4_ENCODER].options = "";
  tools[MP4_ENCODER].quiet   = "--quiet";
  tools[MP4_ENCODER].section = "mp4_encoder";

  tools[MPC_DECODER].bin     = "mpcdec";
  tools[MPC_DECODER].options = "";
  tools[MPC_DECODER].quiet   = NULL;
  tools[MPC_DECODER].section = "mpc_decoder";

  tools[MPC_ENCODER].bin     = "mpcenc --overwrite";
  tools[MPC_ENCODER].options = "--standard";
  tools[MPC_ENCODER].quiet   = "--silent";
  tools[MPC_ENCODER].section = "mpc_encoder";
  tools[MPC_ENCODER].types   = FILE_FLAC_BIT;
  }

static const FXuint decoder_map[]={
  FLAC_DECODER,
  MP3_DECODER,
  OGG_DECODER,
  MP4_DECODER,
  MPC_DECODER
  };

static const FXuint encoder_map[]={
  FLAC_ENCODER,
  MP3_ENCODER,
  OGG_ENCODER,
  MP4_ENCODER,
  MPC_ENCODER
  };

const FXchar *  AudioTools::extension(FXuint type) const {
  return filetype_extensions[type];
  }

FXbool AudioTools::encoder_supports(FXuint type,FXuint desired) const {
  if ((tools[encoder_map[type]].types&(1<<desired))==(1<<desired))
    return true;
  else
    return false;
  }

FXbool AudioTools::run(const FXString & operation) const{
  if (dryrun) {
    fxmessage("%s\n",operation.text());
    }
  else {
    int status = system(operation.text());
    if (status==-1) {
      fxwarning("Error forking\n");
      }
    else if (WIFSIGNALED(status)) {
      fxwarning("\naudioconverter: killed by signal %d\n",WTERMSIG(status));
      }
    else if (WIFSTOPPED(status)) {
      fxwarning("\naudioconverter: stopped by signal %d\n", WSTOPSIG(status));
      }
    else if (WIFCONTINUED(status)) {
      fxwarning("\naudioconverter: continued\n");
      }
    else if (WIFEXITED(status)) {
      if (WEXITSTATUS(status)==0)
        return true;
      }
    return false;
    }
  return true;
  }

FXbool AudioTools::decode(FXuint type,const FXString & input,const FXString & output) const {
  switch(type) {
    case FILE_FLAC  : return run(tools[FLAC_DECODER].bin + " " + tools[FLAC_DECODER].options + " " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_MP3   : return run(tools[ MP3_DECODER].bin + " " + tools[ MP3_DECODER].options + " " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_OGG   : return run(tools[ OGG_DECODER].bin + " " + tools[ OGG_DECODER].options + " " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_MP4   : return run(tools[ MP4_DECODER].bin + " " + tools[ MP4_DECODER].options + " -o " + FXPath::enquote(output) +  " " + FXPath::enquote(input)); break;
    case FILE_MPC   : return run(tools[ MPC_DECODER].bin + " " + tools[ MPC_DECODER].options + " " + FXPath::enquote(input) + " " + FXPath::enquote(output)); break;
    default         : break;
    }
  return false;
  }

FXbool AudioTools::encode(FXuint type,const FXString & input,const FXString & output) const {
  switch(type) {
    case FILE_FLAC  : return run(tools[FLAC_ENCODER].bin + " " + tools[FLAC_ENCODER].options + "  " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_MP3   : return run(tools[ MP3_ENCODER].bin + " " + tools[ MP3_ENCODER].options + " " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_OGG   : return run(tools[ OGG_ENCODER].bin + " " + tools[ OGG_ENCODER].options + " " + FXPath::enquote(input) + " -o " + FXPath::enquote(output)); break;
    case FILE_MP4   : return run(tools[ MP4_ENCODER].bin + " " + tools[ MP4_ENCODER].options + " -o " + FXPath::enquote(output) + " " + FXPath::enquote(input)); break;
    case FILE_MPC   : return run(tools[ MPC_ENCODER].bin + " " + tools[ MPC_ENCODER].options + "  " + FXPath::enquote(input) + " " + FXPath::enquote(output)); break;
    default         : break;
    }
  return false;
  }

static void remove_option(FXString & buffer,const FXchar * opt){
  FXint len=strlen(opt);
  if (!buffer.empty()){
    FXint p = buffer.find(opt);
    if (p==-1) return;
    else if (p==0) {
      if (buffer[p+len+1]==' ')
        buffer.erase(p,len+1);
      else
        buffer.erase(p,len);
      }
    else {
      if (buffer[p-1]==' ')
        buffer.erase(p-1,len+1);
      else
        buffer.erase(p,len);
      }
    }
  }
static void add_option(FXString & buffer,const FXchar * opt){
  if (buffer.empty()) {
    buffer+=opt;
    }
  else if (!buffer.contains(opt)){
    buffer+=" ";
    buffer+=opt;
    }
  }


void AudioTools::quiet(FXbool enable) {
  if (enable) {
    for (FXint i=0;i<NTOOLS;i++) {
      if (tools[i].quiet) add_option(tools[i].options,tools[i].quiet);
      }
    }
  else {
    for (FXint i=0;i<NTOOLS;i++) {
      if (tools[i].quiet) remove_option(tools[i].options,tools[i].quiet);
      }
    }
  }

void AudioTools::load_rc(const FXString & filename){
  FXSettings settings;
#if FOXVERSION < FXVERSION(1,7,25)  
  if (settings.parseFile(filename,true)) {
#else
  if (settings.parseFile(filename)) {
#endif
    for (FXint i=0;i<NTOOLS;i++) {
      tools[i].bin     = settings.readStringEntry(tools[i].section,"bin",tools[i].bin.text());
      tools[i].options = settings.readStringEntry(tools[i].section,"options",tools[i].options.text());
      }
    }
  }

void AudioTools::init_rc(const FXString & filename) {
  FILE * fp = fopen(filename.text(),"w");
  if (fp) {
    for (FXint i=0;i<NTOOLS;i++) {
      fprintf(fp,"#[%s]\n",tools[i].section);
      fprintf(fp,"#bin=%s\n",tools[i].bin.text());
      fprintf(fp,"#options=%s\n\n",tools[i].options.text());
      }
    fclose(fp);
    }
  }

FXbool AudioTools::check(FXuint from,FXuint to) const {
  FXString decoder = tools[decoder_map[from]].bin.before(' ');
  FXString encoder = tools[encoder_map[to]].bin.before(' ');

  decoder=FXPath::search(decoder,FXSystem::getEnvironment("PATH"));
  encoder=FXPath::search(encoder,FXSystem::getEnvironment("PATH"));

  if (decoder.empty()) {

    if (!encoder.empty() || encoder_supports(to,from))
      return true;

    return false;
    }
  else if (encoder.empty()) {
    return false;
    }
  return true;
  }



