//
// The Date class
//

#include "JS.h"

// FIXME: TODO 15.13.3

static PCSTR GMT_DATE_FORMAT = "%a, %d %b %Y %H:%M:%S GMT";

struct JSBuiltinInfo_Date : public JSBuiltinInfo
{
    // Static methods
    JSSymbol s_parse;

    // Methods
    JSSymbol s_format;
    JSSymbol s_formatGMT;
    JSSymbol s_getDate;
    JSSymbol s_getDay;
    JSSymbol s_getHours;
    JSSymbol s_getMinutes;
    JSSymbol s_getMonth;
    JSSymbol s_getSeconds;
    JSSymbol s_getTime;
    JSSymbol s_getTimezoneOffset;
    JSSymbol s_getYear;
    JSSymbol s_setDate;
    JSSymbol s_setHours;
    JSSymbol s_setMinutes;
    JSSymbol s_setMonth;
    JSSymbol s_setSeconds;
    JSSymbol s_setTime;
    JSSymbol s_setYear;
    JSSymbol s_toGMTString;
    JSSymbol s_toLocaleString;
    JSSymbol s_UTC;

    JSBuiltinInfo_Date( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );
    };

// Date instance context
//
struct DateInstanceCtx
{
    time_t secs;
    struct tm localtime;
    };

JSBuiltinInfo_Date:: JSBuiltinInfo_Date( void )
    : JSBuiltinInfo( "Date" )
{
    s_format                 = vm->Intern( "format" );
    s_formatGMT              = vm->Intern( "formatGMT" );
    s_getDate                = vm->Intern( "getDate" );
    s_getDay                 = vm->Intern( "getDay" );
    s_getHours               = vm->Intern( "getHours" );
    s_getMinutes             = vm->Intern( "getMinutes" );
    s_getMonth               = vm->Intern( "getMonth" );
    s_getSeconds             = vm->Intern( "getSeconds" );
    s_getTime                = vm->Intern( "getTime" );
    s_getTimezoneOffset      = vm->Intern( "getTimezoneOffset" );
    s_getYear                = vm->Intern( "getYear" );
    s_parse                  = vm->Intern( "parse" );
    s_setDate                = vm->Intern( "setDate" );
    s_setHours               = vm->Intern( "setHours" );
    s_setMinutes             = vm->Intern( "setMinutes" );
    s_setMonth               = vm->Intern( "setMonth" );
    s_setSeconds             = vm->Intern( "setSeconds" );
    s_setTime                = vm->Intern( "setTime" );
    s_setYear                = vm->Intern( "setYear" );
    s_toGMTString            = vm->Intern( "toGMTString" );
    s_toLocaleString         = vm->Intern( "toLocaleString" );
    s_UTC                    = vm->Intern( "UTC" );
    }

void
JSBuiltinInfo_Date:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger > 7 )
    {
        vm->RaiseError( "Date(): illegal amount of arguments" );
        }

    //
    // We ignore our arguments and return the result of:
    // `new Date ().toString ()'.
    //

    time_t secs = time( NULL );
    struct tm* tm = localtime( &secs );
    PSTR buf = asctime( tm );

    PSTR cp = strchr( buf, '\n' );
    if ( cp )
        *cp = '\0';

    result_return.MakeString( vm, buf );
    }

