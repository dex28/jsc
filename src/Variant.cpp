//
// JSVariant implementation.
//
// JSVariant:: MakeArray
// JSVariant:: ExpandArray
// JSVariant:: MakeString
// JSVariant:: MakeStaticString
// JSVariant:: Mark
// JSVariant:: ToPrimitive
// JSVariant:: ToString
// JSVariant:: ToObject
// JSVariant:: ToNumber
// JSVariant:: ToInt32
// JSVariant:: ToBoolean
// 

#include "JS.h"

void
JSVariant:: MakeArray( JSVirtualMachine* vm, long length )
{
    type = JS_ARRAY;
    varray = new(vm) JSArray;
    varray->prototype = NULL;
    varray->length = length;
    varray->data = new(vm) JSVariant[ length ];

    for ( int i = 0; i < length; i++ )
    {
        varray->data[ i ].type = JS_UNDEFINED;
        }
    }

void
JSVariant:: ExpandArray( JSVirtualMachine* vm, long length )
{
    assert( type == JS_ARRAY );

    if ( varray->length < length )
    {
        varray->data = (JSVariant*) vm->Realloc( varray->data, length * sizeof( JSVariant ) );

        for ( ; varray->length < length; varray->length++ )
        {
            varray->data[ varray->length ].type = JS_UNDEFINED;
            }
        }
    }

void
JSVariant:: MakeString( JSVirtualMachine* vm, PCSTR data, int data_len )
{
    if ( data_len < 0 )
    {
        data_len = strlen( data );
        }

    type = JS_STRING;
    vstring = new(vm) JSString;
    vstring->flags = JSSTRING_NORMAL; // plain string
    vstring->prototype = NULL;
    vstring->len = data_len;
    vstring->data = new(vm) char[ data_len + 1 ];

    if ( data )
    {
        memcpy( vstring->data, data, data_len );
        vstring->data[ data_len ] = 0;
        }
    }

void
JSVariant:: MakeStaticString( JSVirtualMachine* vm, PCSTR data, int data_len )
{
    if ( data_len < 0 )
    {
        data_len = strlen( data );
        }

    type = JS_STRING;
    vstring = new(vm) JSString;
    vstring->flags = JSSTRING_STATIC; // static string
    vstring->prototype = NULL;
    vstring->len = data_len;
    vstring->data = PSTR( data );
    }

void
JSVariant:: Mark( void )
{
    switch( type )
    {
        case JS_STRING:
            if ( ! ( vstring->flags & JSSTRING_DONT_GC ) ) // if it is not 'dont GC'
            {
                JSVirtualMachine::MarkPtr( vstring );

                if ( ! ( vstring->flags & JSSTRING_STATIC ) ) // if it is not 'static'
                {
                    JSVirtualMachine::MarkPtr( vstring->data );
                    }

                vstring->prototype->Mark ();
                }
            break;

        case JS_OBJECT:
            vobject->Mark ();
            break;

        case JS_ARRAY:
            if ( JSVirtualMachine::MarkPtr( varray ) )
            {
                JSVirtualMachine::MarkPtr( varray->data );

                for ( int i = 0; i < varray->length; i++ )
                {
                    varray->data[ i ].Mark ();
                    }

                varray->prototype->Mark ();
                }
            break;

        case JS_BUILTIN:
            if ( JSVirtualMachine::MarkPtr( vbuiltin ) )
            {
                JSVirtualMachine::MarkPtr( vbuiltin->info );

                vbuiltin->info->prototype->Mark ();
                vbuiltin->prototype->Mark ();

                vbuiltin->info->OnMark( vbuiltin->instance_context );
                }
            break;

        case JS_FUNCTION:
            JSVirtualMachine::MarkPtr( vfunction );
            vfunction->prototype->Mark ();
            break;

        case JS_UNDEFINED:
        case JS_NULL:
        case JS_BOOLEAN:
        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
        case JS_INSTR_PTR:
        case JS_FRAME_PTR:
        case JS_WITH_CHAIN:
        case JS_ARGS_FIX:
            // Nothing here
            break;

        default:
            assert( 0 );
        }
    }

void
JSVariant:: ToPrimitive( JSVirtualMachine* vm, JSVariant& result_return,
                         JSVarType preferred_type )
{
    JSVariant args;
    args.type = JS_INTEGER;
    args.vinteger = 0;

    switch ( type )
    {
        case JS_OBJECT:
            if ( preferred_type == JS_STRING )
	        {
	            if ( vm->CallMethod( this, "toString", 0, &args )
                    && vm->exec_result.IsPrimitive () )
                {
	                result_return = vm->exec_result;
                    }
	            else if ( vm->CallMethod( this, "valueOf", 0, &args )
		            && vm->exec_result.IsPrimitive () )
                {
	                result_return = vm->exec_result;
                    }
	            else
	            {
	                vm->RaiseError( "ToPrimitive(): couldn't convert" );
	                }
	            }
            else
	        {
	            // It must be, or it defaults to NUMBER.
                //
	            if ( vm->CallMethod( this, "valueOf", 0, &args )
	                && vm->exec_result.IsPrimitive () )
                {
	                result_return = vm->exec_result;
                    }
	            else
                {
	                ToString( vm, result_return );
                    }
	            }
            break;

        case JS_BUILTIN:
            // TODO ToPrimitive() for built-ins
            //
            vm->RaiseError( "ToPrimitive(): not implemented yet for built-ins" );
            break;

        case JS_UNDEFINED:
        case JS_NULL:
        case JS_BOOLEAN:
        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
        case JS_STRING:
            result_return = *this;
            break;

        default:
            vm->RaiseError( "ToPrimitive(): couldn't convert (%d)", type );
            break;
        }
    }

