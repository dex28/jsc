#ifndef _ODBC_H_INCLUDED
#define _ODBC_H_INCLUDED

#include "JS.h"
#include <windows.h>
#include <sqlext.h>

struct JSSQL_ENV;
struct JSSQL_DBC;
struct JSSQL_ROWSET;

struct JSSQL_ENV
{
    JSVirtualMachine* vm;

    SQLRETURN retcode;
    SQLHENV henv; // environment handle
    bool fEnvOwner; // do we own henv ?

    JSSQL_DBC* pFirstChild; // list of belonging root-DBCs

    JSSQL_ENV( JSVirtualMachine* vm );
    JSSQL_ENV( JSVirtualMachine* vm, SQLHENV hStatic );
    ~JSSQL_ENV( void );

    void Perror( char* where );

    void AddChild( JSSQL_DBC* p );
    void RemoveChild( JSSQL_DBC* p );

    void Free( void );
    };

struct JSSQL_DBC
{
    JSVirtualMachine* vm;

    JSVariant env; // referenced environment object (JS_BUILTIN or JS_UNDEFINED)
    JSVariant dbc; // referenced connection object (JS_BUILTIN or JS_UNDEFINED)

    SQLRETURN retcode;
    SQLHENV henv; // environment handle
    SQLHDBC hdbc; // database connection handle
    SQLHSTMT hstmt; // statement handle

    JSVariant crsr_name; // cursor name that should be used with next select statement
    JSVariant last_stmt; // refernced statement source (JS_STRING or JS_UNDEFINED)

    JSSQL_ROWSET* rowset; // currently appointed row set

    int nNumParams;
    JSVariant params[ 16 ]; // binded parameters

    JSSQL_DBC* pFirstChild; // list of belonging leaf-DBCs

    JSSQL_DBC* pPrev;
    JSSQL_DBC* pNext;

    JSSQL_DBC( SQLHENV henv, JSVirtualMachine* vm );
    JSSQL_DBC( JSVariant& builtin );
    ~JSSQL_DBC( void );

    void AddChild( JSSQL_DBC* p );
    void RemoveChild( JSSQL_DBC* p );
    void Free( void );

    void Perror( char* where );
    void Pwarning( char* where );
    void DumpInfo( void );

    // connection stuff
    //
    void Connect( JSVariant& dsn, JSVariant& uid, JSVariant& pwd );
    void Disconnect( void );
    void Commit( void );
    void Rollback( void );
    void SetAutoCommit( int setOn );
    int GetAutoCommit( void );

    // statement stuff
    //
    void FreeStatement( void );
    void CloseCursor( void );
    JSVariant ExecSQL( JSVariant& stmt, int argc, JSVariant argv [] );
    int Fetch( void );
    };

struct JSSQL_ROWSET
{
    JSSQL_DBC* stmt; // cached statement context
    JSVariant dbc; // referenced connection/statement object

    SQLSMALLINT nNumCols;

    JSSymbol* prop;
    SQLSMALLINT* wCType;
    SQLINTEGER* nColLen;
    SQLINTEGER* nOffset;
    uint8* DataPtr;

    JSSQL_ROWSET( JSSQL_DBC* parenta, JSVariant& parentb );
    ~JSSQL_ROWSET( void );

    void Free( void );

    int Fetch( void );
    };

#endif // _ODBC_H_INCLUDED