#include "JSC.h"

static String globalSymbol( ".global" );
static String arraySymbol( "Array" );
static String objectSymbol( "Object" );
static String htmlOutSymbol( "htmlOut" );

void
JSC_Compiler:: asmGenerate( void )
{
    if ( options.verbose > 0 )
        trace( "generating assembler" );

    // Functions
    //
    for ( FunctionDeclaration* func = functions.first; func; func = (FunctionDeclaration*)func->next )
        func->Assemble( this );

    // Global statements
    //
    if ( global_stmts.first != NULL )
    {
        // Define the `.global' symbol
        //
        ASM_defSymbol( &globalSymbol, global_stmts.first->linenum );

        // Handle local variables
        //
        int num_locals = countLocals( global_stmts );
        if ( num_locals > 0 )
            ASM_locals( num_locals, global_stmts.first->linenum );

        // Generate assembler
        //
        for ( Statement* stmt = global_stmts.first; stmt; stmt = (Statement*)stmt->next )
        {
            stmt->Assemble( this );
            }

        // Fix things so that also the global statement returns something
        // (this is required when we use eval() in JavaScript).
        //
        if ( asm_code.tail_prev == NULL )
        {
            // This is probably illegal, but we don't panic
            //
            ASM_const_undefined( 0 );
            }
        else
        {
            // If the latest op is `pop', remove it. Otherwise, append
            // a `const_undefined'.
            //
            if ( asm_code.tail->opcode == JS_OPCODE_POP )
            {
                asm_code.tail = asm_code.tail_prev;
                asm_code.tail->next = NULL;
                asm_code.tail_prev = NULL;
                }
            else
            {
                ASM_const_undefined( asm_code.tail->linenum );
                }
            }
        }
    }

void FunctionDeclaration:: Assemble( JSC_Compiler* jsc )
{
    jsc->ns.pushFrame( linenum );

    // Define arguments
    //
    int arg_no = 2;
    for ( ArgDeclaration* arg = args.first; arg; arg = (ArgDeclaration*) arg->next )
    {
        jsc->ns.defineSymbol( arg->id, JSC_SCOPE_ARG, arg_no ++, linenum );
        }

    // Define the function name to be a global symbol
    //
    jsc->ASM_defSymbol( name, linenum );

    // Check that function gets the required amount of arguments
    //
    jsc->ASM_min_args( arg_no, lbrace_linenum );

    // Count how many local variables we need
    //
    int num_locals = countLocals( block->list );

    if ( num_locals > 0 )
    {
        jsc->ASM_locals( num_locals, lbrace_linenum );
        }

    // Assembler for our body
    //
    for ( Statement* stmt = block->list.first; stmt; stmt = (Statement*)stmt->next )
    {
        stmt->Assemble( jsc );
        }

    // Every function must return something.  We could check if all
    // control flows in this function ends to a return, but that would
    // bee too hard...  Just append a return const_undefined.  The optimizer
    // will remove it if it is not needed.
    //
    int ln = linenum;
    if ( block->list.last != NULL )
        ln = block->list.last->linenum;

    jsc->ASM_const_undefined( ln );
    jsc->ASM_return( ln );

    // Pop our namespace
    //
    jsc->ns.popFrame ();
    }

void BlockStatement:: Assemble( JSC_Compiler* jsc )
{
    if ( list.first )
        jsc->ns.pushFrame( list.first->linenum );

    for ( Statement* stmt = list.first; stmt; stmt = (Statement*)stmt->next )
    {
        stmt->Assemble( jsc );
        }

    if ( list.first )
        jsc->ns.popFrame ();
    }

void FunctionStatement:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_load_global( function_name, linenum );
    jsc->ASM_load_global( container_name, linenum );
    jsc->ASM_store_property( given_name, linenum );
    }

void VariableStatement:: Assemble( JSC_Compiler* jsc )
{
    // Define all local variables to our namespace
    //
    for ( VariableDeclaration* var = vars.first; var; var = (VariableDeclaration*)var->next )
    {
        if ( ! global_level )
        {
            jsc->ns.defineSymbol( var->id, JSC_SCOPE_LOCAL,
                                  jsc->ns.allocLocal(), linenum );
            }

        if ( var->expr )
        {
            var->expr->Assemble( jsc );

            if ( global_level )
            {
                jsc->ASM_store_global( var->id, linenum );
                }
            else
            {
                SymbolDefinition* r = jsc->ns.lookupSymbol( var->id );

                if ( r == NULL || r->scope != JSC_SCOPE_LOCAL )
                {
                    jsc->error( linenum, "internal compiler error in local variable declaration" );
                    }

                jsc->ASM_store_local( r->value, linenum );
                }
            }
        }
    }

void EmptyStatement:: Assemble( JSC_Compiler* jsc )
{
    // Empty means empty ...
    }

void ExpressionStatement:: Assemble( JSC_Compiler* jsc )
{
    if ( expr )
    {
        expr->Assemble( jsc );
        jsc->ASM_pop( linenum );
        }
    }

void HtmlOutStatement:: Assemble( JSC_Compiler* jsc )
{
    if ( expr1 || expr2 )
    {
        if ( expr2 )
            expr2->Assemble( jsc );

        if ( expr1 )
            expr1->Assemble( jsc );

        if ( expr1 && expr2 )
            jsc->ASM_const_i2( linenum );
        else
            jsc->ASM_const_i1( linenum );

        jsc->ASM_load_global( &htmlOutSymbol, linenum );
        jsc->ASM_jsr( linenum );

        if ( expr1 && expr2 )
            jsc->ASM_pop_n( 5, linenum );
        else
            jsc->ASM_pop_n( 4, linenum );
        }
    }

