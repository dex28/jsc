//
// ODBC class
//

#include "ODBC.h"

// Define a macro to increase the size of a buffer so it is a multiple of the alignment
// size. Thus, if a buffer starts on an alignment boundary, it will end just before the
// next alignment boundary.

#define ALIGNSIZE sizeof(double)

#define ALIGNBUF(Length) \
       ( (Length) % ALIGNSIZE ? (Length) + ALIGNSIZE - ( (Length) % ALIGNSIZE ) : (Length) )

void
GetDefaultCType
(
    SQLINTEGER SqlType,
    SQLSMALLINT& CType,
    SQLINTEGER& CLen
    )
{
    CType = SQL_C_CHAR;
    CLen = 240;
    return;

    switch( SqlType )
    {
        case SQL_SMALLINT:
            CType = SQL_C_USHORT;
            CLen = sizeof( SQLUSMALLINT );
            return;

        case SQL_INTEGER:
            CType = SQL_C_ULONG;
            CLen = sizeof( SQLUINTEGER );
            return;

        case SQL_FLOAT:
            CType = SQL_C_FLOAT;
            CLen = sizeof( SQLREAL );
            return;

        case SQL_DOUBLE:
            CType = SQL_C_DOUBLE;
            CLen = sizeof( SQLDOUBLE );
            return;

        case SQL_TYPE_DATE:
            CType = SQL_C_TYPE_DATE;
            CLen = sizeof( SQL_DATE_STRUCT );
            return;

        case SQL_TYPE_TIME:
            CType = SQL_C_TYPE_TIME;
            CLen = sizeof( SQL_TIME_STRUCT );
            return;

        case SQL_TYPE_TIMESTAMP:
            CType = SQL_C_TYPE_TIMESTAMP;
            CLen = sizeof( SQL_TIMESTAMP_STRUCT );
            return;
        }

    CType = SQL_C_CHAR;
    CLen = 240;
    }

JSSQL_ENV:: JSSQL_ENV
(
    JSVirtualMachine* arg_vm
    )
{
    // printf( "JSSQL_ENV:: Construct\n" );

    vm = arg_vm;
    pFirstChild = NULL;

    henv = SQL_NULL_HENV;
    fEnvOwner = true;

    retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv );

    if ( retcode != SQL_SUCCESS )
    {
        Perror( "JSSQL_ENV::Construct:SQLAllocHandle: " );
        }

    retcode = SQLSetEnvAttr(
        henv,                        // SQLHENV EnvironmentHandle
        SQL_ATTR_ODBC_VERSION,       // SQLINTEGER Attribute
        SQLPOINTER( SQL_OV_ODBC3 ),  // SQLPOINTER Value
        SQL_IS_INTEGER               // SQLINTEGER StringLength
        );

    if ( retcode != SQL_SUCCESS )
    {
        Perror( "JSSQL_ENV:: Construct: SetAttrODBCVersion: " );
        }
    }

JSSQL_ENV:: JSSQL_ENV
(
    JSVirtualMachine* arg_vm,
    SQLHENV hStatic
    )
{
    // printf( "JSSQL_ENV:: Construct\n" );

    vm = arg_vm;
    pFirstChild = NULL;

    henv = hStatic;
    fEnvOwner = false;
    }

JSSQL_ENV:: ~JSSQL_ENV
(
    void
    )
{
    // printf( "JSSQL_ENV:: Destruct\n" );

    Free ();
    }

void
JSSQL_ENV:: AddChild
(
    JSSQL_DBC* pe
    )
{
    pe->pNext = pFirstChild;
    pe->pPrev = NULL;

    if ( pFirstChild != NULL )
    {
        pFirstChild->pPrev = pe;
        pFirstChild = pe;
        }
    else
    {
        pFirstChild = pe;
        }
    }

void
JSSQL_ENV:: Free
(
    void
    )
{
    //printf( "JSSQL_ENV:: Free\n" );

    for ( JSSQL_DBC* pDBC = pFirstChild; pDBC; pDBC = pDBC->pNext )
    {
        pDBC->Free ();
        }

    pFirstChild = NULL;

    if ( fEnvOwner && henv != SQL_NULL_HENV )
    {
        printf( "JSSQL_ENV:: ENV handle is free\n" );

        retcode = SQLFreeHandle( SQL_HANDLE_ENV, henv );
        henv = SQL_NULL_HENV;
        }
    }

