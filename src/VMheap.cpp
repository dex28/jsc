//
// JSVirtualMachine heap implementation.
//
// SizeToList
// JSVirtualMachine:: MarkPtr
// JSVirtualMachine:: IsMarkedPtr
// JSVirtualMachine:: Alloc
// JSVirtualMachine:: Realloc
// JSVirtualMachine:: Free
// JSVirtualMachine:: GarbageCollect
// JSVirtualMachine:: ClearHeap
//

#include "JS.h"

// report verbose GC times
//
#define GC_TIMES

// The size of the smallest increment of the heap, i.e. the heap
// of the virtual machine is growing in BLOCK_SIZE block increments,
// at least.
//
const size_t BLOCK_SIZE = 100 * 1024;

// The size of the minimum block that can be allocated from the heap.
// The block must be big enought to hold one pointer that is used when
// then block is entered to the freelist.
//
const size_t MIN_ALLOC = sizeof( void* );

#ifdef JS_DEBUG_NEW
const uint MAGIC = 0xFE109ABE;
#endif

static inline int
SizeToList( size_t size )
{
    // Map size to heap freelist index
    //
    int list = 0;
    size >>= 3;

    while ( size > 0 )
    {
        size >>= 1;
        list++;
        }

    if ( list >= JS_NUM_HEAP_FREELISTS )
    {
        list = JS_NUM_HEAP_FREELISTS - 1;
        }

    return list;
    }

int
JSVirtualMachine:: MarkPtr( void* ptr )
{
    if ( ptr == NULL )
        return 0;

    JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)ptr - 1;

#ifdef JS_DEBUG_NEW
    assert( b->magic == MAGIC );
#endif

    if ( b->flag_mark != 0 )
        return 0;

    b->flag_mark = 1;

    return 1;
    }

int
JSVirtualMachine:: IsMarkedPtr( void* ptr )
{
    if ( ptr == NULL )
        return 1;

    JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)ptr - 1;

#ifdef JS_DEBUG_NEW
    assert( b->magic == MAGIC );
#endif

    return b->flag_mark != 0;
    }

void*
JSVirtualMachine:: Alloc( size_t size, bool fHeapObjectToClean )
{
    // Round it up to the next pow of two
    //
    size_t alloc_size = MIN_ALLOC;
    while ( alloc_size < size )
        alloc_size <<= 1;

RETRY:

    // Take first block from the freelist that is big enough for us
    //
    for ( int freelist = SizeToList( alloc_size );
          freelist < JS_NUM_HEAP_FREELISTS;
          freelist++
          )
    {
        JSHeapMemoryBlock* b = heap_freelists[ freelist ];
        JSHeapMemoryBlock* prev = NULL;

        for ( ; b; prev = b, b = ((JSHeapFreelistBlock*) b)->next )
        {
            if ( b->size >= alloc_size )
            {
                // Ok, take this one
                //
                if ( prev )
                {
                    ((JSHeapFreelistBlock*) prev)->next
                        = ((JSHeapFreelistBlock*) b)->next;
                    }
                else
                {
                    heap_freelists[ freelist ] = ((JSHeapFreelistBlock*) b)->next;
                    }

                if ( b->size > alloc_size + sizeof( JSHeapMemoryBlock ) + MIN_ALLOC )
                {
                    // We can split it
                    //
                    JSHeapMemoryBlock* nb
                        = ((JSHeapMemoryBlock*)( (uint8*) b
                                                + sizeof( JSHeapMemoryBlock )
                                                + alloc_size ) );
#ifdef JS_DEBUG_NEW
                    nb->magic = MAGIC;
#endif
                    nb->flag_mark = 0;
                    nb->flag_needs_finalize = 0;
                    nb->size = b->size - sizeof( JSHeapMemoryBlock ) - alloc_size;

                    gc.bytes_free -= sizeof( JSHeapMemoryBlock );

                    freelist = SizeToList( nb->size );
                    ((JSHeapFreelistBlock*) nb)->next = heap_freelists[ freelist ];
                    heap_freelists[ freelist ] = nb;

                    b->size = alloc_size;
                    }

                b->flag_mark = 0;
                b->flag_needs_finalize = 0;

                gc.bytes_free -= b->size;
                gc.bytes_allocated += b->size;

                if ( fHeapObjectToClean )
                {
                    memset( b + 1, 0, size );

                    b->flag_needs_finalize = 1;
                    }

                return b + 1;
                }
            }
        }

    // Must allocate new blocks to the freelist
    //
    size_t to_alloc;

    if ( alloc_size > ( BLOCK_SIZE
                    - sizeof( JSHeapBlock )
                    - sizeof( JSHeapMemoryBlock ) ) )
    {
        to_alloc = alloc_size + sizeof( JSHeapBlock ) + sizeof( JSHeapMemoryBlock );
        }
    else
    {
        to_alloc = BLOCK_SIZE;
        }

    if ( options.verbose > 2 )
    {
        s_stderr->PrintfLn(
                 "VM: heap: malloc(%u): needed=%u, size=%lu, free=%lu, allocated=%lu",
                 to_alloc, alloc_size, heap_size,
                 gc.bytes_free, gc.bytes_allocated
                 );
        }

    JSHeapBlock* hb = (JSHeapBlock*) NEW uint8[ to_alloc ];

    heap_size += to_alloc;
    hb->next = heap;
    heap = hb;
    hb->size = to_alloc - sizeof( JSHeapBlock );

    // Link it to the freelist
    //
    JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)( hb + 1 );

