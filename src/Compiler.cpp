#include "JSC.h"

bool
JSC_Compile
(
    JSC_CompilerOptions& co,
    const char* js_filename,
    const char* bc_filename,
    const char* asm_filename
    )
{
    bool fStdinToFree = false;
    bool fStdoutToFree = false;

    if ( co.s_stdin == NULL )
    {
        fStdinToFree = true;
        co.s_stdin = NEW JSIOStreamFile( js_filename, 1, 0 );

        if ( ! JSIOStream::IsValid( co.s_stdin ) )
            return false;
        }

    if ( co.s_stdout == NULL )
    {
        fStdoutToFree = true;
        co.s_stdout = NEW JSIOStreamFile( stderr, 0, 1 );

        if ( ! JSIOStream::IsValid( co.s_stdout ) )
            return false;

        co.s_stdout->SetAutoFlush ();
        }

    {
        JSC_Compiler jsc;
        jsc.options = co;

        jsc.stream.name = js_filename;
        jsc.stream.stream = co.s_stdin;

        try
        {
            // Prefetch token lookahead cache
            //
            jsc.readToken( jsc.next_token );
            jsc.readToken( jsc.next_token2 );

            jsc.parseProgram ();

            jsc.asmGenerate ();
            }
        catch( int )
        {
            return false;
            }

        // We don't need the syntax tree anymore.
        // Free it and save some memory. TODO
        //
        //jsc.asmOptimize ();

        if ( asm_filename != NULL )
        {
            jsc.asmWriteASM( asm_filename );
            }

        if ( bc_filename != NULL )
        {
            jsc.asmWriteBC( bc_filename );
            }
        }

    if ( fStdinToFree )
        delete co.s_stdin;

    if ( fStdoutToFree )
        delete co.s_stdout;

    return true;
    }

JSC_CompilerOptions:: JSC_CompilerOptions( void )
{
    verbose = 1;
    annotate_assembler = true;
    generate_debug_info = true;
    js_pages = false;

    optimize_constant_folding = true;
    optimize_peephole = true;
    optimize_jumps = true;
    optimize_bc_size = true;
    optimize_heavy = true;

    warn_unused_argument = true;
    warn_unused_variable = true;
    warn_shadow = true;
    warn_with_clobber = true;
    warn_missing_semicolon = true;
    warn_strict_ecma = false;
    warn_deprecated = true;

    s_stdin = NULL;
    s_stdout = NULL;
    }

JSC_Compiler:: JSC_Compiler( void )
{
    num_tokens ++;

    cur_token.id = -1;
    next_token.id = -1;

    num_tokens = 0;
    num_missing_semicolons = 0;
    jspCode = false;

    nested_foo_count = 0;
    anonymous_foo_count = 0;

    pTop = NULL;

    ns.Create( this );
    cont_break.Create( this );
    }

JSC_Compiler:: ~JSC_Compiler( void )
{
    long alloc_size = 0;
    long free_size = 0;
    long overhead_size = 0;

    HeapBlock* hb_next;
    for ( HeapBlock* hb = pTop; hb; hb = hb_next )
    {
        hb_next = hb->next;
        alloc_size += hb->alloc_size;
        free_size += hb->free_size;
        overhead_size += sizeof( HeapBlock );
        free( hb );
        }

    trace( "JSC heap: allocated %lu, wasted %lu, overhead %lu",
        alloc_size, free_size, overhead_size );
    }

void
JSC_Compiler:: trace( const char* fmt... )
{
    char line[ 1024 ];

    char* chp = line;
    chp += sprintf( chp, "trace: " );

    va_list marker;
    va_start( marker, fmt );
    chp += vsprintf( chp, fmt, marker );
    va_end( marker );

    chp += sprintf( chp, "\n" );

    if ( options.s_stdout )
    {
        options.s_stdout->Write( line, chp - line );
        }
    }

void
JSC_Compiler:: warning( int ln, const char* fmt... )
{
    char line[ 1024 ];

    char* chp = line;
    chp += sprintf( chp, "warning %s(%d): ", stream.name, ln );

    va_list marker;
    va_start( marker, fmt );
    chp += vsprintf( chp, fmt, marker );
    va_end( marker );

    chp += sprintf( chp, "\n" );

    if ( options.s_stdout )
    {
        options.s_stdout->Write( line, chp - line );
        }
    }

void
JSC_Compiler:: error( int ln, const char* fmt... )
{
    if ( cur_token.id == -1 )
    {
        getToken ();
        ln = cur_token.linenum;
        }

    char line[ 1024 ];

    char* chp = line;
    chp += sprintf( chp, "error %s(%d): ", stream.name, ln );

    va_list marker;
    va_start( marker, fmt );
    chp += vsprintf( chp, fmt, marker );
    va_end( marker );

    chp += sprintf( chp, "\n" );

    if ( options.s_stdout )
    {
        options.s_stdout->Write( line, chp - line );
        }

    dumpLexerStatus ();

    throw 1;
    }

void
JSC_Compiler:: asmWriteASM( const char* asm_filename )
{
    FILE* outf = fopen( asm_filename, "wt" );

    if ( ! options.annotate_assembler )
    {
        for ( ASM_statement* stmt = asm_code.head; stmt; stmt = stmt->next )
        {
            stmt->print( outf );
            }
        }
    else
    {
        // Fix the label line numbers to be the same that the next
        // assembler operand has.
        //
        int last_ln = 0;
        for ( ASM_statement* stmt = asm_code.tail; stmt; stmt = stmt->prev )
        {
            if ( stmt->opcode == JS_OPCODE_LABEL )
                stmt->linenum = last_ln;
            else if ( stmt->linenum != 0 )
                last_ln = stmt->linenum;
            }

        stream.rewind ();

        last_ln = 0;

        for ( stmt = asm_code.head; stmt; stmt = stmt->next )
        {
            while ( stmt->linenum > last_ln )
            {
                char line[ 1000 ];
                stream.readLine( line, sizeof( line ) );

                fprintf( outf, "; %04ld %s\n", ++last_ln, line );
                }

            if ( stmt->linenum < last_ln )
            {
                //fprintf( outf, "; %04ld\n", stmt->linenum );
                }

            stmt->print( outf );
            }
/*
        last_ln = 0;

        for ( ;; )
        {
            char line[ 1000 ];
            if ( -1 == stream->readLine( line, sizeof( line ) ) )
                break;

            fprintf( outf, "; %04ld %s\n", ++last_ln, line );

            for ( stmt = asm_code.head; stmt; stmt = stmt->next )
            {
                if ( stmt->linenum == last_ln )
                    stmt->print( outf );
                }

            if ( last_ln % 100 == 0 )
                printf( " %ld\r", last_ln );
            }
*/
        }

    fclose( outf );
    }
