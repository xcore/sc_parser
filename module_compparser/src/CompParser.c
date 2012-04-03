/* General purpose parser module */

#include <string.h>
#include <limits.h>
#include "CompParser.h"

/* Compiled grammar grammar :
OrExpr = < NextAndExprRelativePosition AndExpr >; # list of alternatives. NextAndExprRelativePosition==0 means no more alternatives #
AndExpr = NextUnaryExprRelativePosition { UnaryExpr NextUnaryExprRelativePosition }; # NextUnaryExprRelativePosition==0 means no more UnaryExpr #
UnaryExpr = OR_EXPR_POS OrExprRelativePosition | ACTION ActionId Arglen Arg | STRING_EQ StringLen String | REP_OP MinRep ExtraOptinalRep UnaryExpr |
 STRING_GE StringLen String | STRING_LE StringLen String | NOT_OP UnaryExpr | REW_OP UnaryExpr |
 REP_0_1_OP UnaryExpr | REP_0_INF_OP UnaryExpr | REP_1_INF_OP UnaryExpr | OR_SUB_EXPR OrExpr | CHAR_RANGE Char Char | CHAR_EQ Char | YIELD;

note: NextAndExprRelativePosition, NextUnaryExprRelativePosition, OrExprRelativePosition are 2 bytes long
*/

/* minimal necessary operators */
#define OR_EXPR_POS   0
#define ACTION       -1
#define CHAR_EQ      -2
/* very usefull operators */
#define REP_OP       -3 /* avoid recursive syntax rules thus reducing stack usage */
#define CHAR_RANGE   -4
#define STRING_EQ    -5
/* optionals operators ( may increase parsing speed, reduce compiled grammar size ) */
#define NOT_OP       -6  // negate the result and restore the source position
#define REW_OP       -7  // restore the source position
//#define REP_0_1_OP   -8
//#define REP_0_INF_OP -9
//#define REP_1_INF_OP -10
//#define OR_SUB_EXPR  -11
//#define STRING_GE    -12
//#define STRING_LE    -13

unsigned long SourcePos;
static F_ACTION **pActFunc;
static F_READ *pfSourceRead;

#ifndef USE_INCR_COMPPARSER

static int TryOrExpr(const unsigned char OrExpr[]);

#ifndef _MSC_VER
#pragma stackfunction 2000 /* usually enough, depends on grammar complexity, recursion... */
#endif
static int TryUnaryExpr(const unsigned char UnaryExpr[])
{
  switch((signed char)*UnaryExpr)
  {
    case OR_EXPR_POS: return TryOrExpr(&UnaryExpr[(signed short)(UnaryExpr[1] << 8) | UnaryExpr[2]]); /* OR_EXPR_POS OrExprRelativePosition */
    case ACTION: 
      if(UCHAR_MAX == UnaryExpr[1]) return PASS; /* Yield predefined action not managed */
      return pActFunc[UnaryExpr[1]]((const unsigned char *)&UnaryExpr[3], UnaryExpr[2]); /* ACTION ActionId Arglen Arg */
    case CHAR_EQ: /* CHAR_EQ Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1)) return FAIL;
      if(*SourceTerminal != UnaryExpr[1]) return FAIL;
      SourcePos++; return PASS;
    }
#ifdef REP_OP
    case REP_OP: /* REP_OP MinRep ExtraOptinalRep UnaryExpr */
    {
      unsigned char Rep = UnaryExpr[1];

      for(UnaryExpr += 3; Rep; Rep--)
        if(!TryUnaryExpr(UnaryExpr)) return FAIL; /* "must" repetitions */
      Rep = UnaryExpr[-1]; /* extra optional repetitions */
      if(UCHAR_MAX == Rep)
        while(TryUnaryExpr(UnaryExpr)); /* infinite (extra) optional repetitions */
      else
        for( ; Rep; Rep--) if(!TryUnaryExpr(UnaryExpr)) break; /* limited (extra) optional repetitions. Stop repeating when TryUnaryExpr fails */   
      return PASS;
    }
#endif
#ifdef CHAR_RANGE
    case CHAR_RANGE: /* CHAR_RANGE Char Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1)) return FAIL;
      if((*SourceTerminal < UnaryExpr[1]) || (*SourceTerminal > UnaryExpr[2])) return FAIL; /* source char not in the range */
      SourcePos++; return PASS;
    }
#endif
#ifdef STRING_EQ
    case STRING_EQ: /* STRING_EQ StringLen String */
    {
      register unsigned char TerminalStringLen = UnaryExpr[1];
      const unsigned char *SourceTerminal, *TerminalString = (const unsigned char *)&UnaryExpr[2];

      if(!pfSourceRead(&SourceTerminal, SourcePos, TerminalStringLen)) return FAIL;
      if(memcmp(SourceTerminal, TerminalString, TerminalStringLen)) return FAIL; /* source != terminal */
      SourcePos += TerminalStringLen; return PASS;
    }
#endif
#ifdef NOT_OP
    case NOT_OP: /* NOT_OP UnaryExpr */
    {
      unsigned long SourcePosCopy = SourcePos;
      int Res = TryUnaryExpr(++UnaryExpr);

      SourcePos = SourcePosCopy; /* rewind */
      return (PASS == Res) ? FAIL : PASS;
    }
#endif
#ifdef REW_OP
    case REW_OP: /* REW_OP UnaryExpr */
    {
      unsigned long SourcePosCopy = SourcePos;
      int Res = TryUnaryExpr(++UnaryExpr);

      SourcePos = SourcePosCopy; /* rewind */
      return Res;
    }
#endif
  }
  return FAIL; /* Panic. Must not happen if grammar is ok */
}

