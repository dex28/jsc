//
// The Math class
//

#include "JS.h"

#define ONE_ARG_double(d)                    \
double d;                                    \
do {                                         \
    if ( args->vinteger != 1 )               \
        goto argument_error;                 \
                                             \
    JSVariant cvt;                              \
    args[ 1 ].ToNumber( cvt );               \
                                             \
    if ( cvt.type == JS_INTEGER )            \
    {                                        \
        d = double( cvt.vinteger );          \
        }                                    \
    else if ( cvt.type == JS_FLOAT )         \
    {                                        \
        d = cvt.vfloat;                      \
        }                                    \
    else                                     \
    {                                        \
        /* Must be NaN. */                   \
        result_return.type = JS_NAN;         \
        return JS_PROPERTY_FOUND;            \
        }                                    \
  } while( 0 )

#define TWO_ARGS_double(d,d2)                \
double d, d2;                                \
do {                                         \
    if ( args->vinteger != 2 )               \
      goto argument_error;                   \
                                             \
    JSVariant cvt;                              \
    args[ 1 ].ToNumber( cvt );               \
                                             \
    if ( cvt.type == JS_INTEGER )            \
    {                                        \
        d = double( cvt.vinteger );          \
        }                                    \
    else if ( cvt.type == JS_FLOAT )         \
    {                                        \
        d = cvt.vfloat;                      \
        }                                    \
    else                                     \
    {                                        \
        /* Must be NaN. */                   \
        result_return.type = JS_NAN;         \
        return JS_PROPERTY_FOUND;            \
        }                                    \
                                             \
    args[ 2 ].ToNumber( cvt );               \
                                             \
    if ( cvt.type == JS_INTEGER )            \
    {                                        \
        d2 = double( cvt.vinteger );         \
        }                                    \
    else if ( cvt.type == JS_FLOAT )         \
    {                                        \
        d2 = cvt.vfloat;                     \
        }                                    \
    else                                     \
    {                                        \
        /* Must be NaN. */                   \
        result_return.type = JS_NAN;         \
        return JS_PROPERTY_FOUND;            \
        }                                    \
                                             \
  } while( 0 )

struct JSBuiltinInfo_Math : public JSBuiltinInfo
{
    // Static Methods
    JSSymbol s_abs;
    JSSymbol s_acos;
    JSSymbol s_asin;
    JSSymbol s_atan;
    JSSymbol s_atan2;
    JSSymbol s_ceil;
    JSSymbol s_cos;
    JSSymbol s_exp;
    JSSymbol s_floor;
    JSSymbol s_log;
    JSSymbol s_max;
    JSSymbol s_min;
    JSSymbol s_pow;
    JSSymbol s_random;
    JSSymbol s_round;
    JSSymbol s_seed;
    JSSymbol s_sin;
    JSSymbol s_sqrt;
    JSSymbol s_tan;

    // Properties
    JSSymbol s_E;
    JSSymbol s_LN10;
    JSSymbol s_LN2;
    JSSymbol s_LOG10E;
    JSSymbol s_LOG2E;
    JSSymbol s_PI;
    JSSymbol s_SQRT1_2;
    JSSymbol s_SQRT2;

    JSBuiltinInfo_Math( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );
    };

JSBuiltinInfo_Math:: JSBuiltinInfo_Math( void )
    : JSBuiltinInfo( "Math" )
{
    s_abs            = vm->Intern( "abs" );
    s_acos           = vm->Intern( "acos" );
    s_asin           = vm->Intern( "asin" );
    s_atan           = vm->Intern( "atan" );
    s_atan2          = vm->Intern( "atan2" );
    s_ceil           = vm->Intern( "ceil" );
    s_cos            = vm->Intern( "cos" );
    s_exp            = vm->Intern( "exp" );
    s_floor          = vm->Intern( "floor" );
    s_log            = vm->Intern( "log" );
    s_max            = vm->Intern( "max" );
    s_min            = vm->Intern( "min" );
    s_pow            = vm->Intern( "pow" );
    s_random         = vm->Intern( "random" );
    s_round          = vm->Intern( "round" );
    s_seed           = vm->Intern( "seed" );
    s_sin            = vm->Intern( "sin" );
    s_sqrt           = vm->Intern( "sqrt" );
    s_tan            = vm->Intern( "tan" );

    s_E              = vm->Intern( "E" );
    s_LN10           = vm->Intern( "LN10" );
    s_LN2            = vm->Intern( "LN2" );
    s_LOG10E         = vm->Intern( "LOG10E" );
    s_LOG2E          = vm->Intern( "LOG2E" );
    s_PI             = vm->Intern( "PI" );
    s_SQRT1_2        = vm->Intern( "SQRT1_2" );
    s_SQRT2          = vm->Intern( "SQRT2" );
    }