void IfStatement:: Assemble( JSC_Compiler* jsc )
{
    expr->Assemble( jsc );

    ASM_labelDef* L1 = jsc->ASM_defLabel ();
    ASM_labelDef* L2 = jsc->ASM_defLabel ();

    if ( expr->lang_type == JS_BOOLEAN )
        jsc->ASM_iffalse_b( L1, linenum );
    else
        jsc->ASM_iffalse( L1, linenum );

    // Code for the then branch
    //
    if_stmt->Assemble( jsc );
    jsc->ASM_jmp( L2, linenum ); // goto Done label

    // Code for the else branch
    //
    jsc->ASM_emitLabel( L1 );
    if ( else_stmt!= NULL )
        else_stmt->Assemble( jsc );

    // emit Done label.
    //
    jsc->ASM_emitLabel( L2 );
    }

void WhileStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* L1 = jsc->ASM_defLabel ();
    ASM_labelDef* L2 = jsc->ASM_defLabel ();

    // Loop label
    //
    jsc->ASM_emitLabel( L1 );

    // Condition
    //
    expr->Assemble( jsc );
    if ( expr->lang_type == JS_BOOLEAN )
        jsc->ASM_iffalse_b( L2, linenum );
    else
        jsc->ASM_iffalse( L2, linenum );

    // Body
    //
    jsc->cont_break.push( NULL, L2, L1 );
    stmt->Assemble( jsc );
    jsc->cont_break.pop ();

    // Goto loop
    //
    jsc->ASM_jmp( L1, linenum );

    // Break label
    //
    jsc->ASM_emitLabel( L2 );
    }

void ForStatement:: Assemble( JSC_Compiler* jsc )
{
    // Code for the init
    //
    if ( vars.first != NULL )
    {
        // We have our own variable scope
        //
        jsc->ns.pushFrame( vars.first->linenum );

        for ( VariableDeclaration* decl = vars.first; decl; decl = (VariableDeclaration*)decl->next )
        {
            jsc->ns.defineSymbol( decl->id, JSC_SCOPE_LOCAL,
                                  jsc->ns.allocLocal (), linenum );

            // Possible init
            //
            if ( decl->expr != NULL )
            {
                decl->expr->Assemble( jsc );

                SymbolDefinition* r = jsc->ns.lookupSymbol( decl->id );

                if ( r == NULL || r->scope != JSC_SCOPE_LOCAL )
                {
                    jsc->error( linenum, "internal compiler error in local variable declaration "
                                 "in for statement" );
                    }

                jsc->ASM_store_local( r->value, linenum );
                }
            }
        }
    else if ( expr1 != NULL )
    {
        expr1->Assemble( jsc );
        jsc->ASM_pop( linenum );
        }

    ASM_labelDef* L1 = jsc->ASM_defLabel ();
    ASM_labelDef* L2 = jsc->ASM_defLabel ();
    ASM_labelDef* L3 = jsc->ASM_defLabel ();

    // Loop label
    //
    jsc->ASM_emitLabel( L1 );

    // Condition
    //
    bool optimize_type = false;
    if ( expr2 != NULL )
    {
        expr2->Assemble( jsc );
        if ( expr2->lang_type == JS_BOOLEAN )
            optimize_type = true;
        }
    else
    {
        jsc->ASM_const_true( linenum );
        optimize_type = true;
        }

    if ( optimize_type )
        jsc->ASM_iffalse_b( L3, linenum );
    else
        jsc->ASM_iffalse( L3, linenum );

    // Body
    //
    jsc->cont_break.push( NULL, L3, L2 );
    stmt->Assemble( jsc );
    jsc->cont_break.pop ();

    // Continue label
    //
    jsc->ASM_emitLabel( L2 );

    // Increment
    //
    if ( expr3 != NULL )
    {
        expr3->Assemble( jsc );
        jsc->ASM_pop( linenum );
        }

    // Goto loop
    //
    jsc->ASM_jmp( L1, linenum );

    // Break label
    //
    jsc->ASM_emitLabel( L3 );

    if ( vars.first != NULL )
    {
        // Pop the local variable scope
        //
        jsc->ns.popFrame ();
        }
    }

void ForInStatement:: Assemble( JSC_Compiler* jsc )
{
    int localId = 0;

    if ( vars.first != NULL )
    {
        // We need our own variable scope here
        //
        jsc->ns.pushFrame( vars.first->linenum );
        localId = jsc->ns.allocLocal ();

        jsc->ns.defineSymbol( vars.first->id, JSC_SCOPE_LOCAL,
                              localId, linenum );

        // Possible init
        //
        if ( vars.first->expr )
        {
            vars.first->expr->Assemble( jsc );
            jsc->ASM_store_local( localId, linenum );
            }
        }

    // Init the world
    //
    expr2->Assemble( jsc );

    jsc->ASM_dup( linenum );
    jsc->ASM_const_i0( linenum );
    jsc->ASM_swap( linenum );
    jsc->ASM_const_i0( linenum );

    ASM_labelDef* L_loop = jsc->ASM_defLabel ();
    ASM_labelDef* L_cont = jsc->ASM_defLabel ();
    ASM_labelDef* L_iffalse_b = jsc->ASM_defLabel ();
    ASM_labelDef* L_break = jsc->ASM_defLabel ();

    // Loop label
    //
    jsc->ASM_emitLabel( L_loop );

    // Fetch nth
    //
    jsc->ASM_nth( linenum );
    jsc->ASM_iffalse_b( L_iffalse_b, linenum );

    // Store value to variable
    //
    if ( vars.first != NULL )
        jsc->ASM_store_local( localId, linenum );
    else
        jsc->ASM_expr_lvalue_store( expr1 );

    // Body
    //
    jsc->cont_break.push( NULL, L_break, L_cont );
    stmt->Assemble( jsc );
    jsc->cont_break.pop ();

    // Continue label
    //
    jsc->ASM_emitLabel( L_cont );

    // Increment
    //
    jsc->ASM_const_i1( linenum );
    jsc->ASM_add( linenum );
    jsc->ASM_dup( linenum );
    jsc->ASM_roll( -3, linenum );
    jsc->ASM_dup( linenum );
    jsc->ASM_roll( 4, linenum );
    jsc->ASM_swap( linenum );

    // Goto loop
    //
    jsc->ASM_jmp( L_loop, linenum );

    // Out label
    //
    jsc->ASM_emitLabel( L_iffalse_b );

    jsc->ASM_pop ( linenum );

    // Break label
    //
    jsc->ASM_emitLabel( L_break );
    jsc->ASM_pop_n( 2, linenum );

    if ( vars.first )
    {
        // Pop the variable scope
        //
        jsc->ns.popFrame ();
        }
    }

void DoWhileStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* L1 = jsc->ASM_defLabel ();
    ASM_labelDef* L2 = jsc->ASM_defLabel ();
    ASM_labelDef* L3 = jsc->ASM_defLabel ();

    // emit Loop label
    //
    jsc->ASM_emitLabel( L1 );

    // Body
    //
    jsc->cont_break.push( NULL, L3, L2 );
    stmt->Assemble( jsc );
    jsc->cont_break.pop ();

    // Condition & continue
    //
    jsc->ASM_emitLabel( L2 );
    expr->Assemble( jsc );
    if ( expr->lang_type == JS_BOOLEAN )
        jsc->ASM_iftrue_b( L1, linenum );
    else
        jsc->ASM_iftrue( L1, linenum );

    // Break label
    //
    jsc->ASM_emitLabel( L3 );
    }

void ContinueStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* l_cont = jsc->cont_break.getContinue( label );

    if ( l_cont == NULL )
    {
        if ( label )
        {
            jsc->cont_break.dump ();
            jsc->error( linenum, "label '%s' not found for continue statemnt", (const char*)*label );
            }
        else
        {
            jsc->error( linenum, "continue statement not within a loop" );
            }
        }

    int nesting = jsc->cont_break.countWithNesting( label );
    if ( nesting > 0 )
        jsc->ASM_with_pop( nesting, linenum );

    nesting = jsc->cont_break.countTryNesting( label );
    if ( nesting > 0 )
        jsc->ASM_try_pop( nesting, linenum );

    nesting = jsc->cont_break.countSwitchNesting( label );
    if ( nesting <= 0 )
        {}
    else if ( nesting == 1 )  // Pop the value of the switch expression
        jsc->ASM_pop( linenum );
    else if ( nesting > 1 )
        jsc->ASM_pop_n( nesting, linenum );

    jsc->ASM_jmp( l_cont, linenum );
    }

void BreakStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* l_break = jsc->cont_break.getBreak( label );

    if ( l_break == NULL )
    {
        if ( label )
        {
            jsc->cont_break.dump ();
            jsc->error( linenum, "label '%s' not found for break statement", label );
            }
        else
        {
            jsc->error( linenum, " break statement not within a loop or switch" );
            }
        }

    int nesting = jsc->cont_break.countWithNesting( label );
    if ( nesting > 0 )
        jsc->ASM_with_pop( nesting, linenum );

    nesting = jsc->cont_break.countTryNesting( label );
    if ( nesting > 0 )
        jsc->ASM_try_pop( nesting, linenum );

    //
    // For non-labeled breaks, the switch nesting is handled in the
    // stmt_switch(). The code after the label, returned by the
    // getBreak(), will handle the switch nesting in these cases.
    // For the labeled breaks, we must pop the switch nesting here.
    //
    if ( label )
    {
        nesting = jsc->cont_break.countSwitchNesting( label );
        if ( nesting <= 0 )
            {}
        else if ( nesting == 1 )
            jsc->ASM_pop( linenum );
        else
            jsc->ASM_pop_n( nesting, linenum );
        }

    jsc->ASM_jmp( l_break, linenum );
    }

void ReturnStatement:: Assemble( JSC_Compiler* jsc )
{
    int nesting = jsc->cont_break.countTryReturnNesting ();
    if ( nesting > 0 )
        jsc->ASM_try_pop( nesting, linenum );

    if ( expr != NULL )
        expr->Assemble( jsc );
    else
        jsc->ASM_const_undefined( linenum );

    jsc->ASM_return( linenum );
    }

void ThrowStatement:: Assemble( JSC_Compiler* jsc )
{
    expr->Assemble( jsc );
    jsc->ASM_throw( linenum );
    }

void WithStatement:: Assemble( JSC_Compiler* jsc )
{
    expr->Assemble( jsc );

    jsc->ASM_with_push( linenum );
    jsc->cont_break.incNumWithNesting ();

    stmt->Assemble( jsc );

    jsc->cont_break.decNumWithNesting ();
    jsc->ASM_with_pop( 1, linenum );
    }

