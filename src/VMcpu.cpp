//
// Virtual Machine instruction dispatcher - CPU core.
// Includes VMcpu.h.
//
// JSVirtualMachine:: ExecuteCode
//

#include "JS.h"

#ifdef PROFILING

#pragma warning(disable:4035) // warning C4035: 'RDTSC' : no return value
inline __int64 RDTSC( void )
{
    // RDTSC - read time-stamp counter *by default return in eax:edx*
    //
    __asm __emit 0x0f
    __asm __emit 0x31
    }
#pragma warning(default:4035)

#define PROFILE_INIT()                               \
    int lastop = 1;                                  \
    __int64 x1 = RDTSC ();

#define PROFILE_OPCODE(opcode)                       \
    __int64 x2 = RDTSC ();                           \
    prof_count[ lastop ]++;                          \
    prof_elapsed[ lastop ] += x2 - x1;               \
    x1 = x2;                                         \
    lastop = (opcode);

#else

#define PROFILE_INIT()
#define PROFILE_OPCODE(opcode)

#endif

#define READ_ARG_INT32()    (PC++)->i32
#define READ_ARG_LABEL()    (PC++)->label
#define READ_ARG_CONST()    (PC++)->variant
#define READ_ARG_SYMBOL()   BP[ (PC++)->symbol ]

// STACKFRAME
//
#define JS_SP0          SP[ 0 ]
#define JS_SP1          SP[ 1 ]
#define JS_SP2          SP[ 2 ]
#define JS_SP(n)        SP[ n ]

#define JS_LOCAL(n)     FP[ -5 - (n) ]
#define JS_ARG(n)       FP[ 1 + (n) ]

#define JS_WITH_CHAINP  FP[ -2 ].with_chain
#define JS_ARGS_FIXP    FP[ -1 ].args_fix

#define JS_PUSH()       (SP--)
#define JS_POP()        (SP++)
#define JS_POP_N(n)     (SP += (n))

#define JS_CALL_HOOK(event)                                      \
do {                                                             \
    if ( options.event_hook )                                    \
    {                                                            \
        int hook_result = options.event_hook->callback( event ); \
        if ( hook_result != 0 )                                  \
        {                                                        \
            RaiseError( "hook break %d", hook_result );          \
            }                                                    \
        }                                                        \
    } while( 0 )

#define JS_MAYBE_GC()                                   \
do {                                                    \
    if ( gc.bytes_allocated >= options.gc_trigger )     \
    {                                                   \
        GarbageCollect( FP, SP );                       \
        JS_CALL_HOOK( JS_VM_EVENT_GARBAGE_COLLECT );    \
        }                                               \
    } while( 0 )

const int JS_RESERVE_STACK_FOR_FUNCTION = 10;

#define JS_SUBROUTINE_CALL(function)                            \
do {                                                            \
    /* Check that we have enought space in the stack. */        \
    if ( SP < stack + JS_RESERVE_STACK_FOR_FUNCTION )           \
    {                                                           \
        RaiseError( "VM: stack overflow" );                     \
        }                                                       \
                                                                \
    /* STACKFRAME */                                            \
                                                                \
    /* Save frame pointer. */                                   \
    JS_SP0.type = JS_FRAME_PTR;                                 \
    JS_SP0.frame_ptr = FP;                                      \
                                                                \
    /* Update FP: FP[ 0 ] */                                    \
    FP = &JS_SP0;                                               \
    JS_PUSH ();                                                 \
                                                                \
    /* Insert an empty args_fix: FP[ -1 ] */                    \
    JS_SP0.type = JS_ARGS_FIX;                                  \
    JS_SP0.args_fix.argc = 0;                                   \
    JS_SP0.args_fix.delta = 0;                                  \
    JS_PUSH ();                                                 \
                                                                \
    /* Insert empty with pointer: FP[ -2 ] */                   \
    JS_SP0.type = JS_WITH_CHAIN;                                \
    JS_SP0.with_chain.cNode = 0;                                \
    JS_SP0.with_chain.node = NULL;                              \
    JS_PUSH ();                                                 \
                                                                \
    /* Save imported-symbols context: FP[ -3 ] */               \
    JS_SP0.type = JS_BASE_PTR;                                  \
    JS_SP0.base_ptr = BP;                                       \
    JS_PUSH ();                                                 \
                                                                \
    /* Save return address: FP[ -4 ] */                         \
    JS_SP0.type = JS_INSTR_PTR;                                 \
    JS_SP0.instr_ptr = PC;                                      \
    JS_PUSH ();                                                 \
                                                                \
    /* And finally, jump to the method code. */                 \
    PC = (function)->code;                                      \
    BP = (function)->linkage;                                   \
                                                                \
    } while( 0 )