#ifdef JS_DEBUG_NEW
    b->magic = MAGIC;
#endif
    b->flag_mark = 0;
    b->flag_needs_finalize = 0;
    b->size = hb->size - sizeof( JSHeapMemoryBlock );

    freelist = SizeToList( b->size );

    ((JSHeapFreelistBlock*) b)->next = heap_freelists[ freelist ];
    heap_freelists[ freelist ] = b;

    gc.bytes_free += b->size;

    goto RETRY;

    return NULL; // NOTREACHED
    }

void*
JSVirtualMachine:: Realloc( void* ptr, size_t new_size )
{
    if ( ptr == NULL )
    {
        return Alloc( new_size );
        }

    // Can we use the old block?
    //

    JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)ptr - 1;

#ifdef JS_DEBUG_NEW
    assert( b->magic == MAGIC );
#endif

    if ( b->size >= new_size )
    {
        // Yes we can...
        //
        return ptr;
        }

    // No we can't. Must allocate a new one
    //
    void* nptr = Alloc( new_size );
    memcpy( nptr, ptr, b->size < new_size ? b->size : new_size );

    Free( ptr );

    return nptr;
    }

void
JSVirtualMachine:: Free( void* ptr )
{
    JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)ptr - 1;

#ifdef JS_DEBUG_NEW
    assert( b->magic == MAGIC );
#endif

    int freelist = SizeToList( b->size );

    ( (JSHeapFreelistBlock*) b)->next = heap_freelists[ freelist ];
    heap_freelists[ freelist ] = b;
    gc.bytes_free += b->size;

    // We could try to compact the heap, but we left it to the garbage
    // collection.
    }

