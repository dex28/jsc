#include "JSC.h"

void
JSC_Compiler:: consumeSemicolon( void )
{
    if ( next_token.id == ';' )
    {
        // Everything ok. It was there, so eat it.
        //
        getToken ();
        return;
        }

    // No semicolon. Let's see if we can insert it there.
    //
    if ( next_token.id == '}' 
        || next_token.id == JSC_tEOF
        || ( options.js_pages && next_token.id == JSC_tENDJSP )
        || cur_token.linenum < next_token.linenum
        )
    {
        // Ok, do the automatic semicolon insertion.
        //
        if ( options.warn_missing_semicolon )
        {
            warning( cur_token.linenum, "missing semicolon" );
            }

        num_missing_semicolons++;
        return;
        }

    // Sorry, could not terminate statement
    //
    error( cur_token.linenum, "S0001: semicolon, closing brace, EOF or newline expected." );
    }

void
JSC_Compiler:: parseProgram( void )
{
    if ( options.verbose > 0 )
    {
        trace( "parsing %s", stream.name );
        }

    while( next_token.id != JSC_tEOF )
    {
        parseSourceElement ();
        }

    //if ( verbose )
    {
        trace( "input stream had %ld lines, %ld tokens, %ld missing semicolons",
            stream.linenum - 1, num_tokens, num_missing_semicolons );
        }
    }

void
JSC_Compiler:: parseSourceElement ()
{
    FunctionDeclaration* foo = parseFunctionDeclaration ();
    if ( foo )
    {
        return;
        }

    Statement* stmt = parseStatement ();
    if ( stmt )
    {
        if ( stmt->type == JSC_VARIABLE_STMT )
        {
            // this is global declaration
            //
            ( (VariableStatement*)stmt )->global_level = true;
            }

        global_stmts.append( stmt );
        return;
        }

    error( cur_token.linenum, "S0002: statement or function declaration expected" );
    }

FunctionDeclaration*
JSC_Compiler:: parseFunctionDeclaration( void )
{
    if ( next_token.id != JSC_tFUNCTION )
        return NULL;

    getToken (); // eat JSC_tFUNCTION

    if ( next_token.id != JSC_tIDENTIFIER )
        error( cur_token.linenum, "S0003: opening parenthesis expected" );

    getToken (); // eat JSC_tIDENTIFIER

    int linenum = cur_token.linenum;
    String* name = cur_token.vstring;
    String* given_name = name;

    if ( nested_foo_count > 0 )
    {
        // This is a nested function declaration
        //
        char buf[ 32 ];
        sprintf( buf, ".F:%ld", ++anonymous_foo_count );

        name = strDup( this, String( buf ) );
        }

    nested_foo_names[ nested_foo_count++ ] = name;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0004: opening parenthesis expected" );

    // Formal parameter list opt
    //

    ListOf<ArgDeclaration> args;

    while ( next_token.id != ')' )
    {
        if ( getToken () != JSC_tIDENTIFIER )
            error( cur_token.linenum, "S0005: function argument expected" );

        args.append( new(this) ArgDeclaration( cur_token.linenum, cur_token.vstring ) );

        if ( next_token.id == ',' )
        {
            getToken ();

            if ( next_token.id != JSC_tIDENTIFIER )
                error( cur_token.linenum, "S0006: function argument expected" );
            }
        else if ( next_token.id != ')' )
        {
            error( cur_token.linenum, "S0007: closing parenthesis expected" );
            }
        }

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0008: closing parenthesis expected" );

    int lbrace_linenum = next_token.linenum;

    BlockStatement* block = parseBlock ();

    if ( block == NULL )
        error( cur_token.linenum, "S0009: function block expected" );

    FunctionDeclaration* foo = new(this) FunctionDeclaration(
        linenum, name, given_name, args, lbrace_linenum, block
        );

    functions.append( foo );

    nested_foo_count--;

    return foo;
    }

