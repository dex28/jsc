#ifndef _JSC_H_INCLUDED
#define _JSC_H_INCLUDED

//////////////////////////////////////
// JavaScript Compiler include file.
//

// JavaScript VM
//
#include "JS.h"

// #define ALLOW_HEAVY_OPTIMIZATION

//
// Internal definitions.
//

class JSC_Compiler;

class ListElement;
    class Statement;
        class FunctionStatement;
        class BlockStatement;
        class VariableStatement;
        class EmptyStatement;
        class ExpressionStatement;
        class HtmlOutStatement;
        class IfStatement;
        class WhileStatement;
        class ForStatement;
        class ForInStatement;
        class DoWhileStatement;
        class ContinueStatement;
        class BreakStatement;
        class ReturnStatement;
        class ThrowStatement;
        class WithStatement;
        class SwitchStatement;
        class TryStatement;
        class LabeledStatement;
    class CatchBlock;
    class CaseClause;
    class VariableDeclaration;
    class ArgDeclaration;
    class FunctionDeclaration;
    class ArgExpression;
    class ObjectInitializerPair;
class Expression;
    class ThisExpression;
    class IdentifierExpression;
    class FloatExpression;
    class IntegerExpression;
    class StringExpression;
    class ArrayInitializerExpression;
    class ObjectInitializerExpression;
    class ObjectInitializerPair;
    class NullExpression;
    class TrueExpression;
    class FalseExpression;
    class MultiplicativeExpression;
    class AdditiveExpression;
    class ShiftExpression;
    class RelationalExpression;
    class EqualityExpression;
    class BitwiseANDExpression;
    class BitwiseORExpression;
    class BitwiseXORExpression;
    class LogicalANDExpression;
    class LogicalORExpression;
    class NewExpression;
    class ObjectPropertyExpression;
    class ObjectArrayExpression;
    class CallExpression;
    class AssignmentExpression;
    class ConditionalExpression;
    class UnaryExpression;
    class PostfixExpression;
    class CommaExpression;
class ASM_statement;
    class ASM_symbolDef;
    class ASM_labelDef;
    class ASM_voidArg;
    class ASM_int32Arg;
    class ASM_labelArg;
    class ASM_constArg;
    class ASM_symbolArg;

///////////////////////////////////////////////////////////////////////////////
// LexerIStream

class LexerIStream
{
public:

    const char* name;
    JSIOStream* stream;
    int last_ch;
    int linenum;

    LexerIStream( void )
    {
        name = NULL;
        stream = NULL;
        last_ch = '\n'; // force linenum to 1 on next readByte
        linenum = 0;
        }

    void rewind( void )
    {
        stream->Seek( 0L, SEEK_SET );
        last_ch = '\n';
        linenum = 0;
        }

    int readLine( char* line, int max_size )
    {
        char* p = line;
        int len = 0;

        while ( len < max_size - 1 )
        {
            int ch = stream->ReadByte ();
            if ( ch == '\r' )
                continue;
            else if ( ch == '\n' )
                break;

            *p++ = ch;
            len++;
            }

        *p = 0;

        return len;
        }

    int peekCh( void )
    {
        int ch = stream->ReadByte ();
        stream->Unget( ch );
        return ch;
        }

    int readCh( void )
    {
        if ( last_ch == '\n' )
            linenum ++;

        last_ch = stream->ReadByte ();
        return last_ch;
        }

    int ungetCh( int ch )
    {
        if ( ch == '\n' )
            linenum --;

        return stream->Unget( ch );
        }
    };

///////////////////////////////////////////////////////////////////////////////
// String

class String
{
public:

    int len;
    char* data;

    String( char* buffer, int arg_len = -1 )
    {
        if ( arg_len < 0 )
            len = strlen( buffer );
        else
            len = arg_len;
        data = buffer;
        }

    void operator += ( int ch ) { data[ len++ ] = char( ch ); data[ len ] = 0; }
    void operator += ( const char* str ) { strcpy( data + len, str ); len += strlen( str ); }
    void operator += ( String* str ) { strcpy( data + len, str->data ); len += str->len; }

    bool operator == ( const char* str ) { return strcmp( data, str ) == 0; }
    bool operator != ( const char* str ) { return strcmp( data, str ) != 0; }

    operator const char* ( void ) { return data; }
    operator double( void ) { double x = 0; sscanf( data, "%lf", &x ); return x; }
    operator long( void ) { long x = 0; sscanf( data, "%ld", &x ); return x; }

    int length () { data[ len ] = 0; return len; }
    };

///////////////////////////////////////////////////////////////////////////////
// Token

enum // Token identifiers
{
    // 0 - 127 are reserved for first row Unicode characters
    //
    JSC_tEOF = 128,

    JSC_tINTEGER,
    JSC_tFLOAT,
    JSC_tSTRING,
    JSC_tIDENTIFIER,

    JSC_tBREAK,
    JSC_tCONTINUE,
    JSC_tDELETE,
    JSC_tELSE,
    JSC_tFOR,
    JSC_tFUNCTION,
    JSC_tIF,
    JSC_tIN,
    JSC_tNEW,
    JSC_tRETURN,
    JSC_tTHIS,
    JSC_tTYPEOF,
    JSC_tVAR,
    JSC_tVOID,
    JSC_tWHILE,
    JSC_tWITH,

