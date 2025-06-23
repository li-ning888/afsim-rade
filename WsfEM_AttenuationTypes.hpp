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

#ifndef WSFEM_ATTENUATIONTYPES_HPP
#define WSFEM_ATTENUATIONTYPES_HPP

#include <list>
#include <functional>

#include "WsfEM_Attenuation.hpp"
#include "WsfObjectTypeList.hpp"

class WsfEM_AttenuationTypes : public WsfObjectTypeList<WsfEM_Attenuation>
{
    //该类继承自 WsfObjectTypeList<WsfEM_Attenuation>，是一个衰减模型的类型管理器，主要功能包括：
    //    •	注册预定义的衰减模型工厂。
    //    •	动态创建衰减模型实例。
    //    •	支持用户定义的衰减模型。
    //    •	解析输入流中的衰减模型定义。
   public:
      //! @name Static methods to return a reference to the type list associated with a scenario.
      //@{
      static WSF_EXPORT WsfEM_AttenuationTypes& Get(WsfScenario& aScenario);
      static WSF_EXPORT const WsfEM_AttenuationTypes& Get(const WsfScenario& aScenario);
      //@}

      typedef std::function<WsfEM_Attenuation*(const std::string&)> Factory;

      WsfEM_AttenuationTypes(WsfScenario& aScenario);
      typedef std::list<Factory> ObjectFactoryList;

      WSF_EXPORT void AddObjectFactory(const Factory& aFactory);
      LoadResult LoadType(UtInput& aInput) override;
      bool LoadReference(UtInput&     aInput,
                         WsfStringId& aTypeName);
      WsfEM_Attenuation* Clone(WsfStringId aTypeName) const override;

   private:

      WsfEM_Attenuation* CreateInstance(const std::string& aTypeName) const;

      // The list of object factories.
      ObjectFactoryList mObjectFactoryList;
      int               mUniqueId;
};

#endif
