#ifndef XML_GRAMMAR_H_INCLUDED
#define XML_GRAMMAR_H_INCLUDED

#include <stdio.h> /* for the printf function */
#include <string.h> /* for the memcpy function */
#include "EbnfGrammarCompiler.h"

/* ebnf grammar (to be compiled) */
static unsigned char EbnfGrammar[] = {"\
#\
EBNF for XML 1.0\
http://www.jelks.nu/XML/xmlebnf.html\
http://www.w3.org/TR/REC-xml/\
modified for single byte char ( ascii ) only\
#\
\
#Document#\
 document = @Init@, prolog, element, 0~?Misc, @Finish@;\
#Character Range#\
 #Char = x9 | xA | xD | [x20-xD7FF] | [xE000-xFFFD] | [x10000-x10FFFF]#\
 Char = '\\09' | '\\0A' | '\\0D' | '\\20':'\\FF';\
#White Space#\
 S = 1~?('\\20' | '\\09' | '\\0D' | '\\0A');\
#Names and Tokens#\
 #NameStartChar = ':' | [A-Z] | '_' | [a-z] | [xC0-xD6] | [xD8-xF6] | [xF8-x2FF] | [x370-x37D] | [x37F-x1FFF] |\
  [x200C-x200D] | [x2070-x218F] | [x2C00-x2FEF] | [x3001-xD7FF] | [xF900-xFDCF] | [xFDF0-xFFFD] | [x10000-xEFFFF]#\
 NameStartChar = ':' | 'A':'Z' | '_' | 'a':'z' | '\\C0':'\\D6' | '\\D8':'\\F6' | '\\F8':'\\FF';\
 #NameChar = NameStartChar | '-' | '.' | [0-9] | xB7 | [x0300-x036F] | [x203F-x2040]#\
 NameChar = NameStartChar | '-' | '.' | '0':'9' | '\\B7';\
 Name = NameStartChar, 0~?NameChar;\
 Names = Name, 0~?('\\20', Name);\
 Nmtoken = 1~?NameChar;\
 Nmtokens = Nmtoken, 0~?('\\20', Nmtoken);\
#Literals#\
AnyChar = '\\00':'\\FF';\
 EntityValue = '\"', 0~?(!('%' | '&' | '\"'), AnyChar| PEReference | Reference), '\"' |\
  '\\'', 0~?(!('%' | '&' | '\\''), AnyChar| PEReference | Reference), '\\'';\
 AttValue = '\"', 0~?(!('<' | '&' | '\"'), AnyChar | Reference), '\"' |\
  '\\'', 0~?(!('<' | '&' | '\\''), AnyChar | Reference), '\\'';\
 SystemLiteral = ('\"', 0~?(!'\"', AnyChar), '\"') | ('\\'', 0~?(!'\\'', AnyChar), '\\'');\
 PubidLiteral = '\"', 0~?PubidChar, '\"' | '\\'', 0~?(!'\\'', PubidChar), '\\'';\
 PubidChar = '\\20' | '\\0D' | '\\0A' | 'a':'z' | 'A':'Z' | '0':'9' | '-' | '\\'' | '(' | ')' |\
  '+' | ',' | '.' | '/' | ':' | '=' | '?' | ';' | '!' | '*' | '#' | '@' | '$' | '_' | '%' | ']';\
#Character Data#\
 #CharData = [^<&]* - ([^<&]* ']]>' [^<&]*) original rule: is ambiguous#\
 CharData = 0~?( !(']]>' | '<' | '&'), AnyChar);\
#Comments#\
 Comment = '<!--', 0~?((!'-', Char) | ('-', (!'-', Char))), '-->';\
#Processing Instructions#\
 #PI = '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' original rule: is ambiguous#\
 PI = '<?', PITarget, 0~1(S, 0~?(!'?>', Char)), '?>';\
 PITarget = !(('X' | 'x'), ('M' | 'm'), ('L' | 'l')), Name;\
#CDATA Sections#\
 CDSect = CDStart, CData, CDEnd;\
 CDStart = '<![CDATA[';\
 #CData = (Char* - (Char* ']]>' Char*)) original rule: is ambiguous#\
 CData = 0~?(!']]>', Char);\
 CDEnd = ']]>';\
#Prolog#\
 prolog = 0~1XMLDecl, 0~?Misc, 0~1(doctypedecl, 0~?Misc);\
 XMLDecl = '<?xml', VersionInfo, 0~1EncodingDecl, 0~1SDDecl, 0~1S, '?>';\
 VersionInfo = S, 'version', Eq, ('\\'', VersionNum, '\\'' | '\"', VersionNum, '\"');\
 Eq = 0~1S, '=', 0~1S;\
 VersionNum = '1.', 1~?'0':'9';\
 Misc = Comment | PI | S;\
#Document Type Definition#\
 doctypedecl = '<!DOCTYPE', S, Name, 0~1(S, ExternalID), 0~1S, 0~1('[', intSubset, ']', 0~1S), '>';\
 DeclSep = PEReference | S;\
 intSubset = 0~?(markupdecl | DeclSep);\
 markupdecl = elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment;\
#External Subset#\
 extSubset = 0~1TextDecl, extSubsetDecl;\
 extSubsetDecl =  0~?( markupdecl | conditionalSect | DeclSep);\
#Standalone Document Declaration#\
 SDDecl = S, 'standalone', Eq, (('\\'', ('yes' | 'no'), '\\'') | ('\"', ('yes' | 'no'), '\"'));\
#Element#\
 element = EmptyElemTag | STag, content, ETag;\
#Start-tag#\
 STag = '<', !'/', @STag@, Name, @NameEnd@, 0~?(S, Attribute), 0~1S, '>', @ContentBegin@;\
 Attribute = Name, Eq, AttValue;\
#End-tag#\
 ETag = '</', @ETag@, Name, 0~1S, '>';\
#Content of Elements#\
 content = 0~1CharData, 0~?((element | Reference | CDSect | PI | Comment), 0~1CharData);\
#Tags for Empty Elements#\
 EmptyElemTag = '<', Name, 0~?(S, Attribute), 0~1S, '/>';\
#Element Type Declaration#\
 elementdecl = '<!ELEMENT', S, Name, S, contentspec, 0~1S, '>';\
 contentspec = 'EMPTY' | 'ANY' | Mixed | children;\
#Element-content Models#\
 children = (choice | seq), 0~1('?' | '*' | '+');\
 cp = (Name | choice | seq), 0~1('?' | '*' | '+');\
 choice = '(', 0~1S, cp,  0~?(  0~1S, '|', 0~1S, cp ),  0~1S, ')';\
 seq = '(', 0~1S, cp, 0~?( 0~1S, ',', 0~1S, cp ), 0~1S, ')';\
#Mixed-content Declaration#\
 Mixed = '(', 0~1S, '#PCDATA', 0~?(0~1S, '|', 0~1S, Name), 0~1S, ')*' |\
 '(', 0~1S, '#PCDATA', 0~1S, ')';\
#Attribute-list Declaration#\
 AttlistDecl = '<!ATTLIST', S, Name, 0~?AttDef, 0~1S, '>';\
 AttDef = S, Name, S, AttType, S, DefaultDecl;\
#Attribute Types#\
 AttType = StringType | TokenizedType | EnumeratedType;\
 StringType = 'CDATA';\
 TokenizedType = 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY' | 'ENTITIES' | 'NMTOKEN' | 'NMTOKENS';\
#Enumerated Attribute Types#\
 EnumeratedType = NotationType | Enumeration;\
 NotationType = 'NOTATION', S, '(', 0~1S, Name, 0~?(0~?S, '|', 0~?S, Name), 0~1S, ')';\
 Enumeration = '(', 0~1S, Nmtoken, 0~?(0~1S, '|', 0~1S, Nmtoken), 0~1S, ')';\
#Attribute Defaults#\
 DefaultDecl = '#REQUIRED' | '#IMPLIED' | 0~1('#FIXED', S), AttValue;\
#Conditional Section#\
 conditionalSect = includeSect | ignoreSect;\
 includeSect = '<![', 0~1S, 'INCLUDE', 0~1S, '[', extSubsetDecl, ']]>';\
 ignoreSect = '<![', 0~1S, 'IGNORE', 0~1S, '[', 0~?ignoreSectContents, ']]>';\
 ignoreSectContents = Ignore,  0~?('<![', ignoreSectContents, ']]>', Ignore);\
 #Ignore = Char* - (Char* ('<![' | ']]>') Char*) original rule: is ambiguous#\
 Ignore = 0~?(!('<![' | ']]>'), Char);\
#Character Reference#\
 CharRef = '&#', 1~?'0':'9', ';' | '&#x', 1~?('0':'9' | 'a':'f' | 'A':'F'), ';';\
#Entity Reference#\
 Reference = EntityRef | CharRef;\
 EntityRef = '&', Name, ';';\
 PEReference = '%', Name, ';';\
#Entity Declaration#\
 EntityDecl = GEDecl | PEDecl;\
 GEDecl = '<!ENTITY', S, Name, S, EntityDef, 0~1S, '>';\
 PEDecl = '<!ENTITY', S, '%', S, Name, S, PEDef, 0~1S, '>';\
 EntityDef = EntityValue | ExternalID, 0~1NDataDecl;\
 PEDef = EntityValue | ExternalID;\
#External Entity Declaration#\
 ExternalID = 'SYSTEM', S, SystemLiteral | 'PUBLIC', S, PubidLiteral, S, SystemLiteral;\
 NDataDecl = S, 'NDATA', S, Name;\
#Text Declaration#\
 TextDecl = '<?xml', 0~1VersionInfo, EncodingDecl, 0~1S, '?>';\
#Well-Formed External Parsed Entity#\
 extParsedEnt = 0~1TextDecl, content;\
#Encoding Declaration#\
 EncodingDecl = S, 'encoding', Eq, ('\"', EncName, '\"' | '\\'', EncName, '\\'');\
 EncName = ('A':'Z' | 'a':'z'), 0~?('A':'Z' | 'a':'z' | '0':'9' | '.' | '_' | '-');\
#Notation Declarations#\
 NotationDecl = '<!NOTATION', S, Name, S, (ExternalID | PublicID), 0~1S, '>';\
 PublicID = 'PUBLIC', S, PubidLiteral;\
#Characters#\
Letter = BaseChar | Ideographic;\
#BaseChar = [x0041-x005A] | [x0061-x007A] | [x00C0-x00D6] | [x00D8-x00F6] | [x00F8-x00FF] | [x0100-x0131] | [x0134-x013E] | [x0141-x0148] | [x014A-x017E] | [x0180-x01C3] |\
 [x01CD-x01F0] | [x01F4-x01F5] | [x01FA-x0217] | [x0250-x02A8] | [x02BB-x02C1] | x0386 | [x0388-x038A] | x038C | [x038E-x03A1] | [x03A3-x03CE] | [x03D0-x03D6] | x03DA |\
 x03DC | x03DE | x03E0 | [x03E2-x03F3] | [x0401-x040C] | [x040E-x044F] | [x0451-x045C] | [x045E-x0481] | [x0490-x04C4] | [x04C7-x04C8] | [x04CB-x04CC] | [x04D0-x04EB] |\
 [x04EE-x04F5] | [x04F8-x04F9] | [x0531-x0556] | x0559 | [x0561-x0586] | [x05D0-x05EA] | [x05F0-x05F2] | [x0621-x063A] | [x0641-x064A] | [x0671-x06B7] | [x06BA-x06BE] |\
 [x06C0-x06CE] | [x06D0-x06D3] | x06D5 | [x06E5-x06E6] | [x0905-x0939] | x093D | [x0958-x0961] | [x0985-x098C] | [x098F-x0990] | [x0993-x09A8] | [x09AA-x09B0] | x09B2 |\
 [x09B6-x09B9] | [x09DC-x09DD] | [x09DF-x09E1] | [x09F0-x09F1] | [x0A05-x0A0A] | [x0A0F-x0A10] | [x0A13-x0A28] | [x0A2A-x0A30] | [x0A32-x0A33] | [x0A35-x0A36] | [x0A38-x0A39] |\
 [x0A59-x0A5C] | x0A5E | [x0A72-x0A74] | [x0A85-x0A8B] | x0A8D | [x0A8F-x0A91] | [x0A93-x0AA8] | [x0AAA-x0AB0] | [x0AB2-x0AB3] | [x0AB5-x0AB9] | x0ABD | x0AE0 | [x0B05-x0B0C] |\
 [x0B0F-x0B10] | [x0B13-x0B28] | [x0B2A-x0B30] | [x0B32-x0B33] | [x0B36-x0B39] | x0B3D | [x0B5C-x0B5D] | [x0B5F-x0B61] | [x0B85-x0B8A] | [x0B8E-x0B90] | [x0B92-x0B95] |\
 [x0B99-x0B9A] | x0B9C | [x0B9E-x0B9F] | [x0BA3-x0BA4] | [x0BA8-x0BAA] | [x0BAE-x0BB5] | [x0BB7-x0BB9] | [x0C05-x0C0C] | [x0C0E-x0C10] | [x0C12-x0C28] | [x0C2A-x0C33] |\
 [x0C35-x0C39] | [x0C60-x0C61] | [x0C85-x0C8C] | [x0C8E-x0C90] | [x0C92-x0CA8] | [x0CAA-x0CB3] | [x0CB5-x0CB9] | x0CDE | [x0CE0-x0CE1] | [x0D05-x0D0C] | [x0D0E-x0D10] |\
 [x0D12-x0D28] | [x0D2A-x0D39] | [x0D60-x0D61] | [x0E01-x0E2E] | x0E30 | [x0E32-x0E33] | [x0E40-x0E45] | [x0E81-x0E82] | x0E84 | [x0E87-x0E88] | x0E8A | x0E8D | [x0E94-x0E97] |\
 [x0E99-x0E9F] | [x0EA1-x0EA3] | x0EA5 | x0EA7 | [x0EAA-x0EAB] | [x0EAD-x0EAE] | x0EB0 | [x0EB2-x0EB3] | x0EBD | [x0EC0-x0EC4] | [x0F40-x0F47] | [x0F49-x0F69] | [x10A0-x10C5] |\
 [x10D0-x10F6] | x1100 | [x1102-x1103] | [x1105-x1107] | x1109 | [x110B-x110C] | [x110E-x1112] | x113C | x113E | x1140 | x114C | x114E | x1150 | [x1154-x1155] | x1159 | [x115F-x1161] |\
 x1163 | x1165 | x1167 | x1169 | [x116D-x116E] | [x1172-x1173] | x1175 | x119E | x11A8 | x11AB | [x11AE-x11AF] | [x11B7-x11B8] | x11BA | [x11BC-x11C2] | x11EB | x11F0 | x11F9 |\
 [x1E00-x1E9B] | [x1EA0-x1EF9] | [x1F00-x1F15] | [x1F18-x1F1D] | [x1F20-x1F45] | [x1F48-x1F4D] | [x1F50-x1F57] | x1F59 | x1F5B | x1F5D | [x1F5F-x1F7D] | [x1F80-x1FB4] |\
 [x1FB6-x1FBC] | x1FBE | [x1FC2-x1FC4] | [x1FC6-x1FCC] | [x1FD0-x1FD3] | [x1FD6-x1FDB] | [x1FE0-x1FEC] | [x1FF2-x1FF4] | [x1FF6-x1FFC] | x2126 | [x212A-x212B] | x212E |\
 [x2180-x2182] | [x3041-x3094] | [x30A1-x30FA] | [x3105-x312C] | [xAC00-xD7A3]#\
BaseChar = '\\41':'\\5A' | '\\61':'\\7A' | '\\C0':'\\D6' | '\\D8':'\\F6' | '\\F8':'\\FF';\
#Ideographic = [x4E00-x9FA5] | x3007 | [x3021-x3029];#\
Ideographic = !AnyChar;\
#CombiningChar = [x0300-x0345] | [x0360-x0361] | [x0483-x0486] | [x0591-x05A1] | [x05A3-x05B9] | [x05BB-x05BD] | x05BF | [x05C1-x05C2] | x05C4 | [x064B-x0652] | x0670 |\
 [x06D6-x06DC] | [x06DD-x06DF] | [x06E0-x06E4] | [x06E7-x06E8] | [x06EA-x06ED] | [x0901-x0903] | x093C | [x093E-x094C] | x094D | [x0951-x0954] | [x0962-x0963] |\
 [x0981-x0983] | x09BC | x09BE | x09BF | [x09C0-x09C4] | [x09C7-x09C8] | [x09CB-x09CD] | x09D7 | [x09E2-x09E3] | x0A02 | x0A3C | x0A3E | x0A3F | [x0A40-x0A42] |\
 [x0A47-x0A48] | [x0A4B-x0A4D] | [x0A70-x0A71] | [x0A81-x0A83] | x0ABC | [x0ABE-x0AC5] | [x0AC7-x0AC9] | [x0ACB-x0ACD] | [x0B01-x0B03] | x0B3C | [x0B3E-x0B43] |\
 [x0B47-x0B48] | [x0B4B-x0B4D] | [x0B56-x0B57] | [x0B82-x0B83] | [x0BBE-x0BC2] | [x0BC6-x0BC8] | [x0BCA-x0BCD] | x0BD7 | [x0C01-x0C03] | [x0C3E-x0C44] | [x0C46-x0C48] |\
 [x0C4A-x0C4D] | [x0C55-x0C56] | [x0C82-x0C83] | [x0CBE-x0CC4] | [x0CC6-x0CC8] | [x0CCA-x0CCD] | [x0CD5-x0CD6] | [x0D02-x0D03] | [x0D3E-x0D43] | [x0D46-x0D48] |\
 [x0D4A-x0D4D] | x0D57 | x0E31 | [x0E34-x0E3A] | [x0E47-x0E4E] | x0EB1 | [x0EB4-x0EB9] | [x0EBB-x0EBC] | [x0EC8-x0ECD] | [x0F18-x0F19] | x0F35 | x0F37 | x0F39 |\
 x0F3E | x0F3F | [x0F71-x0F84] | [x0F86-x0F8B] | [x0F90-x0F95] | x0F97 | [x0F99-x0FAD] | [x0FB1-x0FB7] | x0FB9 | [x20D0-x20DC] | x20E1 | [x302A-x302F] | x3099 | x309A#\
CombiningChar = !AnyChar;\
#Digit = [x0030-x0039] | [x0660-x0669] | [x06F0-x06F9] | [x0966-x096F] | [x09E6-x09EF] | [x0A66-x0A6F] | [x0AE6-x0AEF] | [x0B66-x0B6F] | [x0BE7-x0BEF] | [x0C66-x0C6F] |\
 [x0CE6-x0CEF] | [x0D66-x0D6F] | [x0E50-x0E59] | [x0ED0-x0ED9] | [x0F20-x0F29]#\
 Digit = '\\30':'\\39';\
#Extender = x00B7 | x02D0 | x02D1 | x0387 | x0640 | x0E46 | x0EC6 | x3005 | [x3031-x3035] | [x309D-x309E] | [x30FC-x30FE]#\
Extender = '\\B7';\
."};