void
JSSQL_ENV:: Perror
(
    char* where
    )
{
    char* errorMsg = vm->error;
    int errorMsgLen = sizeof( vm->error ) - 1;

    {
        int len = sprintf( errorMsg, where );
        errorMsgLen -= len;
        errorMsg += len;
        }

    for ( int i = 1 ;; i++ )
    {
        SQLINTEGER NativeError;
        SQLCHAR SqlState[ 6 ];
        SQLSMALLINT MsgLen = errorMsgLen;

        SQLRETURN rc = SQLGetDiagRec(
               SQL_HANDLE_ENV,        // SQLSMALLINT  fHandleType
               henv,                  // SQLHANDLE    handle
               i,                     // SQLSMALLINT  iRecord
               SqlState,              // SQLTCHAR*    szSqlState
               &NativeError,          // SQLINTEGER*  pfNativeError
               (SQLCHAR*)errorMsg,    // SQLTCHAR*    szErrorMsg
               errorMsgLen,           // SQLSMALLINT  cbErrorMsgMax
               &MsgLen                // SQLSMALLINT* pcbErrorMsg
               );

        if ( rc == SQL_NO_DATA )
        {
            if ( i == 1 )
            {
                strcpy( errorMsg, "<unknown>" );
                }
            break;
            }

        errorMsg[ MsgLen ] = '\n';
        errorMsg[ MsgLen + 1 ] = 0;
        errorMsgLen -= MsgLen + 1;
        errorMsg += MsgLen + 1;
        }

    if ( retcode == SQL_SUCCESS_WITH_INFO )
    {
        fprintf( stderr, "%s", vm->error );
        return;
        }

    vm->RaiseError ();
    }

JSSQL_DBC:: JSSQL_DBC
(
    JSVariant& ctx
    )
{
    assert( ctx.type == JS_BUILTIN );

    vm = ctx.vbuiltin->vm;
    pFirstChild = NULL;
    pNext = NULL;
    pPrev = NULL;

    if ( ctx.vbuiltin->info == vm->Intern( "SQL$env" )->vbuiltin->info )
    {
        JSSQL_ENV* parent = (JSSQL_ENV*)ctx.vbuiltin->instance_context;

        env = ctx;
        dbc.type = JS_UNDEFINED;

        henv = parent->henv;
        hdbc = SQL_NULL_HDBC;

        hstmt = SQL_NULL_HSTMT;
        last_stmt.type = JS_UNDEFINED;
        crsr_name.type = JS_UNDEFINED;
        nNumParams = 0;

        retcode = SQLAllocHandle( SQL_HANDLE_DBC, henv, &hdbc );

        if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
        {
            Perror( "SQL.connect: " );
            }

        parent->AddChild( this );
        }
    else if ( ctx.vbuiltin->info == vm->Intern( "SQL" )->vbuiltin->info )
    {
        JSSQL_DBC* parent = (JSSQL_DBC*)ctx.vbuiltin->instance_context;

        env = parent->env;
        dbc = parent->dbc.type == JS_UNDEFINED ? ctx : parent->dbc;

        henv = parent->henv;
        hdbc = parent->hdbc;

        hstmt = SQL_NULL_HSTMT;
        last_stmt.type = JS_UNDEFINED;
        crsr_name.type = JS_UNDEFINED;
        nNumParams = 0;

        if ( parent->dbc.type == JS_UNDEFINED )
        {
            parent->AddChild( this );
            }
        else
        {
            ( ( JSSQL_DBC*)parent->dbc.vbuiltin->instance_context )->AddChild( this );
            }
        }
    else
    {
        vm->RaiseError( "new SQL(): illegal argument: expected SQL or SQL$env object" );
        }
    }

JSSQL_DBC:: ~JSSQL_DBC
(
    void
    )
{
    printf( "JSSQL_DBC:: Destruct\n" );

    Free ();
    }

void
JSSQL_DBC:: AddChild
(
    JSSQL_DBC* pe
    )
{
    pe->pNext = pFirstChild;
    pe->pPrev = NULL;

    if ( pFirstChild != NULL )
    {
        pFirstChild->pPrev = pe;
        pFirstChild = pe;
        }
    else
    {
        pFirstChild = pe;
        }
    }

