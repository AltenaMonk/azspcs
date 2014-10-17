#include "Result.h"

#include <Library/File.h>
#include <Library/SmartCast.h>

namespace azspcs
{

Result::Result()
{
}

Result::~Result()
{
}

void Result::AddItem(TField field)
{
    TFields::iterator iterator(m_fields.find(field->GetSize()));
    if (iterator == m_fields.end())
    {
        m_fields.insert(std::make_pair(field->GetSize(), std::make_pair(field, field)));
    }
    else
    {
        if (field->GetValue() < iterator->second.first->GetValue())
        {
            iterator->second.first = field;
        }
        if (field->GetValue() > iterator->second.second->GetValue())
        {
            iterator->second.second = field;
        }
    }
}

void Result::Merge(Result const & other)
{
    TFields::const_iterator iterator(other.m_fields.begin());
    TFields::const_iterator end     (other.m_fields.end  ());

    for (; iterator != end; ++iterator)
    {
        AddItem(iterator->second.first);
        AddItem(iterator->second.second);
    }
}

void Result::Load(Library::String const & filename)
{
    m_fields.clear();

    Library::String::TStrings load(Library::File(filename).Read().Split("\n"));

    unsigned int count(0);
    for (Library::String::TStrings::const_iterator iterator(load.begin()); iterator != load.end(); ++iterator)
    {
        TField field(new Field());
        field->Load(*iterator, 3 + count/2);
        AddItem(field);
        ++count;
        if (count == 50)
        {
            break;
        }
    }
}

void Result::Save(Library::String const & filename, Library::String const & additionalInformation, bool isPeopleView) const
{
    Library::String data;
    Library::String send;
    Library::String people;
    Library::String peopleBefore;

    Library::String semicolon(";");

    long long sum(0.);

    for (TFields::const_iterator iterator(m_fields.begin()); iterator != m_fields.end(); ++iterator)
    {
        if (--m_fields.end() == iterator)
        {
            semicolon = "";
        }
        if (isPeopleView)
        {

            long long diff(iterator->second.second->GetValue() - iterator->second.first->GetValue());
            sum += diff;
            peopleBefore += Library::SmartCast<Library::String>(iterator->first) + "\t:" + Library::SmartCast<Library::String>(diff) + "\n";
        }
        data += iterator->second.first->Save() + "\n" + iterator->second.second->Save() + "\n";
        people += Library::SmartCast<Library::String>(iterator->first) + "\t" + Library::SmartCast<Library::String>(iterator->second.first->GetValue()) + "\t:" + iterator->second.first->Save() + "\n";
        people += Library::SmartCast<Library::String>(iterator->first) + "\t" + Library::SmartCast<Library::String>(iterator->second.second->GetValue()) + "\t:" + iterator->second.second->Save() + "\n";
        send += iterator->second.first->Save2() + ";\n" + iterator->second.second->Save2() + semicolon + "\n";
    }
    if (isPeopleView)
    {
        people = additionalInformation + Library::SmartCast<Library::String>(sum) + "\n\n" + peopleBefore + "\n\n" + people;
    }
    Library::File(filename + ".out", true).ReWrite(data);
    if (isPeopleView)
    {
        Library::File(filename + ".people", true).ReWrite(people);
        Library::File(filename + ".azspcs", true).ReWrite(send);
    }
}

} // namespace azspcs
