/* Ebnf like grammar compiler for general purpose module */
#include <stdio.h>
#include <string.h>
#include "EbnfGrammarCompiler.h"

#ifdef _MSC_VER
  #pragma warning(disable: 4996)
#endif

/*
# CompParserV2 grammar grammar with actions: #
Passes           = @ActionsInit@, Grammar, @ResetSourcePos@, Grammar, Blanks; # 2 pass compilation #
Grammar          = Blanks, 1~?SyntaxRule, '.';
SyntaxRule       = MetaId, @SymDef@, Blanks, '=', Blanks, OrExpr, ';', Blanks;
OrExpr           = @ExprBegin@, AndExpr, 0~?('|', @NextExpr@, Blanks, AndExpr), @Pop@;
AndExpr          = @ExprBegin@, 0~1(UnaryExpr, @NextExpr@, 0~?(',', Blanks, UnaryExpr, @NextExpr@)), @Pop@;
UnaryExpr        = 0~?UnaryOp, Primary;
UnaryOp          = Integer, @CopyVar@, Blanks, 0~1('~', Blanks, (Integer, Blanks | '?', @SetVar0@, Blanks)), @RepProc@ |
                   '!', Blanks, @RewProc=0@ | '&', Blanks, @RewProc=1@;
Primary          = GroupedSequence | CharRange | String | Action | MetaId, @RefProc@, Blanks;
GroupedSequence  = '(', @GroupedSequenceProc@, Blanks, OrExpr, ')', Blanks;
CharRange        = '\'', !'\'', Char, '\'', Blanks, ':', @CopyVar@, Blanks, '\'', !'\'', Char, '\'', @CharRangeProc@, Blanks;
String           = '\'', @StringBegin@, 0~?(!'\'', Char, @WriteVar@), '\'', @StringProc@, Blanks;
Action           = '@', Blanks, MetaId, @ActionBegin@, Blanks, 0~1('=', 0~?(!'@', Char, @WriteVar@)), '@', @ActionProc@, Blanks;
MetaId           = @StoreSourcePos@, Letter,  0~?(Letter | DecimalDigit);
Integer          = @StoreSourcePos@, 1~3DecimalDigit, @StoreInt@;
Letter           = 'a':'z' | 'A':'Z' | '_' | '$';
DecimalDigit     = '0':'9';
HexDigit         = '0':'9' | 'a':'f' | 'A':'F';
Char             = EscapeSequence | '\00':'\ff', @StoreChar@; # escape sequence or any char #
EscapeSequence   = '\\', (2HexDigit, @StoreHex@ | '\00':'\ff', @StoreChar@);
Blanks           = 0~?(' ' | '\09':'\0d' | Remark);
Remark           = '\23', 0~?(!'\23', '\00':'\ff'), '\23';.

Actions description:
@ActionsInit@ reset stack, symbol list, dest position.
@ResetSourcePos@ set source and dest position to 0.
@SymDef@ add a symbol in symbol list. MetaId is from var0 to current position of source. The symbol list item stores the MetaId position, length and the current position in output binary grammar.
@ExprBegin@ Push dest position, write 0, 0 to dest. 
@NextExpr@ write current destination position in the destination position pop-ed from stack. Push dest position, write 0, 0 to dest.
@Pop@ dummy pop.
@CopyVar@ copy var0 to var1.
@SetVar0@  set var0 to UCHAR_MAX ( =0xFF )
@RepProc@ writes to dest the REP_OP code and the MinRep, ExtraOptinalRep from var1 and var0.
@RewProc@ if argument is 0 writes to dest the NOT_OP code otherwise the REW_OP code.
@GroupedSequenceProc@ writes to dest the OR_EXPR_POS code and 0, 3.
@CharRangeProc@ writes to dest the CHAR_RANGE code and var1, var0.
@StringBegin@ writes to dest the STRING_EQ code, save dest position to var1, write a 0 to dest.
@WriteVar@ writes var0 to dest.
@StringProc@ writes to dest at position var1 the current dest position - var1 - 1. If the value is 1 also chances the code at position var1 - 1 to CHAR_EQ end remove byte at position var1.
@StoreSourcePos@ set var0 with current source position
@StoreInt@ set var0 with the asci to integer conversion of souce at position var0.
@StoreChar@ set var0 with previous source byte value.
@StoreHex@ convert the previous 2 source hex digit to a value into var0.
@ActionBegin@ writes to dest the ACTION code, the action id matching the MetaId pointed by var0 to current source position, writes to var1 the dest position, writes a 0 to dest.
@ActionProc@ writes to dest at position var1 the current dest position - var1 - 1.
@RefProc@ writes to dest the OR_EXPR_POS code followed by 2 bytes relative position in dest of the symbol pointed by var0 to current position in source.

# CompParserV2 grammar grammar without actions: #
Grammar          = Blanks, 1~?SyntaxRule, '.', Blanks;
SyntaxRule       = MetaId, Blanks, '=', Blanks, OrExpr, ';', Blanks;
OrExpr           = AndExpr, 0~?('|', Blanks, AndExpr);
AndExpr          = 0~1(UnaryExpr, 0~?(',', Blanks, UnaryExpr));
UnaryExpr        = 0~?UnaryOp, Primary;
UnaryOp          = Integer, Blanks, 0~1('~', Blanks, (Integer, Blanks | '?', Blanks)) | '!', Blanks | '&', Blanks;
Primary          = GroupedSequence | CharRange | String | Action | MetaId, Blanks;
GroupedSequence  = '(', Blanks, OrExpr, ')', Blanks;
CharRange        = '\'', !'\'', Char, '\'', Blanks, ':', Blanks, '\'', !'\'', Char, '\'', Blanks;
String           = '\'', 0~?(!'\'', Char), '\'', Blanks;
Action           = '@', Blanks, MetaId, Blanks, 0~1('=', 0~?(!'@', Char)), '@', Blanks;
MetaId           = Letter,  0~?(Letter | DecimalDigit);
Integer          = 1~3DecimalDigit;
Letter           = 'a':'z' | 'A':'Z' | '_' | '$';
DecimalDigit     = '0':'9';
HexDigit         = '0':'9' | 'a':'f' | 'A':'F';
Char             = EscapeSequence | '\00':'\ff'; # escape sequence or any char #
EscapeSequence   = '\\', (2HexDigit | '\00':'\ff');
Blanks           = 0~?(' ' | '\09':'\0d' | Remark);
Remark           = '\23', 0~?(!'\23', '\00':'\ff'), '\23';.

Compiled grammar grammar :
OrExpr = <NextAndExprRelativePosition AndExpr>; # list of alternatives. NextAndExprRelativePosition==0 means no more alternatives #
AndExpr = NextUnaryExprRelativePosition {UnaryExpr NextUnaryExprRelativePosition}; # NextUnaryExprRelativePosition==0 means no more UnaryExpr #
UnaryExpr = OR_EXPR_POS OrExprRelativePosition | ACTION ActionId Arglen Arg |  CHAR_EQ Char | REP_OP MinRep ExtraOptinalRep UnaryExpr |
            CHAR_RANGE Char Char | STRING_EQ StringLen String # | NOT UnaryExpr | AND UnaryExpr # ;.

note: NextAndExprRelativePosition, NextUnaryExprRelativePosition, OrExprRelativePosition are 2 bytes long
codes: OR_EXPR_POS = 0; ACTION = -1; CHAR_EQ = -2; REP_OP = -3; CHAR_RANGE = -4; STRING_EQ -5; NOT_OP = -6; REW_OP = -7
*/

