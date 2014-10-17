#ifndef AZSPCS_MODULE_VERSION_H__INCLUDE_GUARD
#define AZSPCS_MODULE_VERSION_H__INCLUDE_GUARD

#include <Library/String.h>

namespace Version
{

#ifdef UNITTEST

Library::String const applicationName="azspcs";
Library::String const applicationVersion="UNITTEST";
Library::String const applicationDate="1986.01.04";

#else

Library::String const applicationName="azspcs";
Library::String const applicationVersion="0.0.2";
Library::String const applicationDate="2014.10.17";

#endif//UNITTEST

} // namespace Version

#endif//AZSPCS_MODULE_VERSION_H__INCLUDE_GUARD