Statement*
JSC_Compiler:: parseStatement( void )
{
    Statement* stmt = NULL;
    
    if ( options.js_pages )
    {
        if ( next_token.id == JSC_tENDJSP )
        {
            getToken ();
            }

        if ( next_token.id == JSC_tHTMLSTRING )
        {
            // parse HTML string
            //
            getToken ();
            Expression* expr1 = new(this) StringExpression( cur_token.linenum, cur_token.vstring );
            Expression* expr2 = NULL;

            if ( next_token.id == JSC_tBEGJSPEXPR )
            {
                getToken ();

                expr2 = parseExpression ();
                if ( expr2 )
                {
                    consumeSemicolon ();
                    }
                }
            else if ( next_token.id == JSC_tBEGJSPCODE )
            {
                getToken ();
                }
            else if ( next_token.id != JSC_tEOF )
            {
                error( cur_token.linenum, "S0340: epxected beginning of JSP code or EOF" );
                }

            return new(this) HtmlOutStatement( expr1->linenum, expr1, expr2 );
            }
        }

    stmt = parseBlock ();
    if ( stmt )
        return stmt;

    stmt = parseFunctionStatement ();
    if ( stmt )
        return stmt;

    stmt = parseVariableStatement ();
    if ( stmt )
        return stmt;

    stmt = parseIfStatement ();
    if ( stmt )
        return stmt;

    stmt = parseDoWhileStatement ();
    if ( stmt )
        return stmt;

    stmt = parseWhileStatement ();
    if ( stmt )
        return stmt;

    stmt = parseForStatement ();
    if ( stmt )
        return stmt;

    stmt = parseEmptyStatement ();
    if ( stmt )
        return stmt;

    stmt = parseContinueStatement ();
    if ( stmt )
        return stmt;

    stmt = parseBreakStatement ();
    if ( stmt )
        return stmt;

    stmt = parseReturnStatement ();
    if ( stmt ) 
        return stmt;

    stmt = parseThrowStatement ();
    if ( stmt )
        return stmt;

    stmt = parseSwitchStatement ();
    if ( stmt )
        return stmt;

    stmt = parseWithStatement ();
    if ( stmt )
        return stmt;

    stmt = parseTryStatement ();
    if ( stmt )
        return stmt;

    // parseLabeledStatement
    //
    if ( next_token.id == JSC_tIDENTIFIER
        && next_token2.id == ':'
        && next_token.linenum == next_token2.linenum
        )
    {
        getToken (); // consume label
        int linenum = cur_token.linenum;
        String* label = cur_token.vstring;

        getToken (); // consume trailing ':'

        stmt = parseStatement ();
        if ( stmt == NULL )
            error( cur_token.linenum, "S0010: statement expected" );

        return new(this) LabeledStatement( linenum, label, stmt );
        }

    // parseExpressionStatement
    //
    Expression* expr = parseExpression ();
    if ( expr )
    {
        consumeSemicolon ();
        return new(this) ExpressionStatement( expr->linenum, expr );
        }

    // Valid statement is not encountered. Maybe statement is optional?
    // So, return nothing - don't report error.
    //
    return NULL;
    }

BlockStatement*
JSC_Compiler:: parseBlock( void )
{
    if ( next_token.id != '{' )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    ListOf<Statement> list;

    // Do we have a statement list?
    //
    if ( next_token.id != '}' )
    {
        for( ;; )
        {
            Statement* item = parseStatement ();

            if ( item == NULL )
            {
                break; // Can't parse more statements. We are done.
                }

            list.append( item );
            }
        }

    if ( getToken () != '}' )
        error( cur_token.linenum, "S0017: closing brace or statement expected" );

    return new(this) BlockStatement( linenum, list );
    }

Statement*
JSC_Compiler:: parseFunctionStatement( void )
{
    FunctionDeclaration* foo = parseFunctionDeclaration ();
    if ( ! foo )
        return NULL;

    // The function declaration as statement might be incomplete.
    //
    if ( nested_foo_count == 0 )
    {
	    // Function declaration at top-level statements
        //
	    return new(this) EmptyStatement( cur_token.linenum );
        }

    // Function declaration inside another function
    //
    String* container_name = nested_foo_names[ nested_foo_count - 1 ];

    return new(this) FunctionStatement(
        cur_token.linenum, container_name, foo->name, foo->given_name
        );
    }

Statement*
JSC_Compiler:: parseVariableStatement( void )
{
    if ( next_token.id != JSC_tVAR )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    ListOf<VariableDeclaration> vars;

    for ( ;; )
    {
        if ( next_token.id != JSC_tIDENTIFIER )
            error( cur_token.linenum, "S0018: variable identifier expected" );

        getToken ();
        int linenum = cur_token.linenum;
        String* id = cur_token.vstring;

        Expression* expr = NULL;

        if ( next_token.id == '=' )
        {
            getToken ();

            expr = parseAssignmentExpression ();

            if ( expr == NULL )
                error( cur_token.linenum, "S0019: assignment expression expected" );
            }
        else
        {
            // FIXME: default initalization of all variables to null ?
            //
            // expr = new(this) NullExpression( cur_token.linenum );
            }

        vars.append( new(this) VariableDeclaration( linenum, id, expr ) );

        if ( next_token.id != ',' )
        {
            consumeSemicolon ();
            break; // No more variable declarations
            }

        getToken ();
        }

    return new(this) VariableStatement( linenum, vars );
    }

Statement*
JSC_Compiler:: parseEmptyStatement( void )
{
    if ( next_token.id != ';' )
        return NULL;

    getToken ();

    return new(this) EmptyStatement( cur_token.linenum );
    }

Statement*
JSC_Compiler:: parseContinueStatement( void )
{
    if ( next_token.id != JSC_tCONTINUE )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    // Check the possible label
    //
    String* label = NULL;

    if ( next_token.id == JSC_tIDENTIFIER
        && cur_token.linenum == next_token.linenum )
    {
        getToken ();
        label = cur_token.vstring;
        }

    Statement* stmt = new(this) ContinueStatement( linenum, label );

    consumeSemicolon ();

    return stmt;
    }

