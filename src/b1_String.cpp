//
// The String class
//

#include "JS.h"

// FIXME: TODO: global method: String (obj) => string

#define UNPACK_NEED(n)                                              \
do {                                                                \
    if ( bufpos + int( n ) > buflen )                               \
    {                                                               \
        vm->RaiseError(                                             \
                 "String.%s(): too short string for the format",    \
                 vm->Symname( method ) );                           \
        }                                                           \
    } while( 0 )

#define UNPACK_EXPAND()                                       \
do {                                                          \
    result_return.ExpandArray( vm, result_len + 1 );          \
    rnode = &result_return.varray->data[ result_len ];        \
    result_len++;                                             \
    } while( 0 )

/*
   SOUNDEX: Original code by N. Dean Pentcheff <dean@violet.berkeley.edu>
  
   char* soundex( char* )
  
   Given as argument: Pointer to a character string.
   Returns: Pointer to a static string, 4 characters long, plus a terminal
      '\0'.  This string is the Soundex key for the argument string.
   Side effects and limitations:
      Does not clobber the string passed in as the argument.
      No limit on argument string length.
      Assumes a character set with continuously ascending and contiguous
         letters within each case and within the digits (e.g. this works for
         ASCII and bombs in EBCDIC.  But then, most things do.).
   Reference: Adapted from Knuth, D.E. (1973) The art of computer programming;
      Volume 3: Sorting and searching.  Addison-Wesley Publishing Company:
      Reading, Mass. Page 392.
   Special cases:
      Leading or embedded spaces, numerals, or punctuation are squeezed out
         before encoding begins.
      Null strings or those with no encodable letters return the code 'Z000'.
   Test data from Knuth (1973):
      Euler   Gauss   Hilbert Knuth   Lloyd   Lukasiewicz
      E460    G200    H416    K530    L300    L222
*/

void soundex( char* in, char* key )
{
    static int code [] =
      {  0,1,2,3,0,1,2,0,0,2,2,4,5,5,0,1,2,6,2,3,0,1,0,2,0,2 };
      /* a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z */

    // Set up default key, complete with trailing '0's
    //
    key[ 0 ] = 'Z';
    key[ 1 ] = key[ 2 ] = key[ 3 ] = '0';
    key[ 4 ] = '\0';

    // Advance to the first letter.  If none present, return default key
    //
    while ( *in != '\0' && ! isalpha( *in ) )
        ++in;

    if ( *in == '\0' )
        return;

    // Pull out the first letter, uppercase it, and set up for main loop
    //
    key[ 0 ] = islower( *in ) ? toupper( *in ) : *in;
    int last = code[ key[ 0 ] - 'A' ];
    ++in;

    // Scan rest of string, stop at end of string or when the key is full
    //
    for ( int count = 1; count < 4 && *in != '\0'; ++in )
	{
        // If non-alpha, ignore the character altogether
        //
        if ( isalpha( *in ) )
	    {
            int ch = isupper( *in ) ? tolower( *in ) : *in;
            // Fold together adjacent letters sharing the same code
            if ( last != code[ch - 'a'] )
		    {
                last = code[ch - 'a'];
                // Ignore code==0 letters except as separators
                if ( last != 0 )
                    key[count++] = '0' + last;
                }
            }
        }
    }

struct JSBuiltinInfo_String : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_append;
    JSSymbol s_charAt;
    JSSymbol s_charCodeAt;
    JSSymbol s_concat;
    JSSymbol s_crc32;
    JSSymbol s_fromCharCode;
    JSSymbol s_indexOf;
    JSSymbol s_lastIndexOf;
    JSSymbol s_pack;
    JSSymbol s_slice;
    JSSymbol s_split;
    JSSymbol s_substr;
    JSSymbol s_substring;
    JSSymbol s_toLowerCase;
    JSSymbol s_toUpperCase;
    JSSymbol s_unpack;

    // Properties
    JSSymbol s_length;

    JSBuiltinInfo_String( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );
    };


