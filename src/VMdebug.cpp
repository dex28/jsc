//
// Debugging utilities.
//
// JSVirtualMachine:: FunctionAt
// JSVirtualMachine:: DebugPosition
// JSVirtualMachine:: StackTrace
//

#include "JS.h"

JSFunction*
JSVirtualMachine:: FunctionAt( Compiled* pc )
{
    // Check the globals
    //
    for ( int i = 0; i < num_globals; i++ )
    {
        if ( globals[ i ].type == JS_FUNCTION )
        {
            JSFunction* f = globals[ i ].vfunction;

            if ( f->code <= pc && pc < f->code + f->code_len )
            {
                return f;
                }
            }
        }

    // No luck. Let's try the stack
    //
    for ( JSVariant* sp = SP + 1; sp < stack + stack_size; sp++ )
    {
        if ( sp->type == JS_FUNCTION )
        {
            JSFunction* f = sp->vfunction;

            if ( f->code <= pc && pc < f->code + f->code_len )
            {
                return f;
                }
            }
        }

    // Still no matches. This shouldn't be reached... ok, who cares?
    //
    assert( 1 );

    return NULL;
    }

JSFunction*
JSVirtualMachine:: DebugPosition( Compiled* pc, PCSTR& fname_return, int& linenum_return )
{
    JSFunction* f = FunctionAt( pc );

    if ( f == NULL )
    {
        return NULL;
        }

    if ( f->dbg_filename == NULL )
    {
        // No debugging information available for this function
        //
        fname_return = NULL;
        return f;
        }

    // Find the correct pc position
    //
    int last_linenum = 0;

    for ( int j = 0; j < f->dbginfo_count; j++ )
    {
        if ( f->dbginfo[ j ].pc > pc )
        {
            break;
            }

        last_linenum = f->dbginfo[ j ].linenum;
        }

    linenum_return = last_linenum;
    fname_return = f->dbg_filename;

    return f;
    }

void
ShowVariant( JSIOStream* stream, JSVariant* n )
{
    switch ( n->type )
    {
        case JS_UNDEFINED:
            stream->Printf( "undefined" );
            break;

        case JS_NULL:
            stream->Printf( "null" );
            break;

        case JS_BOOLEAN:
            stream->Printf( "bool: %s", n->vboolean ? "true" : "false" );
            break;

        case JS_INTEGER:
            stream->Printf( "int: %ld", n->vinteger );
            break;

        case JS_STRING:
            stream->Printf( "str: \"%.*s\"", n->vstring->len, n->vstring->data );
            break;

        case JS_FLOAT:
            stream->Printf( "float: %g", n->vfloat );
            break;

        case JS_ARRAY:
            stream->Printf( "array" );
            break;

        case JS_OBJECT:
            stream->Printf( "object" );
            break;

        case JS_BUILTIN:
            stream->Printf( "builtin" );
            break;

        case JS_FUNCTION:
            stream->Printf( "function" );
            break;

        case JS_INSTR_PTR:
            stream->Printf( "instr_ptr: %p", n->instr_ptr );
            break;

        case JS_BASE_PTR:
            stream->Printf( "base_ptr: %p", n->base_ptr );
            break;

        case JS_FRAME_PTR:
            stream->Printf( "frame_ptr: %p", n->frame_ptr );
            break;

        case JS_WITH_CHAIN:
            stream->Printf( "with_chain" );
            break;

        case JS_ARGS_FIX:
            stream->Printf( "args_fix: count=%d, delta=%d",
                              n->args_fix.argc, n->args_fix.delta );
            break;

        default:
            stream->Printf( "type=%d(!?)", n->type );
            break;
        }
    }

void
JSVirtualMachine:: StackTrace( JSVariant* fp, JSVariant* fpEnd, Compiled* pc )
{
    // fp[ -4 ].instr_ptr:  The return address
    // fp[ -3 ].base_ptr:   Base pointer (imported-symbols bytecode linkage context)
    // fp[ -2 ].with_chain: The with-lookup chain pointer
    // fp[ -1 ].args_fix:   JS_ARGS_FIX variant
    // fp[ 0 ].frame_ptr:   The frame pointer
    // fp[ 1 ]:             "this" object variant
    // fp[ 2 ].vinteger:    argument count: argc
    // fp[ 2 + i ]:         argument value: argv[i]; i >= 0 && i < fp[2].vinteger

    assert( fp[ -4 ].type == JS_INSTR_PTR );
    assert( fp[ -3 ].type == JS_BASE_PTR );
    assert( fp[ -2 ].type == JS_WITH_CHAIN );
    assert( fp[ -1 ].type == JS_ARGS_FIX );
    assert( fp[ 0 ].type == JS_FRAME_PTR );

    PCSTR filename;
    int linenum;

    JSFunction* f = DebugPosition( pc, filename, linenum );
    assert( f != NULL );

    if ( filename )
    {
        s_stderr->PrintfLn( "%s(%d): %s", filename, linenum, f->name );
        }
    else
    {
        s_stderr->PrintfLn( "%s", f->name );
        }

    if ( ! options.verbose_stacktrace )
        return;

    if ( fp[ 2 ].type == JS_INTEGER )
    {
        for ( int i = fp[ 2 ].vinteger + 1; i >= 0; i-- )
        {
            s_stderr->Printf( "     %p ARG(%d)    ", fp + 1 + i, i );

            ShowVariant( s_stderr, fp + 1 + i );

            s_stderr->PrintfLn ();
            }
        }

	s_stderr->PrintfLn( "---> %p | OFP   | %p", fp, fp[ 0 ].frame_ptr );
	s_stderr->PrintfLn( "     %p | AF    | count=%ld,delta=%ld", fp - 1, fp[ -1 ].args_fix.argc, fp[ -1 ].args_fix.delta );
	s_stderr->PrintfLn( "     %p | WP    | %ld", fp - 2, fp[ -2 ].with_chain.cNode );
	s_stderr->PrintfLn( "     %p | BP    | %p", fp - 3, fp[ -3 ].base_ptr );
	s_stderr->PrintfLn( "     %p | RA    | %p", fp - 4, fp[ -4 ].instr_ptr );

    for ( JSVariant* n = fp - 5; n > fpEnd; n-- )
    {
        s_stderr->Printf( "     %p LOCAL(%d)  ", n, int( fp - 5 - n ) );

        ShowVariant( s_stderr, n );

        s_stderr->PrintfLn ();
        }
    }

void
JSVirtualMachine:: StackTrace( int num_frames )
{
    s_stderr->PrintfLn( "VM: Stack Trace: size = %d, used = %d items",
                        stack_size, ( stack + stack_size - SP ) );

    // Find current frame
    //
    JSVariant* fp = SP + 1;
    while ( fp->type != JS_FRAME_PTR )
    {
        fp ++;
        }

    JSVariant* fpEnd = SP;
    Compiled* pc = PC;

    for ( int frame = 0; fp && frame < num_frames; frame++ )
    {
        StackTrace( fp, fpEnd, pc );

        // Next frame
        //
        if ( fp[ 2 ].type == JS_INTEGER )
        {
            // skip pushed arguments; they belong to function
            fpEnd = fp + fp[ 2 ].vinteger + 2;
            }
        else
        {
            fpEnd = fp;
            }

        pc = fp[ -4 ].instr_ptr;
        fp = fp[ 0 ].frame_ptr;
        }
    }
