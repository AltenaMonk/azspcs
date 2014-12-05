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
    , m_type(2)
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

bool Field::Equal(TField other) const
{
    if (other->m_size != m_size)
    {
        return false;
    }
    return memcmp(other->m_data.get(), m_data.get(), m_size * m_size * sizeof(TType)) == 0;
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
    SetType(Library::SmartCast<int>(strings[0]));
    for (int x(0); x < GetSize(); ++x)
    {
        for (int y(0); y < GetSize(); ++y)
        {
            Set(x, y, Library::SmartCast<TType>(strings[x * size + y + 1]));
        }
    }
}

Library::String Field::Save() const
{
    Library::String data("");
    data += Library::SmartCast<Library::String>(GetType()) + " ";
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

int Field::GetValue() const
{
    if (m_isValue)
    {
        return m_value;
    }

    PreValue();

    unsigned int size(GetSize()*GetSize());
    int result(0);
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

void Field::Rotate(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3)
{
    /// Меняем местами указанные значения.
    TType first (Get(x1, y1));
    TType second(Get(x2, y2));
    TType third (Get(x3, y3));
    TType temp(first);
    first  = third ;
    third  = second;
    second = temp;
    Set(x1, y1, first );
    Set(x2, y2, second);
    Set(x3, y3, third );
}

void Field::Rotate(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, unsigned int x4, unsigned int y4, int type, bool forward)
{
    /// Меняем местами указанные значения.
    TType first (Get(x1, y1));
    TType second(Get(x2, y2));
    TType third (Get(x3, y3));
    TType forth (Get(x4, y4));

    if (forward == false)
    {
        static int revert[] = {0, 6, 3, 2, 4, 7, 1, 5, 8};
        type = revert[type];
    }

    switch (type)
    {
        case 0:
        {
            std::swap(first , second);
            std::swap(third , forth );
        }
        break;
        case 1:
        {
            TType temp(first);
            first  = second;
            second = third ;
            third  = forth ;
            forth  = temp  ;
        }
        break;
        case 2:
        {
            TType temp(first);
            first  = second;
            second = forth ;
            forth  = third ;
            third  = temp  ;
        }
        break;
        case 3:
        {
            TType temp(first);

            first  = third ;
            third  = forth ;
            forth  = second;
            second = temp  ;
        }
        break;
        case 4:
        {
            std::swap(first , third);
            std::swap(second, forth );

        }
        break;
        case 5:
        {
            TType temp(first);

            first  = third ;
            third  = second;
            second = forth;
            forth  = temp  ;
        }
        break;
        case 6:
        {
            TType temp(first);

            first  = forth ;
            forth  = third ;
            third  = second;
            second = temp  ;
        }
        break;
        case 7:
        {
            TType temp(first);

            first  = forth ;
            forth  = second;
            second = third ;
            third  = temp  ;
        }
        break;
        case 8:
        {
            std::swap(first , forth );
            std::swap(second, third );
        }
        break;
    }

    Set(x1, y1, first );
    Set(x2, y2, second);
    Set(x3, y3, third );
    Set(x4, y4, forth );
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
        unsigned int a(y * GetSize() + x);
        {
            TType * values(m_nod[Get(x, y)]);
            for (unsigned int b(a+1); b < size; ++b)
            {
                unsigned int x2 = b % GetSize();
                unsigned int y2 = b / GetSize();
                m_rawData[a * size + b] = values[Get(x2, y2)] * Distance(x, y, x2, y2);
            }
        }
    }
    {
        unsigned int b(y * GetSize() + x);
        TType * values(m_nod[Get(x, y)]);
        for (unsigned int a(0); a < b; ++a)
        {
            unsigned int x1 = a % GetSize();
            unsigned int y1 = a / GetSize();
            {
                m_rawData[a*size + b] = values[Get(x1, y1)] * Distance(x1, y1, x, y);
            }
        }
    }
    m_dirty[y * GetSize() + x] = 0;
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
    for (int z(0); z < GetSize() * GetSize(); ++z)
    {
        Set(places[z].first, places[z].second, z + 1);
    }
}

} // namespace azspcs
