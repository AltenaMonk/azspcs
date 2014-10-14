#ifndef AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD
#define AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD

#include <boost/scoped_array.hpp>

#include <Library/String.h>

namespace azspcs
{

class Field
{
public:
    Field();
    ~Field();

    void Initialize(unsigned int size);

    void Load(Library::String const & data, unsigned int size);
    void Save(Library::String & data) const;
    void Save2(Library::String & data) const;

    long long GetValue() const;
    unsigned int GetSize() const;

    long long Get(unsigned int x, unsigned int y) const;
    void Set(unsigned int x, unsigned int y, long long value);

private:
    long long NOD(long long a, long long b) const;
    long long Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) const;
    long long Function() const;

    void RandomFill();

    unsigned int m_size;
    boost::scoped_array<long long> m_data;

    mutable long long m_value;
    mutable bool m_isValue;
};

} // namespace azspcs

#endif//AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD
