#include "JSC.h"

ASM_labelDef*
JSC_Compiler:: ASM_defLabel( void )
{
    return new(this) ASM_labelDef( asm_code.label_count ++ );
    }

void
JSC_Compiler:: ASM_emitLabel( ASM_labelDef* arg_label )
{
    asm_code.Emit( arg_label );
    }

void
JSC_Compiler:: ASM_defSymbol( String* arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolDef( arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_halt( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_HALT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_done( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_DONE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_nop( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_NOP, arg_linenum ) );
    }

void JSC_Compiler:: ASM_dup( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_DUP, arg_linenum ) );
    }

void JSC_Compiler:: ASM_pop( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_POP, arg_linenum ) );
    }

void JSC_Compiler:: ASM_pop_n( int arg_val, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_POP_N, arg_val, arg_linenum ) );
    }

void JSC_Compiler:: ASM_apop( int arg_val, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_APOP, arg_val, arg_linenum ) );
    }

void JSC_Compiler:: ASM_swap( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_SWAP, arg_linenum ) );
    }

void JSC_Compiler:: ASM_roll( int arg_val, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_ROLL, arg_val, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const( long arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_constArg( arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const( double arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_constArg( arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const( String* arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_constArg( arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_null( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_NULL, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_true( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_TRUE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_false( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_FALSE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_undefined( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_UNDEFINED, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_i0( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_I0, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_i1( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_I1, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_i2( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_I2, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_i3( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CONST_I3, arg_linenum ) );
    }

void JSC_Compiler:: ASM_const_i( long arg_val, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_CONST_I, arg_val, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_global( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_LOAD_GLOBAL, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_global_w( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_LOAD_GLOBAL_W, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_store_global( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_STORE_GLOBAL, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_arg( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_LOAD_ARG, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_store_arg( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_STORE_ARG, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_local( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_LOAD_LOCAL, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_store_local( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_STORE_LOCAL, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_property( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_LOAD_PROPERTY, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_store_property( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_STORE_PROPERTY, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_array( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_LOAD_ARRAY, arg_linenum ) );
    }

void JSC_Compiler:: ASM_store_array( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_STORE_ARRAY, arg_linenum ) );
    }

void JSC_Compiler:: ASM_nth( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_NTH, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_eq( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_EQ, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_ne( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_NE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_lt( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_LT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_gt( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_GT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_le( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_LE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_ge( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_GE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_seq( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_SEQ, arg_linenum ) );
    }

void JSC_Compiler:: ASM_cmp_sne( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_CMP_SNE, arg_linenum ) );
    }

void JSC_Compiler:: ASM_sub( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_SUB, arg_linenum ) );
    }

void JSC_Compiler:: ASM_add( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_ADD, arg_linenum ) );
    }

void JSC_Compiler:: ASM_mul( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_MUL, arg_linenum ) );
    }

void JSC_Compiler:: ASM_div( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_DIV, arg_linenum ) );
    }

void JSC_Compiler:: ASM_mod( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_MOD, arg_linenum ) );
    }

void JSC_Compiler:: ASM_neg( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_NEG, arg_linenum ) );
    }

void JSC_Compiler:: ASM_and( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_AND, arg_linenum ) );
    }

void JSC_Compiler:: ASM_not( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_NOT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_or( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_OR, arg_linenum ) );
    }

void JSC_Compiler:: ASM_xor( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_XOR, arg_linenum ) );
    }

void JSC_Compiler:: ASM_shift_left( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_SHIFT_LEFT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_shift_right( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_SHIFT_RIGHT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_shift_rright( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_SHIFT_RRIGHT, arg_linenum ) );
    }

void JSC_Compiler:: ASM_iffalse( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_IFFALSE, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_iftrue( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_IFTRUE, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_call_method( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_CALL_METHOD, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_jmp( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_JMP, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_jsr( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_JSR, arg_linenum ) );
    }

void JSC_Compiler:: ASM_jsr_w( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_JSR_W, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_return( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_RETURN, arg_linenum ) );
    }

void JSC_Compiler:: ASM_typeof( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_TYPEOF, arg_linenum ) );
    }

void JSC_Compiler:: ASM_new( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_NEW, arg_linenum ) );
    }

void JSC_Compiler:: ASM_delete_property( String* arg_name, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_symbolArg( JS_OPCODE_DELETE_PROPERTY, arg_name, arg_linenum ) );
    }

void JSC_Compiler:: ASM_delete_array( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_DELETE_ARRAY, arg_linenum ) );
    }

void JSC_Compiler:: ASM_locals( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_LOCALS, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_min_args( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_MIN_ARGS, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_load_nth_arg( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_LOAD_NTH_ARG, arg_linenum ) );
    }

void JSC_Compiler:: ASM_with_push( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_WITH_PUSH, arg_linenum ) );
    }

void JSC_Compiler:: ASM_with_pop( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_WITH_POP, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_try_push( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_TRY_PUSH, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_try_pop( int arg_value, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_int32Arg( JS_OPCODE_TRY_POP, arg_value, arg_linenum ) );
    }

void JSC_Compiler:: ASM_throw( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_THROW, arg_linenum ) );
    }

void JSC_Compiler:: ASM_iffalse_b( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_IFFALSE_B, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_iftrue_b( ASM_labelDef* arg_label, int arg_linenum )
{
    asm_code.Emit( new(this) ASM_labelArg( JS_OPCODE_IFTRUE_B, arg_label, arg_linenum ) );
    }

void JSC_Compiler:: ASM_add_1_i( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_ADD_1_I, arg_linenum ) );
    }

void JSC_Compiler:: ASM_add_2_i( int arg_linenum )
{
    asm_code.Emit( new(this) ASM_voidArg( JS_OPCODE_ADD_2_I, arg_linenum ) );
    }
