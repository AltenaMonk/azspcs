#include "Field.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <Library/SmartCast.h>

namespace azspcs
{

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
            Set(x, y, Library::SmartCast<long long>(strings[x*size+y], 0));
        }
    }

    m_value = 0;
    m_isValue = false;
}

void Field::Save(Library::String & data) const
{
    for (unsigned int x(0); x < GetSize(); ++x)
    {
        for (unsigned int y(0); y < GetSize(); ++y)
        {
            data += Library::SmartCast<Library::String>(Get(x, y), "") + " ";
        }
    }
}

void Field::Save2(Library::String & data) const
{
    Library::String semicolonX("");
    for (unsigned int x(0); x < GetSize(); ++x)
    {
        data += semicolonX + "(";
        Library::String semicolonY("");
        for (unsigned int y(0); y < GetSize(); ++y)
        {
            data += semicolonY + Library::SmartCast<Library::String>(Get(x, y), "");
            semicolonY = ",";
        }
        data += ")";
        semicolonX = ",\n";
    }
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

long long Field::NOD(long long a, long long b) const
{
    if (a == b)
    {
        return a;
    }
    return NOD(std::max(a, b) - std::min(a, b), std::min(a, b));
}

long long Field::Distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) const
{
    unsigned int xMin(std::min(x1, x2));
    unsigned int xMax(std::max(x1, x2));
    unsigned int yMin(std::min(y1, y2));
    unsigned int yMax(std::max(y1, y2));
    return (xMax-xMin)*(xMax-xMin)+(yMax-yMin)*(yMax-yMin);
}

long long Field::Function() const
{
    long long result(0);
    for (unsigned int x1(0); x1 < GetSize(); ++x1)
    {
        for (unsigned int y1(0); y1 < GetSize(); ++y1)
        {
            for (unsigned int x2(0); x2 < GetSize(); ++x2)
            {
                for (unsigned int y2(0); y2 < GetSize(); ++y2)
                {
                    result += NOD(Get(x1, y1), Get(x2, y2)) * Distance(x1, y1, x2, y2);
                }
            }
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

} // namespace azspcs
