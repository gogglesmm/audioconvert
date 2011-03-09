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
// Uncomment to compile with development taglib
//#define TAGLIB_TRUNK 1

#include <fox.h>

#include <math.h>
#include <errno.h>

#include <tag.h>
#include <fileref.h>
#include <tstring.h>
#include <id3v1genres.h>
#include <id3v2tag.h>
#include <id3v2framefactory.h>

#include <mpegfile.h>
#include <vorbisfile.h>
#include <flacfile.h>
#include <apetag.h>
#include <textidentificationframe.h>
#include <attachedpictureframe.h>

#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
#define TAGLIB_HAVE_MP4 1
#endif

#ifdef TAGLIB_HAVE_MP4
#include "mp4file.h"
#include "mp4tag.h"
#include "mp4coverart.h"
#endif

#include "AudioTags.h"

#include "FXPNGImage.h"
#include "FXJPGImage.h"

/// FOX 1.7 has its own scanf and always uses C locale for number conversions.
static FXdouble gm_parse_number(const FXString & str) {
  if (str.empty())
    return NAN;

  FXdouble value=NAN;
  if (str.scan("%lg",&value)==1)
    return value;
  else
    return NAN;
  }


/******************************************************************************/
/* HELPER CLASS TO ACCESS ADDITIONAL TAGS FROM FILE                           */
/******************************************************************************/

class GMFileTag {
public:
  TagLib::Tag               * tag;
#ifdef TAGLIB_HAVE_MP4
  TagLib::MP4::Tag          * mp4;
#else
  void                      * mp4;
#endif
  TagLib::Ogg::XiphComment  * xiph;
  TagLib::ID3v2::Tag        * id3v2;
  TagLib::APE::Tag          * ape;
protected:
  void xiph_update_field(const FXchar * field,const FXString & value);
  void xiph_update_field(const FXchar * field,const FXStringList & value);
  void id3v2_update_field(const FXchar * field,const FXString & value);
  void id3v2_update_field(const FXchar * field,const FXStringList & value);
  void mp4_update_field(const FXchar * field,const FXString & value);
  void mp4_update_field(const FXchar * field,const FXStringList & value);
  void ape_update_field(const FXchar * field,const FXStringList & value);
  void xiph_get_field(const FXchar * field,FXString &);
  void xiph_get_field(const FXchar * field,FXStringList &);
  void id3v2_get_field(const FXchar * field,FXString &);
  void id3v2_get_field(const FXchar * field,FXStringList &);
  void mp4_get_field(const FXchar * field,FXString &);
  void mp4_get_field(const FXchar * field,FXStringList &);
  void ape_get_field(const FXchar * field,FXString &);
  void ape_get_field(const FXchar * field,FXStringList &);
  void parse_tagids(FXStringList&);
public:
  GMFileTag(const FXString & filename);
  GMFileTag(TagLib::File*);
  void setComposer(const FXString & value);
  void setConductor(const FXString & value);
  void setAlbumArtist(const FXString & value);
  void setTags(const FXStringList & value);
  void setDiscNumber(FXushort disc);
  void getComposer(FXString &);
  void getConductor(FXString &);
  void getAlbumArtist(FXString &);
  void getTags(FXStringList&);
  FXushort getDiscNumber();
  void getGain(FXdouble & track_gain,FXdouble & track_peak,FXdouble & album_gain,FXdouble & album_peak);
  };


/******************************************************************************/

GMFileTag::GMFileTag(const FXString &) :
    tag(NULL),
    mp4(NULL),
    xiph(NULL),
    id3v2(NULL),
    ape(NULL) {
  /// TODO
  }

GMFileTag::GMFileTag(TagLib::File * file) :
    tag(NULL),
    mp4(NULL),
    xiph(NULL),
    id3v2(NULL),
    ape(NULL) {

  TagLib::MPEG::File        * mpgfile   = NULL;
  TagLib::Ogg::Vorbis::File * oggfile   = NULL;
  TagLib::FLAC::File        * flacfile  = NULL;
#ifdef TAGLIB_HAVE_MP4
  TagLib::MP4::File         * mp4file   = NULL;
#endif

  tag = file->tag();

  if ((oggfile = dynamic_cast<TagLib::Ogg::Vorbis::File*>(file))) {
    xiph=oggfile->tag();
    }
  else if ((flacfile = dynamic_cast<TagLib::FLAC::File*>(file))){
    xiph=flacfile->xiphComment();
    id3v2=flacfile->ID3v2Tag();
    }
  else if ((mpgfile = dynamic_cast<TagLib::MPEG::File*>(file))){
    id3v2=mpgfile->ID3v2Tag();
    ape=mpgfile->APETag();
    }
#ifdef TAGLIB_HAVE_MP4
  else if ((mp4file = dynamic_cast<TagLib::MP4::File*>(file))){
    mp4=mp4file->tag();
    }
#endif
  }