/* Action identifiers used in the EbnfGrammar in the same order that will be used for the action function pointer vector passed to parser */
static char const *GrammarActionsSorting[] =
{
  "Debug",        //0
  "Init",         //1
  "STag",         //2
  "NameEnd",      //3
  "ContentBegin", //4
  "ETag",         //5
  "Finish",       //6
};

/* action functions used in grammar */
typedef struct tagELEMENT ELEMENT;
struct tagELEMENT /* Node structure */
{ 
  unsigned int NestLevel;
  unsigned int NamePosition, NameLen; // name position of the element in the source and length
  unsigned int ContentPosition, ContentLen; // content position of the element in the source and length
  ELEMENT *Parent; // pointer to parent
  unsigned int NumChildElements; // number of childs elements
  ELEMENT *Child; // vector of child ELEMENT
};

#define MAX_ELEMENTS 100  //max 100 elements total
static ELEMENT Element[MAX_ELEMENTS];
static unsigned int RootElementsNum;
static unsigned int TotalElementsNum;

#define MAX_NESTED_ELEMENTS 20  //max 20 nested elements
static unsigned int ParentsStack[MAX_NESTED_ELEMENTS];
static unsigned int ParentsStackPointer;

static F_ACTION_PROTO(fDebug)
{
  return PASS;
}

