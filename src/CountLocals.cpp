#include "JSC.h"

int
countLocals( ListOf<Statement>& list )
{
    // How many variables we need at the top level.
    //
    int local_count = 0;

    // Maximum amount needed by the nested blocks.
    //
    int recursed_max_count = 0;

    for ( Statement* p = list.first; p; p = (Statement*)p->next )
    {
        local_count += p->CountLocals( /* recurse= */ false );

        int rc = p->CountLocals( /* recurse= */ true );
        if ( rc > recursed_max_count )
            recursed_max_count = rc;
        }

    return local_count + recursed_max_count;
    }

int
BlockStatement:: CountLocals( bool fRecursive )
{
    if ( ! fRecursive )
        return 0;

    return countLocals( list );
    }

int
SwitchStatement:: CountLocals( bool fRecursive )
{
    int locals = 0;

    if ( fRecursive )
    {
        // For the recursive cases, we need the maximum of our clause stmts
        //
        for( CaseClause* clause = clauses.first; clause; clause = (CaseClause*)clause->next )
        {
            for ( Statement* stmt = clause->list.first; stmt; stmt = (Statement*)stmt->next )
            {
                int rc = stmt->CountLocals( true );
                if ( rc > locals )
                    locals = rc;
                }
            }
        }
    else
    {
        // The case clauses are not blocks. Therefore, we need the amount,
        // needed by the clauses at the top-level.
        //
        for( CaseClause* clause = clauses.first; clause; clause = (CaseClause*)clause->next )
        {
            for ( Statement* stmt = clause->list.first; stmt; stmt = (Statement*)stmt->next )
            {
                locals += stmt->CountLocals( false );
                }
            }
        }

    return locals;
    }

int
TryStatement:: CountLocals( bool fRecursive )
{
    int count = 0;

    if ( fRecursive )
    {
        int rc = try_block->CountLocals( true );
        if ( rc > count)
            count = rc;

        for ( CatchBlock* block = catch_list.first; block; block = (CatchBlock*)block->next )
        {
            rc = block->block->CountLocals( true );
            if ( rc > count )
                count = rc;
            }

        if ( fin_block )
        {
            rc = fin_block->CountLocals( true );
            if ( rc > count )
                count = rc;
            }
        }
    else
    {
        // TODO is this correct ?

        count += try_block->CountLocals( false );

        for ( CatchBlock* block = catch_list.first; block; block = (CatchBlock*)block->next )
        {
            // One for the call variable
            //
            count ++;

            count += block->block->CountLocals( false );
            }

        if ( fin_block )
        {
            count += fin_block->CountLocals( false );
            }
        }

    return count;
    }

int
LabeledStatement:: CountLocals( bool fRecursive )
{
    return stmt->CountLocals( fRecursive );
    }

int
IfStatement:: CountLocals( bool fRecursive )
{
    int lcount = 0;

    if ( ! fRecursive )
    {
        if ( if_stmt->type == JSC_VARIABLE_STMT )
            lcount += ( (VariableStatement*) if_stmt )->vars.length ();

        if ( else_stmt != NULL && else_stmt->type == JSC_VARIABLE_STMT )
            lcount += ( (VariableStatement*) else_stmt )->vars.length ();
        }
    else
    {
        lcount = if_stmt->CountLocals( true );

        if ( else_stmt != NULL )
        {
            int rc = else_stmt->CountLocals( true );
            if ( rc > lcount )
                lcount = rc;
            }
        }

    return lcount;
    }

int
DoWhileStatement:: CountLocals( bool fRecursive )
{
    if ( ! fRecursive )
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
            return ( (VariableStatement*) stmt )->vars.length ();

        return 0;
        }

    return stmt->CountLocals( true );
    }

int
WhileStatement:: CountLocals( bool fRecursive )
{
    if ( ! fRecursive )
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
            return ( (VariableStatement*) stmt )->vars.length ();

        return 0;
        }

    return stmt->CountLocals( true );
    }

int
ForStatement:: CountLocals( bool fRecursive )
{
    int count = 0;

    if ( fRecursive )
    {
        if ( vars.first != NULL)
            count += vars.length ();

        count += stmt->CountLocals( true );
        }
    else
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
            count += ( (VariableStatement*) stmt )->vars.length ();
        }

    return count;
    }

int
ForInStatement:: CountLocals( bool fRecursive )
{
    int count = 0;

    if ( fRecursive )
    {
        if ( vars.first != NULL)
            count += vars.length ();

        count += stmt->CountLocals( true );
        }
    else
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
            count += ( (VariableStatement*) stmt )->vars.length ();
        }

    return count;
    }

int
WithStatement:: CountLocals( bool fRecursive )
{
    if ( ! fRecursive )
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
            return ( (VariableStatement*) stmt )->vars.length ();

        return 0;
        }

    return stmt->CountLocals( true );
    }

int
VariableStatement:: CountLocals( bool fRecursive )
{
    if ( ! fRecursive )
    {
        if ( global_level )
        {
            // We define these as global variables
            //
            return 0;
            }

        return vars.length ();
        }

    return 0;
    }
