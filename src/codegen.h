/*
 *  Copyright 2001-2014 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Ragel.
 *
 *  Ragel is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Ragel is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Ragel; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _C_CODEGEN_H
#define _C_CODEGEN_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "common.h"
#include "gendata.h"
#include "vector.h"

using std::string;
using std::ostream;

/* Integer array line length. */
//#define IALL 8

#define IALL_INTEGRAL 8
#define IALL_STRING 128


/* Forwards. */
struct RedFsmAp;
struct RedStateAp;
struct CodeGenData;
struct GenAction;
struct NameInst;
struct GenInlineItem;
struct GenInlineList;
struct RedAction;
struct LongestMatch;
struct LongestMatchPart;

string itoa( int i );

struct TableArray;
typedef Vector<TableArray*> ArrayVector;
struct CodeGen;


struct TableArray
{
	enum State {
		InitialState = 1,
		AnalyzePass,
		GeneratePass
	};
		
	TableArray( const char *name, CodeGen &codeGen );

	void start();
	void startAnalyze();
	void startGenerate();

	void setType( std::string type, int width, bool isChar )
	{
		this->type = type;
		this->width = width;
		this->isChar = isChar;
	}

	std::string ref() const;

	void value( long long v );

	void valueAnalyze( long long v );
	void valueGenerate( long long v );
	void stringGenerate( long long value );

	void finish();
	void finishAnalyze();
	void finishGenerate();

	void setState( TableArray::State state )
		{ this->state = state; }

	long long size();

	State state;
	const char *name;
	std::string type;
	int width;
	bool isSigned;
	bool isChar;
	bool stringTables;
	int iall;
	long long values;
	long long min;
	long long max;
	CodeGen &codeGen;
	std::ostream &out;
	int ln;
};

struct IlOpts
{
	IlOpts( int targState, bool inFinish, bool csForced )
		: targState(targState), inFinish(inFinish), csForced(csForced) {}

	int targState;
	bool inFinish;
	bool csForced;
};


/*
 * class CodeGen
 */
class CodeGen : public CodeGenData
{
public:
	CodeGen( const CodeGenArgs &args );

	virtual ~CodeGen() {}

	virtual void writeInit();
	virtual void writeStart();
	virtual void writeFirstFinal();
	virtual void writeError();
	virtual void statsSummary();

protected:
	friend class TableArray;
	typedef Vector<TableArray*> ArrayVector;
	ArrayVector arrayVector;

	string FSM_NAME();
	string START_STATE_ID();
	void taActions();
	string TABS( int level );
	string KEY( Key key );
	string LDIR_PATH( char *path );

	void ACTION( ostream &ret, GenAction *action, IlOpts opts );
	void NFA_CONDITION( ostream &ret, GenAction *condition, bool last );
	void CONDITION( ostream &ret, GenAction *condition );
	string ALPH_TYPE();

	bool isAlphTypeSigned();
	long long tableData;
	bool stringTables;

	virtual string GET_KEY();

	string P();
	string PE();
	string vEOF();

	string ACCESS();
	string vCS();
	string STACK();
	string TOP();
	string TOKSTART();
	string TOKEND();
	string ACT();

	string DATA_PREFIX();
	string START() { return DATA_PREFIX() + "start"; }
	string ERROR() { return DATA_PREFIX() + "error"; }
	string FIRST_FINAL() { return DATA_PREFIX() + "first_final"; }

	string CAST( string type );

	string ARR_TYPE( const TableArray &ta )
		{ return ta.type; }

	string ARR_REF( const TableArray &ta )
		{ return ta.ref(); }

