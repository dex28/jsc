#ifndef _JSCONFIG_H_INCLUDED
#define _JSCONFIG_H_INCLUDED

// Define if you have the ANSI C header files
//
#define STDC_HEADERS 1

// Version number
//
#define VERSION "99.04 (06)"

// Canonical host name and its parts
//
#define CANONICAL_HOST         "i386-PC-Win32"
#define CANONICAL_HOST_CPU     "i386"
#define CANONICAL_HOST_VENDOR  "PC"
#define CANONICAL_HOST_OS      "Win32"

// Do we want to profile byte-code operands
//
// #define PROFILING

// The number of bytes in a int
//
#define SIZEOF_INT 4

// The number of bytes in a long
//
#define SIZEOF_LONG 4

// For chmod() & _find{first,next,close}()
//
#include <io.h>

// For alloca()
//
#include <malloc.h>

// For chdir()
//
#include <direct.h>

// Win32 macros
//
#define popen(cmd,mode) _popen( cmd,mode )
#define pclose(fp) _pclose(fp)

// Prototypes for functions defined in Win32.cpp
//
extern unsigned int sleep( unsigned int seconds );
extern unsigned int usleep( unsigned int useconds );

#endif // _JSCONFIG_H_INCLUDED