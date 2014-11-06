#ifndef RESULT_H
#define RESULT_H

#include <map>

#include <boost/shared_ptr.hpp>

#include "Field.h"

namespace azspcs
{

class Result
{
public:
    typedef long long (*MutationFunctor)(TField);

    Result();
    ~Result();

    void AddItem(TField field);

    void Merge(Result const & other);

    void Load(Library::String const & filename);
    void Save(Library::String const & filename, Library::String const & additionalInformation = "", bool isPeopleView = false) const;

    void UpdateFirst(MutationFunctor functorMin, MutationFunctor functorMax, unsigned int tryCount);

private:
    typedef std::map<unsigned int, std::pair<TField, TField> > TFields;

    TFields m_fields;
};

typedef boost::shared_ptr<Result> TResult;

} // namespace azspcs

#endif // RESULT_H
