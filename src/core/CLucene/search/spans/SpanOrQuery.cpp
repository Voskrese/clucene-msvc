/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team

* Updated by https://github.com/farfella/.
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/PriorityQueue.h"

#include "SpanOrQuery.h"
#include "_EmptySpans.h"


CL_NS_DEF2( search, spans )


/////////////////////////////////////////////////////////////////////////////
class SpanOrQuery::SpanQueue : public CL_NS(util)::PriorityQueue<Spans*, CL_NS(util)::Deletor::Object<Spans> >
{
public:
    SpanQueue( int32_t size ) { initialize( size, true ); }
    virtual ~SpanQueue() {}

protected:
    bool lessThan(Spans* spans1, Spans* spans2 )
    {
        if( spans1->doc() == spans2->doc() )
        {
            if( spans1->start() == spans2->start())
                return spans1->end() < spans2->end();
            else
                return spans1->start() < spans2->start();
        }
        else
            return spans1->doc() < spans2->doc();
    }
};


/////////////////////////////////////////////////////////////////////////////
class SpanOrQuery::SpanOrQuerySpans : public Spans
{
private:
    SpanQueue *                 queue;
    SpanOrQuery *               parentQuery;
    CL_NS(index)::IndexReader * reader;

public:
    SpanOrQuerySpans( SpanOrQuery * parentQuery, CL_NS(index)::IndexReader * reader );
    virtual ~SpanOrQuerySpans();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const     { return top()->doc(); }
    int32_t start() const   { return top()->start(); }
    int32_t end() const     { return top()->end(); }

    std::wstring toString() const;

private:
    Spans * top() const     { return queue->top(); }
    bool initSpanQueue( int32_t target );
};


SpanOrQuery::SpanOrQuerySpans::SpanOrQuerySpans( SpanOrQuery * parentQuery, CL_NS(index)::IndexReader * reader )
{
    this->parentQuery = parentQuery;
    this->reader = reader;
    this->queue = NULL;
}

SpanOrQuery::SpanOrQuerySpans::~SpanOrQuerySpans()
{
    _CLLDELETE( queue );
}

bool SpanOrQuery::SpanOrQuerySpans::next()
{
    if( ! queue )
        return initSpanQueue( -1 );

    if( queue->size() == 0 ) // all done
        return false;

    if( top()->next() )     // move to next
    {
        queue->adjustTop();
        return true;
    }

    _CLLDELETE( queue->pop() );  // exhausted a clause
    return queue->size() != 0;
}

bool SpanOrQuery::SpanOrQuerySpans::skipTo( int32_t target )
{
    if( ! queue )
        return initSpanQueue( target );

    while( queue->size() != 0 && top()->doc() < target )
    {
        if( top()->skipTo( target ))
            queue->adjustTop();
        else
            _CLLDELETE( queue->pop() );
    }

    return queue->size() != 0;
}

std::wstring SpanOrQuery::SpanOrQuerySpans::toString() const
{
    std::wstring buffer;
    std::wstring tszQry = parentQuery->toString();

    buffer.append( L"spans(" );
    buffer.append( tszQry );
    buffer.append( L")" );

    return buffer;
}

bool SpanOrQuery::SpanOrQuerySpans::initSpanQueue( int32_t target )
{
    queue = _CLNEW SpanQueue( parentQuery->clausesCount );

    for( size_t i = 0; i < parentQuery->clausesCount; i++ )
    {
        Spans * spans = parentQuery->clauses[ i ]->getSpans( reader );
        if(( target == -1 && spans->next()) || ( target != -1 && spans->skipTo( target )))
            queue->put( spans );
        else
            _CLLDELETE( spans );
    }
    return ( queue->size() != 0 );
}


/////////////////////////////////////////////////////////////////////////////
SpanOrQuery::SpanOrQuery( const SpanOrQuery& clone ) :
    SpanQuery( clone )
{
    this->clauses = _CL_NEWARRAY( SpanQuery *, clone.clausesCount );
    for( size_t i = 0; i < clone.clausesCount; i++ )
        this->clauses[ i ] = (SpanQuery *) clone.clauses[ i ]->clone();

    this->clausesCount = clone.clausesCount;
    this->bDeleteClauses = true;

    setField( clone.field.c_str() );
}

