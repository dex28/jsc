#ifndef _JS_H_INCLUDED
#define _JS_H_INCLUDED

////////////////////////////////////////////
// JavaScript Virtual Machine include file.
//

// We have always JSconfig.h
//
#include "JSconfig.h"

// Standard headers
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <float.h>

// Misc system headers
//
#include <sys/types.h>

//#pragma warning( disable: 4100 ) // MSVC60: unreferenced formal parameter
//#pragma warning( disable: 4514 ) // MSVC60: unreferenced inline function has been removed

#undef DllDecl

#ifdef LIBJS_IMPLEMENTATION
#define DllDecl __declspec(dllexport)
#else
#define DllDecl
#endif

#ifdef JS_DEBUG_NEW

// Memory allocation routines.
//
extern void* operator new( size_t, const char* filename, int linenum );
extern void operator delete( void* ptr );
extern void operator delete( void* ptr, const char* filename, int linenum );
extern void* jsReallocDbg( void* ptr, size_t size, const char* filename, int linenum );

static inline char*
jsStrDupDbg( const char* str, const char* filename, int linenum )
{
    size_t len = strlen( str );
    char* cp = new(filename,linenum) char[ len + 1 ];
    memcpy( cp, str, len );

    return cp;
    }

#define jsRealloc(p,s) jsReallocDbg(p,s,__FILE__,__LINE__)
#define jsStrDup(p) jsStrDupDbg(p,__FILE__,__LINE__)
#define NEW new(__FILE__,__LINE__)

// We don't want new to be used in its native form. Use NEW instead !
//
inline void* operator new( size_t ) { assert( 0 ); return NULL; };

#else

#define NEW new

static inline char*
jsStrDup( const char* str )
{
    size_t len = strlen( str );
    char* cp = new char[ len + 1 ];
    memcpy( cp, str, len );

    return cp;
    }

extern void* jsRealloc( void* ptr, size_t size );

#endif

//////////////////////////
// Type definitions
//

typedef char* PSTR;
typedef const char* PCSTR;
typedef unsigned char uint8;
typedef unsigned int uint;

// Integer types
//
typedef unsigned char JSUInt8;
typedef signed char JSInt8;

typedef unsigned short JSUInt16;
typedef short JSInt16;

typedef unsigned long JSUInt32;
typedef long JSInt32;

// Here defined classes
//
class JSHeapObjectToClean;
class JSBuiltinInfo; 
class JSBuiltinInfo_GM ;
class JSBuiltin;
class JSString;
class JSArray;
class JSFunction;
class JSObject;
class JSVariant;
class JSVirtualMachine;
class JSClass;
class JSByteCode;
class JSIOStream;
class JSIOStreamFile;
class JSIOStreamPipe;
class JSIOStreamIOFunc;
class DIR;
class JSDynaLib;
class JSConext;

// Interned symbol index
//
typedef JSVariant* JSSymbol;

// Some portability features.
//
#ifdef WIN32

const char JS_ENDLINE [] = "\r\n";
const int JS_ENDLINE_LEN = 2;

#else // not WIN32

const char JS_ENDLINE [] = "\n";
const int JS_ENDLINE_LEN = 1;

#endif // WIN32

// Constants
//

// Noticeable virtual machine events. The JS_VM_EVENT_OPERAND_COUNT
// event is generated only if the interpreter was configured with the
// 'enable operand hooks' option.
//
enum
{
    JS_VM_EVENT_OPERAND_COUNT     = 1,
    JS_VM_EVENT_GARBAGE_COLLECT   = 2
    };

///////////////////////////////////////////////////////////////////////////////
// I/O streams
//

class DllDecl JSIOStream
{
protected:

    // Buffer filler and flusher functions
    //
    virtual size_t OnRead( void* ptr, size_t size ) = 0;
    virtual size_t OnWrite( const void* ptr, size_t size ) = 0;
    virtual int OnSeek( long offset, int whence ) = 0;
    virtual long OnGetPosition( void ) = 0;
    virtual long OnGetLength( void ) = 0;

    void FillBuffer( void );

    uint8* buffer;
    int buflen;
    int data_in_buf;
    int bufpos;

    // Flags
    //
    int fCanWrite;
    int fCanRead;

    int at_eof;
    int autoflush;
    int writep;      // Does the buffer contain data to write?

    // The system error code for the last operation that failed
    //
    int error;

public:

    JSIOStream( void );
    virtual ~JSIOStream( void ) = 0;

    static int IsValid( JSIOStream* ios )
        { return ios != NULL && ios->buffer != NULL; }

    void SetBufferSize( size_t new_buflen );
    int GetBufferSize( void ) { return buflen; }

    int GetError( void ) { return error; }
    void ClearError( void ) { error = 0; }

    int Flush( void );
    void SetAutoFlush( int fValue = 1 ) { autoflush = ( fValue != 0 ); }
    int GetAutoFlush( void ) { return autoflush != 0; }