#define JS_OPCODE_CMP_REL(_OP_)                                         \
do {                                                                    \
    if ( JS_SP2.type == JS_STRING && JS_SP1.type == JS_STRING )         \
    {                                                                   \
        JS_SP2.vboolean = JS_SP2.StringCompare( JS_SP1 ) _OP_ 0;        \
        JS_SP2.type = JS_BOOLEAN;                                       \
        JS_POP ();                                                      \
        }                                                               \
    else if (JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER)    \
    {                                                                   \
        JS_SP2.vboolean = JS_SP2.vinteger _OP_ JS_SP1.vinteger;         \
        JS_SP2.type = JS_BOOLEAN;                                       \
        JS_POP ();                                                      \
        }                                                               \
    else                                                                \
    {                                                                   \
        /* Do it the hard way. */                                       \
                                                                        \
        JSVariant left;                                                 \
        switch( JS_SP2.type )                                           \
        {                                                               \
            case JS_INTEGER:                                            \
            case JS_FLOAT:                                              \
            case JS_NAN:                                                \
                left = JS_SP2;                                          \
                break;                                                  \
                                                                        \
            default:                                                    \
                JS_SP2.ToNumber( left );                                \
                break;                                                  \
            }                                                           \
                                                                        \
        JSVariant right;                                                \
        switch( JS_SP1.type )                                           \
        {                                                               \
            case JS_INTEGER:                                            \
            case JS_FLOAT:                                              \
            case JS_NAN:                                                \
                right = JS_SP1;                                         \
                break;                                                  \
                                                                        \
            default:                                                    \
                JS_SP1.ToNumber( right );                               \
                break;                                                  \
            }                                                           \
                                                                        \
        /* Do the comparison. */                                        \
        JS_POP ();                                                      \
                                                                        \
        if ( left.type == JS_NAN || right.type == JS_NAN )              \
            JS_SP1.type = JS_UNDEFINED;                                 \
        else if ( left.type == JS_INTEGER && right.type == JS_INTEGER ) \
        {                                                               \
            JS_SP1.type = JS_BOOLEAN;                                   \
            JS_SP1.vboolean = left.vinteger _OP_ right.vinteger;        \
            }                                                           \
        else                                                            \
        {                                                               \
            double ld;                                                  \
            if ( left.type == JS_FLOAT )                                \
                ld = left.vfloat;                                       \
            else                                                        \
                ld = double( left.vinteger );                           \
                                                                        \
            double rd;                                                  \
            if ( right.type == JS_FLOAT )                               \
                rd = right.vfloat;                                      \
            else                                                        \
                rd = double( right.vinteger );                          \
                                                                        \
            JS_SP1.type = JS_BOOLEAN;                                   \
            JS_SP1.vboolean = ld _OP_ rd;                               \
            }                                                           \
        }                                                               \
    } while( 0 )

