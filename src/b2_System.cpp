//
// The System class
//

#include "JS.h"

/*
    Static methods:

        chdir( string ) => boolean
        error( [ANY...] ) => undefined
        exit( value )
        getcwd( void ) => string
        getenv( string ) => string / undefined
        popen( COMMAND, MODE ) => file
        print( [ANY...] ) => undefined
        sleep( int ) => undefined
        strerror (errno) => string
        system (string) => integer
        usleep (int) => undefined

    Properties:                    type            mutable

        bits                       integer
        canonicalHost              string
        canonicalHostCPU           string
        canonicalHostVendor        string
        canonicalHostOS            string
        errno                      integer
        lineBreakSequence          string
        stderr                     file
        stdin                      file
        stdout                     file
*/

struct JSBuiltinInfo_System : public JSBuiltinInfo
{
    // Static Methods
    JSSymbol s_chdir;
    JSSymbol s_error;
    JSSymbol s_exit;
    JSSymbol s_getcwd;
    JSSymbol s_getenv;
    JSSymbol s_popen;
    JSSymbol s_print;
    JSSymbol s_sleep;
    JSSymbol s_strerror;
    JSSymbol s_system;
    JSSymbol s_usleep;

    // Properties
    JSSymbol s_bits;
    JSSymbol s_canonicalHost;
    JSSymbol s_canonicalHostCPU;
    JSSymbol s_canonicalHostVendor;
    JSSymbol s_canonicalHostOS;
    JSSymbol s_errno;
    JSSymbol s_lineBreakSequence;
    JSSymbol s_stderr;
    JSSymbol s_stdin;
    JSSymbol s_stdout;

    // System file handles
    JSVariant pstderr;
    JSVariant pstdin;
    JSVariant pstdout;

    JSBuiltinInfo_System( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnMark ( void* instance_context );
    };

JSBuiltinInfo_System:: JSBuiltinInfo_System( void )
    : JSBuiltinInfo( "System" )
{
    s_chdir                  = vm->Intern( "chdir" );
    s_error                  = vm->Intern( "error" );
    s_exit                   = vm->Intern( "exit" );
    s_getcwd                 = vm->Intern( "getcwd" );
    s_getenv                 = vm->Intern( "getenv" );
    s_popen                  = vm->Intern( "popen" );
    s_print                  = vm->Intern( "print" );
    s_sleep                  = vm->Intern( "sleep" );
    s_strerror               = vm->Intern( "strerror" );
    s_system                 = vm->Intern( "system" );
    s_usleep                 = vm->Intern( "usleep" );

    s_bits                   = vm->Intern( "bits" );
    s_canonicalHost          = vm->Intern( "canonicalHost" );
    s_canonicalHostCPU       = vm->Intern( "canonicalHostCPU" );
    s_canonicalHostVendor    = vm->Intern( "canonicalHostVendor" );
    s_canonicalHostOS        = vm->Intern( "canonicalHostOS" );
    s_errno                  = vm->Intern( "errno" );
    s_lineBreakSequence      = vm->Intern( "lineBreakSequence" );
    s_stderr                 = vm->Intern( "stderr" );
    s_stdin                  = vm->Intern( "stdin" );
    s_stdout                 = vm->Intern( "stdout" );

    // Enter system properties
    //
    vm->FileNew( pstderr, "stdout", vm->s_stderr, 1 );
    vm->FileNew( pstdin,  "stdin",  vm->s_stdin,  1 );
    vm->FileNew( pstdout, "stdout", vm->s_stdout, 1 );
    }

JSPropertyRC
JSBuiltinInfo_System:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    int secure_mode = vm->options.secure_builtin_system;

    // The default result
    //
    result_return.type = JS_UNDEFINED;