    int AtEOF( void ) { return at_eof != 0; }
    int Seek( long offset, int whence );
    long GetPosition( void );
    long GetLength ( void );

    int ReadByte( void );
    int Unget( int byte );

    size_t Read( void* ptr, size_t size );
    size_t Write( const void* ptr, size_t size );

    void Printf( char* fmt ... );
    void PrintfLn( char* fmt = NULL, ... );
    };

class JSIOStreamFile : public JSIOStream
{
    virtual size_t OnRead( void* ptr, size_t size );
    virtual size_t OnWrite( const void* ptr, size_t size );
    virtual int OnSeek( long offset, int whence );
    virtual long OnGetPosition( void );
    virtual long OnGetLength( void );

protected:

    FILE* pFile;
    int fDoClose;

public:

    JSIOStreamFile( FILE* fp, int readp = 1, int writep = 0, int do_close = 0 );
    JSIOStreamFile( const char* filename, int readp = 1, int writep = 0 );
    virtual ~JSIOStreamFile( void );
    };

class JSIOStreamPipe : public JSIOStreamFile
{
    int fDoClose;

public:

    JSIOStreamPipe( FILE* fp, int readp, int do_close = 1 );
    virtual ~JSIOStreamPipe( void );
    };

// An I/O function that can be set to interpreter options to redirect
// the system's default streams.
//
typedef size_t (*JSIOFunc) ( void* context, void* buffer, size_t amount );

class DllDecl JSIOStreamIOFunc : public JSIOStream
{
    JSIOFunc func;
    void* pContext;
    long position;

    virtual size_t OnRead( void* ptr, size_t size );
    virtual size_t OnWrite( const void* ptr, size_t size );
    virtual int OnSeek( long offset, int whence );
    virtual long OnGetPosition( void );
    virtual long OnGetLength( void );

public:

    JSIOStreamIOFunc( JSIOFunc func, void* context, int readp, int writep );
    virtual ~JSIOStreamIOFunc( void );
    };

///////////////////////////////////////////////////////////////////////////////
// Directory handling.
//
class DIR
{
    PSTR szPath;
    PSTR szDirName;
    long handle;

    // The position in the directory
    unsigned int pos;

public:

    DIR( PCSTR name );
    PSTR Read( void );
    int Close( void );
    void Rewind( void );
    void Seek ( long offset );
    long Tell( void );
    };

///////////////////////////////////////////////////////////////////////////////
// ByteCode
//

// JSVM Instruction opcodes
//
enum JSOpCode
{
    JS_OPCODE_NOP                  =  0,
    JS_OPCODE_DUP                  =  1,
    JS_OPCODE_POP                  =  2,
    JS_OPCODE_POP_N                =  3,
    JS_OPCODE_SWAP                 =  4,
    JS_OPCODE_ROLL                 =  5,
    JS_OPCODE_CONST                =  6,
    JS_OPCODE_CONST_NULL           =  7,
    JS_OPCODE_CONST_UNDEFINED      =  8,
    JS_OPCODE_CONST_TRUE           =  9,
    JS_OPCODE_CONST_FALSE          = 10,
    JS_OPCODE_CONST_I0             = 11,
    JS_OPCODE_CONST_I1             = 12,
    JS_OPCODE_CONST_I2             = 13,
    JS_OPCODE_CONST_I3             = 14,
    JS_OPCODE_CONST_I              = 15,
    JS_OPCODE_LOCALS               = 16,
    JS_OPCODE_APOP                 = 17,
    JS_OPCODE_MIN_ARGS             = 18,
    JS_OPCODE_LOAD_ARG             = 19,
    JS_OPCODE_STORE_ARG            = 20,
    JS_OPCODE_LOAD_NTH_ARG         = 21,
    JS_OPCODE_LOAD_GLOBAL          = 22,
    JS_OPCODE_LOAD_GLOBAL_W        = 23,
    JS_OPCODE_STORE_GLOBAL         = 24,
    JS_OPCODE_LOAD_LOCAL           = 25,
    JS_OPCODE_STORE_LOCAL          = 26,
    JS_OPCODE_LOAD_PROPERTY        = 27,
    JS_OPCODE_STORE_PROPERTY       = 28,
    JS_OPCODE_DELETE_PROPERTY      = 29,
    JS_OPCODE_LOAD_ARRAY           = 30,
    JS_OPCODE_STORE_ARRAY          = 31,
    JS_OPCODE_DELETE_ARRAY         = 32,
    JS_OPCODE_NTH                  = 33,
    JS_OPCODE_CMP_EQ               = 34,
    JS_OPCODE_CMP_NE               = 35,
    JS_OPCODE_CMP_LT               = 36,
    JS_OPCODE_CMP_GT               = 37,
    JS_OPCODE_CMP_LE               = 38,
    JS_OPCODE_CMP_GE               = 39,
    JS_OPCODE_CMP_SEQ              = 40,
    JS_OPCODE_CMP_SNE              = 41,
    JS_OPCODE_ADD_1_I              = 42,
    JS_OPCODE_ADD_2_I              = 43,
    JS_OPCODE_SUB                  = 44,
    JS_OPCODE_ADD                  = 45,
    JS_OPCODE_MUL                  = 46,
    JS_OPCODE_DIV                  = 47,
    JS_OPCODE_MOD                  = 48,
    JS_OPCODE_NEG                  = 49,
    JS_OPCODE_NOT                  = 50,
    JS_OPCODE_AND                  = 51,
    JS_OPCODE_OR                   = 52,
    JS_OPCODE_XOR                  = 53,
    JS_OPCODE_SHIFT_LEFT           = 54,
    JS_OPCODE_SHIFT_RIGHT          = 55,
    JS_OPCODE_SHIFT_RRIGHT         = 56,
    JS_OPCODE_HALT                 = 57,
    JS_OPCODE_DONE                 = 58,
    JS_OPCODE_IFFALSE              = 59,
    JS_OPCODE_IFTRUE               = 60,
    JS_OPCODE_IFFALSE_B            = 61,
    JS_OPCODE_IFTRUE_B             = 62,
    JS_OPCODE_CALL_METHOD          = 63,
    JS_OPCODE_JMP                  = 64,
    JS_OPCODE_JSR                  = 65,
    JS_OPCODE_JSR_W                = 66,
    JS_OPCODE_RETURN               = 67,
    JS_OPCODE_TYPEOF               = 68,
    JS_OPCODE_NEW                  = 69,
    JS_OPCODE_WITH_PUSH            = 70,
    JS_OPCODE_WITH_POP             = 71,
    JS_OPCODE_TRY_PUSH             = 72,
    JS_OPCODE_TRY_POP              = 73,
    JS_OPCODE_THROW                = 74,
    JS_OPCODE_LAST_OPCODE          = 74, // last valid opcode emitted in bytecode
    JS_OPCODE_SYMBOL               = 75, // internal NOP; not bytecode emited
    JS_OPCODE_LABEL                = 76  // internal NOP; not bytecode emited
    };

