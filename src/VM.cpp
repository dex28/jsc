//
// JSVirtualMachine implementation.
// Common stuff.
//

#include "JS.h"

///////////////////////////////////////////////////////////////////////////////
// JSVMOptions class implementation

JSVMOptions:: JSVMOptions( void )
{
    memset( this, 0, sizeof( *this ) );

    gc_trigger = 2L * 1024L * 1024L;

    stack_size = 2048;

    warn_undef = 1;
    }

///////////////////////////////////////////////////////////////////////////////
// JSVirtualMachine implementation

PCSTR
JSVirtualMachine:: Version ()
{
    return VERSION;
    }

JSVirtualMachine:: JSVirtualMachine( void )
{
    should_terminate = 0;

    s_stdin = NULL;
    s_stdout = NULL;
    s_stderr = NULL;

    pSymbLinkage = NULL;

    globals = 0;
    num_globals = 0;
    globals_alloc = 0;

    memset( &globals_hash, 0, sizeof( globals_hash ) );

    stack = NULL;
    stack_size = 0;

    SP = NULL;
    PC = NULL;
	FP = NULL;
    BP = NULL;

    memset( &prim, 0, sizeof( prim ) );

    heap = NULL;
    heap_size = 0;
    memset( &heap_freelists, 0, sizeof( heap_freelists ) );

    gc.bytes_allocated = 0;
    gc.bytes_free = 0;
    gc.count = 0;

    error_handler = NULL;

    error[ 0 ] = 0;

    exec_result.type = JS_UNDEFINED;

#ifdef PROFILING
    memset( &prof_count, 0, sizeof( prof_count ) );
    memset( &prof_elapsed, 0, sizeof( prof_elapsed ) );
#endif
    }

JSVirtualMachine:: ~JSVirtualMachine( void )
{
    // Free all objects from the heap
    //
    ClearHeap ();

    // Free bytecode symbols linkage
    //
    SymbLinkage* sb2;
    for( SymbLinkage* sb = pSymbLinkage; sb; sb = sb2 )
    {
        sb2 = sb->next;
        sb->bc->decrementRefCount ();
        delete sb->linkage;
        delete sb;
        }

    // Free the globals
    //
    for( int i = 0; i < JS_HASH_TABLE_SIZE; i++ )
    {
        HashBucket* hashb_next;
        for( HashBucket* hashb = globals_hash[ i ]; hashb; hashb = hashb_next )
        {
            hashb_next = hashb->next;
            delete hashb->name;
            delete hashb;
            }
        }

    delete globals;

    // Stack
    //
    delete stack;

    // Heap blocks
    //
    JSHeapBlock* hb2;
    for( JSHeapBlock* hb = heap; hb; hb = hb2 )
    {
        hb2 = hb->next;
        delete hb;
        }

    // Error handlers
    //
    JSErrorHandlerFrame* f2;
    for( JSErrorHandlerFrame* f = error_handler; f; f = f2 )
    {
        f2 = f->next;
        delete f;
        }

#ifdef PROFILING

    // Dump profiling data into text file
    //
    {
        FILE* outf = fopen( "Profiling.txt", "wt" );

        // Count total instructions
        //
        long total = 0;
        double total_elapsed = 0;
        for( int i = 0; i <= JS_OPCODE_LAST_OPCODE; i++ )
        {
            total += prof_count[ i ];
            total_elapsed += double( prof_elapsed[ i ] ) / 233.0;
            }

        // Dump individual statistics
        //
        for( i = 0; i <= JS_OPCODE_LAST_OPCODE; i++ )
        {
            double elapsed = double( prof_elapsed[ i ] ) / 233.0;

            fprintf( outf, "%3d %8ld %5.2lf %11.3lf %5.2lf %7.3lf\n",
                     i,
                     prof_count[ i ],
                     double( prof_count[ i ] ) / total * 100.0,
                     elapsed,
                     elapsed / total_elapsed * 100.0,
                     prof_count[ i ] == 0 ? 0 : elapsed / double( prof_count[ i ] )
                     );
            }

        fprintf( outf, "Tot %ld instructions executed in %.6lf sec\n",
                total, total_elapsed / 1.0E6 );

        fclose( outf );
        }

#endif // PROFILING

    // Flush and free the default system streams
    //

    delete s_stdin;
    delete s_stdout;
    delete s_stderr;
    }

