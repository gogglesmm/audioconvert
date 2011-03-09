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
#include "acconfig.h"
#include "AudioTools.h"
#include "AudioConvert.h"

static void ac_print_help() {
  fxmessage("Usage: audioconvert [options] source [destination]\n\n"
            "General options:\n"
            " -h, --help            Display this help page\n"
            " -v, --version         Display version information\n"
            "\n"
            " -n, --dry-run         Print operations instead of performing\n"
            "     --overwrite       Force overwriting existing files\n"
            " -q, --quiet           Suppress Output Messages\n"
            "\n"
            "Output options:\n"
            "     --flac=<action>   Set desired action for flac files\n"
            "     --ogg=<action>    Set desired action for ogg vorbis files\n"
            "     --mp3=<action>    Set desired action for mp3 files\n"
            "     --mp4=<action>    Set desired action for mp4/aac files\n"
            "     --mpc=<action>    Set desired action for mpc files\n"
            "     --all=<action>    Set desired action for all file types\n"
            "\n"
            "Actions:\n"
            "     flac - Convert source file to flac.\n"
            "      ogg - Convert source file to ogg vorbis.\n"
            "      mp3 - Convert source file to mp3.\n"
            "      mp4 - Convert source file to mp4.\n"
            "      mpc - Convert source file to Musepack.\n"
            "     copy - Copy source without conversion to destination.\n"
            "     none - Do nothing. Source file is skipped. (default)\n\n"
            );
  }

static bool ac_display_help(int argc,char * argv[]) {
  if (argc>1) {
    for (int i=1;i<argc;i++){
      if ( (comparecase(argv[i],"--help")==0) || (comparecase(argv[i],"-h")==0) ) {
        ac_print_help();
        return true;
        }
      else if ( (comparecase(argv[i],"--version")==0) || (comparecase(argv[i],"-v")==0) ) {
        fxmessage("audioconvert %d.%d.%d\n",APPLICATION_MAJOR,APPLICATION_MINOR,APPLICATION_LEVEL);
        return true;
        }
      }
    }
  else {
    ac_print_help();
    return true;
    }
  return false;
  }

int main(int argc,char *argv[]){

  if (ac_display_help(argc,argv))
    return 0;

  AudioConverter converter;

  if (!converter.parse(argc,argv))
    return 1;

  return converter.run();
  }