JSPropertyRC
JSBuiltinInfo_Date:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    DateInstanceCtx* ictx = (DateInstanceCtx*)instance_context;

    // The default return type is integer
    //
    result_return.type = JS_INTEGER;

    // Static methods
    //
    //-------------------------------------------------------------------------
    if ( method == s_parse )
    {
        goto not_implemented_yet;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx )
            goto date_to_string; // toString method !!!
        else
            result_return.MakeStaticString( vm, "Date" );

        return JS_PROPERTY_FOUND;
        }

    // Methods
    //
    if ( ! ictx )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //-------------------------------------------------------------------------
    if ( method == s_format || method == s_formatGMT )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        PSTR fmt = args[ 1 ].ToNewCString ();

        int buflen = args[ 1 ].vstring->len * 2 + 1;
        PSTR buf = NEW char[ buflen ];

        struct tm *tm = &ictx->localtime;

        if ( method == s_formatGMT )
        {
            tm = gmtime( &ictx->secs );
            }

        if ( args[ 1 ].vstring->len == 0 )
        {
            buf[ 0 ] = '\0';
            }
        else
        {
            while ( strftime( buf, buflen, fmt, tm ) == 0 )
            {
                // Expand the buffer
                //
                buflen *= 2;
                buf = (PSTR)jsRealloc( buf, buflen );
                }
            }

        result_return.MakeString( vm, buf );

        delete fmt;
        delete buf;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getDate )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_mday;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getDay )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_wday;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getHours )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_hour;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getMinutes )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_min;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getMonth )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_mon;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getSeconds )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_sec;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getTime )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_FLOAT;
        result_return.vfloat = double( ictx->secs ) * 1000.0;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getTimezoneOffset )
    {
        goto not_implemented_yet;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getYear )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vinteger = ictx->localtime.tm_year;
        if ( ictx->localtime.tm_year >= 100
            || ictx->localtime.tm_year < 0 )
            result_return.vinteger += 1900;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setDate )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if (args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( 1 <= args[ 1 ].vinteger && args[ 1 ].vinteger <= 31 )
        {
            ictx->localtime.tm_mday = args[ 1 ].vinteger;
            ictx->secs = mktime( &ictx->localtime );
            }
        else
        {
            goto argument_range_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setHours )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( 0 <= args[ 1 ].vinteger && args[ 1 ].vinteger <= 23 )
        {
            ictx->localtime.tm_hour = args[ 1 ].vinteger;
            ictx->secs = mktime( &ictx->localtime );
            }
        else
        {
            goto argument_range_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setMinutes )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
        goto argument_type_error;

        if ( 0 <= args[ 1 ].vinteger && args[ 1 ].vinteger <= 59 )
        {
            ictx->localtime.tm_min = args[ 1 ].vinteger;
            ictx->secs = mktime( &ictx->localtime );
            }
        else
        {
            goto argument_range_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setMonth )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( 0 <= args[ 1 ].vinteger && args[ 1 ].vinteger <= 11 )
        {
            ictx->localtime.tm_mon = args[ 1 ].vinteger;
            ictx->secs = mktime( &ictx->localtime );
            }
        else
        {
            goto argument_range_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setSeconds )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( 0 <= args[ 1 ].vinteger && args[ 1 ].vinteger <= 59 )
        {
            ictx->localtime.tm_sec = args[ 1 ].vinteger;
            ictx->secs = mktime( &ictx->localtime );
            }
        else
        {
            goto argument_range_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setTime )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type == JS_INTEGER )
            ictx->secs = args[ 1 ].vinteger / 1000;
        else if (args[1].type == JS_FLOAT)
            ictx->secs = long( args[ 1 ].vfloat / 1000 );
        else
            goto argument_type_error;

        memcpy( &ictx->localtime, localtime( &ictx->secs ), sizeof( struct tm ) );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setYear )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        ictx->localtime.tm_year = args[ 1 ].vinteger;
        if ( args[ 1 ].vinteger < 0 || args[1].vinteger >= 100 )
            ictx->localtime.tm_year -= 1900;

        ictx->secs = mktime( &ictx->localtime );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_toGMTString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        struct tm* tm = gmtime( &ictx->secs );

        char buf[ 1024 ]; // This is enought
        strftime( buf, sizeof( buf ), GMT_DATE_FORMAT, tm );

        result_return.MakeString( vm, buf );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_toLocaleString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        date_to_string:

        PCSTR buf = asctime( &ictx->localtime );
        PSTR cp = strchr( buf, '\n' );
        if ( cp )
            *cp = '\0';

        result_return.MakeString( vm, buf );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_UTC )
    {
        goto not_implemented_yet;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

not_implemented_yet:
    vm->RaiseError( "Date.%s(): not implemented yet", vm->Symname( method ) );

argument_error:
    vm->RaiseError( "Date.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "Date.%s(): illegal argument", vm->Symname( method ) );

argument_range_error:
    vm->RaiseError( "Date.%s(): argument out of range", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_Date:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;
    }

void
JSBuiltinInfo_Date:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    DateInstanceCtx *instance;

    instance = NEW DateInstanceCtx;
    // FIXME: ZeroMemory instance

    if ( args->vinteger == 0 )
    {
        instance->secs = time (NULL);
        memcpy( &instance->localtime, localtime( &instance->secs ), sizeof( struct tm ) );
        }
    else if ( args->vinteger == 1 )
    {
        goto not_implemented_yet;
        }
    else if ( args->vinteger == 3 || args->vinteger == 6 )
    {
        for ( int i = 0; i < args->vinteger; i++ )
        {
            if ( args[ i + 1 ].type != JS_INTEGER )
                goto argument_type_error;
            }

        // Year
        //
        instance->localtime.tm_year = args[ 1 ].vinteger;
        if ( args[ 1 ].vinteger < 0 || args[ 1 ].vinteger >= 100 )
            instance->localtime.tm_year -= 1900;

        // Month
        //
        if ( 0 <= args[ 2 ].vinteger && args[ 2 ].vinteger <= 11 )
            instance->localtime.tm_mon = args[ 2 ].vinteger;
        else
            goto argument_range_error;

        // Day
        //
        if ( 1 <= args[ 3 ].vinteger && args[ 3 ].vinteger <= 31 )
            instance->localtime.tm_mday = args[ 3 ].vinteger;
        else
            goto argument_range_error;

        if ( args->vinteger == 6 )
        {
            // Sync the localtime according to year, month and day
            //
            mktime( &instance->localtime );

            // Hours
            //
            if ( 0 <= args[ 4 ].vinteger && args[ 4 ].vinteger <= 23 )
                instance->localtime.tm_hour = args[ 4 ].vinteger;
            else
                goto argument_range_error;

            // Minutes
            //
            if ( 0 <= args[ 5 ].vinteger && args[ 5 ].vinteger <= 59 )
                instance->localtime.tm_min = args[ 5 ].vinteger;
            else
                goto argument_range_error;

            // Seconds
            //
            if ( 0 <= args[ 6 ].vinteger && args[ 6 ].vinteger <= 59 )
                instance->localtime.tm_sec = args[ 6 ].vinteger;
            else
                goto argument_range_error;
            }

        instance->secs = mktime (&instance->localtime);
        }
    else
    {
        delete instance;

        vm->RaiseError( "new Date(): illegal amount of arguments" );
        }

    result_return = new(vm) JSBuiltin( this, instance );

    return;

    //
    // Error handling.
    //

not_implemented_yet:
    vm->RaiseError(  "new Date( %ld args ): not implemented yet", args->vinteger );

argument_type_error:
    vm->RaiseError( "new Date(): illegal argument" );

argument_range_error:
    vm->RaiseError( "new Date(): argument out of range" );

    assert( 1 );
    // NOTREACHED
    }

void
JSBuiltinInfo_Date:: OnFinalize
(
    void* instance_context
    )
{
    DateInstanceCtx *ictx = (DateInstanceCtx*)instance_context;

    if ( ictx )
        delete ictx;
    }

//
// Global methods. -------------------------------------------------------
//

const long MS_PER_SECOND = 1000L;
const long MS_PER_MINUTE = 60L * MS_PER_SECOND;
const long MS_PER_HOUR   = 60L * MS_PER_MINUTE;
const long MS_PER_DAY    = 24L * MS_PER_HOUR;

static void
jsGM_MakeTime
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 4 )
    {
        vm->RaiseError( "MakeTime: illegal amount of argument" );
        }

    if ( ! args[ 1 ].IsNumber () || ! args[ 2 ].IsNumber ()
        || ! args[ 3 ].IsNumber () || ! args[ 4 ].IsNumber () )
    {
        vm->RaiseError( "MakeTime: illegal argument" );
        }

    if ( ! args[ 1 ].IsFinite () || ! args[ 2 ].IsFinite ()
        || ! args[ 3 ].IsFinite () || ! args[ 4 ].IsFinite () )
    {
        result_return.type = JS_NAN;
        return;
        }

    JSInt32 hour = args[ 1 ].ToInt32 ();
    JSInt32 min  = args[ 2 ].ToInt32 ();
    JSInt32 sec  = args[ 3 ].ToInt32 ();
    JSInt32 ms   = args[ 4 ].ToInt32 ();

    result_return.type = JS_FLOAT;
    result_return.vfloat = (hour * MS_PER_HOUR
                                 + min * MS_PER_MINUTE
                                 + sec * MS_PER_SECOND
                                 + ms);
    }

