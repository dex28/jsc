//
// The Boolean class
//

#include "JS.h"

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo_Bool

struct JSBuiltinInfo_Bool : public JSBuiltinInfo
{
    JSBuiltinInfo_Bool( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );
    };


JSBuiltinInfo_Bool:: JSBuiltinInfo_Bool( void )
    : JSBuiltinInfo( "Boolean" )
{
    // VM primitive
    //
    vm->prim[ JS_BOOLEAN ] = this;
    }

void
JSBuiltinInfo_Bool:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    result_return.type = JS_BOOLEAN;

    if ( args->vinteger == 0 )
        result_return.vboolean = 0;

    else if ( args->vinteger == 1 )
        result_return.vboolean = args[ 1 ].ToBoolean ();

    else
    {
        vm->RaiseError( "Boolean(): illegal amount of arguments" );
        }
    }

JSPropertyRC
JSBuiltinInfo_Bool:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
        {
            vm->RaiseError( "Boolean.%s(): illegal amount of arguments",
                            vm->Symname( method ) );
            }

        if ( n )
        {
            PCSTR cp = n->vboolean ? "true" : "false";
            result_return.MakeStaticString( vm, cp );
            }
        else
            result_return.MakeStaticString( vm, "Boolean" );

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == vm->s_valueOf )
    {
        if ( n == NULL )
        {
            result_return = *vm->Intern( "Boolean" );
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
    }

JSPropertyRC
JSBuiltinInfo_Bool:: OnProperty
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
JSBuiltinInfo_Bool:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    result_return.type = JS_BOOLEAN;

    if ( args->vinteger == 0 )
        result_return.vboolean = 0;
    else if ( args->vinteger == 1 )
        result_return.vboolean = args[ 1 ].ToBoolean ();
    else
    {
        vm->RaiseError( "new Boolean(): illegal amount of arguments" );
        }

    // Set the [[Prototype]] and [[Class]] properties
    // FIXME: 15.10.2.1 */
    }

//
// The Boolean class initialization entry
//

void
JSVirtualMachine:: BuiltinBoolean( void )
{
    new(this) JSBuiltinInfo_Bool;
    }

