//
// Memory allocation routines.
//

#include "JS.h"

#ifdef JS_DEBUG_NEW

///////////////////////////////////////////////////////////////////////////////
// MemDebug class declaration and implementation

class MemDebug
{
    friend void jsAllocDumpBlocks( int );
    friend void jsAllocSetMinimum( void );

    unsigned long signature;
    MemDebug* next;
    MemDebug* prev;

    static MemDebug* blocks;

    static long balance;
    static unsigned long alloc_fail;
    static unsigned long alloc_count;

    static size_t alloc_size;
    static size_t min_alloc_size;
    static size_t max_alloc_size;

    PCSTR szFile;
    int nLine;

public:

    size_t nSize;
    static int CheckFail () { return alloc_fail != 0 && ++alloc_count >= alloc_fail; }

    void* operator new( size_t size, size_t extra );
    void operator delete( void* ptr, size_t extra );

    MemDebug( size_t extra, PCSTR filename, int linenum );
    ~MemDebug();
    };

MemDebug* MemDebug:: blocks = NULL;

long MemDebug:: balance = 0;

unsigned long MemDebug:: alloc_fail = 0;

unsigned long MemDebug:: alloc_count = 0;

size_t MemDebug:: alloc_size = 0;

size_t MemDebug:: min_alloc_size = 0;

size_t MemDebug:: max_alloc_size = 0;

void*
MemDebug:: operator new( size_t size, size_t extra )
{
    void* ptr = malloc( size + extra );

    if ( ptr != NULL )
    {
        memset( ptr, 0, size + extra );
        }

    return ptr;
    }

void
MemDebug:: operator delete( void* ptr, size_t extra )
{
    if ( ptr != NULL )
    {
        free( ptr );
        }
    }

MemDebug:: MemDebug( size_t extra, PCSTR filename, int linenum )
{
    signature = 0x31415926L;

    next = NULL;
    prev = NULL;
    szFile = filename;
    nLine = linenum;
    nSize = extra;

    if ( blocks )
    {
        next = blocks;
        blocks->prev = this;
        }

    blocks = this;
    balance ++;
    alloc_size += nSize;

    if ( alloc_size > max_alloc_size )
    {
        max_alloc_size = alloc_size;
        }
    }

MemDebug:: ~MemDebug ()
{
    if ( signature != 0x31415926L )
    {
        fprintf( stderr, "%p delete block without proper signature\n", this );
        abort ();
        }

    if ( szFile == NULL )
    {
        fprintf (stderr, "%p delete the same block twise\n", this );
        abort ();
        }

    if ( next )
        next->prev = prev;

    if ( prev )
        prev->next = next;
    else
        blocks = next;

    szFile = NULL;
    next = NULL;
    prev = NULL;

    balance --;
    alloc_size -= nSize;
    }

void
jsAllocSetMinimum( void )
{
    MemDebug:: min_alloc_size = MemDebug:: alloc_size;
    }

void
jsAllocDumpBlocks( int dump_all )
{
    fprintf( stderr,
        "MemDebug:: #blocks = %ld, #alloc_count = %lu, #alloc_fail = %lu\n",
        MemDebug:: balance, MemDebug:: alloc_count, MemDebug:: alloc_fail
        );

    fprintf( stderr,
        "alloc_size = %lu, min_alloc_size = %lu, max_alloc_size = %lu\n",
        MemDebug:: alloc_size,
        MemDebug:: min_alloc_size,
        MemDebug:: max_alloc_size
        );

    size_t leaks = 0;

    for ( MemDebug* b = MemDebug::blocks; b; b = b->next )
    {
        if ( dump_all )
        {
            fprintf( stderr, "%s:%d: %lu\n", b->szFile, b->nLine, b->nSize );
            }

        leaks += b->nSize;
        }

    if ( leaks != 0 )
    {
        fprintf( stderr, "MemDebug:: leaks = %lu bytes\n", leaks );
        }
    }

///////////////////////////////////////////////////////////////////////////////
// DEBUG: memory opeartors implementation

void *
operator new( size_t size, const char* filename, int linenum )
{
    MemDebug* ptr = new(size) MemDebug( size, filename, linenum );

    if ( MemDebug:: CheckFail () || ptr == NULL )
    {
        return NULL;
        }

    return ptr + 1;
    }


void *
jsReallocDbg( void* ptr, size_t size, const char* filename, int linenum )
{
    if ( ptr == NULL )
    {
        return new(filename,linenum) uint8[ size ];
        }

    void* nptr = new(filename,linenum) uint8[ size ];

    if ( nptr == NULL )
    {
        return NULL;
        }

    MemDebug* b =(MemDebug*)ptr - 1;

    memcpy( nptr, ptr, size < b->nSize ? size : b->nSize );

    delete b;

    return nptr;
    }

void
operator delete( void *ptr )
{
    if ( ptr != NULL )
    {
        delete ( (MemDebug*)ptr - 1 );
        }
    }

void
operator delete( void *ptr, const char* filename, int linenum )
{
    if ( ptr != NULL )
    {
        delete ( (MemDebug*)ptr - 1 );
        }
    }

#else // not defined JS_DEBUG_NEW

///////////////////////////////////////////////////////////////////////////////
// Memory opeartors implementation

void
jsAllocSetMinimum( void )
{
    }   

void
jsAllocDumpBlocks( int )
{
    }

void*
operator new( size_t size )
{
    void* ptr = malloc( size );

    if ( ptr != NULL )
    {
        memset( ptr, 0, size ); // calloc by default
        }

    return ptr;
    }

void*
jsRealloc( void* ptr, size_t size )
{
    if ( ptr == NULL )
    {
        return new uint8[ size ];
        }

    void* nptr = realloc( ptr, size );

    return nptr;
    }

void
operator delete( void* ptr )
{
    if ( ptr != NULL )
    {
        free( ptr );
        }
    }

#endif // defined JS_DEBUG_NEW