void
JSSQL_DBC:: Free
(
    void
    )
{
    // printf( "JSSQL_DBC:: Free\n" );

    for ( JSSQL_DBC* pDBC = pFirstChild; pDBC; pDBC = pDBC->pNext )
    {
        pDBC->Free ();
        }

    pFirstChild = NULL;

    if ( hstmt != SQL_NULL_HSTMT )
    {
        printf( "JSSQL_DBC:: STMT handle is free\n" );

        retcode = SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
        hstmt = SQL_NULL_HSTMT;
        }

    if ( dbc.type == JS_UNDEFINED ) // we are root
    {
        if ( hdbc != SQL_NULL_HDBC )
        {
            // Disconnect and free connection only if we are the owner of it.
            //
            printf( "JSSQL_DBC:: DBC handle is free\n" );

            retcode = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_COMMIT );

            if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
            {
                //Perror( "SQL.disconnect: commit:" );
                }

            retcode = SQLDisconnect( hdbc );

            if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
            {
                //Perror( "SQL.disconnect: " );
                }

            retcode = SQLFreeHandle( SQL_HANDLE_DBC, hdbc );

            if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
            {
                Perror( "SQL.disconnect: free handle:" );
                }

            hdbc = SQL_NULL_HDBC;
            }
        }
    }

void
JSSQL_DBC:: Perror
(
    char* where
    )
{
    char* errorMsg = vm->error;
    int errorMsgLen = sizeof( vm->error ) - 1;

    {
        int len = sprintf( errorMsg, where );
        errorMsgLen -= len;
        errorMsg += len;
        }

    SQLHANDLE h = henv;
    SQLSMALLINT t = SQL_HANDLE_ENV;
    
    if ( hstmt != SQL_NULL_HSTMT )
    {
        h = hstmt;
        t = SQL_HANDLE_STMT;
        strcpy( errorMsg, "STMT: " ); errorMsg += 6; errorMsgLen -= 6;
        }
    else if ( hdbc != SQL_NULL_HDBC )
    {
        h = hdbc;
        t = SQL_HANDLE_DBC;
        strcpy( errorMsg, "DBC: " ); errorMsg += 5; errorMsgLen -= 5;
        }

    for ( int i = 1 ;; i++ )
    {
        SQLINTEGER NativeError;
        SQLCHAR SqlState[ 6 ];
        SQLSMALLINT MsgLen = errorMsgLen;

        SQLRETURN rc = SQLGetDiagRec(
               t,                     // SQLSMALLINT  fHandleType
               h,                     // SQLHANDLE    handle
               i,                     // SQLSMALLINT  iRecord
               SqlState,              // SQLTCHAR*    szSqlState
               &NativeError,          // SQLINTEGER*  pfNativeError
               (SQLCHAR*)errorMsg,    // SQLTCHAR*    szErrorMsg
               errorMsgLen,           // SQLSMALLINT  cbErrorMsgMax
               &MsgLen                // SQLSMALLINT* pcbErrorMsg
               );

        if ( rc == SQL_NO_DATA )
        {
            if ( i == 1 )
            {
                strcpy( errorMsg, "<unknown>" );
                }
            break;
            }

        errorMsg[ MsgLen ] = '\n';
        errorMsg[ MsgLen + 1 ] = 0;
        errorMsgLen -= MsgLen + 1;
        errorMsg += MsgLen + 1;
        }

    if ( retcode == SQL_SUCCESS_WITH_INFO )
    {
        fprintf( stderr, "%s", vm->error );
        return;
        }

    vm->RaiseError ();
    }

void
JSSQL_DBC:: Connect
(
    JSVariant& dsn,
    JSVariant& uid,
    JSVariant& pwd
    )
{
    assert( dsn.type == JS_STRING );
    assert( uid.type == JS_STRING );
    assert( pwd.type == JS_STRING );

    printf( "JSSQL_DBC:: Connect: %.*s\n", dsn.vstring->len, dsn.vstring->data );
/*
    SQLSetConnectAttr(
        hdbc,                                   // SQLHDBC hdbc
        SQL_ATTR_ODBC_CURSORS,                  // SQLINTEGER fAttribute
        SQLPOINTER( SQL_CUR_USE_ODBC ),         // SQLPOINTER rgbValue
        NULL                                    // SQLINTEGER cbValue
        );
*/
    // Connect to data source
    //
    if ( strnicmp( dsn.vstring->data, "DSN=", 4 ) != 0 )
    {
        retcode = SQLConnect(
            hdbc,                          // SQLHDBC hdbc
            (SQLCHAR*) dsn.vstring->data,  // SQLTCHAR* szDSN
            static_cast<SQLSMALLINT>( dsn.vstring->len ), // SQLSMALLINT cbDSN
            (SQLCHAR*) uid.vstring->data,  // SQLTCHAR* szUID
            static_cast<SQLSMALLINT>( uid.vstring->len ), // SQLSMALLINT cbUID
            (SQLCHAR*) pwd.vstring->data,  // SQLTCHAR* szAuthStr
            static_cast<SQLSMALLINT>( pwd.vstring->len ) // SQLSMALLINT cbAuthStr
            );

        if ( retcode != SQL_SUCCESS )
        {
            Perror( "SQL.connect: " );
            }
        }
    else
    {
        SQLSMALLINT lenOutConnStr = 0;
        unsigned char szOutConnStr[ 1024 ];

        retcode = SQLDriverConnect(
             hdbc,                         // SQLHDBC ConnectionHandle,
             NULL,                         // SQLHWND WindowHandle,
             (SQLCHAR*) dsn.vstring->data, // SQLCHAR* InConnectionString,
             static_cast<SQLSMALLINT>( dsn.vstring->len ), // SQLSMALLINT StringLength1,
             szOutConnStr,                 // SQLCHAR* OutConnectionString,
             sizeof( szOutConnStr ),       // SQLSMALLINT BufferLength,
             &lenOutConnStr,               // SQLSMALLINT* StringLength2Ptr,
             SQL_DRIVER_NOPROMPT           // SQLUSMALLINT DriverCompletion
             );

        if ( retcode != SQL_SUCCESS )
        {
            Perror( "SQL.connect: " );
            }
        }
    }

