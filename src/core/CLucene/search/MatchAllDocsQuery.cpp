/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team

* Updated by https://github.com/farfella/.
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Query.h"
#include "MatchAllDocsQuery.h"
#include "Explanation.h"
#include "SearchHeader.h"
#include "Searchable.h"

#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_DEF(search)

class MatchAllDocsQuery::MatchAllDocsWeight : public Weight {
private:
    Similarity* similarity;
    float_t queryWeight;
    float_t queryNorm;
    MatchAllDocsQuery* parentQuery;

public:
    MatchAllDocsWeight(MatchAllDocsQuery* enclosingInstance, Searcher* searcher);
    virtual ~MatchAllDocsWeight(){}

    virtual std::wstring toString();

    Query* getQuery();

    float_t getValue();

    float_t sumOfSquaredWeights();

    void normalize(float_t _queryNorm);

    Scorer* scorer(CL_NS(index)::IndexReader* reader);

    Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc);
};

class MatchAllDocsQuery::MatchAllScorer : public Scorer {
    CL_NS(index)::IndexReader* reader;
    int32_t id;
    int32_t maxId;
    float_t _score;

public:
    MatchAllScorer(CL_NS(index)::IndexReader* _reader, Similarity* similarity, Weight* w);
    virtual ~MatchAllScorer(){}

    Explanation* explain(int32_t doc);

    int32_t doc() const;

    bool next();

    float_t score();

    bool skipTo(int32_t target);

    virtual std::wstring toString();
};

MatchAllDocsQuery::MatchAllScorer::MatchAllScorer(CL_NS(index)::IndexReader* _reader, Similarity* similarity, Weight* w)
			:Scorer(similarity),reader(_reader),id(-1)
{
	maxId = reader->maxDoc() - 1;
	_score = w->getValue();
}

Explanation* MatchAllDocsQuery::MatchAllScorer::explain(int32_t doc) {
	// not called... see MatchAllDocsWeight::explain()
	return NULL;
}

int32_t MatchAllDocsQuery::MatchAllScorer::doc() const {
	return id;
}

bool MatchAllDocsQuery::MatchAllScorer::next() {
	while (id < maxId) {
		id++;
		if (!reader->isDeleted(id)) {
			return true;
		}
	}
	return false;
}

float_t MatchAllDocsQuery::MatchAllScorer::score() {
	return _score;
}

bool MatchAllDocsQuery::MatchAllScorer::skipTo(int32_t target) {
	id = target - 1;
	return next();
}

std::wstring MatchAllDocsQuery::MatchAllScorer::toString(){
	return L"MatchAllScorer";
}

MatchAllDocsQuery::MatchAllDocsWeight::MatchAllDocsWeight(MatchAllDocsQuery* enclosingInstance, Searcher* searcher):
		parentQuery(enclosingInstance){
	this->similarity = searcher->getSimilarity();
}

std::wstring MatchAllDocsQuery::MatchAllDocsWeight::toString() {
	std::wstring buf = L"weight(";

	std::wstring t = parentQuery->toString();
	buf.append(t);

	buf.push_back( L')');
	return buf;
}

Query* MatchAllDocsQuery::MatchAllDocsWeight::getQuery() {
	return parentQuery;
}

float_t MatchAllDocsQuery::MatchAllDocsWeight::getValue() {
	return queryWeight;
}

float_t MatchAllDocsQuery::MatchAllDocsWeight::sumOfSquaredWeights() {
	queryWeight = parentQuery->getBoost();
	return queryWeight * queryWeight;
}

void MatchAllDocsQuery::MatchAllDocsWeight::normalize(float_t _queryNorm) {
	this->queryNorm = _queryNorm;
	queryWeight *= this->queryNorm;
}

Scorer* MatchAllDocsQuery::MatchAllDocsWeight::scorer(CL_NS(index)::IndexReader* reader) {
	return _CLNEW MatchAllScorer(reader, similarity, this);
}

Explanation* MatchAllDocsQuery::MatchAllDocsWeight::explain(CL_NS(index)::IndexReader* reader, int32_t doc) {
	// explain query weight
	Explanation* queryExpl = _CLNEW ComplexExplanation(true, getValue(), L"MatchAllDocsQuery, product of:");
	if (parentQuery->getBoost() != 1.0f) {
		queryExpl->addDetail(_CLNEW Explanation(parentQuery->getBoost(), L"boost"));
	}
	queryExpl->addDetail(_CLNEW Explanation(queryNorm, L"queryNorm"));
	return queryExpl;
}

MatchAllDocsQuery::MatchAllDocsQuery(){}
MatchAllDocsQuery::~MatchAllDocsQuery(){}

Weight* MatchAllDocsQuery::_createWeight(Searcher* searcher){
	return _CLNEW MatchAllDocsWeight(this, searcher);
}

const std::wstring MatchAllDocsQuery::getClassName() {
	return L"MatchAllDocsQuery";
}
const std::wstring MatchAllDocsQuery::getObjectName() const{
	return getClassName();
}

std::wstring MatchAllDocsQuery::toString(const wchar_t* /*field*/) const{
	std::wstring buffer = L"MatchAllDocsQuery";
    buffer.append(boost_to_wstring(getBoost()));
    return buffer;
}

MatchAllDocsQuery::MatchAllDocsQuery(const MatchAllDocsQuery& clone):
  Query(clone)
{
}

Query* MatchAllDocsQuery::clone() const{
	return _CLNEW MatchAllDocsQuery(*this);
}

void MatchAllDocsQuery::extractTerms( TermSet * termset ) const
{
}

bool MatchAllDocsQuery::equals(Query* o) const{
	if (!(o->instanceOf(MatchAllDocsQuery::getClassName())))
		return false;
	MatchAllDocsQuery* other = static_cast<MatchAllDocsQuery*>(o);
	return this->getBoost() == other->getBoost();
}

size_t MatchAllDocsQuery::hashCode() const{
	return (static_cast<size_t>(getBoost())) ^ 0x1AA71190;
}

CL_NS_END
