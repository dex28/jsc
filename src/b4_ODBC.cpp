//
// SQL classes: SQL$env, SQL and SQL$rowset
//

#include "JS.h"
#include "ODBC.h"

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo_SQLenv class interface and implementation

struct JSBuiltinInfo_SQLenv : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_free;

    JSBuiltinInfo_SQLenv( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );
    };

JSBuiltinInfo_SQLenv:: JSBuiltinInfo_SQLenv( void )
    : JSBuiltinInfo( "SQL$env" )
{
    s_free = vm->Intern( "free" );
    }

JSPropertyRC
JSBuiltinInfo_SQLenv:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSSQL_ENV *ictx = (JSSQL_ENV*)instance_context;

    // Set the default return value
    //
    result_return.type = JS_UNDEFINED;

    // Static methods
    //
    //--------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( ictx )
        {
            result_return.MakeStaticString( vm, sql$env" );
            }
        else
        {
            result_return.MakeStaticString( vm, "SQL$env" );
            }

        return JS_PROPERTY_FOUND;
        }

    // Methods
    //
    if ( ictx == NULL )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //--------------------------------------------------------------------
    if ( method == s_free )
    {
        ictx->Free ();

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

//argument_error:
    vm->RaiseError( "SQL.%s(): illegal amount of arguments", vm->Symname( method ) );

//argument_type_error:
    vm->RaiseError( "SQL.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_SQLenv:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger < 0 || args->vinteger > 1 )
    {
        vm->RaiseError( "new SQL$env(): illegal amount of arguments" );
        }

    JSSQL_ENV* ictx = NULL;   

    if ( args->vinteger == 0 )
    {
        ictx = NEW JSSQL_ENV( vm );
        }
    else
    {
        vm->RaiseError( "new SQL$env(): illegal number of arguments" );
        }

    assert( ictx != NULL );

    result_return = new(vm) JSBuiltin( this, ictx );
    }

void
JSBuiltinInfo_SQLenv:: OnFinalize
(
    void* instance_context
    )
{
    JSSQL_ENV* ictx = (JSSQL_ENV*)instance_context;

    if ( ictx )
        delete ictx;
    }

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo_SQL class interface and implementation

struct JSBuiltinInfo_SQL : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_connect;
    JSSymbol s_disconnect;
    JSSymbol s_commit;
    JSSymbol s_rollback;
    JSSymbol s_setAutoCommit;
    JSSymbol s_getAutoCommit;
    JSSymbol s_setCursorName;
    JSSymbol s_execSql;
    JSSymbol s_fetch;
    JSSymbol s_free;

    JSBuiltinInfo_SQL( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );

    virtual void OnMark ( void* instance_context );
    };

JSBuiltinInfo_SQL:: JSBuiltinInfo_SQL( void )
    : JSBuiltinInfo( "SQL" )
{
    s_connect        = vm->Intern( "connect" );
    s_disconnect     = vm->Intern( "disconnect" );
    s_commit         = vm->Intern( "commit" );
    s_rollback       = vm->Intern( "rollback" );
    s_setAutoCommit  = vm->Intern( "setAutoCommit" );
    s_getAutoCommit  = vm->Intern( "getAutoCommit" );
    s_setCursorName  = vm->Intern( "setCursorName" );
    s_execSql        = vm->Intern( "execSql" );
    s_fetch          = vm->Intern( "fetch" );
    s_free           = vm->Intern( "free" );
    }

