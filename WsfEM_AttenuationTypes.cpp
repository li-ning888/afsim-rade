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

#include "WsfEM_AttenuationTypes.hpp"

#include <memory>
#include <sstream>

#include "UtInputBlock.hpp"
#include "WsfScenario.hpp"

// Pre-defined object factories.
#include "WsfEM_BlakeAttenuation.hpp"
#include "WsfEM_SimpleAttenuation.hpp"
#include "WsfEM_ITU_Attenuation.hpp"
#include "WsfTabularAttenuation.hpp"

//! The 'null' (no-effect) model.
//空的衰减模型
//•	功能: 始终返回衰减因子 1.0，表示没有任何衰减。
//•	用途 : 用于场景中未定义衰减模型的情况。
class WsfEM_NullAttenuation : public WsfEM_Attenuation
{
   public:
      WsfEM_NullAttenuation() { }
      WsfEM_Attenuation* Clone() const override { return new WsfEM_NullAttenuation(); }
      bool IsNullModel() const override { return true; }
   protected:
      double ComputeAttenuationFactorP(double aRange,
                                               double aElevation,
                                               double aAltitude,
                                               double aFrequency) override
      {
         return 1.0;
      }
};

// =================================================================================================
//! Return a modifiable reference to the type list associated with the specified scenario.
//•	    功能: 返回与指定场景关联的 WsfEM_AttenuationTypes 实例。
//•	    用途 : 通过场景对象访问衰减模型管理器。
WsfEM_AttenuationTypes& WsfEM_AttenuationTypes::Get(WsfScenario& aScenario)
{
   return aScenario.GetEM_AttenuationTypes();
}

// =================================================================================================
//! Return a const reference to the type list associated with the specified scenario.
const WsfEM_AttenuationTypes& WsfEM_AttenuationTypes::Get(const WsfScenario& aScenario)
{
   return aScenario.GetEM_AttenuationTypes();
}

// =================================================================================================
//•	功能 : 初始化衰减模型管理器，并注册预定义的衰减模型工厂。
//•	预定义模型 :
//•	Blake 衰减模型。
//•	ITU 衰减模型。
//•	Simple 衰减模型。
//•	Tabular 衰减模型。
WsfEM_AttenuationTypes::WsfEM_AttenuationTypes(WsfScenario& aScenario)
   : WsfObjectTypeList<WsfEM_Attenuation>(aScenario, cREDEFINITION_ALLOWED, "attenuation_model")
{
   mUniqueId = 0;
   AddObjectFactory(WsfEM_BlakeAttenuation::ObjectFactory);
   AddObjectFactory(WsfEM_ITU_Attenuation::GetObjectFactory(aScenario));
   AddObjectFactory(WsfEM_SimpleAttenuation::ObjectFactory);
   AddObjectFactory(WsfTabularAttenuation::ObjectFactory);
}

// =================================================================================================
//! Add an object factory for creating an instance from a fundamental type.
//! The factory should be a static method and should be added only once.
//•	    功能: 添加一个衰减模型工厂到工厂列表中。
//•	    用途 : 支持动态扩展新的衰减模型类型。
void WsfEM_AttenuationTypes::AddObjectFactory(const Factory& aFactory)
{
   mObjectFactoryList.push_back(aFactory);
}

