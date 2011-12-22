#ifndef FOX_H
#define FOX_H

#include <new>
#include <fx.h>
#include <FXArray.h>
#include <FXTextCodec.h>
#define FOXVERSION ((FOX_LEVEL) + (FOX_MINOR*1000) + (FOX_MAJOR*100000))
#define FXVERSION(major,minor,release) ((release)+(minor*1000)+(major*100000))


#if FOXVERSION >= FXVERSION(1,7,12)
#define GMStringVal(str) FXString::value(str)
#define GMStringFormat  FXString::value
#define GMFloatVal(str) str.toFloat()
#define GMIntVal(str) str.toInt()
#define GMUIntVal(str) str.toUInt()

#else
#define GMStringVal(str) FXStringVal(str)
#define GMStringFormat  FXStringFormat
#define GMIntVal(str) FXIntVal(str)
#define GMUIntVal(str) FXUIntVal(str)
#endif

typedef FXArray<FXString> FXStringList;
#endif

