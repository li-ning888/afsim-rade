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

#include "WsfEM_PropagationTypes.hpp"

#include <sstream>
#include <memory>

#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "WsfScenario.hpp"

// Pre-defined object factories.
#include "WsfEM_FastMultipath.hpp"
#include "WsfEM_GroundWavePropagation.hpp"

//! The 'null' (no-effect) model.
//! •	功能:
//•	Clone 方法：返回当前对象的深拷贝。
//•	ComputePropagationFactor 方法：始终返回 0，表示无传播效果。
//•	IsNullModel 方法：标记为“空模型”。
//•	用途 : 用于场景中未定义传播模型的情况。
class WsfEM_NullPropagation : public WsfEM_Propagation
{
   public:
      WsfEM_NullPropagation() = default;
      WsfEM_Propagation* Clone() const override { return new WsfEM_NullPropagation(); }
      double ComputePropagationFactor(WsfEM_Interaction& aInteraction,
                                              WsfEnvironment&    aEnvironment) override
      {
         return 0.0;
      }
      bool IsNullModel() const override { return true; }
};


// =================================================================================================
//! Return a modifiable reference to the type list associated with the specified scenario.
WsfEM_PropagationTypes& WsfEM_PropagationTypes::Get(WsfScenario& aScenario)
{
   return aScenario.GetEM_PropagationTypes();
}

// =================================================================================================
//! Return a const reference to the type list associated with the specified scenario.
const WsfEM_PropagationTypes& WsfEM_PropagationTypes::Get(const WsfScenario& aScenario)
{
   return aScenario.GetEM_PropagationTypes();
}

// =================================================================================================
//•	功能 :
//•	初始化传播模型类型列表。
//•	注册默认的传播模型工厂（如 WsfEM_FastMultipath 和 WsfEM_GroundWavePropagation）。
//•	设计 :
//•	继承自 WsfObjectTypeList，支持类型的动态管理。
//•	使用 AddObjectFactory 方法注册工厂函数。
WsfEM_PropagationTypes::WsfEM_PropagationTypes(WsfScenario& aScenario)
   : WsfObjectTypeList<WsfEM_Propagation>(aScenario, cREDEFINITION_ALLOWED, "propagation_model")
{
   mUniqueId = 0;
   AddObjectFactory(WsfEM_FastMultipath::ObjectFactory);
   AddObjectFactory(WsfEM_GroundWavePropagation::ObjectFactory);
}

// =================================================================================================
//! Add an object factory for creating an instance from a fundamental type.
//! The factory should be a static method and should be added only once.
//static
//•	功能 : 将工厂函数添加到工厂列表中。
//•	用途 : 支持动态创建传播模型实例。
void WsfEM_PropagationTypes::AddObjectFactory(FactoryPtr aFactoryPtr)
{
   mObjectFactoryList.push_back(aFactoryPtr);
}

