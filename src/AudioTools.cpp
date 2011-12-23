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

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>



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

#if 0
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
#endif

static void make_argv(FXArray<FXchar*> & argv,const FXString & operation){
  FXint pos=0;  
  argv.no(1);  
  do {
    argv[argv.no()-1]=(FXchar*)(operation.text()+pos);
    argv.no(argv.no()+1);
    pos=operation.find('\0',pos);
    if (pos==-1 || pos+1>=operation.length()) break;
    pos+=1;
    }
  while(1);
  argv[argv.no()-1]=NULL;  

#if 0
  argv[argv.no()-1]=NULL;
  for (int i=0;i<argv.no();i++) {
    fxmessage("%d:%s\n",i,argv[i]);
    }
#endif
  }



FXbool AudioTools::run(const FXString & operation) const{
  if (dryrun) {
    fxmessage("%s\n",operation.text());
    }
  else {
    FXArray<FXchar*> argv;

    make_argv(argv,operation);
    
    pid_t pid = fork();
    if (pid==-1){ /// Failure delivered to Parent Process
      fxwarning("Error forking\n");  
      return false;
      }
    else if (pid==0) { /// Child Process
      int i = sysconf(_SC_OPEN_MAX);
      while (--i >= 3) {
        close(i);
        }       
      execvp(argv[0],argv.data());
      exit(EXIT_FAILURE);
      }
    else { /// Parent Process
      int status=0;
      while(1) {      
        pid_t child = waitpid(pid,&status,0);        
        if (child==pid) {
          if (WIFEXITED(status)) {
            //fxmessage("status: %d\n",WEXITSTATUS(status));
            return (WEXITSTATUS(status)==0);
            }
          else if (WIFSIGNALED(status)) {
            return false;
            }  
          }
        else if (child==-1) {
          fxmessage("got errno: %d\n",errno);
          }
        }   
      return true;
      }
    }
  return true;
  }    
        
    


























FXbool AudioTools::runTool(FXuint tool,const FXString & input,const FXString & output) const {
  FXString cmd;

  cmd = tools[tool].bin + '\0';
  if (!tools[tool].options.empty())
    cmd += tools[tool].options + '\0';
  cmd.substitute(' ','\0');

  switch(tool) {
    case FLAC_DECODER: 
    case FLAC_ENCODER: 
    case MP3_DECODER :  
    case MP3_ENCODER : 
    case OGG_DECODER :
    case OGG_ENCODER : cmd += input;
                       cmd.append("\0-o\0",4);
                       cmd += output;
                       break;

    case MP4_DECODER :
    case MP4_ENCODER : cmd.append("-o\0",3);
                       cmd += output + '\0' + input;
                       break;

    case MPC_DECODER :
    case MPC_ENCODER : cmd += input+'\0' +output;        
                       break;
    default          : return false; 
                       break;
    }
  return run(cmd);
  }


FXbool AudioTools::decode(FXuint type,const FXString & input,const FXString & output) const {
  return runTool(decoder_map[type],input,output);
  }   

FXbool AudioTools::encode(FXuint type,const FXString & input,const FXString & output) const {
  return runTool(encoder_map[type],input,output);
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

void AudioTools::load_rc(FXSettings & settings){
  for (FXint i=0;i<NTOOLS;i++) {
    tools[i].bin     = settings.readStringEntry(tools[i].section,"bin",tools[i].bin.text());
    tools[i].options = settings.readStringEntry(tools[i].section,"options",tools[i].options.text());
    }
  }

void AudioTools::init_rc(FILE * fp) {
  for (FXint i=0;i<NTOOLS;i++) {
    fprintf(fp,"#[%s]\n",tools[i].section);
    fprintf(fp,"#bin=%s\n",tools[i].bin.text());
    fprintf(fp,"#options=%s\n\n",tools[i].options.text());
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