int
JSVirtualMachine:: Create( void )
{
    // The default system streams.
    //
    s_stdin = NULL;
    s_stdout = NULL;
    s_stderr = NULL;

    if ( options.s_stdin )
        s_stdin = NEW JSIOStreamIOFunc( options.s_stdin, options.s_context, 1, 0 );
    else
        s_stdin = NEW JSIOStreamFile( stdin, 1, 0, 0 );

    if ( ! JSIOStream::IsValid( s_stdin ) )
        goto ERROR_OUT;

    if ( options.s_stdout )
        s_stdout = NEW JSIOStreamIOFunc( options.s_stdout, options.s_context, 0, 1 );
    else
        s_stdout = NEW JSIOStreamFile( stdout, 0, 1, 0 );

    if ( ! JSIOStream::IsValid( s_stdout ) )
        goto ERROR_OUT;

    s_stdout->SetAutoFlush ();

    if ( options.s_stderr )
        s_stderr = NEW JSIOStreamIOFunc( options.s_stderr, options.s_context, 0, 1 );
    else
        s_stderr = NEW JSIOStreamFile( stderr, 0, 1, 0 );

    if ( ! JSIOStream::IsValid( s_stderr ) )
        goto ERROR_OUT;

    s_stderr->SetAutoFlush ();

    // Streams are created, create the rest of the virtual machine.
    //
    should_terminate = 0;

    stack_size = options.stack_size;
    stack = NEW JSVariant[ stack_size ];
    if ( stack == NULL )
    {
        goto ERROR_OUT;
        }

    // Set the initial stack pointer
    //
    SP = stack + stack_size - 1;

    error_handler = NULL;

    {
        bool result_OK = true;

        // Top-level error handler
        //
        JSErrorHandlerFrame handler( error_handler );
        error_handler = &handler;

        try
        {
            // Intern some commonly used symbols
            //
            s___proto__    = Intern( "__proto__" );
            s_prototype    = Intern( "prototype" );
            s_toSource     = Intern( "toSource" );
            s_toString     = Intern( "toString" );
            s_valueOf      = Intern( "valueOf" );
            s_undefined    = Intern( "undefined" );
            s___Nth__      = Intern( "__Nth__" );

            // Intern system built-in objects.

            // The core global object methods
            //
            BuiltinCoreGM ();       // b0_Core.cpp

            // Language objects
            //
            BuiltinArray ();        // b1_Array.cpp
            BuiltinBoolean ();      // b1_Bool.cpp
            BuiltinFunction ();     // b1_Func.cpp
            BuiltinNumber ();       // b1_Number.cpp
            BuiltinObject ();       // b1_Object.cpp
            BuiltinString ();       // b1_String.cpp

            // Builtin extensions
            //
            BuiltinDate ();         // b2_Date.cpp
            BuiltinDirectory ();    // b2_Dir.cpp
            BuiltinFile ();         // b2_File.cpp
            BuiltinMath ();         // b2_Math.cpp
            BuiltinSystem ();       // b2_System.cpp
            BuiltinVM ();           // b2_VM.cpp

            // More builtin extensions
            //
            BuiltinMD5 ();          // b4_MD5.cpp
            BuiltinSQL ();          // b4_SQL.cpp
            }
        catch( JSErrorHandlerFrame* )
        {
            result_OK = false;
            }

        // Pop the error handler
        //
        assert( error_handler == &handler );
        error_handler = error_handler->next;

        if ( ! result_OK )
        {
            // Argh, the initialization failed
            //
            goto ERROR_OUT;
            }
        }

    // Let's see how much memory an empty interpreter takes:
    //
    extern void jsAllocSetMinimum( void );
    jsAllocSetMinimum ();

    // Ok, we're done.
    //
    return 1;

////////////////////
// Error handling.
//
ERROR_OUT:

    if ( s_stdin )
        delete s_stdin;

    if ( s_stdout )
        delete s_stdout;

    if ( s_stderr )
        delete s_stderr;

    return 0;
    }