enum JSOpArgType
{
    JS_OPARG_none,
    JS_OPARG_INT32,
    JS_OPARG_CONST,
    JS_OPARG_SYMBOL,
    JS_OPARG_LABEL,

    JS_OPARG_INT8,
    JS_OPARG_INT16,
    };

struct JSOpcodeDesc
{
    PCSTR desc;
    JSOpArgType arg_type;
    int stack_delta;
    };

union Compiled
{
    JSOpCode   op;
    JSInt32    i32;
    JSInt32    symbol;
    Compiled*  label;
    JSVariant* variant;
    };

struct DebugInfo
{
    Compiled* pc;
    int linenum;
    };

class DllDecl JSByteCode
{
public:

    struct Entry
    {
        PSTR name;
        Compiled* code;
        long code_len;
        DebugInfo* dbginfo;
        long dbginfo_count;
        };

    Compiled* code;
    long code_len;

    JSVariant* consts;
    long const_count;

    PSTR dbg_filename;
    DebugInfo* dbginfo;
    long dbginfo_count;

    PSTR* i_symb;
    long i_symb_count;

    Entry* e_symb;
    long e_symb_count;

    volatile long usage_count;

    JSByteCode( void );
    bool Load( PCSTR filename );
    bool Save( PCSTR filename );
    ~JSByteCode( void );
    void Dump( void );

    void incrementRefCount( void ) { usage_count ++; }
    void decrementRefCount( void ) { usage_count --; }
    };

extern const JSOpcodeDesc OpcodeDesc[];

///////////////////////////////////////////////////////////////////////////////
// Pseudo-random number generator
//
class DllDecl RAND48
{
    unsigned int x[ 3 ]; // holds 48-bit integer
    unsigned int a[ 3 ]; // holds 48-bit integer
    unsigned int c;

public:

    RAND48( void );
    void Seed( unsigned long seedval );
    double Random( void );
    };

///////////////////////////////////////////////////////////////////////////////
// Pure abstract class of all objects that require spcefic cleanup before GC.
//
class DllDecl JSHeapObjectToClean
{
public:

    JSVirtualMachine* vm; // Initialized by operator new !

    virtual void OnFinalize( void ) = 0;
    void* operator new( size_t, JSVirtualMachine* vm );
    void operator delete( void* ptr, JSVirtualMachine* vm ); // needed only on exception
    };

///////////////////////////////////////////////////////////////////////////////
// BuiltinInfo class
//

// Registry information for builtin objects properties and methods.
//
enum JSPropertyRC
{
    JS_PROPERTY_UNKNOWN = 0,
    JS_PROPERTY_FOUND   = 1
    };

class DllDecl JSBuiltinInfo : public JSHeapObjectToClean
{
public:

    PCSTR szName;
    JSObject* prototype;

    JSBuiltinInfo( PCSTR szName = NULL );

    virtual void OnFinalize( void );

