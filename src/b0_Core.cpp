//
// Core global methods
//

#include "JS.h"

/*
    Global methods:

        parseInt( string[, radix] )
        parseFloat( string )
        escape( string )
        unescape( string )
        isNaN( any )
        isFinite( any )
        debug( any )
        error( string )
        float( any )
        int( any )
        isFloat( any )
        isInt( any )
        print( any[,...] )
        eval( string... )
        loadClass( string... )
        callMethod( string... )
*/

static void
gm_parseInt
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 && args->vinteger != 2 )
    {
        vm->RaiseError( "parseInt(): illegal amount of arguments" );
        }

    result_return.type = JS_INTEGER;

    PSTR cp;
    if ( args[ 1 ].type == JS_STRING )
    {
        cp = args[ 1 ].ToNewCString ();
        }
    else
    {
        // Convert the input to string
        //
        JSVariant input;
        args[ 1 ].ToString( vm, input );
        cp = input.ToNewCString ();
        }

    JSInt32 base = 0;

    if ( args->vinteger == 2 )
    {
        if (args[ 2 ].type == JS_INTEGER )
            base = args[ 2 ].vinteger;
        else
            base = args[ 2 ].ToInt32 ();
        }

    PSTR end;
    result_return.vinteger = strtol( cp, &end, base );
    delete cp;

    if ( cp == end )
        result_return.type = JS_NAN;
    }

static void
gm_parseFloat
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "parseFloat(): illegal amount of arguments" );
        }

    result_return.type = JS_FLOAT;

    PSTR cp;
    if ( args[ 1 ].type == JS_STRING )
    {
        cp = args[ 1 ].ToNewCString ();
        }
    else
    {
        // Convert the input to string
        //
        JSVariant input;
        args[ 1 ].ToString( vm, input );
        cp = input.ToNewCString ();
        }

    PSTR end;
    result_return.vfloat = strtod( cp, &end );
    delete cp;

    if ( cp == end ) // Couldn't parse, return NaN
        result_return.type = JS_NAN;
    }

static void
gm_escape
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "escape(): illegal amount of arguments" );
        }

    JSVariant source = args[ 1 ];

    if ( source.type != JS_STRING )
    {
        // Convert the argument to string
        //
        args[ 1 ].ToString( vm, source );
        }

    // Allocate the result string, Let's guess that we need at least
    // <source.vstring->len> bytes of data.
    //
    int n = source.vstring->len;
    PSTR dp = source.vstring->data;
    result_return.MakeString( vm, NULL, n );
    result_return.vstring->len = 0;

    //
    // Scan for characters requiring escapes.
    //
    for ( int i = 0; i < n; i ++ )
    {
        int c = dp[ i ];

        static PCSTR szNotESC = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@*_+-./";

        if ( strchr ( szNotESC, c ) )
        {
            char buf[ 1 ] = { c };
            vm->StrCat( result_return, buf, 1 );
            }
        else if ( c > 0xFF )
        {
            char buf[ 16 ];
            sprintf( buf, "%%u%04x", c );
            vm->StrCat( result_return, buf, 6 );
            }
        else
        {
            char buf[ 16 ];
            sprintf( buf, "%%%02x", c );
            vm->StrCat( result_return, buf, 3 );
            }
        }
    }

// A helper function for unescape()
//
static int
scanhexdigits( PCSTR dp, int nd, int& cp )
{
    static PCSTR szDigits = "0123456789abcdefABCDEF";

    cp = 0;
    for ( int i = 0; i < nd; i ++ )
    {
        PCSTR szDigit = strchr( szDigits, dp[ i ] );
        if ( ! szDigit )
            return 0;

        int d = szDigit - szDigits;
        if ( d >= 16 )
            d -= 6;

        cp <<= 4;
        cp += d;
        }

    return 1;
    }

