//
// JS Virtual Machine CPU Engine
//

case JS_OPCODE_NOP:
//
//  Do nothing; no operation.
//
    break;

case JS_OPCODE_DUP:
//
//  Duplicate the item at the top of the stack.
//
    JS_SP0 = JS_SP1;
    JS_PUSH ();
    break;

case JS_OPCODE_POP:
//
//  Remove one item from the top of the stack.
//
    JS_POP ();
    break;

case JS_OPCODE_POP_N:
//
//  Remove 'Int32' items from the top of the stack.
//
    arg_int32 = READ_ARG_INT32();
    JS_POP_N( arg_int32 );
    break;

case JS_OPCODE_SWAP:
//
//  Swap the two topmost items in the stack.
//
    JS_SP0 = JS_SP2;
    JS_SP2 = JS_SP1;
    JS_SP1 = JS_SP0;
    break;

case JS_OPCODE_ROLL:
//
//  Roll the 'Int32' topmost items in the stack left/right depending
//  on 'Int32' sign.
//
    arg_int32 = READ_ARG_INT32();

    if ( arg_int32 > 1 )
    {
        for ( JSInt32 j = 0; j < arg_int32; j++ )
            JS_SP( j ) = JS_SP( j + 1 );

        JS_SP( arg_int32 ) = JS_SP0;
        }
    else if ( arg_int32 < -1 )
    {
        arg_int32 = - arg_int32;

        JS_SP0 = JS_SP( arg_int32 );
        for ( ; arg_int32 > 0; arg_int32-- )
            JS_SP( arg_int32 ) = JS_SP( arg_int32 - 1 );
        }
    break;

case JS_OPCODE_CONST:
//
//  Push a constant from the constant section to the stack.
//  The constant is specified by the value 'Int32'.
//
    arg_variant = READ_ARG_CONST();
    JS_SP0 = *arg_variant;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_NULL:
//
//  Push value null to the stack.
//
    JS_SP0.type = JS_NULL;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_UNDEFINED:
//
//  Push value undefined to the stack.
//
    JS_SP0.type = JS_UNDEFINED;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_TRUE:
//
//  Push value true to the stack.  
//
    JS_SP0.type = JS_BOOLEAN;
    JS_SP0.vboolean = 1;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_FALSE:
//
//  Push value false to the stack.
//
    JS_SP0.type = JS_BOOLEAN;
    JS_SP0.vboolean = 0;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_I0:
//
//  Push integer number 0 to the stack.
//
    JS_SP0.type = JS_INTEGER;
    JS_SP0.vinteger = 0;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_I1:
//
//  Push integer number 1 to the stack.
//
    JS_SP0.type = JS_INTEGER;
    JS_SP0.vinteger = 1;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_I2:
//
//  Push integer number 2 to the stack.
//
    JS_SP0.type = JS_INTEGER;
    JS_SP0.vinteger = 2;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_I3:
//
//  Push integer number 3 to the stack.
//
    JS_SP0.type = JS_INTEGER;
    JS_SP0.vinteger = 3;
    JS_PUSH ();
    break;

case JS_OPCODE_CONST_I:
//
//  Push integer number 'Int32' to the stack.
//
    arg_int32 = READ_ARG_INT32();
    JS_SP0.type = JS_INTEGER;
    JS_SP0.vinteger = arg_int32;
    JS_PUSH ();
    break;

case JS_OPCODE_LOCALS:
//
//  Allocate 'Int32' local variables from the stack frame.
//  The opcode 'locals' must be called in the beginning of the function code.
//  The opcode will push 'Int32' 'undefined' values to the stack.
//  The values will be the place-holders for the local variables.
//
    arg_int32 = READ_ARG_INT32();

    if ( SP - arg_int32 - JS_RESERVE_STACK_FOR_FUNCTION < stack )
    {
        RaiseError( "VM[locals]: stack overflow" );
        }

    for ( ; arg_int32 > 0; arg_int32-- )
    {
        JS_SP0.type = JS_UNDEFINED;
        JS_PUSH ();
        }
    break;

case JS_OPCODE_APOP:
//
//  Remove 'Int32' items from the top of the stack, leaving the topmost
//  item on the top of the stack. This opcode is used to remove arguments
//  of a function call, leaving the function's return value to
//  the top of the stack.
//
    arg_int32 = READ_ARG_INT32();
    JS_SP( arg_int32 + 1 ) = JS_SP1;
    JS_POP_N( arg_int32 );
    break;

case JS_OPCODE_MIN_ARGS:
//
//  If the number of the arguments for the function integer is smaller than 'Int32',
//  expand the stack frame so that the function gets 'Int32' arguments. The created
//  arguments will have value undefined.
//
{
    arg_int32 = READ_ARG_INT32();

    // Number of arguments is integer stored is JS_ARG( 1 )
    // JS_ARG( 0 ) contains 'this' pointer,
    // Actual arguments are JS_ARG( 2 + i ), i = 0..JS_ARG(1)-1
    //
    if ( JS_ARG( 1 ).type != JS_INTEGER )
    {
        RaiseError( "VM[min_args]: invalid stack frame" );
        }

    JSInt32 argc = JS_ARG( 1 ).vinteger + 2;

    if ( argc < arg_int32 )
    {
        JSInt32 delta = arg_int32 - argc;

        memmove( &JS_SP1 - delta, &JS_SP1, ( FP - &JS_SP0 + argc ) * sizeof( JSVariant ) );
        SP -= delta;
        FP -= delta;

        // Fill up the fix_args slot.
        //
        JS_ARGS_FIXP.argc = argc;
        JS_ARGS_FIXP.delta = delta;

        for ( ; argc < arg_int32; argc++ )
        {
            JS_ARG( argc ).type = JS_UNDEFINED;
            }
        }
    }
    break;

case JS_OPCODE_LOAD_ARG:
//
//  Push the value of the argument 'Int32' to the stack.
//
    arg_int32 = READ_ARG_INT32();
    JS_SP0 = JS_ARG( arg_int32 );
    JS_PUSH ();
    break;

case JS_OPCODE_STORE_ARG:
//
//  Store the topmost item of the stack to the argument 'Int32'.
//
    arg_int32 = READ_ARG_INT32();
    JS_ARG( arg_int32 ) = JS_SP1;
    JS_POP ();
    break;

case JS_OPCODE_LOAD_NTH_ARG:
//
//  Push the 'integer'th argument of function to the top of the stack. The index
//  'integer' must be an integer number.
//
{
    JSInt32 index = JS_SP1.vinteger;
    JS_SP1 = JS_ARG( index );
    }
    break;

case JS_OPCODE_LOAD_GLOBAL:
//
//  Push the value of the global variable 'Symbol' to the stack.
//  The opcode will *not* lookup the variable 'Symbol' from the with-chain.
//
{
    arg_symbol = READ_ARG_SYMBOL();

    // Use the global value only
    //
    JS_SP0 = *arg_symbol;
    JS_PUSH ();

    if ( JS_SP1.type == JS_UNDEFINED && options.warn_undef
        && PC->op != JS_OPCODE_JSR_W && PC->op != JS_OPCODE_JSR
        && arg_symbol != s_undefined
        )
    {
        s_stderr->PrintfLn( "VM: warning: using undefined global '%s'",
                            Symname( arg_symbol ) );
        }
    }
    break;