void SwitchStatement:: Assemble( JSC_Compiler* jsc )
{
    // Evaluate the switch expression to the top of the stack
    //
    expr->Assemble( jsc );

    // The switch statement define a break label
    //
    ASM_labelDef* L_break = jsc->ASM_defLabel ();
    jsc->cont_break.push( NULL, L_break, NULL, true /*inswitch*/ );

    // For each clause (except the first), insert check and body labels.
    //
    ASM_labelDef* L_cur_check = NULL;
    ASM_labelDef* L_cur_body = NULL;

    // Generate code for each clause
    //
    for ( CaseClause* clause = clauses.first; clause; clause = (CaseClause*)clause->next )
    {
        // Next label for the last clause in list is break label.
        //
        ASM_labelDef* L_next_check = clause->next ? jsc->ASM_defLabel () : L_break;
        ASM_labelDef* L_next_body = clause->next ? jsc->ASM_defLabel () : L_break;

        // If clause->expr is NULL, this is the default clause that matches always.
        //
        if ( clause->expr == NULL )
        {
            if ( L_cur_check != NULL )
                jsc->ASM_emitLabel( L_cur_check ); // default clause label
            }
        else
        {
            // Must check if this clause matches the expression.
            //
            if ( L_cur_check != NULL )
                jsc->ASM_emitLabel( L_cur_check ); // check label of clause

            jsc->ASM_dup( clause->linenum );
            clause->expr->Assemble( jsc );
            jsc->ASM_cmp_eq( clause->linenum );
            jsc->ASM_iffalse_b( L_next_check, clause->linenum );
            }

        // Generate assembler for the body
        //
        if ( L_cur_body )
        {
            jsc->ASM_emitLabel( L_cur_body );
            }
            
        for ( Statement* stmt = clause->list.first; stmt; stmt = (Statement*)stmt->next )
        {
            stmt->Assemble( jsc );
            }

        // And finally, jump to the next body. (this is the fallthrough case)
        //
        jsc->ASM_jmp( L_next_body, clause->last_linenum );

        L_cur_check = L_next_check;
        L_cur_body = L_next_body;
        }

    jsc->cont_break.pop ();

    // The break label
    //
    jsc->ASM_emitLabel( L_break );

    // Pop the value of the switch expression
    //
    jsc->ASM_pop( last_linenum );
    }

void TryStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* L_finally = jsc->ASM_defLabel ();

    // Protect and execute the try-block.

    ASM_labelDef* L_try_error = jsc->ASM_defLabel ();
    jsc->ASM_try_push( L_try_error, linenum );
    jsc->cont_break.incNumTryNesting ();

    try_block->Assemble( jsc );

    jsc->cont_break.decNumTryNesting ();
    jsc->ASM_try_pop( 1, tryblock_last_linenum );

    // All ok so far. Push a 'false' to indicate no error and jump to
    // the finally block (or out if we have no finally block).
    //
    jsc->ASM_const_false( tryblock_last_linenum );
    jsc->ASM_jmp( L_finally, tryblock_last_linenum );

    // Handle try block failures. The thrown value is on the top of the
    // stack.
    //
    jsc->ASM_emitLabel( L_try_error );

    if ( catch_list.first != NULL )
    {
        // We keep one boolean variable below the thrown value.  Its default
        // value is false.  When one of our catch blocks are entered, it is
        // set to true to indicate that we shouldn't throw the error
        // anymore.
        //
        jsc->ASM_const_false( catch_list.first->linenum );
        jsc->ASM_swap( catch_list.first->linenum );

        // Protect and execute the catch list.
        //
        ASM_labelDef* L_catch_list_error = jsc->ASM_defLabel ();
        jsc->ASM_try_push( L_catch_list_error, catch_list.first->linenum );
        jsc->cont_break.incNumTryNesting ();

        // A label for the catch list end.
        //
        ASM_labelDef* L_catch_list_end = jsc->ASM_defLabel ();

        // A label for the catch start (NULL if the catch is not referenced)
        //
        ASM_labelDef* L_start = NULL;

        // Process the individual catches.
        //
        for ( CatchBlock* block = catch_list.first; block; block = (CatchBlock*)block->next )
        {
            // This is the starting point of this catch frame.
            //
            if ( L_start != NULL )
            {
                jsc->ASM_emitLabel( L_start );
                }

            // Create a new namespace frame and bind the catch's
            // identifier to the thrown exception.
            //
            jsc->ns.pushFrame( block->linenum );
            SymbolDefinition* sym = jsc->ns.defineSymbol( block->varname, JSC_SCOPE_LOCAL,
                                                          jsc->ns.allocLocal (), block->linenum );

            jsc->ASM_dup( block->linenum );
            jsc->ASM_store_local( sym->value, block->linenum );

            // Check the possible guard. We must protect its calculation.
            //
            if ( block->guard_expr != NULL )
            {
                // We have Body label
                //
                ASM_labelDef* L_body = jsc->ASM_defLabel ();

                ASM_labelDef* L_guard_error = jsc->ASM_defLabel ();
                jsc->ASM_try_push( L_guard_error, block->linenum );
                jsc->cont_break.incNumTryNesting ();

                // Calculate the guard
                //
                block->guard_expr->Assemble( jsc );

                jsc->cont_break.decNumTryNesting ();
                jsc->ASM_try_pop( 1, block->linenum );

                // Wow! We managed to do it. Now, let's check if we
                // accept this catch case.
                //
                ASM_labelDef* next_label = L_catch_list_end;
                L_start = NULL; // next catch is not referenced

                if ( block->next != NULL )
                {
                    L_start = jsc->ASM_defLabel ();
                    next_label = L_start;
                    }

                if ( block->guard_expr->lang_type == JS_BOOLEAN )
                    jsc->ASM_iffalse_b( next_label, block->linenum );
                else
                    jsc->ASM_iffalse( next_label, block->linenum );

                // Yes, we do accept it. Just jump to do the stuff.
                //
                jsc->ASM_jmp( L_body, block->linenum );

                // The evaluation of the guard failed.  Do the cleanup
                // and jump to the next case.
                //
                jsc->ASM_emitLabel( L_guard_error );

                // Pop the exception.
                //
                jsc->ASM_pop( block->linenum );

                // Check the next case.
                //
                jsc->ASM_jmp( next_label, block->linenum );

                // Referenced body label
                //
                jsc->ASM_emitLabel( L_body );
                }

            // We did enter the catch body.  Let's update our boolean
            // status variable to reflect this fact.
            //
            jsc->ASM_swap( block->linenum );
            jsc->ASM_pop( block->linenum );
            jsc->ASM_const_true( block->linenum );
            jsc->ASM_swap( block->linenum );

            // Code for the catch body.
            //
            for ( Statement* stmt = block->block->list.first; stmt; stmt = (Statement*)stmt->next )
            {
                stmt->Assemble( jsc );
                }

            // We'r done with the namespace frame.
            //
            jsc->ns.popFrame ();

            // The next catch tag, or the L_catch_list_end follows us,
            // so we don't need a jumps here.
            }

        // The catch list was evaluated without errors.
        //
        jsc->ASM_emitLabel( L_catch_list_end );
        jsc->cont_break.decNumTryNesting ();
        jsc->ASM_try_pop( 1, catchlist_last_linenum );

        // Did we enter any of our catch lists?

        ASM_labelDef* L_we_did_enter = jsc->ASM_defLabel ();
        jsc->ASM_swap( catchlist_last_linenum );
        jsc->ASM_iftrue_b( L_we_did_enter, catchlist_last_linenum );

        // No we didn't.

        // Push `true' to indicate an exception and jump to the finally
        // block.  The exception is now on the top of the stack.
        //
        jsc->ASM_const_true( catchlist_last_linenum );
        jsc->ASM_jmp( L_finally, catchlist_last_linenum );

        // Yes, we did enter one (or many) of our catch lists.
        //
        jsc->ASM_emitLabel( L_we_did_enter );

        // Pop the try-block's exception
        //
        jsc->ASM_pop( catchlist_last_linenum );

        // Push a `false' to indicate "no errors" and jump to the
        // finally block.
        //
        jsc->ASM_const_false( catchlist_last_linenum );
        jsc->ASM_jmp( L_finally, catchlist_last_linenum );

        // Handle catch list failures.  The thrown value is on the top of the
        // stack.
        //
        jsc->ASM_emitLabel( L_catch_list_error );

        // Pop the try-block's exception and our boolean 'did we enter a
        // catch block' variable. They are below our new exception.
        //
        jsc->ASM_apop( 2, catchlist_last_linenum );

        // Push `true' to indicate an exception. We will fallthrough to
        // the finally part, so no jump is needed here.
        //
        jsc->ASM_const_true( catchlist_last_linenum );
        }
    else
    {
        // No catch list
        //
        jsc->ASM_const_true( tryblock_last_linenum );
        }

    // The possible finally block.
    //
    jsc->ASM_emitLabel(L_finally );

    if ( fin_block != NULL )
    {
        // Execute it without protection.
        //
        fin_block->Assemble( jsc );
        }

    // We'r almost there. Let's see if we have to raise a new exception.
    //
    ASM_labelDef* L_out = jsc->ASM_defLabel ();
    jsc->ASM_iffalse_b( L_out, try_last_linenum );

    // Do raise it
    //
    jsc->ASM_throw( try_last_linenum );

    // The possible exception is handled. Please, continue.
    //
    jsc->ASM_emitLabel( L_out );
    }

