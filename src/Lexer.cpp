
#include "JSC.h"

// TODO: eliminate ungetCh -- use peekCh instead
// TODO: eliminate peekCh -- make readCh to cache lookahead bytes

void
dumpToken( Token& token )
{
    static const char* szToken[] =
    {
        "EOF",      "Integer",    "Float",  "String", "Identifier",

        "BREAK",    "CONTINUE",   "DELETE", "ELSE",   "FOR",  "FUNCTION",
        "IF",       "IN",         "NEW",    "RETURN", "THIS", "TYPEOF", "VAR",
        "VOID",     "WHILE",      "WITH",

        "CASE",     "CATCH",      "CLASS",  "CONST",  "DEBUGGER",
        "DEFAULT",  "DO",         "ENUM",   "EXPORT", "EXTENDS", "FINALLY",
        "IMPORT",   "SUPER",      "SWITCH", "THROW",  "TRY",

        "NULL", "TRUE", "FALSE",

        "==", "!=", "<=", ">=", "&&", "||",
        "++", "--",
        "*=", "/=", "%=", "+=", "-=", "&=", "^=", "|=", "<<=",
        "<<", ">>", ">>>", ">>=", ">>>=", "===", "!==",

        "%>", "<%", "<%=", "HtmlCode"
        };

    if ( token.id < 0 || token.id >= JSC_tLASTONE  )
    {
        printf( "<!?%d>", token.id );
        }
    else if ( token.id < JSC_tEOF )
    {
        printf( "%c", token.id );
        }
    else if ( token.id == JSC_tINTEGER )
    {
        printf( "Integer(%ld)\n", token.vinteger );
        }
    else if ( token.id == JSC_tFLOAT )
    {
        printf( "Float(%lf)\n", token.vfloat );
        }
    else if ( token.id == JSC_tSTRING || token.id == JSC_tHTMLSTRING )
    {
        printf( "%s(", token.id == JSC_tSTRING ? "String" : "Html" );

        token.vstring->length ();

        for ( const char* chp = *token.vstring; *chp; chp++ )
        {
            if ( *chp == '\\' ) printf( "\\\\" );
            else if ( *chp == '\n' ) printf( "\\n" );
            else if ( *chp == '\r' ) printf( "\\r" );
            else if ( *chp == '\t' ) printf( "\\t" );
            else if ( *chp == '\f' ) printf( "\\f" );
            else if ( *chp == '\"' ) printf( "\\\"" );
            else printf( "%c", *chp );
            }

        printf( ")" );
        }
    else if ( token.id == JSC_tIDENTIFIER )
    {
        printf( "Identifier(%s)", token.vstring->data );
        }
    else
    {
        printf( "%s", szToken[ token.id - JSC_tEOF ] );
        }
    }

void
JSC_Compiler:: dumpLexerStatus( void )
{
    printf( "Lexer status: " );

    dumpToken( cur_token );

    printf( " " );

    dumpToken( next_token );

    printf( " " );

    dumpToken( next_token2 );

    printf( "\n" );
    }

int
JSC_Compiler:: getToken( void )
{
    num_tokens ++;

    cur_token = next_token;
    next_token = next_token2;

    readToken( next_token2 );

    return cur_token.id;
    }

