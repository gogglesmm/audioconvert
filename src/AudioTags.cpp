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
// Uncomment to compile with development taglib
//#define TAGLIB_TRUNK 1

#include <fox.h>

#include <math.h>
#include <errno.h>


#include <tag.h>
#ifndef TAGLIB_MAJOR_VERSION
#error "missing taglib_major_version"
#endif
#define MKVERSION(major,minor,release) ((release)+(minor*1000)+(major*1000000))
#define TAGLIB_VERSION MKVERSION(TAGLIB_MAJOR_VERSION,TAGLIB_MINOR_VERSION,TAGLIB_PATCH_VERSION)

#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
#define TAGLIB_HAVE_MP4 1
#endif

/// TagLib
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
#ifdef TAGLIB_HAVE_MP4
#include "mp4file.h"
#include "mp4tag.h"
#include "mp4coverart.h"
#endif


#include "AudioTags.h"

#include "FXPNGImage.h"
#include "FXJPGImage.h"

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


/// FlacPictureBlock structure.
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


namespace TagLib {
  class File;
  class Tag;
#ifdef TAGLIB_HAVE_MP4
  namespace MP4 {
    class Tag;
    }
#endif
  namespace Ogg {
    class XiphComment;
    }
  namespace ID3v2 {
    class Tag;
    }
  namespace APE {
    class Tag;
    }
  }


enum {
  FILETAG_TAGS            = 0x0, // Read TAGS from file
  FILETAG_AUDIOPROPERTIES = 0x1  // Determine audio properties from file
  };


class GMCover;
typedef FXArray<GMCover*> GMCoverList;

class GMFileTag {
protected:
  TagLib::File              * file;
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
  void id3v2_get_field(const FXchar * field,FXString &) const;
  void id3v2_get_field(const FXchar * field,FXStringList &) const;
  void id3v2_update_field(const FXchar * field,const FXString & value);
  void id3v2_update_field(const FXchar * field,const FXStringList & value);

  void xiph_get_field(const FXchar * field,FXString &) const;
  void xiph_get_field(const FXchar * field,FXStringList &) const;
  void xiph_update_field(const FXchar * field,const FXString & value);
  void xiph_update_field(const FXchar * field,const FXStringList & value);

  void mp4_get_field(const FXchar * field,FXString &) const;
  void mp4_get_field(const FXchar * field,FXStringList &) const;
  void mp4_update_field(const FXchar * field,const FXString & value);
  void mp4_update_field(const FXchar * field,const FXStringList & value);

  void ape_get_field(const FXchar * field,FXString &) const;
  void ape_get_field(const FXchar * field,FXStringList &) const;
  void ape_update_field(const FXchar * field,const FXStringList & value);

  void parse_tagids(FXStringList&) const;
public:
  GMFileTag();

  FXbool open(const FXString & filename,FXuint opts);
  FXbool save();

  void setComposer(const FXString & value);
  void getComposer(FXString &) const;

  void setConductor(const FXString & value);
  void getConductor(FXString &) const;

  void setAlbumArtist(const FXString & value);
  void getAlbumArtist(FXString &) const;

  void setArtist(const FXString &);
  void getArtist(FXString&) const;

  void setAlbum(const FXString &);
  void getAlbum(FXString&) const;

  void setTitle(const FXString &);
  void getTitle(FXString&) const;

  void setTags(const FXStringList & value);
  void getTags(FXStringList&) const;

  void setDiscNumber(FXushort disc);
  FXushort getDiscNumber() const;

  void setTrackNumber(FXushort no);
  FXushort getTrackNumber() const;

  void setYear(FXint);
  FXint getYear() const;

  FXint getTime() const;
  FXint getBitRate() const;
  FXint getChannels() const;
  FXint getSampleRate() const;

  GMCover * getFrontCover() const;
  FXint getCovers(GMCoverList &) const;

  ~GMFileTag();
  };



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

//// Parse FLAC picture block from buffer
static GMCover * xiph_parse_flac_picture_block(const FXuchar * buffer,FXint len) {
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
    return new GMCover(picture.data,picture.data_size,picture.type,picture.description);
    }
  return NULL;
  }


/// Load xiph cover
static GMCover * xiph_load_cover(const TagLib::ByteVector & tbuf) {
  GMCover * cover = NULL;
  if (tbuf.size()) {
    FXuchar * buffer=NULL;
    FXint   len=tbuf.size();

    allocElms(buffer,len);
    memcpy(buffer,tbuf.data(),len);
    if (gm_decode_base64(buffer,len)) {
      cover = xiph_parse_flac_picture_block(buffer,len);
      }
    freeElms(buffer);
    }
  return cover;
  }

static GMCover * id3v2_load_cover(TagLib::ID3v2::AttachedPictureFrame * frame) {
  FXString mime = frame->mimeType().toCString(true);
  /// Skip File Icon
  if (frame->type()==TagLib::ID3v2::AttachedPictureFrame::FileIcon ||
      frame->type()==TagLib::ID3v2::AttachedPictureFrame::OtherFileIcon ||
      frame->type()==TagLib::ID3v2::AttachedPictureFrame::ColouredFish) {
    return NULL;
    }
  return new GMCover(frame->picture().data(),frame->picture().size(),frame->type());
  }

static FXbool id3v2_is_front_cover(TagLib::ID3v2::AttachedPictureFrame * frame){
  if (frame->type()==TagLib::ID3v2::AttachedPictureFrame::FrontCover)
    return true;
  else
    return false;
  }



#if TAGLIB_VERSION >= MKVERSION(1,7,0)
GMCover* flac_load_cover_from_taglib(const TagLib::FLAC::Picture * picture) {
  if (picture) {
    if (picture->type()==TagLib::FLAC::Picture::FileIcon ||
        picture->type()==TagLib::FLAC::Picture::OtherFileIcon ||
        picture->type()==TagLib::FLAC::Picture::ColouredFish) {
        return NULL;
        }
    return new GMCover(picture->data().data(),picture->data().size(),picture->type(),picture->description().toCString(true));
    }
  return NULL;
  }

GMCover* flac_load_frontcover_from_taglib(const TagLib::FLAC::Picture * picture) {
  if (picture && picture->type()==TagLib::FLAC::Picture::FrontCover) {
    return new GMCover(picture->data().data(),picture->data().size(),picture->type(),picture->description().toCString(true));
    }
  return NULL;
  }
#endif


/******************************************************************************/

GMFileTag::GMFileTag() :
    file(NULL),
    tag(NULL),
    mp4(NULL),
    xiph(NULL),
    id3v2(NULL),
    ape(NULL) {
  /// TODO
  }

GMFileTag::~GMFileTag() {
  if (file) delete file;
  }


FXbool GMFileTag::open(const FXString & filename,FXuint opts) {

  file = TagLib::FileRef::create(filename.text(),(opts&FILETAG_AUDIOPROPERTIES));
  if (file==NULL || !file->isValid() || file->tag()==NULL) {
    if (file) {
      delete file;
      file=NULL;
      }
    return false;
    }

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
  return true;
  }

FXbool GMFileTag::save() {
  return file->save();
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


void  GMFileTag::xiph_get_field(const FXchar * field,FXString & value) const {
  FXASSERT(field);
  FXASSERT(xiph);
  if (xiph->contains(field))
    value=xiph->fieldListMap()[field].front().toCString(true);
  else
    value.clear();
  }

void GMFileTag::xiph_get_field(const FXchar * field,FXStringList & list)  const{
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

void GMFileTag::ape_get_field(const FXchar * field,FXString & value)  const{
  FXASSERT(field);
  FXASSERT(ape);
  if (ape->itemListMap().contains(field) && !ape->itemListMap()[field].isEmpty())
    value=ape->itemListMap()[field].toString().toCString(true);
  else
    value.clear();
  }

void GMFileTag::ape_get_field(const FXchar * field,FXStringList & list)  const{
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

void  GMFileTag::id3v2_get_field(const FXchar * field,FXString & value) const{
  FXASSERT(field);
  FXASSERT(id3v2);
  if (id3v2->frameListMap().contains(field) && !id3v2->frameListMap()[field].isEmpty() )
    value=id3v2->frameListMap()[field].front()->toString().toCString(true);
  else
    value.clear();
  }

void  GMFileTag::id3v2_get_field(const FXchar * field,FXStringList & list) const {
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


void GMFileTag::mp4_get_field(const FXchar * field,FXString & value) const {
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


void GMFileTag::mp4_get_field(const FXchar * field,FXStringList & list) const{
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

void GMFileTag::getComposer(FXString & composer) const{
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

void GMFileTag::getConductor(FXString & conductor) const{
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


void GMFileTag::getAlbumArtist(FXString & albumartist) const{
  if (xiph)
    xiph_get_field("ALBUMARTIST",albumartist);
  else if (id3v2)
    id3v2_get_field("TPE2",albumartist);
  else if (mp4)
    mp4_get_field("aART",albumartist);
  else
    albumartist.clear();

  if (albumartist.empty())
    getArtist(albumartist);
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

void GMFileTag::getTags(FXStringList & tags) const {
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


void GMFileTag::setArtist(const FXString & value){
  tag->setArtist(TagLib::String(value.text(),TagLib::String::UTF8));
  }

void GMFileTag::getArtist(FXString& value) const{
  value=tag->artist().toCString(true);
  value.trim();
  }


void GMFileTag::setAlbum(const FXString & value){
  tag->setAlbum(TagLib::String(value.text(),TagLib::String::UTF8));
  }
void GMFileTag::getAlbum(FXString& value) const{
  value=tag->album().toCString(true);
  value.trim();
  }

void GMFileTag::setTitle(const FXString & value){
  tag->setTitle(TagLib::String(value.text(),TagLib::String::UTF8));
  }

void GMFileTag::getTitle(FXString& value) const {
  if (xiph) {
    FXStringList vals;
    xiph_get_field("TITLE",vals);
    value.clear();
    if (vals.no()) {
      for (FXint i=0;i<vals.no();i++) {
        vals[i].trim();
        if (!value.empty()) value+=" - ";
        value+=vals[i];
        }
      }
    }
  else {
    value=tag->title().toCString(true);
    value.trim();
    }
  }

void GMFileTag::parse_tagids(FXStringList & tags)const{
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
  return FXMIN(GMUIntVal(disc.before('/')),0xFFFF);
  }

FXushort GMFileTag::getDiscNumber() const{
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

FXint GMFileTag::getTime() const{
  FXASSERT(file);
  TagLib::AudioProperties * properties = file->audioProperties();
  if (properties)
    return properties->length();
  else
    return 0;
  }

FXint GMFileTag::getBitRate() const{
  FXASSERT(file);
  TagLib::AudioProperties * properties = file->audioProperties();
  if (properties)
    return properties->bitrate();
  else
    return 0;
  }

FXint GMFileTag::getSampleRate() const{
  FXASSERT(file);
  TagLib::AudioProperties * properties = file->audioProperties();
  if (properties)
    return properties->sampleRate();
  else
    return 0;
  }


FXint GMFileTag::getChannels() const{
  FXASSERT(file);
  TagLib::AudioProperties * properties = file->audioProperties();
  if (properties)
    return properties->channels();
  else
    return 0;
  }


void GMFileTag::setTrackNumber(FXushort track) {
  tag->setTrack(track);
  }

FXushort GMFileTag::getTrackNumber() const{
  return FXMIN(tag->track(),0xFFFF);
  }

void GMFileTag::setYear(FXint year) {
  tag->setYear(year);
  }

FXint GMFileTag::getYear()const {
  return tag->year();
  }

GMCover * GMFileTag::getFrontCover() const {
#if TAGLIB_VERSION >= MKVERSION(1,7,0)
  TagLib::FLAC::File * flacfile = dynamic_cast<TagLib::FLAC::File*>(file);
  if (flacfile) {
    const TagLib::List<TagLib::FLAC::Picture*> picturelist = flacfile->pictureList();
    for(TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = picturelist.begin(); it != picturelist.end(); it++) {
      GMCover * cover = flac_load_frontcover_from_taglib((*it));
      if (cover) return cover;
      }
    }
#endif
  if (id3v2) {
    TagLib::ID3v2::FrameList framelist = id3v2->frameListMap()["APIC"];
    if(!framelist.isEmpty()){
      /// First Try Front Cover
      for(TagLib::ID3v2::FrameList::Iterator it = framelist.begin(); it != framelist.end(); it++) {
        TagLib::ID3v2::AttachedPictureFrame * frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
        FXASSERT(frame);
        if (id3v2_is_front_cover(frame)) {
          GMCover * cover = id3v2_load_cover(frame);
          if (cover) return cover;
          }
        }
      for(TagLib::ID3v2::FrameList::Iterator it = framelist.begin(); it != framelist.end(); it++) {
        TagLib::ID3v2::AttachedPictureFrame * frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
        FXASSERT(frame);
        GMCover * cover = id3v2_load_cover(frame);
        if (cover) return cover;
        }
      }
    }
  else if (xiph) {
    if (xiph->contains("METADATA_BLOCK_PICTURE")) {
      const TagLib::StringList & coverlist = xiph->fieldListMap()["METADATA_BLOCK_PICTURE"];
      for(TagLib::StringList::ConstIterator it = coverlist.begin(); it != coverlist.end(); it++) {
        GMCover * cover = xiph_load_cover((*it).data(TagLib::String::UTF8));
        if (cover) return cover;
        }
      }
    }
#ifdef TAGLIB_HAVE_MP4
  if (mp4) { /// MP4
    if (mp4->itemListMap().contains("covr")) {
      TagLib::MP4::CoverArtList coverlist = mp4->itemListMap()["covr"].toCoverArtList();
      for(TagLib::MP4::CoverArtList::Iterator it = coverlist.begin(); it != coverlist.end(); it++) {
        return new GMCover(it->data().data(),it->data().size());
        }
      }
    }
#endif
  return NULL;
  }

FXint GMFileTag::getCovers(GMCoverList & covers) const {
  TagLib::FLAC::File * flacfile = dynamic_cast<TagLib::FLAC::File*>(file);
  if (flacfile) {
    const TagLib::List<TagLib::FLAC::Picture*> picturelist = flacfile->pictureList();
    for(TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = picturelist.begin(); it != picturelist.end(); it++) {
      GMCover * cover = flac_load_cover_from_taglib((*it));
      if (cover) covers.append(cover);
      }
    if (covers.no()) return covers.no();
    }
  if (xiph) {
    if (xiph->contains("METADATA_BLOCK_PICTURE")) {
      const TagLib::StringList & coverlist = xiph->fieldListMap()["METADATA_BLOCK_PICTURE"];
      for(TagLib::StringList::ConstIterator it = coverlist.begin(); it != coverlist.end(); it++) {
        GMCover * cover = xiph_load_cover((*it).data(TagLib::String::UTF8));
        if (cover) covers.append(cover);
        }
      }
    }
  if (id3v2) {
    TagLib::ID3v2::FrameList framelist = id3v2->frameListMap()["APIC"];
    if(!framelist.isEmpty()){
      for(TagLib::ID3v2::FrameList::Iterator it = framelist.begin(); it != framelist.end(); it++) {
        TagLib::ID3v2::AttachedPictureFrame * frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
        GMCover * cover = id3v2_load_cover(frame);
        if (cover) covers.append(cover);
        }
      }
    }
#ifdef TAGLIB_HAVE_MP4
  else if (mp4) {
    if (mp4->itemListMap().contains("covr")) {
      TagLib::MP4::CoverArtList coverlist = mp4->itemListMap()["covr"].toCoverArtList();
      for(TagLib::MP4::CoverArtList::Iterator it = coverlist.begin(); it != coverlist.end(); it++) {
        covers.append(new GMCover(it->data().data(),it->data().size(),0));
        }
      }
    }
#endif
  return covers.no();
  }





/* GMCOVER */
GMCover::GMCover() : data(NULL),len(0), type(0) {
  }

GMCover::GMCover(const void * ptr,FXuval size,FXuint t,const FXString & label,FXbool owned) : data(NULL), len(size),description(label),type(t) {
  if (owned==false) {
    allocElms(data,len);
    memcpy(data,(const FXuchar*)ptr,len);
    }
  else {
    data=(FXuchar*)ptr;
    }
  }

GMCover::~GMCover() {
  freeElms(data);
  }


FXString GMCover::fileExtension() const{
  if (     data[0]==137 &&
           data[1]==80  &&
           data[2]==78  &&
           data[3]==71  &&
           data[4]==13  &&
           data[5]==10  &&
           data[6]==26  &&
           data[7]==10) {
    return ".png";
    }
  else if (data[0]==0xFF &&
           data[1]==0xD8){
    return ".jpg";
    }
  else if (data[0]=='B' &&
           data[1]=='M'){
    return ".bmp";
    }
  else if (data[0]==0x47 &&
           data[1]==0x49 &&
           data[2]==0x46){
    return ".gif";
    }
  return FXString::null;
  }

extern FXbool gm_make_path(const FXString & path,FXuint perm=FXIO::OwnerFull|FXIO::GroupFull|FXIO::OtherFull);


FXbool GMCover::save(const FXString & filename) {
  FXString path = FXPath::directory(filename);
  if (FXStat::exists(path) || gm_make_path(path)) {
    FXFile file (filename,FXIO::Writing);
    file.writeBlock(data,len);
    file.close();
    return true;
    }
  return false;
  }

FXint GMCover::fromTag(const FXString & mrl,GMCoverList & covers) {
  FXString extension = FXPath::extension(mrl);
  GMFileTag tags;
  if (!tags.open(mrl,FILETAG_TAGS)) {
    return 0;
    }
  tags.getCovers(covers);
  return covers.no();
  }



GMCover * GMCover::fromTag(const FXString & mrl) {
  FXString extension = FXPath::extension(mrl);
  GMFileTag tags;
  if (!tags.open(mrl,FILETAG_TAGS)) {
    return NULL;
    }
  return tags.getFrontCover();
  }

GMCover * GMCover::fromFile(const FXString & filename) {
  FXFile file(filename,FXIO::Reading);
  FXuval size = file.size();
  if (file.isOpen() && size) {
    FXchar * data=NULL;
    allocElms(data,size);
    if (file.readBlock(data,size)==size) {
      return new GMCover(data,size);
      }
    freeElms(data);
    }
  return NULL;
  }

GMCover * GMCover::fromPath(const FXString & path) {
  static const FXchar * covernames[]={"cover","album","front","albumart",".folder","folder",NULL};
  FXString * files=NULL;
  FXString * names=NULL;
  GMCover  * cover=NULL;

  FXint nfiles = FXDir::listFiles(files,path,"*.(png,jpg,jpeg,bmp,gif)",FXDir::NoDirs|FXDir::NoParent|FXDir::CaseFold|FXDir::HiddenFiles);
  if (nfiles) {
    names = new FXString[nfiles];
    for (FXint i=0;i<nfiles;i++)
      names[i]=FXPath::title(files[i]);

    for (FXint c=0;covernames[c]!=NULL;c++) {
      for (FXint i=0;i<nfiles;i++){
        if (comparecase(names[i],covernames[c])==0) {
          cover = GMCover::fromFile(path+PATHSEPSTRING+files[i]);
          if (cover) {
            delete [] names;
            delete [] files;
            return cover;
            }
          }
        }
      }

    delete [] names;
    /// No matching cover name found. Just load the first file.
    cover = GMCover::fromFile(path+PATHSEPSTRING+files[0]);
    delete [] files;
    }
  return cover;
  }


GMTrack::GMTrack() :
  year(0),
  no(0){
  }

void GMTrack::adopt(GMTrack & t) {
  title.adopt(t.title);
  artist.adopt(t.artist);
  album.adopt(t.album);
  album_artist.adopt(t.album_artist);
  composer.adopt(t.composer);
  conductor.adopt(t.conductor);
  tags.adopt(t.tags);
  year=t.year;
  no=t.no;
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
  }

FXbool GMTrack::saveTag(const FXString & filename,FXuint /*opts=0*/) {
  GMFileTag filetags;

  if (!FXStat::isWritable(filename))
    return false;

  if (!filetags.open(filename,FILETAG_TAGS))
    return false;

  filetags.setTitle(title);
  filetags.setArtist(artist);
  filetags.setAlbum(album);
  filetags.setYear(year);
  filetags.setTrackNumber(getTrackNumber());
  filetags.setDiscNumber(getDiscNumber());
  filetags.setComposer(composer);
  filetags.setConductor(conductor);
  filetags.setTags(tags);
  if (album_artist!=artist && !album_artist.empty())
    filetags.setAlbumArtist(album_artist);
  else
    filetags.setAlbumArtist(FXString::null);

  return filetags.save();
  }


FXbool GMTrack::loadTag(const FXString & filename) {
  GMFileTag filetags;

  if (!filetags.open(filename,FILETAG_TAGS|FILETAG_AUDIOPROPERTIES)){
    clear();
    return false;
    }

  filetags.getTitle(title);
  filetags.getAlbum(album);
  filetags.getArtist(artist);
  filetags.getAlbumArtist(album_artist);
  filetags.getComposer(composer);
  filetags.getConductor(conductor);
  filetags.getTags(tags);

  year    = filetags.getYear();
  no      = filetags.getTrackNumber();

  setDiscNumber(filetags.getDiscNumber());
  return true;
  }