JSBuiltinInfo_String:: JSBuiltinInfo_String( void )
    : JSBuiltinInfo( "String" )
{
    s_append         = vm->Intern( "append" );
    s_charAt         = vm->Intern( "charAt" );
    s_charCodeAt     = vm->Intern( "charCodeAt" );
    s_concat         = vm->Intern( "concat" );
    s_crc32          = vm->Intern( "crc32" );
    s_fromCharCode   = vm->Intern( "fromCharCode" );
    s_indexOf        = vm->Intern( "indexOf" );
    s_lastIndexOf    = vm->Intern( "lastIndexOf" );
    s_pack           = vm->Intern( "pack" );
    s_slice          = vm->Intern( "slice" );
    s_split          = vm->Intern( "split" );
    s_substr         = vm->Intern( "substr" );
    s_substring      = vm->Intern( "substring" );
    s_toLowerCase    = vm->Intern( "toLowerCase" );
    s_toUpperCase    = vm->Intern( "toUpperCase" );
    s_unpack         = vm->Intern( "unpack" );

    s_length         = vm->Intern( "length" );

    // VM primitive
    //
    vm->prim[ JS_STRING ] = this;
    }

void
JSBuiltinInfo_String:: OnGlobalMethod
(
    void* instance_context,
    JSVariant& result_return,
    JSVariant args []
    )
{
    if ( args->vinteger == 0 )
    {
        result_return.MakeStaticString( vm, "" );
        }
    else if (args->vinteger == 1)
    {
        args[ 1 ].ToString( vm, result_return );
        }
    else
    {
        vm->RaiseError( "String(): illegal amount of arguments" );
        }
    }