JSPropertyRC
JSBuiltinInfo_SQL:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSSQL_DBC *ictx = (JSSQL_DBC*)instance_context;

    // Set the default return value
    //
    result_return.type = JS_UNDEFINED;

    // Static methods
    //
    //--------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( ictx )
        {
            result_return.MakeStaticString( vm, "sql" );
            }
        else
        {
            result_return.MakeStaticString( vm, "SQL" );
            }

        return JS_PROPERTY_FOUND;
        }

    // Methods
    //
    if ( ictx == NULL )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //--------------------------------------------------------------------
    if ( method == s_connect )
    {
        if ( args->vinteger < 1 && args->vinteger > 3 )
            goto argument_error;

        JSVariant dsn = args[ 1 ];
        if ( dsn.type != JS_STRING )
        {
            args[ 1 ].ToString( vm, dsn );
            }

        JSVariant uid = args[ 2 ];
        if ( uid.type != JS_STRING )
        {
            args[ 1 ].ToString( vm, uid );
            }

        JSVariant pwd = args[ 3 ];
        if ( pwd.type != JS_STRING )
        {
            args[ 1 ].ToString( vm, pwd );
            }

        ictx->Connect( dsn, uid, pwd );
        ictx->DumpInfo ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_disconnect )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Disconnect ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_commit )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Commit ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_rollback )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Rollback ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_setAutoCommit )
    {
        if ( args->vinteger < 0 || args->vinteger > 1 )
            goto argument_error;

        if ( args->vinteger == 0 )
        {
            ictx->SetAutoCommit( 1 );
            }
        else
        {
            if ( args[ 1 ].type != JS_BOOLEAN )
                goto argument_type_error;

            ictx->SetAutoCommit( args[ 1 ].vboolean );
            }

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_getAutoCommit )
    {
        if ( args->vinteger < 0 || args->vinteger > 1 )
            goto argument_error;

        result_return.type = JS_INTEGER;
        result_return.vboolean = ictx->GetAutoCommit ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_execSql )
    {
        if ( args->vinteger < 1 )
        {
            goto argument_error;
            }

        JSVariant stmt = args[ 1 ];
        if ( stmt.type != JS_STRING )
        {
            args[ 1 ].ToString( vm, stmt );
            }

        result_return = ictx->ExecSQL( stmt, args->vinteger - 1, args + 2 );

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_fetch )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_BOOLEAN;
        result_return.vboolean = ictx->Fetch ();

        return JS_PROPERTY_FOUND;
        }

    //--------------------------------------------------------------------
    else if ( method == s_free )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        ictx->Free ();

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "SQL.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "SQL.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_SQL:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger < 0 || args->vinteger > 1 )
    {
        vm->RaiseError( "new SQL(): illegal amount of arguments" );
        }

    JSSQL_DBC* ictx = NULL;   

    if ( args->vinteger == 0 )
    {
        JSBuiltinInfo* info = vm->Intern( "SQL$env" )->vbuiltin->info;
        JSVariant env;
        env = new(vm) JSBuiltin( info, NEW JSSQL_ENV( vm ) );

        ictx = NEW JSSQL_DBC( env );
        }
    else if ( args[ 1 ].type == JS_BUILTIN )
    {
        ictx = NEW JSSQL_DBC( args[ 1 ] );
        }
    else
    {
        vm->RaiseError( "new SQL(): illegal argument" );
        }

    assert( ictx != NULL );

    result_return = new(vm) JSBuiltin( this, ictx );
    }

void
JSBuiltinInfo_SQL:: OnFinalize
(
    void* instance_context
    )
{
    JSSQL_DBC *ictx = (JSSQL_DBC*)instance_context;

    if ( ictx )
        delete ictx;
    }