case JS_OPCODE_LOAD_GLOBAL_W:
//
//  Push the value of the global variable 'Symbol' to the stack. The
//  opcode will lookup the property 'Symbol' from the currently active
//  with-chain.
//
{
    arg_symbol = READ_ARG_SYMBOL();

    bool found = false;

    // Loop over the with chain.
    //
    if ( JS_WITH_CHAINP.cNode > 0 )
    {
        for ( JSInt32 i = JS_WITH_CHAINP.cNode - 1; i >= 0; i-- )
        {
            JSVariant *w = &JS_WITH_CHAINP.node[ i ];
            JSPropertyRC result = JS_PROPERTY_UNKNOWN;

            if ( w->type == JS_BUILTIN )
            {
                result = w->vbuiltin->info->OnProperty
                (
                    w->vbuiltin->instance_context,  // void* instance context
                    arg_symbol,                     // JSSymbol property
                    false,                          // BOOL set
                    builtin_result                  // JSVariant& result return
                    );
                }
            else if ( w->type == JS_OBJECT )
            {
                result = w->vobject->LoadProperty( arg_symbol, builtin_result );
                }
            else
            {
                RaiseError( "VM[load_global]: corrupted with-chain" );
                }

            if ( result == JS_PROPERTY_FOUND )
            {
                JS_SP0 = builtin_result;
                JS_PUSH ();
                found = true;
                break;
                }
            }
        }

    if ( ! found )
    {
        // Use the global value.
        //
        JS_SP0 = *arg_symbol;
        JS_PUSH ();

        if ( JS_SP1.type == JS_UNDEFINED && options.warn_undef
            && PC->op != JS_OPCODE_JSR_W && PC->op != JS_OPCODE_JSR
            )
        {
            s_stderr->PrintfLn( "VM: warning: using undefined global `%s'",
                                Symname( arg_symbol ) );
            }
        }
    }
    break;

case JS_OPCODE_STORE_GLOBAL:
//
//  Store the topmost item of the stack to the global variable 'Symbol'.
//
    arg_symbol = READ_ARG_SYMBOL();

    // Opcode store_global do not check the with-chain. WITHCHAIN

    // Set the global value.
    //
    *arg_symbol = JS_SP1;
    JS_POP ();
    break;

case JS_OPCODE_LOAD_LOCAL:
//
//  Push the value of the local variable 'Int16' to the stack.
//
    arg_int32 = READ_ARG_INT32();
    JS_SP0 = JS_LOCAL( arg_int32 );
    JS_PUSH ();
    break;

case JS_OPCODE_STORE_LOCAL:
//
//  Store the topmost item of the stack to the local variable 'Int16'.
//
    arg_int32 = READ_ARG_INT32();
    JS_LOCAL( arg_int32 ) = JS_SP1;
    JS_POP ();
    break;

case JS_OPCODE_LOAD_PROPERTY:
//
//  Push the value of the property 'Symbol' of object 'object' to the stack.
//
    arg_symbol = READ_ARG_SYMBOL();

    if ( JS_SP1.type == JS_BUILTIN )
    {
        JSPropertyRC rc = JS_SP1.vbuiltin->info->OnProperty
        (
            JS_SP1.vbuiltin->instance_context, // void* instance context
            arg_symbol,                        // JSSymbol property
            false,                             // BOOL set
            builtin_result                     // JSVariant& result return
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            if ( arg_symbol == s_prototype )
            {
                // Looking up the prototype.
                //
                builtin_result.type = JS_OBJECT;

                if ( JS_SP1.vbuiltin->prototype )
                {
                    // This is an instance.
                    //
                    builtin_result.vobject = JS_SP1.vbuiltin->prototype;
                    }
                else
                {
                    // This is a class.
                    //
                    builtin_result.vobject = JS_SP1.vbuiltin->info->prototype;
                    }
                }
            else
            {
                // Looking up stuffs from the prototype.
                //
                if ( JS_SP1.vbuiltin->prototype )
                {
                    // An instance.
                    //
                    JS_SP1.vbuiltin->prototype->LoadProperty( arg_symbol, builtin_result );
                    }
                else
                {
                    // A class.
                    //
                    JS_SP1.vbuiltin->info->prototype->LoadProperty( arg_symbol, builtin_result );
                    }
                }
            }

        JS_SP1 = builtin_result;
        }

    else if ( JS_SP1.type == JS_OBJECT )
    {
        JS_SP1.vobject->LoadProperty( arg_symbol, JS_SP1 );
        }

    else if ( prim[ JS_SP1.type ] )
    {
        // The primitive language types.
        //
        JSPropertyRC rc = prim[ JS_SP1.type ]->OnProperty
        (
            &JS_SP1,        // void* instance context
            arg_symbol,     // JSSymbol property
            false,          // BOOL set
            builtin_result  // JSVariant& result return
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            if ( arg_symbol == s_prototype )
            {
                // Looking up the prototype.
                //
                switch( JS_SP1.type )
                {
                    case JS_STRING:
                        if ( JS_SP1.vstring->prototype )
                        {
                            builtin_result.type = JS_OBJECT;
                            builtin_result.vobject = JS_SP1.vstring->prototype;
                            }
                        else
                            // No prototype yet.
                            builtin_result.type = JS_NULL;
                        break;

                    case JS_ARRAY:
                        if ( JS_SP1.varray->prototype )
                        {
                            builtin_result.type = JS_OBJECT;
                            builtin_result.vobject = JS_SP1.varray->prototype;
                            }
                        else
                        {
                            // No prototype yet.
                            builtin_result.type = JS_NULL;
                            }
                        break;

                    case JS_FUNCTION:
                        if ( JS_SP1.vfunction->prototype )
                        {
                            builtin_result.type = JS_OBJECT;
                            builtin_result.vobject = JS_SP1.vfunction->prototype;
                            }
                        else
                        {
                            // No prototype yet.
                            builtin_result.type = JS_NULL;
                            }
                        break;

                    default:
                        // The rest do not have prototype.
                        builtin_result.type = JS_NULL;
                    }
                }
            else
            {
                // Looking up stuffs from the prototype.
                //
                switch ( JS_SP1.type )
                {
                    case JS_STRING:
                        if ( JS_SP1.vstring->prototype )
                        {
                            JS_SP1.vstring->prototype->LoadProperty( arg_symbol, builtin_result );
                            }
                        else
                        {
                            // Take it from the class' prototype
                            //
                            goto _OP_LOAD_PROPERTY_TRY_PROTO;
                            }
                        break;

                    case JS_ARRAY:
                        if ( JS_SP1.varray->prototype )
                        {
                            JS_SP1.varray->prototype->LoadProperty( arg_symbol, builtin_result );
                            }
                        else
                        {
                            // Take it from the class' prototype
                            //
                            goto _OP_LOAD_PROPERTY_TRY_PROTO;
                            }
                        break;

                    case JS_FUNCTION:
                        if ( JS_SP1.vfunction->prototype )
                        {
                            JS_SP1.vfunction->prototype->LoadProperty( arg_symbol, builtin_result );
                            }
                        else
                        {
                            // Take it from the class' prototype
                            //
                            goto _OP_LOAD_PROPERTY_TRY_PROTO;
                            }
                        break;

                    default:
                        //
                        // The rest do not have instance prototypes; use the
                        // class prototypes.
                        //
                        _OP_LOAD_PROPERTY_TRY_PROTO:
                        prim[ JS_SP1.type ]->prototype->LoadProperty( arg_symbol, builtin_result );
                        break;
                    }
                }
            }

        JS_SP1 = builtin_result;
        }
    else
    {
        RaiseError( "VM[load_property]: illegal object" );
        }
    break;