JSPropertyRC
JSBuiltinInfo_String:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    // Static methods
    //
    //-------------------------------------------------------------------------
    if ( method == s_fromCharCode )
    {
        result_return.MakeString( vm, NULL, args->vinteger );

        for ( int i = 0; i < args->vinteger; i++ )
        {
            if ( args[ i + 1 ].type != JS_INTEGER )
                goto argument_type_error;

            result_return.vstring->data[ i ] = (uint8) args[ i + 1 ].vinteger;
            }

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == s_pack )
    {
        if ( args->vinteger < 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        double dval;
        uint8* buffer = NULL;
        int bufpos = 0;

        int arg = 2;

        for ( int op = 0; op < args[ 1 ].vstring->len; op++ )
        {
            if ( arg >= args->vinteger + 1 )
            {
                vm->RaiseError( "String.%s(): too few arguments for format",
                       vm->Symname( method));
                }

            switch ( args[ 1 ].vstring->data[ op ] )
            {
                case 'C':
                    if ( args[ arg ].type != JS_INTEGER )
                        goto argument_type_error;

                    buffer = (uint8*)vm->Realloc( buffer, bufpos + 1);
                    buffer[ bufpos++ ] = uint8( args[ arg++ ].vinteger );
                    break;

                case 'n':
                {
                    if ( args[ arg ].type != JS_INTEGER )
                        goto argument_type_error;

                    JSUInt32 ui = args[ arg++ ].vinteger;

                    buffer = (uint8*)vm->Realloc( buffer, bufpos + 2 );
                    buffer[bufpos++] = uint8( (ui & 0x0000ff00) >> 8 );
                    buffer[bufpos++] = uint8( ui & 0x000000ff );
                    }
                    break;

                case 'N':
                {
                    if ( args[ arg ].type != JS_INTEGER )
                        goto argument_type_error;

                    JSUInt32 ui = args[ arg++ ].vinteger;

                    buffer = (uint8*)vm->Realloc( buffer, bufpos + 4 );
                    buffer[bufpos++] = uint8( (ui & 0xff000000) >> 24 );
                    buffer[bufpos++] = uint8( (ui & 0x00ff0000) >> 16 );
                    buffer[bufpos++] = uint8( (ui & 0x0000ff00) >> 8 );
                    buffer[bufpos++] = uint8( (ui & 0x000000ff) );
                    }
                    break;

                case 'd':
                    if ( args[arg].type == JS_INTEGER )
                        dval = (double) args[arg].vinteger;
                    else if ( args[ arg ].type == JS_FLOAT )
                        dval = args[arg].vfloat;
                    else
                        goto argument_type_error;
                    arg++;

                    buffer = (uint8*)vm->Realloc( buffer, bufpos + sizeof(double) );
                    memcpy( buffer + bufpos, &dval, sizeof( double ) );
                    bufpos += sizeof( double );
                    break;

                default:
                    ; // Silently ignore it
                }
            }

        result_return.MakeStaticString( vm, PSTR( buffer ), bufpos );
        result_return.vstring->flags = JSSTRING_NORMAL; // clear 'static' flag

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_toString )
    {
        if ( n )
            result_return = *n;
        else
            result_return.MakeStaticString( vm, "String" );

        return JS_PROPERTY_FOUND;
        }

    //-------------------------------------------------------------------------
    else if ( method == vm->s_valueOf )
    {
        if ( n == NULL )
        {
            result_return = *vm->Intern( "String" );
            }
        else
        {
            result_return = *n;
            }

        return JS_PROPERTY_FOUND;
        }

    // Instance methods.
    //
    if ( n == NULL )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //--------------------------------------------------------------------
    if ( method == s_append )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( n->vstring->flags & JSSTRING_STATIC )
        {
            vm->RaiseError( "String.%s(): can't append to a static string",
                            vm->Symname( method ) );
            }

        if ( n->vstring->flags & JSSTRING_DONT_GC )
        {
            vm->RaiseError( "String.%s(): can't append to a 'dont GC' string",
                            vm->Symname( method ) );
            }

        if ( args[ 1 ].type == JS_STRING )
        {
            // Append a string
            //
            n->vstring->data = (PSTR)vm->Realloc( n->vstring->data,
                                              n->vstring->len
                                              + args[ 1 ].vstring->len );

            memcpy( n->vstring->data + n->vstring->len,
                  args[ 1 ].vstring->data, args[ 1 ].vstring->len );

            n->vstring->len += args[ 1 ].vstring->len;
            }
        else if ( args[ 1 ].type == JS_INTEGER )
        {
            // Append a character
            //
            n->vstring->data = (PSTR)vm->Realloc( n->vstring->data,
                                              n->vstring->len + 1 );

            n->vstring->data[ n->vstring->len++ ]
                = (unsigned char) args[ 1 ].vinteger;
            }
        else
        {
            goto argument_type_error;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_charAt )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        result_return.MakeString( vm, NULL, 1 );

        int i = args[ 1 ].vinteger;

        if ( i >= n->vstring->len )
            result_return.vstring->len = 0;
        else
            result_return.vstring->data[ 0 ] = n->vstring->data[ i ];

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_charCodeAt )
    {
        if ( args->vinteger != 0 && args->vinteger != 1 )
            goto argument_error;

        int i = 0;
        if ( args->vinteger == 1 )
        {
            if ( args[ 1 ].type != JS_INTEGER )
                goto argument_type_error;

            i = args[ 1 ].vinteger;
            }

        if ( i >= n->vstring->len )
        {
            vm->RaiseError( "String.%s(): index out of range", vm->Symname( method ) );
            }

        result_return.type = JS_INTEGER;
        result_return.vinteger = n->vstring->data[ i ];

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_concat )
    {
        // Count the new length
        //
        int nlen = n->vstring->len;

        for ( int i = 0; i < args->vinteger; i++ )
        {
            if ( args[ i + 1 ].type != JS_STRING )
                goto argument_type_error;

            nlen += args[ i + 1 ].vstring->len;
            }

        result_return.MakeString( vm, NULL, nlen );

        memcpy (result_return.vstring->data, n->vstring->data,
                n->vstring->len );

        // Append the argumens
        //
        int pos = n->vstring->len;

        for ( i = 0; i < args->vinteger; i++ )
        {
            memcpy( result_return.vstring->data + pos,
                    args[ i + 1 ].vstring->data, args[ i + 1 ].vstring->len );

            pos += args[ i + 1 ].vstring->len;
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_crc32 )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_INTEGER;
        result_return.vinteger = jsCRC32( n->vstring->data, n->vstring->len );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_indexOf )
    {
        if ( args->vinteger < 1 || args->vinteger > 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        int start_index = 0;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER )
                goto argument_type_error;

            start_index = args[ 2 ].vinteger;
            }

        result_return.type = JS_INTEGER;
        result_return.vinteger = -1;

        if ( start_index >= 0
            && start_index + args[ 1 ].vstring->len <= n->vstring->len )
        {
            // Use the Brute Force Luke!
            //
            for ( ; start_index + args[1].vstring->len <= n->vstring->len; start_index++ )
            {
                if ( memcmp( n->vstring->data + start_index,
                        args[ 1 ].vstring->data,
                        args[ 1 ].vstring->len) == 0 )
                {
                    result_return.vinteger = start_index;
                    break;
                    }
                }
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_lastIndexOf )
    {
        if ( args->vinteger < 1 || args->vinteger > 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        int start_index = n->vstring->len - args[ 1 ].vstring->len;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER )
                goto argument_type_error;

            start_index = args[ 2 ].vinteger;
            }

        result_return.type = JS_INTEGER;
        result_return.vinteger = -1;

        if ( start_index >= 0
            && start_index + args[ 1 ].vstring->len <= n->vstring->len )
        {
            for (; start_index >= 0; start_index-- )
            {
                if ( memcmp( n->vstring->data + start_index,
                            args[ 1 ].vstring->data,
                            args[ 1 ].vstring->len ) == 0 )
                {
                    result_return.vinteger = start_index;
                    break;
                    }
                }
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_slice )
    {
        if ( args->vinteger != 1 && args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        int start = args[ 1 ].vinteger;

        if ( start < 0 )
            start += n->vstring->len;
        if ( start < 0 )
            start = 0;

        if ( start > n->vstring->len )
            start = n->vstring->len;

        int end = n->vstring->len;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER )
                goto argument_type_error;

            end = args[ 2 ].vinteger;

            if ( end < 0 )
                end += n->vstring->len;

            if ( end < 0 )
                end = 0;

            if ( end > n->vstring->len )
                end = n->vstring->len;
            }

        if ( start > end )
            end = start;

        result_return.MakeString( vm, n->vstring->data + start, end - start );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_split )
    {
        if ( args->vinteger == 0 )
        {
            result_return.MakeArray( vm, 1);
            result_return.varray->data[ 0 ].MakeString( vm, n->vstring->data, n->vstring->len );
            }
        else
        {
            if ( args->vinteger != 1 && args->vinteger != 2 )
                goto argument_error;

            int limit = -1;

            if ( args->vinteger == 2 )
            {
                if ( args[ 2 ].type != JS_INTEGER )
                    goto argument_type_error;

                limit = args[ 2 ].vinteger;
                }

            if ( args[ 1 ].type == JS_STRING )
            {
                int alen = 0;

                result_return.MakeArray( vm, alen );

                int start = 0;

                for ( int pos = 0; alen < limit && pos + args[1].vstring->len <= n->vstring->len; )
                {
                    if ( memcmp ( n->vstring->data + pos,
                                 args[ 1 ].vstring->data,
                                 args[ 1 ].vstring->len) != 0 )
                    {
                        pos++;
                        }
                    else
                    {
                        // Found the separator
                        //
                        result_return.ExpandArray( vm, alen + 1);
                        result_return.varray->data[ alen ].MakeString( vm,
                                        n->vstring->data + start, pos - start );
                        alen++;

                        if ( args[ 1 ].vstring->len == 0 )
                        {
                            start = pos;
                            pos++;
                            }
                        else
                        {
                            pos += args[1].vstring->len;
                            start = pos;
                            }
                        }
                    }

                if ( alen < limit )
                {
                    // And finally, insert all leftovers
                    //
                    result_return.ExpandArray( vm, alen + 1 );
                    result_return.varray->data[ alen ].MakeString( vm,
                                    n->vstring->data + start,
                                    n->vstring->len - start );
                    }
                }

            else
            {
                goto argument_type_error;
                }
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_substr )
    {
        if ( args->vinteger != 1 && args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        int start = args[ 1 ].vinteger;
        int length = n->vstring->len;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER)
                goto argument_type_error;

            length = args[ 2 ].vinteger;
            if ( length < 0 )
                length = 0;
            }

        if ( start < 0 )
            start += n->vstring->len;

        if ( start < 0 )
            start = 0;

        if ( start > n->vstring->len )
            start = n->vstring->len;

        if ( start + length > n->vstring->len )
            length = n->vstring->len - start;

        result_return.MakeString( vm, n->vstring->data + start, length );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_substring )
    {
        if ( args->vinteger != 1 && args->vinteger != 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        int start = args[ 1 ].vinteger;
        int end = n->vstring->len;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER )
                goto argument_type_error;

            end = args[ 2 ].vinteger;
            }

        if ( start < 0 )
            start = 0;

        if ( end > n->vstring->len )
            end = n->vstring->len;

        if ( start > end )
        {
            vm->RaiseError( "String.%s(): start index is bigger than end",
                            vm->Symname( method ) );
            }

        result_return.MakeString( vm, n->vstring->data + start, end - start );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_toLowerCase )
    {
        if ( args->vinteger != 0 )
            goto argument_type_error;

        result_return.MakeString( vm, n->vstring->data, n->vstring->len );

        for ( int i = 0; i < result_return.vstring->len; i++ )
        {
            int ch = result_return.vstring->data[ i ];
            if ( isupper( ch ) )
                result_return.vstring->data[ i ] = tolower( ch );
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_toUpperCase )
    {
        if ( args->vinteger != 0 )
            goto argument_type_error;

        result_return.MakeString( vm, n->vstring->data, n->vstring->len );

        for ( int i = 0; i < result_return.vstring->len; i++ )
        {
            int ch = result_return.vstring->data[ i ];
            if ( islower( ch ) )
                result_return.vstring->data[ i ] = toupper( ch );
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_unpack )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_STRING )
            goto argument_type_error;

        result_return.MakeArray( vm, 0);

        uint8* buffer = (uint8*)n->vstring->data;
        int buflen = n->vstring->len;

        int bufpos = 0;
        int result_len = 0;
        JSVariant *rnode;

        for ( int op = 0; op < args[ 1 ].vstring->len; op++ )
        {
            switch( args[ 1 ].vstring->data[ op ] )
            {
                case 'C':
                    UNPACK_NEED( 1 );
                    UNPACK_EXPAND ();
                    rnode->type = JS_INTEGER;
                    rnode->vinteger = buffer[ bufpos++ ];
                    break;

                case 'n':
                {
                    UNPACK_NEED( 2 );
                    UNPACK_EXPAND ();

                    JSUInt32 ui = buffer[ bufpos++ ];
                    ui <<= 8;
                    ui |= buffer[ bufpos++ ];

                    rnode->type = JS_INTEGER;
                    rnode->vinteger = ui;
                    }
                    break;

                case 'N':
                {
                    UNPACK_NEED( 4 );
                    UNPACK_EXPAND ();

                    JSUInt32 ui = buffer[ bufpos++ ];
                    ui <<= 8;
                    ui |= buffer[ bufpos++ ];
                    ui <<= 8;
                    ui |= buffer[ bufpos++ ];
                    ui <<= 8;
                    ui |= buffer[ bufpos++ ];

                    rnode->type = JS_INTEGER;
                    rnode->vinteger = ui;
                    }
                    break;

                case 'd':
                    UNPACK_NEED( sizeof( double ) );
                    UNPACK_EXPAND ();

                    rnode->type = JS_FLOAT;
                    memcpy( &rnode->vfloat, buffer + bufpos, sizeof( double ) );
                    bufpos += sizeof( double );
                    break;

                default:
                    ; // Silently ignore it
                }
            }

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "String.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "String %s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_String:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    if ( n && property == s_length )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = n->vstring->len;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set)
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

immutable:
    vm->RaiseError( "String.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_String:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger == 0 )
    {
        result_return.MakeString( vm, NULL, 0);
        }
    else if (args->vinteger == 1)
    {
        JSVariant* source = &args[ 1 ];
        JSVariant source_n;

        if ( args[ 1 ].type != JS_STRING )
        {
            args[ 1 ].ToString( vm, source_n );
            source = &source_n;
            }

        result_return.MakeString( vm, source->vstring->data, source->vstring->len );
        }
    else
    {
        vm->RaiseError( "new String(): illegal amount of arguments" );
        }

    // Set the [[Prototype]] and [[Class]] properties
    // FIXME: 15.8.2
    }

//
// The String class initialization entry
//

void
JSVirtualMachine:: BuiltinString( void )
{
    new(this) JSBuiltinInfo_String;
    }