void GMFileTag::xiph_update_field(const FXchar * field,const FXString & value) {
  FXASSERT(field);
  FXASSERT(xiph);
  if (!value.empty())
    xiph->addField(field,TagLib::String(value.text(),TagLib::String::UTF8),true);
  else
    xiph->removeField(field);
  }


void GMFileTag::xiph_update_field(const FXchar * field,const FXStringList & list) {
  FXASSERT(field);
  FXASSERT(xiph);
  xiph->removeField(field);
  for (FXint i=0;i<list.no();i++) {
    xiph->addField(field,TagLib::String(list[i].text(),TagLib::String::UTF8),false);
    }
  }


void  GMFileTag::xiph_get_field(const FXchar * field,FXString & value) {
  FXASSERT(field);
  FXASSERT(xiph);
  if (xiph->contains(field))
    value=xiph->fieldListMap()[field].front().toCString(true);
  else
    value.clear();
  }

void GMFileTag::xiph_get_field(const FXchar * field,FXStringList & list) {
  FXASSERT(field);
  FXASSERT(xiph);
  if (xiph->contains(field)) {
    const TagLib::StringList & fieldlist = xiph->fieldListMap()[field];
    for(TagLib::StringList::ConstIterator it = fieldlist.begin(); it != fieldlist.end(); it++) {
      list.append(it->toCString(true));
      }
    }
  else {
    list.clear();
    }
  }

void GMFileTag::ape_get_field(const FXchar * field,FXString & value) {
  FXASSERT(field);
  FXASSERT(ape);
  if (ape->itemListMap().contains(field) && !ape->itemListMap()[field].isEmpty())
    value=ape->itemListMap()[field].toString().toCString(true);
  else
    value.clear();
  }

void GMFileTag::ape_get_field(const FXchar * field,FXStringList & list) {
  FXASSERT(field);
  FXASSERT(ape);
  if (ape->itemListMap().contains(field)) {
    TagLib::StringList fieldlist = ape->itemListMap()[field].toStringList();
    for(TagLib::StringList::ConstIterator it = fieldlist.begin(); it != fieldlist.end(); it++) {
      list.append(it->toCString(true));
      }
    }
  else {
    list.clear();
    }
  }

void GMFileTag::ape_update_field(const FXchar * field,const FXStringList & list) {
  FXASSERT(field);
  FXASSERT(ape);
  ape->removeItem(field);

  TagLib::StringList values;
  for (FXint i=0;i<list.no();i++) {
    values.append(TagLib::String(list[i].text(),TagLib::String::UTF8));
    }

  ape->setItem(field,TagLib::APE::Item(field,values));
  }


void GMFileTag::id3v2_update_field(const FXchar * field,const FXString & value) {
  FXASSERT(field);
  FXASSERT(id3v2);
  if (value.empty()) {
    id3v2->removeFrames(field);
    }
  else if (id3v2->frameListMap().contains(field) && !id3v2->frameListMap()[field].isEmpty()) {
    id3v2->frameListMap()[field].front()->setText(TagLib::String(value.text(),TagLib::String::UTF8));
    }
  else {
    TagLib::ID3v2::TextIdentificationFrame *frame = new TagLib::ID3v2::TextIdentificationFrame(field,TagLib::ID3v2::FrameFactory::instance()->defaultTextEncoding());
    frame->setText(TagLib::String(value.text(),TagLib::String::UTF8) );
    id3v2->addFrame(frame);
    }
  }

void GMFileTag::id3v2_update_field(const FXchar * field,const FXStringList & list) {
  FXASSERT(field);
  FXASSERT(id3v2);
  if (list.no()==0) {
    id3v2->removeFrames(field);
    }
  else {
    TagLib::ID3v2::TextIdentificationFrame * frame = NULL;
    if (id3v2->frameListMap().contains(field) && !id3v2->frameListMap()[field].isEmpty()) {
      frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3v2->frameListMap()[field].front());
      }
    else {
      frame = new TagLib::ID3v2::TextIdentificationFrame(field,TagLib::ID3v2::FrameFactory::instance()->defaultTextEncoding());
      id3v2->addFrame(frame);
      }
    FXASSERT(frame);

    TagLib::StringList values;
    for (FXint i=0;i<list.no();i++) {
      values.append(TagLib::String(list[i].text(),TagLib::String::UTF8));
      }
    frame->setText(values);
    }
  }

void  GMFileTag::id3v2_get_field(const FXchar * field,FXString & value) {
  FXASSERT(field);
  FXASSERT(id3v2);
  if (id3v2->frameListMap().contains(field) && !id3v2->frameListMap()[field].isEmpty() )
    value=id3v2->frameListMap()[field].front()->toString().toCString(true);
  else
    value.clear();
  }

