#ifndef FOX_H
#define FOX_H

#include <new>
#include <fx.h>
#include <FXArray.h>
#define FOXVERSION ((FOX_LEVEL) + (FOX_MINOR*1000) + (FOX_MAJOR*100000))
#define FXVERSION(major,minor,release) ((release)+(minor*1000)+(major*100000))


#if FOXVERSION >= FXVERSION(1,7,12)
#define GMStringVal(str) FXString::value(str)
#define GMStringFormat  FXString::value
#define GMFloatVal(str) str.toFloat()
#else
#define GMStringVal(str) FXStringVal(str)
#define GMStringFormat  FXStringFormat
#endif

typedef FXArray<FXString> FXStringList;
#endif