    //-------------------------------------------------------------------------
    if ( method == s_chdir )
    {
        if ( secure_mode )
            goto insecure_feature;;

        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        result_return.type= JS_BOOLEAN;

        PSTR cp = args[ 1 ].ToNewCString ();
        result_return.vboolean = ( chdir( cp ) == 0 );
        delete cp;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_exit )
    {
        if ( secure_mode )
            goto insecure_feature;;

        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        exit( args[ 1 ].vinteger ); // Exit

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_getcwd )
    {
        if ( secure_mode )
            goto insecure_feature;;

        if ( args->vinteger != 0 )
            goto argument_error;

        int size = _MAX_PATH;
        PSTR cp = NEW char[ size ];

        if ( getcwd( cp, size ) )
        {
            result_return.MakeString( vm, cp );
            }
        else
        {
            result_return.type = JS_BOOLEAN;
            result_return.vboolean = 0;
            }

        delete cp;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_getenv )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[1].type != JS_STRING )
            goto argument_type_error;

        PSTR cp = args[ 1 ].ToNewCString ();
        PSTR val = getenv( cp );
        delete cp;

        if ( val )
        {
            result_return.MakeString( vm, val );
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_popen )
    {
        if ( secure_mode )
            goto insecure_feature;;

        if ( args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING || args[ 2 ].type != JS_STRING
            || args[ 2 ].vstring->len > 10 )
            goto argument_type_error;

        PSTR cmd = args[ 1 ].ToNewCString ();
        PSTR mode = args[ 2 ].ToNewCString ();

        FILE* fp = popen( cmd, mode );

        int readp = strchr( mode, 'r' ) != NULL;
        JSIOStreamPipe* ios = NEW JSIOStreamPipe( fp, readp );

        if ( ! JSIOStream::IsValid( ios ) )
        {
            delete ios;
            ios = NULL;
            }

        if ( ios != NULL )
        {
            vm->FileNew( result_return, cmd, ios, 0 );
            }

        delete mode;
        delete cmd;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_print || method == s_error )
    {
        JSIOStream *stream;

        if ( method == s_print )
            stream = vm->s_stdout;
        else
            stream = vm->s_stderr;

        for ( int i = 1; i <= args->vinteger; i++ )
        {
            JSVariant result;
            args[ i ].ToString( vm, result );
            stream->Write( result.vstring->data, result.vstring->len );
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_sleep )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        sleep( args[ 1 ].vinteger );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_strerror )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        PSTR cp = strerror( args[ 1 ].vinteger );
        result_return.MakeString( vm, cp );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_system )
    {
        if ( secure_mode )
            goto insecure_feature;;

        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        result_return.type = JS_INTEGER;

        PSTR cp = args[ 1 ].ToNewCString ();
        result_return.vinteger = system( cp );
        delete cp;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.MakeStaticString( vm, "System" );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_usleep )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[1].type != JS_INTEGER )
            goto argument_type_error;

        usleep( args[ 1 ].vinteger );

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "System.%s(): illegal amout of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "System.%s(): illegal argument", vm->Symname( method ) );

insecure_feature:
    vm->RaiseError( "System.%s(): not allowed in secure mode", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_System:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    //-------------------------------------------------------------------------
    if (property == s_bits)
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;

#if SIZEOF_INT == 2
        node.vinteger = 16;
#elif SIZEOF_LONG == 4
        node.vinteger = 32;
#elif SIZEOF_LONG == 8
        node.vinteger = 64;
#else // Do not know
        node.vinteger = 0;
#endif

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_canonicalHost )
    {
        if ( set )
           goto immutable;

        node.MakeStaticString( vm, CANONICAL_HOST );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_canonicalHostCPU )
    {
        if ( set )
            goto immutable;

        node.MakeStaticString( vm, CANONICAL_HOST_CPU );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_canonicalHostVendor )
    {
        if ( set )
            goto immutable;

        node.MakeStaticString( vm, CANONICAL_HOST_VENDOR );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_canonicalHostOS )
    {
        if ( set )
            goto immutable;

        node.MakeStaticString( vm, CANONICAL_HOST_OS );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_errno )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = errno;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_lineBreakSequence )
    {
        if ( set )
            goto immutable;

        node.MakeStaticString( vm, JS_ENDLINE, JS_ENDLINE_LEN );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_stderr )
    {
        if ( set )
            goto immutable;

        node = pstderr;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_stdin )
    {
        if ( set )
            goto immutable;

        node = pstdin;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_stdout )
    {
        if ( set )
            goto immutable;

        node = pstdout;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

immutable:
    vm->RaiseError( "System.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_System:: OnMark
(
    void* instance_context
    )
{
    pstderr.Mark ();
    pstdin.Mark ();
    pstdout.Mark ();
    }

//
// The System class initialization entry
//

void
JSVirtualMachine:: BuiltinSystem( void )
{
    new(this) JSBuiltinInfo_System;
    }