void  GMFileTag::id3v2_get_field(const FXchar * field,FXStringList & list) {
  FXASSERT(field);
  FXASSERT(id3v2);
  if (id3v2->frameListMap().contains(field) && !id3v2->frameListMap()[field].isEmpty() ) {
    TagLib::ID3v2::TextIdentificationFrame * frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3v2->frameListMap()[field].front());
    TagLib::StringList fieldlist = frame->fieldList();
    for(TagLib::StringList::ConstIterator it = fieldlist.begin(); it != fieldlist.end(); it++) {
      list.append(it->toCString(true));
      }
    }
  else {
    list.clear();
    }
  }


void GMFileTag::mp4_update_field(const FXchar * field,const FXString & value) {
#ifdef TAGLIB_HAVE_MP4
  FXASSERT(field);
  FXASSERT(mp4);
  if (!value.empty())
    mp4->itemListMap()[field] = TagLib::StringList(TagLib::String(value.text(),TagLib::String::UTF8));
  else
    mp4->itemListMap().erase(field);
#endif
  }


void GMFileTag::mp4_update_field(const FXchar * field,const FXStringList & list) {
#ifdef TAGLIB_HAVE_MP4
  FXASSERT(field);
  FXASSERT(mp4);
  if (list.no()==0) {
    mp4->itemListMap().erase(field);
    }
  else {
    TagLib::StringList values;
    for (FXint i=0;i<list.no();i++) {
      values.append(TagLib::String(list[i].text(),TagLib::String::UTF8));
      }
    mp4->itemListMap()[field]=values;
    }
#endif
  }


void GMFileTag::mp4_get_field(const FXchar * field,FXString & value) {
#ifdef TAGLIB_HAVE_MP4
  FXASSERT(field);
  FXASSERT(mp4);
  if (mp4->itemListMap().contains(field) && !mp4->itemListMap().isEmpty())
    value=mp4->itemListMap()[field].toStringList().toString(", ").toCString(true);
  else
    value.clear();
#else
  value.clear();
#endif
  }


void GMFileTag::mp4_get_field(const FXchar * field,FXStringList & list) {
#ifdef TAGLIB_HAVE_MP4
  FXASSERT(field);
  FXASSERT(mp4);
  if (mp4->itemListMap().contains(field) && !mp4->itemListMap().isEmpty()) {
    TagLib::StringList fieldlist = mp4->itemListMap()[field].toStringList();
    for(TagLib::StringList::ConstIterator it = fieldlist.begin(); it != fieldlist.end(); it++) {
      list.append(it->toCString(true));
      }
    }
  else
    list.clear();
#else
  list.clear();
#endif
  }



/******************************************************************************/

void GMFileTag::setComposer(const FXString & composer) {
  if (xiph)
    xiph_update_field("COMPOSER",composer);
  else if (id3v2)
    id3v2_update_field("TCOM",composer);
  else if (mp4)
    mp4_update_field("\251wrt",composer);
  }

void GMFileTag::getComposer(FXString & composer) {
  if (xiph)
    xiph_get_field("COMPOSER",composer);
  else if (id3v2)
    id3v2_get_field("TCOM",composer);
  else if (mp4)
    mp4_get_field("\251wrt",composer);
  else
    composer.clear();
  }

void GMFileTag::setConductor(const FXString & conductor) {
  if (xiph)
    xiph_update_field("COMPOSER",conductor);
  else if (id3v2)
    id3v2_update_field("TPE3",conductor);
  else if (mp4)
    mp4_update_field("----:com.apple.iTunes:CONDUCTOR",conductor);
  }

void GMFileTag::getConductor(FXString & conductor) {
  if (xiph)
    xiph_get_field("COMPOSER",conductor);
  else if (id3v2)
    id3v2_get_field("TPE3",conductor);
  else if (mp4)
    mp4_get_field("----:com.apple.iTunes:CONDUCTOR",conductor);
  else
    conductor.clear();
  }


void GMFileTag::setAlbumArtist(const FXString & albumartist) {
  if (xiph)
    xiph_update_field("ALBUMARTIST",albumartist);
  else if (id3v2)
    id3v2_update_field("TPE2",albumartist);
  else if (mp4)
    mp4_update_field("aART",albumartist);
  }


void GMFileTag::getAlbumArtist(FXString & albumartist) {
  if (xiph)
    xiph_get_field("ALBUMARTIST",albumartist);
  else if (id3v2)
    id3v2_get_field("TPE2",albumartist);
  else if (mp4)
    mp4_get_field("aART",albumartist);
  else
    albumartist.clear();
  }