SpanOrQuery::~SpanOrQuery()
{
    if( bDeleteClauses )
    {
        for( size_t i = 0; i < clausesCount; i++ )
            _CLLDELETE( clauses[ i ] );
    }

    clausesCount = 0;
    _CLDELETE_LARRAY( clauses );
}

CL_NS(search)::Query * SpanOrQuery::clone() const
{
    return _CLNEW SpanOrQuery( *this );
}

const std::wstring SpanOrQuery::getClassName()
{
	return L"SpanOrQuery";
}

const std::wstring SpanOrQuery::getObjectName() const
{
	return getClassName();
}

SpanQuery ** SpanOrQuery::getClauses() const
{
    return clauses;
}

size_t SpanOrQuery::getClausesCount() const
{
    return clausesCount;
}

void SpanOrQuery::setField( const wchar_t * field_ )
{
    this->field = field_;
}

const wchar_t * SpanOrQuery::getField() const
{
    return field.c_str();
}

void SpanOrQuery::extractTerms( CL_NS(search)::TermSet * terms ) const
{
    for( size_t i = 0; i < clausesCount; i++ )
        clauses[ i ]->extractTerms( terms );
}

CL_NS(search)::Query * SpanOrQuery::rewrite( CL_NS(index)::IndexReader * reader )
{
    SpanOrQuery * clone = NULL;
    for( size_t i = 0; i < clausesCount; i++ )
    {
        SpanQuery * c = clauses[ i ];
        SpanQuery * query = (SpanQuery *) c->rewrite( reader );
        if( query != c )
        {                     // clause rewrote: must clone
            if( clone == NULL )
                clone = (SpanOrQuery *) this->clone();

            _CLLDELETE( clone->clauses[ i ] );
            clone->clauses[ i ] = query;
        }
    }
    if( clone )
        return clone;                        // some clauses rewrote
    else
        return this;                         // no clauses rewrote
}

std::wstring SpanOrQuery::toString( const wchar_t* field ) const
{
    std::wstring buffer = L"spanOr([";
    for( size_t i = 0; i < clausesCount; i++ )
    {
        if( i != 0 )
            buffer.append( L", ");

        std::wstring tszClause = clauses[ i ]->toString( field );
        buffer.append( tszClause );
    }

    buffer.append( L"])" );
    buffer.append( boost_to_wstring(getBoost()) );

    return buffer;
}

bool SpanOrQuery::equals( Query* other ) const
{
    if( this == other ) return true;
    if( other == NULL || !( other->instanceOf( SpanOrQuery::getClassName() )))
	    return false;

	SpanOrQuery * that = (SpanOrQuery *) other;
    if( 0 != wcscmp( field.c_str(), that->field.c_str() )
        || getBoost() != that->getBoost())
    {
        return false;
    }

    if( clausesCount != that->clausesCount )
        return false;
    for( size_t i = 0; i < clausesCount; i++ )
    {
        if( ! clauses[ i ]->equals( that->clauses[ i ] ))
            return false;
    }

    return true;
}

size_t SpanOrQuery::hashCode() const
{
	size_t h = 1;
    for( size_t i = 0; i < clausesCount; i++ )
	    h = 31*h + clauses[ i ]->hashCode();

    h ^= (h << 10) | (h >> 23);
    h ^= Similarity::floatToByte( getBoost() );

    return h;
}

Spans * SpanOrQuery::getSpans( CL_NS(index)::IndexReader * reader )
{
    if( clausesCount == 0 )
        return _CLNEW EmptySpans();              // CLucene: 0-clause case

    if( clausesCount == 1 )                      // optimize 1-clause case
        return clauses[ 0 ]->getSpans( reader );

    return _CLNEW SpanOrQuerySpans( this, reader );
}

CL_NS_END2
