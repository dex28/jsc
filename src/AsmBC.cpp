#include "JSC.h"

///////////////////////////////////////////////////////////////////////////////
// Symbols and Constants database

class SymbConst_DB
{
public:

    struct HashBucket
    {
        HashBucket* hash_next; // in-hash list
        HashBucket* seq_next;  // sequential order
        int id;
        int type;
        void* data;
        long len;
        };

private:

    int hash_size;
    HashBucket** pHash;
    int* hash_lengths;

    JSC_Compiler* jsc;

public:

    long totalc;
    long next_id;

    HashBucket* first;  // sequential order
    HashBucket* last;

    SymbConst_DB( JSC_Compiler* arg_jsc, int arg_hash_size = 32 );
    ~SymbConst_DB( void );

    int lookup( int data_type, void* data, int data_len );

    void dump( void );
    };

SymbConst_DB:: SymbConst_DB( JSC_Compiler* arg_jsc, int arg_hash_size )
{
    jsc = arg_jsc;
    hash_size = arg_hash_size;

    pHash = new(jsc) HashBucket*[ hash_size ];

    memset( pHash, 0, hash_size * sizeof( HashBucket* ) );

    hash_lengths = new(jsc) int[ hash_size ];

    memset( hash_lengths, 0, hash_size * sizeof( int ) );

    first = NULL;
    last = NULL;

    next_id = 0;
    totalc = 0;
    }

SymbConst_DB:: ~SymbConst_DB( void )
{
    }

void
SymbConst_DB:: dump( void )
{
    printf( "SymbConst_DB hash_size %d, next_id %ld, totalc %ld\n",
        hash_size, next_id, totalc );

    for ( int i = 0; i < hash_size; i++ )
    {
        printf( "%d %d\n", i, hash_lengths[ i ] );
        }
    }

int
SymbConst_DB:: lookup( int data_type, void* data, int data_len )
{
    totalc ++;

    int hash = jsHashFunction( data, data_len ) % hash_size;

    HashBucket* b = pHash[ hash ];

    for ( ; b != NULL; b = b->hash_next )
    {
        if ( b->type == data_type
            && b->len == data_len
            && memcmp( b->data, data, data_len ) == 0
            )
        {
            // Ok, we already have a bucket
            //
            return b->id;
            }
        }

    // Create a new bucket
    //
    b = new(jsc) HashBucket;
    b->id = next_id ++;
    b->type = data_type;
    b->len = data_len;
    b->data = new(jsc) unsigned char[ b->len ];
    memcpy( b->data, data, data_len );

    // make it hashed
    //
    b->hash_next = pHash[ hash ];
    pHash[ hash ] = b;

    hash_lengths[ hash ] ++;

    // make sequential scan possible
    //
    b->seq_next = NULL;
    if ( last )
    {
        last->seq_next = b;
        last = b;
        }
    else
    {
        first = b;
        last = b;
        }

    return b->id;
    }

