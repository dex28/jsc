#include "JSC.h"

static inline ASM_statement*
lookupNextOp( ASM_statement* stmt )
{
    while ( stmt != NULL
        &&( stmt->opcode == JS_OPCODE_LABEL 
            || stmt->opcode == JS_OPCODE_SYMBOL )
        )
    {
        stmt = stmt->next;
        }

    return stmt;
    }

static inline int
stackDelta( ASM_statement* stmt )
{
    if ( stmt->opcode == JS_OPCODE_LOCALS )
    {
        return value_int32( stmt );
        }
    else if ( stmt->opcode == JS_OPCODE_POP_N )
    {
        return - value_int32( stmt );
        }
    else if ( stmt->opcode == JS_OPCODE_APOP )
    {
        return - value_int32( stmt );
        }

    return OpcodeDesc[ stmt->opcode ].stack_delta;
    }

void
JSC_Compiler:: asmOptimize( void )
{
    // Simple peephole optimization
    //
    if ( options.optimize_peephole )
    {
        if ( options.verbose )
            trace( "optimize: peephole" );

        for ( ASM_statement* item = asm_code.head; item; item = item->next )
        {
            // Optimization for DUP ... POP cases where POP removes the
            // item duplicated by DUP.
            //
            if ( item->next != NULL && item->next->opcode == JS_OPCODE_DUP )
            {
                int balance = 2;
                bool found = false;

                for ( ASM_statement* i1 = item->next->next;
                      i1 != NULL && i1->next != NULL;
                      i1 = i1->next
                      )
                {
                    ASM_statement* i2 = i1->next;

                    // The lookup ends on branches, and on dup, throw,
                    // and try_pop operands. We optimize on a basic
                    // block and we match the closest dup-pop pairs.
                    //
                    if ( i1->arg_type == JS_OPARG_LABEL // is local jump
                        || i1->opcode == JS_OPCODE_JSR
                        || i1->opcode == JS_OPCODE_JSR_W
                        || i1->opcode == JS_OPCODE_NEW
                        || i1->opcode == JS_OPCODE_CALL_METHOD
                        || i1->opcode == JS_OPCODE_RETURN
                        || i1->opcode == JS_OPCODE_DUP
                        || i1->opcode == JS_OPCODE_TRY_POP
                        || i1->opcode == JS_OPCODE_THROW
                        || i1->opcode == JS_OPCODE_SYMBOL
                        || i1->opcode == JS_OPCODE_LABEL
                        )
                    {
                        break;
                        }

                    balance += stackDelta( i1 );

                    if ( balance <= 0 ) // Going to negative. Stop here.
                        break;

                    if ( i2->opcode == JS_OPCODE_POP && balance == 1 )
                    {
                        // Found a matching pop
                        //
                        found = true;
                        i1->next = i2->next;
                        break;
                        }
                    }

                if ( found )
                {
                    // The dup can be removed
                    //
                    item->next = item->next->next;
                    }
                }

            // Two instruction optimization (starting from item->next)
            //
            if ( item->next != NULL && item->next->next != NULL )
            {
                ASM_statement* i1 = item->next;
                ASM_statement* i2 = i1->next;

                if ( i1->opcode == JS_OPCODE_APOP
                    && i2->opcode == JS_OPCODE_POP
                    )
                {
                    // i1: apop n
                    // i2: pop               => pop_n n + 1
                    //
                    ASM_statement* i = new(this) ASM_int32Arg( JS_OPCODE_POP_N, value_int32( i1 ) + 1, i1->linenum );
                    item->next = i;
                    i->next = i2->next;
                    }
                }

            if ( item->next != NULL && item->next->next != NULL )
            {
                ASM_statement* i1 = item->next;
                ASM_statement* i2 = i1->next;

                if ( i1->opcode == JS_OPCODE_CONST_TRUE
                  && ( i2->opcode == JS_OPCODE_IFFALSE || i2->opcode == JS_OPCODE_IFFALSE_B )
                    )
                {
                    // i1: const_true
                    // i2: iffalse{,_b} .LX  => ---
                    //
                    item->next = i2->next;
                    }
                }

            if ( item->next != NULL && item->next->next != NULL )
            {
                ASM_statement* i1 = item->next;
                ASM_statement* i2 = i1->next;

                if ( i1->opcode == JS_OPCODE_CONST_FALSE
                  && (i2->opcode == JS_OPCODE_IFTRUE
                      || i2->opcode == JS_OPCODE_IFTRUE_B )
                    )
                {
                    // i1: const_false
                    // i2: iftrue{,_b} .LX  => ---
                    //
                    item->next = i2->next;
                    }
                }

            if ( item->next != NULL && item->next->next != NULL )
            {
                ASM_statement* i1 = item->next;
                ASM_statement* i2 = i1->next;

                if ( i1->opcode == JS_OPCODE_CONST_FALSE
                    && ( i2->opcode == JS_OPCODE_IFFALSE || i2->opcode == JS_OPCODE_IFFALSE_B )
                    )
                {
                    // i1: const_false
                    // i2: iffalse{,_b} .LX  => jmp .LX
                    //
                    ASM_statement* i = new(this) ASM_labelArg( JS_OPCODE_JMP, value_label( i2 ), i1->linenum );
                    item->next = i;
                    i->next = i2->next;
                    }
                }

            if ( item->next != NULL && item->next->next != NULL )
            {
                ASM_statement* i1 = item->next;
                ASM_statement* i2 = i1->next;

                if ( i1->opcode == JS_OPCODE_CONST_TRUE 
                    && ( i2->opcode == JS_OPCODE_IFTRUE || i2->opcode == JS_OPCODE_IFTRUE_B )
                    )
                {
                    // i1: const_true
                    // i2: iftrue{,_b} .LX  => jmp .LX
                    //
                    ASM_statement* i = new(this) ASM_labelArg( JS_OPCODE_JMP, value_label( i2 ), i1->linenum );
                    item->next = i;
                    i->next = i2->next;
                    }
                }
                  
            }
        }

    // Jumps to jumps
    //
    if ( options.optimize_jumps )
    {
        if ( options.verbose > 0 )
            trace( "optimize: jumps to jumps" );

        for ( ASM_statement* item = asm_code.head; item; item = item->next )
        {
            if ( item->arg_type == JS_OPARG_LABEL ) // is local jump
            {
                ASM_statement* i2 = lookupNextOp( value_label( item ) );

                if ( i2 != NULL && i2->opcode == JS_OPCODE_JMP )
                {
                    // Ok, we can jump there directly
                    //
                    ( (ASM_labelArg*)item )->label = value_label( i2 );
                    }
                }
            }
        }

    if ( options.optimize_heavy )
        asmOptimizeHeavy ();

    // Optimizations for the size of the generated byte-code. It isn't
    // probably worth of doing these optimization for interactive
    // scripts since these won't affect the speed of the execution.
    // However, these optimizations make the byte-code files smaller so
    // these are nice for batch-compiled files.
    //
    if ( options.optimize_bc_size )
    {
        if ( options.verbose > 0 )
            trace( "optimize: removing un-referenced labels with dead code elimination" );

        bool delta = true;

        while ( delta )
        {
            delta = false;

            // Remove un-referenced labels
            //

            // First, make all labels unreferenced
            //
            for ( ASM_statement* item = asm_code.head; item != NULL; item = item->next )
            {
                if ( item->opcode == JS_OPCODE_LABEL )
                    ( (ASM_labelDef*)item )->referenced = false;
                }

            // Second, mark all referenced labels
            //
            for ( item = asm_code.head; item != NULL; item = item->next )
            {
                if ( item->arg_type == JS_OPARG_LABEL )
                    value_label( item )->referenced = true;
                }

            // Third, remove all un-referenced labels
            //
            for ( item = asm_code.head; item != NULL; item = item->next )
            {
                while ( item->next != NULL && item->next->opcode == JS_OPCODE_LABEL
                        && ! ( (ASM_labelDef*)item->next )->referenced
                        && item->next->next != NULL )
                {
                    item->next = item->next->next;
                    delta = true;
                    }
                }

            // Dead code elimination
            //
            for ( item = asm_code.head; item != NULL; item = item->next )
            {
                if ( item->opcode == JS_OPCODE_RETURN || item->opcode == JS_OPCODE_JMP )
                {
                    while ( item->next != NULL && item->next->opcode != JS_OPCODE_SYMBOL
                         && item->next->opcode != JS_OPCODE_LABEL)
                    {
                        item->next = item->next->next;
                        delta = true;
                        }
                    }
                }

            // Simple peephole optimization
            //
            for ( item = asm_code.head; item != NULL; item = item->next )
            {
                // Two instruction optimization (starting from item->next)
                //
                if (item->next != NULL && item->next->next != NULL)
                {
                    ASM_statement* i1 = item->next;
                    ASM_statement* i2 = i1->next;

                    if ( i1->opcode == JS_OPCODE_JMP
                        && i2->opcode == JS_OPCODE_LABEL
                        && value_label( i1 ) == i2
                        )
                    {
                        // i1:    jmp .LX
                        // i2:    .LX             => .LX
                        //
                        item->next = i2;
                        delta = true;
                        }
                    }
                }
            }
        }
    }

