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

#include "WsfAntennaPatternTypes.hpp"

#include "WsfScenario.hpp"
#include "WsfStandardAntennaPattern.hpp"

// =================================================================================================
//! Return a modifiable reference to the type list associated with the specified scenario.
//! •	功能: 提供对场景中天线模式类型列表的访问。
WsfAntennaPatternTypes& WsfAntennaPatternTypes::Get(WsfScenario& aScenario)
{
   return aScenario.GetAntennaPatternTypes();
}

// =================================================================================================
//! Return a const reference to the type list associated with the specified scenario.
const WsfAntennaPatternTypes& WsfAntennaPatternTypes::Get(const WsfScenario& aScenario)
{
   return aScenario.GetAntennaPatternTypes();
}

// =================================================================================================
//•	功能 : 初始化 WsfAntennaPatternTypes 对象。
//•	实现细节 :
//•	调用基类 WsfObjectTypeList 的构造函数，传入场景对象、延迟加载标志（cNO_DELAY_LOAD）和类型名称（"antenna_pattern"）。
//•	设置对象工厂列表 mObjectFactoryList，用于动态创建天线模式对象。
//•	设置默认工厂为 WsfStandardAntennaPattern，即默认创建标准天线模式对象。
WsfAntennaPatternTypes::WsfAntennaPatternTypes(WsfScenario& aScenario)

   : WsfObjectTypeList<WsfAntennaPattern>(aScenario, cNO_DELAY_LOAD, "antenna_pattern")
{
   SetObjectFactory(&mObjectFactoryList);
   SetObjectFactoryDefault<WsfStandardAntennaPattern>();
}

// =================================================================================================
//•	功能 : 初始化单个天线模式对象。
//•	实现细节 :
//•	调用天线模式对象的 Initialize 方法。
//•	传入 nullptr，表示不需要额外的初始化参数。
bool WsfAntennaPatternTypes::InitializeType(WsfAntennaPattern* aObjectPtr)
{
   return aObjectPtr->Initialize(nullptr);
}

// =================================================================================================
//! Add an object factory for creating an instance from a fundamental type.
//! The factory should be a static method and should be added only once.
//static
//•	功能 : 添加一个对象工厂，用于创建特定类型的天线模式对象。
//•	实现细节 :
//•	工厂是一个静态方法指针（FactoryPtr），可以动态注册。
//•	工厂方法被存储在 mObjectFactoryList 中，支持多种天线模式类型的扩展。
void WsfAntennaPatternTypes::AddObjectFactory(FactoryPtr aFactoryPtr)
{
   mObjectFactoryList.push_back(aFactoryPtr);
}