static void
jsGM_MakeDay
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 3 )
    {
        vm->RaiseError( "MakeDay: illegal amount of argument" );
        }

    if ( ! args[ 1 ].IsNumber () || ! args[ 2 ].IsNumber ()
        || ! args[ 3 ].IsNumber () )
    {
        vm->RaiseError( "MakeDay: illegal argument" );
        }

    if ( ! args[ 1 ].IsFinite () || ! args[ 2 ].IsFinite ()
        || ! args[ 3 ].IsFinite () )
    {
        result_return.type = JS_NAN;
        return;
        }

    vm->RaiseError( "MakeDay: not implemented yet");

    // JSInt32 year  = args[ 1 ].ToInt32 ();
    // JSInt32 month = args[ 2 ].ToInt32 ();
    // JSInt32 day   = args[ 3 ].ToInt32 ();
    }

static void
jsGM_MakeDate
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 2 )
    {
        vm->RaiseError( "MakeDate: illegal amount of argument" );
        }

    if ( ! args[ 1 ].IsNumber () || ! args[ 2 ].IsNumber () )
    {
        vm->RaiseError( "MakeDate: illegal argument" );
        }

    if ( ! args[ 1 ].IsFinite () || ! args[ 2 ].IsFinite () )
    {
        result_return.type = JS_NAN;
        return;
        }

    JSInt32 day  = args[ 1 ].ToInt32 ();
    JSInt32 time = args[ 2 ].ToInt32 ();

    result_return.type = JS_FLOAT;
    result_return.vfloat = double( day ) * MS_PER_DAY + time;
    }

static void
jsGM_TimeClip
(
    JSVirtualMachine* vm,
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "TimeClip: illegal amount of argument" );
        }

    if ( ! args[ 1 ].IsNumber () )
    {
        vm->RaiseError( "TimeClip: illegal argument" );
        }

    if ( ! args[ 1 ].IsNumber () )
    {
        result_return.type = JS_NAN;
        return;
        }
    result_return.type = JS_FLOAT;

    if ( args[ 1 ].type == JS_INTEGER )
        result_return.vfloat = double( args[ 1 ].vinteger );
    else
        result_return.vfloat = args[ 1 ].vfloat;

    if ( result_return.vfloat > 8.64e15 || result_return.vfloat < -8.64e15 )
        result_return.type = JS_NAN;
    }

//
// The Date class and date global methods initialization entry
//

void
JSVirtualMachine:: BuiltinDate( void )
{
    new(this) JSBuiltinInfo_Date;

    // global methods...
    //
    new(this) JSBuiltinInfo_GM( "MakeTime",   jsGM_MakeTime );
    new(this) JSBuiltinInfo_GM( "MakeDay",    jsGM_MakeDay );
    new(this) JSBuiltinInfo_GM( "MakeDate",   jsGM_MakeDate );
    new(this) JSBuiltinInfo_GM( "TimeClip",   jsGM_TimeClip );
    }
