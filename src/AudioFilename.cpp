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
#include "fox.h"
#include <FX88591Codec.h>
#include <FX88592Codec.h>
#include <FX88593Codec.h>
#include <FX88594Codec.h>
#include <FX88595Codec.h>
#include <FX88596Codec.h>
#include <FX88597Codec.h>
#include <FX88598Codec.h>
#include <FX88599Codec.h>
#include <FX885910Codec.h>
#include <FX885911Codec.h>
#include <FX885913Codec.h>
#include <FX885914Codec.h>
#include <FX885915Codec.h>
#include <FX885916Codec.h>
#include <FXCP437Codec.h>
#include <FXCP850Codec.h>
#include <FXCP852Codec.h>
#include <FXCP855Codec.h>
#include <FXCP856Codec.h>
#include <FXCP857Codec.h>
#include <FXCP860Codec.h>
#include <FXCP861Codec.h>
#include <FXCP862Codec.h>
#include <FXCP863Codec.h>
#include <FXCP864Codec.h>
#include <FXCP865Codec.h>
#include <FXCP866Codec.h>
#include <FXCP869Codec.h>
#include <FXCP874Codec.h>
#include <FXCP1250Codec.h>
#include <FXCP1251Codec.h>
#include <FXCP1252Codec.h>
#include <FXCP1253Codec.h>
#include <FXCP1254Codec.h>
#include <FXCP1255Codec.h>
#include <FXCP1256Codec.h>
#include <FXCP1257Codec.h>
#include <FXCP1258Codec.h>
#include <FXKOI8RCodec.h>
#include <FXUTF8Codec.h>

#include "AudioTags.h"
#include "AudioFilename.h"

/*
  Conditionals
  ------------

    ?c<a|b> => display a if c is not empty, display b if c is empty)
    ?c      => display c if not empty

    T => track title
    A => album title
    P => album artist name
    p => track artist name
    G => genre
    N => 2 digit track number
    n => track number
    d => disc number
    y => track year
    w => composer
    c => conductor
*/


using namespace GMFilename;

