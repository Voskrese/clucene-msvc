/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team

* Updated by https://github.com/farfella/.
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "QueryParser.h"
#include "_TokenList.h"
#include "QueryToken.h"
#include "_Lexer.h"

#include "CLucene/util/CLStreams.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/_FastCharStream.h"

CL_NS_USE(util)

CL_NS_DEF2(queryParser,legacy)
Lexer::Lexer(QueryParserBase* queryparser, const wchar_t* query) {
   //Func - Constructor
   //Pre  - query != NULL and contains the query string
   //Post - An instance of Lexer has been created

	this->queryparser = queryparser;

   CND_PRECONDITION(query != NULL, L"query is NULL");

   //The InputStream of Reader must be destroyed in the destructor
   delSR = true;

   StringReader *r = _CLNEW StringReader(query);

   //Check to see if r has been created properly
   CND_CONDITION(r != NULL, L"Could not allocate memory for StringReader r");

   //Instantie a Faswchar_tStream instance using r and assign it to reader
   reader = _CLNEW FastCharStream(r);

   //Check to see if reader has been created properly
   CND_CONDITION(reader != NULL, L"Could not allocate memory for Faswchar_tStream reader");

   //The InputStream of Reader must be destroyed in the destructor
   delSR = true;

}


Lexer::Lexer(QueryParserBase* queryparser, BufferedReader* source) {
   //Func - Constructor
   //       Initializes a new instance of the Lexer class with the specified
   //       TextReader to lex.
   //Pre  - Source contains a valid reference to a Reader
   //Post - An instance of Lexer has been created using source as the reader

	this->queryparser = queryparser;

   //Instantie a FastCharStream instance using r and assign it to reader
   reader = _CLNEW FastCharStream(source);

   //Check to see if reader has been created properly
   CND_CONDITION(reader != NULL, L"Could not allocate memory for Faswchar_tStream reader");

   //The InputStream of Reader must not be destroyed in the destructor
   delSR  = false;
}


Lexer::~Lexer() {
   //Func - Destructor
   //Pre  - true
   //Post - if delSR was true the InputStream input of reader has been deleted
   //       The instance of Lexer has been destroyed

   if (delSR) {
      _CLDELETE(reader->input);
   }

   _CLDELETE(reader);
}


void Lexer::Lex(TokenList *tokenList) {
   //Func - Breaks the input stream onto the tokens list tokens
   //Pre  - tokens != NULL and contains a TokenList in which the tokens can be stored
   //Post - The tokens have been added to the TokenList tokens

   CND_PRECONDITION(tokenList != NULL, L"tokens is NULL");

   //Get all the tokens
   while(true) {
      //Add the token to the tokens list
	  
	  //Get the next token
	  QueryToken* token = _CLNEW QueryToken;
	  if ( !GetNextToken(token) ){
		_CLDELETE(token);
		break;
	  }
      tokenList->add(token);
   }

   //The end has been reached so create an EOF_ token
   //Add the final token to the TokenList _tokens
   tokenList->add(_CLNEW QueryToken( QueryToken::EOF_));
}


bool Lexer::GetNextToken(QueryToken* token) {
   while(!reader->Eos()) {
      int ch = reader->GetNext();

	  if ( ch == -1 )
		break;

      // skipping whitespaces
      if( _istspace(ch)!=0 ) {
         continue;
      }
      wchar_t buf[2] = {ch,'\0'};
      switch(ch) {
         case '+':
            token->set(buf, QueryToken::PLUS);
            return true;
         case '-':
            token->set(buf, QueryToken::MINUS);
            return true;
         case '(':
            token->set(buf, QueryToken::LPAREN);
            return true;
         case ')':
            token->set(buf, QueryToken::RPAREN);
            return true;
         case ':':
            token->set(buf, QueryToken::COLON);
            return true;
         case '!':
            token->set(buf, QueryToken::NOT);
            return true;
         case '^':
            token->set(buf, QueryToken::CARAT);
            return true;
         case '~':
            if( _istdigit( reader->Peek() )!=0 ) {
				wchar_t number[LUCENE_MAX_FIELD_LEN];
                ReadIntegerNumber(ch, number,LUCENE_MAX_FIELD_LEN);
                token->set(number, QueryToken::SLOP);
                return true;
            }else{
                token->set(buf, QueryToken::FUZZY);
                return true;
            }
			break;
         case '"':
			 return ReadQuoted(ch, token);
         case '[':
            return ReadInclusiveRange(ch, token);
         case '{':
            return ReadExclusiveRange(ch, token);
         case ']':
         case '}':
         case '*':
            queryparser->throwParserException( L"Unrecognized char %d at %d::%d.", 
               ch, reader->Column(), reader->Line() );
            return false;
         default:
            return ReadTerm(ch, token);

   // end of swith
      }

   }
   return false;
}