static int TryOrExpr(const unsigned char OrExpr[])
{ /* OrExpr = < NextAndExprRelativePosition AndExpr >; */
  unsigned long SourcePosCopy = SourcePos; /* save current source positions */

  for( ; ; OrExpr = &OrExpr[((short)OrExpr[0] << 8) | OrExpr[1]]) /* try the alternatives, return PASS at first one that passes */
  {
    const unsigned char *AndExpr = OrExpr + 2;
    for( ; ; AndExpr = &AndExpr[((short)AndExpr[0] << 8) | AndExpr[1]]) /* try the terms, return FAIL at first one that fails */
    { /* AndExpr = { NextUnaryExprRelativePosition UnaryExpr }; */
      if(!(AndExpr[0] | AndExpr[1])) return PASS; /* no more terms, all passed */
      if(!TryUnaryExpr(AndExpr + 2)) break; /* UnaryExpr failed, do not try further of current AndExpr */
    }
    SourcePos = SourcePosCopy; /* TryAndExpr failed, restore source position */
    if(!(OrExpr[0] | OrExpr[1])) return FAIL; /* no more alternatives, all failed */
  }
}

/****************************/

int StartParse(const unsigned char Grammar[], F_READ SourceRead, F_ACTION *ActFunc[])
{
  SourcePos = 0; pfSourceRead = SourceRead; pActFunc = ActFunc;
  return TryOrExpr(Grammar);  /* try the start symbol ( Alternatives list of the first syntax rule ) */
}

#else

#ifndef INCR_COMPPARSER_STACK_SIZE
  #define INCR_COMPPARSER_STACK_SIZE 256 /* usually enough also for quite complex grammars */
#endif

union tagSTACK_ITEM
{ // items that can be pushed in stack
  const unsigned char *pGrammar;
  unsigned long LongVal;
};
typedef union tagSTACK_ITEM STACK_ITEM;

static STACK_ITEM Stack[INCR_COMPPARSER_STACK_SIZE];
static unsigned int Sp;
static int RetVal;

/* emulation of function call made by code using goto */
#define CALL(Func, Arg, CallId) \
{\
  if(Sp > INCR_COMPPARSER_STACK_SIZE - 2) return FAIL; /* stack overflow */\
  Stack[Sp].LongVal = CallId; /* push the return label index */ \
  Stack[Sp + 1].pGrammar = Arg; /* push the argument */\
  Sp += 2; goto Func; /* call */\
  Ret ## CallId:; /* return label */\
}

