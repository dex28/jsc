//
// Dummy dynamic loading functions for systems that do not support dload.
//

#include "JS.h"

#include <windows.h>

JSDynaLib:: JSDynaLib( PCSTR szFilename, PSTR error_return, int error_return_len )
{
    context = NULL;

    HINSTANCE hLib = GetModuleHandle( szFilename );

    if ( hLib == NULL )
    {
        hLib = LoadLibrary( szFilename );
        printf( "Loading library: %s\n", szFilename );
        }
    else
    {
        printf( "Referencing library: %s\n", szFilename );
        }

    if ( hLib == NULL )
    {
        DWORD retCode = GetLastError ();

        FormatMessage
        (
            FORMAT_MESSAGE_FROM_SYSTEM,    // source and processing options
            NULL,                          // pointer to  message source
            retCode,                       // requested message identifier
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // language identifier for requested message
            error_return,                  // pointer to message buffer
            error_return_len - 1,          // maximum size of message buffer
            NULL                           // address of array of message inserts
            );

        if ( ! error_return )
        {
            sprintf( error_return, "Return Code =", retCode );
            }
        }

    context = hLib;
    }

void*
JSDynaLib:: GetSymbol( PCSTR szProcName, PSTR error_return, int error_return_len )
{
    if ( ! context )
    {
        strcpy( error_return, "JSDynaLib:: not open." );
        return NULL;
        }

    void* hProc = GetProcAddress( HINSTANCE( context ), szProcName );

    if ( ! hProc )
    {
        DWORD retCode = GetLastError ();

        FormatMessage
        (
            FORMAT_MESSAGE_FROM_SYSTEM,    // source and processing options
            NULL,                          // pointer to  message source
            retCode,                       // requested message identifier
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // language identifier for requested message
            error_return,                  // pointer to message buffer
            error_return_len - 1,          // maximum size of message buffer
            NULL                           // address of array of message inserts
            );

        if ( ! error_return )
        {
            sprintf( error_return, "Return code =", retCode );
            }
        }

    return hProc;
    }
