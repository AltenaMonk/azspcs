#ifndef AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD
#define AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <Library/String.h>

namespace azspcs
{

class Field
{
public:
    typedef boost::shared_ptr<Field> TField;

    Field();
    ~Field();

    TField Clone() const;

    static void InitializeClass(unsigned int maxSize);
    static void ReleaseClass(unsigned int maxSize);

    void Initialize(unsigned int size);

    void Load(Library::String const & data, unsigned int size);
    Library::String Save() const;
    Library::String Save2() const;

    long long GetValue() const;
    unsigned int GetSize() const;

    long long Get(unsigned int x, unsigned int y) const;
    void Set(unsigned int x, unsigned int y, long long value);

    bool operator>(Field const & other) const;
    bool operator<(Field const & other) const;

private:
    static long long NOD(long long a, long long b);
    static long long Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    inline long long Function() const;

    void RandomFill();

    unsigned int m_size;
    boost::scoped_array<long long> m_data;

    mutable long long m_value;
    mutable bool m_isValue;

    static long long ** m_nod;
};

typedef Field::TField TField;

bool operator>(TField const & first, TField const & second);
bool operator<(TField const & first, TField const & second);

} // namespace azspcs

#endif//AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD
