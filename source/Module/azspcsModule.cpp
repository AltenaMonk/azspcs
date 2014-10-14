#include "azspcsModule.h"

#include <csignal>
#include <cstring>

#include <Library/BaseSocket.h>
#include <Library/File.h>
#include <Library/LogManager.h>
#ifdef UNITTEST
#include <Library/FileSystemManager.h>
#endif//UNITTEST
#include <Library/TimeManager.h>
#include <Library/ModuleCreator.h>
#include <Library/TypeToKey.h>

#include "Version.h"
#include "Field.h"

namespace azspcs
{

void sigHandler (int)
{
    static int i(0);
    LOG_PROG_INFO << "sigHandler = " << ++i;
    if (i == 1)
    {
        GetModule()->KillThreads(4000);  /// @internal Задержка в ms между установкой флага на выход и срубанием потоков
    }
    else
    {
        LOG_PROG_INFO << "sigHandler blocked!";
    }
}

azspcsModule::azspcsModule()
    : Library::Module(Version::applicationName, Version::applicationVersion, Version::applicationDate)
{
}

azspcsModule::~azspcsModule()
{
}

void azspcsModule::InitializeFactories()
{
    InitFactory<Library::BaseSocket>();
}

typedef boost::shared_ptr<Field> TField;
typedef std::list<TField> TPopulation;

unsigned long Random(unsigned long x)
{
    return static_cast<unsigned long>(rand()%static_cast<long long>(x));
}

void MutationChange(TField field)
{
    unsigned long count(field->GetSize() + Random(field->GetSize()));
    for (unsigned long z(0); z < count; ++z)
    {
        unsigned int x1(Random(field->GetSize()));
        unsigned int y1(Random(field->GetSize()));
        unsigned int x2(Random(field->GetSize()));
        unsigned int y2(Random(field->GetSize()));
        long long value(field->Get(x1, y1));
        field->Set(x1, y1, field->Get(x2, y2));
        field->Set(x2, y2, value);
    }
}

void Mutation(TPopulation & population, unsigned long populationSize)
{
    while (population.size() < populationSize)
    {
        unsigned long size(population.size());
        for (unsigned long index(0); index < size; ++index)
        {
            TPopulation::iterator iterator(population.begin());
            std::advance(iterator, index);
            population.push_back(*iterator);
            MutationChange(population.back());
        }
    }
}

void Genetic(long long mutationStep, unsigned long populationSize, TField & min, TField & max)
{
    typedef std::vector<long long> TResults;
    TPopulation populationMin;
    populationMin.push_back(min);
    TPopulation populationMax;
    populationMax.push_back(max);

    for (long long step(0); step < mutationStep; ++step)
    {
        Mutation(populationMin, populationSize);
        Mutation(populationMax, populationSize);

        TResults minResult;
        TResults maxResult;
        for (unsigned long index(0); index < populationMin.size(); ++index)
        {
            TPopulation::iterator iterator(populationMin.begin());
            std::advance(iterator, index);
            minResult.push_back((*iterator)->GetValue());
        }
        for (unsigned long index(0); index < populationMax.size(); ++index)
        {
            TPopulation::iterator iterator(populationMax.begin());
            std::advance(iterator, index);
            maxResult.push_back((*iterator)->GetValue());
        }

        if (populationMin.size() > populationSize)
        {
            long long middle(0);
            for (unsigned long index(0); index < populationMin.size(); ++index)
            {
                middle += minResult[index];
            }
            middle /= populationMin.size();
            for (unsigned long index(populationMin.size()); index != 0; --index)
            {
                if (minResult[index] > middle)
                {
                    TPopulation::iterator iterator(populationMin.begin());
                    std::advance(iterator, index - 1);
                    populationMin.erase(iterator);
                }
            }
        }
        if (populationMax.size() > populationSize)
        {
            long long middle(0);
            for (unsigned long index(0); index < populationMax.size(); ++index)
            {
                middle += maxResult[index];
            }
            middle /= populationMax.size();
            for (unsigned long index(populationMax.size()); index != 0; --index)
            {
                if (maxResult[index] < middle)
                {
                    TPopulation::iterator iterator(populationMax.begin());
                    std::advance(iterator, index - 1);
                    populationMax.erase(iterator);
                }
            }
        }
    }
    long long maxIndex=0;
    long long minIndex=0;
    long long maxValue=0;
    long long minValue=0x7FFFFFFF;
    for (unsigned long index(0); index < populationMin.size(); ++index)
    {
        TPopulation::iterator iterator(populationMin.begin());
        std::advance(iterator, index);
        if ((*iterator)->GetValue()  < minValue)
        {
            minIndex = index;
        }
    }
    for (unsigned long index(0); index < populationMax.size(); ++index)
    {
        TPopulation::iterator iterator(populationMax.begin());
        std::advance(iterator, index);
        if ((*iterator)->GetValue()  > maxValue)
        {
            maxIndex = index;
        }
    }
    {
        TPopulation::iterator iterator(populationMin.begin());
        std::advance(iterator, minIndex);
        min = *iterator;
    }
    {
        TPopulation::iterator iterator(populationMax.begin());
        std::advance(iterator, maxIndex);
        max = *iterator;
    }
}

void Compare(TPopulation & best, TPopulation const & item)
{
    int size(3);
    int count(0);
    TPopulation::const_iterator iter(item.begin());
    for (TPopulation::iterator iterator(best.begin()); iterator != best.end(); ++iterator, ++iter)
    {
        long long bestValue = (*iterator)->GetValue();
        long long itemValue = (*iter)->GetValue();

        if (count == 0)
        {
            if (itemValue < bestValue)
            {
                *iterator = *iter;
            }
        }
        if (count == 1)
        {
            if (itemValue > bestValue)
            {
                *iterator = *iter;
            }
        }
        ++count;
        if (count == 2)
        {
            count = 0;
            ++size;
        }
        if (size == 28)
        {
            break;
        }
    }
}

void Load(Library::String const & filename, TPopulation & population)
{
    Library::String::TStrings load = Library::File(filename).Read().Split("\n");
    population.clear();

    int size(3);
    int count(0);
    for (Library::String::TStrings::const_iterator iterator(load.begin()); iterator != load.end(); ++iterator)
    {
        TField field(new Field());
        field->Load(*iterator, size);
        population.push_back(field);
        ++count;
        if (count == 2)
        {
            count = 0;
            ++size;
        }
        if (size == 28)
        {
            break;
        }
    }
}

void Save(Library::String const & filename, TPopulation const & population, bool people)
{
    Library::String data;
    Library::String data2;
    long long result(0);
    long long result2(0);
    long long signum = -1;
    long long counter(0);
    for (TPopulation::const_iterator iterator(population.begin()); iterator != population.end(); ++iterator)
    {
        if (people)
        {
            long long temp((*iterator)->GetValue());
            result += temp * signum;
            result2 += temp * signum;

            data += Library::SmartCast<Library::String>((*iterator)->GetSize(), "") + "\t " + Library::SmartCast<Library::String>(temp, "") + "\t: ";
            ++counter;
            if (counter == 2)
            {
                counter = 0;
                data2 += Library::SmartCast<Library::String>((*iterator)->GetSize(), "") + "\t: " + Library::SmartCast<Library::String>(result2, "") + "\n";
                result2 = 0;
            }

        }
        (*iterator)->Save(data);
        data += "\n";
        signum *= -1;
    }
    if (people)
    {
        data = Library::SmartCast<Library::String>(result, "") + "\n\n" + data2 + "\n\n" + data;
    }
    Library::File(filename, true).ReWrite(data);
}

void Save2(Library::String const & filename, TPopulation const & population)
{
    Library::String data;
    Library::String semicolonA("");
    for (TPopulation::const_iterator iterator(population.begin()); iterator != population.end(); ++iterator)
    {
        (*iterator)->Save2(data);
        data += semicolonA;
        data += "\n";
        data += "\n";
        data += "\n";
        semicolonA = ";";
    }
    Library::File(filename, true).ReWrite(data);
}

bool azspcsModule::ParseCommandLineParameters(TParameters const & parameters, bool IsManagersReady)
{
    /// @internal Без инициализированных логов проверяем только версию:
    if (IsManagersReady == false)
    {
        /// @internal Если любой из параметров равен "-v" или "--version" - выводим версию и выходим
        for (size_t i(1); i < parameters.size(); ++i)
        {
            if ((Library::String(parameters[i]) == "-v") || (Library::String(parameters[i]) == "--version"))
            {
                Library::Module * module(Library::GetModule());
                std::cout << module->GetApplicationName() << " " << module->GetApplicationVersion() << " " << module->GetApplicationDate() << " " << std::endl;
                return false;
            }
        }
        return true;
    }

    LOG_PROG_INFO <<  "Command line parameters:";
    for (size_t i(0); i < parameters.size(); ++i)
    {
        LOG_PROG_INFO << i << " - " << parameters[i];
    }

    srand(time(NULL));

    for (long long counter(0); counter < 1000; ++counter)
    {
        LOG_ADMIN_INFO << "Counter: " << counter;
        TPopulation population;
        for (long long x(3); x < 28; ++x)
        {
            LOG_ADMIN_INFO << "index: " << x;
            TField min(new Field()), max(new Field());
            min->Initialize(x);
            max->Initialize(x);
            Genetic(3+x*100, 3+x*50, min, max);
            long long minValue(min->GetValue());
            long long maxValue(max->GetValue());
            if (minValue > maxValue)
            {
                std::swap(min, max);
                std::swap(minValue, maxValue);
            }
            population.push_back(min);
            population.push_back(max);
        }

        Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
        Save("../result/" + time + "out", population, false);
        Save("../result/" + time + "out.people", population, true);
        Save2("../result/" + time + "azspcs", population);
        TPopulation best;

        Load("../result/best", best);
        Compare(best, population);
        Save("../result/best", best, false);

        Save("../result/" + time + "best.out", best, false);
        Save("../result/" + time + "best.out.people", best, true);
        Save2("../result/" + time + "best.azspcs", best);
    }

    return true;
}

bool azspcsModule::InitializeManagers(Library::BaseModuleManager::TParameters const & parameters)
{
    return Library::Module::InitializeManagers(parameters) &&
            InitLogManager();
}

template <typename TType>
void azspcsModule::InitFactory()
{
    typedef Library::Factory<Library::String, TType, true, boost::shared_ptr<TType> > TFactory;
    AddFactory(Library::TypeToKey<TFactory>::Value(), boost::shared_ptr<Library::BaseFactory>(new TFactory()));
}

template <typename TType>
bool azspcsModule::InitManager(Library::BaseModuleManager::TParameters const & parameters)
{
    boost::shared_ptr<Library::BaseModuleManager> manager(new TType());
    bool result(manager->Initialize(parameters));
    SetManager(Library::TypeToKey<TType>::Value(), manager);
    return result;
}

void azspcsModule::InitializeThreads(Library::BaseModuleManager::TParameters const & )
{
}

bool azspcsModule::InitLogManager()
{
    if (IsManager<Library::LogManager>() == false)
    {
        return false;
    }

    GetModule()->GetManager<Library::LogManager>()->AddLog("./logs/programmer")
        .SetLogMessageImportanceLevel(Library::LogMessageImportance_Information)
        .SetLogMessageViewers(Library::LogMessageViewer_All)
        .SetStringEnd("\n")
        .SetDateTimeFormat("YYYY/MM/DD hh:mm:ss,lll ")
        .SetLogMessageImportanceEnabled(true)
        .SetDailySeparate(true)
        .SetPrettyFunction(true)
        .SetThreadId(true)
        .SetProcessId(true);
        ;

    LOG_ADMIN_INFO << "";
    LOG_ADMIN_INFO << "";
    LOG_ADMIN_INFO << "";
    LOG_ADMIN_INFO <<  "--------- Start application ---------";
    LOG_ADMIN_INFO << GetApplicationName() << " " << GetApplicationVersion() << " " << GetApplicationDate() << " ";
    LOG_ADMIN_INFO << "";
    return true;
}

void azspcsModule::OnRootPermissions() const
{
    /// @internal Ругаемся на то, что запустили с правами root'а.
    std::cerr << "Application run with root privileges." << std::endl;
    /// @internal Выходим.
    exit(0);
}

azspcsModule * GetModule()
{
    /// @internal Возвращаем проинициализированный объект.
    return Library::ModuleCreator<azspcsModule>::GetModule(true);
}

} // namespace azspcs

namespace Library
{

Module * GetModule()
{
    return azspcs::GetModule();
}

} // namespace Library
