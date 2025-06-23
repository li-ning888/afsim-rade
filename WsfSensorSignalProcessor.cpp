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

#include "WsfSensorSignalProcessor.hpp"

#include <memory>

#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtLog.hpp"

#include "WsfSensorResult.hpp"

using namespace std;

WsfSensorSignalProcessor::ObjectFactoryList WsfSensorSignalProcessor::sObjectFactoryList;

// ================================================================================================
//virtual
//在仿真开始时调用
//
//传递传感器和模式信息
bool WsfSensorSignalProcessor::Initialize(double         aSimTime,
                                          WsfSensor*     aSensorPtr,
                                          WsfSensorMode* aModePtr,
                                          size_t         aBeamIndex /*= 0*/)
{
   return true;
}


// ================================================================================================
//virtual
//解析输入参数
//
//配置处理器参数(如抑制因子、比例因子)
bool WsfSensorSignalProcessor::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command = aInput.GetCommand();
   if (command == "debug")
   {
      mDebug = true;
   }
   else
   {
      myCommand = false;
   }
   return myCommand;
}

// ================================================================================================
// Start of nested class WsfSensorSignalProcessor::List
// ================================================================================================

// ================================================================================================
bool WsfSensorSignalProcessor::List::Initialize(double         aSimTime,
                                                WsfSensor*     aSensorPtr,
                                                WsfSensorMode* aModePtr,
                                                size_t         aBeamIndex/* = 0*/)
{
   bool ok = true;
   for (auto& processorPtr : mProcessorPtrs)
   {
      if (!processorPtr->Initialize(aSimTime, aSensorPtr, aModePtr))

// ================================================================================================
      {
         auto out = ut::log::error() << "Initialization failed for 'signal_processor'.";
         out.AddNote() << "Type: " << processorPtr->GetType();
         ok = false;
      }
   }
   return ok;
}

// ================================================================================================
bool WsfSensorSignalProcessor::List::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());

   if (command == "signal_processor")
   {
      // If a processor of the requested type has already been defined then the input is used to
      // edit the existing definition. Otherwise, a new signal processor is added.
      std::string typeName;
      aInput.ReadValue(typeName);
      auto it = std::find_if(mProcessorPtrs.begin(), mProcessorPtrs.end(), [&typeName](const ListType::value_type& aProc)
                             { return (aProc->GetType() == typeName); });

      if (it == mProcessorPtrs.end())
      {
         // Adding a new signal processor.
         std::unique_ptr<WsfSensorSignalProcessor> newProcessorPtr(CreateInstance(typeName));
         if (newProcessorPtr == nullptr)
         {
            throw UtInput::BadValue(aInput, "Unknown signal_processor type: " + typeName);
         }
         UtInputBlock inputBlock(aInput);
         while (inputBlock.ReadCommand())
         {
            if (! newProcessorPtr->ProcessInput(inputBlock.GetInput()))
            {
               throw UtInput::UnknownCommand(aInput);
            }
         }
         mProcessorPtrs.emplace_back(std::move(newProcessorPtr));
      }
      else
      {
         // Editing an existing signal processor
         UtInputBlock inputBlock(aInput);
         while (inputBlock.ReadCommand())
         {
            if (! (*it)->ProcessInput(inputBlock.GetInput()))
            {
               throw UtInput::UnknownCommand(aInput);
            }
         }
      }
   }
   else if (command == "delete_signal_processor")
   {
      std::string typeName;
      aInput.ReadValue(typeName);
      mProcessorPtrs.erase(std::remove_if(mProcessorPtrs.begin(), mProcessorPtrs.end(),
                                          [&typeName](const ListType::value_type& aProc) { return (aProc->GetType() == typeName); }),
                           mProcessorPtrs.end());
   }
   else
   {
      myCommand = false;
   }
   return myCommand;
}

// ================================================================================================
//•	在仿真时间步中执行信号处理逻辑。
//•	预定义处理器的逻辑：
//•	ConstantClutterSuppression : 调整 aResult.mClutterPower。
//•	ScaleFactor : 调整 aResult.mRcvdPower。
void WsfSensorSignalProcessor::List::Execute(double           aSimTime,
                                             WsfSensorResult& aResult)
{
   for (auto& processorPtr : mProcessorPtrs)
   {
      processorPtr->Execute(aSimTime, aResult);
   }
}

