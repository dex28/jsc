//
// The File class
//

#include "JS.h"
#include <sys/stat.h>

/*
    Static methods.

        byteToString( BYTE ) => string
      + chmod( string, int ) => boolean
      + lstat( PATH ) => array / boolean
      + remove( PATH ) => boolean
      + rename( FROM, TO ) => boolean
      + stat( PATH ) => array / boolean
        fsize( PATH ) => integer
        stringToByte( STRING ) => number

    Methods:

        open( MODE ) => boolean
        close( void ) => boolean
        setPosition( POSITION [, WHENCE] ) => boolean
        getPosition( void ) => integer
        eof( void ) => boolean
        read( SIZE ) => string
        readln( void ) => string
        readByte( void ) => integer
        write( STRING ) => boolean
        writeln( STRING ) => boolean
        writeByte( INTEGER ) => boolean
      + ungetByte( BYTE ) => boolean
        flush( void ) => boolean
        getLength( void ) => integer
        exists( void ) => boolean
        error( void ) => integer
        clearError( void ) => true

   Properties:

        autoFlush  boolean         mutable
        bufferSize integer         mutable
*/

struct JSBuiltinInfo_File : public JSBuiltinInfo
{
    // Static methods
    JSSymbol s_byteToString;
    JSSymbol s_chmod;
    JSSymbol s_lstat;
    JSSymbol s_remove;
    JSSymbol s_rename;
    JSSymbol s_stat;
    JSSymbol s_fsize;
    JSSymbol s_wavSize;
    JSSymbol s_stringToByte;

    // Methods
    JSSymbol s_open;
    JSSymbol s_close;
    JSSymbol s_setPosition;
    JSSymbol s_getPosition;
    JSSymbol s_eof;
    JSSymbol s_read;
    JSSymbol s_readln;
    JSSymbol s_readByte;
    JSSymbol s_write;
    JSSymbol s_writeln;
    JSSymbol s_writeByte;
    JSSymbol s_ungetByte;
    JSSymbol s_flush;
    JSSymbol s_getLength;
    JSSymbol s_exists;
    JSSymbol s_error;
    JSSymbol s_clearError;

    // Properties
    JSSymbol s_autoFlush;
    JSSymbol s_bufferSize;

    JSBuiltinInfo_File( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );
    };

// Instance context
//
struct FileInstanceCtx
{
    // Flags
    bool dont_close;

    PSTR path;
    JSIOStream* stream;
    };


JSBuiltinInfo_File:: JSBuiltinInfo_File( void )
    : JSBuiltinInfo( "File" )
{
    s_byteToString   = vm->Intern( "byteToString" );
    s_chmod          = vm->Intern( "chmod" );
    s_lstat          = vm->Intern( "lstat" );
    s_remove         = vm->Intern( "remove" );
    s_rename         = vm->Intern( "rename" );
    s_stat           = vm->Intern( "stat" );
    s_fsize          = vm->Intern( "fsize" );
    s_wavSize        = vm->Intern( "wavSize" );
    s_stringToByte   = vm->Intern( "stringToByte" );

    s_open           = vm->Intern( "open" );
    s_close          = vm->Intern( "close" );
    s_setPosition    = vm->Intern( "setPosition" );
    s_getPosition    = vm->Intern( "getPosition" );
    s_eof            = vm->Intern( "eof" );
    s_read           = vm->Intern( "read" );
    s_readln         = vm->Intern( "readln" );
    s_readByte       = vm->Intern( "readByte" );
    s_write          = vm->Intern( "write" );
    s_writeln        = vm->Intern( "writeln" );
    s_writeByte      = vm->Intern( "writeByte" );
    s_ungetByte      = vm->Intern( "ungetByte" );
    s_flush          = vm->Intern( "flush" );
    s_getLength      = vm->Intern( "getLength" );
    s_exists         = vm->Intern( "exists" );
    s_error          = vm->Intern( "error" );
    s_clearError     = vm->Intern( "clearError" );

    s_autoFlush      = vm->Intern( "autoFlush" );
    s_bufferSize     = vm->Intern( "bufferSize" );
    }