namespace GMFilename {


FXbool parsecodec(const FXString & codec,FXTextCodec *& textcodec){
  if (comparecase(codec,"ascii")==0)
    textcodec=NULL;
  else if (comparecase(codec,"utf-8")==0 || comparecase(codec,"utf8")==0)
    textcodec=new FXUTF8Codec;
  else if (comparecase(codec,"iso8859-1")==0)
    textcodec=new FX88591Codec;
  else if (comparecase(codec,"iso8859-2")==0)
    textcodec=new FX88592Codec;
  else if (comparecase(codec,"iso8859-3")==0)
    textcodec=new FX88593Codec;
  else if (comparecase(codec,"iso8859-4")==0)
    textcodec=new FX88594Codec;
  else if (comparecase(codec,"iso8859-5")==0)
    textcodec=new FX88595Codec;
  else if (comparecase(codec,"iso8859-6")==0)
    textcodec=new FX88596Codec;
  else if (comparecase(codec,"iso8859-7")==0)
    textcodec=new FX88597Codec;
  else if (comparecase(codec,"iso8859-8")==0)
    textcodec=new FX88598Codec;
  else if (comparecase(codec,"iso8859-9")==0)
    textcodec=new FX88599Codec;
  else if (comparecase(codec,"iso8859-10")==0)
    textcodec=new FX885910Codec;
  else if (comparecase(codec,"iso8859-11")==0)
    textcodec=new FX885911Codec;
  else if (comparecase(codec,"iso8859-13")==0)
    textcodec=new FX885913Codec;
  else if (comparecase(codec,"iso8859-14")==0)
    textcodec=new FX885914Codec;
  else if (comparecase(codec,"iso8859-15")==0)
    textcodec=new FX885915Codec;
  else if (comparecase(codec,"iso8859-16")==0)
    textcodec=new FX885916Codec;
  else if (comparecase(codec,"cp437")==0)
    textcodec=new FXCP437Codec;
  else if (comparecase(codec,"cp850")==0)
    textcodec=new FXCP850Codec;
  else if (comparecase(codec,"cp852")==0)
    textcodec=new FXCP852Codec;
  else if (comparecase(codec,"cp855")==0)
    textcodec=new FXCP855Codec;
  else if (comparecase(codec,"cp856")==0)
    textcodec=new FXCP856Codec;
  else if (comparecase(codec,"cp857")==0)
    textcodec=new FXCP857Codec;
  else if (comparecase(codec,"cp860")==0)
    textcodec=new FXCP860Codec;
  else if (comparecase(codec,"cp861")==0)
    textcodec=new FXCP861Codec;
  else if (comparecase(codec,"cp862")==0)
    textcodec=new FXCP862Codec;
  else if (comparecase(codec,"cp863")==0)
    textcodec=new FXCP863Codec;
  else if (comparecase(codec,"cp864")==0)
    textcodec=new FXCP864Codec;
  else if (comparecase(codec,"cp865")==0)
    textcodec=new FXCP865Codec;
  else if (comparecase(codec,"cp866")==0)
    textcodec=new FXCP866Codec;
  else if (comparecase(codec,"cp869")==0)
    textcodec=new FXCP869Codec;
  else if (comparecase(codec,"cp874")==0)
    textcodec=new FXCP874Codec;
  else if (comparecase(codec,"cp1250")==0)
    textcodec=new FXCP1250Codec;
  else if (comparecase(codec,"cp1251")==0)
    textcodec=new FXCP1251Codec;
  else if (comparecase(codec,"cp1252")==0)
    textcodec=new FXCP1252Codec;
  else if (comparecase(codec,"cp1253")==0)
    textcodec=new FXCP1253Codec;
  else if (comparecase(codec,"cp1254")==0)
    textcodec=new FXCP1254Codec;
  else if (comparecase(codec,"cp1255")==0)
    textcodec=new FXCP1255Codec;
  else if (comparecase(codec,"cp1256")==0)
    textcodec=new FXCP1256Codec;
  else if (comparecase(codec,"cp1257")==0)
    textcodec=new FXCP1257Codec;
  else if (comparecase(codec,"cp1258")==0)
    textcodec=new FXCP1257Codec;
  else if (comparecase(codec,"koi8-r")==0)
    textcodec=new FXKOI8RCodec;
  else
    return false;

  return true;
  }


/*
    0) trim white spaces
    1) only want printable characters
    2) substitute spaces for underscores [optional]
    3) make everything lowercase [optional]
    4) do not use any of the shell dangerous character set \'\\#~!\"$&();<>|`^*?[]/
*/
static FXString filter(const FXString & input,const FXString & forbidden,FXuint options){
//  const FXString forbidden = "\'\\#~!\"$&();<>|`^*?[]/.:";  // Allowed  %+,-=@_{}
  FXString result;
  FXwchar w;
  FXint i;
  FXbool lastspace=false;
  for (i=0;i<input.length();i=input.inc(i)){
    w = input.wc(i);
    if (Unicode::isPrint(w)) {
      if (Unicode::isSpace(w)) {
        if (!lastspace) {
          if (options&NOSPACES)
            result+="_";
          else
            result.append(&w,1);
          lastspace=true;
          }
        }
      else if (options&LOWERCASE && Unicode::isUpper(w)){
        w = Unicode::toLower(w);
        result.append(&w,1);
        lastspace=false;
        }
      else if (Unicode::isAscii(w) && Ascii::isPunct(input[i])) {
        if (forbidden.find(input[i])==-1)
        result.append(&w,1);
        lastspace=false;
        }
      else {
        result.append(&w,1);
        lastspace=false;
        }
      }
    }
  return result.trim();
  }


/* convert UTF8 to given 8 bit codec. decompose if necessary */
static FXString convert_and_decompose(const FXString & input,FXTextCodec * codec) {
  register FXint i=0,j=0;
  FXint len;
  FXString result;
  FXString input_decompose;
  FXchar c;

  for (i=0;i<input.length();i=input.inc(i)){
    len = codec->utf2mb(&c,1,&input[i],input.extent(i));
    if (len>0 && c!=0x1A) {
      result+=c;
      }
    else {
      input_decompose.assign(&input[i],input.extent(i));
      input_decompose = decompose(input_decompose,DecomposeCompat);
      for (j=0;j<input_decompose.length();j=input_decompose.inc(j)){
        len = codec->utf2mb(&c,1,&input_decompose[j],input_decompose.extent(j));
        if (len>0 && c!=0x1A) {
          result+=c;
          }
        }
      }
    }
  return result;
  }

/* convert UTF8 to given 7 bit assci */
static FXString convert_and_decompose(const FXString & input) {
  register FXint i=0;
  FXString result;
  FXString in = decompose(input,DecomposeCanonical);
  for (i=0;i<in.length();i=in.inc(i)){
    if (Ascii::isAscii(in[i]) && Ascii::isPrint(in[i]) ) {
      result+=in[i];
      }
    }
  return result;
  }


static FXString to_8bit_codec(const FXString & input,FXTextCodec * codec,const FXString & forbidden,FXuint opts) {
  FXString result;

  /// Filter the input
  result = filter(input,forbidden,opts);

  /// Make sure it is properly composed. Should we do this?
  result = compose(result,DecomposeCompat);

  /// convert to given codec.
  if (dynamic_cast<FXUTF8Codec*>(codec)==NULL)
    result = convert_and_decompose(result,codec);

  return result;
  }


static FXString to_8bit_ascii(const FXString & input,const FXString & forbidden,FXuint opts) {
  FXString result;

  /// Filter the input
  result = filter(input,forbidden,opts);

  /// Make sure it is properly composed. Should we do this?
  result = compose(result,DecomposeCompat);

  /// convert to given codec.
  result = convert_and_decompose(result);

  /// Return result
  return result;
  }

static FXString convert(const FXString & input,FXTextCodec * codec,const FXString & forbidden,FXuint opts) {
  if (codec)
    return to_8bit_codec(input,codec,forbidden,opts);
  else
    return to_8bit_ascii(input,forbidden,opts);
  }

static FXString get_field(FXchar field,const GMTrack & track,FXTextCodec * codec,const FXString & forbidden,FXuint opts){
  switch(field) {
    case 'T': return convert(track.title,codec,forbidden,opts); break;
    case 'A': return convert(track.album,codec,forbidden,opts); break;
    case 'P': return convert(track.album_artist,codec,forbidden,opts); break;
    case 'p': return convert(track.artist,codec,forbidden,opts); break;
    case 'G': return convert(track.tags[0],codec,forbidden,opts); break;
    case 'w': return convert(track.composer,codec,forbidden,opts); break;
    case 'c': return convert(track.conductor,codec,forbidden,opts); break;
    case 'N': return GMStringFormat("%.2d",GMTRACKNO(track.no)); break;
    case 'n': return GMStringVal(GMTRACKNO(track.no));  break;
    case 'd': return GMStringVal(GMDISCNO(track.no)); break;
    case 'y': return GMStringVal(track.year); break;

    }
  return FXString::null;
  }


static FXbool eval_field(FXchar field,const GMTrack & track,FXTextCodec * codec,const FXString & forbidden,FXuint opts){
  switch(field) {
    case 'T': return !convert(track.title,codec,forbidden,opts).empty(); break;
    case 'A': return !convert(track.album,codec,forbidden,opts).empty(); break;
    case 'P': return !convert(track.album_artist,codec,forbidden,opts).empty(); break;
    case 'p': return !convert(track.artist,codec,forbidden,opts).empty(); break;
    case 'G': return !convert(track.tags[0],codec,forbidden,opts).empty(); break;
    case 'w': return !convert(track.composer,codec,forbidden,opts).empty(); break;
    case 'c': return !convert(track.conductor,codec,forbidden,opts).empty(); break;
    case 'N': return GMTRACKNO(track.no)!=0; break;
    case 'n': return GMTRACKNO(track.no)!=0; break;
    case 'd': return GMDISCNO(track.no)!=0; break;
    case 'y': return track.year!=0; break;
    }
  return false;
  }



FXString format(const GMTrack & track,const FXString & path,const FXString & forbidden,const FXuint & options,FXTextCodec * textcodec){
  FXString field;
  FXwchar w;

  /// Valid field identifiers
  const FXchar valid[]="TAPpGNndy";

  /// Condition Stack
  FXint depth=0;
  FXArray<FXbool> cs(1);
  cs[0]=true;

  FXString result;

  /// Parse the field entries
  for (FXint i=0;i<path.length();i=path.inc(i)){

    /// Check for end of condition or else statement
    if (depth) {

      /// Else clause, the condition state is the opposite of the 'if clause', but only if
      /// the parent condition is true. if the parent clause is false, than both if and else clause
      /// should be false and we shouldn't change it.
      if (path[i]=='|') {
        if (cs[depth-1]==true)
          cs[depth]=!cs[depth];
        continue;
        }
      else if (path[i]=='>') {
        depth--;
        cs.erase(cs.no()-1);
        continue;
        }
      }

    /// Check beginning of conditional
    if (path[i]=='?' && strchr(valid,path[i+1]) ) {

      /// Condition with if else clause
      if (path[i+2]=='<') {
        /// condition is true if eval_fied returns true and the parent condition is also true.
        cs.append(cs[depth] && eval_field(path[i+1],track,textcodec,forbidden,options));
        i+=2;
        depth++;
        continue;
        }
      /// Simplified condition just display if not empty
      else if (cs[depth]) {
        if (eval_field(path[i+1],track,textcodec,forbidden,options)) {
          field = get_field(path[i+1],track,textcodec,forbidden,options);
          if (field.empty())
            result.trim();
          else
            result+=field;
          }
        i+=1;
        continue;
        }
      }

    /// only add stuff if our current condition allows it
    if (cs[depth]) {

      /// get field
      if (path[i]=='%' && strchr(valid,path[i+1]) ) {
        field = get_field(path[i+1],track,textcodec,forbidden,options);
        if (field.empty())
          result.trim();
        else
          result+=field;
        i+=1;
        continue;
        }

      /// Add anything else
      w = path.wc(i);
      result.append(&w,1);
      }

    }

  result.trim();
  return result;
  }


void parse(const FXString & mrl,GMTrack & track,const FXString & mask,FXuint options) {
  FXint nsep=0,i,j;
  FXString input,field;
  FXString dir;
  FXchar sep,item;
  FXint beg=0,end=0;

  // Determine number of path separators
  for (i=0;i<mask.length();i++){
    if (mask[i]=='\\') i+=1;
    else if (mask[i]=='/') nsep++;
    }

  /// Remove path components from the front that are not part of the mask
  /// Or if the mask doesn't specifiy path components, we just take the title of the filename.
  if (nsep) {
    input=FXPath::title(mrl);
    FXString dir=FXPath::directory(mrl);
    for (i=0;i<nsep;i++) {
      input = FXPath::name(dir) + PATHSEPSTRING + input;
      dir=FXPath::upLevel(dir);
      }
    }
  else {
    input=FXPath::title(mrl);
    }

  // Start from fresh
  if (options&OVERWRITE)
    track.clear();

//  fxmessage("filename: %s\n",track.mrl.text());
//  fxmessage("mask: %s\n",mask.text());
//  fxmessage("input to scan: %s \n",input.text());

  for (i=0;i<mask.length()&&beg<input.length();i++) {
    if (mask[i]=='%' && ((i+1)<mask.length()) && ( i==0 || mask[i-1]!='\\')) {
      i+=1;
      item=mask[i];
      j=i+1;

      /// Determine Separator
      sep=' ';
      if (j>=mask.length()) {
        sep='\0';
        }
      else if (mask[j]!='%') {
        i++;
        if (mask[j]=='\\') {
          j++;i++;
          if (j<mask.length())
            sep=mask[j];
          else
            sep=' ';
          }
        else {
          sep=mask[j];
          }
        }

      /// Get substring until separator
      end=beg;
      while(input[end]!=sep && end<input.length()) end++;

      if (end-beg>0) {
        field=input.mid(beg,end-beg).simplify();
        if (options&REPLACE_UNDERSCORE) field.substitute('_',' ');
        switch(item) {
          case 'T' : if (options&OVERWRITE || track.title.empty()) track.title.adopt(field); break;
          case 'P' : if (options&OVERWRITE || track.album_artist.empty()) track.album_artist.adopt(field); break;
          case 'A' : if (options&OVERWRITE || track.album.empty()) track.album.adopt(field); break;
          case 'p' : if (options&OVERWRITE || track.artist.empty()) track.artist.adopt(field); break;
          case 'n' :
          case 'N' : if (options&OVERWRITE) track.no = GMIntVal(field); break;
          }
        //fxmessage("%%%c=\"%s\"\n",item,field.text());
        beg=end+1;
        }
      }
    else {
      //fxmessage("eat %c: %s\n",mask[i],&input[beg]);
      while(input[beg]!=mask[i] && beg<input.length()) beg++;
      beg++;
      }
    }

  }

}
