//
// The VM class
//

#include "JS.h"

/*
    Static methods:

        VM.garbageCollect( void )
        VM.stackTrace( void )

    Properties:                    type            mutable

        VM.gcCount                 integer
        VM.gcTrigger               integer         yes
        VM.heapAllocated           integer
        VM.heapFree                integer
        VM.heapSize                integer
        VM.numGlobals              integer
        VM.stackSize               integer
        VM.stacktraceOnError       boolean         yes
        VM.verbose                 integer         yes
        VM.verboseStackTrace       boolean         yes
        VM.version                 string
        VM.versionMajor            integer
        VM.versionMinor            integer
        VM.versionPatch            integer
        VM.warnUndef               boolean         yes
*/

struct JSBuiltinInfo_VM : public JSBuiltinInfo
{
    // Static Methods
    JSSymbol s_garbageCollect;
    JSSymbol s_stackTrace;

    // Static Properties
    JSSymbol s_gcCount;
    JSSymbol s_gcTrigger;
    JSSymbol s_heapAllocated;
    JSSymbol s_heapFree;
    JSSymbol s_heapSize;
    JSSymbol s_numGlobals;
    JSSymbol s_stackSize;
    JSSymbol s_stacktraceOnError;
    JSSymbol s_verbose;
    JSSymbol s_verboseStacktrace;
    JSSymbol s_version;
    JSSymbol s_versionMajor;
    JSSymbol s_versionMinor;
    JSSymbol s_versionPatch;
    JSSymbol s_warnUndef;

    JSBuiltinInfo_VM( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );
    };


JSBuiltinInfo_VM:: JSBuiltinInfo_VM( void )
    : JSBuiltinInfo( "VM" )
{
    s_garbageCollect         = vm->Intern( "garbageCollect" );
    s_stackTrace             = vm->Intern( "stackTrace" );

    s_gcCount                = vm->Intern( "gcCount" );
    s_gcTrigger              = vm->Intern( "gcTrigger" );
    s_heapAllocated          = vm->Intern( "heapAllocated" );
    s_heapFree               = vm->Intern( "heapFree" );
    s_heapSize               = vm->Intern( "heapSize" );
    s_numGlobals             = vm->Intern( "numGlobals" );
    s_stackSize              = vm->Intern( "stackSize" );
    s_stacktraceOnError      = vm->Intern( "stacktraceOnError" );
    s_verbose                = vm->Intern( "verbose" );
    s_verboseStacktrace      = vm->Intern( "verboseStackTrace" );
    s_version                = vm->Intern( "version" );
    s_versionMajor           = vm->Intern( "versionMajor" );
    s_versionMinor           = vm->Intern( "versionMinor" );
    s_versionPatch           = vm->Intern( "versionPatch" );
    s_warnUndef              = vm->Intern( "warnUndef" );
    }

JSPropertyRC
JSBuiltinInfo_VM:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    // The default return value is undefined
    //
    result_return.type = JS_UNDEFINED;

    //-------------------------------------------------------------------------
    if ( method == s_garbageCollect )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        // Ok, let's trigger a garbage collection
        //
        vm->gc.bytes_allocated = vm->options.gc_trigger + 1;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_stackTrace )
    {
        if ( args->vinteger != 0 && args->vinteger != 1 )
            goto argument_type_error;

        if ( args->vinteger == 1 )
        {
            if ( args[ 1 ].type != JS_INTEGER )
                goto argument_type_error;

            vm->StackTrace( args[ 1 ].vinteger );
            }
        else
        {
            vm->StackTrace ();
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.MakeStaticString( vm, "VM" );

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "VM.%s(): illegal amout of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "VM.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_VM:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    //--------------------------------------------------------------------
    if ( property == s_gcCount )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->gc.count;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_gcTrigger )
    {
        if ( set )
        {
            if ( node.type != JS_INTEGER )
                goto value_error;

            vm->options.gc_trigger = node.vinteger;
            }
        else
        {
            node.type = JS_INTEGER;
            node.vinteger = vm->options.gc_trigger;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_heapAllocated )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->gc.bytes_allocated;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_heapFree )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->gc.bytes_free;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_heapSize )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->heap_size;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_numGlobals )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->num_globals;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_stackSize )
    {
        if (set)
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = vm->stack_size;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_stacktraceOnError )
    {
        if ( set )
        {
            if ( node.type != JS_BOOLEAN )
                goto value_error;

            vm->options.stacktrace_on_error = node.vboolean;
            }
        else
        {
            node.type = JS_BOOLEAN;
            node.vboolean = vm->options.stacktrace_on_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_verbose )
    {
        if ( set )
        {
            if ( node.type != JS_INTEGER )
                goto value_error;

            vm->options.verbose = node.vinteger;
            }
        else
        {
            node.type = JS_INTEGER;
            node.vinteger = vm->options.verbose;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_verboseStacktrace )
    {
        if ( set )
        {
            if ( node.type != JS_BOOLEAN)
                goto value_error;

            vm->options.verbose_stacktrace = node.vboolean;
            }
        else
        {
            node.type = JS_BOOLEAN;
            node.vboolean = vm->options.verbose_stacktrace;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_version )
    {
        if ( set )
            goto immutable;

        node.MakeStaticString( vm, VERSION );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_versionMajor )
    {
        if ( set )
            goto immutable;

        int minor = 0, major = 0, patch = 0;
        sscanf( VERSION, "%d.%d (%d)", &major, &minor, &patch );
        node.type = JS_INTEGER;
        node.vinteger = major;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_versionMinor )
    {
        if ( set )
            goto immutable;

        int minor = 0, major = 0, patch = 0;
        sscanf( VERSION, "%d.%d (%d)", &major, &minor, &patch );
        node.type = JS_INTEGER;
        node.vinteger = minor;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_versionPatch )
    {
        if ( set )
            goto immutable;

        int minor = 0, major = 0, patch = 0;
        sscanf( VERSION, "%d.%d (%d)", &major, &minor, &patch );
        node.type = JS_INTEGER;
        node.vinteger = patch;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_warnUndef )
    {
        if ( set )
        {
            if ( node.type != JS_INTEGER )
                goto value_error;

            vm->options.warn_undef = node.vinteger != 0;
            }
        else
        {
            node.type = JS_INTEGER;
            node.vinteger = vm->options.warn_undef;
            }

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

value_error:
    vm->RaiseError( "VM.%s: illegal value", vm->Symname( property ) );

immutable:
    vm->RaiseError( "VM.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

//
// Builtin initialization entry
//

void
JSVirtualMachine:: BuiltinVM( void )
{
    new(this) JSBuiltinInfo_VM;
    }