void GMFileTag::setTags(const FXStringList & tags){
  if (xiph)
    xiph_update_field("GENRE",tags);
  else if (id3v2)
    id3v2_update_field("TCON",tags);
  else if (mp4)
    mp4_update_field("\251gen",tags);
  else if (ape)
    ape_update_field("GENRE",tags);
  else {
    if (tags.no())
      tag->setGenre(TagLib::String(tags[0].text(),TagLib::String::UTF8));
    else
      tag->setGenre(TagLib::String("",TagLib::String::UTF8));
    }
  }

void GMFileTag::getTags(FXStringList & tags) {
  if (xiph)
    xiph_get_field("GENRE",tags);
  else if (id3v2) {
    id3v2_get_field("TCON",tags);
    parse_tagids(tags);
    }
  else if (mp4)
    mp4_get_field("\251gen",tags);
  else if (ape)
    ape_get_field("GENRE",tags);
  else
    tags.append(FXString(tag->genre().toCString(true)));
  }

static FXbool to_int(const FXString & str,FXint & val){
  char * endptr=NULL;
  errno=0;
  val=strtol(str.text(),&endptr,10);
  if (errno==0) {
    if (endptr==str.text())
      return false;
    return true;
    }
  return false;
  }


void GMFileTag::parse_tagids(FXStringList & tags){
  FXint id;
  for (FXint i=tags.no()-1;i>=0;i--){
    if (to_int(tags[i],id)) {
      tags[i]=TagLib::ID3v1::genre(id).toCString(true);
      }
    }
  }


void GMFileTag::setDiscNumber(FXushort disc) {
  if (xiph) {
    if (disc>0)
      xiph_update_field("DISCNUMBER",GMStringFormat("%d",disc));
    else
      xiph_update_field("DISCNUMBER",FXString::null);
    }
  else if (id3v2) {
    if (disc>0)
      id3v2_update_field("TPOS",GMStringFormat("%d",disc));
    else
      id3v2_update_field("TPOS",FXString::null);
    }
#ifdef TAGLIB_HAVE_MP4
  else if (mp4) {
    if (disc>0)
      mp4->itemListMap()["disk"] = TagLib::MP4::Item(disc,0);
    else
      mp4->itemListMap().erase("disk");
    }
#endif
  }


static FXushort string_to_disc_number(const FXString & disc) {
  if (disc.empty())
    return 0;
#if FOXVERSION >= FXVERSION(1,7,12)
  return FXMIN(disc.before('/').toUInt(),0xFFFF);
#else
  return FXMIN(FXUIntVal(disc.before('/')),0xFFFF);
#endif
  }

FXushort GMFileTag::getDiscNumber() {
  FXString disc;
  if (xiph) {
    xiph_get_field("DISCNUMBER",disc);
    return string_to_disc_number(disc);
    }
  else if (id3v2) {
    id3v2_get_field("TPOS",disc);
    return string_to_disc_number(disc);
    }
#ifdef TAGLIB_HAVE_MP4
  else if (mp4) {
    if (mp4->itemListMap().contains("disk"))
      return FXMIN(mp4->itemListMap()["disk"].toIntPair().first,0xFFFF);
    }
#endif
  return 0;
  }

void GMFileTag::getGain(FXdouble & track_gain,FXdouble & track_peak,FXdouble & album_gain,FXdouble & album_peak) {
  track_gain=track_peak=album_gain=album_peak=NAN;
  FXString tmp;
  if (xiph) {
    xiph_get_field("REPLAYGAIN_ALBUM_GAIN",tmp);
    album_gain=gm_parse_number(tmp);

    xiph_get_field("REPLAYGAIN_ALBUM_PEAK",tmp);
    album_peak=gm_parse_number(tmp);

    xiph_get_field("REPLAYGAIN_TRACK_GAIN",tmp);
    track_gain=gm_parse_number(tmp);

    xiph_get_field("REPLAYGAIN_TRACK_PEAK",tmp);
    track_peak=gm_parse_number(tmp);

    if (isnan(track_peak) && isnan(album_gain)) {
      xiph_get_field("RG_RADIO",tmp);
      track_gain=gm_parse_number(tmp);

      xiph_get_field("RG_PEAK",tmp);
      track_peak=gm_parse_number(tmp);

      xiph_get_field("RG_AUDIOPHILE",tmp);
      album_gain=gm_parse_number(tmp);
      }
    }
  else if (ape) {
    ape_get_field("REPLAYGAIN_ALBUM_GAIN",tmp);
    album_gain=gm_parse_number(tmp);

    ape_get_field("REPLAYGAIN_ALBUM_PEAK",tmp);
    album_peak=gm_parse_number(tmp);

    ape_get_field("REPLAYGAIN_TRACK_GAIN",tmp);
    track_gain=gm_parse_number(tmp);

    ape_get_field("REPLAYGAIN_TRACK_PEAK",tmp);
    track_peak=gm_parse_number(tmp);
    }
  }



