//
// JSObject implementation.
//

#include "JS.h"

const int HASH_SIZE = 128;

void*
JSObject:: operator new( size_t size, JSVirtualMachine* vm )
{
    JSObject* ptr = (JSObject*)vm->Alloc( size );
    ptr->vm = vm;
    return ptr;
    }

void
JSObject:: operator delete( void* ptr, JSVirtualMachine* vm )
{
    ( (JSObject*)ptr )->vm->Free( ptr );
    }

JSObject:: JSObject( void )
{
    cHash = 0;
    pHash = NULL;

    num_props = 0;
    pProps = NULL;
    }

void
JSObject:: Mark( void )
{
    JSObject* obj = this;

TAIL_RECURSIVE:

    if ( ! JSVirtualMachine::MarkPtr( obj ) )
    {
        // This object has already been marked.  Nothing to do here
        return;
        }

    JSVirtualMachine::MarkPtr( obj->pProps );

    // Mark property hash
    //
    if ( obj->pHash )
    {
        JSVirtualMachine::MarkPtr( obj->pHash );

        for ( int i = 0; i < cHash; i++ )
        {
            for ( HashBucket* b = obj->pHash[ i ].pFirst; b; b = b->pNext )
            {
                JSVirtualMachine::MarkPtr( b );
                JSVirtualMachine::MarkPtr( b->data );
                }
            }
        }

    // Mark all non-object properties
    //
    int num_objects = 0;
    for ( int i = 0; i < obj->num_props; i++ )
    {
        if ( obj->pProps[ i ].value.type == JS_OBJECT )
        {
            if ( ! JSVirtualMachine::IsMarkedPtr( obj->pProps[ i ].value.vobject ) )
            {
                num_objects++;
                }
            }
        else
        {
            obj->pProps[ i ].value.Mark ();
            }
        }

    // And finally, mark all objects we have left
    //
    if ( num_objects > 0 )
    {
        // Find the objects
        //
        for ( i = 0; i < obj->num_props; i++ )
        {
            if ( obj->pProps[ i ].value.type == JS_OBJECT
                 && ! JSVirtualMachine::IsMarkedPtr( obj->pProps[ i ].value.vobject ) )
            {
                if ( num_objects == 1 )
                {
                    // This is the only non-marked object.
                    // We can do a tail-recursion optimization.
                    //
                    obj = obj->pProps[ i ].value.vobject;
                    goto TAIL_RECURSIVE;
                    }

                // Just mark it
                //
                obj->pProps[ i ].value.Mark ();
                }
            }
        }
    }

JSPropertyRC
JSObject:: LoadProperty( JSSymbol prop, JSVariant& value_return )
{
    JSSymbol link_sym = vm->s___proto__;

    JSObject* obj = this;

    for ( ;; )
    {
        JSObject* link_obj = NULL;

        // Check if we know this property
        //
        for ( int i = 0; i < obj->num_props; i++ )
        {
            if ( obj->pProps[ i ].name == prop )
            {
                value_return = obj->pProps[ i ].value;
                return JS_PROPERTY_FOUND;
                }
            else if ( obj->pProps[ i ].name == link_sym
                     && obj->pProps[ i ].value.type == JS_OBJECT )
            {
                link_obj = obj->pProps[ i ].value.vobject;
                }
            }

        // Undefined so far
        //
        if ( link_obj == NULL )
        {
            break;
            }

        // Follow the link
        //
        obj = link_obj;
        }

    // Undefined. Make it undef.
    //
    value_return.type = JS_UNDEFINED;
    return JS_PROPERTY_UNKNOWN;
    }

