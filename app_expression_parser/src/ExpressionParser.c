/*
 ============================================================================
 Name        : ExpressionParsing.c
 Description : Sample code for general purpose parser,
               it doesn't make use of any specific hardware platform
 ============================================================================
 */
#include <stdio.h> /* for the printf function */
#include <string.h> /* for the memcpy function */
#include "EbnfGrammarCompiler.h"

/* ebnf grammar (to be compiled) */
static unsigned char EbnfGrammar[] = {"\
# Expression Grammar #\
Expression          = @StackReset@, Blanks, Additive_expr, @PrintResult=Result: %d\\0a\\0d\\00@;\
Additive_expr       = Multiplicative_expr, 0~?(PLUS, Multiplicative_expr, @DoPlus@ | MINUS, Multiplicative_expr, @DoMinus@);\
Multiplicative_expr = UnaryExpr, 0~?(MUL, UnaryExpr, @DoMul@ | DIV, UnaryExpr, @DoDiv@);\
UnaryExpr           = UMINUS, UnaryExpr, @DoUMinus@ | UPLUS, UnaryExpr | Operand;\
Operand             = NestedExpr | DecInteger;\
NestedExpr          = OPENPAREN, Additive_expr, CLOSEPAREN;\
# simple objects #\
DecInteger = @DecIntegerBegin@, 1~8(DecDigit, @DecDigit@), @DecIntegerEnd@, Blanks;\
DecDigit   = '0':'9';\
Blanks     = 0~?(' ' | '\\09' | '\\0a');\
# terminals #\
UMINUS = '-', Blanks;    UPLUS = '+', Blanks;\
MUL = '*', Blanks;       DIV = '/', Blanks;\
PLUS = '+', Blanks;      MINUS = '-', Blanks;\
OPENPAREN = '(', Blanks; CLOSEPAREN = ')', Blanks;\
. # end of grammar #\
"};

/* Action identifiers used in the EbnfGrammar in the same order that will be used for the action function pointer vector passed to parser */
static char const *GrammarActionsSorting[] =
{
  "Debug", // usefull when debugging grammar
  "StackReset",
  "DoPlus",
  "DoMinus",
  "DoMul",
  "DoDiv",
  "DoUMinus",
  "DecIntegerBegin",
  "DecDigit",
  "DecIntegerEnd",
  "PrintResult"
};

/* function for reading ebnf grammar */
static FREAD_PROTO(EbnfGrammarRead)
{
  if(Pos + NumChar >= sizeof(EbnfGrammar))
    return FAIL;
  *Ch = &EbnfGrammar[Pos];
  return PASS;
}

#define BIN_GRAMMAR_SIZE 6000 /* usually enough room also for very complex grammars */
static unsigned char BinGrammar[BIN_GRAMMAR_SIZE];

/* function for writing the compiled grammar */
static FWRITE_PROTO(CompiledGrammarWrite)
{
  if(Pos + NumChar >= sizeof(BinGrammar))
  {
    printf("BIN_GRAMMAR_SIZE too small\r\n");
    return FAIL;
  }
  memcpy(&BinGrammar[Pos], Ch, NumChar);
  return PASS;
}

/******** data to be parsed *********/
const unsigned char SourceData[] = {"\
 -3 + 5*( 8 / 2 )                 \
"};

/* function for reading data to be parsed */
static FREAD_PROTO(SourceRead)
{
  if(Pos + NumChar >= sizeof(SourceData))
    return FAIL;
  *Ch = &SourceData[Pos];
  return PASS;
}

/* action functions used in grammar */
static F_ACTION_PROTO(Debug)
{
  return PASS;
}

static long Stack[10]; /* stack for nested parentheses in the expression */
static int SP; /* stack pointer */

static F_ACTION_PROTO(ActionStackReset)
{
  SP = 0;
  return PASS;
}

static F_ACTION_PROTO(ActionDoPlus)
{ /* pop operands, do sum, push result */
  Stack[SP - 2] = Stack[SP - 2] + Stack[SP - 1];
  SP--;
  return PASS;
}

static F_ACTION_PROTO(ActionDoMinus)
{ /* pop operands, do subtraction, push result */
  Stack[SP - 2] = Stack[SP - 2] - Stack[SP - 1];
  SP--;
  return PASS;
}

static F_ACTION_PROTO(ActionDoMul)
{ /* pop operands, do multiplication, push result */
  Stack[SP - 2] = Stack[SP - 2] * Stack[SP - 1];
  SP--;
  return PASS;
}

static F_ACTION_PROTO(ActionDoDiv)
{ /* pop operands, do division, push result */
  Stack[SP - 2] = Stack[SP - 2] / Stack[SP - 1];
  SP--;
  return PASS;
}

static F_ACTION_PROTO(ActionDoUMinus)
{ /* pop operand, change sign, push result */
  Stack[SP - 1] = -Stack[SP - 1];
  return PASS;
}

static F_ACTION_PROTO(ActionDecIntegerBegin)
{ /* start of an operand */
  Stack[SP] = 0;
  return PASS;
}

static F_ACTION_PROTO(ActionDecDigit)
{ /* digit of an operand */
  const unsigned char *Ch;

  if(FAIL == SourceRead(&Ch, SourcePos - 1, 1))
    return FAIL;
  Stack[SP] = 10 * Stack[SP] + *Ch - '0';
  /* TODO: check for overflow */
  return PASS;
}

static F_ACTION_PROTO(ActionDecIntegerEnd)
{ /* end of an operand */
  SP++;
  return PASS;
}

static F_ACTION_PROTO(ActionPrintResult)
{ /* print expression evaluation result */
  if(SP!=1)
    return FAIL; /* should never happen if grammar is correct */
  printf((char*)Arg, (int)Stack[0]);
  return PASS;
}

static F_ACTION *ActFunc[] = { /* beware: in the same order of GrammarActionsSorting vector of actions metaid */
  Debug,
  ActionStackReset,
  ActionDoPlus,
  ActionDoMinus,
  ActionDoMul,
  ActionDoDiv,
  ActionDoUMinus,
  ActionDecIntegerBegin,
  ActionDecDigit,
  ActionDecIntegerEnd,
  ActionPrintResult
};


int main()
{
  unsigned int BinGrammarSize;

  BinGrammarSize = EbnfGrammarCompiler(EbnfGrammarRead, GrammarActionsSorting, sizeof(GrammarActionsSorting) / sizeof(char *), CompiledGrammarWrite);
  if(!BinGrammarSize)
  {
    printf("Grammar NOK\r\n");
    return 0;
  }
  printf("Grammar OK\r\n");

  if(FAIL == Parse(BinGrammar, SourceRead, ActFunc))
  {
    printf("Parsing NOK\r\n");
    return 0;
  }
  printf("Parsing OK\r\n");
  return 1;
}