/******************************************************************************/
/* GMTRACK IMPLEMENTATION                                                     */
/******************************************************************************/

FXbool GMTrack::saveTag(const FXString & filename,FXuint /*opts=0*/) {

  if (!FXStat::isWritable(filename))
    return false;

  TagLib::FileRef file(filename.text(),false);
  if (file.isNull() || !file.tag()) {
    return false;
    }

  TagLib::Tag * tag = file.tag();

  tag->setTitle(TagLib::String(title.text(),TagLib::String::UTF8));
  tag->setArtist(TagLib::String(artist.text(),TagLib::String::UTF8));
  tag->setAlbum(TagLib::String(album.text(),TagLib::String::UTF8));

  tag->setYear(year);
  tag->setTrack(getTrackNumber());

  GMFileTag filetags(file.file());

  filetags.setDiscNumber(getDiscNumber());
  filetags.setComposer(composer);
  filetags.setConductor(conductor);
  filetags.setTags(tags);

  if (album_artist!=artist && !album_artist.empty())
    filetags.setAlbumArtist(album_artist);
  else
    filetags.setAlbumArtist(FXString::null);

  return file.save();
  }


FXbool GMTrack::loadTag(const FXString & filename) {
  FXString disc,value;

  TagLib::FileRef file(filename.text());
  if (file.isNull() || !file.tag()) {
    clear();
    return false;
    }

  TagLib::Tag * tag = file.tag();

  artist       = tag->artist().toCString(true);
  album        = tag->album().toCString(true);
  title        = tag->title().toCString(true);
  year         = tag->year();
  no           = FXMIN(tag->track(),0xFFFF);

  artist.trim();
  album.trim();
  title.trim();

  GMFileTag filetags(file.file());

  filetags.getAlbumArtist(album_artist);
  filetags.getComposer(composer);
  filetags.getConductor(conductor);
  filetags.getGain(track_gain,track_peak,album_gain,album_peak);
  filetags.getTags(tags);

  if (album_artist.empty())
    album_artist=artist;

  setDiscNumber(filetags.getDiscNumber());
  return true;
  }

GMTrack::GMTrack() :
  year(0),
  no(0),
  album_gain(NAN),
  album_peak(NAN),
  track_gain(NAN),
  track_peak(NAN){
  }

void GMTrack::clear() {
  title.clear();
  artist.clear();
  album.clear();
  album_artist.clear();
  tags.clear();
  composer.clear();
  conductor.clear();
  year=0;
  no=0;
  album_gain=NAN;
  album_peak=NAN;
  track_gain=NAN;
  track_peak=NAN;
  }









static FXbool gm_decode_base64(FXuchar * buffer,FXint & len){
  static const char base64[256]={
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x3e,0x80,0x80,0x80,0x3f,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
    0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x80,0x80,0x80,0x80,0x80,
    0x80,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};

  FXuint  pos=0;
  FXuchar v;
  for (FXint i=0,b=0;i<len;i++) {
    v=base64[buffer[i]];
    if (v!=0x80) {
      switch(b) {
        case 0: buffer[pos]=(v<<2);
                b++;
                break;
        case 1: buffer[pos++]|=(v>>4);
                buffer[pos]=(v<<4);
                b++;
                break;
        case 2: buffer[pos++]|=(v>>2);
                buffer[pos]=(v<<6);
                b++;
                break;
        case 3: buffer[pos++]|=v;
                b=0;
                break;
        }
      }
    else {
      if (buffer[i]=='=' && b>1) {
        len=pos;
        return true;
        }
      else {
        return false;
        }
      }
    }
  len=pos;
  return true;
  }










static GMCover * id3v2_load_cover(TagLib::ID3v2::AttachedPictureFrame * frame) {
  FXString mime = frame->mimeType().toCString(true);
  /// Skip File Icon
  if (frame->type()==TagLib::ID3v2::AttachedPictureFrame::FileIcon ||
      frame->type()==TagLib::ID3v2::AttachedPictureFrame::OtherFileIcon ||
      frame->type()==TagLib::ID3v2::AttachedPictureFrame::ColouredFish) {
    return NULL;
    }
  return new GMCover((const FXuchar*)frame->picture().data(),frame->picture().size(),mime,frame->type());
  }







