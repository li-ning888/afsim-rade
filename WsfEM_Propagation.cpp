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

#include "WsfEM_Propagation.hpp"

#include "UtInput.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfEM_XmtrRcvr.hpp"

// =================================================================================================
//! Constructor
//! •	功能: 初始化 WsfEM_Propagation 对象。
//•	设计 :
//•	继承自 WsfObject，表明这是一个通用的对象类型。
//•	默认禁用调试功能（mDebugEnabled = false）。
WsfEM_Propagation::WsfEM_Propagation()
   : WsfObject(),
     mDebugEnabled(false)
{
}

// =================================================================================================
//! Copy constructor
//! •	功能: 创建当前对象的副本。
//•	设计 :
//•	复制基类 WsfObject 的状态。
//•	复制调试状态 mDebugEnabled。
WsfEM_Propagation::WsfEM_Propagation(const WsfEM_Propagation& aSrc)
   : WsfObject(aSrc),
     mDebugEnabled(aSrc.mDebugEnabled)
{
}


// =================================================================================================
//! Initialize the propagation object.
//! @param aXmtrRcvrPtr The transmitter/receiver to which the propagation object is attached.
//virtual
//•	功能 :
//•	初始化传播对象。
//•	提供两种初始化方式：
//1.	使用 WsfEM_XmtrRcvr（推荐）。
//2.	使用 WsfEM_Xmtr（已废弃，向后兼容）。
//•	设计 :
//•	默认实现返回 true，表示初始化成功。
//•	第二种方式通过类型转换调用第一种方式。
bool WsfEM_Propagation::Initialize(WsfEM_XmtrRcvr* aXmtrRcvrPtr)
{
   return true;
}

// =================================================================================================
//! Initialize the propagation object.
//! @param aXmtrPtr The transmitter to which the propagation object is attached.
//! @note This method is deprecated and is provided simply so existing models will continue to
//! function. All new models should use the form that accepts a WsfEM_XmtrRcvr.
//virtual
bool WsfEM_Propagation::Initialize(WsfEM_Xmtr* aXmtrPtr)
{
   return Initialize(static_cast<WsfEM_XmtrRcvr*>(aXmtrPtr));
}

// =================================================================================================
//virtual
//•	功能 :
//•	处理用户输入。
//•	支持启用调试功能的命令（debug）。
//•	设计 :
//•	如果命令是 debug，则启用调试功能。
//•	否则，将输入传递给基类 WsfObject 的 ProcessInput 方法。
bool WsfEM_Propagation::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "debug")
   {
      mDebugEnabled = true;
   }
   else
   {
      myCommand = WsfObject::ProcessInput(aInput);
   }
   return myCommand;
}