    virtual void OnGlobalMethod ( void* instance_context,
        JSVariant& result_return, JSVariant args [] );

    // Function to call method <method> from the object.
    //
    virtual JSPropertyRC OnMethod ( void* instance_context,
        JSSymbol method, JSVariant& result_return, JSVariant args [] );

    // Function to load and set property <property> of object. If <set>
    // is true, property <property> should be set to value <variant>. Otherwise
    // function should return the value of property <property> in <variant>.
    //
    virtual JSPropertyRC OnProperty ( void* instance_context,
        JSSymbol property, bool set, JSVariant& variant ); // TODO make 'set' boolean

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );

    virtual void OnFinalize ( void* instance_context );

    virtual void OnMark ( void* instance_context );
    };

// BuiltinInfo internal global method class
//
class DllDecl JSBuiltinInfo_GM : public JSBuiltinInfo
{
public:

    typedef void (*GlobalMethodProc)
    (
        JSVirtualMachine* vm,
        void* instance_context,
        JSVariant& result_return,
        JSVariant args []
        );

    GlobalMethodProc globalMethod;

    JSBuiltinInfo_GM( PSTR szName, GlobalMethodProc proc, void* instance_context = NULL );

    virtual void OnGlobalMethod( void* instance_context, JSVariant& result_return, JSVariant args [] );
    };

///////////////////////////////////////////////////////////////////////////////
// Builtin class
//
class DllDecl JSBuiltin : public JSHeapObjectToClean
{
public:

    JSObject* prototype;
    JSBuiltinInfo* info;
    void* instance_context;

    JSBuiltin( JSBuiltinInfo* info, void* instance_context = NULL );
    virtual void OnFinalize( void );
    };

///////////////////////////////////////////////////////////////////////////////
// String
//
enum
{
    JSSTRING_NORMAL  = 0x00,
    JSSTRING_STATIC  = 0x01,
    JSSTRING_DONT_GC = 0x02
    };

class JSString
{
public:

    long len;
    PSTR data;

    uint flags;

    JSObject* prototype;
    };

///////////////////////////////////////////////////////////////////////////////
// Array
//
class JSArray
{
public:

    long length;
    JSVariant* data;

    JSObject* prototype;
    };

///////////////////////////////////////////////////////////////////////////////
// Function
//
class JSFunction : public JSHeapObjectToClean
{
public:

    PCSTR name;

    Compiled* code;
    long code_len;

    PCSTR dbg_filename;
    DebugInfo* dbginfo;
    long dbginfo_count;

    JSSymbol* linkage; // linkage of imported-symbols
    JSObject* prototype;

    JSFunction( JSByteCode* bc, long i, JSSymbol* arg_linkage )
    {
        name = bc->e_symb[ i ].name;
        code = bc->e_symb[ i ].code;
        code_len = bc->e_symb[ i ].code_len;
        dbg_filename = bc->dbg_filename;
        dbginfo = bc->e_symb[ i ].dbginfo;
        dbginfo_count = bc->e_symb[ i ].dbginfo_count;

        linkage = arg_linkage;
        prototype = NULL;
        }

    virtual void OnFinalize( void ) {}
    };

///////////////////////////////////////////////////////////////////////////////
// Variant
//
enum JSVarType
{
    JS_UNDEFINED  = 0,
    JS_NULL       = 1,
    JS_BOOLEAN    = 2,
    JS_INTEGER    = 3,
    JS_NAN        = 4,
    JS_FLOAT      = 5,
    JS_STRING     = 6,
    JS_ARRAY      = 7,
    JS_OBJECT     = 8,
    JS_FUNCTION   = 9,
    JS_BUILTIN    = 10,
    JS_INSTR_PTR  = 11,
    JS_BASE_PTR   = 12,
    JS_FRAME_PTR  = 13,
    JS_WITH_CHAIN = 14,
    JS_ARGS_FIX   = 15
    };

class DllDecl JSVariant
{
public:

    JSVarType type;

    union
    {
        int vboolean;
        long vinteger;
        double vfloat;

        JSString* vstring;
        JSObject* vobject;
        JSArray* varray;

        JSBuiltin* vbuiltin;
        JSFunction* vfunction;

        // Internal values used mostly for stack frame maintenance
        //
        Compiled* instr_ptr;
        JSSymbol* base_ptr;
        JSVariant* frame_ptr;

        struct
        {
            JSInt32 cNode;
            JSVariant* node;
            } with_chain;

        struct
        {
            JSInt32 argc;
            JSInt32 delta;
            } args_fix;
        };

    void Mark( void );

    void MakeArray( JSVirtualMachine* vm, long length );

    void ExpandArray( JSVirtualMachine* vm, long length );

    void MakeString( JSVirtualMachine* vm, PCSTR data, int data_len = -1 );

    void MakeStaticString( JSVirtualMachine* vm, PCSTR data, int data_len = -1 );

    void MakePositiveInfinity( void )
        { type = JS_FLOAT; vfloat = HUGE_VAL; }