JSPropertyRC
JSBuiltinInfo_Math:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    // The default return value
    //
    result_return.type = JS_FLOAT;

    //-------------------------------------------------------------------------
    if ( method == s_abs )
    {
        ONE_ARG_double( d );

        result_return.vfloat = fabs( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_acos )
    {
        ONE_ARG_double( d );

        if ( d < -1.0 || d > 1.0 )
        {
            result_return.type = JS_NAN;
            }
        else
        {
            result_return.vfloat = acos( d );
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_asin )
    {
        ONE_ARG_double( d );

        if ( d < -1.0 || d > 1.0 )
        {
            result_return.type = JS_NAN;
            }
        else
        {
            result_return.vfloat = asin( d );
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_atan )
    {
        ONE_ARG_double( d );

        result_return.vfloat = atan( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_atan2 )
    {
        TWO_ARGS_double( d, d2 );

        result_return.vfloat = atan2 (d, d2);

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_ceil )
    {
        ONE_ARG_double( d );

        result_return.vfloat = ceil( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_cos )
    {
        ONE_ARG_double( d );

        result_return.vfloat = cos( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_exp )
    {
        ONE_ARG_double( d );

        result_return.vfloat = exp( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_floor )
    {
        ONE_ARG_double( d );

        result_return.vfloat = floor( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_log )
    {
        ONE_ARG_double( d );

        result_return.vfloat = log( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_max || method == s_min )
    {
        if ( args->vinteger < 1 )
            goto argument_error;

        // Take the initial argument
        //

        JSVariant cvt;
        args[ 1 ].ToNumber( cvt );

        if ( cvt.type == JS_NAN )
        {
            result_return.type = JS_NAN;
            return JS_PROPERTY_FOUND;
            }

        double d;
        if ( cvt.type == JS_INTEGER )
        {
            d = double( cvt.vinteger );
            }
        else
        {
            d = cvt.vfloat;
            }

        // Handle the rest
        //
        for ( int i = 1; i < args->vinteger; i++ )
        {
            args[ i + 1 ].ToNumber( cvt );

            if ( cvt.type == JS_NAN )
            {
                result_return.type = JS_NAN;
                return JS_PROPERTY_FOUND;
                }

            double d2;
            if ( cvt.type == JS_INTEGER )
            {
                d2 = double( cvt.vinteger );
                }
            else
            {
                d2 = cvt.vfloat;
                }

            if ( method == s_max )
            {
                if ( d2 > d )
                    d = d2;
                }
            else
            {
                if ( d2 < d )
                    d = d2;
                }
            }

        result_return.type = JS_FLOAT;
        result_return.vfloat = d;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_pow )
    {
        TWO_ARGS_double( d, d2 );

        result_return.vfloat = pow( d, d2 );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_random )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.vfloat  = vm->Rand48ctx.Random ();

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_round )
    {
        ONE_ARG_double( d );

        result_return.type = JS_INTEGER;
        result_return.vinteger = long( d + 0.5 );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_seed )
    {
        if ( args->vinteger == 0 )
        {
            vm->Rand48ctx.Seed( time( NULL ) );
            }
        else
        {
            ONE_ARG_double( d );
            vm->Rand48ctx.Seed( (unsigned long) d );
            }

        result_return.type = JS_UNDEFINED;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_sin )
    {
        ONE_ARG_double( d );

        result_return.vfloat = sin( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_sqrt )
    {
        ONE_ARG_double( d );
        result_return.vfloat = sqrt( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_tan )
    {
        ONE_ARG_double( d );
        result_return.vfloat = tan( d );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        result_return.MakeStaticString( vm, "Math" );

        return JS_PROPERTY_FOUND;
        }

    return JS_PROPERTY_UNKNOWN;

    //
    // Error handling.
    //

argument_error:
    vm->RaiseError( "Math.%s(): illegal amount of arguments", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_Math:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    // The default result is float
    //
    node.type = JS_FLOAT;

    //-------------------------------------------------------------------------
    if ( property == s_E )
    {
        if ( set )
            goto immutable;

        node.vfloat = 2.71828182845904523536028747135266250;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_LN10 )
    {
        if ( set )
            goto immutable;

        node.vfloat = 2.302585092994046;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_LN2 )
    {
        if ( set )
            goto immutable;

        node.vfloat = 0.693147180559945309417232121458176568;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_LOG10E )
    {
        if ( set )
            goto immutable;

        node.vfloat = 0.434294481903251827651128918916605082;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_LOG2E )
    {
        if ( set )
            goto immutable;

        node.vfloat = 1.44269504088896340735992468100189214;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_PI )
    {
        if ( set )
            goto immutable;

        node.vfloat = 3.14159265358979323846264338327950288;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_SQRT1_2 )
    {
        if ( set )
            goto immutable;

        node.vfloat = 0.707106781186547524400844362104849039;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( property == s_SQRT2 )
    {
        if (set)
            goto immutable;

        node.vfloat = 1.41421356237309504880168872420969808;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

 immutable:
    vm->RaiseError( "Math.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }


//
// The Math class initialization entry
//

void
JSVirtualMachine:: BuiltinMath( void )
{
    new(this) JSBuiltinInfo_Math;
    }
