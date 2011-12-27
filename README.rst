PARSER MODULE 
.............

:Stable release:  1.0.3

:Status:  Feature complete

:Maintainer:  `Raffaele Bacchi <https://github.com/leleb>`_ 

:Description:  A general purpose parser module


Key Features
============

* custom EBNF grammar

To Do
=====

* Check for duplicated symbol definition in grammar by the grammar compiler
* Check for left recursion in grammar by the grammar compiler.

Firmware Overview
=================

CompParser: a module for parsing data according to a compiled grammar.
EbnfGrammarCompiler: a module to generate compiled grammar from an EBNF like grammar.
ExpressionParser: a demo usage of the parser modules that implements an expression interpreter.
XmlParser: a demo usage of the parser modules that builds a tree structure that reflects the element structure of an xml source.

Known Issues
============

* <Bullet pointed list of problems>

Required Repositories
================

* xcommon git\@github.com:xcore/xcommon.git

Support
=======

Issues may be submitted via the Issues tab in this github repo. Response to any issues submitted are at the discretion of the maintainer of this component.