void Lexer::ReadIntegerNumber(const wchar_t ch, wchar_t* buf, int buflen) {
   int bp=0;
   buf[bp++] = ch;

   int c = reader->Peek();
   while( c!=-1 && _istdigit(c)!=0 && bp<buflen-1 ) {
      buf[bp++] = reader->GetNext();
      c = reader->Peek();
   }
   buf[bp++] = 0;
}


bool Lexer::ReadInclusiveRange(const wchar_t prev, QueryToken* token) {
   int ch = prev;
   std::wstring range;
   range.push_back(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();
	  if ( ch == -1 )
		break;
      range.push_back(ch);

      if(ch == ']'){
         token->set(range.c_str(), QueryToken::RANGEIN);
         return true;
      }
   }
   queryparser->throwParserException( L"Unterminated inclusive range! %d %d::%d",' ',
      reader->Column(),reader->Column());
   return false;
}


bool Lexer::ReadExclusiveRange(const wchar_t prev, QueryToken* token) {
   int ch = prev;
   std::wstring range;
   range.push_back(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();

	  if (ch==-1)
		break;
	  range.push_back(ch);

      if(ch == '}'){
         token->set(range.c_str(), QueryToken::RANGEEX);
        return true;
      }
   }
   queryparser->throwParserException( L"Unterminated exclusive range! %d %d::%d",' ',
      reader->Column(),reader->Column() );
   return false;
}

bool Lexer::ReadQuoted(const wchar_t prev, QueryToken* token) {
   int ch = prev;
   std::wstring quoted;
   quoted.push_back(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();

	  if (ch==-1)
		break;

      quoted.push_back(ch);

      if(ch == '"'){
         token->set(quoted.c_str(), QueryToken::QUOTED);
         return true;
      }
   }
   queryparser->throwParserException( L"Unterminated string! %d %d::%d",' ',
      reader->Column(),reader->Column());
   return false;
}


bool Lexer::ReadTerm(const wchar_t prev, QueryToken* token) {
   int ch = prev;
   bool completed = false;
   int32_t asteriskCount = 0;
   bool hasQuestion = false;

   std::wstring val;
   wchar_t buf[3]; //used for readescaped

   while(true) {
      switch(ch) {
		  case -1:
			  break;
         case '\\':
         {
            if ( ReadEscape(ch, buf) )
                val.append( buf );
			else
				return false;
         }
         break;

         case LUCENE_WILDCARDTERMENUM_WILDCARD_STRING:
            asteriskCount++;
            val.push_back(ch);
            break;
         case LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR:
            hasQuestion = true;
            val.push_back(ch);
            break;
         case '\n':
         case '\t':
         case ' ':
         case '+':
         case '-':
         case '!':
         case '(':
         case ')':
         case ':':
         case '^':
         case '[':
         case ']':
         case '{':
         case '}':
         case '~':
         case '"':
            // create new QueryToken
            reader->UnGet();
            completed = true;
            break;
         default:
            val.push_back(ch);
            break;
   // end of switch
      }

      if(completed || ch==-1 || reader->Eos() )
         break;
      else
         ch = reader->GetNext();
   }

   // create new QueryToken
   if(hasQuestion)
      token->set(val.c_str(), QueryToken::WILDTERM);
   else if(asteriskCount == 1 && val.c_str()[val.length() - 1] == '*')
      token->set(val.c_str(), QueryToken::PREFIXTERM);
   else if(asteriskCount > 0)
      token->set(val.c_str(), QueryToken::WILDTERM);
   else if( _tcsicmp(val.c_str(), L"AND")==0 || wcscmp(val.c_str(), L"&&")==0 )
      token->set(val.c_str(), QueryToken::AND_);
   else if( _tcsicmp(val.c_str(), L"OR")==0 || wcscmp(val.c_str(), L"||")==0)
      token->set(val.c_str(), QueryToken::OR);
   else if( _tcsicmp(val.c_str(), L"NOT")==0 )
      token->set(val.c_str(), QueryToken::NOT);
   else {
      bool isnum = true;
      int32_t nlen=val.length();
      for (int32_t i=0;i<nlen;++i) {
         wchar_t ch=val.c_str()[i];
         if ( _istalpha(ch) ) {
            isnum=false;
            break;
         }
      }

      if ( isnum )
         token->set(val.c_str(), QueryToken::NUMBER);
      else
         token->set(val.c_str(), QueryToken::TERM);
   }
   return true;
}


bool Lexer::ReadEscape(wchar_t prev, wchar_t* buf) {
   wchar_t ch = prev;
   int bp=0;
   buf[bp++] = ch;

   ch = reader->GetNext();
   int32_t idx = wcscspn( buf, L"\\+-!():^[]{}\"~*" );
   if(idx == 0) {
    buf[bp++] = ch;
    buf[bp++]=0;
    return true;
   }
   queryparser->throwParserException( L"Unrecognized escape sequence at %d %d::%d", ' ',
      reader->Column(),reader->Line());
   return false;
}


CL_NS_END2