void LabeledStatement:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* L_continue = jsc->ASM_defLabel ();
    ASM_labelDef* L_break = jsc->ASM_defLabel ();

    // It is an error if we already have a labeled statement with the
    // same id.
    //
    if ( ! jsc->cont_break.isUniqueLabel( label ) )
    {
        jsc->error( linenum, "labeled statement is enclosed by another labeled statement "
                    "with the same label" );
        }

    // Push the break and continue labels
    //
    jsc->cont_break.push( label, L_break, L_continue );

    // Dump the assembler
    //
    jsc->ASM_emitLabel( L_continue );
    stmt->Assemble( jsc );
    jsc->ASM_emitLabel( L_break );

    // And we'r done with our label scope
    //
    jsc->cont_break.pop ();
    }

void ThisExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_load_arg( 0, linenum );
    }

void IdentifierExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_expr_lvalue_load( this );
    }

void FloatExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_const( value, linenum );
    }

void IntegerExpression:: Assemble( JSC_Compiler* jsc )
{
    if ( value == 0 )
        jsc->ASM_const_i0( linenum );
    else if ( value == 1 )
        jsc->ASM_const_i1( linenum );
    else if ( value == 2 )
        jsc->ASM_const_i2( linenum );
    else if ( value == 3 )
        jsc->ASM_const_i3( linenum );
    else
        jsc->ASM_const_i( value, linenum );
    }

void StringExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_const( value, linenum );
    }

void ArrayInitializerExpression:: Assemble( JSC_Compiler* jsc )
{
    // Generate assembler for the individual items
    // *** list must be created in reverse order ***
    //
    for ( ArgExpression* item = items.first; item; item = (ArgExpression*)item->next )
    {
        if ( item->expr )
            item->expr->Assemble( jsc );
        else
            jsc->ASM_const_undefined( linenum );
        }

    // The number of items as a negative integer.  The Array object's
    // constructor knows that if the number of arguments is negative, it
    // is called from the array initializer.  Why?  Because the code:
    //
    //   new Array (5);
    //
    // creates an array of length of 5, but code:
    //
    //   [5]
    //
    // creates an array with one item: integer number five. These cases
    // must be separatable from the code and that's why the argument
    // counts for the array initializers are negative.
    //
    jsc->ASM_const_i( - long( items.length () ), linenum );

    // Call the constructor.
    //
    jsc->ASM_load_global( &arraySymbol, linenum );
    jsc->ASM_new( linenum );
    jsc->ASM_swap( linenum );
    jsc->ASM_apop( items.length () + 2, linenum );
    }