void
JSVirtualMachine:: SetTerminate( int politeLevel )
{
    should_terminate = politeLevel;
    }

void
JSVirtualMachine:: Result( JSVariant& result_return )
{
    result_return = exec_result;
    }

void
JSVirtualMachine:: SetVar( PCSTR name, JSVariant& value )
{
    *Intern( name ) = value;
    }

void
JSVirtualMachine:: GetVar( PCSTR name, JSVariant& value )
{
    value = *Intern( name );
    }

JSSymbol
JSVirtualMachine:: Intern( PCSTR name, int name_len )
{
    if ( name_len < 0 )
    {
        name_len = strlen( name );
        }

    int pos = jsHashFunction( name, name_len ) % JS_HASH_TABLE_SIZE;

    for( HashBucket* b = globals_hash[ pos ]; b; b = b->next )
    {
        if ( strcmp( b->name, name ) == 0 )
        {
            return b->vsymbol;
            }
        }

    b = NEW HashBucket;
    b->name = jsStrDup( name );

    b->next = globals_hash[ pos ];
    globals_hash[ pos ] = b;

    // Alloc space from the globals array
    //
    if ( num_globals >= globals_alloc )
    {
        globals = (JSVariant*)jsRealloc( globals,
                                ( globals_alloc + 1024 ) * sizeof( JSVariant ) );
        globals_alloc += 1024;
        }

    // Initialize symbol's name spaces
    //
    globals[ num_globals ].type = JS_UNDEFINED;
    b->vsymbol = &globals[ num_globals++ ];

    return b->vsymbol;
    }

PCSTR
JSVirtualMachine:: Symname( JSSymbol sym )
{
    for( int i = 0; i < JS_HASH_TABLE_SIZE; i++ )
    {
        for( HashBucket* b = globals_hash[ i ]; b; b = b->next )
        {
            if ( b->vsymbol == sym )
            {
                return b->name;
                }
            }
        }

    return "???";
    }

void
JSVirtualMachine:: RaiseError( PCSTR fmt ... )
{
    va_list marker;
    va_start( marker, fmt );
    vsprintf( error, fmt, marker );
    va_end( marker );

    RaiseError ();
    }

void
JSVirtualMachine:: RaiseError( void )
{
    PCSTR file; // debug source filename info
    int ln; // debug source line number info

    if ( DebugPosition( PC, file, ln ) && file )
    {
        char szErrStr[ 1024 ];
        sprintf( szErrStr, "%s:%d: %s", file, ln, error );
        strcpy( error, szErrStr );
        }

    if ( options.stacktrace_on_error )
    {
        s_stderr->PrintfLn( "VM: error: %s", error );
        StackTrace ();
        }

    if ( error_handler->SP )
    {
        // We are jumping to a catch-block. Format our error message to
        // the 'thrown' variant.
        //
        error_handler->thrown.MakeString( this, error );
        }

    throw error_handler;

    assert( 1 ); // NOTREACHED (I hope)
    }

///////////////////////////////////////////////////////////////////////////////
// Execute