#if FOX_BIGENDIAN == 0
#define FLAC_LAST_BLOCK   0x80
#define FLAC_BLOCK_TYPE_MASK 0x7f
#define FLAC_BLOCK_TYPE(h) (h&0x7f)
#define FLAC_BLOCK_SIZE(h) ( ((h&0xFF00)<<8) | ((h&0xFF0000)>>8) | ((h&0xFF000000)>>24) )
#define FLAC_BLOCK_SET_TYPE(h,type) (h|=(type&FLAC_BLOCK_TYPE_MASK))
#define FLAC_BLOCK_SET_SIZE(h,size) (h|=(((size&0xFF)<<24) | ((size&0xFF0000)>>16) | ((size&0xFF00)<<8)))
#else
#define FLAC_LAST_BLOCK      0x80000000
#define FLAC_BLOCK_TYPE_MASK 0x7F000000
#define FLAC_BLOCK_TYPE(h)   ((h&0x7F000000)>>24)
#define FLAC_BLOCK_SIZE(h)   (h&0xFFFFFF)
#define FLAC_BLOCK_SET_TYPE(h,type) (h|=((type<<24)&FLAC_BLOCK_TYPE_MASK))
#define FLAC_BLOCK_SET_SIZE(h,size) (h|=(size&0xFFFFFF))
#endif


enum {
  FLAC_BLOCK_STREAMINFO     = 0,
  FLAC_BLOCK_PADDING        = 1,
  FLAC_BLOCK_VORBIS_COMMENT = 4,
  FLAC_BLOCK_PICTURE        = 6
  };


struct FlacPictureBlock{
  FXString    mimetype;
  FXString    description;
  FXuint      type;
  FXuint      width;
  FXuint      height;
  FXuint      bps;
  FXuint      ncolors;
  FXuint      data_size;
  FXuchar*    data;

  FXuint size() const {
    return 64 + description.length() + mimetype.length() + data_size;
    }
  };


static FXbool gm_read_uint32_be(FXIO & io,FXuint & v) {
#if FOX_BIGENDIAN == 0
  FXuchar block[4];
  if (io.readBlock(block,4)!=4) return false;
  ((FXuchar*)&v)[3]=block[0];
  ((FXuchar*)&v)[2]=block[1];
  ((FXuchar*)&v)[1]=block[2];
  ((FXuchar*)&v)[0]=block[3];
#else
  if (io.readBlock(&v,4)!=4) return false;
#endif
  return true;
  }


static FXbool gm_read_string_be(FXIO & io,FXString & v) {
  FXuint len=0;
  gm_read_uint32_be(io,len);
  if (len>0) {
    v.length(len);
    if (io.readBlock(&v[0],len)!=len)
      return false;
    }
  return true;
  }


static FXbool gm_uint32_be(const FXuchar * block,FXuint & v) {
#if FOX_BIGENDIAN == 0
  ((FXuchar*)&v)[3]=block[0];
  ((FXuchar*)&v)[2]=block[1];
  ((FXuchar*)&v)[1]=block[2];
  ((FXuchar*)&v)[0]=block[3];
#else
  ((FXuchar*)&v)[3]=block[3];
  ((FXuchar*)&v)[2]=block[2];
  ((FXuchar*)&v)[1]=block[1];
  ((FXuchar*)&v)[0]=block[0];
#endif
  return true;
  }






static GMCover * gm_flac_parse_block_picture(const FXuchar * buffer,FXint len) {
  FlacPictureBlock picture;
  const FXuchar * p = buffer;
  FXuint sz;
  gm_uint32_be(p,picture.type);

  /// Skip useless icons
  if (picture.type==GMCover::FileIcon || picture.type==GMCover::OtherFileIcon ||
      picture.type==GMCover::Fish) {
    return NULL;
    }
  p+=4;

  gm_uint32_be(p,sz);
  picture.mimetype.length(sz);
  picture.mimetype.assign((const FXchar*)p+4,sz);

  p+=(4+sz);

  gm_uint32_be(p,sz);
  picture.description.length(sz);
  picture.description.assign((const FXchar*)p+4,sz);

  p+=(4+sz);

  gm_uint32_be(p+0,picture.width);
  gm_uint32_be(p+4,picture.height);
  gm_uint32_be(p+8,picture.bps);
  gm_uint32_be(p+12,picture.ncolors);
  gm_uint32_be(p+16,picture.data_size);

  if (picture.data_size>0) {
    picture.data = (FXuchar*) p+20;
    if (picture.data+picture.data_size>buffer+len)
      return NULL;

    return new GMCover(picture.data,picture.data_size,picture.mimetype,picture.type,picture.description);
    }
  return NULL;
  }


