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

#include "WsfEM_ClutterTypes.hpp"

#include <memory>
#include <sstream>

#include "UtInput.hpp"
#include "UtInputBlock.hpp"

#include "WsfScenario.hpp"

// Pre-defined object factories
#include "WsfEM_SurfaceClutterTable.hpp"

class WsfEM_NullClutter : public WsfEM_Clutter
{
   public:
      WsfEM_NullClutter()
         : WsfEM_Clutter()
      { }
      WsfEM_Clutter* Clone() const override { return new WsfEM_NullClutter(); }
      double ComputeClutterPower(WsfEM_Interaction& aInteraction,
                                         WsfEnvironment&    aEnvironment,
                                         double             aProcessingFactor) override
      {
         return 0.0;
      }
      bool IsNullModel() const override { return true; }
};

// =================================================================================================
//! Return a modifiable reference to the type list associated with the specified scenario.
//• 功能: 返回与指定场景关联的 WsfEM_ClutterTypes 实例。
//•	实现细节 :
//•	提供了非 const 和 const 两个版本，分别返回可修改和只读的实例引用。
//•	用途 : 用于在场景中访问或操作杂波类型列表。
WsfEM_ClutterTypes& WsfEM_ClutterTypes::Get(WsfScenario& aScenario)
{
   return aScenario.GetEM_ClutterTypes();
}

// =================================================================================================
//! Return a const reference to the type list associated with the specified scenario.
const WsfEM_ClutterTypes& WsfEM_ClutterTypes::Get(const WsfScenario& aScenario)
{
   return aScenario.GetEM_ClutterTypes();
}

// =================================================================================================
//•	功能 : 初始化 WsfEM_ClutterTypes 对象，并与指定的场景关联。
//•	实现细节 :
//•	调用基类构造函数。
//•	初始化 mUniqueId 和其他成员变量。
WsfEM_ClutterTypes::WsfEM_ClutterTypes(WsfScenario& aScenario)
   : WsfObjectTypeList<WsfEM_Clutter>(aScenario, cREDEFINITION_ALLOWED, "clutter_model")
{
   mUniqueId = 0;
   mObjectFactoryList.push_back(WsfEM_SurfaceClutterTable::ObjectFactory);
}

// =================================================================================================
//! Add an object factory for creating an instance from a fundamental type.
//! The factory should be a static method and should be added only once.
//static
//•	功能 : 向工厂列表中添加一个新的对象工厂函数。
//•	实现细节 :
//•	将工厂函数指针存储到 mObjectFactoryList 中。
//•	用途 : 支持动态扩展，允许用户定义新的杂波类型。
void WsfEM_ClutterTypes::AddObjectFactory(FactoryPtr aFactoryPtr)
{
   mObjectFactoryList.push_back(aFactoryPtr);
}

