#ifndef AZSPCS_MODULE_AZSPCSMODULE_H__INCLUDE_GUARD
#define AZSPCS_MODULE_AZSPCSMODULE_H__INCLUDE_GUARD

#include <Library/BaseModuleManager.h>
#include <Library/Module.h>

namespace Library
{
class XML;
template <bool isExternal>
class BaseTime;
typedef BaseTime<true> Time;
}

namespace azspcs
{

class azspcsModule : public Library::Module
{
public:
    azspcsModule();
    virtual ~azspcsModule();

    virtual void InitializeFactories();
    virtual bool ParseCommandLineParameters(Library::ApplicationParameters const & parameters, bool isManagersReady);
    virtual bool InitializeManagers(Library::ApplicationParameters const & parameters);

private:
    template <typename TType>
    void InitFactory();

    template <typename TType>
    bool InitManager(Library::ApplicationParameters const & parameters);

    virtual void InitializeThreads(Library::ApplicationParameters const & );

    bool InitLogManager();

    virtual void OnRootPermissions() const;
};

azspcsModule * GetModule();

} // namespace azspcs

#endif//AZSPCS_MODULE_AZSPCSMODULE_H__INCLUDE_GUARD
