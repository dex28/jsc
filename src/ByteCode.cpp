//
// Byte code handling routines.
//

#include "JS.h"

const JSOpcodeDesc OpcodeDesc [] =
{
    { "nop",             JS_OPARG_none,     0, },
    { "dup",             JS_OPARG_none,     1, },
    { "pop",             JS_OPARG_none,    -1, },
    { "pop_n",           JS_OPARG_INT32,   -1, },
    { "swap",            JS_OPARG_none,     0, },
    { "roll",            JS_OPARG_INT32,    0, },
    { "const",           JS_OPARG_CONST,    1, },
    { "const_null",      JS_OPARG_none,     1, },
    { "const_undefined", JS_OPARG_none,     1, },
    { "const_true",      JS_OPARG_none,     1, },
    { "const_false",     JS_OPARG_none,     1, },
    { "const_i0",        JS_OPARG_none,     1, },
    { "const_i1",        JS_OPARG_none,     1, },
    { "const_i2",        JS_OPARG_none,     1, },
    { "const_i3",        JS_OPARG_none,     1, },
    { "const_i",         JS_OPARG_INT32,    1, },
    { "locals",          JS_OPARG_INT32,    1, },
    { "apop",            JS_OPARG_INT32,   -1, },
    { "min_args",        JS_OPARG_INT32,   -1, },
    { "load_arg",        JS_OPARG_INT32,    1, },
    { "store_arg",       JS_OPARG_INT32,   -1, },
    { "load_nth_arg",    JS_OPARG_none,     0, },
    { "load_global",     JS_OPARG_SYMBOL,   1, },
    { "load_global_w",   JS_OPARG_SYMBOL,   1, },
    { "store_global",    JS_OPARG_SYMBOL,  -1, },
    { "load_local",      JS_OPARG_INT32,    1, },
    { "store_local",     JS_OPARG_INT32,   -1, },
    { "load_property",   JS_OPARG_SYMBOL,   0, },
    { "store_property",  JS_OPARG_SYMBOL,  -2, },
    { "delete_property", JS_OPARG_SYMBOL,   0, },
    { "load_array",      JS_OPARG_none,    -1, },
    { "store_array",     JS_OPARG_none,    -3, },
    { "delete_array",    JS_OPARG_none,    -1, },
    { "nth",             JS_OPARG_none,     0, },
    { "cmp_eq",          JS_OPARG_none,    -1, },
    { "cmp_ne",          JS_OPARG_none,    -1, },
    { "cmp_lt",          JS_OPARG_none,    -1, },
    { "cmp_gt",          JS_OPARG_none,    -1, },
    { "cmp_le",          JS_OPARG_none,    -1, },
    { "cmp_ge",          JS_OPARG_none,    -1, },
    { "cmp_seq",         JS_OPARG_none,    -1, },
    { "cmp_sne",         JS_OPARG_none,    -1, },
    { "add_1_i",         JS_OPARG_none,     0, },
    { "add_2_i",         JS_OPARG_none,     0, },
    { "sub",             JS_OPARG_none,    -1, },
    { "add",             JS_OPARG_none,    -1, },
    { "mul",             JS_OPARG_none,    -1, },
    { "div",             JS_OPARG_none,    -1, },
    { "mod",             JS_OPARG_none,    -1, },
    { "neg",             JS_OPARG_none,    -1, },
    { "not",             JS_OPARG_none,    -1, },
    { "and",             JS_OPARG_none,    -1, },
    { "or",              JS_OPARG_none,    -1, },
    { "xor",             JS_OPARG_none,    -1, },
    { "shift_left",      JS_OPARG_none,    -1, },
    { "shift_right",     JS_OPARG_none,    -1, },
    { "shift_rright",    JS_OPARG_none,    -1, },
    { "halt",            JS_OPARG_none,     0, },
    { "done",            JS_OPARG_none,     0, },
    { "iffalse",         JS_OPARG_LABEL,   -1, },
    { "iftrue",          JS_OPARG_LABEL,   -1, },
    { "iffalse_b",       JS_OPARG_LABEL,   -1, },
    { "iftrue_b",        JS_OPARG_LABEL,   -1, },
    { "call_method",     JS_OPARG_SYMBOL,   1, },
    { "jmp",             JS_OPARG_LABEL,    0, },
    { "jsr",             JS_OPARG_none,     1, },
    { "jsr_w",           JS_OPARG_SYMBOL,   1, },
    { "return",          JS_OPARG_none,     0, },
    { "typeof",          JS_OPARG_none,     0, },
    { "new",             JS_OPARG_none,     1, },
    { "with_push",       JS_OPARG_none,    -1, },
    { "with_pop",        JS_OPARG_INT32,    0, },
    { "try_push",        JS_OPARG_LABEL,    0, },
    { "try_pop",         JS_OPARG_INT32,    0, },
    { "throw",           JS_OPARG_none,    -1, },
    { "symbol",          JS_OPARG_none,     0, },
    { "label",           JS_OPARG_none,     0, },
    };