static const unsigned char GrammarBin[] =
{ /* translation in compiled format of CompParserV2 grammar grammar with actions */
  0, 0, 0, 5, 255, 0, 0, 0, 5, 0, 0, 20, 0, 5, 255, 1, 0, 0, 5, 0, 
  0, 10, 0, 5, 0, 3, 48, 0, 0, 0, 0, 0, 5, 0, 3, 39, 0, 8, 253, 1, 
  255, 0, 0, 9, 0, 4, 254, 46, 0, 0, 0, 0, 0, 5, 0, 2, 75, 0, 5, 255, 
  2, 0, 0, 5, 0, 3, 8, 0, 4, 254, 61, 0, 5, 0, 2, 255, 0, 5, 0, 0, 
  14, 0, 4, 254, 59, 0, 5, 0, 2, 241, 0, 0, 0, 0, 0, 5, 255, 3, 0, 0, 
  5, 0, 0, 41, 0, 31, 253, 0, 255, 0, 0, 3, 0, 0, 0, 4, 254, 124, 0, 5, 
  255, 4, 0, 0, 5, 0, 2, 203, 0, 5, 0, 0, 12, 0, 0, 0, 5, 255, 5, 0, 
  0, 0, 0, 0, 0, 5, 255, 3, 0, 0, 53, 253, 0, 1, 0, 0, 3, 0, 0, 0, 
  5, 0, 0, 48, 0, 5, 255, 4, 0, 0, 31, 253, 0, 255, 0, 0, 3, 0, 0, 0, 
  4, 254, 44, 0, 5, 0, 2, 143, 0, 5, 0, 0, 19, 0, 5, 255, 4, 0, 0, 0, 
  0, 0, 0, 5, 255, 5, 0, 0, 0, 0, 0, 0, 8, 253, 0, 255, 0, 0, 10, 0, 
  5, 0, 0, 125, 0, 0, 0, 82, 0, 5, 0, 1, 195, 0, 5, 255, 6, 0, 0, 5, 
  0, 2, 88, 0, 58, 253, 0, 1, 0, 0, 3, 0, 0, 0, 4, 254, 126, 0, 5, 0, 
  2, 69, 0, 37, 0, 0, 3, 0, 14, 0, 5, 0, 1, 154, 0, 5, 0, 2, 52, 0, 
  0, 0, 0, 0, 4, 254, 63, 0, 5, 255, 7, 0, 0, 5, 0, 2, 34, 0, 0, 0, 
  0, 0, 5, 255, 8, 0, 0, 0, 0, 19, 0, 4, 254, 33, 0, 5, 0, 2, 12, 0, 
  6, 255, 9, 1, 48, 0, 0, 0, 0, 0, 4, 254, 38, 0, 5, 0, 1, 249, 0, 6, 
  255, 9, 1, 49, 0, 0, 0, 9, 0, 5, 0, 0, 51, 0, 0, 0, 9, 0, 5, 0, 
  0, 74, 0, 0, 0, 9, 0, 5, 0, 0, 134, 0, 0, 0, 9, 0, 5, 0, 0, 179, 
  0, 0, 0, 0, 0, 5, 0, 0, 255, 0, 5, 255, 21, 0, 0, 5, 0, 1, 188, 0, 
  0, 0, 0, 0, 4, 254, 40, 0, 5, 255, 10, 0, 0, 5, 0, 1, 170, 0, 5, 0, 
  254, 185, 0, 4, 254, 41, 0, 5, 0, 1, 156, 0, 0, 0, 0, 0, 4, 254, 39, 0, 
  5, 250, 254, 39, 0, 5, 0, 1, 71, 0, 4, 254, 39, 0, 5, 0, 1, 129, 0, 4, 
  254, 58, 0, 5, 255, 6, 0, 0, 5, 0, 1, 115, 0, 4, 254, 39, 0, 5, 250, 254, 
  39, 0, 5, 0, 1, 34, 0, 4, 254, 39, 0, 5, 255, 11, 0, 0, 5, 0, 1, 87, 
  0, 0, 0, 0, 0, 4, 254, 39, 0, 5, 255, 12, 0, 0, 27, 253, 0, 255, 0, 0, 
  3, 0, 0, 0, 5, 250, 254, 39, 0, 5, 0, 0, 243, 0, 5, 255, 13, 0, 0, 0, 
  0, 4, 254, 39, 0, 5, 255, 14, 0, 0, 5, 0, 1, 33, 0, 0, 0, 0, 0, 4, 
  254, 64, 0, 5, 0, 1, 20, 0, 5, 0, 0, 72, 0, 5, 255, 19, 0, 0, 5, 0, 
  1, 5, 0, 43, 253, 0, 1, 0, 0, 3, 0, 0, 0, 4, 254, 61, 0, 27, 253, 0, 
  255, 0, 0, 3, 0, 0, 0, 5, 250, 254, 64, 0, 5, 0, 0, 160, 0, 5, 255, 13, 
  0, 0, 0, 0, 0, 0, 4, 254, 64, 0, 5, 255, 20, 0, 0, 5, 0, 0, 204, 0, 
  0, 0, 0, 0, 5, 255, 15, 0, 0, 5, 0, 0, 53, 0, 26, 253, 0, 255, 0, 0, 
  3, 0, 9, 0, 5, 0, 0, 38, 0, 0, 0, 0, 0, 5, 0, 0, 63, 0, 0, 0, 
  0, 0, 0, 0, 5, 255, 15, 0, 0, 8, 253, 1, 2, 0, 0, 44, 0, 5, 255, 16, 
  0, 0, 0, 0, 9, 0, 5, 252, 97, 122, 0, 0, 0, 9, 0, 5, 252, 65, 90, 0, 
  0, 0, 8, 0, 4, 254, 95, 0, 0, 0, 0, 0, 4, 254, 36, 0, 0, 0, 0, 0, 
  5, 252, 48, 57, 0, 0, 0, 9, 0, 5, 252, 48, 57, 0, 0, 0, 9, 0, 5, 252, 
  97, 102, 0, 0, 0, 0, 0, 5, 252, 65, 70, 0, 0, 0, 9, 0, 5, 0, 0, 19, 
  0, 0, 0, 0, 0, 5, 252, 0, 255, 0, 5, 255, 17, 0, 0, 0, 0, 0, 0, 4, 
  254, 92, 0, 36, 0, 0, 3, 0, 17, 0, 8, 253, 2, 0, 0, 255, 188, 0, 5, 255, 
  18, 0, 0, 0, 0, 0, 0, 5, 252, 0, 255, 0, 5, 255, 17, 0, 0, 0, 0, 0, 
  0, 0, 0, 34, 253, 0, 255, 0, 0, 3, 0, 8, 0, 4, 254, 32, 0, 0, 0, 9, 
  0, 5, 252, 9, 13, 0, 0, 0, 0, 0, 5, 0, 0, 7, 0, 0, 0, 0, 0, 0, 
  0, 4, 254, 35, 0, 22, 253, 0, 255, 0, 0, 3, 0, 0, 0, 5, 250, 254, 35, 0, 
  5, 252, 0, 255, 0, 0, 0, 4, 254, 35, 0, 0
};