/* emulation of a return made by code using goto */
#define RETURN(ReturnValue, SpPops)\
 { RetVal = ReturnValue; Sp -= SpPops; goto RetDecode; }

int ContinueParse(void)
{ /* OrExpr = < NextAndExprRelativePosition AndExpr >; */
  const unsigned char *pCharTmp;
  if(2 != Sp) goto ResumeParsing; // continue from a paused parsing

  TryOrExpr: // alias static int TryOrExpr(const unsigned char OrExpr[]) of not incremenal parser
  /* stack frame:
  -return label index
  -OrExpr        : incoming parameter
  -SourcePosCopy : local parameter
  -AndExpr       : local parameter */
  if(Sp > INCR_COMPPARSER_STACK_SIZE - 2) return FAIL; /* stack overflow */
  Stack[Sp].LongVal = SourcePos; Sp += 2; /* save current source positions and room for AndExpr in stack */
  for(pCharTmp = Stack[Sp - 3].pGrammar; ; ) /* try the alternatives, return PASS at first one that passes */
  {
    for(Stack[Sp - 1].pGrammar = pCharTmp = pCharTmp + 2; ; ) /* try the terms, return FAIL at first one that fails */
    { /* AndExpr = { NextUnaryExprRelativePosition UnaryExpr }; */
      if(!(pCharTmp[0] | pCharTmp[1])) RETURN(PASS, 4); /* no more terms, all passed */
      CALL(TryUnaryExpr, pCharTmp + 2, 1)
      if(!RetVal) break; /* UnaryExpr failed, do not try further of current AndExpr */
      pCharTmp = Stack[Sp - 1].pGrammar;
      Stack[Sp - 1].pGrammar = pCharTmp = &pCharTmp[((short)pCharTmp[0] << 8) | pCharTmp[1]];
    }
    SourcePos = Stack[Sp - 2].LongVal; /* TryAndExpr failed, restore source position */
    pCharTmp = Stack[Sp - 3].pGrammar;
    if(!(pCharTmp[0] | pCharTmp[1])) RETURN(FAIL, 4) /* no more alternatives, all failed */
    Stack[Sp - 3].pGrammar = pCharTmp = &pCharTmp[((short)pCharTmp[0] << 8) | pCharTmp[1]];
  }
  
  TryUnaryExpr:
  /* stack frame:
  -return label index
  -UnaryExpr : incoming parameter */
  pCharTmp = Stack[Sp - 1].pGrammar;
  switch((signed char)*pCharTmp)
  {
    case OR_EXPR_POS:
      CALL(TryOrExpr, &pCharTmp[(signed short)(pCharTmp[1] << 8) | pCharTmp[2]], 2)  /* OR_EXPR_POS OrExprRelativePosition */
      RETURN(RetVal, 2)
    case ACTION:
      if(UCHAR_MAX == pCharTmp[1])
      { return YIELD; ResumeParsing: RETURN(PASS, 2) }
      RETURN(pActFunc[pCharTmp[1]]((const unsigned char *)&pCharTmp[3], pCharTmp[2]), 2) /* ACTION ActionId Arglen Arg */
    case CHAR_EQ: /* CHAR_EQ Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1)) RETURN(FAIL, 2)
      if(*SourceTerminal != pCharTmp[1]) RETURN(FAIL, 2)
      SourcePos++; RETURN(PASS, 2)
    }
#ifdef REP_OP
    case REP_OP: /* REP_OP MinRep ExtraOptinalRep UnaryExpr */
    { 
      unsigned char i;
      /* stack frame:
      -return label index
      -UnaryExpr : incoming parameter
      -Rep       : local parameter */
      if(Sp > INCR_COMPPARSER_STACK_SIZE - 1) return FAIL; /* stack overflow */
      Stack[Sp].LongVal = i = pCharTmp[1]; Sp++;
      for(Stack[Sp - 2].pGrammar += 3; i; i = (unsigned char)--Stack[Sp - 1].LongVal)
      {
        CALL(TryUnaryExpr, Stack[Sp - 2].pGrammar, 3) /* "must" repetitions */
        if(!RetVal) RETURN(FAIL, 3)
      }
      Stack[Sp - 1].LongVal = i = Stack[Sp - 2].pGrammar[-1]; /* extra optional repetitions */
      if(UCHAR_MAX == i)
        do /* infinite (extra) optional repetitions */
          CALL(TryUnaryExpr, Stack[Sp - 2].pGrammar, 4)
        while(RetVal);
      else /* limited (extra) optional repetitions. Stop repeating when TryUnaryExpr fails */
        for( i = (unsigned char)Stack[Sp - 1].LongVal; i; i = (unsigned char)--Stack[Sp - 1].LongVal)
        {
          CALL(TryUnaryExpr, Stack[Sp - 2].pGrammar, 5)
          if(!RetVal) break;
        }
      RETURN(PASS, 3)
    }
#endif
#ifdef CHAR_RANGE
    case CHAR_RANGE: /* CHAR_RANGE Char Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1)) RETURN(FAIL, 2)
      pCharTmp = Stack[Sp - 1].pGrammar;
      if((*SourceTerminal < pCharTmp[1]) || (*SourceTerminal > pCharTmp[2]))  RETURN(FAIL, 2) /* source char not in the range */
      SourcePos++; RETURN(PASS, 2)
    }
#endif
#ifdef STRING_EQ
    case STRING_EQ: /* STRING_EQ StringLen String */
    {
      register unsigned char TerminalStringLen = Stack[Sp - 1].pGrammar[1];
      const unsigned char *SourceTerminal, *TerminalString = (const unsigned char *)&Stack[Sp - 1].pGrammar[2];

      if(!pfSourceRead(&SourceTerminal, SourcePos, TerminalStringLen)) RETURN(FAIL, 2)
      if(memcmp(SourceTerminal, TerminalString, TerminalStringLen)) RETURN(FAIL, 2) /* source != terminal */
      SourcePos += TerminalStringLen; RETURN(PASS, 2)
    }
#endif
#ifdef NOT_OP
    case NOT_OP: /* NOT_OP UnaryExpr */
    {  /* stack frame:
      -return label index
      -UnaryExpr     : incoming parameter
      -SourcePosCopy : local parameter */
      if(Sp > INCR_COMPPARSER_STACK_SIZE - 1) return FAIL; /* stack overflow */
      Stack[Sp++].LongVal = SourcePos; /* save current source positions in stack */
      CALL(TryUnaryExpr, ++Stack[Sp - 2].pGrammar, 6)
      SourcePos = Stack[Sp - 1].LongVal; /* rewind */
      RETURN((PASS == RetVal) ? FAIL : PASS, 3)
    }
#endif
#ifdef REW_OP
    case REW_OP: /* REW_OP UnaryExpr */
    {  /* stack frame:
      -return label index
      -UnaryExpr     : incoming parameter
      -SourcePosCopy : local parameter */
      if(Sp > INCR_COMPPARSER_STACK_SIZE - 1) return FAIL; /* stack overflow */
      Stack[Sp++].LongVal = SourcePos; /* save current source positions in stack */
      CALL(TryUnaryExpr, ++Stack[Sp - 2].pGrammar, 7)
      SourcePos = Stack[Sp - 1].LongVal; /* rewind */
      RETURN(RetVal, 3)
    }
#endif
  }

  RetDecode:
  switch(Stack[Sp].LongVal)
  {
    case 0: return RetVal;
    case 1: goto Ret1;
    case 2: goto Ret2;
    case 3: goto Ret3;
    case 4: goto Ret4;
    case 5: goto Ret5;
    case 6: goto Ret6;
    case 7: goto Ret7;
    default: return FAIL; // ?? panic
  }
}

/****************************/

int StartParse(const unsigned char Grammar[], F_READ SourceRead, F_ACTION *ActFunc[])
{
  SourcePos = 0; pfSourceRead = SourceRead;
  pActFunc = ActFunc; Sp = 2;
  Stack[0].LongVal = 0; Stack[1].pGrammar = Grammar;
  return ContinueParse();  /* try the start symbol ( Alternatives list of the first syntax rule ) */
}

#endif