void
JSObject:: StoreProperty( JSSymbol prop, JSVariant& val )
{
    int free_slot = -1;

    // Check if we already know this property
    //
    for ( int i = 0; i < num_props; i++ )
    {
        if ( pProps[ i ].name == prop )
        {
            pProps[ i ].value = val;
            return;
            }
        else if ( pProps[ i ].name == NULL )
        {
            free_slot = i;
            }
        }

    // Must create a new property
    //
    if ( free_slot == -1 )
    {
        // Expand our array of properties
        //
        pProps = (Property*)vm->Realloc( pProps,
                                         ( num_props + 1 ) * sizeof( Property ) );
        free_slot = num_props++;
        }

    pProps[ free_slot ].name = prop;
    pProps[ free_slot ].attributes = 0;
    pProps[ free_slot ].value = val;

    // Insert it to the hash (if the hash has been created)
    //
    if ( pHash != NULL )
    {
        PCSTR name = vm->Symname( prop );
        HashInsert( name, strlen( name ), free_slot );
        }
    }

void
JSObject:: DeleteProperty( JSSymbol prop )
{
    // Check if we already know this property
    //
    for ( int i = 0; i < num_props; i++ )
    {
        if ( pProps[ i ].name == prop )
        {
            // Found, remove it from our list of properties
            //
            pProps[ i ].name = NULL;
            pProps[ i ].value.type = JS_UNDEFINED;

            // Remove its name from the hash (if present)
            //
            if ( pHash )
            {
                PCSTR name = vm->Symname( prop );
                HashDelete( name, strlen( name ) );
                }

            // All done here
            //
            return;
            }
        }
    }

void
JSObject:: LoadArray( JSVariant& sel, JSVariant& value_return )
{
    if ( sel.type == JS_INTEGER )
    {
        if (sel.vinteger < 0 || sel.vinteger >= num_props )
        {
            value_return.type = JS_UNDEFINED;
            }
        else
        {
            value_return = pProps[ sel.vinteger ].value;
            }
        }
    else if ( sel.type == JS_STRING )
    {
        if ( pHash == NULL )
        {
            HashCreate ();
            }

        int pos = HashLookup ( sel.vstring->data, sel.vstring->len );

        if ( pos < 0 )
        {
            value_return.type = JS_UNDEFINED;
            }
        else
        {
            value_return = pProps[ pos ].value;
            }
        }
    else
    {
        vm->RaiseError( "JSObject::LoadProperty: illegal array index" );
        }
    }

void
JSObject:: StoreArray( JSVariant& sel, JSVariant& value )
{
    if (sel.type == JS_INTEGER)
    {
        if (sel.vinteger < 0)
        {
            vm->RaiseError( "JSObject::StoreArray: array index can't be nagative" );
            }

        if (sel.vinteger >= num_props)
        {
            // Expand properties
            //
            pProps = (Property*)vm->Realloc( pProps,
                                  ( sel.vinteger + 1 ) * sizeof( Property ) );

            // Init the possible gap
            //
            for (; num_props <= sel.vinteger; num_props++ )
            {
                pProps[ num_props ].name = 0;
                pProps[ num_props ].attributes = 0;
                pProps[ num_props ].value.type = JS_UNDEFINED;
                }
            }

        pProps[ sel.vinteger ].value = value;
        }

    else if ( sel.type == JS_STRING )
    {
        if ( pHash == NULL )
        {
            HashCreate ();
            }

        int pos = HashLookup( sel.vstring->data, sel.vstring->len );

        if ( pos >= 0 )
        {
            pProps[ pos ].value = value;
            }
        else
        {
            // It is undefined, define it
            //
            pProps = (Property*)vm->Realloc( pProps,
                                      ( num_props + 1 ) * sizeof( Property ) );

            // FIXME: if <sel> is a valid symbol, intern it and set symbol's
            // name below.
            //
            pProps[ num_props ].name = NULL;
            pProps[ num_props ].attributes = 0;
            pProps[ num_props ].value = value;

            HashInsert( sel.vstring->data, sel.vstring->len, num_props );

            num_props++;
            }
        }
    }