    void MakeNegativeInfinity( void )
        { type = JS_FLOAT; vfloat = -HUGE_VAL; }

    void operator = ( JSBuiltin* ptr )
        { type = JS_BUILTIN; vbuiltin = ptr; }

    void operator = ( JSFunction* f )
        { type = JS_FUNCTION; vfunction = f; }

    int IsPrimitive( void )
        { return type == JS_UNDEFINED || type == JS_NULL || type == JS_BOOLEAN
                 || type == JS_INTEGER || type == JS_FLOAT || type == JS_NAN
                 || type == JS_STRING; }

    int IsNumber( void )
        { return type == JS_INTEGER || type == JS_FLOAT || type == JS_NAN; }

    int IsPositiveInfinity( void )
        { return type == JS_FLOAT && vfloat == HUGE_VAL; }

    int IsNegativeInfinity( void )
        { return type == JS_FLOAT && vfloat == -HUGE_VAL; }

    int IsFinite( void )
        { return ! IsPositiveInfinity () && ! IsNegativeInfinity () && type != JS_NAN; }

    int IsTrue( void )
    {
        return type > JS_INTEGER
               || ( type == JS_BOOLEAN && vboolean )
               || ( type == JS_INTEGER && vinteger );
        }

    int IsFalse( void )
    {
        return type < JS_BOOLEAN
               || ( type == JS_BOOLEAN && ! vboolean )
               || ( type == JS_INTEGER && ! vinteger );
        }

    int StringCompare( JSVariant& b )
    {
        assert( type == JS_STRING );
        assert( b.type == JS_STRING );

        for ( int i = 0; i < vstring->len && i < b.vstring->len; i++)
        {
            if ( vstring->data[ i ] < b.vstring->data[ i ] )
                return -1;

            if ( vstring->data[ i ] > b.vstring->data[ i ] )
                return 1;
            }

        if ( vstring->len < b.vstring->len )
            return -1;

        if ( vstring->len > b.vstring->len )
            return 1;

        return 0;
        }

    // Convert variant to its primitive with a given hint as a preffered type
    // return the result in <result_return>.
    //
    void ToPrimitive( JSVirtualMachine* vm, JSVariant& result_return,
                   JSVarType preffered_type = JS_INTEGER );

    // Convert variant to its string presentations and
    // return the result in <result_return>.
    //
    void ToString( JSVirtualMachine* vm, JSVariant& result_return );

    // Convert variant to object according to its type.
    //
    void ToObject( JSVirtualMachine* vm, JSVariant& result_return );

    // Convert variant to its number presentations and
    // return the result in <result_return>.
    //
    void ToNumber( JSVariant& result_return );

    // Convert variant to its signed 32 bit integer
    // presentation and return the result.
    //
    JSInt32 ToInt32( void );

    // Convert variant to a boolean value and return the result.
    //
    int ToBoolean( void );

    // Convert JS_STRING type variant
    // to zero terminated C-type string allocated on heap.
    //
    PSTR ToNewCString( void )
    {
        assert( type == JS_STRING );

        PSTR cp = NEW char[ vstring->len + 1 ];
        memcpy( cp, vstring->data, vstring->len );
        cp[ vstring->len ] = '\0';

        return cp;
        }
    };

///////////////////////////////////////////////////////////////////////////////
// Object
//

// The attribute flags for object's properties
//
enum
{
    JS_ATTRIB_READONLY      = 0x01,
    JS_ATTRIB_DONTENUM      = 0x02,
    JS_ATTRIB_DONTDELETE    = 0x04,
    JS_ATTRIB_Internal      = 0x08
    };

class DllDecl JSObject
{
    // Hash variant for object's properties
    //
    struct HashBucket
    {
        HashBucket* pNext;
        uint8* data;
        long len;
        uint value;
        };

    // List of hash buckets
    //
    struct HashBucketList
    {
        HashBucket* pFirst;
        long length;
        };

    // Object's property
    //
    struct Property
    {
        JSSymbol name;
        JSVariant value;
        uint attributes;
        };

    JSVirtualMachine* vm; // Initialized with operator new, not constructor.

    HashBucketList* pHash;  // hashed object properties
    int cHash;              // hash size

    Property* pProps;    // Object properties
    int num_props;       // Number of properties in this object

    void HashCreate( void );
    void HashInsert( PCSTR name, int name_len, int pos );
    void HashDelete( PCSTR name, int name_len);
    int HashLookup( PCSTR name, int name_len );

public:

    void* operator new( size_t, JSVirtualMachine* vm );
    void operator delete( void* ptr, JSVirtualMachine* vm );

    JSObject( void );

    void Mark( void );

    JSPropertyRC LoadProperty( JSSymbol prop, JSVariant& value_return );
    void StoreProperty( JSSymbol prop, JSVariant& value );
    void DeleteProperty( JSSymbol prop );

    void LoadArray( JSVariant& sel, JSVariant& value_return );
    void StoreArray( JSVariant& sel, JSVariant& value );
    void DeleteArray( JSVariant& sel );

