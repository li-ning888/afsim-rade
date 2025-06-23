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

#include "WsfEM_SimpleAttenuation.hpp"

#include "UtInput.hpp"
#include "UtMath.hpp"

// =================================================================================================
//•	功能 : 初始化类的成员变量。
//•	设计 :
//•	mAttenuationFactor：固定衰减因子，默认为 0。
//•	mSpecificAttenuation：特定衰减率，默认为 0。
//•	拷贝构造函数会复制所有成员变量。
WsfEM_SimpleAttenuation::WsfEM_SimpleAttenuation()
   : WsfEM_Attenuation(),
     mAttenuationFactor(0.0),
     mSpecificAttenuation(0.0)
{
}
// =================================================================================================
WsfEM_SimpleAttenuation::WsfEM_SimpleAttenuation(const WsfEM_SimpleAttenuation& aSrc)
   : WsfEM_Attenuation(aSrc),
     mAttenuationFactor(aSrc.mAttenuationFactor),
     mSpecificAttenuation(aSrc.mSpecificAttenuation)
{
}

// =================================================================================================
//! Factory method called by WsfEM_AttenuationTypes.
//static
//•	功能 : 根据类型名称创建 WsfEM_SimpleAttenuation 对象。
//•	设计 : 静态方法，支持多种类型名称（如 "WSF_SIMPLE_ATTENUATION" 和 "simple"）。
WsfEM_Attenuation* WsfEM_SimpleAttenuation::ObjectFactory(const std::string& aTypeName)
{
   WsfEM_Attenuation* instancePtr = nullptr;
   if ((aTypeName == "WSF_SIMPLE_ATTENUATION") ||
       (aTypeName == "simple"))
   {
      instancePtr = new WsfEM_SimpleAttenuation();
   }
   return instancePtr;
}

// =================================================================================================
//virtual
//•	功能 : 创建当前对象的深拷贝。
//•	设计 : 使用拷贝构造函数实现。
WsfEM_Attenuation* WsfEM_SimpleAttenuation::Clone() const
{
   return new WsfEM_SimpleAttenuation(*this);
}

// =================================================================================================
//virtual
//•	功能 : 解析用户输入，设置衰减参数。
//•	逻辑 :
//1.	如果命令是 "specific_attenuation"：
//•	读取值和单位（如 "dB/m"）。
//•	解析单位，计算特定衰减率 mSpecificAttenuation。
//•	禁用固定衰减因子 mAttenuationFactor。
//2.	如果命令是 "attenuation_factor"：
//•	读取固定衰减因子 mAttenuationFactor。
//•	确保其在[0.0, 1.0] 范围内。
//3.	如果命令不匹配，调用基类的 ProcessInput 方法。
//•	注意 : 如果单位格式不正确，会抛出异常。
bool WsfEM_SimpleAttenuation::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "specific_attenuation")
   {
      double value;
      std::string units;
      aInput.ReadValue(value);
      aInput.ReadValue(units);

      // Parse the units.
      std::string::size_type slashPos = units.find('/');
      if ((slashPos != std::string::npos) &&
          (slashPos != 0) &&
          ((slashPos + 1) != units.size()))
      {
         std::string ratioUnits(units.substr(0, slashPos));
         std::string lengthUnits(units.substr(slashPos + 1));
         double ratioValue = aInput.ConvertValue(value, ratioUnits, UtInput::cRATIO);
         double ratioValue_dB = UtMath::LinearToDB(ratioValue);
         double lengthFactor = aInput.ConvertValue(1.0, lengthUnits, UtInput::cLENGTH);
         mSpecificAttenuation = ratioValue_dB / lengthFactor;
      }
      else
      {
         throw UtInput::BadValue(aInput, "Unknown specific attenuation units " + units);
      }
      mAttenuationFactor = 0.0;        // specific_attenuation disables attenuation_factor
   }
   else if (command == "attenuation_factor")
   {
      aInput.ReadValueOfType(mAttenuationFactor, UtInput::cRATIO);
      aInput.ValueInClosedRange(mAttenuationFactor, 0.0, 1.0);
   }
   else
   {
      myCommand = WsfEM_Attenuation::ProcessInput(aInput);
   }
   return myCommand;
}

// =================================================================================================
//protected virtual
//•	功能 : 计算信号的衰减因子。
//•	逻辑 :
//1.	如果固定衰减因子 mAttenuationFactor 大于 0，直接返回。
//2.	否则，基于特定衰减率 mSpecificAttenuation 和距离 aRange 计算：
//•	atten_dB = mSpecificAttenuation * aRange：计算总衰减（单位：dB）。
//•	atten = pow(10.0, -0.1 * atten_dB)：将 dB 转换为线性比例。
//3.	返回衰减因子。
//•	注意 : 该方法忽略了仰角 aElevation、高度 aAltitude 和频率 aFrequency。
double WsfEM_SimpleAttenuation::ComputeAttenuationFactorP(double aRange,
                                                          double aElevation,
                                                          double aAltitude,
                                                          double aFrequency)
{
   double atten = mAttenuationFactor;
   if (atten <= 0.0)
   {
      double atten_dB = mSpecificAttenuation * aRange;     // dB/m * range(m) = dB
      atten = pow(10.0, -0.1 * atten_dB);
   }
   return atten;
}