/* grammar related action functions */

static FREAD *pfRead;
static FWRITE *pfWrite;

static unsigned long DestPos = 0;

static long Var0, Var1;

#define SYM_SIZE 400 /* max number of syntax rules in the grammar */

struct tagSYMBOL
{
  unsigned long SymSourcePos, DestPos;
  unsigned char SymLen;
};
typedef struct tagSYMBOL SYMBOL;
static SYMBOL *pSym;
static unsigned short SymCount = 0;

#define STACK_SIZE 100 /* usually enough, depends on the nested brackets in grammar */
static long *pStack;
static unsigned short SP = 0;

static int Pass = 0;

static unsigned long MaxSourcePos;

static int Error(char *Msg)
{
  fprintf(stderr, "\r\n%s.\r\n", Msg);
  return FAIL;
}

static F_ACTION_PROTO(ActionsInit)
{ /* reset stack, symbol list, dest position, pass count. */
  DestPos = Pass = SP = SymCount = 0;
  return PASS;
}

static F_ACTION_PROTO(ResetSourcePos)
{ /* reset source and dest position and set second pass. */
  MaxSourcePos = DestPos = SourcePos = 0;
  Pass = 1;
  return PASS;
}

static F_ACTION_PROTO(SymDef)
{ /* add a symbol in symbol list. MetaId is from var0 to current position of source. The symbol list item stores the MetaId position, length and the current position in output binary grammar. */
  if(SymCount >= SYM_SIZE)
    return Error("Too may syntax rules, increase SYM_SIZE");
  pSym[SymCount].SymSourcePos = Var0;
  pSym[SymCount].SymLen = (unsigned char)(SourcePos - Var0);
  pSym[SymCount++].DestPos = DestPos;
  return PASS;
}

