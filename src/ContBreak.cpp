#include "JSC.h"

void
ContBreakNest:: Create
(
    JSC_Compiler* ctx
    )
{
    jsc = ctx;
    pTop = NULL;
    pDeleted = NULL;

    push( NULL, NULL, NULL, false );
    }

void
ContBreakNest:: push
(
    String* arg_label,
    ASM_labelDef* arg_loopbreak,
    ASM_labelDef* arg_loopcont,
    bool arg_in_switch
    )
{
    ContBreakFrame* frame = NULL;
    if ( pDeleted )
    {
        frame = pDeleted;
        pDeleted = pDeleted->next;
        }
    else
    {
        frame = new(jsc) ContBreakFrame;
        }

    frame->label = arg_label;
    frame->loop_break = arg_loopbreak;
    frame->loop_continue = arg_loopcont;
    frame->in_switch = false;
    frame->num_with_nesting = 0;
    frame->num_try_nesting = 0;

    frame->next = pTop;
    pTop = frame;
    }

void
ContBreakNest:: pop
(
    void
    )
{
    assert( pTop != NULL );

    ContBreakFrame* frame = pTop;
    pTop = frame->next;
    frame->next = pDeleted;
    pDeleted = frame;
    }

int
ContBreakNest:: countTryReturnNesting( void )
{
    // Count the currently active 'try' nesting that should be removed
    // on 'return' statement

    int count = 0;

    for( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        count += frame->num_try_nesting;
        }

    return count;
    }

int
ContBreakNest:: countWithNesting( String* label )
{
    // Count currently active 'with' nesting that should be removed
    // on 'continue' or 'break' statement.

    int count = 0;

    for( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        count += frame->num_with_nesting;

        if ( label )
        {
            if ( frame->label && *frame->label == *label )
                break;
            }
        else
        {
            if ( frame->loop_continue )
                break;
            }
        }

    return count;
    }

int
ContBreakNest:: countTryNesting( String* label )
{
    // Count the currently active 'try' nesting that should be removed 
    // on 'continue' or 'break' statement.

    int count = 0;

    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        count += frame->num_try_nesting;

        if ( label )
        {
            if ( frame->label && *frame->label == *label )
                break;
            }
        else
        {
            if ( frame->loop_continue )
                break;
            }
        }

    return count;
    }

int
ContBreakNest:: countSwitchNesting( String* label )
{
    int count = 0;

    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        if ( frame->in_switch )
            count ++;

        if ( label )
        {
            if ( frame->label && *frame->label == *label )
                break;
            }
        else
        {
            if ( frame->loop_continue )
                break;
            }
        }

    return count;
    }

ASM_labelDef*
ContBreakNest:: getContinue( String* label )
{
    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        if ( label )
        {
            if ( frame->label && *frame->label == *label )
                return frame->loop_continue;
            }
        else
        {
            if ( frame->loop_continue )
                return frame->loop_continue;
            }
        }

    return NULL;
    }

ASM_labelDef*
ContBreakNest:: getBreak( String* label )
{
    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        if ( label )
        {
            if ( frame->label && *frame->label == *label )
                return frame->loop_break;
            }
        else
        {
            if ( frame->loop_break )
                return frame->loop_break;
            }
        }

    return NULL;
    }

bool
ContBreakNest:: isUniqueLabel( String* label )
{
    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        if ( frame->label && *frame->label == *label )
            return false;
        }

    return true;
    }

void
ContBreakNest:: dump( void )
{
    printf( "ContBreak dump:\n" );

    for ( ContBreakFrame* frame = pTop; frame; frame = frame->next )
    {
        printf(
            "label: %s, loop_break: %d, loop_continue: %d, in_switch: %d, withs: %d, trys: %d\n",
            frame->label ? (char*)frame->label : "<null>",
            frame->loop_break ? frame->loop_break->linenum : -1,
            frame->loop_continue ? frame->loop_continue->linenum : -1,
            frame->num_with_nesting,
            frame->num_try_nesting
            );
        }
    }
