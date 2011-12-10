/* General purpose parser module */

#ifndef COMPPARSER_H_INCLUDED
#define COMPPARSER_H_INCLUDED

#define FAIL 0
#define PASS 1

/* Prototype for the SourceRead function that the user must provide to parser: 
   the function should read the source file from position Pos for NumChar chars.
   It is up to the function to provide the pointer to the buffer where returned data are stored.
   Note: the parser will not ask the function to return a number of char more than the longest terminal.
   Must return PASS if succeeded, FAIL if not. */
#define FREAD_PROTO(FuncName) int FuncName(const unsigned char *Ch[], unsigned int Pos, unsigned char NumChar)
typedef FREAD_PROTO(FREAD);

/* Action functions prototype. Actions receive a pointer to the argument and the argument length.
   Must return PASS if succeeded, FAIL if not. */
#define F_ACTION_PROTO(FuncName) int FuncName(const unsigned char Arg[], unsigned char ArgLen)
typedef F_ACTION_PROTO(F_ACTION);

/* Parsing function */
extern int Parse(const unsigned char Grammar[], FREAD SourceRead, F_ACTION *ActFunc[]);

/* can be used in action functions to know or set actual source position */
extern unsigned long SourcePos;

#endif
/* end of CompParser.h */