bool
JSVirtualMachine:: Execute( JSByteCode* bc )
{
    JSErrorHandlerFrame* handler = NEW JSErrorHandlerFrame( error_handler );
    if ( handler == NULL )
    {
        strcpy( error, "VM: out of memory" );
        return 0;
        }

    JSErrorHandlerFrame* saved_handler = error_handler;
    error_handler = handler;

    // Clear error message and old exec result
    //
    error[ 0 ] = '\0';
    exec_result.type = JS_UNDEFINED;

    JSVariant* saved_sp = SP;

    bool result_OK = true;
    JSFunction* global_f = NULL;

    // Find out do we have bytecode already used ?
    //
    for ( SymbLinkage* linkp = pSymbLinkage; linkp; linkp = linkp->next )
    {
        if ( linkp->bc == bc )
        {
            global_f = linkp->global_f;
            break;
            }
        }

    if ( ! global_f )
    {
        // First use of bytecode
        //
        bc->incrementRefCount ();

        // Resolve byte-code imported-symbols.
        //
        JSSymbol* linkage = NEW JSSymbol[ bc->i_symb_count ];
        for( long i = 0; i < bc->i_symb_count; i++ )
        {
            linkage[ i ] = Intern( bc->i_symb[ i ] );
            }

        // Update symbolic linkage list
        //
        SymbLinkage* sl = NEW SymbLinkage;
        sl->next = pSymbLinkage;
        sl->linkage = linkage;
        sl->bc = bc;
        sl->global_f = NULL;
        pSymbLinkage = sl;

        try
        {
            // Enter all functions to the known functions of the VM
            //

            for( long i = 0; i < bc->e_symb_count; i++ )
            {
                // Link the code to our environment
                //
                JSFunction* f = new(this) JSFunction( bc, i, linkage );

                if ( strcmp( bc->e_symb[ i ].name, ".global" ) == 0 )
                {
                    sl->global_f = f;
                    global_f = f;
                    }
                else
                {
                    *Intern( bc->e_symb[ i ].name ) = f;
                    }
                }
            }
        catch( JSErrorHandlerFrame* )
        {
            // We had an error down there somewhere
            //
            result_OK = false;
            }
        }

    if ( result_OK && global_f )
    {
        try
        {
            // Execute
            //
            ExecuteCode( NULL, global_f, 0, NULL );
            }
        catch( JSErrorHandlerFrame* )
        {
            // We had an error down there somewhere
            //
            result_OK = false;
            }
        }

    // Pop all error handler frames from the handler chain
    //
    while( error_handler != saved_handler )
    {
        handler = error_handler->next;
        delete error_handler;
        error_handler = handler;
        }

    // Restore virtual machine's idea about the stack top
    //
    SP = saved_sp;

    return result_OK;
    }

bool 
JSVirtualMachine:: Apply
(
    PCSTR func_name,
    JSVariant* func,
    int argc,
    JSVariant argv []
    )
{
    // Initialize error handler
    //
    JSErrorHandlerFrame* handler = NEW JSErrorHandlerFrame( error_handler );
    if ( handler == NULL )
    {
        strcpy( error, "VM: out of memory" );
        return 0;
        }

    JSErrorHandlerFrame* saved_handler = error_handler;
    error_handler = handler;

    bool result_OK = true;
    JSVariant* saved_sp = SP;

    try
    {
        // Clear error message and old exec result
        //
        error[ 0 ] = '\0';
        exec_result.type = JS_UNDEFINED;

        if ( func_name )
        {
            // Lookup the function
            //
            func = Intern( func_name );
            }

        // Check what kind of function should be called
        //
        if ( func->type == JS_FUNCTION )
        {
            // Call function
            //
            if ( options.verbose > 1 )
            {
                s_stderr->PrintfLn( "VM: calling function" );
                }

            ExecuteCode( NULL, func->vfunction, argc, argv );
            }
        else if ( func->type == JS_BUILTIN )
        {
            func->vbuiltin->info->OnGlobalMethod
            (
                func->vbuiltin->instance_context, // void* instance context
                exec_result,                      // JSVariant& result return
                argv                              // JSVariant[] argumetns
                );
            }
        else
        {
            if ( func_name )
            {
                sprintf( error, "Apply: undefined function `%s'", func_name );
                }
            else
            {
                strcpy( error, "Apply: undefined function" );
                }

            result_OK = false;
            }
        }
    catch( JSErrorHandlerFrame* )
    {
        // An error occurred
        //
        result_OK = false;
        }

    // Pop all error handler frames from the handler chain
    //
    while( error_handler != saved_handler )
    {
        handler = error_handler->next;
        delete error_handler;
        error_handler = handler;
        }

    // Restore virtual machine's idea about the stack top
    //
    SP = saved_sp;

    return result_OK;
    }

