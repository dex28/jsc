//
// The Directory class
//

#include "JS.h"

struct JSBuiltinInfo_Dir : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_close;
    JSSymbol s_open;
    JSSymbol s_read;
    JSSymbol s_rewind;
    JSSymbol s_seek;
    JSSymbol s_tell;

    JSBuiltinInfo_Dir( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );
    };

// Instance context
//
struct DirInstanceCtx
{
    DIR* dir;
    PSTR path;
    };

JSBuiltinInfo_Dir:: JSBuiltinInfo_Dir( void )
    : JSBuiltinInfo( "Directory" )
{
    s_close          = vm->Intern( "close" );
    s_open           = vm->Intern( "open" );
    s_read           = vm->Intern( "read" );
    s_rewind         = vm->Intern( "rewind" );
    s_seek           = vm->Intern( "seek" );
    s_tell           = vm->Intern( "tell" );
    }

JSPropertyRC
JSBuiltinInfo_Dir:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    DirInstanceCtx* ictx = (DirInstanceCtx*)instance_context;
    int secure_mode = vm->options.secure_builtin_file;

    if ( secure_mode )
        goto insecure_feature;

    // Static methods
    //
    //--------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx )
            result_return.MakeString( vm, ictx->path );
        else
            result_return.MakeStaticString( vm, "Directory" );

        return JS_PROPERTY_FOUND;
        }

    // Instance methods.
    //
    //--------------------------------------------------------------------
    if ( ! ictx )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    if ( method == s_close )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->dir )
        {
            if ( ictx->dir->Close () >= 0 )
                ictx->dir = NULL;
            }

        result_return.type = JS_BOOLEAN;
        result_return.vboolean = ictx->dir == NULL;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if (method == s_open)
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->dir == NULL )
            ictx->dir = NEW DIR( ictx->path );

        result_return.type = JS_BOOLEAN;
        result_return.vboolean = ( ictx->dir != NULL );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_read )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->dir == NULL )
            goto not_open;

        PSTR szDirName = ictx->dir->Read ();

        if ( szDirName )
        {
            result_return.MakeString( vm, szDirName );
            }
        else
        {
            result_return.type = JS_BOOLEAN;
            result_return.vboolean = 0;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_rewind )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->dir == NULL )
            goto not_open;

        ictx->dir->Rewind ();
        result_return.type = JS_UNDEFINED;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_seek )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( ictx->dir == NULL )
            goto not_open;

        ictx->dir->Seek( args[ 1 ].vinteger );
        result_return.type = JS_UNDEFINED;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_tell )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->dir == NULL )
            goto not_open;

        result_return.vinteger = ictx->dir-> Tell ();

        if ( result_return.vinteger < 0 )
        {
            result_return.type = JS_BOOLEAN;
            result_return.vboolean = 0;
            }
        else
        {
            result_return.type = JS_INTEGER;
            }

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "Directory.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "Directory.%s(): illegal argument", vm->Symname( method ) );

not_open:
    vm->RaiseError( "Directory.%s(): directory is no opened", vm->Symname( method ) );

insecure_feature:
    vm->RaiseError( "Directory.%s(): not allowed in secure mode", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_Dir:: OnProperty
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
JSBuiltinInfo_Dir:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "new Directory(): illegal amount of arguments" );
        }

    if ( args[ 1 ].type != JS_STRING )
    {
        vm->RaiseError( "new Directory(): illegal argument" );
        }

    DirInstanceCtx* instance = NEW DirInstanceCtx;
    // FIXME: ZeorMemory instance

    instance->path = args[ 1 ].ToNewCString ();

    result_return = new(vm) JSBuiltin( this, instance );
    }

void
JSBuiltinInfo_Dir:: OnFinalize
(
    void* instance_context
    )
{
    DirInstanceCtx* ictx = (DirInstanceCtx*)instance_context;

    if ( ictx )
    {
        if ( ictx->dir )
            ictx->dir->Close ();

        delete ictx->path;
        delete ictx;
        }
    }


//
// The Directory class initialization entry
//

void
JSVirtualMachine:: BuiltinDirectory( void )
{
    new(this) JSBuiltinInfo_Dir;
    }
