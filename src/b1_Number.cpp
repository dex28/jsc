//
// The Number class
//

#include "JS.h"

struct JSBuiltinInfo_Number : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_MAX_VALUE;
    JSSymbol s_MIN_VALUE;
    JSSymbol s_NaN;
    JSSymbol s_NEGATIVE_INFINITY;
    JSSymbol s_POSITIVE_INFINITY;

    JSBuiltinInfo_Number( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );
    };


JSBuiltinInfo_Number:: JSBuiltinInfo_Number( void )
    : JSBuiltinInfo( "Number" )
{
    s_MAX_VALUE          = vm->Intern( "MAX_VALUE" );
    s_MIN_VALUE          = vm->Intern( "MIN_VALUE" );
    s_NaN                = vm->Intern( "NaN" );
    s_NEGATIVE_INFINITY  = vm->Intern( "NEGATIVE_INFINITY" );
    s_POSITIVE_INFINITY  = vm->Intern( "POSITIVE_INFINITY" );

    // VM primitives
    //
    vm->prim[ JS_INTEGER ]  = this;
    vm->prim[ JS_FLOAT ]    = this;
    vm->prim[ JS_NAN ]      = this;
    }

void
JSBuiltinInfo_Number:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger == 0 )
    {
        result_return.type = JS_INTEGER;
        result_return.vinteger = 0;
        }
    else if ( args->vinteger == 1 )
    {
        args[ 1 ].ToNumber( result_return );
        }
    else
    {
        vm->RaiseError( "Number(): illegal amount of arguments" );
        }
    }

JSPropertyRC
JSBuiltinInfo_Number:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSVariant *n = (JSVariant*)instance_context;

    //--------------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( n == NULL )
        {
            if ( args->vinteger != 0 )
                goto argument_error;

            result_return.MakeStaticString( vm, "Number" );

            return JS_PROPERTY_FOUND;
            }

        if ( args->vinteger != 0 && args->vinteger != 1 )
            goto argument_error;

        int radix = 10;

        if ( args->vinteger == 1 )
        {
            if ( args[ 1 ].type != JS_INTEGER )
                goto argument_type_error;

            radix = args[ 1 ].vinteger;
            }

        char buf[ 256 ];
        if ( n->type == JS_FLOAT )
        {
            if ( n->IsPositiveInfinity () || n->IsNegativeInfinity () )
                strcpy( buf, "Infinity" );
            else
                sprintf( buf, "%g", n->vfloat );
            }
        else if ( n->type != JS_INTEGER )
        {
            sprintf( buf, "NaN" );
            }
        else 
        {
            switch ( radix )
            {
                case 2:
                {
                    char buf2[ 256 ];
                    int bit = 1;
                    unsigned long ul = (unsigned long) n->vinteger;

                    for ( int i = 0; bit > 0; bit <<= 1, i++ )
                        buf2[ i ] = ( ul & bit ) ? '1' : '0';

                    for ( i--; i > 0 && buf2[ i ] == '0'; i-- )
                        {}

                    bit = i;
                    for ( ; i >= 0; i-- )
                        buf[ bit - i ] = buf2[ i ];
                    buf[ bit + 1 ] = '\0';
                    }
                    break;

                case 8:
                    sprintf( buf, "%lo", (unsigned long) n->vinteger );
                    break;

                case 10:
                    sprintf( buf, "%ld", n->vinteger );
                    break;

                case 16:
                    sprintf( buf, "%lx", (unsigned long) n->vinteger );
                    break;

                default:
                    vm->RaiseError(
                        "Number.%s(): illegal radix %d", vm->Symname( method ), radix );
                }
            }

        result_return.MakeString( vm, buf );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( method == vm->s_valueOf )
    {
        if ( n == NULL )
        {
            result_return = *vm->Intern( "Number" );
            }
        else
        {
            result_return = *n;
            }

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "Number.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "Number.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_Number:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    // The default result type
    //
    node.type = JS_FLOAT;

    //--------------------------------------------------------------------------
    if ( property == s_MAX_VALUE )
    {
        if ( set )
            goto immutable;

        node.vfloat = DBL_MAX;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( property == s_MIN_VALUE )
    {
        if ( set )
            goto immutable;

        node.vfloat = DBL_MIN;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( property == s_NaN )
    {
        if ( set )
            goto immutable;

        node.type = JS_NAN;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( property == s_NEGATIVE_INFINITY )
    {
        if ( set )
            goto immutable;

        node.MakeNegativeInfinity ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( property == s_POSITIVE_INFINITY )
    {
        if ( set )
            goto immutable;

        node.MakePositiveInfinity ();

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

immutable:
    vm->RaiseError( "Number.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_Number:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger == 0 )
    {
        result_return.type = JS_INTEGER;
        result_return.vinteger = 0;
        }
    else if ( args->vinteger == 1 )
    {
        args[ 1 ].ToNumber( result_return );
        }
    else
    {
        vm->RaiseError( "new Number(): illegal amount of arguments" );
        }
    }

//
// The Number class initialization entry
//

void
JSVirtualMachine:: BuiltinNumber( void )
{
    new(this) JSBuiltinInfo_Number;
    }