static void
gm_unescape
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "unescape(): illegal amount of arguments" );
        }

    JSVariant source = args[ 1 ];

    if ( source.type != JS_STRING )
    {
        args[ 1 ].ToString( vm, source );
        }

    // Allocate the result string, Let's guess that we need at least
    // <source.vstring->len> bytes of data.
    //
    int n = source.vstring->len;
    PSTR dp = source.vstring->data;
    result_return.MakeString( vm, NULL, n );
    result_return.vstring->len = 0;

    //
    // Scan for escapes requiring characters.
    //
    for ( int i = 0; i < n; )
    {
        int c = dp[ i ];

        if ( c != '%' )
            i ++;
        else if ( i <= n - 6 && dp[ i + 1 ] == 'u' && scanhexdigits( dp + i + 2, 4, c ) )
            i += 6;
        else if ( i <= n - 3 && scanhexdigits( dp + i + 1, 2, c ) )
            i += 3;
        else
        {
            c = dp[ i ];
            i ++;
            }

        char buf[ 1 ] = { c };
        vm->StrCat( result_return, buf, 1 );
        }
    }

static void
gm_isNaN
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "isNaN(): illegal amount of arguments" );
        }

    int result;

    switch ( args[ 1 ].type )
    {
        case JS_NAN:
            result = 1;
            break;

        case JS_INTEGER:
        case JS_FLOAT:
            result = 0;
            break;

        default:
        {
            JSVariant cvt;
            args[ 1 ].ToNumber( cvt );
            result = ( cvt.type == JS_NAN );
            }
        }

    result_return.type = JS_BOOLEAN;
    result_return.vboolean = result;
    }

static void
gm_isFinite
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "isFinite(): illegal amount of arguments" );
        }

    JSVariant source = args[ 1 ];

    if ( ! source.IsNumber () )
    {
        args[ 1 ].ToNumber( source );
        }

    int result = 0;

    switch ( source.type )
    {
        case JS_NAN:
            result = 0;
            break;

        case JS_INTEGER:
            result = 1;
            break;

        case JS_FLOAT:
            result = ! source.IsPositiveInfinity ()
                   && ! source.IsNegativeInfinity ();
            break;

        default:
            assert( 1 );
            // NOTREACHED
        }

    result_return.type = JS_BOOLEAN;
    result_return.vboolean = result;
    }

static void
gm_debug
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "debug(): illegal amount of arguments" );
        }

    // Maybe we should prefix the debug message with
    // `Debug message:' prompt?
    //
    JSVariant sitem;
    args[ 1 ].ToString( vm, sitem );
    fwrite( sitem.vstring->data, 1, sitem.vstring->len, stderr );

    result_return.type = JS_UNDEFINED;
    }

static void
gm_error
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "error(): illegal amount of arguments" );
        }

    if ( args[ 1 ].type != JS_STRING )
    {
        vm->RaiseError( "error(): illegal argument" );
        }

    int len = args[ 1 ].vstring->len;

    if ( len > sizeof( vm->error ) - 1 )
        len = sizeof( vm->error ) - 1;

    memcpy( vm->error, args[ 1 ].vstring->data, len );
    vm->error[ len ] = '\0';

    // Here we go...
    //
    vm->RaiseError ();

    assert( 1 );
    // NOTREACHED
    }

static void
gm_float
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "float(): illegal amount of arguments" );
        }

    double fval = 0.0;

    switch ( args[ 1 ].type )
    {
        case JS_BOOLEAN:
            fval = double( args[ 1 ].vboolean != 0 );
            break;

        case JS_INTEGER:
            fval = double( args[ 1 ].vinteger );
            break;

        case JS_STRING:
        {
            PSTR cp = args[ 1 ].ToNewCString ();
            PSTR end;
            fval = strtod( cp, &end );
            delete cp;

            if ( cp == end )
                fval = 0.0;
            }
            break;

        case JS_FLOAT:
            fval = args[ 1 ].vfloat;
            break;

        case JS_ARRAY:
            fval = double( args[ 1 ].varray->length );
            break;
        }

    result_return.type = JS_FLOAT;
    result_return.vfloat = fval;
    }

