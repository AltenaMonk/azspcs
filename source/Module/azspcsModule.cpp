#include "azspcsModule.h"

#include <csignal>
#include <cstring>

#include <boost/bind.hpp>

#include <Library/ApplicationParameters.h>
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

int Random(unsigned long x)
{
    return rand()%static_cast<int>(x);
}

typedef void (*Mutation)(TField);

inline void MutationRandomChange(TField field)
{
    unsigned int x1(Random(field->GetSize()));
    unsigned int y1(Random(field->GetSize()));
    unsigned int x2(Random(field->GetSize()));
    unsigned int y2(Random(field->GetSize()));
    int value(field->Get(x1, y1));
    field->Set(x1, y1, field->Get(x2, y2));
    field->Set(x2, y2, value);
}

inline void MutationNearChange(TField field)
{
    int x1(Random(field->GetSize()));
    int y1(Random(field->GetSize()));
    int x2(x1 + Random(3) - 2);
    int y2(y1 + Random(3) - 2);

    if (x2 < 0 || x2 >= field->GetSize() || y2 < 0 || y2 >= field->GetSize())
    {
        return;
    }

    int value(field->Get(x1, y1));
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

inline void Invert(unsigned int & value, unsigned int size)
{
    value = size - 1 - value;
}

inline void Detect(unsigned int & a, unsigned int & b, unsigned int value, unsigned int size)
{
    a = value / size;
    b = value % size;
}

template <char TMethod>
inline void GetXY(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size);

template <>
inline void GetXY<0>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(x, y, value, size);
}

template <>
inline void GetXY<1>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(x, y, value, size);
    Invert(x, size);
}

template <>
inline void GetXY<2>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(x, y, value, size);
    Invert(y, size);
}

template <>
inline void GetXY<3>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(x, y, value, size);
    Invert(x, size);
    Invert(y, size);
}

template <>
inline void GetXY<4>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(y, x, value, size);
}

template <>
inline void GetXY<5>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(y, x, value, size);
    Invert(x, size);
}

template <>
inline void GetXY<6>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(y, x, value, size);
    Invert(y, size);
}

template <>
inline void GetXY<7>(unsigned int & x, unsigned int & y, unsigned int value, unsigned int size)
{
    Detect(y, x, value, size);
    Invert(x, size);
    Invert(y, size);
}

template <typename TCompare, char TMethod>
int MutationUpdateFirst(TField field)
{
    TCompare compareFunctor;
    unsigned int size(field->GetSize());
    unsigned int maxValue(size * size);
    TField bestValue(field->Clone());
    switch (field->GetType())
    {
        case 2:
        {
            for (int value1(0); value1 < maxValue; ++value1)
            {
                unsigned int x1(0), y1(0);
                GetXY<TMethod>(x1, y1, value1, size);
                for (int value2(value1 + 1); value2 < maxValue; ++value2)
                {
                    unsigned int x2(0), y2(0);
                    GetXY<TMethod>(x2, y2, value2, size);
                    field->Swap(x1, y1, x2, y2);

                    if (compareFunctor(field, bestValue))
                    {
                        return std::max(bestValue->GetValue(), field->GetValue()) - std::min(bestValue->GetValue(), field->GetValue());
                    }

                    field->Swap(x1, y1, x2, y2);
                }
            }
        }
        break;
        case 3:
        {
            for (int value1(0); value1 < maxValue; ++value1)
            {
                unsigned int x1(0), y1(0);
                GetXY<TMethod>(x1, y1, value1, size);
                for (int value2(value1 + 1); value2 < maxValue; ++value2)
                {
                    unsigned int x2(0), y2(0);
                    GetXY<TMethod>(x2, y2, value2, size);
                    for (int value3(value2 + 1); value3 < maxValue; ++value3)
                    {
                        unsigned int x3(0), y3(0);
                        GetXY<TMethod>(x3, y3, value3, size);

                        field->Rotate(x1, y1, x2, y2, x3, y3);

                        if (compareFunctor(field, bestValue))
                        {
                            return std::max(bestValue->GetValue(), field->GetValue()) - std::min(bestValue->GetValue(), field->GetValue());
                        }

                        field->Rotate(x1, y1, x2, y2, x3, y3);

                        if (compareFunctor(field, bestValue))
                        {
                            return std::max(bestValue->GetValue(), field->GetValue()) - std::min(bestValue->GetValue(), field->GetValue());
                        }

                        field->Rotate(x1, y1, x2, y2, x3, y3);
                    }
                }
            }
        }
        break;
    }
    return 0;
}