void
JSSQL_DBC:: DumpInfo
(
    void
    )
{
    char str[ 1024 ];

    retcode = SQLGetInfo( hdbc, SQL_SERVER_NAME, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "Server Name: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DBMS_NAME, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "DBMS Name: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DBMS_VER, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "DBMS Product Version: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DRIVER_NAME, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "Driver Name: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DRIVER_VER, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "Driver Version: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DM_VER, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "Driver Manager Version: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_DRIVER_ODBC_VER, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "Driver ODBC Version: %s\n", str );

    retcode = SQLGetInfo( hdbc, SQL_ODBC_VER, SQLPOINTER( str ), sizeof( str ), NULL );
    printf( "ODBC Version: %s\n", str );
    }

void
JSSQL_DBC:: Disconnect
(
    void
    )
{
    printf( "JSSQL_DBC:: Disconnect\n" );

    for ( JSSQL_DBC* pDBC = pFirstChild; pDBC; pDBC = pDBC->pNext )
    {
        pDBC->Disconnect ();
        }

    pFirstChild = NULL;

    if ( hstmt != SQL_NULL_HSTMT )
    {
        printf( "JSSQL_DBC:: STMT handle is free\n" );

        retcode = SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
        hstmt = SQL_NULL_HSTMT;
        }

    if ( dbc.type == JS_UNDEFINED ) // we are root
    {
        if ( hdbc != SQL_NULL_HDBC )
        {
            // Disconnect and free connection only if we are the owner of it.
            //
            printf( "JSSQL_DBC:: DBC handle is disconnected\n" );

            retcode = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_COMMIT );

            if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
            {
                //Perror( "SQL.disconnect: commit:" );
                }

            retcode = SQLDisconnect( hdbc );

            if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
            {
                //Perror( "SQL.disconnect: " );
                }
            }
        }
    }

void
JSSQL_DBC:: Commit
(
    void
    )
{
    printf( "JSSQL_DBC:: Commit\n" );

    retcode = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_COMMIT );

    if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
    {
        Perror( "SQL.commit: " );
        }
    }

void
JSSQL_DBC:: Rollback
(
    void
    )
{
    printf( "JSSQL_DBC:: Rollback\n" );

    retcode = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK );

    if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
    {
        Perror( "SQL.rollback: " );
        }
    }

void
JSSQL_DBC:: SetAutoCommit
(
    int setOn
    )
{
    printf( "JSSQL_DBC:: SetAutoCommit: %d\n", setOn );

    retcode = SQLSetConnectAttr(
        hdbc,                                   // SQLHDBC hdbc
        SQL_ATTR_AUTOCOMMIT,                    // SQLINTEGER fAttribute
        SQLPOINTER( setOn ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF), //SQLPOINTER rgbValue
        NULL                                     // SQLINTEGER cbValue
        );

    if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
    {
        Perror( "SQL.setAutoCommit: " );
        }
    }

