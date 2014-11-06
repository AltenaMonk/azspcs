#include "Field.h"

#include <cstring>

#include <algorithm>
#include <utility>
#include <vector>

#include <Library/SmartCast.h>

namespace azspcs
{

Field::TType ** Field::m_nod = NULL;

Field::Field()
    : m_size(0)
    , m_data()
    , m_rawData()
    , m_dirty()
    , m_value(0)
    , m_isPreValue(false)
    , m_isValue(false)
{
}

Field::~Field()
{
}

Field::TField Field::Clone() const
{
    TField field(new Field());
    field->m_size = m_size;
    field->m_data.reset(new TType[m_size * m_size]);
    memcpy(field->m_data.get(), m_data.get(), m_size * m_size * sizeof(m_data[0]));
    field->m_rawData.reset(new TType[m_size * m_size * m_size * m_size]);
    memcpy(field->m_rawData.get(), m_rawData.get(), m_size * m_size * m_size * m_size * sizeof(m_rawData[0]));
    field->m_dirty.reset(new unsigned int[m_size * m_size]);
    memcpy(field->m_dirty.get(), m_dirty.get(), m_size * m_size * sizeof(m_dirty[0]));
    field->m_value = m_value;
    field->m_isPreValue = m_isPreValue;
    field->m_isValue = m_isValue;
    return field;
}

void Field::InitializeClass(unsigned int maxSize)
{
    m_nod = new TType * [maxSize*maxSize + 1];
    for (int y(1); y <= maxSize*maxSize; ++y)
    {
        m_nod[y] = new TType [maxSize*maxSize + 1];
        for (int x(1); x <= maxSize*maxSize; ++x)
        {
            m_nod[y][x] = NOD(y, x);
        }
    }
}

void Field::ReleaseClass(unsigned int maxSize)
{
    for (int y(1); y <= maxSize; ++y)
    {
        delete [] m_nod[y];
    }
    delete [] m_nod;
}

void Field::Initialize(unsigned int size)
{
    m_size = size;
    m_data.reset(new TType[size * size]());
    m_rawData.reset(new TType[size * size * size * size]());
    m_dirty.reset(new unsigned int[size * size]());
    memset(m_dirty.get(), 1, size * size * sizeof(m_dirty[0]));
    RandomFill();
    m_value = 0;
    m_isPreValue = false;
    m_isValue = false;
}

void Field::Load(Library::String const & data, unsigned int size)
{
    m_size = size;
    m_data.reset(new TType[size * size]());
    m_rawData.reset(new TType[size * size * size * size]());
    m_dirty.reset(new unsigned int[size * size]());
    memset(m_dirty.get(), 1, size * size * sizeof(m_dirty[0]));

    Library::String::TStrings strings = data.Split(" ");
    for (int x(0); x < GetSize(); ++x)
    {
        for (int y(0); y < GetSize(); ++y)
        {
            Set(x, y, Library::SmartCast<TType>(strings[x*size+y]));
        }
    }
}

Library::String Field::Save() const
{
    Library::String data("");
    for (unsigned int x(0); x < GetSize(); ++x)
    {
        for (unsigned int y(0); y < GetSize(); ++y)
        {
            data += Library::SmartCast<Library::String>(Get(x, y)) + " ";
        }
    }
    return data;
}

Library::String Field::Save2() const
{
    Library::String data("");
    Library::String semicolonX("");
    for (unsigned int x(0); x < GetSize(); ++x)
    {
        data += semicolonX + "(";
        Library::String semicolonY("");
        for (unsigned int y(0); y < GetSize(); ++y)
        {
            data += semicolonY + Library::SmartCast<Library::String>(Get(x, y));
            semicolonY = ",";
        }
        data += ")";
        semicolonX = ",\n";
    }
    return data;
}

long long Field::GetValue() const
{
    if (m_isValue)
    {
        return m_value;
    }

    PreValue();

    unsigned int size(GetSize()*GetSize());
    long long result(0);
    for (unsigned int a(0); a < size; ++a)
    {
        for (unsigned int b(a+1); b < size; ++b)
        {
            result += m_rawData[a*size + b];
        }
    }

    m_isValue = true;
    m_value = result;

    return result;
}

void Field::Swap(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    /// Меняем местами указанные значения.
    TType first(Get(x1, y1));
    TType second(Get(x2, y2));
    std::swap(first, second);
    Set(x1, y1, first);
    Set(x2, y2, second);
}

Field::TType Field::NOD(TType a, TType b)
{
    if (a == b)
    {
        return a;
    }
    return NOD(std::max(a, b) - std::min(a, b), std::min(a, b));
}

Field::TType Field::Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    unsigned int xMin(std::min(x1, x2));
    unsigned int xMax(std::max(x1, x2));
    unsigned int yMin(std::min(y1, y2));
    unsigned int yMax(std::max(y1, y2));
    return (xMax-xMin)*(xMax-xMin)+(yMax-yMin)*(yMax-yMin);
}

void Field::PreValue() const
{
    if (m_isPreValue == false)
    {
        unsigned int size(GetSize() * GetSize());
        for (unsigned int a(0); a < size; ++a)
        {
            if (m_dirty[a] == 1)
            {
                Recalc(a % GetSize(), a / GetSize());
            }
        }
        m_isPreValue = true;
    }
}


void Field::Recalc(unsigned int x, unsigned int y) const
{
    unsigned int size(GetSize()*GetSize());
    {
        unsigned int a(x * GetSize() + y);
        {
            TType * values(m_nod[Get(x, y)]);
            for (unsigned int b(a+1); b < size; ++b)
            {
                unsigned int x2 = b / GetSize();
                unsigned int y2 = b % GetSize();
                m_rawData[a * size + b] = values[Get(x2, y2)] * Distance(x, y, x2, y2);
            }
        }
    }
    {
        unsigned int b(x * GetSize() + y);
        TType * values(m_nod[Get(x, y)]);
        for (unsigned int a(0); a < b; ++a)
        {
            unsigned int x1 = a / GetSize();
            unsigned int y1 = a % GetSize();
            {
                m_rawData[a*size + b] = values[Get(x1, y1)] * Distance(x1, y1, x, y);
            }
        }
    }
    m_dirty[x * GetSize() + y] = 0;
}

void Field::RandomFill()
{
    typedef std::vector<std::pair<unsigned int, unsigned int> > TPlaces;
    TPlaces places;
    places.reserve(GetSize() * GetSize());
    for (unsigned int x(0); x < GetSize(); ++x)
    {
        for (unsigned int y(0); y < GetSize(); ++y)
        {
            places.push_back(std::make_pair(x, y));
        }
    }
    std::random_shuffle(places.begin(), places.end());
    for (long long z(0); z < GetSize() * GetSize(); ++z)
    {
        Set(places[z].first, places[z].second, z + 1);
    }
}

} // namespace azspcs
