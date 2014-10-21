#include "Field.h"

#include <cstring>

#include <algorithm>
#include <utility>
#include <vector>

#include <Library/SmartCast.h>

namespace azspcs
{

long long ** Field::m_nod = NULL;

Field::Field()
    : m_size(0)
    , m_data()
    , m_value(0)
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
    field->m_data.reset(new long long[m_size * m_size]);
    memcpy(field->m_data.get(), m_data.get(), m_size * m_size * sizeof(m_data[0]));
    field->m_value = m_value;
    field->m_isValue = m_isValue;
    return field;
}

void Field::InitializeClass(unsigned int maxSize)
{
    m_nod = new long long * [maxSize*maxSize + 1];
    for (int y(1); y <= maxSize*maxSize; ++y)
    {
        m_nod[y] = new long long [maxSize*maxSize + 1];
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
    m_data.reset(new long long[size * size]());
    RandomFill();
    m_value = 0;
    m_isValue = false;
}

void Field::Load(Library::String const & data, unsigned int size)
{
    m_size = size;
    m_data.reset(new long long[size * size]());

    Library::String::TStrings strings = data.Split(" ");
    for (int x(0); x < GetSize(); ++x)
    {
        for (int y(0); y < GetSize(); ++y)
        {
            Set(x, y, Library::SmartCast<long long>(strings[x*size+y]));
        }
    }

    m_value = 0;
    m_isValue = false;
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
    m_value = Function();
    m_isValue = true;
    return m_value;
}

unsigned int Field::GetSize() const
{
    return m_size;
}

long long Field::Get(unsigned int x, unsigned int y) const
{
    return m_data[y*GetSize() + x];
}

void Field::Set(unsigned int x, unsigned int y, long long value)
{
    m_isValue = false;
    m_data[y*GetSize() + x] = value;
}

bool Field::operator>(Field const & other) const
{
    return GetValue() > other.GetValue();
}

bool Field::operator<(Field const & other) const
{
    return GetValue() < other.GetValue();
}

long long Field::NOD(long long a, long long b)
{
    if (a == b)
    {
        return a;
    }
    return NOD(std::max(a, b) - std::min(a, b), std::min(a, b));
}

long long Field::Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    unsigned int xMin(std::min(x1, x2));
    unsigned int xMax(std::max(x1, x2));
    unsigned int yMin(std::min(y1, y2));
    unsigned int yMax(std::max(y1, y2));
    return (xMax-xMin)*(xMax-xMin)+(yMax-yMin)*(yMax-yMin);
}

inline long long Field::Function() const
{
    long long result(0);
    unsigned int size(GetSize()*GetSize());
    for (unsigned int a(0); a < size; ++a)
    {
        unsigned int x1 = a / GetSize();
        unsigned int y1 = a % GetSize();
        long long * values(m_nod[Get(x1, y1)]);
        for (unsigned int b(a+1); b < size; ++b)
        {
            unsigned int x2 = b / GetSize();
            unsigned int y2 = b % GetSize();
            result += values[Get(x2, y2)] * Distance(x1, y1, x2, y2);
        }
    }
    return result;
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

bool operator>(TField const & first, TField const & second)
{
    return *first > *second;
}

bool operator<(TField const & first, TField const & second)
{
    return *first < *second;
}


} // namespace azspcs