static F_ACTION_PROTO(ExprBegin)
{ /* Push dest position, write 0, 0 to dest.  */
  if(SP >= STACK_SIZE)
    return Error("Ebnf compiler stack overflow, increase STACK_SIZE");
  pStack[SP++] = DestPos;
  if(FAIL == pfWrite((unsigned char *)"\0\0", DestPos, 2))
    return Error("Compiled grammar write error");
  DestPos += 2;
  return PASS;
}

static F_ACTION_PROTO(NextExpr)
{ /* write difference from current destination position and the destination position pop-ed from stack in the pop-ed position. Push dest position, write 0, 0 to dest. */
  unsigned short BackAnnotateDestPos;
  unsigned char Ch[2];

  if(SP == 0)
    return Error("Ebnf compiler stack error");
  BackAnnotateDestPos = (unsigned short)pStack[--SP];
  Ch[0] = (unsigned char)((DestPos - BackAnnotateDestPos) >> 8);
  Ch[1] = ((DestPos - BackAnnotateDestPos) & 0xFF);
  if(FAIL == pfWrite(Ch, BackAnnotateDestPos, 2))
    return Error("Compiled grammar write error");
  pStack[SP++] = DestPos;
  if(FAIL == pfWrite((unsigned char *)"\0\0", DestPos, 2))
    return Error("Compiled grammar write error");
  DestPos += 2;
  return PASS;
}