    JSC_tCASE,
    JSC_tCATCH,
    JSC_tCLASS,
    JSC_tCONST,
    JSC_tDEBUGGER,
    JSC_tDEFAULT,
    JSC_tDO,
    JSC_tENUM,
    JSC_tEXPORT,
    JSC_tEXTENDS,
    JSC_tFINALLY,
    JSC_tIMPORT,
    JSC_tSUPER,
    JSC_tSWITCH,
    JSC_tTHROW,
    JSC_tTRY,

    JSC_tNULL,
    JSC_tTRUE,
    JSC_tFALSE,

    JSC_tEQUAL,
    JSC_tNEQUAL,
    JSC_tLE,
    JSC_tGE,
    JSC_tAND,
    JSC_tOR,
    JSC_tPLUSPLUS,
    JSC_tMINUSMINUS,
    JSC_tMULA,
    JSC_tDIVA,
    JSC_tMODA,
    JSC_tADDA,
    JSC_tSUBA,
    JSC_tANDA,
    JSC_tXORA,
    JSC_tORA,
    JSC_tLSIA,
    JSC_tLSHIFT,
    JSC_tRSHIFT,
    JSC_tRRSHIFT,
    JSC_tRSIA,
    JSC_tRRSA,
    JSC_tSEQUAL,
    JSC_tSNEQUAL,

    JSC_tENDJSP,
    JSC_tBEGJSPCODE,
    JSC_tBEGJSPEXPR,
    JSC_tHTMLSTRING,

    JSC_tLASTONE
    };

class Token
{
public:
    int id;
    int linenum;
    union
    {
        long vinteger;
        double vfloat;
        String* vstring;
        };
    };

///////////////////////////////////////////////////////////////////////////////
// ListElement

class ListElement
{
public:
    ListElement* next;

    ListElement( void ) { next = NULL; }
    };

// Example of the template keyword
template <class ListElement>
class ListOf
{
public:
    int count;
    ListElement* first;
    ListElement* last;

    ListOf( void )
    {
        count = 0;
        first = NULL;
        last = NULL;
        }

    void append( ListElement* pe )
    {
        count ++;

        pe->next = NULL;

        if ( last != NULL )
            last->next = pe;
        else
            first = pe;

        last = pe;
        }

    void appendReverse( ListElement* pe )
    {
        count ++;

        pe->next = first;
        first = pe;

        if ( last == NULL )
            last = first;
        }

    int length( void )
    {
        return count;
        }
    };

extern int countLocals( ListOf<Statement>& list );

///////////////////////////////////////////////////////////////////////////////
// Statements

enum // Statement types
{
    JSC_NONVAR_STMT,
    JSC_VARIABLE_STMT
    };

class Statement : public ListElement
{
public:
    short type;
    int linenum;

    Statement( int arg_type, int arg_linenum ) { type = arg_type; linenum = arg_linenum; }
    virtual void Assemble( JSC_Compiler* jsc ) {}
    virtual int CountLocals( bool fRecursive ) { return 0; }
    };

class BlockStatement : public Statement
{
public:
    ListOf<Statement> list;