void
JSC_Compiler:: asmOptimizeHeavy( void )
{
#ifdef ALLOW_HEAVY_OPTIMIZATION
    if ( options.verbose )
        trace( "optimize: liveness analyzing" );

    // First, set the prev pointers and zero usage flags
    //
    for ( ASM_statement* item = asm_code.head; item != NULL; item = item->next )
    {
        item->live_args = 0;
        item->live_locals = 0;
        item->live_used = false;
        }

    // For each function
    //
    ASM_statement* fhead = NULL;
    for ( ASM_statement* ftail = asm_code.tail; ftail != NULL; ftail = fhead->prev )
    {
        bool change = true;

        // While there is change in the liveness
        //
        while ( change )
        {
            change = false;

            for ( fhead = ftail; fhead->opcode != JS_OPCODE_SYMBOL; fhead = fhead->prev )
            {
                unsigned long floc;
                unsigned long farg;

                if ( fhead->next != NULL )
                {
                    floc = fhead->next->live_locals;
                    farg = fhead->next->live_args;
                    }
                else
                    floc = farg = 0;

                if ( fhead->opcode == JS_OPCODE_LOAD_LOCAL
                    && value_int32( fhead ) < 32 )
                    floc |= ( 1 << value_int32( fhead ) );

                if ( fhead->opcode == JS_OPCODE_STORE_LOCAL
                    && value_int32( fhead ) < 32 )
                    floc &= ~( 1 << value_int32( fhead ) );

                if ( fhead->opcode == JS_OPCODE_LOAD_ARG
                    && value_int32( fhead ) < 32 )
                    farg |= ( 1 << value_int32( fhead ) );

                if ( fhead->opcode == JS_OPCODE_STORE_ARG
                    && value_int32( fhead ) < 32 )
                    farg &= ~( 1 << value_int32( fhead ) );

                if ( fhead->arg_type == JS_OPCODE_LABEL ) // is local jump
                {
                    floc |= value_label( fhead )->live_locals;
                    value_label( fhead )->live_used = true;
                    }

                if ( fhead->live_used && ( fhead->live_locals != floc
                                      || fhead->live_args != farg ) )
                {
                    change = true;
                    }

                fhead->live_used = false;
                fhead->live_locals = floc;
                fhead->live_args = farg;
                }
            }
        }

    // When we have the liveness analyzing performed,
    // we can do some fancy optimizations.
    //
    for ( item = asm_code.head; item != NULL; item = item->next )
    {
        // Three instruction optimization
        //
        if ( item->next != NULL && item->next->next != NULL
             && item->next->next->next != NULL)
        {
            ASM_statement* i1 = item->next;
            ASM_statement* i2 = i1->next;
            ASM_statement* i3 = i2->next;

            if ( i1->opcode == JS_OPCODE_STORE_LOCAL
                && i2->opcode == JS_OPCODE_LOAD_LOCAL
                && value_int32( i1 ) == value_int32( i2 )
                && ( i3->live_locals & ( 1 << value_int32( i1 ) ) ) == 0
                )
            {
                // i1:    store_local     n
                // i2:    load_local      n
                // i3:    nnn (n not live)        => nnn
                //
                item->next = i3;
                }
            }
        }
#endif
    }