#define JS_OPCODE_CMP_EQ(_OP_,_VAL_)                                    \
while( 1 )                                                              \
{                                                                       \
    int res;                                                            \
    if ( JS_SP2.type == JS_SP1.type )                                   \
    {                                                                   \
        /* Comparsion between same types. */                            \
        switch ( JS_SP2.type )                                          \
        {                                                               \
            case JS_INTEGER:                                            \
                res = JS_SP2.vinteger _OP_ JS_SP1.vinteger;             \
                break;                                                  \
                                                                        \
            case JS_STRING:                                             \
                res = JS_SP2.StringCompare( JS_SP1 ) _OP_ 0;            \
                break;                                                  \
                                                                        \
            case JS_FLOAT:                                              \
                res = JS_SP2.vfloat _OP_ JS_SP1.vfloat;                 \
                break;                                                  \
                                                                        \
            case JS_NAN:                                                \
                /* 11.9.3: cases 5 and 6 */                             \
                res = !_VAL_;                                           \
                break;                                                  \
                                                                        \
            case JS_BOOLEAN:                                            \
                res = JS_SP2.vboolean _OP_ JS_SP1.vboolean;             \
                break;                                                  \
                                                                        \
            case JS_OBJECT:                                             \
                res = JS_SP2.vobject _OP_ JS_SP1.vobject;               \
                break;                                                  \
                                                                        \
            case JS_BUILTIN:                                            \
                res = ( ( JS_SP2.vbuiltin->info == JS_SP1.vbuiltin->info \
                      && ( JS_SP2.vbuiltin->instance_context            \
                          == JS_SP1.vbuiltin->instance_context ) )      \
                     ? _VAL_ : ! _VAL_ );                               \
                break;                                                  \
                                                                        \
            case JS_FUNCTION:                                           \
                res = JS_SP2.vfunction _OP_ JS_SP1.vfunction;           \
                break;                                                  \
                                                                        \
            case JS_INSTR_PTR:                                          \
                res = JS_SP2.instr_ptr _OP_ JS_SP1.instr_ptr;           \
                break;                                                  \
                                                                        \
            case JS_FRAME_PTR:                                          \
                res = JS_SP2.frame_ptr _OP_ JS_SP1.frame_ptr;           \
                break;                                                  \
                                                                        \
            case JS_WITH_CHAIN:                                         \
                res = JS_SP2.with_chain.node _OP_ JS_SP1.with_chain.node; \
                break;                                                  \
                                                                        \
            default:                                                    \
                res = _VAL_;                                            \
                break;                                                  \
            }                                                           \
        }                                                               \
    else                                                                \
    {                                                                   \
        /* Type conversions between different types. */                 \
                                                                        \
        if ( ( JS_SP2.type == JS_UNDEFINED || JS_SP2.type == JS_NULL )  \
          && ( JS_SP1.type == JS_UNDEFINED || JS_SP1.type == JS_NULL )  \
            )                                                           \
        {                                                               \
            res = _VAL_;                                                \
            }                                                           \
        else if ( JS_SP2.IsNumber() && JS_SP1.IsNumber () )             \
        {                                                               \
            /* Numbers. */                                              \
            if ( JS_SP2.type == JS_NAN || JS_SP1.type == JS_NAN )       \
                /* 11.9.3: cases 5 and 6 */                             \
                res = !_VAL_;                                           \
            else if (JS_SP2.type == JS_INTEGER)                         \
                /* Integer-integer was already handled. */              \
                res = (double) JS_SP2.vinteger _OP_ JS_SP1.vfloat;      \
            else                                                        \
                /* Integer-integer was already handled. */              \
                res = JS_SP2.vfloat _OP_ (double) JS_SP1.vinteger;      \
            }                                                           \
        else                                                            \
        {                                                               \
            /* Must perform type casts. */                              \
                                                                        \
            if ( ( JS_SP2.type == JS_STRING || JS_SP2.type == JS_BOOLEAN \
               || JS_SP2.IsNumber () )                                  \
               && ( JS_SP1.type == JS_STRING                            \
                  || JS_SP1.type == JS_BOOLEAN                          \
                  || JS_SP1.IsNumber () ) )                             \
            {                                                           \
                JSVariant left;                                         \
                JS_SP2.ToNumber( left );                                \
                                                                        \
                JSVariant right;                                        \
                JS_SP1.ToNumber( right );                               \
                                                                        \
                if ( left.type == JS_NAN || right.type == JS_NAN )      \
                {                                                       \
                    res = !_VAL_;                                       \
                    }                                                   \
                else if ( left.type == JS_INTEGER )                     \
                {                                                       \
                    if ( right.type == JS_INTEGER )                     \
                        res = left.vinteger _OP_ right.vinteger;        \
                    else                                                \
                        res = (double) left.vinteger _OP_ right.vfloat; \
                    }                                                   \
                else                                                    \
                {                                                       \
                    if ( right.type == JS_INTEGER )                     \
                        res = left.vfloat _OP_ (double) right.vinteger; \
                    else                                                \
                        res = left.vfloat _OP_ right.vfloat;            \
                    }                                                   \
                }                                                       \
            else if ( JS_SP2.type == JS_OBJECT                          \
                  && ( JS_SP1.type == JS_STRING || JS_SP1.IsNumber () ) \
                )                                                       \
            {                                                           \
                /* ECMA 11.9.3.21. No preffered type specified */       \
                JSVariant cvt;                                          \
                JS_SP2.ToPrimitive( this, cvt, JS_UNDEFINED );          \
                JS_SP2 = cvt;                                           \
                continue;                                               \
                }                                                       \
            else if ( JS_SP1.type == JS_OBJECT                          \
                 && ( JS_SP2.type == JS_STRING || JS_SP2.IsNumber () )  \
                )                                                       \
            {                                                           \
                /* ECMA 11.9.3.20. No preffered type specified */       \
                JSVariant cvt;                                          \
                JS_SP1.ToPrimitive( this, cvt, JS_UNDEFINED );          \
                JS_SP1 = cvt;                                           \
                continue;                                               \
                }                                                       \
            else                                                        \
            {                                                           \
                res = ! _VAL_;                                          \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
    JS_SP2.type = JS_BOOLEAN;                                           \
    JS_SP2.vboolean = res;                                              \
    JS_POP ();                                                          \
                                                                        \
    break;                                                              \
    }

#define JS_OPCODE_CMP_SEQ(_OP_,_VAL_)                                   \
do {                                                                    \
    int res;                                                            \
    if ( JS_SP2.type == JS_SP1.type )                                   \
    {                                                                   \
        switch ( JS_SP2.type )                                          \
        {                                                               \
            case JS_INTEGER:                                            \
              res = JS_SP2.vinteger _OP_ JS_SP1.vinteger;               \
              break;                                                    \
                                                                        \
            case JS_FLOAT:                                              \
              res = JS_SP2.vfloat _OP_ JS_SP1.vfloat;                   \
              break;                                                    \
                                                                        \
            case JS_NAN:                                                \
              /* 11.9.6: cases 3 and 4 */                               \
              res = !_VAL_;                                             \
              break;                                                    \
                                                                        \
            case JS_STRING:                                             \
              res = JS_SP2.StringCompare( JS_SP1 ) _OP_ 0;              \
              break;                                                    \
                                                                        \
            case JS_BOOLEAN:                                            \
              res = JS_SP2.vboolean _OP_ JS_SP1.vboolean;               \
              break;                                                    \
                                                                        \
            case JS_OBJECT:                                             \
              res = JS_SP2.vobject _OP_ JS_SP1.vobject;                 \
              break;                                                    \
                                                                        \
            case JS_BUILTIN:                                            \
              res = ( ( JS_SP2.vbuiltin->info == JS_SP1.vbuiltin->info  \
                      && ( JS_SP2.vbuiltin->instance_context            \
                          == JS_SP1.vbuiltin->instance_context ) )      \
                     ? _VAL_ : !_VAL_ );                                \
              break;                                                    \
                                                                        \
            case JS_FUNCTION:                                           \
              res = JS_SP2.vfunction _OP_ JS_SP1.vfunction;             \
              break;                                                    \
                                                                        \
            default:                                                    \
              /* 11.9.6: case 12 */                                     \
              res = ! _VAL_;                                            \
              break;                                                    \
            }                                                           \
        }                                                               \
    else                                                                \
    {                                                                   \
        /* Only numbers are allowed here. */                            \
        if ( JS_SP2.IsNumber () && JS_SP1.IsNumber () )                 \
        {                                                               \
            if ( JS_SP2.type == JS_NAN || JS_SP1.type == JS_NAN )       \
                /* 11.9.6: cases 3 and 4 */                             \
                res = !_VAL_;                                           \
            else if (JS_SP2.type == JS_INTEGER)                         \
                res = (double) JS_SP2.vinteger _OP_ JS_SP1.vfloat;      \
            else                                                        \
                res = JS_SP2.vfloat _OP_ (double) JS_SP1.vinteger;      \
            }                                                           \
        else                                                            \
            res = !_VAL_;                                               \
        }                                                               \
                                                                        \
    JS_SP2.type = JS_BOOLEAN;                                           \
    JS_SP2.vboolean = res;                                              \
    JS_POP ();                                                          \
                                                                        \
    } while( 0 )

#define JS_OPCODE_BINARY(_OP_)                                    \
do {                                                              \
    if ( JS_SP2.type == JS_INTEGER && JS_SP1.type == JS_INTEGER ) \
    {                                                             \
        JS_SP2.vinteger = JSInt32( JS_SP2.vinteger )              \
                          _OP_ JSInt32( JS_SP1.vinteger );        \
        JS_POP ();                                                \
        }                                                         \
    else                                                          \
    {                                                             \
        JSInt32 left = JS_SP2.ToInt32 ();                         \
        JSInt32 right = JS_SP1.ToInt32 ();                        \
                                                                  \
        JS_SP2.vinteger = left _OP_ right;                        \
        JS_SP2.type = JS_INTEGER;                                 \
        JS_POP ();                                                \
        }                                                         \
    } while( 0 )


void
JSVirtualMachine:: ExecuteCode
(
    JSVariant* object,
    JSFunction* foo,
    int argc, JSVariant argv []
    )
{
    // Save stack context
    //
    JSVariant* saved_sp = SP;
    Compiled* saved_pc = PC;
    JSSymbol* saved_bp = BP;
	JSVariant* saved_fp = FP;

    // Protect the function from gc
    //
    JS_SP0 = foo;
    JS_PUSH ();

    // Push arguments to the stack
    //
    {
        for ( int i = argc - 1; i >= 0; i-- )
        {
            JS_SP0 = argv[ i ];
            JS_PUSH ();
            }
        }

    // This pointer
    //
    if ( object )
    {
        JS_SP0 = *object;
        JS_PUSH ();
        }
    else
    {
        JS_SP0.type = JS_NULL;
        JS_PUSH ();
        }

    // Init FP, PC and BP so our SUBROUTINE_CALL will work
    //
    FP = NULL;
    PC = NULL;
    BP = NULL;

    JS_SUBROUTINE_CALL( foo );

    // Result for all builtins
    //
    JSVariant builtin_result;

    // Instruction argument holders
    //
    JSInt32 arg_int32;
    JSVariant* arg_variant;
    Compiled* arg_label;
    JSSymbol arg_symbol;

    // Ok, now we are ready to run
    //
    PROFILE_INIT ();

    for (;;) try // until goto ALL_DONE;
    {
        for ( ;; )
        {
            if ( should_terminate )
            {
                if ( should_terminate == 1 )
                {
                    should_terminate = 0;
                    RaiseError( "Terminate request." );
                    }
                else
                {
                    exec_result.type = JS_BOOLEAN;
                    exec_result.vboolean = 0;
                    return;
                    }
                }

            PROFILE_OPCODE( PC->op );

#if 0
            // Trace CPU execution
            //
            fprintf( stderr, "%p: %-16s", PC, OpcodeDesc[ PC->op ].desc );
            Compiled* pc_next = PC + 1;

            switch( OpcodeDesc[ PC->op ].arg_type )
            {
                case JS_OPARG_INT32:
                    fprintf( stderr, "int32:%ld\n", pc_next->i32 );
                    break;

                case JS_OPARG_CONST:
                    switch( pc_next->variant->type )
                    {
                        case JS_STRING:
                            fprintf( stderr, "const:\"%s\"\n", pc_next->variant->vstring->data );
                            break;
                        case JS_FLOAT:
                            fprintf( stderr, "const:%lf\n", pc_next->variant->vfloat );
                            break;
                        case JS_INTEGER:
                            fprintf( stderr, "const:%lf\n", pc_next->variant->vinteger );
                            break;
                        default:
                            fprintf( stderr, "const:ERROR\n" );
                        }
                    break;

                case JS_OPARG_SYMBOL:
                    fprintf( stderr, "symbol:%s\n", Symname( BP[ pc_next->i32 ] ) );
                    break;

                case JS_OPARG_LABEL:
                    fprintf( stderr, "label:%p\n", pc_next->label );
                    break;

                case JS_OPARG_none:
                    fprintf( stderr, "\n" );
                    break;
                }
#endif

            switch ( (PC++)->op ) // until exception or goto ALL_DONE;
            {
                ////////////////
                #include "VMcpu.h"
                ////////////////

            default:
                RaiseError( "VM: unknown instruction opcode %d", PC[ -1 ].op );
                }
            }
        }
    catch( JSErrorHandlerFrame* )
    {
        // Ok, we caught an error
        //
        if ( error_handler->SP == NULL )
        {
            // Top-level error handler should handle it
            //
            throw;
            }

        // Restore our state
        //
        SP = error_handler->SP;     // restore stack pointer
        FP = error_handler->FP;     // restore frame pointer
        PC = error_handler->PC;     // actual jump to the catch block
        BP = error_handler->BP;     // restore imported-symbols context

        // Push the thrown value to the stack
        //
        JS_SP0 = error_handler->thrown;
        JS_PUSH ();

        // Remove this handler frame
        //
        JSErrorHandlerFrame* frame = error_handler;
        error_handler = error_handler->next;
        delete frame;
        }

ALL_DONE:
    //
    // All done
    //
    exec_result = JS_SP1;

    SP = saved_sp;
    PC = saved_pc;
    BP = saved_bp;
	FP = saved_fp;
    }