// =================================================================================================
//! Process a possible reference to a propagation model type.
//! @param aInput    [input]  The input stream.
//! @param aTypeName [output] The name of the referenced propagation model type
//! (valid only if the function return value is true).
//! @returns true if the command was a reference to a propagation model or false if not.
//•	功能 :
//•	解析输入流，加载传播模型的引用。
//•	支持两种情况：
//1.	引用现有模型：直接返回模型名称。
//2.	内联定义模型：动态创建模型实例，并注册为用户定义类型。
//•	设计 :
//•	使用 CreateInstance 方法动态创建模型实例。
//•	支持内联定义的唯一命名（如 __propagation : 1__）。
bool WsfEM_PropagationTypes::LoadReference(UtInput&     aInput,
                                           WsfStringId& aTypeName)
{
   std::string command(aInput.GetCommand());
   if ((command != "propagation") &&
       (command != "propagation_model"))
   {
      return false;
   }

   std::string blockTerminator("end_" + command);
   std::string typeName;
   aInput.ReadValue(typeName);
   if (typeName == "none")
   {
      aTypeName.Clear();
      // 'end_propagation[_model]' is optional after 'none', but it must be the next command.
      aInput.ReadValue(command);
      if (command != blockTerminator)
      {
         aInput.PushBack(command);
      }
   }
   else
   {
      // This is a bit of a hack, but it is provided for backward compatibility.
      //
      // If the requested type is one of the core object types then it is assumed to be and inline
      // definition (terminated by an 'end_propagation[_model]]'). In that case we'll implicitly create
      // a user type and return the generated name of the created type.
      //
      // If it wasn't one of the requested types then this is simply a reference to a user model type
      // which will be resolved when the requesting object is initialized. In this case there is no
      // 'end_propagation[_model]]'.
      //
      // The latter is the preferred form because it allows the user to define a default model that can
      // be overridden.

      std::unique_ptr<WsfEM_Propagation> userTypePtr(CreateInstance(typeName));
      if (userTypePtr == nullptr)
      {
         // The requested type name was not one of the core object types, so simply return the name.
         aTypeName = typeName;
      }
      else
      {
         // Inline definition. Populate with the data up to the 'end_propagation[_model]' command.
         UtInputBlock inputBlock(aInput, blockTerminator);
         inputBlock.ProcessInput(userTypePtr.get());

         // Generate a unique name for the inline definition and add to the list of user-defined types.
         std::ostringstream oss;
         oss << "__propagation:" << ++mUniqueId << "__";
         aTypeName = oss.str();                  // Capture and return the generated name
         if (! Add(aTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register propagation_model: " + typeName);
         }
      }
   }
   return true;
}

// =================================================================================================
//! Process propagation_model type input.
//! @param aInput The current input stream.
//! @return 'true' if the current command was recognized and processed
//! 'false' if not recognized.
//•	功能 :
//•	解析输入流，加载传播模型类型。
//•	支持用户定义的传播模型类型。
//•	设计 :
//•	如果 baseTypeName 为 "none"，则创建 WsfEM_NullPropagation。
//•	否则，动态创建指定类型的实例。
WsfEM_PropagationTypes::LoadResult WsfEM_PropagationTypes::LoadType(UtInput& aInput)
{
   LoadResult result;
   std::string command(aInput.GetCommand());
   if ((command == "propagation") ||
       (command == "propagation_model"))
   {
      std::string blockTerminator("end_" + command);
      std::string userTypeName;
      std::string baseTypeName;
      aInput.ReadValue(userTypeName);
      aInput.ReadValue(baseTypeName);

      if (baseTypeName == "none")
      {
         // 'end_propagation[_model]' is optional after 'none', but it must be the next command.
         aInput.ReadValue(command);
         if (command != blockTerminator)
         {
            aInput.PushBack(command);
         }
         std::unique_ptr<WsfEM_Propagation> userTypePtr(new WsfEM_NullPropagation());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register propagation_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
      else
      {
         UtInputBlock inputBlock(aInput, blockTerminator);
         std::unique_ptr<WsfEM_Propagation> userTypePtr(CreateInstance(baseTypeName));
         if (userTypePtr == nullptr)
         {
            throw UtInput::BadValue(aInput, "Unknown propagation_model type: " + baseTypeName);
         }
         inputBlock.ProcessInput(userTypePtr.get());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register propagation_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
   }

   return result;
}


// =================================================================================================
//private
//•	功能 : 根据类型名称动态创建传播模型实例。
//•	设计 :
//•	遍历工厂列表，调用工厂函数创建实例。
//•	如果未找到匹配的工厂，返回 nullptr。
WsfEM_Propagation* WsfEM_PropagationTypes::CreateInstance(const std::string& aTypeName)
{
   WsfEM_Propagation* instancePtr = nullptr;

   auto fli = mObjectFactoryList.begin();
   while ((fli != mObjectFactoryList.end()) &&
          (instancePtr == nullptr))
   {
      FactoryPtr factoryPtr = *fli;
      instancePtr = (*factoryPtr)(aTypeName);
      ++fli;
   }
   return instancePtr;
}
