#ifndef POPULATION_H
#define POPULATION_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "Field.h"

namespace azspcs
{

template <typename TCompare>
class Population
{
public:
    typedef void (*MutationFunctor)(TField);

    Population();
    ~Population();

    void Initialize(unsigned int fieldSize, unsigned int populationSize);
    void AddItem(TField field);

    TField Genetic(MutationFunctor mutationFunctor, unsigned int populationSize, unsigned int mutationStep);

private:
    typedef std::vector<TField> TFields;

    unsigned int Size() const;

    TFields m_fields;
};

template <typename TCompare>
boost::shared_ptr<Population<TCompare> > CreatePopulation(TCompare compare);

} // namespace azspcs

#include "Population.inl"

#endif // POPULATION_H