case JS_OPCODE_STORE_PROPERTY:
//
//  Save the value value to the property 'Symbol' of object 'object'.
//
    arg_symbol = READ_ARG_SYMBOL();

    if ( JS_SP1.type == JS_BUILTIN )
    {
        JSPropertyRC rc = JS_SP1.vbuiltin->info->OnProperty
        (
            JS_SP1.vbuiltin->instance_context, // void* instance context
            arg_symbol,                        // JSSymbol property
            true,                              // BOOL set
            JS_SP2                             // JSVariant& result return
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            if ( arg_symbol == s_prototype )
            {
                // Setting the prototype.
                //
                if ( JS_SP2.type != JS_OBJECT )
                {
                    RaiseError( "VM[store_property]: illegal value" );
                    }

                if ( JS_SP1.vbuiltin->prototype )
                {
                    // Setting the instance's prototype.
                    JS_SP1.vbuiltin->prototype = JS_SP2.vobject;
                    }
                else
                {
                    // Setting the class' prototype.
                    JS_SP1.vbuiltin->info->prototype = JS_SP2.vobject;
                    }
                }
            else
            {
                // Setting stuff to the prototype.
                if ( JS_SP1.vbuiltin->prototype )
                {
                    // An instance.
                    JS_SP1.vbuiltin->prototype->StoreProperty( arg_symbol, JS_SP2 );
                    }
                else
                {
                    // A class.
                    JS_SP1.vbuiltin->info->prototype->StoreProperty( arg_symbol, JS_SP2 );
                    }
                }
            }

        JS_POP ();
        JS_POP ();
        }
    else if ( JS_SP1.type == JS_OBJECT )
    {
        JS_SP1.vobject->StoreProperty( arg_symbol, JS_SP2 );
        JS_POP ();
        JS_POP ();
        }
    else if ( prim[ JS_SP1.type ] )
    {
        // The primitive language types.
        //
        JSPropertyRC rc = prim[ JS_SP1.type ]->OnProperty
        (
            &JS_SP1,     // void* instance context
            arg_symbol,  // JSSymbol property
            true,        // BOOL set
            JS_SP2       // JSVariant& result return
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            if ( arg_symbol == s_prototype )
            {
                // Setting the prototype.
                //
                if ( JS_SP2.type != JS_OBJECT )
                {
                    RaiseError( "VM[store_property]: illegal value" );
                    }

                switch ( JS_SP1.type )
                {
                    case JS_STRING:
                        JS_SP1.vstring->prototype = JS_SP2.vobject;
                        break;

                    case JS_ARRAY:
                        JS_SP1.varray->prototype = JS_SP2.vobject;
                        break;

                    case JS_FUNCTION:
                        JS_SP1.vfunction->prototype = JS_SP2.vobject;
                        break;

                    default:
                        RaiseError( "VM[store_property]: illegal object" );
                    }
                }
            else
            {
                JSVariant prototype;

                // Setting to the prototype.  We create them on demand.
                //
                switch ( JS_SP1.type )
                {
                    case JS_STRING:
                        if ( JS_SP1.vstring->prototype == NULL )
                        {
                            prototype.type = JS_OBJECT;

                            // Create the prototype and set its __proto__.
                            //
                            JS_SP1.vstring->prototype = new(this) JSObject;
                            prototype.vobject = prim[ JS_OBJECT ]->prototype;
                            JS_SP1.vstring->prototype->StoreProperty( s___proto__, prototype );
                            }

                        JS_SP1.vstring->prototype->StoreProperty( arg_symbol, JS_SP2 );
                        break;

                    case JS_ARRAY:
                        if ( JS_SP1.varray->prototype == NULL )
                        {
                            prototype.type = JS_OBJECT;

                            // Create the prototype and set its __proto__.
                            //
                            JS_SP1.varray->prototype = new(this) JSObject;
                            prototype.vobject = prim[ JS_OBJECT ]->prototype;
                            JS_SP1.varray->prototype->StoreProperty( s___proto__, prototype );
                            }
                        JS_SP1.varray->prototype->StoreProperty( arg_symbol, JS_SP2 );
                        break;

                    case JS_FUNCTION:
                        if ( JS_SP1.vfunction->prototype == NULL )
                        {
                            prototype.type = JS_OBJECT;

                            // Create the prototype and set its __proto__.
                            //
                            JS_SP1.vfunction->prototype = new(this) JSObject;
                            prototype.vobject = prim[ JS_OBJECT ]->prototype;
                            JS_SP1.vfunction->prototype->StoreProperty( s___proto__, prototype );
                            }
                        JS_SP1.vfunction->prototype->StoreProperty( arg_symbol, JS_SP2 );
                        break;

                    default:
                        RaiseError( "VM[store_property]: illegal object" );
                    }
                }
            }

        JS_POP ();
        JS_POP ();
        }
    else
    {
        RaiseError( "VM[store_property]: illegal object" );
        }

    JS_MAYBE_GC ();
    break;

case JS_OPCODE_DELETE_PROPERTY:
//
//  Delete property 'Symbol' from object 'object'.
//  Push value undefined to the stack.
//
    arg_symbol = READ_ARG_SYMBOL();

    if ( JS_SP1.type == JS_BUILTIN )
    {
        // FIXME: It should be possible to apply delete instruction
        // to builtin objects.
        //
        RaiseError( "VM[delete_property]: not implemented yet for the builtin" );
        }

    else if ( JS_SP1.type == JS_OBJECT )
    {
        JS_SP1.vobject->DeleteProperty( arg_symbol );
        }

    else if ( JS_SP1.type == JS_NULL )
    {
        // Delete a property from an object in the with-chain. WITHCHAIN
        //
        RaiseError( "VM[delete_property]: not implemented yet for the with-chain objects" );
        }

    // The primitive language types.
    //
    // FIXME: Since we can't delete properties from builtins,
    // we can't delete them from the primitive language types.
    //
    else
    {
        RaiseError( "VM[delete_property]: illegal object" );
        }

    // The delete opcode returns an undefined value.
    //
    JS_SP1.type = JS_UNDEFINED;
    break;

case JS_OPCODE_LOAD_ARRAY:
//
//  Push the 'index':th item of object 'object' to the stack.
//
    if ( JS_SP2.type == JS_BUILTIN )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            RaiseError( "VM[load_array]: integer indexes not implemented yet for builtin" );
            }
        else if ( JS_SP1.type == JS_STRING )
        {
            // Intern the string
            //
            JSSymbol j = Intern( JS_SP1.vstring->data, JS_SP1.vstring->len );

            // The code below must be in sync with opcode `LoadProperty'
            //
            JSPropertyRC rc = JS_SP2.vbuiltin->info->OnProperty
            (
                JS_SP2.vbuiltin->instance_context, // void* instance context
                j,                                 // JSSymbol property
                false,                             // BOOL set
                builtin_result                     // JSVariant& result return
                );

            if ( JS_PROPERTY_UNKNOWN == rc )
            {
                if ( j == s_prototype )
                {
                    // Looking up the prototype.

                    builtin_result.type = JS_OBJECT;
                    if ( JS_SP2.vbuiltin->prototype )
                    {
                        // This is an instance.
                        builtin_result.vobject = JS_SP2.vbuiltin->prototype;
                        }
                    else
                    {
                        // This is a class.
                        builtin_result.vobject = JS_SP2.vbuiltin->info->prototype;
                        }
                    }
                else
                {
                    // Looking up stuffs from the prototype.

                    if ( JS_SP2.vbuiltin->prototype )
                    {
                        // An instance.
                        JS_SP2.vbuiltin->prototype->LoadProperty( j, builtin_result );
                        }
                    else
                    {
                        // A class.
                        JS_SP2.vbuiltin->info->prototype->LoadProperty( j, builtin_result );
                        }
                    }
                }

            JS_SP2 = builtin_result;
            JS_POP ();
            }
        else
        {
            RaiseError( "LoadArray: illegal array index type(%d)", JS_SP1.type );
            }
        }

    else if ( JS_SP2.type == JS_OBJECT )
    {
        JS_SP2.vobject->LoadArray( JS_SP1, JS_SP2 );
        JS_POP ();
        }

    else if ( JS_SP2.type == JS_ARRAY )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            if ( JS_SP1.vinteger < 0
                || JS_SP1.vinteger >= JS_SP2.varray->length )
            {
                JS_SP2.type = JS_UNDEFINED;
                }
            else
            {
                JS_SP2 = JS_SP2.varray->data[ JS_SP1.vinteger ];
                }
            JS_POP ();
            }
        else
        {
            RaiseError( "LoadArray: illegal array index type(%d)", JS_SP1.type );
            }
        }

    else if ( JS_SP2.type == JS_STRING )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            if ( JS_SP1.vinteger < 0
                || JS_SP1.vinteger >= JS_SP2.vstring->len)
            {
                RaiseError( "VM[load_array]: string index out of range" );
                }

            int ch = JS_SP2.vstring->data[ JS_SP1.vinteger ];
            JS_SP2.type = JS_INTEGER;
            JS_SP2.vinteger = ch;

            JS_POP ();
            }
        else
        {
            RaiseError( "VM[load_array]: illegal string index" );
            }
        }

    else
    {
        RaiseError( "VM[load_array]: illegal object" );
        }
    break;