static F_ACTION_PROTO(fInit)
{
  RootElementsNum = TotalElementsNum = ParentsStackPointer = 0;
  return PASS;
}

static F_ACTION_PROTO(fSTag)
{ // new element at position TotalElementsNum of Element vector
  unsigned int ParentElementIndex;

  if(ParentsStackPointer == 0)
  {
    RootElementsNum++; // a new root element
    Element[TotalElementsNum].Parent = NULL;
  }
  else
  { // a new child element
    ParentElementIndex = ParentsStack[ParentsStackPointer - 1];
    Element[ParentElementIndex].NumChildElements++; // increment number of childs of the parent
    if(ParentElementIndex == TotalElementsNum - 1) // if the parent is just before the current element...
    {
      Element[ParentElementIndex].Child = &Element[TotalElementsNum]; //... then the element is its first child
      Element[TotalElementsNum].Parent = &Element[ParentElementIndex]; // set the pointer to the parent
    }
    else
      Element[TotalElementsNum].Parent = NULL; // not a first child
  }
  Element[TotalElementsNum].NestLevel = ParentsStackPointer;
  ParentsStack[ParentsStackPointer] = TotalElementsNum; // push new element in the parents stack
  if(ParentsStackPointer == MAX_NESTED_ELEMENTS)
    return FAIL;
  ParentsStackPointer++;
  Element[TotalElementsNum].NumChildElements = 0; // initialize new element with no childs
  Element[TotalElementsNum].Child = NULL;
  Element[TotalElementsNum].NamePosition = SourcePos;
  return PASS;
}

