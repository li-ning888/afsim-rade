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

#ifndef WSFSENSORSIGNALPROCESSOR_HPP
#define WSFSENSORSIGNALPROCESSOR_HPP

#include <list>

#include "UtCloneablePtr.hpp"
#include "wsf_export.h"
#include "WsfObject.hpp"
#include "WsfSensor.hpp"
class     WsfSensorMode;

//! An abstract base class that defines a signal processing technique for a sensor.
//!
//! Not all sensors support this feature, but for those that do it provides access to an extendable
//! library that implement signal processing techniques. When a sensor makes use of this interface
//! it typically computes the unprocessed signal and then calls any defined processors to adjust the
//! received power, clutter power, etc., which are then used to calculate the signal-to-interference
//! ratio and subsequently tested for detectability.
//!
//! Additional processors can be added by registering an object factory (AddObjectFactory) that will
//! create instances with the desired functionality.
class WSF_EXPORT WsfSensorSignalProcessor : public WsfObject
{
   public:

      using FactoryPtr = WsfSensorSignalProcessor * (*)(const std::string&);
      using ObjectFactoryList = std::list<FactoryPtr>;

      //! A convenience class to support attaching a list of signal processors to a sensor.
      class WSF_EXPORT List
      {
          //提供添加、删除和修改信号处理器的功能
         public:
            using ListType = std::vector<UtCloneablePtr<WsfSensorSignalProcessor>>;

            List() = default;
            List(const List& aSrc) = default;
            List& operator=(const List& aRhs) = default;
            ~List() = default;

            bool Empty() const  { return mProcessorPtrs.empty(); }
            size_t Size() const { return mProcessorPtrs.size(); }

            bool Initialize(double         aSimTime,
                             WsfSensor*     aSensorPtr,
                             WsfSensorMode* aModePtr,
                             size_t         aBeamIndex = 0);

            bool ProcessInput(UtInput& aInput);

            void Execute(double           aSimTime,
                         WsfSensorResult& aResult);

         private:
            ListType mProcessorPtrs;
      };

      //TODO:STATIC -- remove these
      //! @name Maintenance of object types.
      //@{
      static void AddObjectFactory(FactoryPtr aFactoryPtr);
      static void RemoveObjectFactory(FactoryPtr aFactoryPtr);
      static WsfSensorSignalProcessor* CreateInstance(const std::string& aTypeName);
      static void ClearTypes();
      //@}

      WsfSensorSignalProcessor* Clone() const override = 0;

      //•	职责 : 提供信号处理器的基类，定义了通用接口和工厂方法。
      //    •	主要方法 :
      //•	Initialize : 初始化信号处理器。
      //    •	ProcessInput : 处理输入命令。
      //    •	Execute : 执行信号处理逻辑。
      

      virtual bool Initialize(double         aSimTime,
                              WsfSensor*     aSensorPtr,
                              WsfSensorMode* aModePtr,
                              size_t         aBeamIndex = 0);

      bool ProcessInput(UtInput& aInput) override;

      virtual void Execute(double           aSimTime,
                           WsfSensorResult& aResult) = 0;

   protected:

      //! If 'true' additional information is written out to aid debugging
      bool mDebug{ false };

   private:
       //    •	工厂方法（AddObjectFactory、RemoveObjectFactory、CreateInstance） : 动态创建信号处理器实例。

      static WsfSensorSignalProcessor* PredefinedObjectFactory(const std::string& aTypeName);

      //! The list of object factories
      static ObjectFactoryList sObjectFactoryList;
};

#endif