void ObjectInitializerExpression:: Assemble( JSC_Compiler* jsc )
{
    // Create a new object.
    //
    jsc->ASM_const_i0( linenum );
    jsc->ASM_load_global( &objectSymbol, linenum );
    jsc->ASM_new( linenum );
    jsc->ASM_swap( linenum );
    jsc->ASM_apop( 2, linenum );

    // Insert the items
    //
    for ( ObjectInitializerPair* pair = items.first; pair; pair = (ObjectInitializerPair*)pair->next )
    {

        jsc->ASM_dup( pair->linenum );
        pair->expr->Assemble( jsc );
        jsc->ASM_swap( pair->linenum );

        switch ( pair->id_type )
        {
            case JSC_tIDENTIFIER:
                jsc->ASM_store_property( pair->id, pair->linenum );
                break;
/* TODO:
            case JSC_tSTRING:
                jsc->ASM_defConst( pair->id, pair->linenum );
                jsc->ASM_store_array( pair->linenum );
                break;

            case JSC_tINTEGER:
                switch ( pair->id )
                {
                    case 0:
                        jsc->ASM_const_i0( pair->linenum );
                        break;

                    case 1:
                        jsc->ASM_const_i1( pair->linenum );
                        break;

                    case 2:
                        jsc->ASM_const_i2( pair->linenum );
                        break;

                    case 3:
                        jsc->ASM_const_i3( pair->linenum );
                        break;

                    default:
                        jsc->ASM_const_i( pair->id, pair->linenum );
                    break;
                    }

                jsc->ASM_store_array( pair->linenum );
                break;
*/
            }
        }
    }

void NullExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_const_null( linenum );
    }

void TrueExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_const_true( linenum );
    }

void FalseExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_const_false( linenum );
    }

void MultiplicativeExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    if ( token == '*' )
        jsc->ASM_mul( linenum );
    else if ( token == '/' )
        jsc->ASM_div( linenum );
    else
        jsc->ASM_mod( linenum );
    }

void AdditiveExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );
    if ( token == '+' )
        jsc->ASM_add( linenum );
    else
        jsc->ASM_sub( linenum );
    }

void ShiftExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    if ( token == JSC_tLSHIFT )
        jsc->ASM_shift_left( linenum );
    else if ( token == JSC_tRSHIFT )
        jsc->ASM_shift_right( linenum );
    else
        jsc->ASM_shift_rright( linenum );
    }

void RelationalExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    if ( token == '<')
        jsc->ASM_cmp_lt ( linenum );
    else if ( token == '>')
        jsc->ASM_cmp_gt ( linenum );
    else if ( token == JSC_tLE)
        jsc->ASM_cmp_le ( linenum );
    else
        jsc->ASM_cmp_ge ( linenum );
    }

void EqualityExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    switch ( token )
    {
        case JSC_tEQUAL:
            jsc->ASM_cmp_eq ( linenum );
            break;

        case JSC_tNEQUAL:
            jsc->ASM_cmp_ne ( linenum );
            break;

        case JSC_tSEQUAL:
            jsc->ASM_cmp_seq ( linenum );
            break;

        case JSC_tSNEQUAL:
            jsc->ASM_cmp_sne ( linenum );
            break;

        default:
            jsc->error( linenum, "expr_equality: internal compiler error" );
            break;
        }
    }

void BitwiseANDExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    jsc->ASM_and( linenum );
    }

void BitwiseORExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    jsc->ASM_or( linenum );
    }

void BitwiseXORExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    expr2->Assemble( jsc );

    jsc->ASM_xor( linenum );
    }

void LogicalANDExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );

    ASM_labelDef* L_done = jsc->ASM_defLabel ();
    jsc->ASM_dup( linenum );

    if ( expr1->lang_type == JS_BOOLEAN )
        jsc->ASM_iffalse_b( L_done, linenum );
    else
        jsc->ASM_iffalse( L_done, linenum );

    jsc->ASM_pop( linenum );

    expr2->Assemble( jsc );

    // Done label
    //
    jsc->ASM_emitLabel( L_done );
    }

void LogicalORExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );

    ASM_labelDef* L_done = jsc->ASM_defLabel ();
    jsc->ASM_dup( linenum );

    if ( expr1->lang_type == JS_BOOLEAN )
        jsc->ASM_iftrue_b( L_done, linenum );
    else
        jsc->ASM_iftrue( L_done, linenum );

    jsc->ASM_pop( linenum );

    expr2->Assemble( jsc );

    // Done label
    //
    jsc->ASM_emitLabel( L_done );
    }

void NewExpression:: Assemble( JSC_Compiler* jsc )
{
    long arg_count = 0;

    if ( args.first == NULL )
    {
        // A `new Foo' call. This is identical to `new Foo ()'.
        //
        jsc->ASM_const_i0( linenum );
        }
    else
    {
        // Code for the arguments
        // *** list must be created in reverse order ***
        //
        for ( ArgExpression* arg = args.first; arg; arg = (ArgExpression*)arg->next )
        {
            arg->expr->Assemble( jsc );
            arg_count ++;
            }

        if ( arg_count == 0 )
            jsc->ASM_const_i0( linenum );
        else if ( arg_count == 1 )
            jsc->ASM_const_i1( linenum );
        else if ( arg_count == 2 )
            jsc->ASM_const_i2( linenum );
        else if ( arg_count == 3 )
            jsc->ASM_const_i3( linenum );
        else
            jsc->ASM_const_i( arg_count, linenum );
        }

    // Object
    //
    expr->Assemble( jsc );

    // Call new
    //
    jsc->ASM_new( linenum );

    // Replace the constructor's return value with the object
    //
    jsc->ASM_swap( linenum );

    // Remove the arguments and return the new object
    //
    jsc->ASM_apop( 2 + arg_count, linenum );
    }

void ObjectPropertyExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_expr_lvalue_load( this );
    }

void ObjectArrayExpression:: Assemble( JSC_Compiler* jsc )
{
    jsc->ASM_expr_lvalue_load( this );
    }

void CallExpression:: Assemble( JSC_Compiler* jsc )
{
    // Code for the arguments
    // *** list must be created in reverse order ***
    //
    for ( ArgExpression* arg = args.first; arg; arg = (ArgExpression*)arg->next )
    {
        arg->expr->Assemble( jsc );
        }

    if ( args.count == 0 )
        jsc->ASM_const_i0( linenum );
    else if ( args.count == 1 )
        jsc->ASM_const_i1( linenum );
    else if ( args.count == 2 )
        jsc->ASM_const_i2( linenum );
    else if ( args.count == 3 )
        jsc->ASM_const_i3( linenum );
    else
        jsc->ASM_const_i( args.count, linenum );

    // Check the function type
    //
    if ( expr->type == JSC_IDENTIFIER_EXPR )
    {
        // The simple subroutine or global object method invocation.
        // Calculate expression the same way as it is calculated
        // if the expression was out of 'with' nesting.
        //
        int n = jsc->cont_break.numWithNesting (); // remember old with-nesting value
        jsc->cont_break.setNumWithNesting( 0 );    // and set it to 0 (global level)

        jsc->ASM_expr_lvalue_load( expr );

        jsc->cont_break.setNumWithNesting( n );    // restore old with-nesting value

        if ( n > 0 )
        {
            IdentifierExpression* e = (IdentifierExpression*)expr;
            jsc->ASM_jsr_w( e->value, linenum );
            }
        else
        {
            jsc->ASM_jsr( linenum );
            }

        jsc->ASM_apop( 2 + args.count, linenum );
        }
    else if ( expr->type == JSC_OBJECT_PROPERTY_EXPR )
    {
        // Method invocation
        //
        ObjectPropertyExpression* e = (ObjectPropertyExpression*)expr;
        e->expr->Assemble( jsc );
        jsc->ASM_call_method( e->id, linenum );
        jsc->ASM_apop( 2 + args.count, linenum );
        }
    else
    {
        // Something like a function pointer invocation
        //
        jsc->ASM_expr_lvalue_load( expr );
        jsc->ASM_jsr( linenum );
        jsc->ASM_apop( 2 + args.count, linenum );
        }
    }

void AssignmentExpression:: Assemble( JSC_Compiler* jsc )
{
    if ( token != '=' )
        jsc->ASM_expr_lvalue_load( expr1 );

    // Count the rvalue
    //
    expr2->Assemble( jsc );

    switch( token )
    {
        case '=':
            break;

        case JSC_tMULA:
            jsc->ASM_mul ( linenum );
            break;

        case JSC_tDIVA:
            jsc->ASM_div ( linenum );
            break;

        case JSC_tMODA:
            jsc->ASM_mod ( linenum );
            break;

        case JSC_tADDA:
            jsc->ASM_add ( linenum );
            break;

        case JSC_tSUBA:
            jsc->ASM_sub ( linenum );
            break;

        case JSC_tLSIA:
            jsc->ASM_shift_left ( linenum );
            break;

        case JSC_tRSIA:
            jsc->ASM_shift_right ( linenum );
            break;

        case JSC_tRRSA:
            jsc->ASM_shift_rright ( linenum );
            break;

        case JSC_tANDA:
            jsc->ASM_and ( linenum );
            break;

        case JSC_tXORA:
            jsc->ASM_xor ( linenum );
            break;

        case JSC_tORA:
            jsc->ASM_or ( linenum );
            break;

        default:
            jsc->error( linenum, "internal compiler error in assignment expression '%c'", token );
        }

    // Duplicate the value
    //
    jsc->ASM_dup ( linenum );

    // Store it to the lvalue
    //
    jsc->ASM_expr_lvalue_store( expr1 );
    }

void ConditionalExpression:: Assemble( JSC_Compiler* jsc )
{
    ASM_labelDef* L_cond = jsc->ASM_defLabel ();
    ASM_labelDef* L_done = jsc->ASM_defLabel ();

    // Code for the condition
    //
    expr1->Assemble( jsc );

    if ( expr1->lang_type == JS_BOOLEAN )
        jsc->ASM_iffalse_b( L_cond, linenum );
    else
        jsc->ASM_iffalse( L_cond, linenum );

    // Code for the true branch
    //
    expr2->Assemble( jsc );
    jsc->ASM_jmp( L_done, linenum );

    // Code for the false branch
    //
    jsc->ASM_emitLabel( L_cond );
    expr3->Assemble( jsc );

    // Done label
    //
    jsc->ASM_emitLabel( L_done );
    }