JSPropertyRC
JSBuiltinInfo_File:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    FileInstanceCtx* ictx = (FileInstanceCtx*)instance_context;
    int secure_mode = vm->options.secure_builtin_file;

    // The default result is false
    //
    result_return.type = JS_BOOLEAN;
    result_return.vboolean = 0;

    // Static methods.
    //
    //-------------------------------------------------------------------------
    if ( method == s_byteToString )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        int i = -1;
        if ( args[ 1 ].type == JS_INTEGER )
        {
            i = args[ 1 ].vinteger;
            if ( i < 0 || i > 255 )
                i = -1;
            }

        result_return.MakeString( vm, NULL, 1 );

        if ( i < 0 )
            result_return.vstring->len = 0;
        else
            result_return.vstring->data[ 0 ] = i;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_chmod )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        if ( args[ 2 ].type != JS_INTEGER )
            goto argument_type_error;

        result_return.type= JS_BOOLEAN;

        PSTR cp = args[ 1 ].ToNewCString ();
        result_return.vboolean = ( chmod( cp, args[ 2 ].vinteger ) == 0 );
        delete cp;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_lstat || method == s_stat )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 1 )
            goto argument_error;

        PSTR path = args[ 1 ].ToNewCString ();

        struct stat stat_st;
        int result = stat (path, &stat_st);

        delete path;

        if ( result < 0 )
        {
            return JS_PROPERTY_FOUND;
            }

        JSVariant *node;

        // Success
        result_return.MakeArray( vm, 13 );
        node = result_return.varray->data;

        // dev
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_dev;
        node++;

        // ino
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_ino;
        node++;

        // mode
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_mode;
        node++;

        // nlink
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_nlink;
        node++;

        // uid
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_uid;
        node++;

        // gid
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_gid;
        node++;

        // rdev
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_rdev;
        node++;

        // size
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_size;
        node++;

        // atime
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_atime;
        node++;

        // mtime
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_mtime;
        node++;

        // ctime
        node->type = JS_INTEGER;
        node->vinteger = stat_st.st_ctime;
        node++;

        // blksize
        node->type = JS_INTEGER;
        node->vinteger = 0;
        node++;

        // blocks
        node->type = JS_INTEGER;
        node->vinteger = 0;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_fsize )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 1 )
            goto argument_error;

        PSTR path = args[ 1 ].ToNewCString ();

        struct stat stat_st;
        int result = stat( path, &stat_st );

        delete path;

        if ( result < 0 )
        {
            return JS_PROPERTY_FOUND;
            }

        result_return.type = JS_INTEGER;
        result_return.vinteger = stat_st.st_size;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_wavSize )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 1 )
            goto argument_error;

        PSTR path = args[ 1 ].ToNewCString ();

        struct stat stat_st;
        int result = stat( path, &stat_st );

        delete path;

        if ( result < 0 )
        {
            return JS_PROPERTY_FOUND;
            }

        // remove riff; assume 8 kby/sec rate. TODO: generalize
        long sz = ( stat_st.st_size - 42 ) / 8;
        char str[ 256 ];
        sprintf( str, "%ld:%02ld.%03ld", sz / 60000L, ( sz / 1000L ) % 60L, sz % 1000L );

        result_return.MakeString( vm, str );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_remove )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        PSTR path = args[ 1 ].ToNewCString ();
        int  i = remove( path );
        delete path;

        result_return.vboolean = ( i == 0 );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_rename )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING || args[ 2 ].type != JS_STRING )
            goto argument_type_error;

        PSTR path1 = args[ 1 ].ToNewCString ();
        PSTR path2 = args[ 2 ].ToNewCString ();

        int i = rename( path1, path2 );

        delete path2;
        delete path1;

        result_return.vboolean = ( i == 0 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_exists )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        PSTR path = args[ 1 ].ToNewCString ();

        struct stat stat_st;
        result_return.vboolean = ( stat( path, &stat_st ) >= 0 );

        delete path;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_stringToByte )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        result_return.type = JS_INTEGER;

        if ( args[ 1 ].type == JS_STRING && args[ 1 ].vstring->len > 0 )
            result_return.vinteger = args[ 1 ].vstring->data[ 0 ];
        else
            result_return.vinteger = 0;

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx )
            result_return.MakeString( vm, ictx->path );
        else
            result_return.MakeStaticString( vm, "File" );

        return JS_PROPERTY_FOUND;
        }

    // Instance methods.
    //
    if ( ! ictx)
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //-------------------------------------------------------------------------
    if ( method == s_open )
    {
        if ( secure_mode )
            goto insecure_feature;

        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING || args[ 1 ].vstring->len == 0
            || args[ 1 ].vstring->len > 3 )
            goto argument_type_error;

        int len = args[ 1 ].vstring->len;
        char buf[ 256 ];
        memcpy( buf, args[ 1 ].vstring->data, len );

        if ( buf[ len - 1 ] != 'b' )
            buf[ len++ ] = 'b';
        buf[ len ] = '\0';

        int readp = 0;
        int writep = 0;

        // Check that the mode is valid
        //
        if ( strcmp( buf, "rb" ) == 0 )
            readp = 1;
        else if ( strcmp( buf, "wb" ) == 0 )
            writep = 1;
        else if ( strcmp( buf, "ab" ) == 0 )
            writep = 1;
        else if ( strcmp (buf, "r+b" ) == 0 )
            readp = writep = 1;
        else if ( strcmp (buf, "w+b" ) == 0 )
            readp = writep = 1;
        else if ( strcmp (buf, "a+b" ) == 0 )
            readp = writep = 1;
        else
        {
            vm->RaiseError(
                "File.%s(): illegal open mode \"%s\"", vm->Symname( method ), buf );
            }

        if ( ictx->stream == NULL )
        {
            // Do open
            //
            FILE* fp = fopen( ictx->path, buf );
            ictx->stream = NEW JSIOStreamFile( fp, readp, writep, 1 );

            if ( ! JSIOStream::IsValid( ictx->stream ) )
            {
                delete ictx->stream;
                ictx->stream = NULL;
                }

            if ( ictx->stream != NULL )
                result_return.vboolean = 1;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_close )
    {
        if ( ictx->stream == NULL )
            return JS_PROPERTY_FOUND;

        int result = 0;

        if ( ! ictx->dont_close )
        {
            result = 1;
            delete ictx->stream;
            }

        ictx->stream = NULL;
        result_return.vboolean = ( result >= 0 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setPosition )
    {
        if ( args->vinteger != 1 && args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        int li = args[ 1 ].vinteger;
        int i = SEEK_SET;

        if ( args->vinteger == 2 && args[ 2 ].type == JS_INTEGER )
        {
            switch (args[2].vinteger)
            {
                case 1:
                    i = SEEK_CUR;
                    break;

                case 2:
                    i = SEEK_END;
                    break;

                default:
                    i = SEEK_SET;
                    break;
                }
            }

        if ( ictx->stream && ictx->stream->Seek( li, i ) >= 0 )
            result_return.vboolean = 1;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if (method == s_getPosition)
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_INTEGER;

        if (ictx->stream == NULL)
            result_return.vinteger = -1;
        else
            result_return.vinteger = ictx->stream->GetPosition ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_eof )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->stream != NULL )
            result_return.vboolean = ictx->stream->AtEOF ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_read )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER || args[ 1 ].vinteger < 0 )
        goto argument_type_error;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        PSTR buffer = new(vm) char[ args[ 1 ].vinteger + 1 ];

        int got = ictx->stream->Read( buffer, args[ 1 ].vinteger );
        if ( got < 0)
            got = 0;

        result_return.MakeStaticString( vm, buffer, got );
        result_return.vstring->flags = JSSTRING_NORMAL; // clear 'static' flag

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_readln )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        // Flush all buffered output data
        //
        ictx->stream->Flush ();

        int bufpos = 0;
        int buflen = 0;
        PSTR buffer = NULL;

        for( ;; )
        {
            int ch = ictx->stream->ReadByte ();

            if ( ch < 0 || ch == '\n' )
            {
                break;
                }

            if ( bufpos >= buflen )
            {
                buflen += 128;
                buffer = PSTR( vm->Realloc( buffer, buflen ) );
                }

            buffer[ bufpos++ ] = ch;
            }

        // Remove '\r' characters
        //
        while ( bufpos > 0 && buffer[ bufpos - 1 ] == '\r' )
        {
            bufpos--;
            }

        if ( buffer == NULL )
        {
            // An empty string.  Allocate one byte
            //
            buffer = new(vm) char[ 1 ];
            }

        // Use the data we already had. In maximum, it has only
        // 127 bytes overhead.
        //
        result_return.MakeStaticString( vm, buffer, bufpos );
        result_return.vstring->flags = JSSTRING_NORMAL; // clear 'static' flag

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_readByte )
    {
        result_return.type = JS_INTEGER;

        if ( ictx->stream == NULL )
        {
            result_return.vinteger = -1;
            return JS_PROPERTY_FOUND;
            }

        result_return.vinteger = ictx->stream->ReadByte ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_write || method == s_writeln )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        JSVariant n = args[ 1 ];
        if ( args[ 1 ].type != JS_STRING )
            args[ 1 ].ToString( vm, n );

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        int autoflush = ictx->stream->GetAutoFlush ();
        ictx->stream->SetAutoFlush( 0 );

        int wrote = ictx->stream->Write( n.vstring->data, n.vstring->len );

        if ( wrote == n.vstring->len )
        {
            // Success
            //
            result_return.vboolean = 1;

            if ( method == s_writeln )
            {
                wrote = ictx->stream->Write( JS_ENDLINE, JS_ENDLINE_LEN );
                
                if ( wrote != JS_ENDLINE_LEN )
                {
                    // No, it was not a success
                    result_return.vboolean = 0;
                    }
                }
            }

        ictx->stream->SetAutoFlush( autoflush );

        if ( autoflush )
        {
            ictx->stream->Flush ();
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_writeByte )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        uint8 buf[ 1 ] = { uint8( args[ 1 ].vinteger ) };

        if ( ictx->stream != NULL )
            result_return.vboolean = ictx->stream->Write( buf, 1 ) >= 0;

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_ungetByte )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        result_return.vboolean = ( ictx->stream->Unget( args[ 1 ].vinteger ) != 0 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_flush )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        result_return.vboolean = ( ictx->stream->Flush () >= 0 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getLength )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        // The default error code is an integer -1
        //
        result_return.type = JS_INTEGER;
        result_return.vinteger = -1;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        result_return.vinteger = ictx->stream->GetLength ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_exists )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->stream )
        {
            // Since we have opened the file, it must exist
            //
            result_return.vboolean = 1;
            }
        else
        {
            struct stat stat_st;
            result_return.vboolean = ( stat( ictx->path, &stat_st ) >= 0 );
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_error )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_INTEGER;
        result_return.vinteger = -1;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        result_return.vinteger = ictx->stream->GetError ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_clearError )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( ictx->stream == NULL )
        {
            return JS_PROPERTY_FOUND;
            }

        ictx->stream->ClearError ();

        result_return.vboolean = 1;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "File.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "File.%s(): illegal argument", vm->Symname( method ) );