Statement*
JSC_Compiler:: parseBreakStatement( void )
{
    if ( next_token.id != JSC_tBREAK )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    // Check the possible label
    //
    String* label = NULL;

    if ( next_token.id == JSC_tIDENTIFIER
        && cur_token.linenum == next_token.linenum )
    {
        getToken ();
        label = cur_token.vstring;
        }

    Statement* stmt = new(this) BreakStatement( linenum, label );

    consumeSemicolon ();

    return stmt;
    }

Statement*
JSC_Compiler:: parseReturnStatement( void )
{
    if ( next_token.id != JSC_tRETURN )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    Expression* expr = NULL;

    if ( next_token.id == ';' )
    {
        getToken ();
        }
    else if ( next_token.linenum > linenum )
    {
        // A line terminator between tRETURN and the next
        // token that is not a semicolon.
        //
        if ( options.warn_missing_semicolon )
            warning( cur_token.linenum, "missing semicolon" );

        num_missing_semicolons++;
        }
    else
    {
        expr = parseExpression ();
        if ( expr == NULL )
            error( cur_token.linenum, "S0011: expected expression" );

        consumeSemicolon ();
        }

    return new(this) ReturnStatement( linenum, expr );
    }

Statement*
JSC_Compiler:: parseThrowStatement( void )
{
    if ( next_token.id != JSC_tTHROW )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    // Get the next token's linenum. We need it for strict_ecma warning.
    //
    int peek_linenum = next_token.linenum;

    // The expression to throw
    //
    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0016: expression expcted after throw" );

    if ( options.warn_strict_ecma && peek_linenum > linenum )
        warning( cur_token.linenum, "ECMAScript don't allow line terminators between 'throw' and expression");

    consumeSemicolon ();

    return new(this) ThrowStatement( linenum, expr );
    }

Statement*
JSC_Compiler::parseSwitchStatement( void )
{
    if ( next_token.id != JSC_tSWITCH )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0020: opening parenthesis expected after switch" );

    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0021: expression expected aftesr switch(" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0022: closing parenthesis expected after switch(expr" );

    if ( getToken () != '{' )
        error( cur_token.linenum, "S0023: opening brace expected after switch(expr)" );

    // Parse case clauses
    //
    ListOf<CaseClause> clauses;

    int last_linenum = -1;

    for ( ;; )
    {
        getToken ();

        last_linenum = cur_token.linenum;

        if ( cur_token.id == '}' )
            break;

        if ( cur_token.id == JSC_tCASE || cur_token.id == JSC_tDEFAULT )
        {
            Expression* expr = NULL;
            ListOf<Statement> branch_stmts;

            if ( cur_token.id == JSC_tCASE )
            {
                expr = parseExpression ();
                if ( expr == NULL )
                    error( cur_token.linenum, "S0024: expected expression after case" );
                }

            if ( getToken () != ':' )
                error( cur_token.linenum, "S0124: expected colons after switch label" );

            int linenum = cur_token.linenum;

            // Read the statement list
            //
            for ( ;; )
            {
                if ( next_token.id == '}'
                    || next_token.id == JSC_tCASE
                    || next_token.id == JSC_tDEFAULT
                    )
                {
                    // Done with this branch
                    //
                    break;
                    }

                Statement* stmt = parseStatement ();
                if ( stmt == NULL )
                    error( cur_token.linenum, "S0025: statement expected" );

                branch_stmts.append( stmt );
                }

            CaseClause* branch = new(this) CaseClause( linenum, expr, branch_stmts );

            // One clause parsed
            //
            clauses.append( branch );
            }
        else
        {
            error( cur_token.linenum, "S0026: expected case or default label" );
            }
        }

    return new(this) SwitchStatement( linenum, expr, clauses, last_linenum );
    }

Statement*
JSC_Compiler:: parseTryStatement( void )
{
    if ( next_token.id != JSC_tTRY )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    BlockStatement* block = parseBlock ();
    if ( block == NULL )
        error( cur_token.linenum, "S0027: expected block of statements" );

    int try_block_last_linenum = cur_token.linenum;

    // Now we must see `catch' or `finally'
    //
    if ( next_token.id != JSC_tCATCH && next_token.id != JSC_tFINALLY )
        error( cur_token.linenum, "S0028: expected catch or finally block of statements" );

    ListOf<CatchBlock> catch_list;
    BlockStatement* fin_block = NULL;

    if ( next_token.id == JSC_tCATCH )
    {
        // Parse catch list
        //
        while ( next_token.id == JSC_tCATCH )
        {
            getToken ();
            int linenum = cur_token.linenum;

            if ( getToken () != '(' )
                error( cur_token.linenum, "S0029: opening parenthesis expected after catch" );

            if ( getToken () != JSC_tIDENTIFIER )
                error( cur_token.linenum, "S0030: variable identifier expected" );

            String* id = cur_token.vstring;

            Expression* guard = NULL;

            if ( next_token.id == JSC_tIF )
            {
                getToken ();

                guard = parseExpression ();
                if ( guard == NULL )
                    error( cur_token.linenum, "S0031: catch guard expression expected" );
                }

            if ( getToken () != ')' )
                error( cur_token.linenum, "S0032: closing parenthesis expected after catch() clause" );

            BlockStatement* stmt = parseBlock ();
            if ( stmt == NULL )
                error( cur_token.linenum, "S0033: try/catch block of statements expected" );

            catch_list.append( new(this) CatchBlock( linenum, id, guard, stmt ) );
            }
        }

    if ( next_token.id == JSC_tFINALLY )
    {
        // Parse the finally
        //
        getToken ();

        fin_block = parseBlock ();
        if ( fin_block == NULL )
            error( cur_token.linenum, "S0034: try/finally block of statements expected" );
        }

    return new(this) TryStatement( linenum,
        block, catch_list, fin_block,
        try_block_last_linenum,  cur_token.linenum );
    }