JSByteCode:: JSByteCode( void )
{
    code = NULL;
    code_len = 0;

    consts = NULL;
    const_count = 0;

    dbg_filename = NULL;
    dbginfo = NULL;
    dbginfo_count = 0;

    i_symb = NULL;
    i_symb_count = 0;

    e_symb = NULL;
    e_symb_count = 0;

    usage_count = 0;
    }

bool
JSByteCode:: Load( PCSTR filename )
{
    FILE* f = fopen( filename, "rb" );
    if ( ! f )
        return false;

    unsigned char magic[ 8 ];
    fread( magic, 1, sizeof( magic ), f );

    fread( &code_len, 1, sizeof( long ), f );
    fread( &const_count, 1, sizeof( long ), f );
    fread( &dbginfo_count, 1, sizeof( long ), f );
    fread( &i_symb_count, 1, sizeof( long ), f );
    fread( &e_symb_count, 1, sizeof( long ), f );

    // Source filename
    //
    long dbg_filename_len;
    fread( &dbg_filename_len, 1, sizeof( long ), f );
    dbg_filename = NEW char[ dbg_filename_len + 1 ];
    fread( dbg_filename, 1, dbg_filename_len, f );
    dbg_filename[  dbg_filename_len ] = 0;

    code = NEW Compiled[ code_len + 1 ];
    consts = NEW JSVariant[ const_count ];
    dbginfo = NEW DebugInfo[ dbginfo_count ];
    i_symb = NEW PSTR[ i_symb_count ];
    e_symb = NEW Entry[ e_symb_count ];

    // Read code
    //
    for ( long i = 0; i < code_len; i++ )
    {
        unsigned char op;
        fread( &op, 1, sizeof( op ), f );

        if ( op < 0 || op > JS_OPCODE_LAST_OPCODE )
        {
            // We have encountered invalid opcode, so bytecode is invalid.
            // Anyway, JSVM would complain also (as well on jumps on invalid opcodes).
            //
            fclose( f );
            return false;
            }

        code[ i ].op = JSOpCode( op );

        switch( OpcodeDesc[ op ].arg_type )
        {
            case JS_OPARG_INT32:
            {
                JSInt32 val;
                fread( &val, 1, sizeof( val ), f );
                code[ ++i ].i32 = val;
                }
                break;

            case JS_OPARG_CONST:
            {
                JSInt32 val;
                fread( &val, 1, sizeof( val ), f );
                code[ ++i ].variant = &consts[ val ];
                }
                break;

            case JS_OPARG_SYMBOL:
            {
                JSInt32 val;
                fread( &val, 1, sizeof( val ), f );
                code[ ++i ].symbol = val;
                }
                break;

            case JS_OPARG_LABEL:
            {
                JSInt32 val;
                fread( &val, 1, sizeof( val ), f );
                code[ ++i ].label = &code[ val ];
                }
                break;

            case JS_OPARG_none:
                break;
            }
        }

    // Terminate the code with op `done'
    //
    code[ code_len ].op = JS_OPCODE_DONE;

    // Read constants
    //
    for ( i = 0; i < const_count; i++ )
    {
        unsigned char type;
        fread( &type, 1, sizeof( type ), f );

        switch ( type )
        {
            case JS_STRING:
            {
                long len;
                fread( &len, 1, sizeof( long ), f );
                consts[ i ].type = JS_STRING;
                consts[ i ].vstring = NEW JSString;
                consts[ i ].vstring->data = NEW char[ len + 1 ];
                consts[ i ].vstring->len = len;
                consts[ i ].vstring->flags = JSSTRING_DONT_GC;
                consts[ i ].vstring->prototype = NULL;
                if ( len > 0 )
                {
                    fread( consts[ i ].vstring->data, 1, len, f );
                    }
                consts[ i ].vstring->data[ len ] = 0;
                }
                break;

            case JS_INTEGER:
                consts[ i ].type = JS_INTEGER;
                fread( &consts[ i ].vinteger, 1, sizeof( long ), f );
                break;

            case JS_FLOAT:
                consts[ i ].type = JS_FLOAT;
                fread( &consts[ i ].vfloat, 1, sizeof( double ), f );
                break;
            }
        }

    // Read debugging info
    //
    for ( i = 0; i < dbginfo_count; i++ )
    {
        long offset;
        fread( &offset, 1, sizeof( long ), f );
        fread( &( dbginfo[ i ].linenum ), 1, sizeof( long ), f );
        dbginfo[ i ].pc = &code[ offset ];
        }

    // Read imported-symbol table
    //
    for ( i = 0; i < i_symb_count; i++ )
    {
        unsigned char type;
        fread( &type, 1, sizeof( type ), f );

        long len;
        fread( &len, 1, sizeof( long ), f );

        i_symb[ i ] = NEW char[ len + 1 ];

        fread( i_symb[ i ], 1, len, f );
        i_symb[ i ][ len ] = 0;
        }

    // Read exported-symbol table
    //
    long last_offset;
    for ( i = 0; i < e_symb_count; i++ )
    {
        unsigned char type;
        fread( &type, 1, sizeof( type ), f );

        long offset;
        fread( &offset, 1, sizeof( long ), f );
        e_symb[ i ].code = &code[ offset ];
        if ( i > 0 )
        {
            e_symb[ i - 1 ].code_len = offset - last_offset;
            }
        last_offset = offset;

        long len;
        fread( &len, 1, sizeof( long ), f );

        e_symb[ i ].name = NEW char[ len + 1 ];
        
        fread( e_symb[ i ].name, 1, len, f );
        e_symb[ i ].name[ len ] = 0;

        e_symb[ i ].dbginfo = NULL;
        e_symb[ i ].dbginfo_count = 0;
        }

    if ( e_symb_count > 0 )
    {
        e_symb[ e_symb_count - 1 ].code_len = code_len - last_offset;
        }

    // Calculate dbginfo lines ranges of entries
    //
    for ( i = 0; i < e_symb_count; i++ )
    {
        int di_start = -1;
        int di_end = -1;

        for ( int j = 0; j < dbginfo_count; j++ )
        {
            if ( di_start < 0 )
            {
                if ( dbginfo[ j ].pc > e_symb[ i ].code )
                {
                    di_start = j - 1;
                    }
                }

            if ( di_end < 0 )
            {
                if ( dbginfo[ j ].pc >= e_symb[ i ].code + e_symb[ i ].code_len )
                {
                    di_end = j;
                    }
                }

            // We are done with this entry when we have both
            // the first and the last line.
            //
            if ( di_start >= 0 && di_end >= 0 )
            {
                break;
                }
            }

        if ( di_end < 0 )
        {
            di_end = dbginfo_count;
            }

        e_symb[ i ].dbginfo = dbginfo + di_start;
        e_symb[ i ].dbginfo_count = di_end - di_start;
        }

    fclose( f );

    return true;
    }