GMCover * gm_flac_parse_block_picture(FXIO & io) {
  GMCover*  cover=NULL;
  FlacPictureBlock picture;

  gm_read_uint32_be(io,picture.type);

  /// Skip useless icons
  if (picture.type==GMCover::FileIcon || picture.type==GMCover::OtherFileIcon ||
      picture.type==GMCover::Fish) {
    return NULL;
    }

  gm_read_string_be(io,picture.mimetype);
  gm_read_string_be(io,picture.description);
  gm_read_uint32_be(io,picture.width);
  gm_read_uint32_be(io,picture.height);
  gm_read_uint32_be(io,picture.bps);
  gm_read_uint32_be(io,picture.ncolors);
  gm_read_uint32_be(io,picture.data_size);

  if (picture.data_size>0) {
    allocElms(picture.data,picture.data_size);
    if (io.readBlock(picture.data,picture.data_size)==picture.data_size) {
      cover = new GMCover(picture.data,picture.data_size,picture.mimetype,picture.type,picture.description);
      }
    freeElms(picture.data);
    }
  return cover;
  }


static FXbool gm_flac_parse_header(FXIO & io,FXuint & header) {
  FXchar  marker[4];

  if (io.readBlock(marker,4)!=4 || compare(marker,"fLaC",4))
    return false;

  if (io.readBlock(&header,4)!=4 || FLAC_BLOCK_TYPE(header)!=FLAC_BLOCK_STREAMINFO || FLAC_BLOCK_SIZE(header)!=34  || (header&FLAC_LAST_BLOCK))
    return false;

  /// Go to beginning of meta data
  io.position(34,FXIO::Current);
  return true;
  }

static FXbool gm_flac_next_block(FXIO & io,FXuint & header) {
  if (!(header&FLAC_LAST_BLOCK) && (io.readBlock(&header,4)==4))
    return true;
  else
    return false;
  }

static FXint flac_load_covers(const FXString & mrl,GMCoverList & covers) {
  FXuint  header;
  FXFile io;

  if (io.open(mrl,FXIO::Reading)) {

    if (!gm_flac_parse_header(io,header))
      return 0;

    while(gm_flac_next_block(io,header)) {
      if (FLAC_BLOCK_TYPE(header)==FLAC_BLOCK_PICTURE) {
        GMCover * cover = gm_flac_parse_block_picture(io);
        if (cover) covers.append(cover);
        }
      else if (!(header&FLAC_LAST_BLOCK)){
        io.position(FLAC_BLOCK_SIZE(header),FXIO::Current);
        }
      }
    }
  return covers.no();
  }





static GMCover * xiph_load_cover(const TagLib::ByteVector & tbuf) {
  GMCover * cover = NULL;
  if (tbuf.size()) {
    FXuchar * buffer=NULL;
    FXint   len=tbuf.size();

    allocElms(buffer,len);
    memcpy(buffer,tbuf.data(),len);
    if (gm_decode_base64(buffer,len)) {
      cover = gm_flac_parse_block_picture(buffer,len);
      }
    freeElms(buffer);
    }
  return cover;
  }


struct raw_pixels {
  FXColor * data;
  FXint     width;
  FXint     height;

  raw_pixels() : data(NULL) {}
  ~raw_pixels() {
    if (data) freeElms(data);
    }
  };


static FXbool gm_load_pixels(const void * data,FXuval size,const FXString & mime,raw_pixels & pixels) {
  FXint q;
  FXMemoryStream ms;  
#if FOXVERSION < FXVERSION(1,7,18)
  ms.open(FXStreamLoad,size,(FXuchar*)data);
#else
  ms.open(FXStreamLoad,(FXuchar*)data,size);
#endif
  if ((comparecase(mime,"image/jpg")==0) || (comparecase(mime,"image/jpeg")==0) || (comparecase(mime,"JPG")==0)) {
    if (fxloadJPG(ms,pixels.data,pixels.width,pixels.height,q)) return true;
    }
  else if (comparecase(mime,FXPNGImage::mimeType)==0) {
    if (fxloadPNG(ms,pixels.data,pixels.width,pixels.height)) return true;
    }
  else if ((comparecase(mime,"image/bmp")==0) || (comparecase(mime,"image/x-bmp")==0) ) {
    if (fxloadBMP(ms,pixels.data,pixels.width,pixels.height)) return true;
    }
  else if ((comparecase(mime,FXGIFImage::mimeType)==0)) {
    if (fxloadGIF(ms,pixels.data,pixels.width,pixels.height)) return true;
    }
  return false;
  }



GMCover::GMCover() : data(NULL),data_size(0),type(0) {
  }

GMCover::GMCover(const FXuchar *d,FXival sz,const FXString & mimetype,FXuint t,const FXString & label) : data(NULL),data_size(0),mime(mimetype),type(t),description(label) {
  data_size=sz;
  allocElms(data,data_size);
  memcpy(data,d,data_size);
  }

GMCover::~GMCover() {
  if (data) {
    freeElms(data);
    data=NULL;
    }
  }