static void
gm_int
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if (args->vinteger != 1)
    {
        vm->RaiseError( "int(): illegal amount of arguments" );
        }

    long i = 0;

    switch ( args[ 1 ].type )
    {
        case JS_BOOLEAN:
            i = long( args[ 1 ].vboolean != 0 );
            break;

        case JS_INTEGER:
            i = args[ 1 ].vinteger;
            break;

        case JS_STRING:
        {
            PSTR cp = args[ 1 ].ToNewCString ();
            PSTR end;
            i = strtol( cp, &end, 0 );
            delete cp;

            if ( cp == end )
                i = 0;
            }
            break;

        case JS_FLOAT:
            i = long( args[ 1 ].vfloat );
            break;

        case JS_ARRAY:
            i = long( args[ 1 ].varray->length );
            break;
        }

    result_return.type = JS_INTEGER;
    result_return.vinteger = i;
    }

static void
gm_isFloat
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{

    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "isFloat(): illegal amount of arguments" );
        }

    // The default result is false
    //
    result_return.type = JS_BOOLEAN;
    result_return.vboolean = 0;

    if ( args[ 1 ].type == JS_FLOAT )
        result_return.vboolean = 1;
    }


static void
gm_isInt
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "isInt(): illegal amount of arguments" );
        }

    // The default result is false
    //
    result_return.type = JS_BOOLEAN;
    result_return.vboolean = 0;

    if ( args[ 1 ].type == JS_INTEGER )
        result_return.vboolean = 1;
    }

static void
gm_print
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    // The result is undefined
    //
    result_return.type = JS_UNDEFINED;

    for ( int i = 1; i <= args->vinteger; i++ )
    {
        JSVariant result;

        args[ i ].ToString( vm, result );
        vm->s_stdout->Write( result.vstring->data, result.vstring->len );

        if ( i + 1 <= args->vinteger )
        {
            vm->s_stdout->Write( " ", 1 );
            }
        }

    vm->s_stdout->Write( JS_ENDLINE, JS_ENDLINE_LEN );
    }

static void
gm_htmlOut
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    // The result is undefined
    //
    result_return.type = JS_UNDEFINED;

    for ( int i = 1; i <= args->vinteger; i++ )
    {
        JSVariant result;

        args[ i ].ToString( vm, result );
        vm->s_stdout->Write( result.vstring->data, result.vstring->len );
        }
    }

static void
gm_eval
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "eval(): illegal amount of arguments" );
        }

    if ( args[1].type != JS_STRING )
    {
        // Return it to the caller
        //
        result_return = args[ 1 ];
        return;
        }

    // TODO implementation

    // Pass the return value to our caller
    //
    result_return = vm->exec_result;
    }

static void
gm_loadClass
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger == 0 )
    {
        vm->RaiseError( "loadClass(): no arguments given" );
        }

    for ( int i = 1; i <= args->vinteger; i++ )
    {
        if ( args[ i ].type != JS_STRING )
        {
            vm->RaiseError( "loadClass(): illegal argument" );
            }

        PSTR cp = args[ i ].ToNewCString ();

        // Extract the function name
        //
        PSTR szFuncName = strrchr (cp, ':' );
        if ( szFuncName == NULL )
        {
            szFuncName = strrchr( cp, '/' );

            if ( szFuncName == NULL )
                szFuncName = cp;
            else
                szFuncName++;
            }
        else
        {
            *szFuncName = '\0';
            szFuncName ++;
            }

        char szMsg[ 512 ]; // error message holder for JSDynaLib functions

        // Try to open the library
        //
        JSDynaLib lib( cp, szMsg, sizeof( szMsg ) );
        if ( ! lib )
        {
            vm->RaiseError( "loadClass(): couldn't open library `%s': %s", cp, szMsg );
            }

        // Strip all suffixes from the library name: if the <szFuncName>
        // is extracted from it, this will convert the library name
        // `foo.so.x.y' or 'foo.dll' to the canonical entry point name `foo'.
        //
        PSTR cp2 = strchr( cp, '.' );
        if ( cp2 )
            *cp2 = '\0';

        void (*fooInit)(JSVirtualMachine*)
            = (void(*)(JSVirtualMachine*)) lib.GetSymbol( szFuncName, szMsg, sizeof( szMsg ) );

        if ( fooInit == NULL )
        {
            vm->RaiseError( "loadClass(): couldn't find the init function `%s': %s",
                   szFuncName, szMsg );
            }

        // All done with this argument
        //
        delete cp;

        // And finally, call the library entry point.  All possible errors
        // will throw us to the containing Top-level error handler.
        //
        fooInit( vm );
        }

    result_return.type = JS_UNDEFINED;
    }