case JS_OPCODE_STORE_ARRAY:
//
//  Store the value 'value' to the 'index':th position of object 'object'.
//
    if ( JS_SP2.type == JS_BUILTIN )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            RaiseError( "VM[store_array]: integer index not implemented yet for builtin" );
            }
        else if ( JS_SP1.type == JS_STRING )
        {
            // Intern the string.
            //
            JSSymbol j = Intern( JS_SP1.vstring->data, JS_SP1.vstring->len );

            // The code below msut be in sync with opcode `StoreProperty'.
            //
            JSPropertyRC rc = JS_SP2.vbuiltin->info->OnProperty
            (
                JS_SP2.vbuiltin->instance_context, // void* instance context
                j,                                 // JSSymbol property
                true,                              // BOOL set
                JS_SP( 3 )                         // JSVariant& result return
                );

            if ( JS_PROPERTY_UNKNOWN == rc )
            {
                if ( j == s_prototype )
                {
                    // Setting the prototype.
                    //
                    if ( JS_SP( 3 ).type != JS_OBJECT )
                    {
                        RaiseError( "VM[store_array]: illegal value for prototype" );
                        }

                    if ( JS_SP2.vbuiltin->prototype )
                    {
                        // Setting the instance's prototype.
                        JS_SP2.vbuiltin->prototype = JS_SP( 3 ).vobject;
                        }
                    else
                    {
                        // Setting the class' prototype.
                        JS_SP2.vbuiltin->info->prototype = JS_SP( 3 ).vobject;
                        }
                    }
                else
                {
                    // Setting stuff to the prototype.
                    //
                    if ( JS_SP2.vbuiltin->prototype )
                    {
                        // An instance.
                        JS_SP2.vbuiltin->prototype->StoreProperty( j, JS_SP( 3 ) );
                        }
                    else
                    {
                        // A class.
                        JS_SP2.vbuiltin->info->prototype->StoreProperty( j, JS_SP( 3 ) );
                        }
                    }
                }

            JS_POP_N( 3 );
            }

        else
        {
            RaiseError( "VM[store_array]: illegal array index" );
            }
        }

    else if ( JS_SP2.type == JS_OBJECT )
    {
        JS_SP2.vobject->StoreArray( JS_SP1, JS_SP( 3 ) );
        JS_POP_N( 3 );
        }

    else if ( JS_SP2.type == JS_ARRAY )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            if ( JS_SP1.vinteger < 0 )
            {
                RaiseError( "VM[store_array]: negative array index" );
                }

            if ( JS_SP1.vinteger >= JS_SP2.varray->length )
            {
                JS_SP2.ExpandArray( this, JS_SP1.vinteger + 1 );
                }

          JS_SP2.varray->data[ JS_SP1.vinteger ] = JS_SP( 3 );
          JS_POP_N( 3 );
        }
        else
        {
            RaiseError( "VM[store_array]: illegal array index" );
            }
        }

    else if ( JS_SP2.type == JS_STRING )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            if ( JS_SP2.vstring->flags & JSSTRING_STATIC )
            {
                RaiseError( "VM[store_array]: static string" );
                }

            if ( JS_SP2.vstring->flags & JSSTRING_DONT_GC )
            {
                RaiseError( "VM[store_array]: dont GC string" );
                }

            if ( JS_SP1.vinteger < 0 )
            {
                RaiseError( "VM[store_array]: negative string index" );
                }

            if ( JS_SP( 3 ).type != JS_INTEGER )
            {
                RaiseError( "VM[store_array]: non-integer value to store into string" );
                }

            if ( JS_SP1.vinteger >= JS_SP2.vstring->len )
            {
                // Expand the string.
                //
                JS_SP2.vstring->data = (PSTR)Realloc( JS_SP2.vstring->data,
                                                       JS_SP1.vinteger + 2 );

                JS_SP2.vstring->data[ JS_SP1.vinteger + 1 ] = 0;

                // Fill the gap with ' '.
                //
                while( JS_SP2.vstring->len <= JS_SP1.vinteger )
                {
                    JS_SP2.vstring->data[ JS_SP2.vstring->len++ ] = ' ';
                    }
                }

            JS_SP2.vstring->data[ JS_SP1.vinteger ] = uint8( JS_SP( 3 ).vinteger );

            JS_POP_N( 3 );
            }

        else
        {
            RaiseError( "VM[store_array]: illegal string index" );
            }
        }

    else
    {
        RaiseError( "VM[store_array]: illegal object" );
        }

    JS_MAYBE_GC ();
    break;

case JS_OPCODE_DELETE_ARRAY:
//
//  Delete the 'index':th property of object 'object'.
//  Push value undefined to the stack.
//
    if ( JS_SP2.type == JS_BUILTIN )
    {
        RaiseError( "VM[delete_array]: not implemented yet for the builtin" );
        }
    else if ( JS_SP2.type == JS_OBJECT )
    {
        JS_SP2.vobject->DeleteArray( JS_SP1 );
        JS_POP ();
        }
    else if ( JS_SP2.type == JS_ARRAY )
    {
        if ( JS_SP1.type == JS_INTEGER )
        {
            if ( 0 <= JS_SP1.vinteger
                && JS_SP1.vinteger < JS_SP2.varray->length )
            {
                JS_SP2.varray->data[ JS_SP1.vinteger ].type = JS_UNDEFINED;
                }

            JS_POP ();
            }
        else
        {
            RaiseError( "VM[delete_array]: illegal array index" );
            }
        }
    else
    {
        RaiseError( "VM[delete_array]: illegal object" );
        }

    // The delete opcode returns an undefined value.
    //
    JS_SP1.type = JS_UNDEFINED;
    break;

case JS_OPCODE_NTH:
//
//  Push the 'integer':th item of object 'any' to the stack.
//  Push a boolean success status that tells whether the object
//  any did contain integer:th item.
//
    if ( JS_SP2.type == JS_STRING )
    {
        if ( JS_SP1.vinteger < 0
            || JS_SP1.vinteger >= JS_SP2.vstring->len )
        {
            JS_SP2.type = JS_UNDEFINED;

            JS_SP1.type = JS_BOOLEAN;
            JS_SP1.vboolean = 0;
            }
        else
        {
            JS_SP2.type = JS_INTEGER;
            JS_SP2.vinteger = JS_SP2.vstring->data[ JS_SP1.vinteger ];

            JS_SP1.type = JS_BOOLEAN;
            JS_SP1.vboolean = 1;
            }
        }

    else if ( JS_SP2.type == JS_ARRAY )
    {
        if ( JS_SP1.vinteger < 0
            || JS_SP1.vinteger >= JS_SP2.varray->length )
        {
            JS_SP2.type = JS_UNDEFINED;

            JS_SP1.type = JS_BOOLEAN;
            JS_SP1.vboolean = 0;
            }
        else
        {
            JS_SP2 = JS_SP2.varray->data[ JS_SP1.vinteger ];

            JS_SP1.type = JS_BOOLEAN;
            JS_SP1.vboolean = 1;
            }
        }

    else if ( JS_SP2.type == JS_OBJECT )
    {
        JSInt32 i = JS_SP2.vobject->Nth( JS_SP1.vinteger, JS_SP2 );
        JS_SP1.type = JS_BOOLEAN;
        JS_SP1.vboolean = i;
        }

    else if ( JS_SP2.type == JS_BUILTIN )
    {
        JSVariant arg[ 2 ];
        arg[ 0 ].type = JS_INTEGER;
        arg[ 0 ].vinteger = 1;
        arg[ 1 ] = JS_SP1;

        JSPropertyRC rc = JS_SP2.vbuiltin->info->OnMethod
        (
            JS_SP2.vbuiltin->instance_context, // void* instance context
            s___Nth__,                         // JSSymbol method
            JS_SP2,                            // JSVariant& result return
            arg                                // JSVariant[] arguments
            );

        JS_SP1.type = JS_BOOLEAN;
        JS_SP1.vboolean = rc == JS_PROPERTY_FOUND;
        }

    else
    {
        RaiseError( "VM[nth]: illegal object" );
        }
    break;

case JS_OPCODE_CMP_EQ:
//
//  Compare the two objects 'any1', 'any2' for equality and push
//  a boolean result code to the stack.
//
    JS_OPCODE_CMP_EQ( ==, 1 );
    break;

case JS_OPCODE_CMP_NE:
//
//  Compare the two objects 'any1', 'any2' for inequality and push
//  a boolean result code to the stack.
//
    JS_OPCODE_CMP_EQ( !=, 0 );
    break;

case JS_OPCODE_CMP_LT:
//
//  Compare whether object 'any1' is smaller than object 'any2'.
//  Push a boolean result code to the stack.
//
    JS_OPCODE_CMP_REL( < );
    break;

case JS_OPCODE_CMP_GT:
//
//  Compare whether object 'any1' is greater than object 'any2'.
//  Push a boolean result code to the stack.
//
    JS_OPCODE_CMP_REL( > );
    break;

case JS_OPCODE_CMP_LE:
//
//  Compare whether object 'any1' is smaller than, or equal to object 'any2'.
//  Push a boolean result code to the stack.
//
    JS_OPCODE_CMP_REL( <= );
    break;

case JS_OPCODE_CMP_GE:
//
//  Compare whether object 'any1' is greater than, or equal to object 'any2'.
//  Push a boolean result code to the stack.
//
    JS_OPCODE_CMP_REL( >= );
    break;

case JS_OPCODE_CMP_SEQ:
//
//  Compare the two objects 'any1', 'any2' for strict equality and
//  push a boolean result code to the stack.
//
    JS_OPCODE_CMP_SEQ( ==, 1 );
    break;

case JS_OPCODE_CMP_SNE:
//
//  Compare the two objects 'any1', 'any2' for strict inequality and
//  push a boolean result code to the stack.
//
    JS_OPCODE_CMP_SEQ( !=, 0 );
    break;

case JS_OPCODE_ADD_1_I:
//
//  Add integer number 1 to the top most item in the stack.
//  The opcode assumes that the topmost item is an integer number.
//
    assert( JS_SP1.type == JS_INTEGER );
    JS_SP1.vinteger++;
    break;

case JS_OPCODE_ADD_2_I:
//
//  Add integer number 2 to the top most item in the stack.
//  The opcode assumes that the topmost item is an integer number.
//
    assert( JS_SP1.type == JS_INTEGER );
    JS_SP1.vinteger += 2;
    break;

case JS_OPCODE_SUB:
//
//  Substract object 'any2' from object 'any1' and push the result to the stack.
//
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        JS_SP2.vinteger -= JS_SP1.vinteger;
        }
    else
    {
        JSVariant left = JS_SP2;

        if ( ! left.IsNumber () )
        {
            JS_SP2.ToNumber( left );
            }

        JSVariant right = JS_SP1;

        if ( ! right.IsNumber () )
        {
            JS_SP1.ToNumber( right );
            }

        if ( left.type == JS_NAN || right.type == JS_NAN )
        {
            JS_SP2.type = JS_NAN;
            }
        else if ( left.type == JS_INTEGER )
        {
            if ( right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_INTEGER;
                JS_SP2.vinteger = left.vinteger - right.vinteger;
                }
            else
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = double( left.vinteger ) - right.vfloat;
                }
            }
        else
        {
            if ( right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = left.vfloat - double( right.vinteger );
                }
            else
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = left.vfloat - right.vfloat;
                }
            }
        }

    JS_POP ();
    break;

case JS_OPCODE_ADD:
//
//  Add object 'any2' to object 'any1' and push the result to the stack.
//
    if ( JS_SP2.type == JS_STRING || JS_SP1.type == JS_STRING )
    {
        JSVariant cvt;

        char* d2;
        unsigned int d2_len;
        if ( JS_SP2.type == JS_STRING )
        {
            d2 = JS_SP2.vstring->data;
            d2_len = JS_SP2.vstring->len;
            }
        else
        {
            JS_SP2.ToString( this, cvt );
            d2 = cvt.vstring->data;
            d2_len = cvt.vstring->len;
            }

        char* d1;
        unsigned int d1_len;
        if ( JS_SP1.type == JS_STRING )
        {
            d1 = JS_SP1.vstring->data;
            d1_len = JS_SP1.vstring->len;
            }
        else
        {
            JS_SP1.ToString( this, cvt );
            d1 = cvt.vstring->data;
            d1_len = cvt.vstring->len;
            }

        unsigned int nlen = d2_len + d1_len;
        char* ndata = new(this) char[ nlen + 1 ];
        memcpy( ndata, d2, d2_len );
        memcpy( ndata + d2_len, d1, d1_len );
        ndata[ nlen ] = 0;

        JS_SP2.MakeStaticString( this, ndata, nlen );
        JS_SP2.vstring->flags = JSSTRING_NORMAL; // clear 'static' flag
        JS_POP ();
        JS_MAYBE_GC ();
        }

    else if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        JS_SP2.vinteger += JS_SP1.vinteger;
        JS_POP ();
        }

    else
    {
        JSVariant left = JS_SP2;

        if ( ! left.IsNumber () )
        {
            JS_SP2.ToNumber( left );
            }

        JSVariant right = JS_SP1;

        if ( ! right.IsNumber () )
        {
            JS_SP1.ToNumber( right );
            }

        if ( left.type == JS_NAN || right.type == JS_NAN )
        {
            JS_SP2.type = JS_NAN;
            }
        else if ( left.type == JS_INTEGER )
        {
            if ( right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_INTEGER;
                JS_SP2.vinteger = left.vinteger + right.vinteger;
                }
            else
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = double( left.vinteger ) + right.vfloat;
                }
            }
        else
        {
            if ( right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = left.vfloat + double( right.vinteger );
                }
            else
            {
                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = left.vfloat + right.vfloat;
                }
            }

        JS_POP ();
        }
    break;

case JS_OPCODE_MUL:
//
//  Multiply object 'any1' with object 'any2' and push the result to the stack.
//
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        JS_SP2.vinteger *= JS_SP1.vinteger;
        }
    else
    {
        JSVariant left = JS_SP2;

        if ( ! left.IsNumber () )
        {
            JS_SP2.ToNumber( left );
            }

        JSVariant right = JS_SP1;

        if ( ! right.IsNumber () )
        {
            JS_SP1.ToNumber( right );
            }

        if ( left.type == JS_NAN || right.type == JS_NAN )
        {
            JS_SP2.type = JS_NAN;
            }
        else if ( left.type == JS_INTEGER )
        {
            if ( right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_INTEGER;
                JS_SP2.vinteger = left.vinteger * right.vinteger;
            }
            else
            {
                if ( left.vinteger == 0
                    && ( right.IsPositiveInfinity () || right.IsNegativeInfinity () ) )
                {
                    JS_SP2.type = JS_NAN;
                    }
                else
                {
                    JS_SP2.type = JS_FLOAT;
                    JS_SP2.vfloat = double( left.vinteger ) * right.vfloat;
                    }
                }
            }
        else
        {
            if ( ( left.IsPositiveInfinity () || left.IsNegativeInfinity () )
                && ((right.type == JS_INTEGER && right.vinteger == 0 )
                    || (right.type == JS_FLOAT && right.vfloat == 0.0 ) ) )
            {
                JS_SP2.type = JS_NAN;
                }
            else
            {
                JS_SP2.type = JS_FLOAT;

                if ( right.type == JS_INTEGER )
                {
                    JS_SP2.vfloat = left.vfloat * double( right.vinteger );
                    }
                else
                {
                    JS_SP2.vfloat = left.vfloat * right.vfloat;
                    }
                }
            }
        }

    JS_POP ();
    break;

case JS_OPCODE_DIV:
//
//  Divide object 'any1' with object 'any2' and push the result to the stack.
//
{
    bool fNAN = false;

    JSVariant n = JS_SP2;

    if ( ! n.IsNumber () )
    {
        // Convert divident to float.
        //
        JS_SP2.ToNumber( n );
        }

    int l_inf = 0;
    double left;

    switch( n.type )
    {
        case JS_INTEGER:
            left = double( n.vinteger );
            break;

        case JS_FLOAT:
            left = n.vfloat;
            l_inf = ( n.IsPositiveInfinity () || n.IsNegativeInfinity () );
            break;

        case JS_NAN:
        default:
            fNAN = true;
        }

    n = JS_SP1;

    if ( ! n.IsNumber () )
    {
        // Convert divisor to float.
        //
        JS_SP2.ToNumber( n );
        }

    int r_inf = 0;
    double right;

    switch( n.type )
    {
        case JS_INTEGER:
            right = double( n.vinteger );
            break;

        case JS_FLOAT:
            right = n.vfloat;
            r_inf = ( n.IsPositiveInfinity () || n.IsNegativeInfinity () );
            break;

        case JS_NAN:
        default:
            fNAN = true;
        }

    // Do the division.
    //
    JS_POP ();

    if ( fNAN || ( l_inf && r_inf ) )
    {
        JS_SP1.type = JS_NAN;
        }
    else
    {
        if ( l_inf && right == 0.0 )
        {
            // <left> is already an infinity.
            JS_SP1.type = JS_FLOAT;
            JS_SP1.vfloat = left;
            }
        else if ( left == 0.0 && right == 0.0 )
        {
            JS_SP1.type = JS_NAN;
            }
        else
        {
            JS_SP1.type = JS_FLOAT;
            JS_SP1.vfloat = left / right;
            }
        }
    }
    break;

