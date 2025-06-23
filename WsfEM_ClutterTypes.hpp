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

#ifndef WSFEM_CLUTTERTYPES_HPP
#define WSFEM_CLUTTERTYPES_HPP

#include <vector>

#include "WsfEM_Clutter.hpp"
#include "WsfObjectTypeList.hpp"
//•	继承自 WsfObjectTypeList<WsfEM_Clutter>，这是一个泛型类，专门用于管理 WsfEM_Clutter 类型的对象列表。
//•	WsfEM_Clutter 是杂波的基类，表示单个杂波类型。
class WsfEM_ClutterTypes : public WsfObjectTypeList<WsfEM_Clutter>
{
   public:
      //! @name Static methods to return a reference to the type list associated with a scenario.
      //@{
      static WSF_EXPORT WsfEM_ClutterTypes& Get(WsfScenario& aScenario);
      static WSF_EXPORT const WsfEM_ClutterTypes& Get(const WsfScenario& aScenario);
      //@}

      typedef WsfEM_Clutter* (*FactoryPtr)(const std::string&);

      WsfEM_ClutterTypes(WsfScenario& aScenario);

      LoadResult LoadType(UtInput& aInput) override;

      WSF_EXPORT void AddObjectFactory(FactoryPtr aFactoryPtr);

      WSF_EXPORT bool LoadReference(UtInput&     aInput,
                                    WsfStringId& aTypeName);

   private:
      WsfEM_Clutter* CreateInstance(const std::string& aTypeName);
      //1.	mObjectFactoryList:
      //•	类型 : std::vector<FactoryPtr>。
      //    •	用于存储对象工厂函数的列表，每个工厂函数负责创建特定类型的 WsfEM_Clutter 对象。
      //2.	mUniqueId :
      //    •	类型 : int。
      //    •	用于为每个杂波类型分配唯一标识符。
      // The list of object factories.
      typedef std::vector<FactoryPtr> ObjectFactoryList;
      ObjectFactoryList mObjectFactoryList;
      int               mUniqueId;
};

#endif
