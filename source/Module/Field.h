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
    typedef unsigned int TType;

    Field();
    ~Field();

    TField Clone() const;
    bool Equal(TField other) const;

    static void InitializeClass(unsigned int maxSize);
    static void ReleaseClass(unsigned int maxSize);

    void Initialize(unsigned int size);

    void Load(Library::String const & data, unsigned int size);
    Library::String Save() const;
    Library::String Save2() const;

    int GetValue() const;
    inline unsigned int GetSize() const
    {
        return m_size;
    }

    inline Field::TType Get(unsigned int x, unsigned int y) const
    {
        return m_data[y*GetSize() + x];
    }

    inline void Set(unsigned int x, unsigned int y, TType value)
    {
        m_value = 0;
        m_isValue = false;
        m_isPreValue = false;
        m_dirty[y*GetSize() + x] = 1;
        m_data[y*GetSize() + x] = value;
    }

    void Swap(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void Rotate(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3);
    void Rotate(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, unsigned int x4, unsigned int y4, int type, bool forward);

    inline bool operator>(Field const & other) const
    {
        return GetValue() > other.GetValue();
    }

    inline bool operator<(Field const & other) const
    {
        return GetValue() < other.GetValue();
    }

    inline void SetType(int type)
    {
        m_type = std::max(type, 2);
    }

    inline int GetType() const
    {
        return m_type;
    }

private:
    static TType NOD(TType a, TType b);
    static TType Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void PreValue() const;
    void Recalc(unsigned int x, unsigned int y) const;

    void RandomFill();

    unsigned int m_size;
    boost::scoped_array<TType> m_data;
    boost::scoped_array<TType> m_rawData;
    boost::scoped_array<unsigned int> m_dirty;

    mutable int m_value;
    mutable bool m_isPreValue;
    mutable bool m_isValue;

    static TType ** m_nod;

    int m_type;
};

typedef Field::TField TField;

inline bool operator>(TField const & first, TField const & second)
{
    return *first > *second;
}

inline bool operator<(TField const & first, TField const & second)
{
    return *first < *second;
}

} // namespace azspcs

#endif//AZSPCS_MODULE_FIELD_H__INCLUDE_GUARD