static F_ACTION_PROTO(Pop)
{ /* dummy pop. */
  if(SP == 0)
    return Error("Ebnf compiler stack error");
  SP--;
  return PASS;
}

static F_ACTION_PROTO(CopyVar)
{ /* copy var0 to var1. */
  Var1 = Var0;
  return PASS;
}

static F_ACTION_PROTO(SetVar0)
{ /* set var0 to UCHAR_MAX ( =0xFF ) */
  Var0 = 0xFF;
  return PASS;
}

static F_ACTION_PROTO(RepProc)
{ /* writes to dest the REP_OP code and the MinRep, ExtraOptinalRep from var1 and var0. */
  unsigned char Ch[3];

  if(Var0 < Var1)
    return Error("Repetitions parameter error");
  Ch[0] = (unsigned char)-3;
  Ch[1] = (unsigned char)Var1;
  Ch[2] = (unsigned char)((Var0 == 0xFF) ? 0xFF : Var0 - Var1);
  if(FAIL == pfWrite(Ch, DestPos, 3))
    return Error("Compiled grammar write error");
  DestPos += 3;
  return PASS;
}

static F_ACTION_PROTO(RewProc)
{ /* if argument is 0 writes to dest the NOT_OP code otherwise the REW_OP code. */
  unsigned char Ch = (unsigned char)((*Arg == '0') ? -6 : -7);

  if(FAIL == pfWrite(&Ch, DestPos, 1))
    return Error("Compiled grammar write error");
  DestPos++;
  return PASS;
}

static F_ACTION_PROTO(GroupedSequenceProc)
{ /* writes to dest the OR_EXPR_POS code and 0, 3. */
  if(FAIL == pfWrite((unsigned char *)"\0\0\03", DestPos, 3))
    return Error("Compiled grammar write error");
  DestPos += 3;
  return PASS;
}

static F_ACTION_PROTO(CharRangeProc)
{ /* writes to dest the CHAR_RANGE code and var1, var0. */
  unsigned char Ch[3];

  Ch[0] = (unsigned char)-4;
  Ch[1] = (unsigned char)Var1;
  Ch[2] = (unsigned char)Var0;
  if(FAIL == pfWrite(Ch, DestPos, 3))
    return Error("Compiled grammar write error");
  DestPos += 3;
  return PASS;
}

static F_ACTION_PROTO(StringBegin)
{ /* writes to dest the STRING_EQ code, copy dest position to var1, write a 0 to dest. */
  unsigned char Ch[2];

  Ch[0] = (unsigned char)-5;
  Ch[1] = 0;
  if(FAIL == pfWrite(Ch, DestPos, 2))
    return Error("Compiled grammar write error");
  Var1 = ++DestPos;
  DestPos++;
  return PASS;
}