Statement*
JSC_Compiler:: parseIfStatement( void )
{
    if ( next_token.id != JSC_tIF )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0035: opening parenthesis expected after if" );

    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0036: expression expected after if(" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0037: closing parentheis expected after if(expr" );

    Statement* if_stmt = parseStatement ();
    if ( if_stmt == NULL )
        error( cur_token.linenum, "S0038: statement expected after if(expr)" );

    Statement* else_stmt = NULL;

    if ( next_token.id == JSC_tELSE )
    {
        getToken ();

        else_stmt = parseStatement ();
        if ( else_stmt == NULL )
            error( cur_token.linenum, "S0039: statement expected after if/else" );
        }

    return new(this) IfStatement( linenum, expr, if_stmt, else_stmt );
    }

Statement*
JSC_Compiler:: parseDoWhileStatement( void )
{
    if ( next_token.id != JSC_tDO )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    Statement* stmt = parseStatement ();
    if ( stmt == NULL )
        error( cur_token.linenum, "S0040: statement expected" );

    if ( getToken () != JSC_tWHILE )
        error( cur_token.linenum, "S0041: while expected after statement in do" );

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0042: opening parenthesis expected after while in do" );

    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0043: expression expected after while( in do" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0044: closing parenthesis expected after while(expr in do" );

    consumeSemicolon ();

    return new(this) DoWhileStatement( linenum, expr, stmt );
    }

Statement*
JSC_Compiler:: parseWhileStatement( void )
{
    if ( next_token.id != JSC_tWHILE )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0045: opening parenthesis expected after while" );

    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0046: expression expected after while(" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0047: closing parenthesis expected after while(expr" );

    Statement* stmt = parseStatement ();
    if ( stmt == NULL )
        error( cur_token.linenum, "S0048: statement expected after while(expr)" );

    return new(this) WhileStatement( linenum, expr, stmt );
    }