void UnaryExpression:: Assemble( JSC_Compiler* jsc )
{
    if ( token == '!' )
    {
        expr->Assemble( jsc );
        jsc->ASM_not ( linenum );
        }
    else if ( token == '+' )
    {
        expr->Assemble( jsc );
        // Nothing here
        //
        }
    else if ( token == '~' )
    {
        expr->Assemble( jsc );
        jsc->ASM_const_i( -1L, linenum );
        jsc->ASM_xor( linenum );
        }
    else if ( token == '-')
    {
        expr->Assemble( jsc );
        jsc->ASM_neg( linenum );
        }
    else if ( token == JSC_tDELETE)
    {
        if ( expr->type == JSC_OBJECT_PROPERTY_EXPR )
        {
            ObjectPropertyExpression* e = (ObjectPropertyExpression*)expr;
            e->expr->Assemble( jsc );
            jsc->ASM_delete_property( e->id, linenum );
            }
        else if ( expr->type == JSC_OBJECT_ARRAY_EXPR )
        {
            ObjectArrayExpression* e = (ObjectArrayExpression*)expr;
            e->expr1->Assemble( jsc );
            e->expr2->Assemble( jsc );
            jsc->ASM_delete_array( linenum );
            }
        else if ( expr->type == JSC_IDENTIFIER_EXPR )
        {
            IdentifierExpression* e = (IdentifierExpression*)expr;

            if ( jsc->cont_break.numWithNesting () == 0 )
            {
                jsc->error( linenum, "'delete property' called outside of a with-block" );
                }
            else
            {
                jsc->ASM_const_null( linenum );
                jsc->ASM_delete_property( e->value, linenum );
                }
            }
        else
        {
            jsc->error( linenum, "illegal target for the delete operand" );
            }
        }
    else if ( token == JSC_tVOID )
    {
        expr->Assemble( jsc );
        jsc->ASM_pop( linenum );
        jsc->ASM_const_undefined( linenum );
        }
    else if ( token == JSC_tTYPEOF )
    {
        expr->Assemble( jsc );
        jsc->ASM_typeof( linenum );
        }
    else if ( token == JSC_tPLUSPLUS
          ||  token == JSC_tMINUSMINUS )
    {
        // Fetch the old value
        //
        jsc->ASM_expr_lvalue_load( expr );

        // Do the operation
        //
        jsc->ASM_const_i1( linenum );

        if ( token == JSC_tPLUSPLUS )
            jsc->ASM_add( linenum );
        else
            jsc->ASM_sub( linenum );

        // Duplicate the value and store one copy pack to lvalue
        //
        jsc->ASM_dup( linenum );
        jsc->ASM_expr_lvalue_store( expr );
        }
    else
    {
        jsc->error( linenum, "internal error: unary expr's type is '%c'", token );
        }
    }

void PostfixExpression:: Assemble( JSC_Compiler* jsc )
{
    // Fetch the old value
    //
    jsc->ASM_expr_lvalue_load( expr );

    // Duplicate the value since it is the expression's value
    //
    jsc->ASM_dup( linenum );

    // Do the operation
    //
    jsc->ASM_const_i1( linenum );

    if ( token == JSC_tPLUSPLUS )
        jsc->ASM_add( linenum );
    else
        jsc->ASM_sub( linenum );

    // And finally, store it back
    //
    jsc->ASM_expr_lvalue_store( expr );
    }

void CommaExpression:: Assemble( JSC_Compiler* jsc )
{
    expr1->Assemble( jsc );
    jsc->ASM_pop ( linenum );
    expr2->Assemble( jsc );
    }

void
JSC_Compiler:: ASM_expr_lvalue_load( Expression* expr )
{
    if ( expr->type == JSC_IDENTIFIER_EXPR )
    {
        IdentifierExpression* e = (IdentifierExpression*)expr;

        // Must check global / local / argument
        //
        SymbolDefinition* sym = ns.lookupSymbol( e->value );

        if ( sym == NULL )
        {
	        if ( cont_break.numWithNesting () > 0 )
            {
	            ASM_load_global_w( e->value, e->linenum );
                }
	        else
            {
	            ASM_load_global( e->value, e->linenum );
                }
            }
        else if ( sym->scope == JSC_SCOPE_ARG )
        {
            if ( cont_break.numWithNesting () > 0 && options.warn_with_clobber )
            {
                warning( expr->linenum, "the with-lookup of symbol '%s' "
                         "is clobbered by the argument definition",
                         (const char*)*sym->name );
                }

            ASM_load_arg( sym->value, e->linenum );
            }
        else
        {
            if ( cont_break.numWithNesting () > 0 && options.warn_with_clobber )
            {
                warning( expr->linenum, "the with-lookup of symbol '%s' "
                         " is clobbered by the local variable definition",
                          sym->name );
                }

            ASM_load_local( sym->value, e->linenum );
            }
        }
    else if ( expr->type == JSC_OBJECT_PROPERTY_EXPR )
    {
        ObjectPropertyExpression* e = (ObjectPropertyExpression*) expr;
        e->expr->Assemble( this );
        ASM_load_property( e->id, e->linenum );
        }
    else if ( expr->type == JSC_OBJECT_ARRAY_EXPR )
    {
        ObjectArrayExpression* e = (ObjectArrayExpression*) expr;
        e->expr1->Assemble( this );
        e->expr2->Assemble( this );
        ASM_load_array( e->linenum );
        }
    else
    {
        error( expr->linenum, "syntax error: expressioon left value load failed" );
        }
    }

void
JSC_Compiler:: ASM_expr_lvalue_store( Expression* expr )
{
    if ( expr->type == JSC_IDENTIFIER_EXPR )
    {
        IdentifierExpression* e = (IdentifierExpression*)expr;

        SymbolDefinition* sym = ns.lookupSymbol( e->value );

        if ( sym == NULL )
            ASM_store_global( e->value, e->linenum );
        else if ( sym->scope == JSC_SCOPE_ARG )
            ASM_store_arg( sym->value, e->linenum );
        else
            ASM_store_local( sym->value, e->linenum );
        }
    else if ( expr->type == JSC_OBJECT_PROPERTY_EXPR )
    {
        ObjectPropertyExpression* e = (ObjectPropertyExpression*)expr;
        e->expr->Assemble( this );
        ASM_store_property( e->id, e->linenum );
        }
    else if ( expr->type == JSC_OBJECT_ARRAY_EXPR )
    {
        ObjectArrayExpression* e = (ObjectArrayExpression*)expr;
        e->expr1->Assemble( this );
        e->expr2->Assemble( this );
        ASM_store_array( e->linenum );
        }
    else
    {
        error( expr->linenum, "syntax error: expressioon left value store failed" );
        }
    }


