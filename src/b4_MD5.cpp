//
// The MD5 class
//

#include "JS.h"
#include "MD5.h"

struct JSBuiltinInfo_MD5 : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_final;
    JSSymbol s_finalBinary;
    JSSymbol s_init;
    JSSymbol s_update;

    JSBuiltinInfo_MD5( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );
    };

JSBuiltinInfo_MD5:: JSBuiltinInfo_MD5( void )
    : JSBuiltinInfo( "MD5" )
{
    s_final           = vm->Intern( "final" );
    s_finalBinary     = vm->Intern( "finalBinary" );
    s_init            = vm->Intern( "init" );
    s_update          = vm->Intern( "update" );
    }

JSPropertyRC
JSBuiltinInfo_MD5:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    MD5* ictx = (MD5*)instance_context;
    uint8 final[ 16 ];

    // Static methods
    //
    //--------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( ictx )
            goto default_to_string;

        result_return.MakeStaticString( vm, "MD5" );

        return JS_PROPERTY_FOUND;
        }

    // Methods
    //
    if ( ictx == NULL )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //--------------------------------------------------------------------
    if ( method == s_final )
    {
        default_to_string:

        if ( args->vinteger != 0 )
            goto argument_error;

        char buf[ 34 ];  // 16 hex +1 for the trailing '\0'.
        ictx->FinalHex( buf );

        result_return.MakeString( vm, buf, 32 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_finalBinary )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Final( final );
        result_return.MakeString( vm, PSTR( final ), 16 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_init )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Init ();
        result_return.type = JS_UNDEFINED;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_update )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        ictx->Update( (uint8*)args[ 1 ].vstring->data, args[ 1 ].vstring->len );
        result_return.type = JS_UNDEFINED;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "MD5.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "MD5.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_MD5:: OnProperty
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
JSBuiltinInfo_MD5:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger != 0 )
    {
        vm->RaiseError( "new MD5(): illegal amount of arguments" );
        }

    MD5* ictx = NEW MD5;
    ictx->Init ();

    result_return = new(vm) JSBuiltin( this, ictx );
    }

void
JSBuiltinInfo_MD5:: OnFinalize
(
    void* instance_context
    )
{
    MD5* ictx = (MD5*)instance_context;

    if ( ictx )
        delete ictx;
    }

//
// The MD5 class initialization entry
//

void
JSVirtualMachine:: BuiltinMD5( void )
{
    new(this) JSBuiltinInfo_MD5;
    }