int
JSSQL_DBC:: GetAutoCommit
(
    void
    )
{
    SQLINTEGER option = SQL_AUTOCOMMIT_OFF;

    retcode = SQLGetConnectAttr(
        hdbc,                   // SQLHDBC hdbc
        SQL_ATTR_AUTOCOMMIT,    // SQLINTEGER fAttribute
        &option,                // SQLPOINTER rgbValue
        SQL_IS_INTEGER,         // SQLINTEGER cbValueMax
        NULL                    // SQLINTEGER* pcbValue
        );

    if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
    {
        Perror( "SQL.getAutoCommit: " );
        }

    printf( "JSSQL_DBC:: GetAutoCommit: %d\n", option == SQL_AUTOCOMMIT_ON );

    return option == SQL_AUTOCOMMIT_ON;
    }

void
JSSQL_DBC:: FreeStatement
(
    void
    )
{
    retcode;

    if ( hstmt != SQL_NULL_HSTMT )
    {
        printf( "JSSQL_DBC:: FreeStatement\n" );
        retcode = SQLCloseCursor( hstmt );
        retcode = SQLFreeStmt( hstmt, SQL_UNBIND );
        retcode = SQLFreeStmt( hstmt, SQL_RESET_PARAMS );
        }

    last_stmt.type = JS_UNDEFINED;
    crsr_name.type = JS_UNDEFINED;
    nNumParams = 0;
    }

void
JSSQL_DBC:: CloseCursor
(
    void
    )
{
    retcode;

    if ( hstmt != SQL_NULL_HSTMT )
    {
        printf( "JSSQL_DBC:: CloseCursor\n" );
        retcode = SQLCloseCursor( hstmt );
        }

    last_stmt.type = JS_UNDEFINED;
    crsr_name.type = JS_UNDEFINED;
    nNumParams = 0;
    }