void
JSBuiltinInfo_SQL:: OnMark
(
    void* instance_context
    )
{
    if ( instance_context )
    {
        JSSQL_DBC *ictx = (JSSQL_DBC*)instance_context;

        ictx->dbc.Mark ();
        ictx->env.Mark ();

        for ( int i = 0; i < ictx->nNumParams; i++ )
        {
            ictx->params[ i ].Mark ();
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////
// JSBuiltinInfo_SQL class interface and implementation

struct JSBuiltinInfo_SQLrowset : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_fetch;

    JSBuiltinInfo_SQLrowset( void );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );

    virtual void OnMark ( void* instance_context );
    };

JSBuiltinInfo_SQLrowset:: JSBuiltinInfo_SQLrowset( void )
    : JSBuiltinInfo( "SQL$rowset" )
{
    s_fetch          = vm->Intern( "fetch" );
    }

JSPropertyRC
JSBuiltinInfo_SQLrowset:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSSQL_ROWSET *ictx = (JSSQL_ROWSET*)instance_context;

    // Set the default return value
    //
    result_return.type = JS_UNDEFINED;

    // Static methods
    //
    //--------------------------------------------------------------------
    if ( method == vm->s_toString )
    {
        if ( ictx )
        {
            result_return.MakeStaticString( vm, "sql$rowset" );
            }
        else
        {
            result_return.MakeStaticString( vm, "SQL$rowset" );
            }

        return JS_PROPERTY_FOUND;
        }

    // Methods
    //
    if ( ictx == NULL )
    {
        return JS_PROPERTY_UNKNOWN;
        }

    //--------------------------------------------------------------------
    if ( method == vm->s___Nth__ )
    {
        if ( args->vinteger != 1 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( args[ 1 ].vinteger >= 0 && args[ 1 ].vinteger < ictx->nNumCols )
        {
            result_return.MakeStaticString( vm, vm->Symname( ictx->prop[ args[ 1 ].vinteger ] ) );

            return JS_PROPERTY_FOUND;
            }
        else
        {
            return JS_PROPERTY_UNKNOWN;
            }
        }

    //--------------------------------------------------------------------
    else if ( method == s_fetch )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        result_return.type = JS_BOOLEAN;
        result_return.vboolean = ictx->Fetch ();

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "SQL.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "SQL.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_SQLrowset:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    JSSQL_ROWSET* ictx = (JSSQL_ROWSET*)instance_context;

    if ( property == vm->Intern( "pera2" ) )
    {
        node.type = JS_BOOLEAN;
        node.vboolean = TRUE;
        return JS_PROPERTY_FOUND;
        }

    for ( int i = 0; i < ictx->nNumCols; i++ )
    {
        if ( property == ictx->prop[ i ] )
        {
            if ( set )
                goto immutable;

            if ( ictx->wCType[ i ] != SQL_C_CHAR )
            {
                node.type = JS_UNDEFINED;
                return JS_PROPERTY_FOUND;
                }

            SQLINTEGER len = *(SQLINTEGER*)( ictx->DataPtr + ictx->nOffset[ i ] + ictx->nColLen[ i ] );

            switch( len )
            {
                case SQL_NULL_DATA:
                    node.type = JS_NULL;
                    break;
                case SQL_NO_TOTAL:
                case SQL_DATA_AT_EXEC:
                case SQL_COLUMN_IGNORE:
                    node.type = JS_UNDEFINED;
                    break;
                case SQL_NTS:
                    node.MakeStaticString( vm, (char*)ictx->DataPtr + ictx->nOffset[ i ] );
                    break;
                default:
                    node.MakeStaticString( vm, (char*)ictx->DataPtr + ictx->nOffset[ i ], len );
                }

            return JS_PROPERTY_FOUND;
            }
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

immutable:
    vm->RaiseError( "System.%s: immutable property", vm->Symname( property ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_SQLrowset:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    vm->RaiseError( "new SQL(): possible only in SQL.ExecSQL" );

    result_return.type = JS_UNDEFINED;
    }

void
JSBuiltinInfo_SQLrowset:: OnFinalize
(
    void* instance_context
    )
{
    JSSQL_ROWSET *ictx = (JSSQL_ROWSET*)instance_context;

    if ( ictx )
        delete ictx;
    }

void
JSBuiltinInfo_SQLrowset:: OnMark
(
    void* instance_context
    )
{
    if ( instance_context )
    {
        JSSQL_ROWSET *ictx = (JSSQL_ROWSET*)instance_context;

        ictx->dbc.Mark ();
        }
    }

//
// The SQL class initialization entry
//

void
JSVirtualMachine:: BuiltinSQL( void )
{
    new(this) JSBuiltinInfo_SQLenv;
    new(this) JSBuiltinInfo_SQL;
    new(this) JSBuiltinInfo_SQLrowset;
    }
