//
// The Array class
//

#include "JS.h"

/*
    Mehtods:

        concat( array ) => array
        join( [glue] ) => string
        pop( void ) => last_element
        push( any... ) => last_element_added
        reverse( void )
        shift( void ) => first_element
        slice( start, end ) => array
        splice( index, how_many[, any...] ) => array
        sort( [sort_function] )
        toString( void ) => string
        unshift( any... ) => length_of_the_array

   Properties:

        length
*/

struct JSBuiltinInfo_Array : public JSBuiltinInfo
{
    // Methods
    JSSymbol s_concat;
    JSSymbol s_join;
    JSSymbol s_pop;
    JSSymbol s_push;
    JSSymbol s_reverse;
    JSSymbol s_shift;
    JSSymbol s_slice;
    JSSymbol s_splice;
    JSSymbol s_sort;
    JSSymbol s_unshift;

    // Properties
    JSSymbol s_length;

    JSBuiltinInfo_Array( void );

    virtual void OnGlobalMethod (
        void* instance_context, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnMethod ( 
        void* instance_context, JSSymbol method, JSVariant& result_return, JSVariant args [] );

    virtual JSPropertyRC OnProperty (
        void* instance_context, JSSymbol property, bool set, JSVariant& node );

    virtual void OnNew ( JSVariant args [], JSVariant& result_return );
    };

// Context for array sorts with JavaScript functions; see jsMergesort
//
struct ArraySortCtx
{
    JSVirtualMachine* vm;
    JSVariant* func;
    JSVariant argv[ 3 ];
    };

static int
sort_default_cmp_func( const void* aptr, const void* bptr, void* context )
{
    JSVirtualMachine *vm = (JSVirtualMachine*)context;

    JSVariant* a = (JSVariant*)aptr;
    if ( a->type == JS_UNDEFINED )
        return 1;

    JSVariant* b = (JSVariant*)bptr;
    if ( b->type == JS_UNDEFINED )
        return -1;

    JSVariant astr;
    a->ToString( vm, astr );

    JSVariant bstr;
    b->ToString( vm, bstr );

    return astr.StringCompare( bstr );
    }

static int
sort_cmp_func( const void* aptr, const void* bptr, void* context )
{
    ArraySortCtx* ctx = (ArraySortCtx*)context;

    //
    // Finalize the argument array.  The argumnet count has already been set.
    // when the context were initialized.
    //
    ctx->argv[ 1 ] = *(JSVariant*)aptr;
    ctx->argv[ 2 ] = *(JSVariant*)bptr;

    // Call the function
    //
    if ( ! ctx->vm->Apply( NULL, ctx->func, 3, ctx->argv ) )
        ctx->vm->RaiseError ();

    // Fetch the return value
    //
    if ( ctx->vm->exec_result.type != JS_INTEGER )
    {
        ctx->vm->RaiseError( "Array.sort(): comparison function didn't return integer" );
        }

    return ctx->vm->exec_result.vinteger;
    }

///////////////////////////////////////////////////////////////////////////////
// Re-entrant mergesort.
//
typedef int (*MergesortCompFunc)( const void* a, const void* b, void* context );

void jsMergesort( void* base, int number_of_elements, int size,
                  MergesortCompFunc comparison_func, void* comparison_func_context );

#define COPY( buf1, idx1, buf2, idx2 ) \
    memcpy( (buf1) + (idx1) * size, (buf2) + (idx2) * size, size );

static void
do_mergesort
(
    uint8* base, uint size,
    uint8* tmp,
    uint l,
    uint r,
    MergesortCompFunc func, void* func_ctx
    )
{
    if ( r <= l )
        return;

    uint m = ( r + l ) / 2;
    do_mergesort( base, size, tmp, l, m, func, func_ctx );
    do_mergesort( base, size, tmp, m + 1, r, func, func_ctx );

    memcpy( tmp + l * size, base + l * size, ( r - l + 1 ) * size );

    uint i = l;
    uint j = m + 1;
    uint k = l;

    // Merge them
    //
    while ( i <= m && j <= r )
    {
        if ( func( tmp + i * size, tmp + j * size, func_ctx ) <= 0 )
        {
            COPY( base, k, tmp, i );
            i++;
            }
        else
        {
            COPY( base, k, tmp, j );
            j++;
            }
        k++;
        }

    // Copy left-overs.  Only one of the following will be executed
    //
    for( ; i <= m; i++ )
    {
        COPY( base, k, tmp, i );
        k++;
        }

    for( ; j <= r; j++ )
    {
        COPY( base, k, tmp, j );
        k++;
        }
    }

void
jsMergesort
(
    void* base, int number_of_elements, int size,
    MergesortCompFunc comparison_func, void* comparison_func_context
    )
{
    if ( number_of_elements == 0 )
        return;

    // Allocate tmp buffer
    //
    uint8* tmp = (uint8*)alloca( number_of_elements * size );
    assert( tmp != NULL );

    do_mergesort( (uint8*)base, size, tmp, 0, number_of_elements - 1,
                  comparison_func, comparison_func_context );
    }

JSBuiltinInfo_Array:: JSBuiltinInfo_Array( void )
    : JSBuiltinInfo( "Array" )
{
    s_concat    = vm->Intern( "concat" );
    s_join      = vm->Intern( "join" );
    s_pop       = vm->Intern( "pop" );
    s_push      = vm->Intern( "push" );
    s_reverse   = vm->Intern( "reverse" );
    s_shift     = vm->Intern( "shift" );
    s_slice     = vm->Intern( "slice" );
    s_splice    = vm->Intern( "splice" );
    s_sort      = vm->Intern( "sort" );
    s_unshift   = vm->Intern( "unshift" );

    s_length    = vm->Intern( "length" );

    // VM primitive
    //
    vm->prim[ JS_ARRAY ] = this;
    }

void
JSBuiltinInfo_Array:: OnGlobalMethod
(
    void* /*instance_context*/,
    JSVariant& result_return,
    JSVariant args []
    )
{
    // This does exactly the same as the new_proc
    //
    OnNew( args, result_return );
    }

JSPropertyRC
JSBuiltinInfo_Array:: OnMethod
(
    void* instance_context,
    JSSymbol method,
    JSVariant& result_return,
    JSVariant args []
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    // FIXME: 15.7.4.3 toSource()

    // Static methods
    //
    if ( instance_context == NULL )
    {
        // --------------------------------------------------------------------
        if ( method == vm->s_toString )
        {
            result_return.MakeStaticString( vm, "Array" );
            return JS_PROPERTY_FOUND;
            }

        return JS_PROPERTY_UNKNOWN;
        }

    // Instance methods
    //

    // Set the default result type
    //
    result_return.type = JS_UNDEFINED;

    // ------------------------------------------------------------------------
    if ( method == s_concat )
    {
        // Count the new len
        //
        int nlen = n->varray->length;
        for ( int i = 0; i < args->vinteger; i++ )
        {
            if ( args[ i + 1 ].type != JS_ARRAY )
                goto argument_error;

            nlen += args[ i + 1 ].varray->length;
            }

        result_return.MakeArray( vm, nlen );

        // Insert the items
        //
        memcpy( result_return.varray->data, n->varray->data,
                n->varray->length * sizeof( JSVariant ) );

        int pos = n->varray->length;
        for ( i = 0; i < args->vinteger; i++ )
        {
            memcpy( &result_return.varray->data[ pos ],
                   args[ i + 1 ].varray->data,
                   args[ i + 1 ].varray->length * sizeof( JSVariant )
                   );
            pos += args[ i + 1 ].varray->length;
            }

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_join || method == vm->s_toString )
    {
        PSTR glue = NULL;

        if ( method == vm->s_toString )
        {
            if ( args->vinteger != 0 )
                goto argument_error;
            }
        else
        {
            if ( args->vinteger == 0 )
                ;
            else if ( args->vinteger == 1 )
            {
                JSVariant glueCvt;
                args[ 1 ].ToString( vm, glueCvt );
                glue = glueCvt.ToNewCString ();
                }
            else
            {
                goto argument_error;
                }
            }

        // Ok, ready to run
        //
        if ( n->varray->length == 0 )
        {
            result_return.MakeStaticString( vm, "" );
            }
        else
        {
            int glue_len = glue ? strlen( glue ) : 1;

            // Estimate the result length
            //
            int len = n->varray->length * 5
                    + ( n->varray->length - 1 ) * glue_len;

            result_return.MakeString( vm, NULL, len );
            result_return.vstring->len = 0;

            // Do the join
            //
            for ( int i = 0; i < n->varray->length; i++ )
            {
                JSVariant sitem;
                n->varray->data[ i ].ToString( vm, sitem );

                int delta = sitem.vstring->len;

                if ( i + 1 < n->varray->length )
                    delta += glue_len;

                result_return.vstring->data
                    = (PSTR)vm->Realloc( result_return.vstring->data,
                                         result_return.vstring->len + delta );

                memcpy( result_return.vstring->data + result_return.vstring->len,
                        sitem.vstring->data,
                        sitem.vstring->len );

                result_return.vstring->len += sitem.vstring->len;

                if ( i + 1 < n->varray->length )
                {
                    if ( glue )
                    {
                        memcpy (result_return.vstring->data + result_return.vstring->len,
                                glue, glue_len);
                        result_return.vstring->len += glue_len;
                        }
                    else
                    {
                        result_return.vstring->data[ result_return.vstring->len++ ] = ',';
                        }
                    }
                }
            }

        if ( glue )
            delete glue;

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_pop )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( n->varray->length == 0)
        {
            result_return.type = JS_UNDEFINED;
            }
        else
        {
            result_return = n->varray->data[ n->varray->length - 1 ];
            n->varray->length--;
            }

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_push )
    {
        if ( args->vinteger == 0 )
            goto argument_error;

        int old_len = n->varray->length;
        n->ExpandArray( vm, n->varray->length + args->vinteger );

        for ( int i = 0; i < args->vinteger; i++ )
            n->varray->data[ old_len + i ] = args[ i + 1 ];

        result_return = args[ i ];

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_reverse )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        for ( int i = 0, j = n->varray->length - 1; i < n->varray->length / 2; i++, j-- )
        {
            JSVariant tmp = n->varray->data[ i ];
            n->varray->data[ i ] = n->varray->data[ j ];
            n->varray->data[ j ] = tmp;
            }

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_shift )
    {
        if ( args->vinteger != 0 )
            goto argument_error;

        if ( n->varray->length == 0 )
        {
            result_return.type = JS_UNDEFINED;
            }
        else
        {
            result_return = n->varray->data[ 0 ];
            memmove ( &n->varray->data[ 0 ], &n->varray->data[ 1 ],
                     ( n->varray->length - 1 ) * sizeof( JSVariant ));
            n->varray->length--;
            }

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_slice )
    {
        if ( args->vinteger < 1 || args->vinteger > 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER )
            goto argument_type_error;

        int start = args[ 1 ].vinteger;
        int end = n->varray->length;

        if ( args->vinteger == 2 )
        {
            if ( args[ 2 ].type != JS_INTEGER )
                goto argument_type_error;

            end = args[ 2 ].vinteger;
            }

        if ( end < 0 )
            end += n->varray->length;

        if ( end < 0 )
            end = start;

        result_return.MakeArray( vm, end - start );

        // Copy items
        //
        for ( int i = 0; i < end - start; i++ )
            result_return.varray->data[ i ] = n->varray->data[ start + i ];

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_splice )
    {
        if ( args->vinteger < 2 )
            goto argument_error;

        if ( args[ 1 ].type != JS_INTEGER || args[ 2 ].type != JS_INTEGER )
            goto argument_type_error;

        if ( args[ 2 ].vinteger == 0 && args->vinteger == 2 )
        {
            // No deletions: must specify at least one item to insert
            goto argument_error;
            }

        int new_length = n->varray->length;
        int old_length = new_length;
        if (args[ 1 ].vinteger < new_length )
        {
            if ( args[ 2 ].vinteger > new_length - args[ 1 ].vinteger )
            {
                args[ 2 ].vinteger = new_length - args[ 1 ].vinteger;
                new_length = args[ 1 ].vinteger;
                }
            else
            {
                new_length -= args[ 2 ].vinteger;
                }
            }
        else
        {
            new_length = args[ 1 ].vinteger;
            args[ 2 ].vinteger = 0;
            }

        new_length += args->vinteger - 2;

        if ( new_length > n->varray->length )
        {
            n->ExpandArray( vm, new_length);
            }
        else
        {
            // Cut the array
            n->varray->length = new_length;
            }

        // Do the stuffs we must do
        //

        // Create the result array
        //
        if ( args[ 2 ].vinteger == 0 )
        {
            result_return.type = JS_UNDEFINED;
            }
        else
        {
            result_return.MakeArray( vm, args[ 2 ].vinteger );
            for ( int i = 0; i < args[ 2 ].vinteger; i++ )
                result_return.varray->data[ i ] =
                         n->varray->data[ args[ 1 ].vinteger + i ];
            }

        // Delete and move
        //
        int delta = args->vinteger - 2 - args[ 2 ].vinteger;
        memmove ( &n->varray->data[ args[ 1 ].vinteger + args[ 2 ].vinteger
                                    + delta ],
                  &n->varray->data[ args[ 1 ].vinteger + args[ 2 ].vinteger ],
                  ( old_length - ( args[ 1 ].vinteger + args[ 2 ].vinteger ) )
                    * sizeof( JSVariant )
                  );

        // Insert
        //
        for ( int i = 0; i < args->vinteger - 2; i++ )
            n->varray->data[ args[ 1 ].vinteger + i ] = args[ i + 3 ];

        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_sort )
    {
        if ( args->vinteger != 0 && args->vinteger != 1 )
        {
            goto argument_error;
            }

        ArraySortCtx array_sort_ctx;
        MergesortCompFunc func = sort_default_cmp_func;
        void *func_ctx = vm;

        if ( args->vinteger == 1 )
        {
            if ( args[ 1 ].type != JS_FUNCTION && args[ 1 ].type != JS_BUILTIN )
            {
                goto argument_type_error;
                }

            func = sort_cmp_func;

            // Init context
            //
            array_sort_ctx.vm = vm;
            array_sort_ctx.func = &args[ 1 ];

            // Init the argc part of the argument vector here
            //
            array_sort_ctx.argv[ 0 ].type = JS_INTEGER;
            array_sort_ctx.argv[ 0 ].vinteger = 3;

            func_ctx = &array_sort_ctx;
            }

        jsMergesort( n->varray->data, n->varray->length, sizeof( JSVariant ),
                     func, func_ctx );
        
        return JS_PROPERTY_FOUND;
        }

    // ------------------------------------------------------------------------
    else if ( method == s_unshift )
    {
        if ( args->vinteger == 0 )
            goto argument_error;

        int old_len = n->varray->length;
        n->ExpandArray( vm, n->varray->length + args->vinteger);

        memmove( &n->varray->data[ args->vinteger ], n->varray->data,
                old_len * sizeof( JSVariant ) );

        for ( int i = 0; i < args->vinteger; i++ )
            n->varray->data[ i ] = args[ args->vinteger - i ];

        result_return.type = JS_INTEGER;
        result_return.vinteger = n->varray->length;

        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    return JS_PROPERTY_UNKNOWN;

argument_error:
    vm->RaiseError( "Array.%s(): illegal amount of arguments", vm->Symname( method ) );

argument_type_error:
    vm->RaiseError( "Array.%s(): illegal argument", vm->Symname( method ) );

    assert( 1 );
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

JSPropertyRC
JSBuiltinInfo_Array:: OnProperty
(
    void* instance_context,
    JSSymbol property,
    bool set,
    JSVariant& node
    )
{
    JSVariant* n = (JSVariant*)instance_context;

    if ( n != NULL && property == s_length )
    {
        if ( set )
            goto immutable;

        node.type = JS_INTEGER;
        node.vinteger = n->varray->length;
        return JS_PROPERTY_FOUND;
        }

    //
    // Error handling.
    //

    if ( ! set )
        node.type = JS_UNDEFINED;

    return JS_PROPERTY_UNKNOWN;

immutable:
    vm->RaiseError( "Array.%s: immutable property", vm->Symname( property ) );

    assert( 1 ) ;
    return JS_PROPERTY_UNKNOWN; // NOTREACHED
    }

void
JSBuiltinInfo_Array:: OnNew
(
    JSVariant args [],
    JSVariant& result_return
    )
{
    if ( args->vinteger == 1 && args[ 1 ].type == JS_INTEGER )
    {
        // Create a fixed length array
        //
        result_return.MakeArray( vm, args[ 1 ].vinteger );
        }
    else
    {
        if ( args->vinteger < 0 )
        {
            // We are called from the array initializer.
            //
            args->vinteger = - args->vinteger;
            }

        result_return.MakeArray( vm, args->vinteger );

        for ( int i = 0; i < args->vinteger; i++ )
        {
            result_return.varray->data[ i ] = args[ i + 1 ];
            }
        }

    // Set the [[Prototype]] and [[Class]] properties
    // FIXME: 15.7.2.1
    }

//
// The Array class initialization entry
//

void
JSVirtualMachine:: BuiltinArray( void )
{
    new(this) JSBuiltinInfo_Array;
    }