void
JSVirtualMachine:: GarbageCollect( JSVariant* fp, JSVariant* sp )
{
    if ( options.verbose > 1 )
    {
        s_stderr->PrintfLn( "VM: heap: garbage collect: num_globals=%d", num_globals );
        }

    gc.count++;

    // Mark
    //

#ifdef GC_TIMES
    clock_t start_clock = clock ();
#endif

    // Mark all globals
    //
    for ( int i = 0; i < num_globals; i++ )
    {
        globals[ i ].Mark ();
        }

    // Mark the buitin-infos of the core objects
    //
    for ( i = 0; i <= JS_NAN; i++ )
    {
        MarkPtr( prim[ i ] );
        }

    // Mark stack
    //

    // STACKFRAME
    //

    // Use brute force and mark the whole stack
    //
    for ( sp++; sp < stack + stack_size; sp++ )
    {
        if ( sp->type == JS_INSTR_PTR )
        {
            // Handle the stack frames here
            //

            // Skip the return address
            //
            sp++;

            // Skip the imported-symbols context
            //
            sp++;

            // Possible with-chain
            //
            if ( sp->with_chain.cNode > 0 )
            {
                assert( sp->type == JS_WITH_CHAIN );

                JSVariant* wp = sp->with_chain.node;

                // Mark the with-chain block
                //
                MarkPtr( wp );

                // Mark the objects in the with-chain
                //
                int cNode = sp->with_chain.cNode;

                for ( i = 0; i < cNode; i++ )
                {
                    wp[ i ].Mark ();
                    }
                }

            // Skip with-chain
            //
            sp++;

            // Skip the args_fix
            //
            sp++;

            // And now we point to the old_fp.  We skip it too at the
            // for-loop.
            //
            }
        else
        {
            // Mark this stack item
            sp->Mark ();
            }
        }

    // Sweep all blocks and collect free variants to the freelist
    //

#ifdef GC_TIMES
    clock_t after_mark_clock = clock ();
#endif

    unsigned long bytes_in_use = ClearHeap ();

#ifdef GC_TIMES
    clock_t after_sweep_clock = clock ();
#endif

    if ( options.verbose > 1 )
    {
        s_stderr->PrintfLn( "VM: heap: bytes_in_use=%lu, bytes_free=%lu",
                          bytes_in_use, gc.bytes_free );
        }

#ifdef GC_TIMES
    if ( options.verbose > 1 )
    {
        s_stderr->PrintfLn( "VM: heap: mark_time=%.4f, sweep_time=%.4f",
               double( after_mark_clock - start_clock ) / CLOCKS_PER_SEC,
               double( after_sweep_clock - after_mark_clock ) / CLOCKS_PER_SEC
               );
        }
#endif
    }

unsigned long
JSVirtualMachine:: ClearHeap( void )
{
    // Just sweep without marking
    //

    unsigned long bytes_in_use = 0;

    for ( int i = 0; i < JS_NUM_HEAP_FREELISTS; i++ )
        heap_freelists[ i ] = NULL;

    gc.bytes_free = 0;
    gc.bytes_allocated = 0;

    for ( JSHeapBlock* hb = heap; hb; hb = hb->next )
    {
        JSHeapMemoryBlock* b = (JSHeapMemoryBlock*)
            ( (uint8*) hb + sizeof( JSHeapBlock ) );

        JSHeapMemoryBlock* e = (JSHeapMemoryBlock*)
            ( (uint8*) hb + sizeof( JSHeapBlock ) + hb->size );

        JSHeapMemoryBlock* bnext;

        for (; b < e; b = bnext)
        {
#ifdef JS_DEBUG_NEW
            assert( b->magic == MAGIC );
#endif
            bnext = (JSHeapMemoryBlock*)( (uint8*) b
                                         + sizeof (JSHeapMemoryBlock) + b->size );

            if ( b->flag_mark )
            {
                bytes_in_use += b->size;
                b->flag_mark = 0;
                gc.bytes_allocated = b->size;
                }
            else
            {
                if ( b->flag_needs_finalize )
                {
                    JSHeapObjectToClean* obj = (JSHeapObjectToClean*) ( b + 1 );
                    obj->OnFinalize ();
                    }

                // Pack consecutive free blocks to one big block
                //
                while ( bnext < e && bnext->flag_mark == 0 )
                {
#ifdef JS_DEBUG_NEW
                    assert( bnext->magic == MAGIC );
#endif
                    if ( bnext->flag_needs_finalize )
                    {
                        JSHeapObjectToClean* obj = (JSHeapObjectToClean*) ( bnext + 1 );
                        obj->OnFinalize ();
                        }

                    b->size += bnext->size + sizeof( JSHeapMemoryBlock );
                    bnext = (JSHeapMemoryBlock*)( (uint8*) bnext
                                                 + sizeof( JSHeapMemoryBlock )
                                                 + bnext->size );
                    }

                b->flag_mark = 0;
                b->flag_needs_finalize = 0;

                // Link it to the freelist
                //
                int freelist = SizeToList( b->size );

                ((JSHeapFreelistBlock*) b)->next = heap_freelists[ freelist ];
                heap_freelists[ freelist ] = b;
                gc.bytes_free += b->size;
                }
            }
        }

    return bytes_in_use;
    }