bool
JSByteCode:: Save( PCSTR filename )
{
    FILE* f = fopen( filename, "wb" );
    if ( ! f )
        return false;

    static const unsigned char magic[ 8 ] = "#! JSC\n";
    fwrite( magic, 1, sizeof( magic ), f );

    fwrite( &code_len, 1, sizeof( long ), f );
    fwrite( &const_count, 1, sizeof( long ), f );
    fwrite( &dbginfo_count, 1, sizeof( long ), f );
    fwrite( &i_symb_count, 1, sizeof( long ), f );
    fwrite( &e_symb_count, 1, sizeof( long ), f );

    // Write source filename
    //
    long dbg_filename_len = strlen( dbg_filename );
    fwrite( &dbg_filename_len, 1, sizeof( long ), f );
    fwrite( dbg_filename, 1, dbg_filename_len, f );

    // Write code
    //
    for ( long i = 0; i < code_len; i++ )
    {
        unsigned char op = code[ i ].op;
        fwrite( &op, 1, sizeof( op ), f );

        switch( OpcodeDesc[ code[ i ].op ].arg_type )
        {
            case JS_OPARG_INT32:
            {
                JSInt32 val = code[ ++i ].i32;
                fwrite( &val, 1, sizeof( val ), f );
                }
                break;

            case JS_OPARG_CONST:
            {
                JSInt32 val = code[ ++i ].variant - consts;
                fwrite( &val, 1, sizeof( val ), f );
                }
                break;

            case JS_OPARG_SYMBOL:
            {
                JSInt32 val = code[ ++i ].symbol;
                fwrite( &val, 1, sizeof( val ), f );
                }
                break;

            case JS_OPARG_LABEL:
            {
                JSInt32 val = code[ ++i ].label - code;
                fwrite( &val, 1, sizeof( val ), f );
                }
                break;

            case JS_OPARG_none:
                break;
            }
        }

    // Write constants
    //
    for ( i = 0; i < const_count; i++ )
    {
        unsigned char type = consts[ i ].type;
        fwrite( &type, 1, sizeof( type ), f );

        switch ( consts[ i ].type )
        {
            case JS_STRING:
            {
                long len = consts[ i ].vstring->len;
                fwrite( &len, 1, sizeof( long ), f );
                fwrite( consts[ i ].vstring->data, 1, len, f );
                }
                break;

            case JS_INTEGER:
                fwrite( &consts[ i ].vinteger, 1, sizeof( long ), f );
                break;

            case JS_FLOAT:
                fwrite( &consts[ i ].vfloat, 1, sizeof( double ), f );
                break;
            }
        }

    // Write debugging info
    //
    for ( i = 0; i < dbginfo_count; i++ )
    {
        long offset = dbginfo[ i ].pc - code;
        fwrite( &offset, 1, sizeof( long ), f );
        fwrite( &( dbginfo[ i ].linenum ), 1, sizeof( long ), f );
        }

    // Write imported-symbol table
    //
    for ( i = 0; i < i_symb_count; i++ )
    {
        unsigned char type = 0;
        if ( memcmp( i_symb[ i ], ".F:", 3 ) == 0 )
            type = 0xFF;

        fwrite( &type, 1, sizeof( type ), f );

        long len = strlen( i_symb[ i ] );
        fwrite( &len, 1, sizeof( long ), f );

        fwrite( i_symb[ i ], 1, len, f );
        }

    // Write exported-symbol table
    //
    for ( i = 0; i < e_symb_count; i++ )
    {
        unsigned char type = 0;
        if ( memcmp( e_symb[ i ].name, ".F:", 3 ) == 0 )
            type = 0xFF;

        fwrite( &type, 1, sizeof( type ), f );

        long offset = e_symb[ i ].code - code;
        fwrite( &offset, 1, sizeof( long ), f );

        long len = strlen( e_symb[ i ].name );
        fwrite( &len, 1, sizeof( long ), f );

        fwrite( e_symb[ i ].name, 1, len, f );
        }

    fclose( f );

    return true;
    }

