//
// Win32 specific functions.
//

#include "JS.h"

#define STRICT
#include <windows.h>

uint
sleep( uint seconds)
{
    Sleep( ULONG( seconds ) * 1000L );

    // Should count how many seconcs we actually slept and return it
    //
    return seconds;
    }


uint
usleep( uint useconds )
{
    Sleep( ULONG( useconds ) / 1000L );
    return 0;
    }

///////////////////////////////////////////////////////////////////////////////
// Directory handling.
//

DIR:: DIR( PCSTR name )
{
    szPath = NEW char[ strlen( name ) + 5 ];
    assert( szPath != NULL );

    strcpy( szPath, name );
    strcat( szPath, "\\*.*" );

    handle = -1;
    pos = 0;
    }

PSTR
DIR:: Read( void )
{
    if ( pos == 0 )
    {
        _finddata_t finddata;
        handle = _findfirst( szPath, &finddata );

        if ( handle < 0 ) // It was an empty directory
            return NULL;

        szDirName = finddata.name;
        }
    else
    {
        if ( handle < 0 ) // It was an empty directory
            return NULL;

        _finddata_t finddata;
        if ( _findnext( handle, &finddata ) < 0 )
            return NULL;

        szDirName = finddata.name;
        }

    pos++;

    return szDirName;
    }

int
DIR:: Close( void )
{
    _findclose( handle );

    delete szPath;
    delete this;

    return 0;
    }

void
DIR:: Rewind( void )
{
    _findclose( handle );

    pos = 0;
    handle = -1;
    }


void
DIR:: Seek( long offset )
{
    }

long
DIR:: Tell( void )
{
    return -1;
    }