bool
JSVirtualMachine:: CallMethod
(
    JSVariant* object,
    PCSTR method_name,
    int argc,
    JSVariant argv []
    )
{
    // Initialize error handler
    //
    JSErrorHandlerFrame* handler = NEW JSErrorHandlerFrame( error_handler );
    if ( handler == NULL )
    {
        strcpy( error, "VM: out of memory" );
        return 0;
        }

    JSErrorHandlerFrame* saved_handler = error_handler;
    error_handler = handler;

    bool result_OK = true;
    JSSymbol symbol;
    JSVariant* saved_sp = SP;

    try
    {
        // Intern the method name
        //
        symbol = Intern( method_name );

        // Clear error message and old exec result
        //
        error[ 0 ] = '\0';
        exec_result.type = JS_UNDEFINED;

        // What kind of object was called?
        //
        if ( object->type == JS_BUILTIN )
        {
            JSPropertyRC rc = object->vbuiltin->info->OnMethod
            (
                object->vbuiltin->instance_context, // void* instance context
                symbol,                             // JSSymbol method
                exec_result,                        // JSVariant& result return
                argv                                // JSVariant[] arguments
                );

            if ( JS_PROPERTY_UNKNOWN == rc )
            {
                strcpy( error, "CallMethod: unknown method" );
                result_OK = false;
                }
            }
        else if ( object->type == JS_OBJECT )
        {
            JSVariant method;

            // Fetch the method's implementation
            //
            JSPropertyRC rc = object->vobject->LoadProperty( symbol, method );

            if ( JS_PROPERTY_FOUND == rc )
            {
                // The property has been defined in the object
                //
                if ( method.type != JS_FUNCTION )
                {
                    strcpy( error, "CallMethod: unknown method" );
                    result_OK = false;
                    }
                else
                {
                    if ( options.verbose > 1 )
                    {
                        s_stderr->PrintfLn( "VM: calling function" );
                        }

                    ExecuteCode( object, method.vfunction, argc, argv );
                    }
                }
            else
            {
                // Let the built-in Object handle this
                //
                JSPropertyRC rc = prim[ object->type ]->OnMethod
                (
                    object,       // void* instance context
                    symbol,       // JSSymbol method
                    exec_result,  // JSVariant& result return
                    argv          // JSVariant[] arguments
                    );

                if ( JS_PROPERTY_UNKNOWN == rc )
                {
                    strcpy( error, "CallMethod: unknown method" );
                    result_OK = false;
                    }
                }
            }
        else if ( prim[ object->type ] )
        {
            // The primitive language types
            //
            JSPropertyRC rc = prim[ object->type ]->OnMethod
            (
                object,       // void* instance context
                symbol,       // JSSymbol method
                exec_result,  // JSVariant& result return
                argv          // JSVariant[] arguments
                );

            if ( JS_PROPERTY_UNKNOWN == rc )
            {
                strcpy( error, "CallMethod: unknown method" );
                result_OK = false;
                }
            }
        else
        {
            strcpy( error, "CallMethod: illegal object" );
            result_OK = false;
            }
        }
    catch( JSErrorHandlerFrame* )
    {
        // An error occurred
        //
        result_OK = false;
        }

    // Pop all error handler frames from the handler chain
    //
    while ( error_handler != saved_handler )
    {
        handler = error_handler->next;
        delete error_handler;
        error_handler = handler;
        }

    // Restore virtual machine's idea about the stack top
    //
    SP = saved_sp;

    return result_OK;
    }

/////////////////////
// Modules
//

bool
JSVirtualMachine:: DefineModule( JSModuleInitProc module_init_proc )
{
    bool result_OK = true;

    // Just call the init proc in a toplevel.

    // Top-level error handler
    //
    JSErrorHandlerFrame handler( error_handler );
    error_handler = &handler;

    try
    {
        // Call the module init proc
        //
        module_init_proc( this );
        }
    catch( JSErrorHandlerFrame* )
    {
        // An error occurred
        //
        result_OK = false;
        }

    // Pop the error handler
    //
    assert( error_handler == &handler );
    error_handler = error_handler->next;

    return result_OK;
    }