// =================================================================================================
//! Process a possible reference to a clutter model type.
//! @param aInput    [input]  The input stream.
//! @param aTypeName [output] The name of the referenced clutter model type
//! (valid only if the function return value is true).
//! @returns true if the command was a reference to a clutter model or false if not.
//•	功能 : 加载杂波类型的引用。
//•	实现细节 :
//•	从输入中读取类型名称，并尝试在当前列表中查找匹配的类型。
//•	如果找不到，可能会触发错误或警告。
//•	用途 : 用于解析场景中对杂波类型的引用。
bool WsfEM_ClutterTypes::LoadReference(UtInput&     aInput,
                                       WsfStringId& aTypeName)
{
   std::string command(aInput.GetCommand());
   if ((command != "clutter") &&
       (command != "clutter_model"))
   {
      return false;
   }

   std::string blockTerminator("end_" + command);
   std::string typeName;
   aInput.ReadValue(typeName);
   if (typeName == "none")
   {
      aTypeName.Clear();
      // 'end_clutter[_model]' is optional after 'none', but it must be the next command.
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
      // definition (terminated by an 'end_clutter[_model]]'). In that case we'll implicitly create
      // a user type and return the generated name of the created type.
      //
      // If it wasn't one of the requested types then this is simply a reference to a user model type
      // which will be resolved when the requesting object is initialized. In this case there is no
      // 'end_clutter[_model]]'.
      //
      // The latter is the preferred form because it allows the user to define a default model that can
      // be overridden.

      std::unique_ptr<WsfEM_Clutter> userTypePtr(CreateInstance(typeName));
      if (userTypePtr.get() == nullptr)
      {
         // The requested type name was not one of the core object types, so simply return the name.
         aTypeName = typeName;
      }
      else
      {
         // Inline definition. Populate with the data up to the 'end_clutter[_model]' command.
         UtInputBlock inputBlock(aInput, blockTerminator);
         inputBlock.ProcessInput(userTypePtr.get());

         // Generate a unique name for the inline definition and add to the list of user-defined types.
         std::ostringstream oss;
         oss << "__clutter:" << ++mUniqueId << "__";
         aTypeName = oss.str();                  // Capture and return the generated name
         if (! Add(aTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register clutter_model: " + typeName);
         }
      }
   }
   return true;
}

// =================================================================================================
//! Process clutter_model type input.
//! @param aInput The current input stream.
//! @return 'true' if the current command was recognized and processed
//! 'false' if not recognized.
//  功能: 从输入文件或流中加载杂波类型。
//•	实现细节 :
//•	解析输入数据，创建并注册新的 WsfEM_Clutter 对象。
//•	重写了基类的虚函数 LoadType。
//•	用途 : 用于动态加载杂波类型配置。
WsfEM_ClutterTypes::LoadResult WsfEM_ClutterTypes::LoadType(UtInput& aInput)
{
   LoadResult result;
   std::string command(aInput.GetCommand());
   if ((command == "clutter") ||
       (command == "clutter_model"))
   {
      std::string blockTerminator("end_" + command);
      std::string userTypeName;
      std::string baseTypeName;
      aInput.ReadValue(userTypeName);
      aInput.ReadValue(baseTypeName);

      if (baseTypeName == "none")
      {
         // 'end_clutter[_model]' is optional after 'none', but it must be the next command.
         aInput.ReadValue(command);
         if (command != blockTerminator)
         {
            aInput.PushBack(command);
         }
         std::unique_ptr<WsfEM_Clutter> userTypePtr(new WsfEM_NullClutter());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register clutter_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
      else
      {
         UtInputBlock inputBlock(aInput, blockTerminator);
         std::unique_ptr<WsfEM_Clutter> userTypePtr(CreateInstance(baseTypeName));
         if (userTypePtr.get() == nullptr)
         {
            throw UtInput::BadValue(aInput, "Unknown clutter_model type: " + baseTypeName);
         }
         inputBlock.ProcessInput(userTypePtr.get());
         result.mObjectTypePtr = userTypePtr.get();
         if (! Add(userTypeName, std::move(userTypePtr)))
         {
            throw UtInput::BadValue(aInput, "Unable to register clutter_model: " + userTypeName);
         }
         result.mIsCommandProcessed = true;
      }
   }

   return result;
}

//•	功能 : 根据类型名称创建一个 WsfEM_Clutter 实例。
//•	实现细节 :
//•	遍历 mObjectFactoryList，调用每个工厂函数，尝试创建匹配的对象。
//•	如果没有匹配的工厂函数，返回 nullptr。
//•	用途 : 动态创建杂波类型实例。
// =================================================================================================
WsfEM_Clutter* WsfEM_ClutterTypes::CreateInstance(const std::string& aTypeName)
{
   WsfEM_Clutter* instancePtr = nullptr;
   for (size_t i = 0; i < mObjectFactoryList.size() && !instancePtr; ++i)
   {
      instancePtr = (*mObjectFactoryList[i])(aTypeName);
   }
   return instancePtr;
}