static F_ACTION_PROTO(WriteVar)
{ /* writes var0 to dest. */
  unsigned char Ch;

  Ch = (unsigned char)Var0;
  if(FAIL == pfWrite(&Ch, DestPos, 1))
    return Error("Compiled grammar write error");
  DestPos++;
  return PASS;
}

static F_ACTION_PROTO(StringProc)
{ /* writes to dest at position var1 the difference between current dest position and var1 - 1. */
  unsigned char Ch[2];

  if(DestPos - Var1 - 1 != 1) /* null or multi char string */
  {
    Ch[0] = (unsigned char)(DestPos - Var1 - 1);
    if(FAIL == pfWrite(Ch, Var1, 1))
      return Error("Compiled grammar write error");
  }
  else
  { /* single char string, replace the opcode to CHAR_EQ = -2, remove len field and overwrite with Var0 */
    Ch[0] = (unsigned char)-2;
    Ch[1] = (unsigned char)Var0;
    if(FAIL == pfWrite(Ch, Var1 - 1, 2))
      return Error("Compiled grammar write error");
    DestPos--;
  }
  return PASS;
}

static F_ACTION_PROTO(StoreSourcePos)
{ /* set var0 with current source position. */
  Var0 = SourcePos;
  return PASS;
}

static F_ACTION_PROTO(StoreInt)
{ /* set var0 with the asci to integer conversion of souce at position var0. */
  const unsigned char *SourceTerminal;
  unsigned char Len = (unsigned char)(SourcePos - Var0);

  if(FAIL == pfRead(&SourceTerminal, Var0, Len))
    return Error("Source read error");
  for(Var0 = 0; Len; Len--, SourceTerminal++)
    Var0 += 10 * Var0 + *SourceTerminal - '0';
  if(Var0 > 254)
    return Error("Invalid integer, maximum 254");
  return PASS;
}

static F_ACTION_PROTO(StoreChar)
{ /* set var0 with previous source byte value. */
  const unsigned char *SourceTerminal;

  if(FAIL == pfRead(&SourceTerminal, SourcePos - 1, 1))
    return Error("Source read error");
  Var0 = *SourceTerminal;
  return PASS;
}

static F_ACTION_PROTO(StoreHex)
{ /* convert the previous 2 source hex digit to a value into var0. */
  const unsigned char *SourceTerminal;

  if(FAIL == pfRead(&SourceTerminal, SourcePos - 2, 2))
    return Error("Source read error");

  if((SourceTerminal[0] >= '0') && (SourceTerminal[0] <= '9')) Var0 = (SourceTerminal[0] - '0') * 16;
  else if((SourceTerminal[0] >= 'A') && (SourceTerminal[0] <= 'F')) Var0 = (SourceTerminal[0] - 'A' + 10) * 16;
  else Var0 = (SourceTerminal[0] - 'a' + 10) * 16;

  if((SourceTerminal[1] >= '0') && (SourceTerminal[1] <= '9')) Var0 += SourceTerminal[1] - '0';
  else if((SourceTerminal[1] >= 'A') && (SourceTerminal[1] <= 'F')) Var0 += SourceTerminal[1] - 'A' + 10;
  else Var0 +=  SourceTerminal[1] - 'a' + 10;

  return PASS;
}

static char const * const *UserAct;
static unsigned int UserActNum;

static F_ACTION_PROTO(ActionBegin)
{ /* writes to dest the ACTION code, the action id matching the MetaId pointed by var0 to current source position, writes to var1 the dest position, writes a 0 to dest. */
  unsigned char i, Len = (unsigned char)(SourcePos - Var0);
  const unsigned char *SourceTerminal;
  unsigned char Ch[3];

  Ch[0] = (unsigned char)-1;
  Ch[2] = 0;
  if(FAIL == pfRead(&SourceTerminal, Var0, Len))
    return Error("Source read error");
  for(i = 0; i < UserActNum; i++)
    if(strlen(UserAct[i]) == Len)
      if(!memcmp(UserAct[i], SourceTerminal, Len))
      {
        Ch[1] = (unsigned char)i;
        if(FAIL == pfWrite(Ch, DestPos, 3))
          return Error("Compiled grammar write error");
        Var1 = DestPos + 2; 
        DestPos += 3;
        return PASS;
      }
  return Error("Referencing action in grammar not present in ActNames list");
}

