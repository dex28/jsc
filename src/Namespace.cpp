#include "JSC.h"

void
NamespaceNest:: Create( JSC_Compiler* ctx )
{
    jsc = ctx;
    pDeleted = NULL;
    pTop = new(jsc) NamespaceFrame;
    pTop->defs = NULL;
    pTop->next = NULL;
    pTop->num_locals = 0;
    pTop->begin_linenum = 0;
    pTop->level = 0;
    }

void
NamespaceNest:: pushFrame( int linenum )
{
    NamespaceFrame* frame = new(jsc) NamespaceFrame;

    frame->num_locals = pTop->num_locals;
    frame->begin_linenum = linenum;
    frame->level = pTop->level + 1;
    frame->defs = NULL;
    frame->next = pTop;

    pTop = frame;

    //printf( "NS %d; line %d\n", pTop->level, pTop->begin_linenum );
    }

void
NamespaceNest:: popFrame( void )
{
    //printf( "NS %d: line %d END; num_locals = %d\n", pTop->level, pTop->begin_linenum, pTop->num_locals );

    for ( SymbolDefinition* sym = pTop->defs; sym; sym = sym->next )
    {
        if ( sym->use_count == 0 )
        {
            if ( sym->scope == JSC_SCOPE_ARG && jsc->options.warn_unused_argument )
            {
                jsc->warning( sym->linenum, "unused argument '%s'", (const char*)*sym->name );
                }
            else if ( sym->scope == JSC_SCOPE_LOCAL && jsc->options.warn_unused_variable )
            {
                jsc->warning( sym->linenum, "unused variable '%s'", (const char*)*sym->name );
                }
            }
        }

    NamespaceFrame* frame = pTop;
    pTop = frame->next;
    frame->next = pDeleted;
    pDeleted = frame;
    }

int
NamespaceNest:: allocLocal( void )
{
    return pTop->num_locals ++;
    }

SymbolDefinition*
NamespaceNest:: defineSymbol( String* name, int scope, int value, int linenum )
{
    for ( SymbolDefinition* sym = pTop->defs; sym; sym = sym->next )
    {
        if ( *sym->name == *name )
        {
            if ( sym->scope == scope )
            {
                jsc->error( linenum, "redeclaration of '%s'", (const char*)*name );
                }

            if ( sym->scope == JSC_SCOPE_ARG ) //&& warn_shadow )
            {
                jsc->warning( linenum, "declaration of '%s' shadows a parameter", (const char*)*name );
                }

            sym->scope = scope;
            sym->value = value;
            sym->linenum = linenum;
            sym->use_count = 0; // TODO: is this ok ?
            return sym;
            }
        }

    sym = new(jsc) SymbolDefinition;

    sym->name = name;
    sym->scope = scope;
    sym->value = value;
    sym->linenum = linenum;
    sym->use_count = 0;

    sym->next = pTop->defs;
    pTop->defs = sym;
/*
    char prefix[] = ". . . . . . . . . . . . . . . . ";
    printf( "%04d %.*s%s(%d) = '%s' [line %d]\n",
            pTop->begin_linenum, pTop->level * 2, prefix,
            sym->scope == JSC_SCOPE_ARG ? "ARG" : "LOCAL", sym->value,
            PCSTR( *sym->name ),
            sym->linenum
            ); 
*/
    return sym;
    }

SymbolDefinition*
NamespaceNest:: lookupSymbol( String* name )
{
    for ( NamespaceFrame* frame = pTop; frame; frame = frame->next )
    {
        for ( SymbolDefinition* sym = frame->defs; sym; sym = sym->next )
        {
            if ( *sym->name == *name )
            {
                sym->use_count ++;
                return sym;
                }
            }
        }

    return NULL;
    }

