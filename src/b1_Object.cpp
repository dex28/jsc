//
// The Object class
//

#include "JS.h"

struct JSBuiltinInfo_Object : public JSBuiltinInfo
{
    JSBuiltinInfo_Object( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );
    };

JSBuiltinInfo_Object:: JSBuiltinInfo_Object( void )
    : JSBuiltinInfo( "Object" )
{
    // VM primitive
    //
    vm->prim[ JS_OBJECT ] = this;
    }

void
JSBuiltinInfo_Object:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger > 1 )
    {
        vm->RaiseError( "Object(): illegal amount of arguments" );
        }

    if ( args->vinteger == 0
        || ( args->vinteger == 1
            && ( args[1].type == JS_NULL
                || args[1].type == JS_UNDEFINED 
                )
            )
        )
    {
        // Create a fresh new object
        //
        result_return.type = JS_OBJECT;
        result_return.vobject = new(vm) JSObject;
        }
    else
    {
        // We have one argument. Call ToObject() for it
        //
        args[ 1 ].ToObject( vm, result_return );
        }
    }

JSPropertyRC
JSBuiltinInfo_Object:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    //--------------------------------------------------------------------------
    if ( method == vm->s_toSource )
    {
        if ( instance_context )
        {
            result_return.type = JS_UNDEFINED;
            // FIXME: 15.2.4.3
            }
        else
        {
            PSTR source = "new Object()";
            result_return.MakeStaticString( vm, source );
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( instance_context )
            result_return.MakeStaticString( vm, "[object Object]" );
        else
            result_return.MakeStaticString( vm, "Object" );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------------
    else if ( method == vm->s_valueOf )
    {
        if ( instance_context == NULL )
        {
            result_return = *vm->Intern( "Object" );
            }
        else
        {
            result_return = *n;
            }

        return JS_PROPERTY_FOUND;
        }

    return JS_PROPERTY_UNKNOWN;
    }

JSPropertyRC
JSBuiltinInfo_Object:: OnProperty
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
JSBuiltinInfo_Object:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger == 0 )
    {
        return_native_object:
        result_return.type = JS_OBJECT;
        result_return.vobject = new(vm) JSObject;

        // Set the [[Prototype]] and [[Class]] properties
        // FIXME: 15.2.2.2
        }
    else if ( args->vinteger == 1 )
    {
        switch ( args[1].type )
        {
            case JS_OBJECT:
                result_return = args[ 1 ];
                break;

            case JS_STRING:
            case JS_BOOLEAN:
            case JS_INTEGER:
            case JS_FLOAT:
            case JS_NAN:
                args[ 1 ].ToObject( vm, result_return );
                break;

            case JS_NULL:
            case JS_UNDEFINED:
                goto return_native_object;
                break;

            default:
                // The rest are implementation dependent
                //
                result_return = args[ 1 ];
                break;
            }
        }
    else
    {
        vm->RaiseError( "new Object(): illegal amount of arguments");
        }
    }

//
// The Object class initialization entry
//

void
JSVirtualMachine:: BuiltinObject( void )
{
    new(this) JSBuiltinInfo_Object;
    }