void
JSObject:: DeleteArray( JSVariant& sel )
{
    if ( sel.type == JS_INTEGER )
    {
        if ( 0 <= sel.vinteger && sel.vinteger < num_props )
        {
            JSSymbol sym = pProps[ sel.vinteger ].name;
            pProps[ sel.vinteger ].name = NULL;
            pProps[ sel.vinteger ].value.type = JS_UNDEFINED;

            // Remove its name from the hash (if present and it is not NULL)
            //
            if ( sym != NULL && pHash )
            {
                PCSTR name = vm->Symname( sym );
                HashDelete( name, strlen( name ) );
                }
            }
        }
    else if ( sel.type == JS_STRING )
    {
        if ( pHash == NULL )
        {
            HashCreate ();
            }

        int pos = HashLookup( sel.vstring->data, sel.vstring->len );

        if ( pos >= 0 )
        {
            // Found it
            //
            pProps[ pos ].name = NULL;
            pProps[ pos ].value.type = JS_UNDEFINED;

            // And, delete its name from the hash
            //
            HashDelete ( sel.vstring->data, sel.vstring->len );
            }
        }
    else
    {
        vm->RaiseError( "JSObject::DeleteArray: illegal array index" );
        }
    }


int
JSObject:: Nth( int nth, JSVariant& value_return )
{
    value_return.type = JS_UNDEFINED;

    if ( nth < 0 )
        return 0;

    if ( pHash == NULL )
    {
        HashCreate ();
        }

    for ( int i = 0; i < cHash && nth >= pHash[ i ].length; i++ )
    {
        nth -= pHash[ i ].length;
        }

    if ( i >= cHash )
    {
        return 0;
        }

    // The chain <i> is the correct one
    //
    HashBucket* b = pHash[ i ].pFirst;

    while( b && nth > 0 )
    { 
        b = b->pNext;
        nth--;
        }

    if ( b == NULL )
    {
        vm->s_stderr->PrintfLn( "JSObject::Nth(): chain didn't contain that many items" );
        abort ();
        }

    value_return.MakeString( vm, PSTR( b->data ), b->len );

    return 1;
    }

void
JSObject:: HashCreate( void )
{
    cHash = HASH_SIZE;
    pHash = new(vm) HashBucketList[ cHash ];

    memset( pHash, 0, cHash * sizeof( HashBucketList ) );

    // Insert all known properties to the hash
    //
    for ( int i = 0; i < num_props; i++ )
    {
        if ( pProps[ i ].name != NULL )
        {
            PCSTR name = vm->Symname( pProps[ i ].name );
            HashInsert( name, strlen( name ), i );
            }
        }
    }

void
JSObject:: HashInsert( PCSTR name, int name_len, int pos )
{
    int hash = jsHashFunction( name, name_len ) % cHash;

    HashBucket* b = pHash[ hash ].pFirst;

    for ( ; b != NULL; b = b->pNext )
    {
        if ( b->len == name_len && memcmp( b->data, name, name_len ) == 0 )
        {
            // Ok, we already have a bucket
            //
            b->value = pos;
            return;
            }
        }

    // Create a new bucket
    //
    b = new(vm) HashBucket;
    b->len = name_len;
    b->data = new(vm) uint8[ b->len ];
    memcpy( b->data, name, b->len );

    b->value = pos;

    b->pNext = pHash[ hash ].pFirst;
    pHash[ hash ].pFirst = b;
    pHash[ hash ].length ++;
    }

void
JSObject:: HashDelete( PCSTR name, int name_len )
{
    int hash = jsHashFunction( name, name_len ) % cHash;

    HashBucket* b = pHash[ hash ].pFirst;
    HashBucket* prev = NULL;

    for ( ; b; prev = b, b = b->pNext )
    {
        if ( b->len == name_len && memcmp (b->data, name, name_len ) == 0 )
        {
            // Ok, found it
            //
            if ( prev )
            {
                prev->pNext = b->pNext;
                }
            else
            {
                pHash[ hash ].pFirst = b->pNext;
                }

            pHash[ hash ].length --;

            break;
            }
        }
    }

int
JSObject:: HashLookup( PCSTR name, int name_len )
{
    int hash = jsHashFunction( name, name_len ) % cHash;

    for ( HashBucket* b = pHash[ hash ].pFirst; b; b = b->pNext )
    {
        if ( b->len == name_len && memcmp( b->data, name, name_len ) == 0 )
        {
            return b->value;
            }
        }

    return -1;
    }
