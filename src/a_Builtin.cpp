//
// JSBuiltin class and JSHeapObjectToClean class derivates
//

#include "JS.h"

///////////////////////////////////////////////////////////////////////////////
// JSHeapObjectToClean implementation

void*
JSHeapObjectToClean:: operator new( size_t size, JSVirtualMachine* vm )
{
    JSHeapObjectToClean* ptr = (JSHeapObjectToClean*)vm->Alloc( size, 1 );
    ptr->vm = vm;
    return ptr;
    }

void
JSHeapObjectToClean:: operator delete( void* ptr, JSVirtualMachine* vm )
{
    ( (JSHeapObjectToClean*)ptr )->vm->Free( ptr );
    }

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo implementation

JSBuiltinInfo:: JSBuiltinInfo
(
    PCSTR arg_szName
    )
{
    szName = arg_szName;
    prototype = new(vm) JSObject;

    // Set the __proto__ property to null.
    // We have no prototype object above us.
    //
    JSVariant prototypeNull;
    prototypeNull.type = JS_NULL;
    prototype->StoreProperty( vm->s___proto__, prototypeNull );

    // If it is possible, define it
    //
    if ( szName )
    {
        *vm->Intern( szName ) = new(vm) JSBuiltin( this );
        }
    }

void
JSBuiltinInfo:: OnFinalize
(
    void
    )
{
    }

void
JSBuiltinInfo:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    result_return.type = JS_UNDEFINED;
    }

JSPropertyRC
JSBuiltinInfo:: OnMethod
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
JSBuiltinInfo:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    return JS_PROPERTY_UNKNOWN;
    }

void
JSBuiltinInfo:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    result_return.type = JS_UNDEFINED;
    }

void
JSBuiltinInfo:: OnFinalize
(
    void* instance_context
    )
{
    }

void
JSBuiltinInfo:: OnMark
(
    void* instance_context
    )
{
    }

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo_GM implementation

JSBuiltinInfo_GM:: JSBuiltinInfo_GM
(
    PSTR arg_szName,
    GlobalMethodProc proc,
    void* instance_context
    )
{
    szName = arg_szName;
    globalMethod = proc;

    *vm->Intern( szName ) = new(vm) JSBuiltin( this, instance_context );
    }

void
JSBuiltinInfo_GM:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    globalMethod( vm, instance_context, result_return, args );
    }

///////////////////////////////////////////////////////////////////////////////
// JSBuiltin implementation

JSBuiltin:: JSBuiltin
(
    JSBuiltinInfo* arg_info,
    void* arg_instance_context
    )
{
    assert( arg_info != NULL && vm == arg_info->vm );

    info = arg_info;

    instance_context = arg_instance_context;
    prototype = NULL;

    if ( instance_context )
    {
        prototype = new(vm) JSObject;

        // Set the __proto__ chain
        //
        JSVariant prototypeObject;
        prototypeObject.type = JS_OBJECT;
        prototypeObject.vobject = info->prototype;

        prototype->StoreProperty( vm->s___proto__, prototypeObject );
        }
    }

void
JSBuiltin:: OnFinalize
(
    void
    )
{
    if ( instance_context )
    {
        info->OnFinalize( instance_context );
        }
    }