    BlockStatement( int arg_linenum, ListOf<Statement>& arg_stmts  )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        list = arg_stmts;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class EmptyStatement : public Statement
{
public:
    EmptyStatement( int arg_linenum )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ContinueStatement : public Statement
{
public:
    String* label;

    ContinueStatement( int arg_linenum, String* arg_label )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        label = arg_label;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class BreakStatement : public Statement
{
public:
    String* label;

    BreakStatement( int arg_linenum, String* arg_label )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        label = arg_label;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ReturnStatement : public Statement
{
public:
    Expression* expr;

    ReturnStatement( int arg_linenum, Expression* arg_expr )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ThrowStatement : public Statement
{
public:
    Expression* expr;

    ThrowStatement( int arg_linenum, Expression* arg_expr )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class SwitchStatement : public Statement
{
public:
    Expression* expr;
    ListOf<CaseClause> clauses;
    int last_linenum;

    SwitchStatement( int arg_linenum, Expression* arg_expr, ListOf<CaseClause>& arg_clauses, int arg_lastln )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        clauses = arg_clauses;
        last_linenum = arg_lastln;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class CaseClause : public ListElement
{
public:
    Expression* expr;
    ListOf<Statement> list;
    int linenum;
    int last_linenum;

    CaseClause( int arg_linenum, Expression* arg_expr, ListOf<Statement>& arg_stmts )
    {
        linenum = arg_linenum;
        last_linenum = 0; // TODO do it right way
        expr = arg_expr;
        list = arg_stmts;
        }
    };

class WithStatement : public Statement
{
public:
    Expression* expr;
    Statement* stmt;

    WithStatement( int arg_linenum, Expression* arg_expr, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class TryStatement : public Statement
{
public:
    BlockStatement* try_block;
    ListOf<CatchBlock> catch_list;
    BlockStatement* fin_block;
    int tryblock_last_linenum;
    int catchlist_last_linenum;
    int try_last_linenum;

    TryStatement( int arg_linenum, BlockStatement* arg_try, ListOf<CatchBlock>& arg_catches, BlockStatement* arg_fin, int arg_trylastln, int arg_lastln )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        try_block = arg_try;
        catch_list = arg_catches;
        fin_block = arg_fin;
        tryblock_last_linenum = arg_trylastln;
        catchlist_last_linenum = 0; // TODO fix this
        try_last_linenum = arg_lastln;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class CatchBlock : public ListElement
{
public:
    int linenum;
    String* varname;
    Expression* guard_expr;
    BlockStatement* block;

    CatchBlock( int arg_linenum, String* arg_varname, Expression* arg_guard, BlockStatement* arg_block )
    {
        linenum = arg_linenum;
        varname = arg_varname;
        guard_expr = arg_guard;
        block = arg_block;
        }
    };

class LabeledStatement : public Statement
{
public:
    String* label;
    Statement* stmt;

    LabeledStatement( int arg_linenum, String* arg_label, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        label = arg_label;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class ExpressionStatement : public Statement
{
public:
    Expression* expr;

    ExpressionStatement( int arg_linenum, Expression* arg_expr )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class HtmlOutStatement : public Statement
{
public:
    Expression* expr1;
    Expression* expr2;

    HtmlOutStatement( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class IfStatement : public Statement
{
public:
    Expression* expr;
    Statement* if_stmt;
    Statement* else_stmt;

    IfStatement( int arg_linenum, Expression* arg_expr, Statement* arg_if, Statement* arg_else )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        if_stmt = arg_if;
        else_stmt = arg_else;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class DoWhileStatement : public Statement
{
public:
    Expression* expr;
    Statement* stmt;

    DoWhileStatement( int arg_linenum, Expression* arg_expr, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class WhileStatement : public Statement
{
public:
    Expression* expr;
    Statement* stmt;

    WhileStatement( int arg_linenum, Expression* arg_expr, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        expr = arg_expr;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class ForStatement : public Statement
{
public:
    ListOf<VariableDeclaration> vars;
    Expression* expr1;
    Expression* expr2;
    Expression* expr3;
    Statement* stmt;

    ForStatement( int arg_linenum, ListOf<VariableDeclaration>& arg_vars, Expression* arg_expr1, Expression* arg_expr2, Expression* arg_expr3, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        vars = arg_vars;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        expr3 = arg_expr3;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class ForInStatement : public Statement
{
public:
    ListOf<VariableDeclaration> vars;
    Expression* expr1;
    Expression* expr2;
    Statement* stmt;

    ForInStatement( int arg_linenum, ListOf<VariableDeclaration>& arg_vars, Expression* arg_expr1, Expression* arg_expr2, Statement* arg_stmt )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        vars = arg_vars;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        stmt = arg_stmt;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class FunctionStatement : public Statement
{
public:
    String* container_name;
    String* function_name;
    String* given_name;

    FunctionStatement( int arg_linenum, String* arg_container_name, String* arg_function_name, String* arg_given_name )
        : Statement( JSC_NONVAR_STMT, arg_linenum )
    {
        container_name = arg_container_name;
        function_name = arg_function_name;
        given_name = arg_given_name;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class VariableStatement : public Statement
{
public:
    bool global_level;
    ListOf<VariableDeclaration> vars;

    VariableStatement( int arg_linenum, ListOf<VariableDeclaration>& arg_vars )
        : Statement( JSC_VARIABLE_STMT, arg_linenum )
    {
        vars = arg_vars;
        global_level = false;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    virtual int CountLocals( bool fRecursive );
    };

class VariableDeclaration : public ListElement
{
public:
    int linenum;
    String* id;
    Expression* expr;

    VariableDeclaration( int arg_linenum, String* arg_id, Expression* arg_expr )
    {
        linenum = arg_linenum;
        id = arg_id;
        expr = arg_expr;
        }
    };

class FunctionDeclaration : public ListElement
{
public:
    int linenum;
    int lbrace_linenum;
    String* name;
    String* given_name;
    ListOf<ArgDeclaration> args;
    BlockStatement* block;

    FunctionDeclaration( int arg_linenum, String* arg_name, String* arg_given_name, ListOf<ArgDeclaration>& arg_args, int arg_lbraceln, BlockStatement* arg_block )
    {
        linenum = arg_linenum;
        name = arg_name;
        given_name = arg_given_name;
        args = arg_args;
        lbrace_linenum = arg_lbraceln;
        block = arg_block;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ArgDeclaration : public ListElement
{
public:
    String* id;

    ArgDeclaration( int arg_linenum, String* arg_id )
    {
        id = arg_id;
        }
    };

///////////////////////////////////////////////////////////////////////////////
// Expressions

enum // Expression types
{
    JSC_COMMA_EXPR,
    JSC_ASSIGNMENT_EXPR,
    JSC_CONDITIONAL_EXPR,
    JSC_LOGICAL_OR_EXPR,
    JSC_LOGICAL_AND_EXPR,
    JSC_BITWISE_OR_EXPR,
    JSC_BITWISE_XOR_EXPR,
    JSC_BITWISE_AND_EXPR,
    JSC_EQUALITY_EXPR,
    JSC_RELATIONAL_EXPR,
    JSC_SHIFT_EXPR,
    JSC_MULTIPLICATIVE_EXPR,
    JSC_ADDITIVE_EXPR,
    JSC_THIS_EXPR,
    JSC_NULL_EXPR,
    JSC_TRUE_EXPR,
    JSC_FALSE_EXPR,
    JSC_IDENTIFIER_EXPR,
    JSC_FLOAT_EXPR,
    JSC_INTEGER_EXPR,
    JSC_STRING_EXPR,
    JSC_CALL_EXPR,
    JSC_OBJECT_PROPERTY_EXPR,
    JSC_OBJECT_ARRAY_EXPR,
    JSC_NEW_EXPR,
    JSC_DELETE_EXPR,
    JSC_VOID_EXPR,
    JSC_TYPEOF_EXPR,
    JSC_POSTFIX_EXPR,
    JSC_UNARY_EXPR,
    JSC_ARRAY_INITIALIZER_EXPR,
    JSC_OBJECT_INITIALIZER_EXPR,
    };

class Expression
{
public:
    int linenum;
    short type;
    short lang_type;

    Expression( int arg_linenum, int arg_type, int arg_langtype = -1 )
    {
        linenum = arg_linenum;
        type = arg_type;
        lang_type = arg_langtype;
        }

    virtual void Assemble( JSC_Compiler* jsc ) {}
    };

class ThisExpression : public Expression
{
public:
    ThisExpression( int arg_linenum )
        : Expression( arg_linenum, JSC_THIS_EXPR )
    {
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class IdentifierExpression : public Expression
{
public:
    String* value;

    IdentifierExpression( int arg_linenum, String* arg_value )
        : Expression( arg_linenum, JSC_IDENTIFIER_EXPR )
    {
        value = arg_value;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class FloatExpression : public Expression
{
public:
    double value;

    FloatExpression( int arg_linenum, double arg_value )
        : Expression( arg_linenum, JSC_FLOAT_EXPR, JS_FLOAT )
    {
        value = arg_value;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class IntegerExpression : public Expression
{
public:
    long value;

    IntegerExpression( int arg_linenum, long arg_value )
        : Expression( arg_linenum, JSC_INTEGER_EXPR, JS_INTEGER )
    {
        value = arg_value;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class StringExpression : public Expression
{
public:
    String* value;

    StringExpression( int arg_linenum, String* arg_value )
        : Expression( arg_linenum, JSC_STRING_EXPR, JS_STRING )
    {
        value = arg_value;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ArrayInitializerExpression : public Expression
{
public:
    ListOf<ArgExpression> items;

    ArrayInitializerExpression( int arg_linenum, ListOf<ArgExpression>& arg_items )
        : Expression( arg_linenum, JSC_ARRAY_INITIALIZER_EXPR, JS_ARRAY )
    {
        items = arg_items;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ArgExpression : public ListElement
{
public:
    Expression* expr;

    ArgExpression( Expression* arg_expr )
    {
        expr = arg_expr;
        }
    };

class ObjectInitializerExpression : public Expression
{
public:
    ListOf<ObjectInitializerPair> items;

    ObjectInitializerExpression( int arg_linenum, ListOf<ObjectInitializerPair>& arg_items )
        : Expression( arg_linenum, JSC_OBJECT_INITIALIZER_EXPR, JS_OBJECT )
    {
        items = arg_items;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ObjectInitializerPair : public ListElement
{
public:
    int linenum;
    int id_type;
    String* id;
    Expression* expr;

    ObjectInitializerPair( int arg_linenum, int arg_idtype, String* arg_id, Expression* arg_expr )
    {
        linenum = arg_linenum;
        id_type = arg_idtype;
        id = arg_id;
        expr = arg_expr;
        }
    };

class NullExpression : public Expression
{
public:
    NullExpression( int arg_linenum )
        : Expression( arg_linenum, JSC_NULL_EXPR, JS_NULL )
    {
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class TrueExpression : public Expression
{
public:
    TrueExpression( int arg_linenum )
        : Expression( arg_linenum, JSC_TRUE_EXPR, JS_BOOLEAN )
    {
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class FalseExpression : public Expression
{
public:
    FalseExpression( int arg_linenum )
        : Expression( arg_linenum, JSC_FALSE_EXPR, JS_BOOLEAN )
    {
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class MultiplicativeExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    MultiplicativeExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_MULTIPLICATIVE_EXPR )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class AdditiveExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    AdditiveExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_ADDITIVE_EXPR )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ShiftExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    ShiftExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_SHIFT_EXPR )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class RelationalExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    RelationalExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_RELATIONAL_EXPR, JS_BOOLEAN )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class EqualityExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    EqualityExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_EQUALITY_EXPR, JS_BOOLEAN )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class BitwiseANDExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    BitwiseANDExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_BITWISE_AND_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class BitwiseORExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    BitwiseORExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_BITWISE_OR_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class BitwiseXORExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    BitwiseXORExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_BITWISE_XOR_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class LogicalANDExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    LogicalANDExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_LOGICAL_AND_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class LogicalORExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    LogicalORExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_LOGICAL_OR_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class NewExpression : public Expression
{
public:
    Expression* expr;
    ListOf<ArgExpression> args;

    NewExpression( int arg_linenum, Expression* arg_expr, ListOf<ArgExpression>& arg_args )
        : Expression( arg_linenum, JSC_NEW_EXPR )
    {
        expr = arg_expr;
        args = arg_args;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ObjectPropertyExpression : public Expression
{
public:
    String* id;
    Expression* expr;

    ObjectPropertyExpression( int arg_linenum, String* arg_id, Expression* arg_expr )
        : Expression( arg_linenum, JSC_OBJECT_PROPERTY_EXPR )
    {
        id = arg_id;
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ObjectArrayExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    ObjectArrayExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_OBJECT_ARRAY_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class CallExpression : public Expression
{
public:
    Expression* expr;
    ListOf<ArgExpression> args;

    CallExpression( int arg_linenum, Expression* arg_expr, ListOf<ArgExpression>& arg_args )
        : Expression( arg_linenum, JSC_CALL_EXPR )
    {
        expr = arg_expr;
        args = arg_args;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class AssignmentExpression : public Expression
{
public:
    int token;
    Expression* expr1;
    Expression* expr2;

    AssignmentExpression( int arg_linenum, int arg_token, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_ASSIGNMENT_EXPR )
    {
        token = arg_token;
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class ConditionalExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;
    Expression* expr3;

    ConditionalExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2, Expression* arg_expr3 )
        : Expression( arg_linenum, JSC_CONDITIONAL_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        expr3 = arg_expr3;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class UnaryExpression : public Expression
{
public:
    int token;
    Expression* expr;

    UnaryExpression( int arg_linenum, int arg_token, Expression* arg_expr )
        : Expression( arg_linenum, JSC_UNARY_EXPR )
    {
        token = arg_token;
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class PostfixExpression : public Expression
{
public:
    int token;
    Expression* expr;

    PostfixExpression( int arg_linenum, int arg_token, Expression* arg_expr )
        : Expression( arg_linenum, JSC_POSTFIX_EXPR )
    {
        token = arg_token;
        expr = arg_expr;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

class CommaExpression : public Expression
{
public:
    Expression* expr1;
    Expression* expr2;

    CommaExpression( int arg_linenum, Expression* arg_expr1, Expression* arg_expr2 )
        : Expression( arg_linenum, JSC_COMMA_EXPR )
    {
        expr1 = arg_expr1;
        expr2 = arg_expr2;
        }

    virtual void Assemble( JSC_Compiler* jsc );
    };

///////////////////////////////////////////////////////////////////////////////
// ContinueBreak Frame

/*
    The handling of the 'continue' and 'break' labels for looping constructs.
    
    The instance contains a valid chain of looping constructs and
    the currently active with and try nesting levels. The actual 'continue',
    'break', and 'return' statements investigate the chain and generate
    appropriate 'with_pop' and 'try_pop' opcodes.

    If the instance variable 'inswitch' is true, the continue statement
    is inside a switch statement. In this case, the continue statement
    must pop one item from the stack. That item is the value of the case
    expression.
*/

class ContBreakNest
{
    struct ContBreakFrame
    {
        String* label;
        ASM_labelDef* loop_break;
        ASM_labelDef* loop_continue;
        bool in_switch;
        int num_with_nesting;
        int num_try_nesting;
        ContBreakFrame* next;
        };

    JSC_Compiler* jsc; // context
    ContBreakFrame* pTop; // top of cont/break frame stack
    ContBreakFrame* pDeleted; // top of cont/break frame pool

public:
    void Create( JSC_Compiler* ctx );

    void push( String* arg_label, ASM_labelDef* arg_loopbreak, ASM_labelDef* arg_loopcont, bool arg_in_switch = false );
    void pop( void );

    int countTryReturnNesting( void );
    int countWithNesting( String* label = NULL );
    int countTryNesting( String* label = NULL );
    int countSwitchNesting( String* label = NULL );

    int numTryNesting () { return pTop->num_try_nesting; }
    void incNumTryNesting () { pTop->num_try_nesting ++; }
    void decNumTryNesting () { pTop->num_try_nesting --; }

    int numWithNesting () { return pTop->num_with_nesting; }
    void incNumWithNesting () { pTop->num_with_nesting ++; }
    void decNumWithNesting () { pTop->num_with_nesting --; }
    void setNumWithNesting( int n ) { pTop->num_with_nesting = n; }

    ASM_labelDef* getContinue( String* label );
    ASM_labelDef* getBreak( String* label );
    bool isUniqueLabel( String* label );

    void dump( void );
    };

///////////////////////////////////////////////////////////////////////////////
// Namespace

enum
{
    JSC_SCOPE_ARG    = 1,
    JSC_SCOPE_LOCAL  = 2
    };

struct SymbolDefinition
{
    String* name;
    int scope;
    int value;
    int linenum;
    int use_count;
    SymbolDefinition* next;
    };

class NamespaceNest
{
    struct NamespaceFrame
    {
        int num_locals;
        int begin_linenum;
        int level;
        SymbolDefinition* defs;
        NamespaceFrame* next;
        };

    NamespaceFrame* pTop;
    NamespaceFrame* pDeleted;

    JSC_Compiler* jsc; // context

public:

    void Create( JSC_Compiler* ctx );

    void pushFrame( int linenum );
    void popFrame( void );

    int allocLocal( void );
    SymbolDefinition* defineSymbol( String* name, int scope, int value, int linenum );
    SymbolDefinition* lookupSymbol( String* name );
    };

///////////////////////////////////////////////////////////////////////////////
// AsmCode

class ASM_statement
{
public:
    ASM_statement* next;
    ASM_statement* prev;
    long offset;
    short opcode;
    short arg_type;
    int linenum;

#ifdef ALLOW_HEAVY_OPTIMIZATION
    unsigned long live_args;
    unsigned long live_locals;
    bool live_used;
#endif

    virtual void print( FILE* outf ) = 0;
    };

class AsmCode
{
public:
    ASM_statement* head;
    ASM_statement* tail;
    ASM_statement* tail_prev;

    int label_count;

    AsmCode( void )
    {
        head = NULL;
        tail = NULL;
        tail_prev = NULL;

        label_count = 1;
        }

    void Emit( ASM_statement* p )
    {
        p->next = NULL;
        p->prev = tail;

        if ( tail != NULL )
        {
            tail_prev = tail;
            tail->next = p;
            }
        else
        {
            head = p;
            }

        tail = p;
        }
    };

class ASM_symbolDef : public ASM_statement
{
public:
    String* value;

    ASM_symbolDef( String* arg_value, int arg_linenum )
    {
        opcode = JS_OPCODE_SYMBOL;
        arg_type = JS_OPARG_none;
        linenum = arg_linenum;
        value = arg_value;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "\n%s:\n", (const char*)*value );
        }
    };

class ASM_labelDef : public ASM_statement
{
public:
    int value;
    bool referenced;

    ASM_labelDef( int arg_value )
    {
        linenum = 0;
        opcode = JS_OPCODE_LABEL;
        arg_type = JS_OPARG_none;
        value = arg_value;
        referenced = false;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "L_%d:\n", value );
        }
    };

class ASM_voidArg : public ASM_statement
{
public:
    ASM_voidArg( int arg_opcode, int arg_linenum )
    {
        opcode = arg_opcode;
        arg_type = JS_OPARG_none;
        linenum = arg_linenum;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "\t%s\n", OpcodeDesc[ opcode ].desc );
        }
    };

class ASM_int32Arg : public ASM_statement
{
public:
    int value;

    ASM_int32Arg( int arg_opcode, long arg_value, int arg_linenum )
    {
        opcode = arg_opcode;
        arg_type = JS_OPARG_INT32;
        linenum = arg_linenum;
        value = arg_value;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "\t%-16s%d\n", OpcodeDesc[ opcode ].desc, value );
        }
    };

class ASM_constArg : public ASM_statement
{
public:
    int type;

    union
    {
        long id; // used in ByteCode generation phase
        long vinteger;
        double vfloat;
        String* vstring;
        };

    ASM_constArg( long arg_value, int arg_linenum )
    {
        opcode = JS_OPCODE_CONST;
        arg_type = JS_OPARG_CONST;
        linenum = arg_linenum;
        type = JS_INTEGER;
        vinteger = arg_value;
        }

    ASM_constArg( double arg_value, int arg_linenum )
    {
        opcode = JS_OPCODE_CONST;
        arg_type = JS_OPARG_CONST;
        linenum = arg_linenum;
        type = JS_FLOAT;
        vfloat = arg_value;
        }

    ASM_constArg( String* arg_value, int arg_linenum )
    {
        opcode = JS_OPCODE_CONST;
        arg_type = JS_OPARG_CONST;
        linenum = arg_linenum;
        type = JS_STRING;
        vstring = arg_value;
        }

    void print( FILE* outf )
    {
        if ( type == JS_INTEGER )
        {
            fprintf( outf, "\t%-16s%ld\n", "const", vinteger );
            }
        else if ( type == JS_FLOAT )
        {
            fprintf( outf, "\t%-16s%lf\n", "const", vfloat );
            }
        else if ( type == JS_STRING )
        {
            fprintf( outf, "\t%-16s\"", "const" );

            if ( vstring )
            {
                vstring->length ();

                for ( const char* chp = *vstring; *chp; chp++ )
                {
                    if ( *chp == '\\' ) fprintf( outf, "\\\\" );
                    else if ( *chp == '\n' ) fprintf( outf, "\\n" );
                    else if ( *chp == '\r' ) fprintf( outf, "\\r" );
                    else if ( *chp == '\t' ) fprintf( outf, "\\t" );
                    else if ( *chp == '\f' ) fprintf( outf, "\\f" );
                    else if ( *chp == '\"' ) fprintf( outf, "\\\"" );
                    else fprintf( outf, "%c", *chp );
                    }
                }

            fprintf( outf, "\"\n" );
            }
        }
    };

class ASM_symbolArg : public ASM_statement
{
public:
    union
    {
        long id; // used in ByteCode generation phase
        String* name;
        };

    ASM_symbolArg( int arg_opcode, String* arg_name, int arg_linenum )
    {
        opcode = arg_opcode;
        arg_type = JS_OPARG_SYMBOL;
        linenum = arg_linenum;
        name = arg_name;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "\t%-16s%s\n", OpcodeDesc[ opcode ].desc, (const char*)*name );
        }
    };

class ASM_labelArg : public ASM_statement
{
public:
    ASM_labelDef* label;

    ASM_labelArg( int arg_opcode, ASM_labelDef* arg_label, int arg_linenum )
    {
        opcode = arg_opcode;
        arg_type = JS_OPARG_LABEL;
        linenum = arg_linenum;
        label = arg_label;
        }

    void print( FILE* outf )
    {
        fprintf( outf, "\t%-16sL_%d\n", OpcodeDesc[ opcode ].desc, label->value );
        }
    };

inline long value_int32( ASM_statement* stmt )
{
    //assert( stmt->arg_type == JS_OPARG_INT32 );
    return ( (ASM_int32Arg*)stmt )->value;
    }

inline ASM_labelDef* value_label( ASM_statement* stmt )
{
    //assert( stmt->arg_type == JS_OPARG_LABEL );
    return ( (ASM_labelArg*)stmt )->label;
    }

///////////////////////////////////////////////////////////////////////////////
// JSC_Compiler

class JSC_Compiler
{
public:
    JSC_CompilerOptions options;

    void trace( const char* fmt... );
    void error( int ln, const char* fmt... );
    void warning( int ln, const char* fmt... );

    JSC_Compiler( void );
    ~JSC_Compiler( void );

    bool CompileFile( const char* js_filename, const char* bc_filename = NULL, const char* asm_filename = NULL );
    bool CompileString( const char* js_string, const char* bc_filename = NULL, const char* asm_filename = NULL );

    // Heap management
    //

    struct HeapBlock
    {
        HeapBlock* next;
        size_t alloc_size;
        size_t free_size;
        char* free_area;
        };

    HeapBlock* pTop;

    void* Alloc( size_t size )
    {
        if ( pTop == NULL || size > pTop->free_size )
        {
            size_t alloc_size = size > 10240 ? size : 10240;
            HeapBlock* b = (HeapBlock*)malloc( alloc_size + sizeof( HeapBlock ) );
            b->alloc_size = alloc_size;
            b->free_size = alloc_size;
            b->free_area = (char*)&b[ 1 ];
            b->next = pTop;
            pTop = b;

            memset( b->free_area, 0, alloc_size );
            }

        pTop->free_size -= size;
        void* p = pTop->free_area;
        pTop->free_area += size;

        return p;
        }

    // Lexical analysis
    //
    LexerIStream stream;
    Token cur_token;        // current token
    Token next_token;       // first lookahead token
    Token next_token2;      // second lookahead token
    int num_tokens;
    bool jspCode;           // in JSPages: true if inside <%-%> otherwise false

    void dumpLexerStatus ();
    int getToken( void );

    // Lexical analysis helpers
    //
    void eofInConstant( int possible_start, const char* what );
    bool isIdentifierLetter( int ch );
    bool isOctalDigit( int ch );
    bool isDecimalDigit( int ch );
    bool isHexDigit( int ch );
    bool isWhiteSpace( int ch );
    int hexToDec( int ch );
    int readBackslashEscape( int possible_start, const char* what );
    String* readString( int ender );
    void readToken( Token& token );

    // Parser
    //
    int anonymous_foo_count;
    String* nested_foo_names[ 100 ]; // stack
    int nested_foo_count;
    ListOf<FunctionDeclaration> functions;
    ListOf<Statement> global_stmts;

    void parseProgram( void );
    void parseJSPages( void );

    // Parser helpers
    //
    int num_missing_semicolons;
    void consumeSemicolon( void );

    void parseSourceElement( void );

    // Statements parser helpers
    //
    Statement* parseStatement( void );
    FunctionDeclaration* parseFunctionDeclaration( void );
    BlockStatement* parseBlock( void );
    Statement* parseFunctionStatement( void );
    Statement* parseVariableStatement( void );
    Statement* parseIfStatement( void );
    Statement* parseDoWhileStatement( void );
    Statement* parseWhileStatement( void );
    Statement* parseForStatement( void );
    Statement* parseSwitchStatement( void );
    Statement* parseTryStatement( void );
    Statement* parseContinueStatement( void );
    Statement* parseBreakStatement( void );
    Statement* parseReturnStatement( void );
    Statement* parseThrowStatement( void );
    Statement* parseEmptyStatement( void );
    Statement* parseWithStatement( void );

    // Expressions parser helpers
    //
    Expression* parseExpression( void );
    Expression* parseAssignmentExpression( void );
    Expression* parseConditionalExpression( void );
    Expression* parseLogicalORExpression( void );
    Expression* parseLogicalANDExpression( void );
    Expression* parseBitwiseORExpression( void );
    Expression* parseBitwiseXORExpression( void );
    Expression* parseBitwiseANDExpression( void );
    Expression* parseEqualityExpression( void );
    Expression* parseRelationalExpression( void );
    Expression* parseShiftExpression( void );
    Expression* parseAdditiveExpression( void );
    Expression* parseMultiplicativeExpression( void );
    Expression* parseUnaryExpression( void );
    Expression* parsePostfixExpression( void );
    Expression* parseLeftHandSideExpression( void );
    Expression* parseMemberExpression( void );
    Expression* parsePrimaryExpression( void );
    void parseArguments( ListOf<ArgExpression>& args );
    Expression* constant_folding( Expression* expr );

    // Code Generation
    //
    ContBreakNest cont_break;
    NamespaceNest ns;
    AsmCode asm_code;

    void asmGenerate( void );
    void asmOptimize( void );
    void asmOptimizeHeavy( void );
    void asmWriteASM( const char* asm_filename );
    void asmWriteBC( const char* bc_filename );

    // Code Generation helpers
    //
    ASM_labelDef* ASM_defLabel( void ); // NOTE: statement must be emitted manually
    void ASM_emitLabel( ASM_labelDef* arg_label );
    void ASM_defSymbol( String* arg_name, int arg_linenum );
    void ASM_nop( int arg_linenum );
    void ASM_dup( int arg_linenum );
    void ASM_pop( int arg_linenum );
    void ASM_pop_n( int arg_val, int arg_linenum );
    void ASM_swap( int arg_linenum );
    void ASM_roll( int arg_val, int arg_linenum );
    void ASM_const( long arg_value, int arg_linenum );
    void ASM_const( double arg_value, int arg_linenum );
    void ASM_const( String* arg_value, int arg_linenum );
    void ASM_const_null( int arg_linenum );
    void ASM_const_true( int arg_linenum );
    void ASM_const_false( int arg_linenum );
    void ASM_const_undefined( int arg_linenum );
    void ASM_const_i0( int arg_linenum );
    void ASM_const_i1( int arg_linenum );
    void ASM_const_i2( int arg_linenum );
    void ASM_const_i3( int arg_linenum );
    void ASM_const_i( long arg_val, int arg_linenum );
    void ASM_locals( int arg_value, int arg_linenum );
    void ASM_apop( int arg_val, int arg_linenum );
    void ASM_min_args( int arg_value, int arg_linenum );
    void ASM_load_arg( int arg_value, int arg_linenum );
    void ASM_store_arg( int arg_value, int arg_linenum );
    void ASM_load_nth_arg( int arg_linenum );
    void ASM_load_global( String* arg_name, int arg_linenum );
    void ASM_load_global_w( String* arg_name, int arg_linenum );
    void ASM_store_global( String* arg_name, int arg_linenum );
    void ASM_load_local( int arg_value, int arg_linenum );
    void ASM_store_local( int arg_value, int arg_linenum );
    void ASM_load_property( String* arg_name, int arg_linenum );
    void ASM_store_property( String* arg_name, int arg_linenum );
    void ASM_delete_property( String* arg_name, int arg_linenum );
    void ASM_load_array( int arg_linenum );
    void ASM_store_array( int arg_linenum );
    void ASM_delete_array( int arg_linenum );
    void ASM_nth( int arg_linenum );
    void ASM_cmp_eq( int arg_linenum );
    void ASM_cmp_ne( int arg_linenum );
    void ASM_cmp_lt( int arg_linenum );
    void ASM_cmp_gt( int arg_linenum );
    void ASM_cmp_le( int arg_linenum );
    void ASM_cmp_ge( int arg_linenum );
    void ASM_cmp_seq( int arg_linenum );
    void ASM_cmp_sne( int arg_linenum );
    void ASM_add_1_i( int arg_linenum );
    void ASM_add_2_i( int arg_linenum );
    void ASM_sub( int arg_linenum );
    void ASM_add( int arg_linenum );
    void ASM_mul( int arg_linenum );
    void ASM_div( int arg_linenum );
    void ASM_mod( int arg_linenum );
    void ASM_neg( int arg_linenum );
    void ASM_not( int arg_linenum );
    void ASM_and( int arg_linenum );
    void ASM_or( int arg_linenum );
    void ASM_xor( int arg_linenum );
    void ASM_shift_left( int arg_linenum );
    void ASM_shift_right( int arg_linenum );
    void ASM_shift_rright( int arg_linenum );
    void ASM_halt( int arg_linenum );
    void ASM_done( int arg_linenum );
    void ASM_iffalse( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_iftrue( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_iffalse_b( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_iftrue_b( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_call_method( String* arg_name, int arg_linenum );
    void ASM_jmp( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_jsr( int arg_linenum );
    void ASM_jsr_w( String* arg_name, int arg_linenum );
    void ASM_return( int arg_linenum );
    void ASM_typeof( int arg_linenum );
    void ASM_new( int arg_linenum );
    void ASM_with_push( int arg_linenum );
    void ASM_with_pop( int arg_value, int arg_linenum );
    void ASM_try_push( ASM_labelDef* arg_label, int arg_linenum );
    void ASM_try_pop( int arg_value, int arg_linenum );
    void ASM_throw( int arg_linenum );

    void ASM_expr_lvalue_load( Expression* expr );
    void ASM_expr_lvalue_store( Expression* expr );
    };

inline void* operator new( size_t size, JSC_Compiler* ctx )
{
    return ctx->Alloc( size );
    }

inline void operator delete( void* ptr, JSC_Compiler* ctx )
{
    assert( 0 );
    }

inline String* strDup( JSC_Compiler* ctx, String& str )
{
    String* p = new(ctx) String( new(ctx) char[ str.len + 1 ], str.len );
    memcpy( p->data, str.data, str.len + 1 );
    return p;
    }

#endif // _JSC_H_INCLUDED