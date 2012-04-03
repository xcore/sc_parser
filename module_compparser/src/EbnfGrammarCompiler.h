/* Ebnf like grammar compiler for general purpose module */

#ifndef EBNFGRAMMARCOMPILER_H_INCLUDED
#define EBNFGRAMMARCOMPILER_H_INCLUDED

#include "CompParser.h"

#define FWRITE_PROTO(FuncName) int FuncName(const unsigned char *Ch, unsigned int Pos, unsigned int NumChar)
typedef FWRITE_PROTO(F_WRITE);

/* Ebnf like grammar compiler. Return value is the size of compiled grammar ( 0 if unsuccessfull ) */
extern unsigned int EbnfGrammarCompiler(F_READ EbnfGrammarRead, char const * const ActNames[], unsigned char ActNum, F_WRITE CompiledGrammarWrite);

#endif
/* end of EbnfGrammarCompiler.h */
