/*
 ============================================================================
 Name        : XmlParser.c
 Description : Sample code for general purpose parser
 ============================================================================
 */
#include <stdio.h> /* for the printf function */
#include <string.h> /* for the memcpy function */
#include "XmlGrammar.h"
#include "XmlSource.h"

/* function for reading ebnf grammar */
static FREAD_PROTO(EbnfGrammarRead)
{
  if(Pos + NumChar >= sizeof(EbnfGrammar))
    return FAIL;
  *Ch = &EbnfGrammar[Pos];
  return PASS;
}

#define BIN_GRAMMAR_SIZE 5000 /* enough for the xml compiled grammar */
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

static unsigned int MaxSourcePos = 0;

/* function for reading source data */
static FREAD_PROTO(SourceRead)
{
  if(Pos + NumChar >= sizeof(Source))
    return FAIL;
  *Ch = &Source[Pos];
  if(MaxSourcePos < Pos + NumChar)
    MaxSourcePos = Pos + NumChar;
  return PASS;
}

static void DumpElement(ELEMENT elem)
{
  unsigned int i;
  const unsigned char *Str;

  for(i = 0; i < elem.NestLevel; i++)
    printf("  "); // indent = nest level
  SourceRead(&Str, elem.NamePosition, elem.NameLen); // write element name (using SourceRead function so could be retrived from no matter where)
  for(i = 0; i < elem.NameLen; i++)
    putchar(Str[i]);
  if(elem.NumChildElements == 0)
  { // if a leaf...
    printf(" = ");
    SourceRead(&Str, elem.ContentPosition, elem.ContentLen);
    for(i = 0; i < elem.ContentLen; i++)
      putchar(Str[i]); // ...writes the content
  }
  printf("\n");
  for(i= 0; i < elem.NumChildElements; i++) // recursively dumps the child elements
    DumpElement(elem.Child[i]);
}

int main()
{
  unsigned int BinGrammarSize, i;

  BinGrammarSize = EbnfGrammarCompiler(EbnfGrammarRead, GrammarActionsSorting, sizeof(GrammarActionsSorting) / sizeof(char *), CompiledGrammarWrite);
  if(!BinGrammarSize)
  {
    printf("Grammar NOK\r\n");
    return 0;
  }
  printf("Grammar OK\r\n");

  if(FAIL == Parse(BinGrammar, SourceRead, ActFunc))
  {
	printf("Parsing NOK at position %d\r\n", MaxSourcePos);
    return 0;
  }
  printf("Parsing OK\r\n");
  /* now tree structure ready */
  for( i = 0; i < RootElementsNum; i++)
    DumpElement(Element[i]);

  return 1;
}