insecure_feature:
    vm->RaiseError( "File.%s(): not allowed in secure mode", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_File:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    FileInstanceCtx* ictx = (FileInstanceCtx*)instance_context;

    if ( ! ictx)
    {
        if ( ! set )
            node.type = JS_UNDEFINED;

        return JS_PROPERTY_UNKNOWN;
        }

    // Instance properties
    //
    if ( property == s_autoFlush )
    {
        if ( ictx->stream == NULL )
            goto not_open;

        if ( set )
        {
            if ( node.type != JS_BOOLEAN )
                goto argument_type_error;

            ictx->stream->SetAutoFlush( node.vboolean );
            }
        else
        {
            node.type = JS_BOOLEAN;
            node.vboolean = ictx->stream->GetAutoFlush ();
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( property == s_bufferSize )
    {
        if ( ictx->stream == NULL )
            goto not_open;

        if ( set )
        {
            if ( node.type != JS_INTEGER )
                goto argument_type_error;

            ictx->stream->SetBufferSize( node.vinteger );
            }
        else
        {
            node.type = JS_INTEGER;
            node.vinteger = ictx->stream->GetBufferSize ();
            }

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling
    //

    if ( ! set)
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

argument_type_error:
    vm->RaiseError( "File.%s: illegal value", vm->Symname( property ) );

not_open:
    vm->RaiseError( "File.%s: the stream is not opened", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }


void
JSBuiltinInfo_File:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger != 1 )
    {
        vm->RaiseError( "new File(): illegal amount of arguments" );
        }

    if ( args[1].type != JS_STRING )
    {
        vm->RaiseError( "new File(): illegal argument" );
        }

    FileInstanceCtx* instance = NEW FileInstanceCtx;
    // FIXME: ZeroMemory instance

    instance->path = args[ 1 ].ToNewCString ();

    result_return = new(vm) JSBuiltin( this, instance );
    }

void
JSBuiltinInfo_File:: OnFinalize
(
    void* instance_context
    )
{
    FileInstanceCtx* ictx = (FileInstanceCtx*)instance_context;

    if ( ictx )
    {
        if ( ictx->stream )
        {
            if ( ! ictx->dont_close )
            {
                delete ictx->stream;
                }

            ictx->stream = NULL;
            }

        delete ictx->path;
        delete ictx;
        }
    }

//
// The File new object
//

void
JSVirtualMachine:: FileNew
(
    JSVariant& result_return,
    PCSTR path,
    JSIOStream* stream,
    bool dont_close
    )
{
    // Create a file instance
    //
    FileInstanceCtx* ictx = NEW FileInstanceCtx;
    // FIXME: ZeroMemory ictx

    ictx->path = jsStrDup( path );
    ictx->stream = stream;
    ictx->dont_close = dont_close;

    // Lookup our context
    //
    JSBuiltinInfo* info = Intern( "File" )->vbuiltin->info;

    // Create the builtin
    //
    result_return = new(this) JSBuiltin( info, ictx );
    }

//
// The File class initialization entry
//

void
JSVirtualMachine:: BuiltinFile( void )
{
    new(this) JSBuiltinInfo_File;
    }