	void INLINE_LIST( ostream &ret, GenInlineList *inlineList, 
			int targState, bool inFinish, bool csForced );
	virtual void GOTO( ostream &ret, int gotoDest, bool inFinish ) = 0;
	virtual void CALL( ostream &ret, int callDest, int targState, bool inFinish ) = 0;
	virtual void NCALL( ostream &ret, int callDest, int targState, bool inFinish ) = 0;
	virtual void NEXT( ostream &ret, int nextDest, bool inFinish ) = 0;
	virtual void GOTO_EXPR( ostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;
	virtual void NEXT_EXPR( ostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;
	virtual void CALL_EXPR( ostream &ret, GenInlineItem *ilItem, 
			int targState, bool inFinish ) = 0;
	virtual void NCALL_EXPR( ostream &ret, GenInlineItem *ilItem, 
			int targState, bool inFinish ) = 0;
	virtual void RET( ostream &ret, bool inFinish ) = 0;
	virtual void NRET( ostream &ret, bool inFinish ) = 0;
	virtual void BREAK( ostream &ret, int targState, bool csForced ) = 0;
	virtual void NBREAK( ostream &ret, int targState, bool csForced ) = 0;
	virtual void CURS( ostream &ret, bool inFinish ) = 0;
	virtual void TARGS( ostream &ret, bool inFinish, int targState ) = 0;
	void EXEC( ostream &ret, GenInlineItem *item, int targState, int inFinish );
	void LM_SWITCH( ostream &ret, GenInlineItem *item, int targState, 
			int inFinish, bool csForced );
	void LM_EXEC( ostream &ret, GenInlineItem *item, int targState, int inFinish );
	void SET_ACT( ostream &ret, GenInlineItem *item );
	void INIT_TOKSTART( ostream &ret, GenInlineItem *item );
	void INIT_ACT( ostream &ret, GenInlineItem *item );
	void SET_TOKSTART( ostream &ret, GenInlineItem *item );
	void SET_TOKEND( ostream &ret, GenInlineItem *item );
	void GET_TOKEND( ostream &ret, GenInlineItem *item );

	void HOST_STMT( ostream &ret, GenInlineItem *item, int targState, bool inFinish, bool csForced );
	void HOST_EXPR( ostream &ret, GenInlineItem *item, int targState, bool inFinish, bool csForced );
	void HOST_TEXT( ostream &ret, GenInlineItem *item, int targState, bool inFinish, bool csForced );
	void GEN_STMT( ostream &ret, GenInlineItem *item, int targState, bool inFinish, bool csForced );
	void GEN_EXPR( ostream &ret, GenInlineItem *item, int targState, bool inFinish, bool csForced );

	void STATE_IDS();

	string ERROR_STATE();
	string FIRST_FINAL_STATE();

	string STR( int v );

	bool outLabelUsed;
	bool testEofUsed;
	bool againLabelUsed;
	bool useIndicies;
	bool matchCondLabelUsed;

	void VALUE( string type, string name, string value );

	string ACCESS_OPER()
		{ return ""; }

	string OPEN_HOST_EXPR()
		{ return "("; }

	string OPEN_HOST_EXPR( string fileName, int line )
		{ return "("; }

	string CLOSE_HOST_EXPR()
		{ return ")"; }

	string OPEN_HOST_BLOCK( string fileName, int line )
	{ 
		if ( lineDirectives )
			return "{\n#line " + STR(line) + " \"" + fileName + "\"\n";
		else
			return "{\n";
	}

	string OPEN_HOST_BLOCK( GenInlineExpr *inlineExpr )
	{
		return OPEN_HOST_BLOCK( inlineExpr->loc.fileName, inlineExpr->loc.line );
	}

	string CLOSE_HOST_BLOCK()
		{ return "}"; }

	string OPEN_HOST_PLAIN()
		{ return ""; }

	string CLOSE_HOST_PLAIN()
		{ return ""; }

	string OPEN_GEN_EXPR()
		{ return "("; }

	string CLOSE_GEN_EXPR()
		{ return ")"; }

	string OPEN_GEN_BLOCK()
		{ return "{"; }

	string CLOSE_GEN_BLOCK()
		{ return "}"; }

	string OPEN_GEN_PLAIN()
		{ return ""; }

	string CLOSE_GEN_PLAIN()
		{ return ""; }
	
	string UINT()
		{ return "unsigned int"; }

	string INDEX( string type, string name )
	{
		return "const " + type + " *" + name;
	}

	string ENTRY()
	{
		return "";
	}

	string LABEL( string name )
	{
		return name + ": ";
	}

	string OFFSET( string arr, string off )
	{
		return "( " + arr + " + (" + off + "))";
	}

	string TRUE()
	{
		return "1";
	}

	string DEREF( string arr, string off )
	{
		return "(*( " + off + "))";
	}
	
	string CASE( string val )
	{
		return "case " + val + ": ";
	}

	string DEFAULT()
	{
		return "default:";
	}

	string CEND( )
	{
		return " break; ";
	}

	string FALLTHROUGH()
	{
		return " ";
	}

	string NIL()
	{
		return "0";
	}

	string EXPORT( string type, string name, string value )
	{
		return "#define " + name + " " + value;
	}

public:
	virtual void writeExports();
};

#endif