case JS_OPCODE_MOD:
//
//  Count object 'integer1' modulo object 'integer2' and push the result to the stack.
//
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        if ( JS_SP1.vinteger == 0 )
        {
            JS_SP2.type = JS_NAN;
            }
        else
        {
            JS_SP2.vinteger %= JS_SP1.vinteger;
            }
        }
    else
    {
        JSVariant left = JS_SP2;

        if ( ! left.IsNumber () )
        {
            JS_SP2.ToNumber( left );
            }

        JSVariant right = JS_SP1;

        if ( ! right.IsNumber () )
        {
            JS_SP1.ToNumber( right );
            }

        if ( left.type == JS_NAN || right.type == JS_NAN )
        {
            JS_SP2.type = JS_NAN;
            }
        else if ( left.IsPositiveInfinity ()
               || left.IsNegativeInfinity ()
               || ( ( right.type == JS_INTEGER && right.vinteger == 0 )
                   || ( right.type == JS_FLOAT && right.vfloat == 0.0 ) ) )
        {
            JS_SP2.type = JS_NAN;
            }
        else if ( right.IsPositiveInfinity ()
               || right.IsNegativeInfinity () )
        {
            JS_SP2 = left;
            }
        else if ( ( left.type == JS_INTEGER && left.vinteger == 0 )
               || ( left.type == JS_FLOAT && left.vfloat == 0.0 ) )
        {
            JS_SP2 = left;
            }
        else
        {
            if ( left.type == JS_INTEGER && right.type == JS_INTEGER )
            {
                JS_SP2.type = JS_INTEGER;
                JS_SP2.vinteger = left.vinteger % right.vinteger;
                }
            else
            {
                double ld, rd;

                if ( left.type == JS_INTEGER )
                {
                    ld = double( left.vinteger );
                    }
                else
                {
                    ld = left.vfloat;
                    }

                if ( right.type == JS_INTEGER )
                {
                    rd = double( right.vinteger );
                    }
                else
                {
                    rd = right.vfloat;
                    }

                int full = int( ld / rd );

                JS_SP2.type = JS_FLOAT;
                JS_SP2.vfloat = ld - ( full * rd );
                }
            }
        }

    JS_POP ();
    break;

case JS_OPCODE_NEG:
//
//  Negate object 'any' and push the result to the stack.
//
    if ( JS_SP1.type == JS_INTEGER )
    {
        JS_SP1.vinteger = -JS_SP1.vinteger;
        }
    else if ( JS_SP1.type == JS_FLOAT )
    {
        JS_SP1.vfloat = - JS_SP1.vfloat;
        }
    else if ( JS_SP1.type == JS_NAN )
    {
        }
    else
    {
        JSVariant cvt;
        JS_SP1.ToNumber( cvt );

        JS_SP1.type = cvt.type;
        switch ( cvt.type )
        {
            case JS_INTEGER:
                JS_SP1.vinteger = - cvt.vinteger;
                break;

            case JS_FLOAT:
                JS_SP1.vfloat = - cvt.vfloat;
                break;

            case JS_NAN:
            default:
                // Nothing here.
                break;
            }
        }
    break;

case JS_OPCODE_NOT:
//
//  Perform a not operation on object 'any' and push the result to the stack.
//
    JS_SP1.vboolean = JS_SP1.IsFalse ();
    JS_SP1.type = JS_BOOLEAN;
    break;

case JS_OPCODE_AND:
//
//  Perform a bitwise and operation between objects 'any1' and 'any2' and
//  push the result to the stack.
//
    JS_OPCODE_BINARY( & );
    break;

case JS_OPCODE_OR:
//
//  Perform a bitwise or operation between objects 'any1' and 'any2' and
//  push the result to the stack.
//
    JS_OPCODE_BINARY( | );
    break;

case JS_OPCODE_XOR:
//
//  Perform a bitwise xor operation between objects 'any1' and 'any2' and
//  push the result to the stack.
//
    JS_OPCODE_BINARY( ^ );
    break;

case JS_OPCODE_SHIFT_LEFT:
//
//  Shift integer number 'integer1' left 'integer2' bits.
//  Push the result value to the stack.
//
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        JS_SP2.vinteger = JSInt32( JS_SP2.vinteger ) << JSUInt32( JS_SP1.vinteger );
        JS_POP ();
        }
    else
    {
        JSInt32 left;
        JSUInt32 right;

        left = JS_SP2.ToInt32 ();
        right = (JSUInt32) JS_SP1.ToInt32 ();

        JS_SP2.vinteger = left << right;
        JS_SP2.type = JS_INTEGER;
        JS_POP ();
        }
    break;

case JS_OPCODE_SHIFT_RIGHT:
//
//  Shift integer number 'integer1' right 'integer2' bits.
//  Push the result value to the stack.
//
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER )
    {
        JS_SP2.vinteger = JSInt32( JS_SP2.vinteger ) >> JSUInt32( JS_SP1.vinteger );
        JS_POP ();
        }
    else
    {
        JSInt32 left;
        JSUInt32 right;

        left = JS_SP2.ToInt32 ();
        right = JSUInt32( JS_SP1.ToInt32 () );

        JS_SP2.vinteger = left >> right;
        JS_SP2.type = JS_INTEGER;
        JS_POP ();
        }
    break;

case JS_OPCODE_SHIFT_RRIGHT:
//
//  Shift integer number 'integer1' right 'integer2' bits with zero fill.
//  Push the result value to the stack.
//
{
    JSInt32 left;
    JSUInt32 right;

    left = JS_SP2.ToInt32 ();
    right = JSUInt32( JS_SP1.ToInt32 () );

    if ( right > 0 )
    {
        JS_SP2.vinteger = ( left & 0x7fffffff ) >> right;
        }
    else
    {
        JS_SP2.vinteger = left;
        }

    JS_SP2.type = JS_INTEGER;
    JS_POP ();
    }
    break;

case JS_OPCODE_HALT:
//
//  Halt the virtual machine. The program execution stops
//  immediately and "sleep forever" loop is executed for debugging
//  purposes.
//
    s_stderr->PrintfLn( "VM[halt]: Stopped." );

    while( ! should_terminate )
    {
        sleep( 5 );
        }

    break;

case JS_OPCODE_DONE:
//
//  The execution of the byte-code is finished and the control returns
//  to the calling C-function.
//
    goto ALL_DONE;

case JS_OPCODE_IFFALSE:
//
//  If the topmost item in the stack has boolean value 'false',
//  adjust the program counter with relative offset Int32.
//
    arg_label = READ_ARG_LABEL();

    if ( JS_SP1.IsFalse () )
    {
        PC = arg_label;
        // local bytecode segment jump; don't change BP
        }

    JS_POP ();
    break;

case JS_OPCODE_IFTRUE:
//
//  If the topmost item in the stack has boolean value 'true',
//  adjust the program counter with relative offset Int32.
//
    arg_label = READ_ARG_LABEL();

    if ( JS_SP1.IsTrue () )
    {
        PC = arg_label;
        // local bytecode segment jump; don't change BP
        }

    JS_POP ();
    break;

case JS_OPCODE_IFFALSE_B:
//
//  If the topmost item in the stack is false, adjust the program counter with
//  relative offset 'Int32'. The opcode assumes that the topmost item is a boolean
//  value.
//
    arg_label = READ_ARG_LABEL();

    if ( ! JS_SP1.vboolean )
    {
        PC = arg_label;
        // local bytecode segment jump; don't change BP
        }

    JS_POP ();
    break;