    int Nth ( int nth, JSVariant& value_return );
    };

///////////////////////////////////////////////////////////////////////////////
// Datatypes used by modules
//

// Function type for JS modules initialization procedures.
//
typedef void (*JSModuleInitProc)( JSVirtualMachine* interp );

// Function type to free client data.
//
class DllDecl JSContext
{
public:
    virtual void OnFinalize( void ) {}
    };

// Function type for global methods.
//
typedef void (*JSGlobalMethodProc) ( JSContext* context,
                                     JSVirtualMachine* interp,
                                     int argc, JSVariant argv [],
                                     JSVariant& result_return );

///////////////////////////////////////////////////////////////////////////////
// Interpreter options
//

// Hook that is called at certain points during the byte-code execution.
//
enum
{
    JS_EVENT_OPERAND_COUNT   = 1,
    JS_EVENT_GARBAGE_COLLECT = 2
    };

class JSEventHook
{
public:
    virtual int callback( int event ) = 0;
    };

class DllDecl JSVMOptions
{
public:

    int stack_size;

    int verbose; // verbosity with different levels; 0 = no verbosity
    int verbose_stacktrace;

    // Virtual machine flags
    //
    int stacktrace_on_error;
    int warn_undef;
    int secure_builtin_file;
    int secure_builtin_system;

    // The standard system streams.  If these are NULL, the streams are
    // bind to the system's stdin, stdout, and stderr files.
    //
    JSIOFunc s_stdin;
    JSIOFunc s_stdout;
    JSIOFunc s_stderr;
    void* s_context;

    // The callback hook.
    //
    JSEventHook* event_hook;

    // Call the <hook> after the virtual machine has executed this many
    // opcodes.
    //
    int hook_operand_count_trigger;

    // Collect garbage if we allocated more than gc_trigger bytes of
    // memory since the last gc.
    //
    size_t gc_trigger;

    // Initialize interpreter options to system's default values.
    //
    JSVMOptions( void );
    };

///////////////////////////////////////////////////////////////////////////////
// Virtual Machine
//
const int JS_NUM_HEAP_FREELISTS = 20;
const int JS_HASH_TABLE_SIZE = 256;

class DllDecl JSVirtualMachine
{
public:

    // Options for the virtual machine
    //
    JSVMOptions options;

    // Pseudo-random generator context
    //
    RAND48 Rand48ctx;

    // Perimition to run:
    // 0 = run; 1 = raise exception; 2 = kill VM
    //
    volatile int should_terminate;

    // The default system streams
    //
    JSIOStream* s_stdin;
    JSIOStream* s_stdout;
    JSIOStream* s_stderr;

    // Global symbols (both functions and variables).  <globals_hash> is
    // a name-to-index mapping between symbol names and their positions
    // in <globals>.
    //
    struct HashBucket
    {
        HashBucket* next;
        PSTR name;
        JSSymbol vsymbol;
        };

    JSVariant* globals;
    int num_globals;
    int globals_alloc;
    HashBucket* globals_hash[ JS_HASH_TABLE_SIZE ];

    // The next anonymous function id.
    //
    int anonymous_function_next_id;

    // Stack
    //
    JSVariant* stack;
    int stack_size;

    JSVariant* SP;
    Compiled* PC;
    JSSymbol* BP;
	JSVariant* FP;

    // Builtin objects for the primitive datatypes
    //
    JSBuiltinInfo* prim[ 16 ];

    // Some commonly used symbols
    //
    JSSymbol s___proto__;
    JSSymbol s_prototype;
    JSSymbol s_toSource;
    JSSymbol s_toString;
    JSSymbol s_valueOf;
    JSSymbol s_undefined;
    JSSymbol s___Nth__;

    // Bytecode imported symbols' linkage
    //
    struct SymbLinkage
    {
        SymbLinkage* next;
        JSSymbol* linkage;
        JSByteCode* bc;
        JSFunction* global_f;
        };

    // List of allocated relocation linkages of imported symbols
    //
    SymbLinkage* pSymbLinkage;

    // Heap block
    //
    struct JSHeapBlock
    {
        JSHeapBlock* next;
        long size;
        // <size> bytes of data follows the structure
        };

    // All allocated blocks have this header
    //
    struct JSHeapMemoryBlock
    {
#ifdef JS_DEBUG_NEW
        uint magic;
#endif

        uint flag_mark : 1;
        uint flag_needs_finalize : 1;
        uint size : 30;
        // <size> bytes of data follows this header
        };

    // When the block is on the freelist, it has this header.  The first
    // sizeof( void* ) bytes of the block's data is used to hold the
    // freelist next pointer.
    //
    struct JSHeapFreelistBlock : public JSHeapMemoryBlock
    {
        JSHeapMemoryBlock* next;
        };

    JSHeapBlock* heap;
    JSHeapMemoryBlock* heap_freelists[ JS_NUM_HEAP_FREELISTS ];
    unsigned long heap_size;

