#ifndef POPULATION_INL
#define POPULATION_INL

namespace azspcs
{

template <typename TCompare>
Population<TCompare>::Population()
    : m_fields()
{
}

template <typename TCompare>
Population<TCompare>::~Population()
{
}

template <typename TCompare>
void Population<TCompare>::Initialize(unsigned int fieldSize, unsigned int populationSize)
{
    for (unsigned int index(0); index < populationSize; ++index)
    {
        TField field(new Field());
        field->Initialize(fieldSize);
        AddItem(field);
    }
}

template <typename TCompare>
void Population<TCompare>::AddItem(TField field)
{
    m_fields.push_back(field);
}

template <typename TCompare>
TField Population<TCompare>::Genetic(MutationFunctor mutationFunctor, unsigned int populationSize, unsigned int mutationStep)
{
    TCompare compareFunctor;
    for (unsigned int step(0); step < mutationStep; ++step)
    {
        unsigned int max(populationSize * 2);
        if (Size() < max)
        {
            unsigned int size(max - Size());
            for (unsigned int index(0); index < size; ++index)
            {
                TField field(m_fields[index]->Clone());
                m_fields.push_back(field);
                mutationFunctor(field);
            }
        }
        if (Size() > populationSize)
        {
            std::sort(m_fields.begin(), m_fields.end(), compareFunctor);
            m_fields.erase(m_fields.begin() + populationSize, m_fields.end());
        }
    }
    std::sort(m_fields.begin(), m_fields.end(), compareFunctor);
    return m_fields.front();
}

template <typename TCompare>
unsigned int Population<TCompare>::Size() const
{
    return m_fields.size();
}

template <typename TCompare>
boost::shared_ptr<Population<TCompare> > CreatePopulation(TCompare)
{
    return boost::shared_ptr<Population<TCompare> >(new Population<TCompare>());
}

} // namespace azspcs

#endif//POPULATION_INL
