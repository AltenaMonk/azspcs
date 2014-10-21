#include "azspcsModule.h"

#include <csignal>
#include <cstring>

#include <boost/bind.hpp>

#include <Library/BaseSocket.h>
#include <Library/LogManager.h>
#ifdef UNITTEST
#include <Library/FileSystemManager.h>
#endif//UNITTEST
#include <Library/TimeManager.h>
#include <Library/ModuleCreator.h>
#include <Library/TypeToKey.h>

#include "Version.h"
#include "Field.h"
#include "Population.h"
#include "Result.h"

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

long long Random(unsigned long x)
{
    return rand()%static_cast<long long>(x);
}

typedef void (*Mutation)(TField);

inline void MutationRandomChange(TField field)
{
    unsigned int x1(Random(field->GetSize()));
    unsigned int y1(Random(field->GetSize()));
    unsigned int x2(Random(field->GetSize()));
    unsigned int y2(Random(field->GetSize()));
    long long value(field->Get(x1, y1));
    field->Set(x1, y1, field->Get(x2, y2));
    field->Set(x2, y2, value);
}

inline void MutationNearChange(TField field)
{
    long long x1(Random(field->GetSize()));
    long long y1(Random(field->GetSize()));
    long long x2(x1 + Random(3) - 2);
    long long y2(y1 + Random(3) - 2);

    if (x2 < 0 || x2 >= field->GetSize() || y2 < 0 || y2 >= field->GetSize())
    {
        return;
    }

    long long value(field->Get(x1, y1));
    field->Set(x1, y1, field->Get(x2, y2));
    field->Set(x2, y2, value);
}

inline void MutationSpeed(Mutation mutation, unsigned long count, TField field)
{
    for (unsigned long z(0); z < count; ++z)
    {
        mutation(field);
    }
}

inline void MutationRandomChangeFlash(TField field)
{
    unsigned long count(9*field->GetSize());
    MutationSpeed(&MutationRandomChange, count, field);
}

inline void MutationRandomChangeFast(TField field)
{
    unsigned long count(3*field->GetSize());
    MutationSpeed(&MutationRandomChange, count, field);
}

inline void MutationRandomChangeNormal(TField field)
{
    unsigned long count(field->GetSize());
    MutationSpeed(&MutationRandomChange, count, field);
}

inline void MutationRandomChangeSlow(TField field)
{
    unsigned long count(field->GetSize()/3);
    MutationSpeed(&MutationRandomChange, count, field);
}

inline void MutationRandomChangeOnce(TField field)
{
    MutationSpeed(&MutationRandomChange, 1, field);
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

    LOG_ADMIN_INFO <<  "Command line parameters:";
    for (size_t i(0); i < parameters.size(); ++i)
    {
        LOG_PROG_INFO << i << " - " << parameters[i];
    }

    srand(time(NULL));

    unsigned int const minSize(3);
    unsigned int const maxSize(27);

    Field::InitializeClass(maxSize);

    Result best;
    best.Load("../result/best.out");

    for (unsigned int counter(0); counter < 10000; ++counter)
    {
        LOG_ADMIN_INFO << "        Counter: " << counter;
        Result result;
        for (unsigned int size(minSize); size <= maxSize; ++size)
        {
            LOG_ADMIN_INFO << "     Size: " << size;
            boost::shared_ptr<Population<std::less<TField> > > populationMin(new Population<std::less<TField> >());
            boost::shared_ptr<Population<std::greater<TField> > > populationMax(new Population<std::greater<TField> >());

            populationMin->Initialize(size, size*size);
            populationMax->Initialize(size, size*size);

            unsigned int populationSize(2*size);
            unsigned int tryCount(std::min(sqrt(size), 4.)*size);

            for (int step(0); step < 5; ++step)
            {
                LOG_ADMIN_INFO << "Step: " << step;
                if (step < 1)
                {
                    result.AddItem(populationMin->Genetic(&MutationRandomChangeFlash, populationSize, tryCount));
                    result.AddItem(populationMax->Genetic(&MutationRandomChangeFlash, populationSize, tryCount));
                }

                if (step < 2)
                {
                    result.AddItem(populationMin->Genetic(&MutationRandomChangeFast, populationSize, tryCount));
                    result.AddItem(populationMax->Genetic(&MutationRandomChangeFast, populationSize, tryCount));
                }

                if (step < 3)
                {
                    result.AddItem(populationMin->Genetic(&MutationRandomChangeNormal, populationSize, tryCount));
                    result.AddItem(populationMax->Genetic(&MutationRandomChangeNormal, populationSize, tryCount));
                }

                if (step < 4)
                {
                    result.AddItem(populationMin->Genetic(&MutationRandomChangeSlow, populationSize, tryCount));
                    result.AddItem(populationMax->Genetic(&MutationRandomChangeSlow, populationSize, tryCount));
                }

                result.AddItem(populationMin->Genetic(&MutationRandomChangeOnce, populationSize, tryCount));
                result.AddItem(populationMax->Genetic(&MutationRandomChangeOnce, populationSize, tryCount));

                populationSize *= 1.5;
                tryCount *= 1.5;
            }
        }

        Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));

        result.Save("../result/" + time + ".current", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);

        best.Merge(result);

        best.Save("../result/" + time + ".best", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
        best.Save("../result/best");
    }

    Field::ReleaseClass(maxSize);

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