    // Information for the garbage collector
    //
    struct
    {
        unsigned long bytes_allocated;
        unsigned long bytes_free;
        unsigned long count;
        } gc;

    // Error handler frame
    //
    struct JSErrorHandlerFrame
    {
        JSErrorHandlerFrame* next;

        // The value thrown by the throw operand
        //
        JSVariant thrown;

        // Saved state for the `try_push' operand
        //
        JSVariant* SP;
        JSVariant* FP;
        Compiled* PC;
        JSSymbol* BP;

        // Construct Top-level frame
        //
        JSErrorHandlerFrame:: JSErrorHandlerFrame
        (
            JSErrorHandlerFrame* error_handler
            )
        {
            next = error_handler;
            SP = NULL;
            FP = NULL;
            PC = NULL;
            BP = NULL;
            }

        // Construct VM runtime frame
        //
        JSErrorHandlerFrame:: JSErrorHandlerFrame
        (
            JSErrorHandlerFrame* error_handler,
            JSVariant* sp,
            JSVariant* fp,
            Compiled* next_pc,
            JSSymbol* bp
            )
        {
            next = error_handler;
            SP = sp;
            FP = fp;
            PC = next_pc;
            BP = bp;
            }
        };

    JSErrorHandlerFrame* error_handler;

    // Buffer for the error message. Sorry, we don't support long errors ;-)
    //
    char error[ 1024 ];

    // The result from the latest evaluation. This is set when the
    // vm->Execute(), vm->Apply() or vm->CallMethod() functions
    // return to the caller.
    //
    JSVariant exec_result;

#ifdef PROFILING

    // Byte-code operand profiling support
    //
    long prof_count[ 256 ];
    __int64 prof_elapsed[ 256 ];

#endif // PROFILING

    // Raise an error. The error message must have been saved to vm->error
    // before this function is called. The FUNCTION NEVER RETURNS.
    //
    void RaiseError( void );

    // Raise an error. The error message is formatted into vm->error.
    // The FUNCTION NEVER RETURNS.
    //
    void RaiseError( PCSTR fmt ... );

    // Execute byte code contained in function <f> applying arguments
    // <argc, argv>.
    //
    void ExecuteCode( JSVariant* object, JSFunction* f, int argc, JSVariant argv [] );

    // Debug stack contents
    //
    void StackTrace( int num_frames = INT_MAX );
    void StackTrace( JSVariant* fp, JSVariant* fpEnd, Compiled* pc );

    // Retrieve the function from the program counter value
    //
    JSFunction* FunctionAt( Compiled* pc );

    // Map program counter to the source file line
    //
    JSFunction* DebugPosition( Compiled* pc, PCSTR& fname_return, int& linenum_return );

    // Intern symbol <name> to virtual machine and return its JSSymbol id
    //
    JSSymbol Intern( PCSTR name, int name_len = -1 ); // -1 stands for auto strlen

    // Return the name of symbol <sym>
    //
    PCSTR Symname( JSSymbol sym );

    // Builtin initialization
    //
    void BuiltinArray( void );
    void BuiltinBoolean( void );
    void BuiltinFunction( void );
    void BuiltinNumber( void );
    void BuiltinObject( void );
    void BuiltinString( void );
    void BuiltinCoreGM( void );

    void BuiltinDate( void );
    void BuiltinDirectory( void );
    void BuiltinFile( void );
    void BuiltinMath( void );
    void BuiltinSystem( void );
    void BuiltinVM( void );

    void BuiltinMD5( void );
    void BuiltinSQL( void );

    // Enter file to the system
    //
    void FileNew( JSVariant& result_return, PCSTR path, JSIOStream* stream, bool dont_close );

    // Virtual machine heap handling; heap has Garbage Collection policy
    //

    // Allocate on VM heap; does not zeroize memory
    // If fHeapObjectToClean is true, then we allocate JSHeapObjectToClean type
    // and arena is zeroized; otherwise, allocated area contents is random.
    //
    void* Alloc( size_t size, bool fHeapObject = false );

    void* Realloc( void* ptr, size_t new_size );

    void Free( void* ptr );

    void GarbageCollect( JSVariant* fp, JSVariant* sp );

    unsigned long ClearHeap( void );

    void
    StrCat( JSVariant& n, PCSTR str, long len )
    {
        n.vstring->data = PSTR( Realloc( n.vstring->data, n.vstring->len + len ) );
        memcpy( n.vstring->data + n.vstring->len, str, len );
        n.vstring->len += len;
        }

    static int MarkPtr( void* ptr );

    static int IsMarkedPtr( void* ptr );

//
// JavaScript interpreter methods
//

    // Return a string that describes the JavaScript interpreter version
    // number.  The returned string is in format "MAJOR.MINOR.PATCH",
    // where MAJOR, MINOR, and PATCH are integer numbers.
    //
    static PCSTR Version ();