// =================================================================================================
//! Create a clone of a type object of a given name.
//!
//! The returned object can either be used an instance of the specified type
//! or as a base on which to create a new derived type.
//!
//! @param aTypeName The name of a type object that is to be cloned.
//! @returns A pointer to the clone of the requested type or 0 if the requested type does not exist.
//•	功能: 克隆指定名称的衰减模型。
//•	逻辑 :
//1.	尝试克隆用户定义的衰减模型。
//2.	如果失败，则尝试创建预定义的衰减模型实例。
WsfEM_Attenuation* WsfEM_AttenuationTypes::Clone(WsfStringId aTypeName) const
{
   // First try one of the user-defined types.
   WsfEM_Attenuation* attenuationPtr = WsfObjectTypeList<WsfEM_Attenuation>::Clone(aTypeName);
   if (attenuationPtr == nullptr)
   {
      // Not one of the user-defined types, so try one of the core types.
      attenuationPtr = CreateInstance(aTypeName.GetString());
   }
   return attenuationPtr;
}
// =================================================================================================
//! Process a possible reference to a attenuation model type.
//! @param aInput    [input]  The input stream.
//! @param aTypeName [output] The name of the referenced attenuation model type
//! (valid only if the function return value is true).
//! @returns true if the command was a reference to a attenuation model or false if not.
//•	功能: 解析输入流中的衰减模型引用。
//•	逻辑 :
//1.	如果类型为 none，则清空类型名称。
//2.	如果是核心模型，直接返回引用。
//3.	如果是内联定义，则解析并注册为用户定义模型。
bool WsfEM_AttenuationTypes::LoadReference(UtInput&     aInput,
                                           WsfStringId& aTypeName)
{
   std::string command(aInput.GetCommand());
   if ((command != "attenuation") &&
       (command != "attenuation_model"))
   {
      return false;
   }

   std::string blockTerminator("end_" + command);
   std::string typeName;
   aInput.ReadValue(typeName);
   if (typeName == "none")
   {
      aTypeName.Clear();
      // 'end_attenuation[_model]' is optional after 'none', but it must be the next command.
      aInput.ReadValue(command);
      if (command != blockTerminator)
      {
         aInput.PushBack(command);
      }
   }
   else
   {
      std::unique_ptr<WsfEM_Attenuation> userTypePtr(CreateInstance(typeName));
      if (userTypePtr.get() == nullptr)
      {
         // The requested type name was not one of the core object types.
         // It is a reference to a user-defined model.
         aTypeName = typeName;
      }
      else if (! userTypePtr->AcceptsInlineBlockInput())
      {
         // The request type was one of the core types, but does not accept inline block input.
         // (This is typically one of the older models like 'blake').
         //
         // Simply return a reference to the core model.
         aTypeName = typeName;
         // 'end_attenuation[_model]' is optional, but it must be the next command.
         aInput.ReadValue(command);
         if (command != blockTerminator)
         {
            aInput.PushBack(command);
         }
      }
      else
      {
         // Inline definition. Populate with the data up to the 'end_attenuation[_model]' command.
         UtInputBlock inputBlock(aInput, blockTerminator);
         inputBlock.ProcessInput(userTypePtr.get());

         // Generate a unique name for the inline definition and add to the list of user-defined types.
         std::ostringstream oss;
         oss << "__attenuation:" << ++mUniqueId << "__";
         aTypeName = oss.str();                  // Capture and return the generated name
         if (! Add(aTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register attenuation_model: " + typeName);
         }
      }
   }
   return true;
}

// =================================================================================================
//! Process attenuation_model type input.
//! @param aInput The current input stream.
//! @return 'true' if the current command was recognized and processed, 'false' if not recognized.
//•	功能: 解析输入流中的衰减模型定义。
//•	逻辑 :
//1.	如果基类型为 none，则注册一个 NullAttenuation 模型。
//2.	如果是核心模型，则解析并注册为用户定义模型。
WsfEM_AttenuationTypes::LoadResult WsfEM_AttenuationTypes::LoadType(UtInput& aInput)
{
   LoadResult result;
   std::string command(aInput.GetCommand());
   if ((command == "attenuation") ||
       (command == "attenuation_model"))
   {
      std::string blockTerminator("end_" + command);
      std::string userTypeName;
      std::string baseTypeName;
      aInput.ReadValue(userTypeName);
      aInput.ReadValue(baseTypeName);

      if (baseTypeName == "none")
      {
         // 'end_attenuation[_model]' is optional after 'none', but it must be the next command.
         aInput.ReadValue(command);
         if (command != blockTerminator)
         {
            aInput.PushBack(command);
         }
         std::unique_ptr<WsfEM_Attenuation> userTypePtr(new WsfEM_NullAttenuation());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register attenuation_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
      else
      {
         UtInputBlock inputBlock(aInput, blockTerminator);
         std::unique_ptr<WsfEM_Attenuation> userTypePtr(CreateInstance(baseTypeName));
         if (userTypePtr.get() == nullptr)
         {
            throw UtInput::BadValue(aInput, "Unknown attenuation_model type: " + baseTypeName);
         }
         inputBlock.ProcessInput(userTypePtr.get());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register attenuation_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
   }
   return result;
}

WsfEM_Attenuation* WsfEM_AttenuationTypes::CreateInstance(const std::string& aTypeName) const
{
   WsfEM_Attenuation* instancePtr = nullptr;

   ObjectFactoryList::const_iterator fli = mObjectFactoryList.begin();
   while ((fli != mObjectFactoryList.end()) &&
          (instancePtr == nullptr))
   {
      instancePtr = (*fli)(aTypeName);
      ++fli;
   }
   return instancePtr;
}
