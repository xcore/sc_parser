/* General purpose parser module */

#ifndef COMPPARSERCONF_H_INCLUDED
#define COMPPARSERCONF_H_INCLUDED

/* Configuration for parser */
//#define USE_INCR_COMPPARSER // incremental parsing is 30% slower but has the Yield action option

#ifdef USE_INCR_COMPPARSER
  #define INCR_COMPPARSER_STACK_SIZE 200 /* usually enough also for quite complex grammars */
#endif

/* configuration for EbnfGrammarCompiler */
#define EBNF_GRAMMAR_COMPILER_STACK_SIZE 16 /* Used by EbnfGrammarCompiler, usually enough, depends on the nested brackets in grammar */
#define EBNF_GRAMMAR_COMPILER_SYM_SIZE 100 /* max number of syntax rules in the grammar to be compiled */

#endif
/* end of CompParserConf.h */