void
JSC_Compiler:: readToken( Token& token )
{
    token.id = -1; // make token invalid

    if ( options.js_pages && ! jspCode ) // reading JSP html
    {
        int ch = stream.readCh ();

        if ( ch == -1 ) // EOF reached
        {
            token.id = JSC_tEOF;
            return;
            }

        int ch2 = stream.peekCh ();

        token.linenum = stream.linenum;

        char buffer[ 102400 ];
        String str( buffer, 0 );

        for ( ;; )
        {
            if ( ch == '<' && ch2 == '%' )
            {
                stream.ungetCh( ch );
                jspCode = true;
                token.id = JSC_tHTMLSTRING;
                token.vstring = strDup( this, str );
                return;
                }
            else if ( ch == -1 ) // EOF reached
            {
                token.id = JSC_tHTMLSTRING;
                token.vstring = strDup( this, str );
                return;
                }

            str += ch;

            ch = stream.readCh ();
            ch2 = stream.peekCh ();
            }
        }

    for ( ;; )
    {
        int ch = stream.readCh ();

        if ( ch == -1 ) // EOF reached
        {
            token.id = JSC_tEOF;
            return;
            }

        if ( isWhiteSpace( ch ) )
        {
            continue;
            }

        token.linenum = stream.linenum;

        int ch2 = stream.peekCh ();

        if ( ch == '\\' && ch2 == '\n' )
        {
            // TODO: what if \ followed by CRLF
            // if ch,ch2,ch3 == "\\\r\n" or ch,ch2 == "\\\n"
            stream.readCh ();
            continue;
            }

        if ( ch == '/' && ch2 == '*' )
        {
            // Multi line comment
            //
            stream.readCh ();

            for ( ;; )
            {
                ch = stream.readCh ();
                ch2 = stream.peekCh ();

                if ( ch == -1 || ( ch == '*' && ch2 == '/' ) )
                {
                    // Consume the peeked '/' character
                    //
                    stream.readCh ();
                    break;
                    }
                }

            continue;
            }

        if ( ( ch == '/' && ch2 == '/' ) || ( ch == '#' && ch2 == '!' ) )
        {
            // Single line comment
            //
            do ch = stream.readCh ();
                while( ch != -1 && ch != '\n' );

            continue;
            }

        if ( ch == '<' && ch2 == '%' )
        {
            if ( ! options.js_pages )
            {
                error( stream.linenum, "JS Pages extension not allowed in plain JS" );
                }

            stream.readCh ();
            if ( stream.peekCh () == '=' )
            {
                stream.readCh ();
                token.id = JSC_tBEGJSPEXPR;
                return;
                }

            token.id = JSC_tBEGJSPCODE;

            return;
            }

        if ( ch == '%' && ch2 == '>' )
        {
            stream.readCh ();

            if ( ! options.js_pages )
            {
                error( stream.linenum, "JS Pages extension not allowed in plain JS" );
                }

            jspCode = false;
            token.id = JSC_tENDJSP;
            return;
            }

        if ( ch == '"' || ch == '\'' )
        {
            // String constant
            //
            token.id = JSC_tSTRING;
            token.vstring = readString( ch );
            return;
            }

        // Literals
        //
        if ( ch == '=' && ch2 == '=' )
        {
            stream.readCh ();

            if ( stream.peekCh () == '=' )
            {
                stream.readCh ();
                token.id = JSC_tSEQUAL;
                }
            else
            {
                token.id = JSC_tEQUAL;
                }
            return;
            }

        if ( ch == '!' && ch2 == '=' )
        {
            stream.readCh ();
            if ( stream.peekCh () == '=' )
            {
                stream.readCh ();
                token.id = JSC_tSNEQUAL;
                return;
                }

            token.id = JSC_tNEQUAL;
            return;
            }

        if ( ch == '<' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tLE;
            return;
            }

        if ( ch == '>' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tGE;
            return;
            }

        if ( ch == '&' && ch2 == '&' )
        {
            stream.readCh ();
            token.id = JSC_tAND;
            return;
            }

        if ( ch == '|' && ch2 == '|' )
        {
            stream.readCh ();
            token.id = JSC_tOR;
            return;
            }

        if ( ch == '+' && ch2 == '+' )
        {
            stream.readCh ();
            token.id = JSC_tPLUSPLUS;
            return;
            }

        if ( ch == '-' && ch2 == '-' )
        {
            stream.readCh ();
            token.id = JSC_tMINUSMINUS;
            return;
            }

        if ( ch == '*' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tMULA;
            return;
            }

        if ( ch == '/' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tDIVA;
            return;
            }

        if ( ch == '%' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tMODA;
            return;
            }

        if ( ch == '+' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tADDA;
            return;
            }

        if ( ch == '-' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tSUBA;
            return;
            }

        if ( ch == '&' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tANDA;
            return;
            }

        if ( ch == '^' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tXORA;
            return;
            }

        if ( ch == '|' && ch2 == '=' )
        {
            stream.readCh ();
            token.id = JSC_tORA;
            return;
            }

        if ( ch == '<' && ch2 == '<' )
        {
            stream.readCh ();
            if ( stream.peekCh () == '=' )
            {
                stream.readCh ();
                token.id = JSC_tLSIA;
                }
            else
            {
                token.id = JSC_tLSHIFT;
                }
            return;
            }

        if ( ch == '>' && ch2 == '>' )
        {
            stream.readCh ();
            ch2 = stream.peekCh ();
            if ( ch2 == '=' )
            {
                stream.readCh ();
                token.id = JSC_tRSIA;
                }
            else if ( ch2 == '>' )
            {
                stream.readCh ();
                if (stream.peekCh () == '=' )
                {
                    stream.readCh ();
                    token.id = JSC_tRRSA;
                    }
                else
                {
                    token.id = JSC_tRRSHIFT;
                    }
                }
            else
            {
                token.id = JSC_tRSHIFT;
                }
            return;
            }

        // Identifiers and keywords
        //
        if ( isIdentifierLetter ( ch ) )
        {
            // An identifier
            //
            char buffer[ 256 ];
            String id( buffer, 0 );

            for ( ;; )
            {
                id += ch;

                if ( ch2 == -1
                    || ! ( isIdentifierLetter( ch2 ) || isDecimalDigit( ch2 ) )
                    )
                {
                    break;
                    }

                ch = stream.readCh ();
                ch2 = stream.peekCh ();
                }

            // Keywords
            //
            if ( id == "break" )
            {
                token.id = JSC_tBREAK;
                return;
                }
            if ( id == "continue" )
            {
                token.id = JSC_tCONTINUE;
                return;
                }
            if ( id == "delete" )
            {
                token.id = JSC_tDELETE;
                return;
                }
            if ( id == "else" )
            {
                token.id = JSC_tELSE;
                return;
                }
            if ( id == "for" )
            {
                token.id = JSC_tFOR;
                return;
                }
            if ( id == "function" )
            {
                token.id = JSC_tFUNCTION;
                return;
                }
            if ( id == "if" )
            {
                token.id = JSC_tIF;
                return;
                }
            if ( id == "in" )
            {
                token.id = JSC_tIN;
                return;
                }
            if ( id == "new" )
            {
                token.id = JSC_tNEW;
                return;
                }
            if ( id == "return" )
            {
                token.id = JSC_tRETURN;
                return;
                }
            if ( id == "this" )
            {
                token.id = JSC_tTHIS;
                return;
                }
            if ( id == "typeof" )
            {
                token.id = JSC_tTYPEOF;
                return;
                }
            if ( id == "var" )
            {
                token.id = JSC_tVAR;
                return;
                }
            if ( id == "void" )
            {
                token.id = JSC_tVOID;
                return;
                }
            if ( id == "while" )
            {
                token.id = JSC_tWHILE;
                return;
                }
            if ( id == "with" )
            {
                token.id = JSC_tWITH;
                return;
                }
            //
            // Future reserved keywords (some of these is already in use
            // in this implementation).
            //
            if ( id == "case" )
            {
                token.id = JSC_tCASE;
                return;
                }
            if ( id == "catch" )
            {
                token.id = JSC_tCATCH;
                return;
                }
            if ( id == "class" )
            {
                token.id = JSC_tCLASS;
                return;
                }
            if ( id == "const" )
            {
                token.id = JSC_tCONST;
                return;
                }
            if ( id == "debugger" )
            {
                token.id = JSC_tDEBUGGER;
                return;
                }
            if ( id == "default" )
            {
                token.id = JSC_tDEFAULT;
                return;
                }
            if ( id == "do" )
            {
                token.id = JSC_tDO;
                return;
                }
            if ( id == "enum" )
            {
                token.id = JSC_tENUM;
                return;
                }
            if ( id == "export" )
            {
                token.id = JSC_tEXPORT;
                return;
                }
            if ( id == "extends" )
            {
                token.id = JSC_tEXTENDS;
                return;
                }
            if ( id == "finally" )
            {
                token.id = JSC_tFINALLY;
                return;
                }
            if ( id == "import" )
            {
                token.id = JSC_tIMPORT;
                return;
                }
            if ( id == "super" )
            {
                token.id = JSC_tSUPER;
                return;
                }
            if ( id == "switch" )
            {
                token.id = JSC_tSWITCH;
                return;
                }
            if ( id == "throw" )
            {
                token.id = JSC_tTHROW;
                return;
                }
            if ( id == "try" )
            {
                token.id = JSC_tTRY;
                return;
                }
            //
            // Null and boolean literals
            //
            if ( id == "null" )
            {
                token.id = JSC_tNULL;
                return;
                }
            if ( id == "true" )
            {
                token.id = JSC_tTRUE;
                return;
                }
            if ( id == "false" )
            {
                token.id = JSC_tFALSE;
                return;
                }

            // It really is an identifier
            //
            token.id = JSC_tIDENTIFIER;
            token.vstring = strDup( this, id );

            return;
            }
        //
        // Character constants
        //
        if ( ch == '#' && ch2 == '\'' )
        {
            // Skip the starting '\'' and read more. */
            stream.readCh ();

            ch = stream.readCh ();
            if ( ch == '\\' )
            {
                token.id = JSC_tINTEGER;
                token.vinteger = readBackslashEscape( 0, "character" );

                if ( stream.readCh () != '\'' )
                    error( stream.linenum, "malformed character constant" );
                }
            else if (stream.peekCh () == '\'' )
            {
                stream.readCh ();
                token.id = JSC_tINTEGER;
                token.vinteger = ch;
                }
            else
            {
                error( stream.linenum, "malformed character constant" );
                }

            return;
            }
        //
        // Octal and hex numbers
        //
        if ( ch == '0' && ch2 != '.' && ch2 != 'e' && ch2 != 'E' )
        {
            token.id = JSC_tINTEGER;
            token.vinteger = 0;

            ch = stream.readCh ();

            if ( ch == 'x' || ch == 'X' )
            {
                ch2 = stream.peekCh ();

                while ( isHexDigit ( ch2 ) )
                {
                    token.vinteger *= 16;
                    token.vinteger += hexToDec ( ch2 );
                    ch = stream.readCh ();
                    ch2 = stream.peekCh ();
                    }
                }
            else
            {
                while ( isOctalDigit ( ch ) )
                {
                    token.vinteger *= 8;
                    token.vinteger += ch - '0';
                    ch = stream.readCh ();
                    }

                stream.ungetCh ( ch);
                }

            return;
            }
        //
        // Decimal numbers
        //
        if ( isDecimalDigit( ch )
            || ( ch == '.'  && isDecimalDigit( ch2 ) )
            )
        {
            bool is_float = false;

            char buffer[ 256 ];
            String buf( buffer, 0 );
            buf += ch;

            bool accept_dot = true;

            if ( ch == '.' )
            {
                //
                // We started with '.' and we know that the next character
                // is a decimal digit (we peeked it).
                //
                is_float = true;

                ch = stream.readCh ();
                while (isDecimalDigit ( ch) )
                {
                    buf += ch;
                    ch = stream.readCh ();
                    }
                accept_dot = false;
                }
            else
            {
                // We did start with a decimal digit
                //
                ch = stream.readCh ();
                while (isDecimalDigit ( ch) )
                {
                    buf += ch;
                    ch = stream.readCh ();
                    }
                }

            if ((accept_dot && ch == '.' ) || ch == 'e' || ch == 'E' )
            {
                is_float = true;

                if ( ch == '.' )
                {
                    buf += ch;
                    ch = stream.readCh ();
                    while (isDecimalDigit ( ch) )
                    {
                        buf += ch;
                        ch = stream.readCh ();
                        }
                    }

                if ( ch == 'e' || ch == 'E' )
                {
                    buf += ch;
                    ch = stream.readCh ();

                    if ( ch == '+' || ch == '-' )
                    {
                        buf += ch;
                        ch = stream.readCh ();
                        }

                    if (!isDecimalDigit ( ch) )
                        error( stream.linenum, "malformed exponent part in a decimal literal" );

                    while (isDecimalDigit ( ch) )
                    {
                        buf += ch;
                        ch = stream.readCh ();
                        }
                    }
                }

            // Finally, we put the last character pack to the
            //
            stream.ungetCh( ch );

            if ( is_float )
            {
                token.id = JSC_tFLOAT;
                token.vfloat = double( buf );
                return;
                }

            token.id = JSC_tINTEGER;
            token.vinteger = long( buf );
            return;
            }

        // Just return the character as-is
        //
        token.id = ch;
        return;
        }
    }