    // Constructor: interpeter without VM (just store options).
    //
    JSVirtualMachine( void );

    // Destructor: Destroy interpreter.
    //
    ~JSVirtualMachine( void );

    // Real constructor: Create a new JavaScript interpreter.
    //
    int Create( void );

    // Execute byte code <bc>. Function returns 'true' if the operation was
    // successful or 'false' if any errors were encountered. In case of errors,
    // the error message is stored at vm->error.
    //
    bool Execute( JSByteCode* bc );

    // Apply function <func_name> to arguments <argc, argv>. If
    // function's name <func_name> is NULL, then <func> must specify function
    // to which arguments are applied.
    //
    bool Apply( PCSTR func_name, JSVariant* func, int argc, JSVariant argv [] );

    // Call method <method_name> from object <objet> with arguments <argc, argv>.
    //
    bool CallMethod( JSVariant* object, PCSTR method_name, int argc, JSVariant argv [] );

    // Return error message from the latest error.
    //
    PCSTR ErrorMessage( void ) { return error; }

    // Post terminate message
    //
    void SetTerminate( int politeLevel );

    // Get the result of the latest evaluation or execution in interpreter
    // <interp>.  The result is returned in <result_return>.  All data,
    // returned in <result_return>, belongs to the interpreter.  The
    // caller must not modify or changed it in any ways.
    //
    void Result( JSVariant& result_return );

    // Set the value of global variable <name> to <value>.
    //
    void SetVar( PCSTR name, JSVariant& value );

    // Get the value of global variable <name> to <value>.
    //
    void GetVar( PCSTR name, JSVariant& value );

    /////////////////////////////////////////////////////////////////////
    // Modules: user extensions of JavaScript virtual machine.
    //

    // Call the module initialization function <init_proc>.  The function
    // return 1 if the module was successfully initialized or 0 otherwise.
    // In case of error, the error message can be fetched with the
    // ErrorMessage() function.
    //
    bool DefineModule( JSModuleInitProc init_proc );
    };

// Operator new that allocates on virtual machine heap.
// This objects might be freed by VM at any time without specific destructing.
// If you need objects that have specific needs for cleanup procedure
// before garbage collection, inherit JSHeapObjectToClean class.
//
inline void*
operator new( size_t size, JSVirtualMachine* vm )
{
    return vm->Alloc( size );
    }

// Dynamic loading
//
class JSDynaLib
{
    void* context;

public:

    // Try to open shared library <filename>.  If the opening was
    // successful, a handle to the library is returned.  Otherwise, the
    // function returns NULL, and an error message is returned in
    // <error_return>.  The argument <error_return_len> specifies the
    // maximum length of the error message the function should return.
    //
    JSDynaLib( PCSTR filename, PSTR error_return, int error_return_len );

    // Try to fetch the address of the symbol <symbol> from shared library
    // <library>.
    //
    void* GetSymbol( PCSTR symbol, PSTR error_return, int error_return_len );

    // Error check
    //
    operator int () { return context != NULL; }
    };

// Misc helper functions
//

// Count a hash value for <data_len> bytes of data <data>. The resulting
// hash value should be re-mapped to the correct range, for example,
// with the MOD operator.
//
static inline uint
jsHashFunction( const void* data, int data_len )
{
    uint val = 0;
    uint8* p = (uint8*) data;

    for ( int i = 0; i < data_len; i++ )
        val = (val << 5) ^ p[ i ] ^ (val >> 16) ^ (val >> 7);

    return val;
    }

// CRC32 function
//
extern JSUInt32 jsCRC32( const void* s, long len );

///////////////////////////////////////////////////////////////////////////////
// JS Compiler stuff

class DllDecl JSC_CompilerOptions
{
public:
    // general flags
    //
    int verbose;
    bool annotate_assembler;
    bool generate_debug_info;
    bool js_pages;

    // optimization flags
    //
    bool optimize_constant_folding;
    bool optimize_peephole;
    bool optimize_jumps;
    bool optimize_bc_size;
    bool optimize_heavy;

    // syntax parsing options
    //
    bool warn_unused_argument;
    bool warn_unused_variable;
    bool warn_shadow;
    bool warn_with_clobber;
    bool warn_missing_semicolon;
    bool warn_strict_ecma;
    bool warn_deprecated;

    JSIOStream* s_stdin;
    JSIOStream* s_stdout;

    JSC_CompilerOptions( void );
    };

extern DllDecl bool JSC_Compile
(
    JSC_CompilerOptions& co,
    const char* js_filename, const char* bc_filename, const char* asm_filename
    );

//////////////////////////////////////////////
// ODBC connection pooling stuff
//

extern DllDecl void JS_AllocateConnPool
(
    void
    );

extern DllDecl void JS_FreeConnPool
(
    void
    );

//////////////////////////////////////////////
// Memory leakage (reports only in debug mode)
//
extern DllDecl void jsAllocDumpBlocks( int dump_all = 0 );

#endif // _JS_H_INCLUDED