static void
gm_callMethod
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 3 )
    {
        vm->RaiseError( "callMethod(): illegal amount of arguments" );
        }

    if ( args[ 2 ].type != JS_STRING )
    {
        illegal_argument:
        vm->RaiseError( "callMethod(): illegal argument");
        }

    if ( args[ 3 ].type != JS_ARRAY )
        goto illegal_argument;

    // Create the argument array
    //
    JSVariant* argv = NEW JSVariant[ args[ 3 ].varray->length + 1 ];

    // The argument count
    //
    argv[ 0 ].type = JS_INTEGER;
    argv[ 0 ].vinteger = args[3].varray->length;

    for ( int i = 0; i < args[3].varray->length; i++ )
        argv[ i + 1 ] = args[ 3 ].varray->data[ i ];

    // Method name to C string
    //
    PSTR cp = args[ 2 ].ToNewCString ();

    // Call it
    //
    int result = vm->CallMethod( &args[ 1 ], cp, args[ 3 ].varray->length + 1, argv );

    // Cleanup
    //
    delete cp;
    delete argv;

    if ( result )
        result_return = vm->exec_result;
    else
        vm->RaiseError (); // The error message is already there
    }

//
// Core global methods initialization entry
//

void
JSVirtualMachine:: BuiltinCoreGM( void )
{
    // Properties
    //
    Intern( "NaN" )->type = JS_NAN;
    Intern( "Infinity" )->MakePositiveInfinity ();

    // Global methods
    //
    new(this) JSBuiltinInfo_GM( "parseInt",   gm_parseInt );
    new(this) JSBuiltinInfo_GM( "parseFloat", gm_parseFloat );
    new(this) JSBuiltinInfo_GM( "escape",     gm_escape );
    new(this) JSBuiltinInfo_GM( "unescape",   gm_unescape );
    new(this) JSBuiltinInfo_GM( "isNaN",      gm_isNaN );
    new(this) JSBuiltinInfo_GM( "isFinite",   gm_isFinite );
    new(this) JSBuiltinInfo_GM( "debug",      gm_debug );
    new(this) JSBuiltinInfo_GM( "error",      gm_error );
    new(this) JSBuiltinInfo_GM( "float",      gm_float );
    new(this) JSBuiltinInfo_GM( "int",        gm_int );
    new(this) JSBuiltinInfo_GM( "isFloat",    gm_isFloat );
    new(this) JSBuiltinInfo_GM( "isInt" ,     gm_isInt );
    new(this) JSBuiltinInfo_GM( "print",      gm_print );
    new(this) JSBuiltinInfo_GM( "htmlOut",    gm_htmlOut );

    new(this) JSBuiltinInfo_GM( "eval", gm_eval );
    new(this) JSBuiltinInfo_GM( "loadClass",  gm_loadClass );
    new(this) JSBuiltinInfo_GM( "callMethod", gm_callMethod );
    }