void JSC_Compiler:: eofInConstant( int possible_start, const char* what )
{
    error( stream.linenum, "unterminated %s constant", what );
    }

bool JSC_Compiler:: isIdentifierLetter( int ch )
{
    return ( 'a' <= ch && ch <= 'z' )
        || ( 'A' <= ch && ch <= 'Z' )
        || ch == '$'
        || ch == '_';
    }

bool JSC_Compiler:: isOctalDigit( int ch )
{
    return '0' <= ch && ch <= '7';
    }

bool JSC_Compiler:: isDecimalDigit( int ch )
{
    return '0' <= ch && ch <= '9';
    }

bool JSC_Compiler:: isHexDigit( int ch )
{
    return ( '0' <= ch && ch <= '9' )
        || ( 'a' <= ch && ch <= 'f' )
        || ( 'A' <= ch && ch <= 'F' );
    }

bool JSC_Compiler:: isWhiteSpace( int ch )
{
    return ch == ' ' 
        || ch == '\t' || ch == '\v' || ch == '\r'
        || ch == '\f' || ch == '\n';
    }

int JSC_Compiler:: hexToDec( int ch )
{
    return ( '0' <= ch && ch <= '9' ) ? ch - '0' 
        : ( 'a' <= ch && ch <= 'f' ) ? 10 + ch - 'a' 
        : 10 + ch - 'A';
    }