// ================================================================================================
// Start of simple pre-defined signal processors.
//
// These are some very simple signal processors that are included in the baseline that can be used
// as examples.
// ================================================================================================
//恒定杂波抑制
//通过"suppression_factor"参数配置
//
//按比例降低杂波功率
namespace
{
// =============================================================================================
class ConstantClutterSuppression : public WsfSensorSignalProcessor
{
   public:
      ConstantClutterSuppression()
         : mSuppressionFactor(1.0)
      {}
      WsfSensorSignalProcessor* Clone() const override { return new ConstantClutterSuppression(*this); }
      bool ProcessInput(UtInput& aInput) override;
      void Execute(double           aSimTime,
                   WsfSensorResult& aResult) override;
   private:
      double              mSuppressionFactor;
};

bool ConstantClutterSuppression::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   if (aInput.GetCommand() == "suppression_factor")
   {
      aInput.ReadValueOfType(mSuppressionFactor, UtInput::cRATIO);
      aInput.ValueGreater(mSuppressionFactor, 0.0);
   }
   else
   {
      myCommand = WsfSensorSignalProcessor::ProcessInput(aInput);
   }
   return myCommand;
}
//对传感器结果进行处理
//
//可修改接收功率、杂波功率等参数
void ConstantClutterSuppression::Execute(double           aSimTime,
                                         WsfSensorResult& aResult)
{
   aResult.mClutterPower *= mSuppressionFactor;
}

// =============================================================================================
//比例因子
//通过"constant"参数配置
//
//按比例调整接收功率
class ScaleFactor : public WsfSensorSignalProcessor
{
   public:
      ScaleFactor()
         : mScaleFactor(1.0)
      {}
      WsfSensorSignalProcessor* Clone() const override { return new ScaleFactor(*this); }
      bool ProcessInput(UtInput& aInput) override;
      void Execute(double           aSimTime,
                   WsfSensorResult& aResult) override;
   private:
      double              mScaleFactor;
};

bool ScaleFactor::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   if (aInput.GetCommand() == "constant")
   {
      aInput.ReadValue(mScaleFactor);
      aInput.ValueGreater(mScaleFactor, 0.0);
   }
   else
   {
      myCommand = WsfSensorSignalProcessor::ProcessInput(aInput);
   }
   return myCommand;
}

void ScaleFactor::Execute(double           aSimTime,
                          WsfSensorResult& aResult)
{
   aResult.mRcvdPower *= mScaleFactor;
}
}

// ================================================================================================
// Start of static methods for supporting definition of signal processor types and instances.
// ================================================================================================

// ================================================================================================
//! Add an object factory for creating an instance from a fundamental type.
//! The factory should be a static method and should be added only once.
//static
void WsfSensorSignalProcessor::AddObjectFactory(FactoryPtr aFactoryPtr)
{
   // Add the factory for the predefined object types if not already added.
   if (sObjectFactoryList.empty())
   {
      sObjectFactoryList.push_back(WsfSensorSignalProcessor::PredefinedObjectFactory);
   }
   sObjectFactoryList.push_back(aFactoryPtr);
}

// ================================================================================================
//! Add an object factory for removing an instance from a fundamental type.
//! The factory should be a static method and this should be for adding duplicate factories.
//static
//•	提供预定义信号处理器的创建逻辑。
void WsfSensorSignalProcessor::RemoveObjectFactory(FactoryPtr aFactoryPtr)
{
   // Remove the factory for the predefined object types if not already removed.
   sObjectFactoryList.remove(aFactoryPtr);
}

// ================================================================================================
////static
//•	动态创建信号处理器实例。
//•	支持通过类型名称（如 "constant_clutter_suppression"）创建预定义的信号处理器。
WsfSensorSignalProcessor* WsfSensorSignalProcessor::CreateInstance(const std::string& aTypeName)
{
   // Add the factory for the predefined object types if not already added.
   if (sObjectFactoryList.empty())
   {
      sObjectFactoryList.push_back(WsfSensorSignalProcessor::PredefinedObjectFactory);
   }

   // Invoke each object factory to see if it can create an instance of the requested type.
   WsfSensorSignalProcessor* instancePtr = nullptr;
   std::find_if(sObjectFactoryList.begin(), sObjectFactoryList.end(), [&](const FactoryPtr& aFactory)
   {
      instancePtr = (*aFactory)(aTypeName);
      return (instancePtr != nullptr);
   });

   if (instancePtr)
   {
      instancePtr->SetType(aTypeName);
      instancePtr->SetName(aTypeName);
   }

   return instancePtr;
}

//static
void WsfSensorSignalProcessor::ClearTypes()
{
   sObjectFactoryList.clear();
}

// ================================================================================================
//! The object factory for predefined signal processor types.
//private static
WsfSensorSignalProcessor* WsfSensorSignalProcessor::PredefinedObjectFactory(const std::string& aTypeName)
{
   WsfSensorSignalProcessor* instancePtr = nullptr;
   if (aTypeName == "constant_clutter_suppression")
   {
      instancePtr = new ConstantClutterSuppression();
   }
   else if (aTypeName == "scale_factor")
   {
      instancePtr = new ScaleFactor();
   }
   return instancePtr;
}