static F_ACTION_PROTO(ActionProc)
{ /* writes to dest at position var1 the difference between current dest position and var1 - 1. */
  unsigned char Ch = (unsigned char)(DestPos - Var1 - 1);

  if(FAIL == pfWrite(&Ch, Var1, 1))
    return Error("Compiled grammar write error");
  return PASS;
}

static F_ACTION_PROTO(RefProc)
{ /* writes to dest the OR_EXPR_POS code followed by 2 bytes relative position in dest of the symbol pointed by var0 to current position in source. */
  unsigned int i;
  unsigned char Len = (unsigned char)(SourcePos - Var0);
  const unsigned char *SourceTerminal;
  unsigned char RefSym[256], Ch[3];

  if(FAIL == pfRead(&SourceTerminal, Var0, Len))
    return Error("Source read error");
  memcpy(RefSym, SourceTerminal, Len);
  Ch[0] = 0;
  for(i = 0; i < SymCount; i++) /* look for symbol in SymbolList */
    if(pSym[i].SymLen == Len)
    {
      if(FAIL == pfRead(&SourceTerminal, pSym[i].SymSourcePos, Len))
        return FAIL;
      if(!memcmp(SourceTerminal, RefSym, Len))
      {
        Ch[1] = (char)((pSym[i].DestPos - DestPos) >> 8);
        Ch[2] = (char)((pSym[i].DestPos - DestPos) & 0xFF);
        if(FAIL == pfWrite(Ch, DestPos, 3))
          return FAIL;
        DestPos += 3;
        return PASS;
      }
    }
  if(Pass == 1)
    return Error("Referencing a not defined symbol"); /* during second pass means that we are referencing a not defined symbol */ 
  Ch[1] = 0;
  Ch[2] = 0;
  if(FAIL == pfWrite(Ch, DestPos, 3))
    return Error("Compiled grammar write error");
  DestPos += 3;
  return PASS;
}

static F_ACTION_PROTO(Debug)
{
  return PASS;
}

static F_ACTION *ActFunc[] = /* vector of actions functions pointers used by GrammarBin */
{
  ActionsInit,
  ResetSourcePos,
  SymDef,
  ExprBegin,
  NextExpr,
  Pop,
  CopyVar,
  SetVar0,
  RepProc,
  RewProc,
  GroupedSequenceProc,
  CharRangeProc,
  StringBegin,
  WriteVar,
  StringProc,
  StoreSourcePos,
  StoreInt,
  StoreChar,
  StoreHex,
  ActionBegin,
  ActionProc,
  RefProc,
  Debug
};

static FREAD_PROTO(EbnfCompilerSourceRead)
{ /* wrapping the read function to trap the maximum read position */
  int Res = pfRead(Ch, Pos, NumChar);

  if(MaxSourcePos < Pos)
    MaxSourcePos = Pos;
  return Res;
}

SYMBOL Sym[SYM_SIZE]; /* might declare within EbnfGrammarCompiler so memory released at end of compilation */
long Stack[STACK_SIZE];

#pragma stackfunction 40
unsigned int EbnfGrammarCompiler(FREAD EbnfGrammarRead, char const * const ActNames[], unsigned char ActNum, FWRITE CompiledGrammarWrite)
{
  unsigned long i;
  unsigned char *Ch;

  pSym = Sym;
  pStack = Stack;
  pfRead = EbnfGrammarRead;
  pfWrite = CompiledGrammarWrite;
  UserActNum = ActNum;
  UserAct = ActNames;
  MaxSourcePos = 0;
  if( PASS == Parse(GrammarBin, EbnfCompilerSourceRead, ActFunc) )
    return (unsigned int)DestPos;

  fprintf(stderr, "\r\n...");
  for(i = (MaxSourcePos < 200) ? 0 : MaxSourcePos - 200; i < MaxSourcePos; i++)
  { /* write the ( up to ) 200 char before the failed parsing position */
    EbnfGrammarRead((const unsigned char **)&Ch, i, 1);
    putc(*Ch, stderr);
  }
  fprintf(stderr, "\r\nParsing error at position %d\r\n", (int)MaxSourcePos);
  return FAIL;
}