int JSC_Compiler:: readBackslashEscape( int possible_start, const char* what )
{
    int ch = stream.readCh ();

    if ( ch == 'n' )
    {
        ch = '\n';
        }
    else if ( ch == 't' )
    {
        ch = '\t';
        }
    else if ( ch == 'v' )
    {
        ch = '\v';
        }
    else if ( ch == 'b' )
    {
        ch = '\b';
        }
    else if ( ch == 'r' )
    {
        ch = '\r';
        }
    else if ( ch == 'f' )
    {
        ch = '\f';
        }
    else if ( ch == 'a' )
    {
        ch = '\a';
        }
    else if ( ch == '\\' )
    {
        ch = '\\';
        }
    else if ( ch == '?' )
    {
        ch = '?';
        }
    else if ( ch == '\'' )
    {
        ch = '\'';
        }
    else if ( ch == '"' )
    {
        ch = '"';
        }
    else if ( ch == 'x' )
    {
        // HexEscapeSequence
        //
        int c1 = stream.readCh ();
        int c2 = stream.readCh ();

        if ( c1 == -1 || c2 == -1 )
            eofInConstant( possible_start, what );

        if ( ! isHexDigit( c1 ) || !isHexDigit( c2 ) )
            error( stream.linenum, "\\x used with no following hex digits" );

        ch = ( hexToDec( c1 ) << 4 ) + hexToDec( c2 );
        }
    else if ( ch == 'u' )
    {
        // UnicodeEscapeSequence
        //
        int c1 = stream.readCh ();
        int c2 = stream.readCh ();
        int c3 = stream.readCh ();
        int c4 = stream.readCh ();

        if ( c1 == -1 || c2 == -1 || c3 == -1 || c4 == -1 )
            eofInConstant( possible_start, what );

        if ( ! isHexDigit (c1) || !isHexDigit (c2)
            || !isHexDigit (c3) || !isHexDigit (c4)
            )
            error( stream.linenum, "\\u used with no following hex digits" );

        ch = ( hexToDec( c1 ) << 12 )
            + ( hexToDec( c2 ) << 8 )
            + ( hexToDec( c3 ) << 4 )
            + hexToDec( c4 );
        }
    else if ( isOctalDigit ( ch ) )
    {
        int result = ch - '0';
        int i = 1;

        if ( ch == '0' ) // Allow three octal digits after '0'
            i = 0;

        ch = stream.readCh ();
        while ( i < 3 && isOctalDigit( ch ) )
        {
            result *= 8;
            result += ch - '0';
            ch = stream.readCh ();
            i++;
            }
        stream.ungetCh( ch );
        ch = result;
        }
    else
    {
        if ( ch == -1)
            error( stream.linenum, "unterminated %s", what );

        warning( stream.linenum, "unknown escape sequence '\\%c'", ch );
        }

    return ch;
    }

String* JSC_Compiler:: readString( int ender )
{
    char buffer[ 102400 ];
    String str( buffer, 0 );

    int possible_start_ln = stream.linenum;
    bool warned_line_terminator = false;

    bool done = false;

    while ( ! done )
    {
        int ch = stream.readCh ();
        if ( ch == '\n' )
        {
            if ( options.warn_strict_ecma && ! warned_line_terminator )
            {
                warning( stream.linenum, "ECMAScript don't allow line terminators in string constants" );
                warned_line_terminator = true;
                }
            }

        if ( ch == -1)
            eofInConstant (possible_start_ln, "string");
        else if ( ch == ender)
            done = true;
        else
        {
            if ( ch == '\\' )
            {
                if (stream.peekCh () == '\n' )
                {
                    // Backslash followed by a newline character.  Ignore
                    // them both.
                    //
                    stream.readCh ();
                    continue;
                    }

                ch = readBackslashEscape( possible_start_ln, "string" );
                }

            str += ch;
            }
        }

    return strDup( this, str );
    }