void
JSC_Compiler:: asmWriteBC( const char* bc_filename )
{
    if ( options.verbose > 0 )
        trace( "generating byte-code" );

    SymbConst_DB consts( this, 32 );
    SymbConst_DB imports( this, 128 );
    long code_offset = 0;
    long ndbgline_entries = 0;
    long nexport_entries = 0;
    long debug_last_linenum = 0;

    for ( ASM_statement* item = asm_code.head; item; item = item->next )
    {
        item->offset = code_offset;

        if ( item->opcode == JS_OPCODE_SYMBOL )
        {
            nexport_entries++;
            }
        else if ( item->opcode == JS_OPCODE_LABEL )
        {
            }
        else
        {
            // Emitted assembler opcode
            //
            switch( item->arg_type )
            {
                case JS_OPARG_INT32:
                    code_offset += 2;
                    break;

                case JS_OPARG_CONST:
                {
                    ASM_constArg* a = (ASM_constArg*)item;
                    if ( a->type == JS_INTEGER )
                    {
                        a->id = consts.lookup( JS_INTEGER, &a->vinteger, sizeof( a->vinteger ) );
                        }
                    else if ( a->type == JS_FLOAT )
                    {
                        a->id = consts.lookup( JS_FLOAT, &a->vfloat, sizeof( a->vfloat ) );
                        }
                    else if ( a->type == JS_STRING )
                    {
                        a->id = consts.lookup( JS_STRING, a->vstring->data, a->vstring->len );
                        }
                    }
                    code_offset += 2;
                    break;

                case JS_OPARG_SYMBOL:
                {
                    ASM_symbolArg* a = (ASM_symbolArg*)item;
                    a->id = imports.lookup( JS_STRING, a->name->data, a->name->len );
                    }
                    code_offset += 2;
                    break;

                case JS_OPARG_LABEL:
                    code_offset += 2;
                    break;

                case JS_OPARG_none:
                    code_offset += 1;
                    break;
                }

            if ( item->linenum != debug_last_linenum )
            {
                ndbgline_entries ++;
                debug_last_linenum = item->linenum;
                }
            }
        }

    FILE* f = fopen( bc_filename, "wb" );

    long tot_out = 0;

    static const unsigned char magic[ 8 ] = "#! JSC\n";
    tot_out += fwrite( magic, 1, sizeof( magic ), f ); 
    tot_out += fwrite( &code_offset, 1, sizeof( code_offset ), f );
    tot_out += fwrite( &consts.next_id, 1, sizeof( consts.next_id ), f );
    tot_out += fwrite( &ndbgline_entries, 1, sizeof( ndbgline_entries ), f );
    tot_out += fwrite( &imports.next_id, 1, sizeof( imports.next_id ), f );
    tot_out += fwrite( &nexport_entries, 1, sizeof( nexport_entries ), f );

    // Source filename
    //
    long len = strlen( stream.name );
    tot_out += fwrite( &len, 1, sizeof( long ), f );
    tot_out += fwrite( stream.name, 1, len, f );

    trace( "HEADER: %ld bytes", tot_out );
    tot_out = 0;

    // Write code
    //
    for ( item = asm_code.head; item; item = item->next )
    {
        if ( item->opcode != JS_OPCODE_SYMBOL && item->opcode != JS_OPCODE_LABEL )
        {
            unsigned char op = (unsigned char) item->opcode;
            tot_out += fwrite( &op, 1, sizeof( op ), f );

            switch( item->arg_type )
            {
                case JS_OPARG_INT32:
                {
                    long val = value_int32( item );
                    tot_out += fwrite( &val, 1, sizeof( val ), f );
                    }
                    break;

                case JS_OPARG_CONST:
                    tot_out += fwrite( &( ( (ASM_constArg*)item )->id ), 1, sizeof( long ), f );
                    break;

                case JS_OPARG_SYMBOL:
                    tot_out += fwrite( &( ( (ASM_symbolArg*)item )->id ), 1, sizeof( long ), f );
                    break;

                case JS_OPARG_LABEL:
                    tot_out += fwrite( &( value_label( item )->offset ), 1, sizeof( long ), f );
                    break;

                case JS_OPARG_none:
                    break;
                }
            }
        }

    trace( "CODE: %ld bytes in %ld entries", tot_out, code_offset );
    tot_out = 0;

    // Write constants
    //
    for ( SymbConst_DB::HashBucket* p = consts.first; p; p = p->seq_next )
    {
        unsigned char type = (unsigned char) p->type;
        tot_out += fwrite( &type, 1, sizeof( type ), f );

        if ( p->type == JS_STRING )
        {
            tot_out += fwrite( &p->len, 1, sizeof( p->len ), f );
            }

        tot_out += fwrite( p->data, 1, p->len, f );
        }

    trace( "CONSTS: %ld bytes in %ld entries", tot_out, consts.next_id );
    tot_out = 0;

    // Write debugging info
    //
    debug_last_linenum = 0;
    for ( item = asm_code.head; item; item = item->next )
    {
        if ( item->opcode != JS_OPCODE_SYMBOL
            && item->opcode != JS_OPCODE_LABEL
            && item->linenum != debug_last_linenum
            )
        {
            tot_out += fwrite( &item->offset, 1, sizeof( long ), f );
            tot_out += fwrite( &item->linenum, 1, sizeof( long ), f );
            debug_last_linenum = item->linenum;
            }
        }

    trace( "DEBUG: %ld bytes in %ld entries", tot_out, ndbgline_entries );
    tot_out = 0;

    // Write imported-symbol table
    //
    for ( p = imports.first; p; p = p->seq_next )
    {
        unsigned char type = 0;
        if ( p->len >= 3 && memcmp( p->data, ".F:", 3 ) == 0 )
            type = 0xFF;

        tot_out += fwrite( &type, 1, sizeof( type ), f );

        tot_out += fwrite( &p->len, 1, sizeof( long ), f );
        tot_out += fwrite( p->data, 1, p->len, f );
        }

    trace( "IMPORT: %ld bytes in %ld entries", tot_out, imports.next_id );
    tot_out = 0;

    // Write exported-symbol table
    //
    for ( item = asm_code.head; item; item = item->next )
    {
        if ( item->opcode == JS_OPCODE_SYMBOL )
        {
            String* v = ( (ASM_symbolDef*)item )->value;

            unsigned char type = 0;
            if ( v->len >= 3 && memcmp( v->data, ".F:", 3 ) == 0 )
                type = 0xFF;

            tot_out += fwrite( &type, 1, sizeof( type ), f );

            tot_out += fwrite( &item->offset, 1, sizeof( item->offset ), f );
            tot_out += fwrite( &v->len, 1, sizeof( long ), f );
            tot_out += fwrite( v->data, 1, v->len, f );
            }
        }

    trace( "EXPORT: %ld bytes in %ld entries", tot_out, nexport_entries );

    fclose( f );

    // consts.dump ();
    // imports.dump ();
    }

