//
// The Function class
//

#include "JS.h"

struct JSBuiltinInfo_Function : public JSBuiltinInfo
{
    JSBuiltinInfo_Function( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );
    };

JSBuiltinInfo_Function:: JSBuiltinInfo_Function( void )
    : JSBuiltinInfo( "Function" )
{
    // VM primitive
    //
    vm->prim[ JS_FUNCTION ] = this;
    }

JSPropertyRC
JSBuiltinInfo_Function:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    return JS_PROPERTY_UNKNOWN;
    }

JSPropertyRC
JSBuiltinInfo_Function:: OnProperty
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

//
// The Function class initialization entry
//

void
JSVirtualMachine:: BuiltinFunction( void )
{
    new(this) JSBuiltinInfo_Function;
    }
