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
//#define YIELD        -14

unsigned long SourcePos;
static F_ACTION **pActFunc;
static FREAD *pfSourceRead;

static int TryOrExpr(const unsigned char OrExpr[]);

#pragma stackfunction 2000 /* usually enough, depends on grammar complexity, recursion... */
static int TryUnaryExpr(const unsigned char UnaryExpr[])
{
  switch((signed char)*UnaryExpr)
  {
    case OR_EXPR_POS:
      return TryOrExpr(&UnaryExpr[(signed short)(UnaryExpr[1] << 8) | UnaryExpr[2]]); /* OR_EXPR_POS OrExprRelativePosition */
    case ACTION:
      return pActFunc[UnaryExpr[1]]((const unsigned char *)&UnaryExpr[3], UnaryExpr[2]); /* ACTION ActionId Arglen Arg */
    case CHAR_EQ: /* CHAR_EQ Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1))
        return FAIL;
      SourcePos++;
      return (*SourceTerminal == UnaryExpr[1]) ? PASS : FAIL;
    }
#ifdef REP_OP
    case REP_OP: /* REP_OP MinRep ExtraOptinalRep UnaryExpr */
    {
      unsigned char Rep = UnaryExpr[1];

      for(UnaryExpr += 3; Rep; Rep--)
        if(!TryUnaryExpr(UnaryExpr))
          return FAIL; /* "must" repetitions */
      Rep = UnaryExpr[-1]; /* extra optional repetitions */
      if(UCHAR_MAX == Rep)
        while(TryUnaryExpr(UnaryExpr)); /* infinite (extra) optional repetitions */
      else
        for( ; Rep && TryUnaryExpr(UnaryExpr); Rep--);  /* limited (extra) optional repetitions. Stop repeating when TryUnaryExpr fails */
      return PASS;
    }
#endif
#ifdef CHAR_RANGE
    case CHAR_RANGE: /* CHAR_RANGE Char Char */
    {
      const unsigned char *SourceTerminal;

      if(!pfSourceRead(&SourceTerminal, SourcePos, 1))
        return FAIL;
      if((*SourceTerminal < UnaryExpr[1]) || (*SourceTerminal > UnaryExpr[2]))
        return FAIL; /* source char not in the range */
      SourcePos++;
      return PASS;
    }
#endif
#ifdef STRING_EQ
    case STRING_EQ: /* STRING_EQ StringLen String */
    {
      register unsigned char TerminalStringLen = UnaryExpr[1];
      const unsigned char *SourceTerminal, *TerminalString = (const unsigned char *)&UnaryExpr[2];

      if(!pfSourceRead(&SourceTerminal, SourcePos, TerminalStringLen))
        return FAIL;
      if(memcmp(SourceTerminal, TerminalString, TerminalStringLen))
        return FAIL; /* source != terminal */
      SourcePos += TerminalStringLen;
      return PASS;
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
  return FAIL; /* must not happen if grammar is ok */
}

static int TryOrExpr(const unsigned char OrExpr[])
{ /* OrExpr = < NextAndExprRelativePosition AndExpr >; */
  unsigned long SourcePosCopy = SourcePos; /* save current source positions */

  for( ; ; OrExpr = &OrExpr[((short)OrExpr[0] << 8) | OrExpr[1]]) /* try the alternatives, return PASS at first one that passes */
  {
    const unsigned char *AndExpr = OrExpr + 2;
    for( ; ; AndExpr = &AndExpr[((short)AndExpr[0] << 8) | AndExpr[1]]) /* try the terms, return FAIL at first one that fails */
    { /* AndExpr = { NextUnaryExprRelativePosition UnaryExpr }; */
      if(!(AndExpr[0] | AndExpr[1]))
        return PASS; /* no more terms, all passed */
      if(!TryUnaryExpr(AndExpr + 2))
        break; /* UnaryExpr failed, do not try further of current AndExpr */
    }
    SourcePos = SourcePosCopy; /* TryAndExpr failed, restore source position */
    if(!(OrExpr[0] | OrExpr[1]))
      return FAIL; /* no more alternatives, all failed */
  }
}

/****************************/

int Parse(const unsigned char Grammar[], FREAD SourceRead, F_ACTION *ActFunc[])
{
  SourcePos = 0;
  pfSourceRead = SourceRead;
  pActFunc = ActFunc;
  return TryOrExpr(Grammar);  /* try the start symbol ( Alternatives list of the first syntax rule ) */
}