case JS_OPCODE_IFTRUE_B:
//
//  If the topmost item in the stack is true, adjust the program counter with
//  relative offset 'Int32'. The opcode assumes that the topmost item is a boolean
//  value.
//
    arg_label = READ_ARG_LABEL();

    if ( JS_SP1.vboolean )
    {
        PC = arg_label;
        // local bytecode segment jump; don't change BP
        }

    JS_POP ();
    break;

case JS_OPCODE_CALL_METHOD:
//
//  Call method 'Symbol' in the object 'object'.
//  Push the result of the method call to the stack.
//
    arg_symbol = READ_ARG_SYMBOL();

    if ( JS_SP1.type == JS_BUILTIN )
    {
        JSPropertyRC rc = JS_SP1.vbuiltin->info->OnMethod
        (
            JS_SP1.vbuiltin->instance_context, // void* instance context
            arg_symbol,                        // JSSymbol method
            builtin_result,                    // JSVariant& result return
            &JS_SP2                            // JSVariant[] arguments
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            RaiseError( "VM[call_method]/builtin: unknown method: '%s'", Symname( arg_symbol ) );
            }

        JS_SP0 = builtin_result;
        JS_PUSH ();
        JS_MAYBE_GC ();
        }
    else if ( JS_SP1.type == JS_OBJECT )
    {
        JSVariant method;

        if ( JS_PROPERTY_FOUND
            == JS_SP1.vobject->LoadProperty( arg_symbol, method ) )
        {
            // The property has been defined in the object.

            if ( method.type != JS_FUNCTION )
            {
                RaiseError( "VM[call_method]/object: unknown method: '%s'", Symname( arg_symbol ) );
                }

            // And once again. We must do a subroutine call here.
            //
            JS_SUBROUTINE_CALL( method.vfunction );
            }
        else
        {
            // Let our prototype handle this.
            goto _OP_CALL_METHOD_TRY_PROTO;
            }
        }
    else if ( prim[ JS_SP1.type ] )
    {
        // The primitive language types.
_OP_CALL_METHOD_TRY_PROTO:

        JSPropertyRC rc = prim[ JS_SP1.type ]->OnMethod
        (
            &JS_SP1,          // void* instance context
            arg_symbol,       // JSSymbol method
            builtin_result,   // JSVariant& result return
            &JS_SP2           // JSVariant[] arguments
            );

        if ( JS_PROPERTY_UNKNOWN == rc )
        {
            JSVariant method;
            int result = JS_PROPERTY_UNKNOWN;

            // Let's see if we can find it from the prototype.
            //
            if ( JS_SP1.type == JS_STRING && JS_SP1.vstring->prototype )
            {
                result = JS_SP1.vstring->prototype->LoadProperty( arg_symbol, method );
                }
            else if ( JS_SP1.type == JS_ARRAY && JS_SP1.varray->prototype )
            {
                result = JS_SP1.varray->prototype->LoadProperty(arg_symbol, method );
                }
            else if ( JS_SP1.type == JS_FUNCTION && JS_SP1.vfunction->prototype )
            {
                result = JS_SP1.vfunction->prototype->LoadProperty( arg_symbol, method );
                }

            if ( result == JS_PROPERTY_UNKNOWN || method.type != JS_FUNCTION )
            {
                RaiseError( "VM[call_method]/primitive: unknown method: '%s'", Symname( arg_symbol ) );
                }

            // Do the subroutine call.
            //
            JS_SUBROUTINE_CALL( method.vfunction );
            }
        else
        {
            JS_SP0 = builtin_result;
            JS_PUSH ();
            JS_MAYBE_GC ();
            }
        }
    else
    {
        RaiseError( "VM[call_method]: illegal object; type (%d)", JS_SP1.type );
        }
    break;

case JS_OPCODE_JMP:
//
//  Adjust program counter with relative offset 'Int32',
//  e.g. jump to relative position PC + 'Int32'.
//
    arg_label = READ_ARG_LABEL();
    PC = arg_label;
    // local bytecode segment jump; don't change BP
    break;

case JS_OPCODE_JSR:
//
//  Jump to subroutine 'Function' and push the result of the subroutine
//  call to the stack. The opcode will *not* process the with-chain.
//  Used to call the global method.
//
{
    // Fetch the function to our local variable
    //
    JSVariant function = JS_SP1;

    // Reset the 'this' to null
    //
    JS_SP1.type = JS_NULL;

    if ( function.type == JS_BUILTIN )
    {
        function.vbuiltin->info->OnGlobalMethod
        (
            function.vbuiltin->instance_context, // void* instance context
            builtin_result,                      // JSVariant& result return
            &JS_SP2                              // JSVariant[] arguments
            );

        JS_SP0 = builtin_result;
        JS_PUSH ();
        }
    else if ( function.type == JS_FUNCTION )
    {
        JS_SUBROUTINE_CALL( function.vfunction );
        }
    else if ( function.type == JS_UNDEFINED )
    {
        RaiseError( "JSR: function is not defined" );
        }
    else
    {
        RaiseError( "JSR: illegal function object" );
        }
    }
    break;

case JS_OPCODE_JSR_W:
//
//  Jump to subroutine 'Function' and push the result of the subroutine
//  call to the stack. The opcode will lookup the method 'Symbol' from the
//  currently active with-chain. If the method is not found, the argument
//  'Function' is used
//
{
    arg_symbol = READ_ARG_SYMBOL();

    bool found = false;

    // Loop over the with-chain.
    //
    if ( JS_WITH_CHAINP.cNode > 0 )
    {
        for ( JSInt32 i = JS_WITH_CHAINP.cNode - 1; i >= 0; i-- )
        {
            JSVariant *w = &JS_WITH_CHAINP.node[ i ];
            JSPropertyRC result = JS_PROPERTY_UNKNOWN;

            if ( w->type == JS_BUILTIN)
            {
                result = w->vbuiltin->info->OnMethod
                (
                    w->vbuiltin->instance_context, // void* instance context
                    arg_symbol,                    // JSSymbol method
                    builtin_result,                // JSVariant& result return
                    &JS_SP2                        // JSVariant[] arguments
                    );

                JS_MAYBE_GC ();

                if ( JS_PROPERTY_FOUND == result )
                {
                    JS_SP0 = builtin_result;
                    JS_PUSH ();
                    }
                }
            else if ( w->type == JS_OBJECT )
            {
                JSVariant method;

                w->vobject->LoadProperty( arg_symbol, method );

                if ( method.type == JS_FUNCTION )
                {
                    result = JS_PROPERTY_FOUND;

                    // The object defines the method. Do a subroutine call.

                    // First: replace the null `this' with `w'.
                    //
                    JS_SP1 = *w;

                    // Then, do the normal subroutine call.
                    //
                    JS_SUBROUTINE_CALL( method.vfunction );
                    }
                }
            else
            {
                RaiseError( "VM[jsr_w]: corrupted with-chain" );
                }

            if ( result == JS_PROPERTY_FOUND )
            {
                found = true;
                break;
                }
            }
        }

    if ( ! found )
    {
        // Call the global method.
        //
        JSVariant function = *arg_symbol;

        // Reset the 'this' to null
        //
        JS_SP1.type = JS_NULL;

        if ( function.type == JS_BUILTIN )
        {
            function.vbuiltin->info->OnGlobalMethod
            (
                function.vbuiltin->instance_context, // void* instance context
                builtin_result,                      // JSVariant& result return
                &JS_SP2                              // JSVariant[] arguments
                );

            JS_SP0 = builtin_result;
            JS_PUSH ();
            }
        else if ( function.type == JS_FUNCTION )
        {
            JS_SUBROUTINE_CALL( function.vfunction );
            }
        else
        {
            RaiseError( "JSR: symbol `%s' is undefined as function", Symname( arg_symbol ) );
            }
        }
    }
    break;