void GMCover::save(const FXString & filename) {
  raw_pixels pix;

  FXString ext=FXPath::extension(filename);
  if (comparecase(ext,"jpg")==0 || comparecase(ext,"jpeg")==0) {
    if ((comparecase(mime,"image/jpg")==0) || (comparecase(mime,"image/jpeg")==0) || (comparecase(mime,"JPG")==0)) {
      FXFileStream store;
      if (store.open(filename,FXStreamSave)){
        store.save(data,data_size);
        }
      }
    else {
      if (gm_load_pixels(data,data_size,mime,pix)) {
        FXFileStream store;
        if (store.open(filename,FXStreamSave)){
          fxsaveJPG(store,pix.data,pix.width,pix.height,100);
          }
        }
      }
    }
  else if (comparecase(ext,"png")==0) {
    if (comparecase(mime,FXPNGImage::mimeType)==0) {
      FXFileStream store;
      if (store.open(filename,FXStreamSave)){
        store.save(data,data_size);
        }
      }
    else {
      if (gm_load_pixels(data,data_size,mime,pix)) {
        FXFileStream store;
        if (store.open(filename,FXStreamSave)){
          fxsavePNG(store,pix.data,pix.width,pix.height);
          }
        }
      }
    }
  else if (comparecase(ext,"bmp")==0) {
    if ((comparecase(mime,"image/bmp")==0) || (comparecase(mime,"image/x-bmp")==0) ) {
      FXFileStream store;
      if (store.open(filename,FXStreamSave)){
        store.save(data,data_size);
        }
      }
    else {
      if (gm_load_pixels(data,data_size,mime,pix)) {
        FXFileStream store;
        if (store.open(filename,FXStreamSave)){
          fxsaveBMP(store,pix.data,pix.width,pix.height);
          }
        }
      }
    }
  else if (comparecase(ext,"gif")==0) {
    if (comparecase(mime,FXGIFImage::mimeType)==0) {
      FXFileStream store;
      if (store.open(filename,FXStreamSave)){
        store.save(data,data_size);
        }
      }
    else {
      if (gm_load_pixels(data,data_size,mime,pix)) {
        FXFileStream store;
        if (store.open(filename,FXStreamSave)){
          fxsaveGIF(store,pix.data,pix.width,pix.height);
          }
        }
      }
    }
  }

FXint GMCover::fromTag(const FXString & mrl,GMCoverList & covers) {
  FXString extension = FXPath::extension(mrl);

  if (comparecase(extension,"flac")==0){
    flac_load_covers(mrl,covers);
    if (covers.no()) return (covers.no());
    }

  TagLib::FileRef file(mrl.text(),false);
  if (file.isNull() || !file.tag()) {
    return 0;
    }

  GMFileTag tags(file.file());
  FXIconSource src(FXApp::instance());
  if (tags.xiph) {
    if (tags.xiph->contains("METADATA_BLOCK_PICTURE")) {
      const TagLib::StringList & coverlist = tags.xiph->fieldListMap()["METADATA_BLOCK_PICTURE"];
      for(TagLib::StringList::ConstIterator it = coverlist.begin(); it != coverlist.end(); it++) {
        GMCover * cover = xiph_load_cover((*it).data(TagLib::String::UTF8));
        if (cover) covers.append(cover);
        }
      }
    }
  if (tags.id3v2) {
    TagLib::ID3v2::FrameList framelist = tags.id3v2->frameListMap()["APIC"];
    if(!framelist.isEmpty()){
      for(TagLib::ID3v2::FrameList::Iterator it = framelist.begin(); it != framelist.end(); it++) {
        TagLib::ID3v2::AttachedPictureFrame * frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
        GMCover * cover = id3v2_load_cover(frame);
        if (cover) covers.append(cover);
        }
      }
    }
#ifdef TAGLIB_HAVE_MP4
  else if (tags.mp4) {
    if (tags.mp4->itemListMap().contains("covr")) {
      TagLib::MP4::CoverArtList coverlist = tags.mp4->itemListMap()["covr"].toCoverArtList();
      for(TagLib::MP4::CoverArtList::Iterator it = coverlist.begin(); it != coverlist.end(); it++) {
        GMCover * cover = NULL;

        if (it->format()==TagLib::MP4::CoverArt::PNG)
          cover = new GMCover((const FXuchar*)it->data().data(),it->data().size(),FXPNGImage::fileExt);
        else if (it->format()==TagLib::MP4::CoverArt::JPEG)
          cover = new GMCover((const FXuchar*)it->data().data(),it->data().size(),FXJPGImage::fileExt);
        if (cover)
          covers.append(cover);
        }
      }
    }
#endif
  return covers.no();
  }