static F_ACTION_PROTO(fNameEnd)
{
  Element[TotalElementsNum].NameLen = SourcePos - Element[TotalElementsNum].NamePosition;
  return PASS;
}

static F_ACTION_PROTO(fContentBegin)
{
  Element[TotalElementsNum].ContentPosition = SourcePos;
  TotalElementsNum++;
  if(TotalElementsNum >= MAX_ELEMENTS)
    return FAIL;
  return PASS;
}

static F_ACTION_PROTO(fETag)
{
  ParentsStackPointer--;
  Element[ParentsStack[ParentsStackPointer]].ContentLen = SourcePos - Element[ParentsStack[ParentsStackPointer]].ContentPosition - 2;
  return PASS;
}

static F_ACTION_PROTO(fFinish)
{ /* bubble sorting the elements according to their nest level */
  unsigned int i;
  int Swapped;
  ELEMENT TmpElement;

  do
  {
    Swapped = 0;
    for( i = 0; i < TotalElementsNum - 1; i++)
    {
      if(Element[i].NestLevel > Element[i+1].NestLevel)
      { // note: childs have higher nest level than their parent and are already following the parent in the Element vector
        if(Element[i].NumChildElements) // if element i has childs
          Element[i].Child->Parent = &Element[i + 1]; // update parent pointer of first child of element i

        if(Element[i + 1].NumChildElements) // if element i has childs
          Element[i + 1].Child->Parent = &Element[i]; // update parent pointer of first child of element i+1

        if(Element[i].Parent != NULL) // if element i is the first child of its parent
          Element[i].Parent->Child = &Element[i + 1]; // update child pointer of the parent of element i

        if(Element[i + 1].Parent != NULL) // if element i+1 is the first child of its parent
          Element[i + 1].Parent->Child = &Element[i]; // update child pointer of the parent of element i+1

        // do the swap (could do using xor ...)
        TmpElement = Element[i];
        Element[i] = Element[i+1];
        Element[i+1] = TmpElement;
        Swapped = 1;
      }
    }
  }
  while(Swapped);
  for( i = 1; i < TotalElementsNum; i++) // update the parent pointer of not first childs
    if(Element[i].NestLevel && Element[i].Parent == NULL)
      Element[i].Parent = Element[i - 1].Parent;
  return PASS;
}

static F_ACTION *ActFunc[] = { /* beware: in the same order of GrammarActionsSorting vector of actions metaid */
  fDebug,        //0
  fInit,         //1
  fSTag,         //2
  fNameEnd,      //3
  fContentBegin, //4
  fETag,         //5
  fFinish,       //6
};

#endif