JSVariant
JSSQL_DBC:: ExecSQL
(
    JSVariant& stmt,
    int argc,
    JSVariant argv []
    )
{
    JSVariant cursorName = crsr_name;

    retcode;

    if ( hstmt == SQL_NULL_HSTMT )
    {
        retcode = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );

        if ( retcode != SQL_SUCCESS )
        {
            Perror( "SQL.execSQL: SQLAllocHandle statement: " );
            }
        }
    else
    {
        FreeStatement ();
        }

    last_stmt = stmt;

    if ( cursorName.type == JS_STRING )
    {
        // Specify an updatable static cursor with 20 rows of data. Set
        // the cursor name, execute the SELECT statement, and bind the
        // rowset buffers to result set columns in column-wise fashion.
        //
        SQLSetStmtAttr( hstmt, SQL_ATTR_CONCURRENCY, SQLPOINTER( SQL_CONCUR_VALUES ), 0 );
        SQLSetStmtAttr( hstmt, SQL_ATTR_CURSOR_TYPE, SQLPOINTER( SQL_CURSOR_STATIC ), 0 );

        //SQLSetStmtAttr( hstmt, SQL_ATTR_ROW_ARRAY_SIZE, ROWS, 0 );
        //SQLSetStmtAttr( hstmt, SQL_ATTR_ROW_STATUS_PTR, RowStatusArray, 0 );
        //SQLSetStmtAttr( hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &crow, 0 );

        retcode = SQLSetCursorName(
               hstmt,                              // SQLHSTMT  StatementHandle
               (SQLCHAR*)cursorName.vstring->data, // SQLCHAR* CursorName
               static_cast<SQLSMALLINT>( cursorName.vstring->len ) // SQLSMALLINT NameLength
               );

        if ( retcode != SQL_SUCCESS )
        {
            Perror( "SQL.execSQL: SetCursorName: " );
            }

        printf( "------------------ set cursor to: [%s]\n", cursorName.vstring->data );
        }

    printf( "SQL: %.*s\n", stmt.vstring->len, stmt.vstring->data );

    retcode = SQLPrepare( hstmt, (SQLCHAR*) stmt.vstring->data, stmt.vstring->len );

    if ( retcode != SQL_SUCCESS )
    {
        Perror( "SQL.execSQL: Prepare: " );
        }

    // Bind input parameters
    //
    static SQLINTEGER indNULL = SQL_NULL_DATA;
    static SQLINTEGER indInt32 = sizeof( long );
    static SQLINTEGER indDouble = sizeof( double );

    nNumParams = argc;

    for  ( int i = 0; i < nNumParams; i++ )
    {
        switch ( argv[ i ].type )
        {
            case JS_UNDEFINED:
            case JS_NULL:
            case JS_NAN:
                params[ i ].type = JS_NULL;
                break;

            case JS_BOOLEAN:
                params[ i ].type = JS_INTEGER;
                params[ i ].vinteger = argv[ i ].vboolean;
                break;

            case JS_INTEGER:
                params[ i ] = argv[ i ];
                break;

            case JS_FLOAT:
                params[ i ] = argv[ i ];
                break;

            case JS_FUNCTION:
            case JS_ARRAY:
            case JS_OBJECT:
            case JS_BUILTIN:
                argv[ i ].ToString( vm, params[ i ] );
                break;

            case JS_STRING:
                params[ i ] = argv[ i ];
                break;

            case JS_INSTR_PTR:
            case JS_BASE_PTR:
            case JS_FRAME_PTR:
            case JS_WITH_CHAIN:
            case JS_ARGS_FIX:
            default:
                vm->RaiseError( "Unboundable parameter %d type", i + 1 );
            }

        switch ( params[ i ].type )
        {
            case JS_NULL:
                printf( "param[%d] = null\n", i + 1 );

                retcode  = SQLBindParameter(
                    hstmt,             // SQLHSTMT StatementHandle
                    i + 1,             // SQLUSMALLINT ParameterNumber
                    SQL_PARAM_INPUT,   // SQLSMALLINT InputOutputType
                    SQL_C_CHAR,        // SQLSMALLINT ValueType
                    SQL_CHAR,          // SQLSMALLINT ParameterType
                    1,                 // SQLUINTEGER ColumnSize
                    0,                 // SQLSMALLINT DecimalDigits
                    "",                // SQLPOINTER ParameterValuePtr
                    0,                 // SQLINTEGER BufferLength
                    &indNULL           // SQLINTEGER* StrLen_or_IndPtr
                    );
                break;

            case JS_INTEGER:
                params[ i ] = argv[ i ];

                printf( "param[%d] = int(%ld)\n", i + 1, argv[ i ].vinteger );

                retcode  = SQLBindParameter(
                    hstmt,             // SQLHSTMT StatementHandle
                    i + 1,             // SQLUSMALLINT ParameterNumber
                    SQL_PARAM_INPUT,   // SQLSMALLINT InputOutputType
                    SQL_C_LONG,        // SQLSMALLINT ValueType
                    SQL_INTEGER,       // SQLSMALLINT ParameterType
                    4,                 // SQLUINTEGER ColumnSize
                    0,                 // SQLSMALLINT DecimalDigits
                    &params[ i ].vinteger, // SQLPOINTER ParameterValuePtr
                    4,                 // SQLINTEGER BufferLength
                    &indInt32          // SQLINTEGER* StrLen_or_IndPtr
                    );
                break;

            case JS_FLOAT:
                params[ i ] = argv[ i ];

                printf( "param[%d] = float(%lf)\n", i + 1, argv[ i ].vfloat );

                retcode  = SQLBindParameter(
                    hstmt,             // SQLHSTMT StatementHandle
                    i + 1,             // SQLUSMALLINT ParameterNumber
                    SQL_PARAM_INPUT,   // SQLSMALLINT InputOutputType
                    SQL_C_DOUBLE,      // SQLSMALLINT ValueType
                    SQL_DOUBLE,        // SQLSMALLINT ParameterType
                    8,                 // SQLUINTEGER ColumnSize
                    0,                 // SQLSMALLINT DecimalDigits
                    &params[ i ].vfloat, // SQLPOINTER ParameterValuePtr
                    8,                 // SQLINTEGER BufferLength
                    &indDouble         // SQLINTEGER* StrLen_or_IndPtr
                    );
                break;

            case JS_STRING:
            {
                params[ i ] = argv[ i ];

                printf( "param[%d] = str(%.*s)/len=%ld\n",
                    i + 1, params[ i ].vstring->len, params[ i ].vstring->data,
                    params[ i ].vstring->len
                    );

                SQLUINTEGER colSize = params[ i ].vstring->len;
                if ( colSize == 0 )
                    colSize = 1;

                retcode  = SQLBindParameter(
                    hstmt,             // SQLHSTMT StatementHandle
                    i + 1,             // SQLUSMALLINT ParameterNumber
                    SQL_PARAM_INPUT,   // SQLSMALLINT InputOutputType
                    SQL_C_CHAR,        // SQLSMALLINT ValueType
                    SQL_CHAR,          // SQLSMALLINT ParameterType
                    colSize,           // SQLUINTEGER ColumnSize
                    0,                 // SQLSMALLINT DecimalDigits
                    params[ i ].vstring->data, // SQLPOINTER ParameterValuePtr
                    params[ i ].vstring->len,  // SQLINTEGER BufferLength
                    &params[ i ].vstring->len  // SQLINTEGER* StrLen_or_IndPtr
                    );
                }
                break;

            default:
                // if we are here, it's certainly a BUG !
                assert( 1 );
            }

        if ( retcode != SQL_SUCCESS )
        {
            Perror( "SQL.execSQL: SQLBindParameter: " );
            }
        }

    SQLSMALLINT nNumCols;
    SQLNumResultCols( hstmt, &nNumCols );

    if ( nNumCols > 0 )
    {
        rowset = NEW JSSQL_ROWSET( this, dbc );
        }
    else
    {
        rowset = NULL;
        }

    JSVariant result_return;
    result_return.type = JS_UNDEFINED;

    // Finaly execute statement
    //
    retcode = SQLExecute( hstmt );

    printf( "SQLExecute -> " );
    switch( retcode )
    {
        case SQL_SUCCESS:           printf( "success\n" ); break;
        case SQL_SUCCESS_WITH_INFO: printf( "success with info\n" ); break;
        case SQL_NEED_DATA:         printf( "need data\n" ); break;
        case SQL_STILL_EXECUTING:   printf( "still executing\n" ); break;
        case SQL_ERROR:             printf( "error\n" ); break;
        case SQL_NO_DATA:           printf( "no data\n" ); break;
        case SQL_INVALID_HANDLE:    printf( "invalid handle\n" ); break;
        default:                    printf( "retcode = %lu\n", retcode );
        }

    if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO && retcode != SQL_NO_DATA )
    {
        if ( rowset )
            delete rowset;
        rowset = NULL;

        Perror( "SQL.execSQL: Execute: " );
        return result_return;
        }

    if ( ! rowset )
    {
        // It is not select statement; see affected number of rows
        //
        SQLINTEGER nRowCount = 0;
        retcode = SQLRowCount( hstmt, &nRowCount );

        if ( retcode == SQL_SUCCESS )
        {
            result_return.type = JS_INTEGER;
            result_return.vinteger = nRowCount;
            }
        }
    else
    {
        // Create the builtin SQL$rowset for rowset
        //
        JSBuiltinInfo* info = vm->Intern( "SQL$rowset" )->vbuiltin->info;
        result_return = new(vm) JSBuiltin( info, rowset );

        JSVariant val;
        val.MakeString( vm, "pericina vrednost" );

        result_return.vbuiltin->prototype->StoreProperty( vm->Intern( "pera" ), val );
        }

    return result_return;
    }