void
JSVariant:: ToString( JSVirtualMachine* vm, JSVariant& result_return )
{
    // Create empty arguments
    //
    JSVariant args;
    args.type = JS_INTEGER;
    args.vinteger = 0;

    PCSTR tostring = "VM::ToString: unknown type ???";

    switch( type )
    {
        case JS_UNDEFINED:
            tostring = "undefined";
            break;

        case JS_NULL:
            tostring = "null";
            break;

        case JS_BOOLEAN:
        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
        case JS_STRING:
        case JS_ARRAY:
            (void) vm->prim[ type ]->OnMethod
            (
                this,            // void* instance context
                vm->s_toString,  // JSSymbol method
                result_return,   // JSVariant& result return
                &args            // JSVariant[] arguments
                );
            return;

        case JS_OBJECT:
            // Try to call object's toString() method
            //
            if ( vm->CallMethod( this, "toString", 0, &args )
                 && vm->exec_result.type == JS_STRING )
            {
                result_return = vm->exec_result;
                return;
                }

            // No match
            //
            tostring = "object";
            break;

        case JS_BUILTIN:
        {
            JSPropertyRC rc = vbuiltin->info->OnMethod
            (
                vbuiltin->instance_context,    // void* instance context
                vm->s_toString,                // JSSymbol method
                result_return,                 // JSVariant& result return
                &args                          // JSVariant[] arguments
                );

            if ( JS_PROPERTY_FOUND == rc )
                return;

            // Builtin didn't answer toString(). Let's use our default
            //
            tostring = "builtin";
            }
            break;

        case JS_FUNCTION:
            tostring = "function";
            break;

        case JS_INSTR_PTR:
            tostring = "instrpointer";
            break;

        case JS_FRAME_PTR:
            tostring = "framepointer";
            break;

        case JS_WITH_CHAIN:
            tostring = "withchain";
            break;

        case JS_ARGS_FIX:
            tostring = "argsfix";
            break;
        }

    result_return.MakeStaticString( vm, tostring );
    }

void
JSVariant:: ToObject( JSVirtualMachine* vm, JSVariant& result_return )
{
    switch( type )
    {
        case JS_BOOLEAN:
        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
        case JS_OBJECT:
            result_return = *this;
            break;

        case JS_STRING:
            result_return.MakeString( vm, vstring->data, vstring->len );
            break;

        case JS_UNDEFINED:
        case JS_NULL:
        default:
            vm->RaiseError( "ToObject(): illegal argument" );
            break;
        }
    }

void
JSVariant:: ToNumber( JSVariant& result_return )
{
    switch ( type )
    {
        case JS_UNDEFINED:
            result_return.type = JS_NAN;
            break;

        case JS_NULL:
            result_return.type = JS_INTEGER;
            result_return.vinteger = 0;
            break;

        case JS_BOOLEAN:
            result_return.type = JS_INTEGER;
            result_return.vinteger = vboolean ? 1 : 0;
            break;

        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
            result_return = *this;
            break;

        case JS_STRING:
        {
            PSTR cp = ToNewCString ();
            PSTR end;
            result_return.vinteger = strtol( cp, &end, 10 );

            if ( cp == end )
            {
                // It failed. Check the `Infinity'
                //
                for ( int i = 0; cp[ i ]; i++ )
                {
                    if ( cp[ i ] != '\t' && cp[ i ] != ' '
                        && cp[ i ] != '\f' && cp[ i ] != '\v'
                        && cp[ i ] != '\r' && cp[ i ] != '\n'
                        )
                    {
                        break;
                        }
                    }

                if ( cp[ i ] && memcmp( cp + i, "Infinity", 8 ) == 0 )
                {
                    result_return.MakePositiveInfinity ();
                    }
                else
                {
                    result_return.type = JS_NAN;
                    }
                }
            else
            {
                if ( *end == '.' || *end == 'e' || *end == 'E' )
                {
                    // It is a float number
                    //
                    result_return.vfloat = strtod( cp, &end );
                    if ( cp == end )
                    {
                        // Couldn't parse
                        result_return.type = JS_NAN;
                        }
                    else
                    {
                        // Success
                        result_return.type = JS_FLOAT;
                        }
                    }
                else
                {
                    // It is an integer
                    result_return.type = JS_INTEGER;
                    }
                }

            delete cp;
            }
            break;

        case JS_ARRAY:
        case JS_OBJECT:
        case JS_BUILTIN:
            // FIXME: Not implemented yet
            result_return.type = JS_NAN;
            break;

        case JS_FUNCTION:
        case JS_INSTR_PTR:
        case JS_FRAME_PTR:
        case JS_WITH_CHAIN:
        default:
            result_return.type = JS_NAN;
            break;
        }
    }

JSInt32
JSVariant:: ToInt32( void )
{
    JSVariant intermediate;
    ToNumber( intermediate );

    JSInt32 result = 0;

    switch ( intermediate.type )
    {
        case JS_INTEGER:
            result = JSInt32( intermediate.vinteger );
            break;

        case JS_FLOAT:
            if ( intermediate.IsPositiveInfinity ()
                || intermediate.IsNegativeInfinity () )
                result = 0;
            else
                result = JSInt32( intermediate.vfloat );
            break;

        default:
            assert( 0 );
        }

    return result;
    }

int
JSVariant:: ToBoolean( void )
{
    int result = 0;

    switch( type )
    {
        case JS_BOOLEAN:
            result = vboolean;
            break;

        case JS_INTEGER:
            result = ( vinteger != 0 );
            break;

        case JS_FLOAT:
            result = ( vfloat != 0 );
            break;

        case JS_STRING:
            result = ( vstring->len > 0 );
            break;

        case JS_OBJECT:
            result = 1;
            break;
        }

    return result;
    }