case JS_OPCODE_RETURN:
//
//  Return from a subroutine with value result.
//
{
    if ( FP[ 0 ].frame_ptr == NULL )
    {
        // Return from the global scope.
        goto ALL_DONE;
        }

    // STACKFRAME

    // Check if the stack has been modified by min_args.
    //
    if ( JS_ARGS_FIXP.delta > 0 )
    {
        JSInt32 delta = JS_ARGS_FIXP.delta;

        // Yes it was. 
        // Truncate it back to the state where it was before the call.
        //
        memmove( &JS_SP1 + delta, &JS_SP1,
                 ( FP - &JS_SP0 + JS_ARGS_FIXP.argc ) * sizeof( JSVariant ) );

        SP += delta;
        FP += delta;
        }

    // Set PC to the saved return address.
    //
    if ( FP[ -4 ].type != JS_INSTR_PTR )
    {
        RaiseError( "VM[return]: can't find saved return address" );
        }

    PC = FP[ -4 ].instr_ptr;

    // Restore the imported-symbols context
    //
    if ( FP[ -3 ].type != JS_BASE_PTR )
    {
        RaiseError( "VM[return]: can't find saved base pointer" );
        }

    BP = FP[ -3 ].base_ptr;

    // Save old frame pointer.
    //
    if ( FP[ 0 ].type != JS_FRAME_PTR )
    {
        RaiseError( "VM[return]: can't find saved frame pointer" );
        }

    JSVariant* old_fp = FP[ 0 ].frame_ptr;

    // Put return value to its correct location.
    //
    FP[ 0 ] = JS_SP1;

    // Restore SP.
    //
    SP = FP - 1;

    // Restore frame pointer.
    //
    FP = old_fp;
    }
    break;

case JS_OPCODE_TYPEOF:
//
//  Push the type name of object any to the stack.
//
{
    char* typeof_name = "";     // Initialized to make compiler quiet.

    switch ( JS_SP1.type )
    {
        case JS_UNDEFINED:
            typeof_name = "undefined";
            break;

        case JS_NULL:
            typeof_name = "object";
            break;

        case JS_BOOLEAN:
            typeof_name = "boolean";
            break;

        case JS_INTEGER:
        case JS_FLOAT:
        case JS_NAN:
            typeof_name = "number";
            break;

        case JS_STRING:
            typeof_name = "string";
            break;

        case JS_ARRAY:
            typeof_name = "#array";
            break;

        case JS_OBJECT:
            typeof_name = "object";
            break;

        case JS_BUILTIN:
            typeof_name = "#builtin";
            break;

        case JS_FUNCTION:
            typeof_name = "function";
            break;

        case JS_INSTR_PTR:
            typeof_name = "#instrptr";
            break;

        case JS_FRAME_PTR:
            typeof_name = "#frameptr";
            break;

        case JS_WITH_CHAIN:
            typeof_name = "#withchain";
            break;

        case JS_ARGS_FIX:
            typeof_name = "#argsfix";
            break;
        }

    JS_SP1.MakeStaticString( this, typeof_name );
    JS_MAYBE_GC ();
    }
    break;

case JS_OPCODE_NEW:
//
//  Create an instance of object 'object' and call its constructor function.
//  Push the result from the constructor and the new instance to the stack.
//  The return value of the constructor is discarded.
//
    // Check object.
    //
    if ( JS_SP1.type == JS_BUILTIN )
    {
        JS_SP1.vbuiltin->info->OnNew
        (
            &JS_SP2, // JSVariant[] arguments
            JS_SP1   // JSVariant& result return
            );

        // Push a dummy return value for the constructor.  This is ignored.
        //
        JS_SP0.type = JS_UNDEFINED;
        JS_PUSH ();
        }

    else if ( JS_SP1.type == JS_FUNCTION )
    {
        JSObject *obj;
        JSVariant prototype;

        // The prototype is an object.
        //
        prototype.type = JS_OBJECT;

        // Create the prototype for the function, if it is not defined.
        //
        if ( JS_SP1.vfunction->prototype == NULL )
        {
            JS_SP1.vfunction->prototype = new(this) JSObject;

            // Set its __proto__ to point to Object's prototype. 
            //
            prototype.vobject = prim[ JS_OBJECT ]->prototype;
            JS_SP1.vfunction->prototype->StoreProperty( s___proto__, prototype );
            }

        // Allocate a new object and set its prototype.

        obj = new(this) JSObject;

        prototype.vobject = JS_SP1.vfunction->prototype;
        obj->StoreProperty( s___proto__, prototype );

        // Basicly we do a jsr to the function given in JS_SP1.  But first,
        // we must set `this' pointer to the correct value.  See `jsr' for
        // the details.
        //
      
        JSVariant foo = JS_SP1;

        // Replace func with the new object.
        //
        JS_SP1.type = JS_OBJECT;
        JS_SP1.vobject = obj;

        JS_SUBROUTINE_CALL( foo.vfunction );
        }

    else if ( prim[ JS_SP1.type ] )
    {
        // The primitive language types.
        //
        prim[ JS_SP1.type ]->OnNew
        (
            &JS_SP2,  // JSVariant[] arguments
            JS_SP1    // JSVariant& result return
            );

        JS_PUSH ();
        }

    else
    {
        RaiseError( "VM[new]: ilegal object type(%d)", JS_SP1.type );
        }

  JS_MAYBE_GC ();
  break;

case JS_OPCODE_WITH_PUSH:
//
//  Push object 'object' to the function's with-lookup chain.
//
    if ( JS_SP1.type != JS_OBJECT && JS_SP1.type != JS_BUILTIN )
    {
        RaiseError( "VM[with_push]: illegal object" );
        }

    if ( JS_WITH_CHAINP.cNode == 0 )
    {
        JS_WITH_CHAINP.cNode = 1;
        JS_WITH_CHAINP.node = new(this) JSVariant;
        JS_WITH_CHAINP.node[ 0 ] = JS_SP1;
        }
    else
    {
        int cNode = JS_WITH_CHAINP.cNode;
        JSVariant* wp = JS_WITH_CHAINP.node;

        JS_WITH_CHAINP.cNode = cNode + 1;
        JS_WITH_CHAINP.node = (JSVariant*)Realloc( wp, ( cNode + 1 ) * sizeof( JSVariant ) );
        JS_WITH_CHAINP.node[ cNode ] = JS_SP1;
        }

    JS_POP ();
    break;

case JS_OPCODE_WITH_POP:
//
//  Pop 'Int32' objects from the function's with-lookup chain.
//
{
    arg_int32 = READ_ARG_INT32();

    if ( JS_WITH_CHAINP.node == NULL || JS_WITH_CHAINP.cNode < arg_int32 )
    {
        RaiseError( "VM[with_pop]: with-chain stack underflow" );
        }

    JS_WITH_CHAINP.cNode -= arg_int32;
    }
    break;

case JS_OPCODE_TRY_PUSH:
//
//  Push a try-frame with a given catch block label to the
//  virtual machine's try-chain.
//
{
    arg_label = READ_ARG_LABEL();

    error_handler = NEW JSErrorHandlerFrame( error_handler, SP, FP, arg_label, BP );
    }
    break;

case JS_OPCODE_TRY_POP:
//
//  Pop 'Int32' frames from the virtual machine's try-chain.
//
    arg_int32 = READ_ARG_INT32();

    for ( ; arg_int32 > 0; arg_int32-- )
    {
        JSErrorHandlerFrame* ehf = error_handler;
        error_handler = error_handler->next;
        delete ehf;
        }
    break;

case JS_OPCODE_THROW:
//
//  Throw an exception with value any.
//
{
    JSErrorHandlerFrame* ehf = error_handler;

    if ( ehf->SP == NULL )
    {
        // We are jumping to the C-toplevel.  Convert our thrown value
        // to string and store it to the error.
        //
        JSVariant cvt;
        JS_SP1.ToString( this, cvt );

        int len = cvt.vstring->len;
        if ( len + 1 > sizeof( error ) )
        {
            len = sizeof( error ) - 1;
            }

        memcpy( error, cvt.vstring->data, len );
        error[ len ] = '\0';
        }
    else
    {
        ehf->thrown = JS_SP1;
        }

    throw ehf;

    // NOTREACHED (I hope)

    s_stderr->PrintfLn( "VM: no valid error handler initialized" );
    abort ();
    }
    break;