int
JSSQL_DBC:: Fetch
(
    void
    )
{
    retcode = SQLFetch( hstmt );
    
    if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) 
    {
        return 1;
        }

    if ( retcode == SQL_NO_DATA )
    {
        return 0;
        }

    Perror( "SQL.fetch: " );

    return 0;
    }

JSSQL_ROWSET:: JSSQL_ROWSET
(
    JSSQL_DBC* parenta,
    JSVariant& parentb
    )
{
    stmt = parenta;
    dbc = parentb;

    SQLHDBC hdbc = parenta->hdbc;
    SQLHSTMT hstmt = parenta->hstmt;

    // We allocate a buffer that for each column this buffer
    // contains memory for the column's data and length/indicator.
    // For example:
    //
    // |  column 1   |   column 2     |        |   column N   |
    // |<----------->|<-------------->|        |<------------>|
    // |    db1   li1|     db2     li2|        |     dbN   liN|
    // |     |     | |      |       | |        |      |     | |
    // |_____V_____V_|______V_______V_|_...____|______V_____V_|
    // |__________|__|_____________|__|_   _|__|___________|__|
    //
    // dbN = data buffer for column N
    // liN = length/indicator buffer for column N

    // Determine the number of result set columns.  Allocate arrays to hold the C type,
    // byte length, and buffer offset to the data.
    //
    stmt->retcode = SQLNumResultCols( hstmt, &nNumCols );

    prop = NEW JSSymbol[ nNumCols ];
    wCType = NEW SQLSMALLINT[ nNumCols ];
    nColLen = NEW SQLINTEGER[ nNumCols ];
    nOffset = NEW SQLINTEGER[ nNumCols ];

    for ( int i = 0; i < nNumCols; i++ )
    {
        // Determine the column's SQL type. GetDefaultCType contains a switch statement that
        // returns the default C type for each SQL type.
 
        SQLCHAR szColumnName[ 256 ];
        SQLSMALLINT ColumnLen;
        SQLSMALLINT SqlType;
        SQLUINTEGER ColumnSize;
        SQLSMALLINT Nullable;
        SQLSMALLINT DecimalDigits;

        stmt->retcode = SQLDescribeCol(
            hstmt,           // SQLHSTMT StatementHandle
            i + 1,           // SQLSMALLINT ColumnNumber
            szColumnName,    // SQLCHAR* ColumnName
            sizeof( szColumnName ), // SQLSMALLINT BufferLength
            &ColumnLen,      // SQLSMALLINT* NameLengthPtr
            &SqlType,        // SQLSMALLINT* DataTypePtr
            &ColumnSize,     // SQLUINTEGER* ColumnSizePtr
            &DecimalDigits,  // SQLSMALLINT* DecimalDigitsPtr
            &Nullable        // SQLSMALLINT* NullablePtr
            );

        if ( stmt->retcode != SQL_SUCCESS )
        {
            stmt->Perror( "SQL.execSQL: DescribeCol: " );
            }

        prop[ i ] = stmt->vm->Intern( (char*)szColumnName );

        GetDefaultCType( SqlType, wCType[ i ], nColLen[ i ] );

        SQLINTEGER nDisplaySize;

        stmt->retcode = SQLColAttribute(
            hstmt,        // SQLHSTMT hstmt
            i + 1 ,       // SQLSMALLINT iCol
            SQL_DESC_DISPLAY_SIZE, // SQLSMALLINT iFiled
            NULL,         // SQLPOINTER pCharAttr
            0,            // SQLSMALLINT cbCharAttrMax
            NULL,         // SQLSMALLINT* pcbCharAttr
            &nDisplaySize // SQLPOINTER pNumAttr
            );

        SqlType = SQL_C_CHAR;
        nColLen[ i ] = nDisplaySize + 1;

        if ( nColLen[ i ] > 1000 )
            nColLen[ i ] = 1000;

        printf( "col[%s] len=%hd type=%hd size=%d nullable=%hd decdigits=%hd collen=%d\n",
            szColumnName, ColumnLen, SqlType, ColumnSize, Nullable, DecimalDigits,
            nColLen[ i ] );

        // Determine the column's byte length. Calculate the offset in the buffer to the
        // data as the offset to the previous column, plus the byte length of the previous
        // column, plus the byte length of the previous column's length/indicator buffer.
        // Note that the byte length of the column and the length/indicator buffer are
        // increased so that, assuming they start on an alignment boundary, they will end on
        // the byte before the next alignment boundary. Although this might leave some holes
        // in the buffer, it is a relatively inexpensive way to guarantee alignment.

        nColLen[ i ] = ALIGNBUF( nColLen[ i ] );

        if ( i > 0 )
        {
            nOffset[ i ] = nOffset[ i - 1 ] + nColLen[ i - 1 ]
                         + ALIGNBUF( sizeof( SQLINTEGER ) );
            }
        else
        {
            nOffset[ i ] = 0;
            }
        }

    // Allocate the data buffer. The size of the buffer is equal to the offset to the data
    // buffer for the final column, plus the byte length of the data buffer and
    // length/indicator buffer for the last column.

    DataPtr = NEW uint8[ nOffset[ nNumCols - 1 ] + nColLen[ nNumCols - 1 ]
                         + ALIGNBUF( sizeof( SQLINTEGER ) ) ];

    // For each column, bind the address in the buffer at the start of the memory allocated
    // for that column's data and the address at the start of the memory allocated for that
    // column's length/indicator buffer.

    for ( i = 0; i < nNumCols; i++ )
    {
        stmt->retcode = SQLBindCol(
            hstmt,                                // SQLHSTMT StatementHadnle
            i + 1,                                // SQLUSMALLINT ColumnNumber
            wCType[ i ],                          // SQLSMALLINT TargetType
            SQLPOINTER( DataPtr + nOffset[ i ] ), // SQLPOINTER TargetValue
            nColLen[ i ],                         // SQLINTEGER BufferLength
            (SQLINTEGER*)( DataPtr + nOffset[ i ] + nColLen[ i ] ) // SQLINTEGER* StrLen_or_Ind
            );

        if ( stmt->retcode != SQL_SUCCESS )
        {
            stmt->Perror( "SQL.execSQL: BindCol: " );
            }
        }
    }

JSSQL_ROWSET:: ~JSSQL_ROWSET
(
    void
    )
{
    Free ();

    delete prop;
    delete wCType;
    delete nColLen;
    delete nOffset;
    delete DataPtr;
    }

void
JSSQL_ROWSET:: Free
(
    void
    )
{
    stmt = NULL;
    }

int
JSSQL_ROWSET:: Fetch
(
    void
    )
{
    return stmt ? stmt->Fetch () : 0;
    }
