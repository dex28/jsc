//
// The JS shell
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../LibJS/JSconfig.h"
#include "../LibJS/JS.h"

struct ShowEventsHook : public JSEventHook
{
    int callback( int event )
    {
        char* event_name = "unknown";

        switch( event )
        {
            case JS_EVENT_OPERAND_COUNT:
                event_name = "operand count";
                break;

            case JS_EVENT_GARBAGE_COLLECT:
                event_name = "garbage collect";
                break;
            }

        fprintf( stderr, "[%s]\n", event_name );

        return 0;
        }
    };

int
main( int argc, char* argv [] )
{
    if ( argc < 2 )
        return 0;

    clock_t t1 = clock ();

    JSC_CompilerOptions co;
    /*
    co.optimize_bc_size = false;
    co.optimize_constant_folding = false;
    co.optimize_jumps = false;
    co.optimize_peephole = false;
    co.optimize_heavy = false;
    */

    int argv1len = strlen( argv[ 1 ] );
    if ( argv1len > 4
        && ( stricmp( argv[ 1 ] + argv1len - 4, ".jsp" ) == 0
        || stricmp( argv[ 1 ] + argv1len - 4, ".htm" ) == 0 )
        )
    {
        co.js_pages = true;
        }

    char jsc_filename[ 1024 ];
    char jas_filename[ 1024 ];
    strcpy( jsc_filename, argv[ 1 ] );
    strcpy( jas_filename, argv[ 1 ] );

    if ( argv1len > 4
        && ( stricmp( jsc_filename + argv1len - 4, ".htm" ) == 0
        || stricmp( jsc_filename + argv1len - 4, ".jsp" ) == 0 )
        )
    {
        jsc_filename[ argv1len - 4 ] = 0;
        jas_filename[ argv1len - 4 ] = 0;
        strcat( jsc_filename, ".jsc" );
        strcat( jas_filename, ".jas" );

        if ( ! JSC_Compile( co, argv[ 1 ], jsc_filename, jas_filename ) )
            return 0;
        }
    else if ( argv1len > 3
        && stricmp( jsc_filename + argv1len - 3, ".js" ) == 0
        )
    {
        jsc_filename[ argv1len - 3 ] = 0;
        jas_filename[ argv1len - 3 ] = 0;
        strcat( jsc_filename, ".jsc" );
        strcat( jas_filename, ".jas" );

        if ( ! JSC_Compile( co, argv[ 1 ], jsc_filename, jas_filename ) )
            return 0;
        }
    else
    {
        strcpy( jsc_filename, "Temp.jsc" );
        jas_filename[ 0 ] = 0;

        if ( ! JSC_Compile( co, argv[ 1 ], NULL, NULL ) )
            return 0;
        }

    if ( argc > 2 )
        return 0;

    clock_t t2 = clock ();

    JS_AllocateConnPool ();

    {
        clock_t tE1 = clock ();

        JSByteCode bc;

        bc.Load( jsc_filename );
        bc.Save( "dump.jsc" );
        //bc.Dump ();
        //return 0;

        clock_t tE2 = clock ();

        JSVirtualMachine vm;
        vm.options.verbose = 1;

        if ( ! vm.Create() )
        {
            fprintf( stderr, "Couldn't create interpreter\n" );
            exit( 1 );
            }

        fprintf( stderr, "STARTED: -----------------------------------------\n" );

        if ( ! vm.Execute( &bc ) )
        {
            fprintf( stderr, "%s\n", vm.error );
            //vm.StackTrace ();
            }

        fprintf( stderr, "FINISHED: ----------------------------------------\n" );

        clock_t tE3 = clock ();

        fprintf( stderr, "Elapsed C=%.3lf, L=%.3lf, E=%.3lf seconds\n",
            double( t2 - t1 ) / CLOCKS_PER_SEC,
            double( tE2 - tE1 ) / CLOCKS_PER_SEC,
            double( tE3 - tE2 ) / CLOCKS_PER_SEC
            );
        }

    JS_FreeConnPool ();

    jsAllocDumpBlocks ();

    clock_t t3 = clock ();
    fprintf( stderr, "Total %.3lf seconds\n", double( t3 - t2 ) / CLOCKS_PER_SEC );

    return 0;
    }