Statement*
JSC_Compiler:: parseForStatement( void )
{
    if ( next_token.id != JSC_tFOR )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0049: opening parenthesi expected after for" );

    // Init
    //
    ListOf<VariableDeclaration> vars;
    Expression* expr1 = NULL;
    Expression* expr2 = NULL;
    Expression* expr3 = NULL;

    if ( next_token.id == JSC_tVAR )
    {
        getToken ();
        int linenum = cur_token.linenum;

        for ( ;; )
        {
            if ( next_token.id != JSC_tIDENTIFIER )
            {
                // The next token must be tIDENTIFIER
                //
                error( cur_token.linenum, "S0050: variable identifier expected" );
                }

            getToken ();

            int linenum = cur_token.linenum;
            String* id = cur_token.vstring;
            Expression* expr = NULL;

            if ( next_token.id == '=' )
            {
                getToken ();

                expr = parseAssignmentExpression ();

                if ( expr == NULL )
                    error( cur_token.linenum, "S0051: assignment expression expected" );
                }

            vars.append( new(this) VariableDeclaration( linenum, id, expr ) );

            // Check if we have more input
            //
            if ( next_token.id != ',' )
            {
                // No, we don't
                //
                break;
                }

            getToken ();
            }
        }
    else if ( next_token.id != ';')
    {
        expr1 = parseExpression ();

        if ( expr1 == NULL )
            error( cur_token.linenum, "S0052: expected expression after for(" );
        }

    getToken ();
    if ( cur_token.id != ';' && cur_token.id != JSC_tIN )
    {
        error( cur_token.linenum, "S0100: expected in or ; in for statement" );
        }

    if ( cur_token.id == ';' )
    {
        // Normal for(e1;e2;e3)s statement
        //

        if ( next_token.id != ';' )
        {
            expr2 = parseExpression ();
            if ( expr2 == NULL )
                error( cur_token.linenum, "S0053: expected expression" );
            }

        if ( getToken () != ';' )
            error( cur_token.linenum, "S0054: expected semicolon" );

        if ( next_token.id != ')' )
        {
            expr3 = parseExpression ();
            if ( expr3 == NULL )
                error( cur_token.linenum, "S0055: expected expression" );
            }

        if ( getToken () != ')' )
            error( cur_token.linenum, "S0056: expected closing parenthesis" );

        Statement* stmt = parseStatement ();
        if ( stmt == NULL )
            error( cur_token.linenum, "S0057: expected statement" );

        return new(this) ForStatement( linenum, vars, expr1, expr2, expr3, stmt );
        }

    // The for(e1 in e2)s statement
    //
    if ( expr1 )
    {
        // The first expression must be an identifier
        //
        if ( expr1->type != JSC_IDENTIFIER_EXPR )
            error( cur_token.linenum, "S0058: expected identifier expression" );
        }
    else
    {
        // We must have only one variable declaration
        //
        if ( vars.first == NULL || vars.first->next != NULL )
            error( cur_token.linenum, "S0059: expected only one variable declaration" );
        }

    // The second expressions
    //
    expr2 = parseExpression ();
    if ( expr2 == NULL )
        error( cur_token.linenum, "S0060: expected expression" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0061: expected closing parenthesis" );

    Statement* stmt = parseStatement ();
    if ( stmt == NULL )
        error( cur_token.linenum, "S0062: expected statement" );

    return new(this) ForInStatement( linenum, vars, expr1, expr2, stmt );
    }

Statement*
JSC_Compiler:: parseWithStatement( void )
{
    if ( next_token.id != JSC_tWITH )
        return NULL;

    getToken ();
    int linenum = cur_token.linenum;

    if ( getToken () != '(' )
        error( cur_token.linenum, "S0012: opening parenthesis expected after with" );

    Expression* expr = parseExpression ();
    if ( expr == NULL )
        error( cur_token.linenum, "S0013: expression expected" );

    if ( getToken () != ')' )
        error( cur_token.linenum, "S0014: closing parenthesis expected after expression in with" );

    Statement* stmt = parseStatement ();
    if ( stmt == NULL )
        error( cur_token.linenum, "S0015: statement expected after closing parenthesis in with" );

    return new(this) WithStatement( linenum, expr, stmt );
    }

Expression*
JSC_Compiler:: parseExpression( void )
{
    Expression* expr = parseAssignmentExpression ();
    if ( expr == NULL )
        return NULL;

    // Check for the comma expression
    //
    while ( next_token.id == ',' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr2 = parseAssignmentExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0101: expecting assignment expression" );

        expr = new(this) CommaExpression( linenum, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseAssignmentExpression( void )
{
    Expression* expr = parseConditionalExpression ();
    if ( expr == NULL )
        return NULL;

    if ( // it is left hand side expression, i.e:
        expr->type == JSC_CALL_EXPR
        || expr->type == JSC_OBJECT_PROPERTY_EXPR
        || expr->type == JSC_OBJECT_ARRAY_EXPR
        || expr->type == JSC_NEW_EXPR
        || expr->type == JSC_THIS_EXPR
        || expr->type == JSC_IDENTIFIER_EXPR
        || expr->type == JSC_FLOAT_EXPR
        || expr->type == JSC_INTEGER_EXPR
        || expr->type == JSC_STRING_EXPR
        || expr->type == JSC_ARRAY_INITIALIZER_EXPR
        || expr->type == JSC_NULL_EXPR
        || expr->type == JSC_TRUE_EXPR
        || expr->type == JSC_FALSE_EXPR
        )
    {
        int token = next_token.id;

        if ( token == '=' || token == JSC_tMULA
            || token == JSC_tDIVA || token == JSC_tMODA
            || token == JSC_tADDA || token == JSC_tSUBA
            || token == JSC_tLSIA || token == JSC_tRSIA
            || token == JSC_tRRSA || token == JSC_tANDA
            || token == JSC_tXORA || token == JSC_tORA
            )
        {
            getToken ();
            int linenum = cur_token.linenum;

            Expression* expr2 = parseAssignmentExpression ();
            if ( expr2 == NULL )
                error( cur_token.linenum, "S0102: expecting assignment expression" );

            expr = new(this) AssignmentExpression( linenum, token, expr, expr2 );
            }
        }

    if ( options.optimize_constant_folding && expr->type == JSC_ADDITIVE_EXPR )
    {
        return constant_folding( expr );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseConditionalExpression( void )
{
    Expression* expr = parseLogicalORExpression ();
    if ( expr == NULL )
        return NULL;

    if ( next_token.id != '?' )
        return expr;

    getToken ();
    int linenum = cur_token.linenum;

    Expression* expr2 = parseAssignmentExpression ();
    if ( expr2 == NULL )
        error( cur_token.linenum, "S0103: assignment expression expected" );

    if ( getToken () != ':' )
        error( cur_token.linenum, "S0104: semicolon in conditional expression expected" );

    Expression* expr3 = parseAssignmentExpression ();
    if ( expr3 == NULL )
        error( cur_token.linenum, "S0105: assignment expressoin expected" );

    return new(this) ConditionalExpression( linenum, expr, expr2, expr3 );
    }

Expression*
JSC_Compiler:: parseLogicalORExpression( void )
{
    Expression* expr = parseLogicalANDExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == JSC_tOR )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr1 = expr;
        Expression* expr2 = parseLogicalANDExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0106: logical expression expected" );

        expr = new(this) LogicalORExpression( linenum, expr, expr2 );

        if ( expr1->lang_type == JS_BOOLEAN
            && expr2->lang_type == JS_BOOLEAN )
        {
            expr->lang_type = JS_BOOLEAN;
            }
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseLogicalANDExpression( void )
{
    Expression* expr = parseBitwiseORExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == JSC_tAND )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr1 = expr;
        Expression* expr2 = parseBitwiseORExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0107: bitwise OR expression expected" );

        expr = new(this) LogicalANDExpression( linenum, expr, expr2 );

        if ( expr1->lang_type == JS_BOOLEAN
            && expr2->lang_type == JS_BOOLEAN )
        {
            expr->lang_type = JS_BOOLEAN;
            }
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseBitwiseORExpression( void )
{
    Expression* expr = parseBitwiseXORExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '|' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr2 = parseBitwiseXORExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0108: bitwise XOR expression expected" );

        expr = new(this) BitwiseORExpression( linenum, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseBitwiseXORExpression( void )
{
    Expression* expr = parseBitwiseANDExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '^' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr2 = parseBitwiseANDExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0109: bitwise AND expression expected" );

        expr = new(this) BitwiseXORExpression( linenum, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseBitwiseANDExpression( void )
{
    Expression* expr = parseEqualityExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '&' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        Expression* expr2 = parseEqualityExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0110: equality expression expected" );

        expr = new(this) BitwiseANDExpression( linenum, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseEqualityExpression( void )
{
    Expression* expr = parseRelationalExpression ();
    if ( expr == NULL )
        return NULL;

    while( next_token.id == JSC_tEQUAL || next_token.id == JSC_tNEQUAL
        || next_token.id == JSC_tSEQUAL || next_token.id == JSC_tSNEQUAL
        )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr2 = parseRelationalExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0111: relational expression expected" );

        expr = new(this) EqualityExpression( linenum, token, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseRelationalExpression( void )
{
    Expression* expr = parseShiftExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '<' || next_token.id == '>'
         || next_token.id == JSC_tLE || next_token.id == JSC_tGE
         )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr2 = parseShiftExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0112: shift expression expected" );

        expr = new(this) RelationalExpression( linenum, token, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseShiftExpression ( void )
{
    Expression* expr = parseAdditiveExpression ();
    if ( expr == NULL )
        return NULL;
  
    while ( next_token.id == JSC_tLSHIFT || next_token.id == JSC_tRSHIFT
        || next_token.id == JSC_tRRSHIFT
        )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr2 = parseAdditiveExpression ();

        if ( expr2 == NULL )
            error( cur_token.linenum, "S0113: additive expression expected" );

        expr = new(this) ShiftExpression( linenum, token, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseAdditiveExpression( void )
{
    Expression* expr = parseMultiplicativeExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '+' || next_token.id == '-' )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr2 = parseMultiplicativeExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0114: multiplicative expression expected" );

        expr = new(this) AdditiveExpression( linenum, token, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseMultiplicativeExpression( void )
{
    Expression* expr = parseUnaryExpression ();
    if ( expr == NULL )
        return NULL;

    while ( next_token.id == '*' || next_token.id == '/' || next_token.id == '%' )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr2 = parseUnaryExpression ();
        if ( expr2 == NULL )
            error( cur_token.linenum, "S0115: unary expression expected" );

        expr = new(this) MultiplicativeExpression( linenum, token, expr, expr2 );
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseUnaryExpression( void )
{
    if ( next_token.id == JSC_tDELETE
        || next_token.id == JSC_tVOID
        || next_token.id == JSC_tTYPEOF
        || next_token.id == JSC_tPLUSPLUS
        || next_token.id == JSC_tMINUSMINUS
        || next_token.id == '+'
        || next_token.id == '-'
        || next_token.id == '~'
        || next_token.id == '!'
        )
    {
        getToken ();
        int linenum = cur_token.linenum;
        int token = cur_token.id;

        Expression* expr = parseUnaryExpression ();
        if ( expr == NULL )
            error( cur_token.linenum, "S0116: unary expressino expected" );

        return new(this) UnaryExpression( linenum, token, expr );
        }

    return parsePostfixExpression ();
    }


Expression*
JSC_Compiler:: parsePostfixExpression( void )
{
    Expression* expr = parseLeftHandSideExpression ();
    if ( expr == NULL )
        return NULL;

    if ( next_token.id == JSC_tPLUSPLUS || next_token.id == JSC_tMINUSMINUS )
    {
        if ( next_token.linenum > cur_token.linenum )
        {
            if ( options.warn_missing_semicolon )
                warning( cur_token.linenum, "automatic semicolon insertion cuts the expression before ++ or --" );
            }
        else
        {
            getToken ();
            int linenum = cur_token.linenum;
            int token = cur_token.id;

            return new(this) PostfixExpression( linenum, token, expr );
            }
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseLeftHandSideExpression( void )
{
    Expression* expr = parseMemberExpression ();
    if ( expr == NULL )
        return NULL;

    // Parse the possible first pair of arguments
    //
    if ( next_token.id == '(' )
    {
        int linenum = next_token.linenum;

        ListOf<ArgExpression> args;
        parseArguments( args );

        expr = new(this) CallExpression( linenum, expr, args );
        }
    else
    {
        return expr;
        }

    // Parse to possibly following arguments and selectors
    //
    while ( next_token.id == '(' || next_token.id == '[' || next_token.id == '.' )
    {
        if ( next_token.id == '(' )
        {
            int linenum = next_token.linenum;

            ListOf<ArgExpression> args;
            parseArguments( args );

            expr = new(this) CallExpression( linenum, expr, args );
            }
        else if ( next_token.id == '[' )
        {
            getToken ();
            int linenum = cur_token.linenum;

            Expression* expr2 = parseExpression();
            if ( expr2 == NULL )
                error( cur_token.linenum, "S0117: expression epxected" );

            if ( getToken () != ']' )
                error( cur_token.linenum, "S0118: closing ] expected" );

            expr = new(this) ObjectArrayExpression( linenum, expr, expr2 );
            }
        else
        {
            getToken ();
            int linenum = cur_token.linenum;

            if ( getToken () != JSC_tIDENTIFIER )
                error( cur_token.linenum, "S0119: identifier expected" );

            expr = new(this) ObjectPropertyExpression( linenum, cur_token.vstring, expr );
            }
        }

    return expr;
    }

Expression*
JSC_Compiler:: parseMemberExpression( void )
{
    Expression* expr = parsePrimaryExpression ();
    if ( expr == NULL )
    {
        int token = next_token.id;

        if ( token != JSC_tNEW )
        {
            return NULL;
            }

        getToken ();
        int linenum = cur_token.linenum;

        expr = parseMemberExpression ();
        if ( expr == NULL )
            error( cur_token.linenum, "S0120: member expression expected" );

        ListOf<ArgExpression> args;

        if ( next_token.id == '(' )
        {
            parseArguments( args );
            }
        else
        {
            // A `new(this) Foo' call. This is identical to `new(this) Foo ()'
            //
            return new(this) NewExpression( linenum, expr, args ); // TODO: why return ?
            }

        expr = new(this) NewExpression( linenum, expr, args );
        }

    // Ok, now we have valid starter
    //
    while ( next_token.id == '[' || next_token.id == '.' )
    {
        if ( next_token.id == '[' )
        {
            getToken ();
            int linenum = cur_token.linenum;

            Expression* expr2 = parseExpression ();
            if ( expr2 == NULL )
                error( cur_token.linenum, "S0121: expression expected" );

            if ( getToken () != ']' )
                error( cur_token.linenum, "S0122: closing ] expected" );

            expr = new(this) ObjectArrayExpression( linenum, expr, expr2 );
            }
        else // ( next_token.id == '.' )
        {
            getToken ();
            int linenum = cur_token.linenum;

            if ( getToken () != JSC_tIDENTIFIER )
                error( cur_token.linenum, "S0123: identifier expected" );

            expr = new(this) ObjectPropertyExpression( linenum, cur_token.vstring, expr );
            }
        }

    return expr;
    }

Expression*
JSC_Compiler:: parsePrimaryExpression( void )
{
    if ( next_token.id == JSC_tTHIS )
    {
        getToken ();

        return new(this) ThisExpression( cur_token.linenum );
        }

    if ( next_token.id == JSC_tIDENTIFIER )
    {
        getToken ();

        return new(this) IdentifierExpression( cur_token.linenum, cur_token.vstring );
        }

    if ( next_token.id == JSC_tFLOAT )
    {
        getToken ();

        return new(this) FloatExpression( cur_token.linenum, cur_token.vfloat );
        }

    if ( next_token.id == JSC_tINTEGER )
    {
        getToken ();

        return new(this) IntegerExpression( cur_token.linenum, cur_token.vinteger );
        }

    if ( next_token.id == JSC_tSTRING )
    {
        getToken ();

        return new(this) StringExpression( cur_token.linenum, cur_token.vstring );
        }
/*
    if ( next_token.id == '/')
    {
        getToken ();

        // Kludge alert!  The regular expression constants (/.../) and
        // div operands are impossible to distinguish, based only on the
        // lexical analysis.  Therefore, we need some syntactical
        // knowledge when the regular expression constants are possible
        // at all.  This is the place where they can appear.  In all
        // other places, the character `/' is interpreted as a div
        // operator.
        //
        return new(this) RegexpExpression( cur_token.linenum, readRegExpConstantConstant() );
        }
*/
    if ( next_token.id == JSC_tNULL )
    {
        getToken ();

        return new(this) NullExpression( cur_token.linenum );
        }

    if ( next_token.id == JSC_tTRUE )
    {
        getToken ();

        return new(this) TrueExpression( cur_token.linenum );
        }

    if ( next_token.id == JSC_tFALSE )
    {
        getToken ();

        return new(this) FalseExpression( cur_token.linenum );
        }

    if ( next_token.id == '[' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        // Array initializer // TODO: SharpVarDefinition_{opt}
        //

        ListOf<ArgExpression> items;

        while ( next_token.id != ']' )
        {
            if ( next_token.id == ',' )
            {
                getToken ();
                items.appendReverse( new(this) ArgExpression( NULL ) );
                continue;
                }

            Expression* expr = parseAssignmentExpression ();
            if ( expr == NULL )
                error( cur_token.linenum, "S0124: assignment expression" );

            items.appendReverse( new(this) ArgExpression( expr ) );

            // Got one expression. It must be followed by ',' or ']'
            //
            if ( next_token.id == ',' )
            {
                getToken ();
                }
            else if ( next_token.id != ']' )
            {
                error( cur_token.linenum, "S0125: closing ] epxected" );
                }
            }

        getToken ();

        return new(this) ArrayInitializerExpression( linenum, items );
        }

    if ( next_token.id == '{' )
    {
        getToken ();
        int linenum = cur_token.linenum;

        // Object literal // TODO: SharpVarDefinition_{opt}
        //

        ListOf<ObjectInitializerPair> items;

        while ( next_token.id != '}' )
        {
            getToken ();
            int linenum = cur_token.linenum;
            int id_type = cur_token.id;
            String* id = cur_token.vstring;

            if ( cur_token.id != JSC_tSTRING )
                error( cur_token.linenum, "S0126: string constant expected" );

            //TODO: if ( cur_token.id != JSC_tIDENTIFIER && cur_token.id != JSC_tSTRING && cur_token.id != JSC_tINTEGER )
                //error( cur_token.linenum, "S0126: identifier, string or integer constant expected" );

            if ( getToken () != ':' )
                error( cur_token.linenum, "S0127: semicolon expected" );

            Expression* expr = parseAssignmentExpression ();
            if ( expr == NULL )
                error( cur_token.linenum, "S0128: assignment expression expected" );

            items.append( new(this) ObjectInitializerPair( linenum, id_type, id, expr ) );

            //
            // Got one property, initializer pair. It must be followed by ',' or '}'.
            //
            if ( next_token.id == ',' )
            {
                // Ok, we have more items
                //
                getToken ();

                if ( next_token.id != JSC_tIDENTIFIER && next_token.id != JSC_tSTRING && next_token.id != JSC_tINTEGER )
                    error( cur_token.linenum, "S0129: identifier, string or integer constant expected" );
                }
            else if ( next_token.id != '}' )
            {
                error( cur_token.linenum, "S0130: closing brace epxected" );
                }
            }

        getToken ();

        return new(this) ObjectInitializerExpression( linenum, items );
        }

    if ( next_token.id == '(' )
    {
        getToken ();

        Expression* expr = parseExpression ();

        if ( expr == NULL )
            error( cur_token.linenum, "S0131: expression expected" );
        
        if ( getToken () != ')' )
            error( cur_token.linenum, "S0132: closing parenthesis epxected" );

        return expr;
        }

    return NULL;
    }

void
JSC_Compiler:: parseArguments ( ListOf<ArgExpression>& args )
{
    if ( getToken () != '(' )
        error( cur_token.linenum, "S0134: opening parenthesis expected" );

    while ( next_token.id != ')' )
    {
        Expression* item = parseAssignmentExpression ();
        if ( item == NULL )
            error( cur_token.linenum, "S0135: assignment expression expected" );

        args.appendReverse( new(this) ArgExpression( item ) );

        if ( next_token.id == ',' )
        {
            getToken ();
            }
        else if ( next_token.id != ')' )
        {
            error( cur_token.linenum, "S0136: closing parenthesis or comma expected" );
            }
        }

    getToken (); // consume trailing ')'
    }

Expression*
JSC_Compiler:: constant_folding( Expression* expr )
{
    if ( expr->type == JSC_ADDITIVE_EXPR )
    {
        AdditiveExpression* e = (AdditiveExpression*)expr;
        e->expr1 = constant_folding( e->expr1 );
        e->expr2 = constant_folding( e->expr2 );

        // TODO: This could be smarter.
        //
        if ( e->expr1->type == e->expr2->type )
        {
            switch ( e->expr1->type )
            {
                case JSC_INTEGER_EXPR:
                    return new(this) IntegerExpression( e->linenum, 
                                       e->token == '+'
                                       ? ( (IntegerExpression*)e->expr1 )->value + ( (IntegerExpression*)e->expr2 )->value
                                       : ( (IntegerExpression*)e->expr1 )->value - ( (IntegerExpression*)e->expr2 )->value
                                       );

                case JSC_FLOAT_EXPR:
                    return new(this) FloatExpression( e->linenum, 
                                       e->token == '+'
                                       ? ( (FloatExpression*)e->expr1 )->value + ( (FloatExpression*)e->expr2 )->value
                                       : ( (FloatExpression*)e->expr1 )->value - ( (FloatExpression*)e->expr2 )->value
                                       );

                case JSC_STRING_EXPR:
                    if ( e->token == '+' )
                    {
                        // Only the addition is available for the strings.
                        //
                        char buf[ 1024 ];
                        String c( buf, 0 );
                        c += ( (StringExpression*)e->expr1 )->value;
                        c += ( (StringExpression*)e->expr2 )->value;

                        return new(this) StringExpression( e->linenum, strDup( this, c ) );
                        }
                    break;
                }

            }
        }

    return expr;
    }
