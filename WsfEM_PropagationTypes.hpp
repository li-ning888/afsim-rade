// ****************************************************************************
// UNCLASSIFIED//FOUO
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// Distribution authorized to the Department of Defense and U.S. DoD contractors
// REL AUS, CAN, UK, NZ. You may not use this file except in compliance with the
// terms and conditions of 48 C.F.R. 252.204-7000 (Disclosure of Information),
// 48 C.F.R. 252.227-7025 (Limitations on the Use or Disclosure of Government-
// Furnished Information Marked with Restrictive Legends), and the AFSIM
// Memorandum of Understanding or Information Transfer Agreement as applicable.
// All requests for this software must be referred to the Air Force Research
// Laboratory Aerospace Systems Directorate, 2130 8th St., Wright-Patterson AFB,
// OH 45433. This software is provided "as is" without warranties of any kind.
//
// This information is furnished on the condition that it will not be released
// to another nation without specific authority of the Department of the Air Force
// of the United States, that it will be used for military purposes only, that
// individual or corporate rights originating in the information, whether patented
// or not, will be respected, that the recipient will report promptly to the
// United States any known or suspected compromise, and that the information will
// be provided substantially the same degree of security afforded it by the
// Department of Defense of the United States. Also, regardless of any other
// markings on the document, it will not be downgraded or declassified without
// written approval from the originating U.S. agency.
//
// WARNING - EXPORT CONTROLLED
// This document contains technical data whose export is restricted by the
// Arms Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
// Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
// Violations of these export laws are subject to severe criminal penalties.
// Disseminate in accordance with provisions of DoD Directive 5230.25.
// ****************************************************************************

#ifndef WSFEM_PROPAGATIONTYPES_HPP
#define WSFEM_PROPAGATIONTYPES_HPP

#include <vector>

#include "WsfEM_Propagation.hpp"
#include "WsfObjectTypeList.hpp"

class WsfEM_PropagationTypes : public WsfObjectTypeList<WsfEM_Propagation>
{
   public:
      //! @name Static methods to return a reference to the type list associated with a scenario.
      //@{
      static WSF_EXPORT WsfEM_PropagationTypes& Get(WsfScenario& aScenario);
      static WSF_EXPORT const WsfEM_PropagationTypes& Get(const WsfScenario& aScenario);
      //@}

      typedef WsfEM_Propagation* (*FactoryPtr)(const std::string&);

      WsfEM_PropagationTypes(WsfScenario& aScenario);

      LoadResult LoadType(UtInput& aInput) override;

      WSF_EXPORT void AddObjectFactory(FactoryPtr aFactoryPtr);

      WSF_EXPORT bool LoadReference(UtInput&     aInput,
                                    WsfStringId& aTypeName);

   private:
      WsfEM_Propagation* CreateInstance(const std::string& aTypeName);

      // The list of object factories.
      typedef std::vector<FactoryPtr> ObjectFactoryList;
      ObjectFactoryList mObjectFactoryList;
      int               mUniqueId;
};

#endif