JSByteCode:: ~JSByteCode( void )
{
    for ( int i = 0; i < e_symb_count; i++ )
    {
        delete e_symb[ i ].name;
        }

    for ( i = 0; i < i_symb_count; i++ )
    {
        delete i_symb[ i ];
        }

    for ( i = 0; i < const_count; i++ )
    {
        if ( consts[ i ].type == JS_STRING )
        {
            delete consts[ i ].vstring->data;
            delete consts[ i ].vstring;
            }
        }

    delete e_symb;
    delete i_symb;
    delete dbginfo;
    delete consts;
    delete code;

    delete dbg_filename;
    }

void
JSByteCode:: Dump( void )
{
    printf( "File:    \"%s\"\n", dbg_filename );
    printf( "CODE:    %ld entries\n", code_len );
    printf( "CONSTS:  %ld entries\n", const_count );
    printf( "DBGINFO: %ld entries\n", dbginfo_count );
    printf( "IMPORTS: %ld entries\n", i_symb_count );
    printf( "EXPORTS: %ld entries\n", e_symb_count );

    for ( long i = 0; i < code_len; i++ )
    {
        printf( "%p: %-16s", &code[ i ], OpcodeDesc[ code[ i ].op ].desc );

        switch( OpcodeDesc[ code[ i ].op ].arg_type )
        {
            case JS_OPARG_INT32:
                ++i;
                printf( "%ld\n", code[ i ].i32 );
                break;

            case JS_OPARG_CONST:
                ++i;
                switch( code[ i ].variant->type )
                {
                    case JS_STRING:
                        printf( "\"%s\"\n", code[ i ].variant->vstring->data );
                        break;
                    case JS_FLOAT:
                        printf( "%lf\n", code[ i ].variant->vfloat );
                        break;
                    case JS_INTEGER:
                        printf( "%lf\n", code[ i ].variant->vinteger );
                        break;
                    default:
                        printf( "ERROR\n" );
                    }
                break;

            case JS_OPARG_SYMBOL:
                ++i;
                printf( "[%d] %s\n", code[ i ].i32, i_symb[ code[ i ].i32 ] );
                break;

            case JS_OPARG_LABEL:
                ++i;
                printf( "%p\n", code[ i ].label );
                break;

            case JS_OPARG_none:
                printf( "\n" );
                break;
            }
        }

    for ( i = 0; i < e_symb_count; i++ )
    {
        printf( "%p %04ld: %-36s  {%ld,%ld}\n",
            e_symb[ i ].code,
            e_symb[ i ].code_len,
            e_symb[ i ].name,
            e_symb[ i ].dbginfo[ 0 ].linenum,
            e_symb[ i ].dbginfo[ e_symb[ i ].dbginfo_count - 1 ].linenum
            );
        }

    for ( i = 0; i < dbginfo_count; i++ )
    {
        printf( "%p %04ld\n", dbginfo[ i ].pc, dbginfo[ i ].linenum );
        }
    }