bool azspcsModule::ParseCommandLineParameters(Library::ApplicationParameters const & parameters, bool IsManagersReady)
{
    /// @internal Без инициализированных логов проверяем только версию:
    if (IsManagersReady == false)
    {
        if (parameters.Is("--version", "-v"))
        {
            Library::Module * module(Library::GetModule());
            std::cout << module->GetApplicationName() << " " << module->GetApplicationVersion() << " " << module->GetApplicationDate() << " " << std::endl;
            return false;
        }
        return true;
    }

    parameters.Log();

    srand(time(NULL));

    unsigned int const minSize(3);
    unsigned int const maxSize(27);

    Field::InitializeClass(maxSize);

    Result best;
    best.Load("../result/best.out");

    for (unsigned int counter(0); counter < 10000; ++counter)
    {
        LOG_ADMIN_INFO << "        Counter: " << counter;
//        Result result;
//        for (unsigned int size(minSize); size <= maxSize; ++size)
//        {
//            LOG_ADMIN_INFO << "     Size: " << size;
//            boost::shared_ptr<Population<std::less<TField> > > populationMin(new Population<std::less<TField> >());
//            boost::shared_ptr<Population<std::greater<TField> > > populationMax(new Population<std::greater<TField> >());

//            populationMin->Initialize(size, size*size);
//            populationMax->Initialize(size, size*size);

//            unsigned int populationSize(/*2**/size);
//            unsigned int tryCount(std::min(sqrt(size), 4.)/**size*/);
//            double multiplayer(1/*.5*/);

//            for (int step(0); step < 5; ++step)
//            {
//                LOG_ADMIN_INFO << "Step: " << step;
//                if (step < 1)
//                {
//                    result.AddItem(populationMin->Genetic(&MutationRandomChangeFlash, populationSize, tryCount));
//                    result.AddItem(populationMax->Genetic(&MutationRandomChangeFlash, populationSize, tryCount));
//                }

//                if (step < 2)
//                {
//                    result.AddItem(populationMin->Genetic(&MutationRandomChangeFast, populationSize, tryCount));
//                    result.AddItem(populationMax->Genetic(&MutationRandomChangeFast, populationSize, tryCount));
//                }

//                if (step < 3)
//                {
//                    result.AddItem(populationMin->Genetic(&MutationRandomChangeNormal, populationSize, tryCount));
//                    result.AddItem(populationMax->Genetic(&MutationRandomChangeNormal, populationSize, tryCount));
//                }

//                if (step < 4)
//                {
//                    result.AddItem(populationMin->Genetic(&MutationRandomChangeSlow, populationSize, tryCount));
//                    result.AddItem(populationMax->Genetic(&MutationRandomChangeSlow, populationSize, tryCount));
//                }

//                result.AddItem(populationMin->Genetic(&MutationRandomChangeOnce, populationSize, tryCount));
//                result.AddItem(populationMax->Genetic(&MutationRandomChangeOnce, populationSize, tryCount));

//                populationSize *= multiplayer;
//                tryCount *= multiplayer;
//            }
//        }

//        Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));

//        result.Save("../result/" + time + ".current", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);

//        best.Merge(result);

//        best.Save("../result/" + time + ".best", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);

        {
            int tryCount(1);
            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 0>, &MutationUpdateFirst<std::greater<TField>, 0>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 1>, &MutationUpdateFirst<std::greater<TField>, 1>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 2>, &MutationUpdateFirst<std::greater<TField>, 2>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 3>, &MutationUpdateFirst<std::greater<TField>, 3>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 4>, &MutationUpdateFirst<std::greater<TField>, 4>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 5>, &MutationUpdateFirst<std::greater<TField>, 5>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 6>, &MutationUpdateFirst<std::greater<TField>, 6>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }

            {
                best.UpdateFirst(&MutationUpdateFirst<std::less<TField>, 7>, &MutationUpdateFirst<std::greater<TField>, 7>, tryCount);
                Library::String time(Library::Time::Now().ToString("YYYYMMDDhhmmss"));
                best.Save("../result/" + time + ".best2", Version::applicationName + " " + Version::applicationVersion + " " + Version::applicationDate + "\n", true);
                best.Save("../result/best");
            }
        }

//        best.Save("../result/best");
    }

    Field::ReleaseClass(maxSize);

    return true;
}

bool azspcsModule::InitializeManagers(Library::ApplicationParameters const & parameters)
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
bool azspcsModule::InitManager(Library::ApplicationParameters const & parameters)
{
    boost::shared_ptr<Library::BaseModuleManager> manager(new TType());
    bool result(manager->Initialize(parameters));
    SetManager(Library::TypeToKey<TType>::Value(), manager);
    return result;
}

void azspcsModule::InitializeThreads(Library::ApplicationParameters const & )
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
