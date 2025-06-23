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

#include "WsfEM_Xmtr.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <string>

#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfArticulatedPart.hpp"
#include "WsfComponentFactoryList.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Manager.hpp"
#include "WsfEM_Rcvr.hpp"
#include "WsfEM_Util.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

using namespace std;

//! Since WsfEM_Xmtr does not derive from WsfComponent, this template declaration allows it to "act" like a
//! component only within this translation unit by giving it a role, which it must have in order to use component factories.
template <> struct WsfComponentRole<WsfEM_Xmtr> : std::integral_constant<int, cWSF_COMPONENT_EM_XMTR> {};

// =================================================================================================
//! 构造函数
//! @param aFunction 发射器的主要功能类型
//! @param aAntennaPtr 关联的天线指针(可选)
WsfEM_Xmtr::WsfEM_Xmtr(XmtrFunction   aFunction,
                       WsfEM_Antenna* aAntennaPtr /* = 0 */)
    : WsfEM_XmtrRcvr(aAntennaPtr),  // 初始化基类
    mComponents(),                // 组件列表
    mEM_ManagerPtr(nullptr),      // EM管理器指针
    mLinkedRcvrPtr(nullptr),      // 关联的接收器
    mAlternateFrequencyList(),    // 备用频率列表
    mPowerList(),                 // 功率列表
    mPulseCompressionRatio(1.0),  // 脉冲压缩比
    mPulseRepetitionFrequencies(), // 脉冲重复频率
    mPulseRepetitionIntervals(),   // 脉冲重复间隔
    mPulseWidth(0.0),             // 脉冲宽度
    mDutyCycle(1.0),              // 占空比
    mTransmissionEndTime(-1.0),   // 传输结束时间
    mListeners(),                 // 监听器列表
    mChangeListeners(),           // 变化监听器
    mFunction(aFunction),         // 功能类型
    mCurrentAltFreqId(0),         // 当前备用频率ID
    mExplicitFrequencyList(false), // 显式频率列表标志
    mUseDefaultFrequency(false),  // 使用默认频率标志
    mAllowZeroFrequencyInput(false) // 允许零频率输入标志
{
    // 设置新创建组件的父对象
   mComponents.SetParentOfComponents(this);

   // 设置地球半径乘数(4/3地球模型)
   SetEarthRadiusMultiplier(4.0 / 3.0);

   // 初始化平均或单个PRI，脉冲重复间隔。
   SetPulseRepetitionInterval(0.0, 0);
}

// =================================================================================================
//! Copy construct a transmitter.
//! @param aSrc The transmitter from which to copy.
//! @param aAntennaPtr Pointer to the antenna associated with the transmitter. If one isn't
//! provided or if it is zero then call must be made to SetAntenna() prior to calling Initialize().
//! @note The caller retains ownership of the antenna as it may be shared with other
//! receivers or transmitters.
WsfEM_Xmtr::WsfEM_Xmtr(const WsfEM_Xmtr& aSrc,
                       WsfEM_Antenna*    aAntennaPtr /* = 0 */)
   : WsfEM_XmtrRcvr(aSrc, aAntennaPtr),
     mComponents(aSrc.mComponents),
     mEM_ManagerPtr(nullptr),
     mLinkedRcvrPtr(nullptr),
     mAlternateFrequencyList(aSrc.mAlternateFrequencyList),
     mPowerList(aSrc.mPowerList),
     mPulseCompressionRatio(aSrc.mPulseCompressionRatio),
     mPulseRepetitionFrequencies(aSrc.mPulseRepetitionFrequencies),
     mPulseRepetitionIntervals(aSrc.mPulseRepetitionIntervals),
     mPulseWidth(aSrc.mPulseWidth),
     mDutyCycle(aSrc.mDutyCycle),
     mTransmissionEndTime(-1.0),
     mListeners(),
     mChangeListeners(),
     mFunction(aSrc.mFunction),
     mCurrentAltFreqId(aSrc.mCurrentAltFreqId),
     mExplicitFrequencyList(aSrc.mExplicitFrequencyList),
     mUseDefaultFrequency(aSrc.mUseDefaultFrequency),
     mAllowZeroFrequencyInput(aSrc.mAllowZeroFrequencyInput)
{
   // Newly created components will have me as a parent.
   mComponents.SetParentOfComponents(this);
}

// =================================================================================================
//virtual
WsfEM_Xmtr::~WsfEM_Xmtr()
{
   Deactivate();
}

// =================================================================================================
//! Indicate the transmitter is 'turned-on'.
//!
//! This method adds the transmitter from the list of active transmitters within the simulation.
//! •	将发射器添加到仿真中的活动发射器列表。
//！•	通过 mEM_ManagerPtr 与仿真框架交互。
void WsfEM_Xmtr::Activate()
{
   mEM_ManagerPtr = GetEM_Manager();
   assert(mEM_ManagerPtr != nullptr);
   if (mEM_ManagerPtr != nullptr)
   {
      mEM_ManagerPtr->ActivateXmtr(this);
   }
}

// =================================================================================================
//! Indicate the transmitter is 'turned-off'.
//!
//! This method removes the transmitter from the list of active transmitters within the simulation.
//! •	将发射器从活动列表中移除。
void WsfEM_Xmtr::Deactivate()
{
   if (mEM_ManagerPtr != nullptr)
   {
      mEM_ManagerPtr->DeactivateXmtr(this);
   }
}

// =================================================================================================
//! Is this transmitter allowed to interact with (transmit radiation to) the specified
//! receiver.
//!
//! This method should answer the question, 'Should we allow this transmitter to
//! interact with the supplied receiver'.  This method must NOT consider geometry.
//virtual
//决定发射器是否允许与特定接收器交互。
bool WsfEM_Xmtr::AllowInteractionWith(WsfEM_Rcvr* aRcvrPtr)
{
   return true;
}

// =================================================================================================
//! 计算指定方向的有效辐射功率密度
//! @param aTargetAz    目标方位角(弧度)
//! @param aTargetEl    目标仰角(弧度)
//! @param aEBS_Az      波束方位角(相对于天线)
//! @param aEBS_El      波束仰角(相对于天线)
//! @param aFrequency   频率(Hz)
//! @param aAntennaGain [输出]计算得到的天线增益(绝对值，非dB)
//! @return 有效辐射功率(瓦特)
//virtual
double WsfEM_Xmtr::ComputeRadiatedPower(double  aTargetAz,
                                        double  aTargetEl,
                                        double  aEBS_Az,
                                        double  aEBS_El,
                                        double  aFrequency,
                                        double& aAntennaGain)
{
    // 计算天线增益
   double freq = (aFrequency <= 0.0) ? mFrequency : aFrequency;
   aAntennaGain = GetAntennaGain(mPolarization, freq, aTargetAz, aTargetEl, aEBS_Az, aEBS_El);
   double powerDensity = GetPower(freq) * aAntennaGain / mInternalLoss;
   return powerDensity;
}

// =================================================================================================
//! 获取指定ID的备用频率
//! @param aId 备用频率ID
//! @return 频率值(Hz)，如果ID无效返回0.0
double WsfEM_Xmtr::GetAlternateFrequency(unsigned int aId) const
{
   double freq = 0.0;
   // Check if already selected
   if (!mAlternateFrequencyList.empty())
   {
      auto afli = mAlternateFrequencyList.find(aId);
      if (afli != mAlternateFrequencyList.end())
      {
         freq = afli->second;
      }
   }
   return freq;
}

// =================================================================================================
//! Get the peak power for the currently set frequency.
//! @param aFrequency   [input] Frequency of the receiver receiving the signal (Hertz).
//! @return The peak power at the currently set frequency.
//! •	获取指定频率下的峰值功率。
//！•	如果频率未指定，则使用当前频率。
double WsfEM_Xmtr::GetPeakPower(double aFrequency /*= 0.0*/) const
{
   double power = 0.0;
   if (!mPowerList.empty())
   {
      auto pli = mPowerList.begin();

      double freq = (aFrequency <= 0.0) ? mFrequency : aFrequency;
      while (pli != mPowerList.end())
      {
         //sorted by increasing frequency
         if (freq < (*pli).first)
         {
            break;
         }
         ++pli;
      }

      // Frequency in vector is lower bound of antenna pattern
      // frequency validity range.
      if (pli != mPowerList.begin())
      {
         --pli;
      }
      power = (*pli).second;
   }
   return power;
}

// =================================================================================================
//! Get the average power for the currently set frequency.
//! @return The average power at the currently set frequency.
//! •	获取平均功率，计算公式为 峰值功率 × 占空比。
double WsfEM_Xmtr::GetPower(double aFrequency /*= 0.0*/) const
{
   return GetPeakPower(aFrequency) * GetDutyCycle();
}

// =================================================================================================
//! Initialize the transmitter.
//! @returns 'true' if successful or false if some error was encountered.
//virtual
bool WsfEM_Xmtr::Initialize(WsfSimulation& aSimulation)
{
   double simTime(aSimulation.GetSimTime());

   bool ok = true;

   // Allow component factory to inject components and check dependencies.
   WsfComponentFactoryList<WsfEM_Xmtr>::PreInitialize(aSimulation.GetScenario(), simTime, *this);

   // Pre-Initialize components
   for (auto* componentPtr : mComponents)
   {
      ok &= componentPtr->PreInitialize(simTime);
   }

   // Initialize the base class
   ok &= WsfEM_XmtrRcvr::Initialize(aSimulation);

   if (!mAlternateFrequencyList.empty())
   {
      if (mExplicitFrequencyList)
      {
         if ((! mUseDefaultFrequency) ||
             (mFrequency == 0.0))
         {
            // There was frequency channels or a list entered and the user may have not filled out the
            // transmitter frequency in the input or entered some value outside the list so lets set it
            // to the first frequency (index=0) in the list.
            SetFrequency(mAlternateFrequencyList[0]);
            mCurrentAltFreqId = 0;
         }
      }
      else
      {
         // Alternate frequencies were entered need to set the 0 index frequency to the frequency input.
         mAlternateFrequencyList[0] = mFrequency;
      }

      // Check to see if enforcing the default frequency input and take necessary actions.
      if (mUseDefaultFrequency)
      {
         mAlternateFrequencyList.clear();
         mExplicitFrequencyList = false;
      }
   }

   if ((GetPower() == 0.0) ||
       (mFrequency == 0.0))
   {
      if (mAllowZeroFrequencyInput)
      {
         if (GetPower() == 0.0)
         {
            auto out = ut::log::warning() << "Allowing initialization to continue with power of 0.";
            out.AddNote() << "User beware - set power for platform.";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
         }
         if (mFrequency == 0.0)
         {
            auto out = ut::log::warning() << "Allowing initialization to continue with frequency of 0.";
            out.AddNote() << "User beware - set frequency for platform.";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
         }
      }
      else
      {
         ut::log::error() << "Transmitter 'power' and 'frequency' must be provided.";
         ok = false;
      }
   }

   if ((mPulseWidth > 0.0) &&
       (mPulseRepetitionFrequencies[0] <= 0.0))
   {
      ut::log::error() << "Transmitter 'pulse_repetition_frequency' or 'pulse_repetition_interval' must be provided if 'pulse_width' is specified.";
      ok = false;
   }

   // Initialize components
   for (auto* componentPtr : mComponents)
   {
      ok &= componentPtr->Initialize(simTime);
   }

   return ok;
}

// =================================================================================================
//! Process input from a generic source.
//! @see WsfObject::ProcessInput
//•	功能：处理外部输入命令，用于动态配置发射器的属性。
//•	支持的命令：
//•	frequency：设置发射频率。
//•	power：设置发射功率。
//•	pulse_width：设置脉冲宽度。
//•	duty_cycle：设置占空比。
//•	alternate_frequency：设置备用频率。
//•	frequency_list：设置频率列表。
bool WsfEM_Xmtr::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());
   if (command == "allow_zero_frequency_input")
   {
      aInput.ReadValue(mAllowZeroFrequencyInput);
   }
   // Override the base class to allow for some other operations
   else if (command == "frequency")
   {
      mAlternateFrequencyList.clear();
      mExplicitFrequencyList = false;

      double frequency;
      aInput.ReadValueOfType(frequency, UtInput::cFREQUENCY);
      aInput.ValueGreaterOrEqual(frequency, 0.0);
      SetFrequency(frequency);
   }
   else if (command == "alternate_frequency")
   {
      if (mExplicitFrequencyList)
      {
         mAlternateFrequencyList.clear();  // just in case a frequency_list was entered first, clear it.
         mExplicitFrequencyList = false;
      }

      int altFreqId;
      aInput.ReadValue(altFreqId);
      aInput.ValueInClosedRange(altFreqId, 1, static_cast<int>(mAlternateFrequencyList.size() + 1));
      double frequency;
      aInput.ReadValueOfType(frequency, UtInput::cFREQUENCY);
      mAlternateFrequencyList[altFreqId] = frequency;
   }
   else if (command == "frequency_list")
   {
      mAlternateFrequencyList.clear();
      mExplicitFrequencyList = true;

      UtInputBlock block(aInput);
      while (block.ReadCommand(command))
      {
         if ((command == "frequency_id") ||
             (command == "id_frequency") ||
             (command == "frequency"))
         {
            int altFreqId;
            aInput.ReadValue(altFreqId);
            aInput.ValueInClosedRange(altFreqId, 1, static_cast<int>(mAlternateFrequencyList.size() + 1));
            double frequency;
            aInput.ReadValueOfType(frequency, UtInput::cFREQUENCY);
            mAlternateFrequencyList[altFreqId - 1] = frequency;
         }
         else
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   else if (command == "frequency_channels")
   {
      mAlternateFrequencyList.clear();
      mExplicitFrequencyList = true;

      double freq[2];
      double freqStep;
      aInput.ReadValueOfType(freq[0], UtInput::cFREQUENCY);
      aInput.ReadValueOfType(freqStep, UtInput::cFREQUENCY);
      aInput.ReadValueOfType(freq[1], UtInput::cFREQUENCY);
      aInput.ValueGreater(freq[1], freq[0]);
      aInput.ValueLessOrEqual(freqStep, freq[1] - freq[0]);

      int numChannels = static_cast<int>((freq[1] - freq[0]) / freqStep) + 1;
      for (int i = 0; i < numChannels; ++i)
      {
         mAlternateFrequencyList[i] = freq[0] + static_cast<double>(i * freqStep);
      }
   }
   else if (command == "power")
   {
      double power;
      aInput.ReadValueOfType(power, UtInput::cPOWER_DB);
      aInput.ValueGreater(power, 0.0);
      SetPower(power);
   }
   else if (command == "powers")
   {
      mPowerList.clear();
      UtInputBlock inputBlock(aInput, "end_powers");
      while (inputBlock.ReadCommand(command))
      {
         if (command == "frequency")
         {
            double frequency;
            inputBlock.GetInput().ReadValueOfType(frequency, UtInput::cFREQUENCY);
            inputBlock.GetInput().ValueGreater(frequency, 0.0);

            double power;
            inputBlock.GetInput().ReadValueOfType(power, UtInput::cPOWER_DB);
            inputBlock.GetInput().ValueGreater(power, 0.0);

            if (!SetPower(power, frequency))
            {
               throw UtInput::BadValue(aInput, "Power previously defined for frequency.");
            }
         }
         else
         {
            throw UtInput::UnknownCommand(inputBlock.GetInput());
         }
      }
   }
   else if (command == "pulse_compression_ratio")
   {
      double pulseCompressionRatio;
      aInput.ReadValueOfType(pulseCompressionRatio, UtInput::cRATIO);
      aInput.ValueGreater(pulseCompressionRatio, 0.0);
      SetPulseCompressionRatio(pulseCompressionRatio);
   }
   else if (command == "pulse_repetition_interval")
   {
      double pulseRepetitionInterval;
      aInput.ReadValueOfType(pulseRepetitionInterval, UtInput::cTIME);
      aInput.ValueGreater(pulseRepetitionInterval, 0.0);
      SetPulseRepetitionInterval(pulseRepetitionInterval);
   }
   else if (command == "pulse_repetition_intervals")
   {
      UtInputBlock inputBlock(aInput);
      while (inputBlock.ReadCommand(command))
      {
         if ((command == "pulse_repetition_interval") ||
             (command == "pulse_repetition_intervals_id") ||
             (command == "pri_id") ||
             (command == "pri"))
         {
            int priId;
            aInput.ReadValue(priId);
            aInput.ValueInClosedRange(priId, 1, static_cast<int>(GetPulseRepetitionIntervalCount() + 1));
            double pri;
            aInput.ReadValueOfType(pri, UtInput::cFREQUENCY);
            SetPulseRepetitionInterval(pri, priId);
         }
         else
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   else if (command == "pulse_repetition_frequency")
   {
      double pulseRepetitionFrequency;
      aInput.ReadValueOfType(pulseRepetitionFrequency, UtInput::cFREQUENCY);
      aInput.ValueGreater(pulseRepetitionFrequency, 0.0);
      SetPulseRepetitionFrequency(pulseRepetitionFrequency);
   }
   else if (command == "pulse_repetition_frequencies")
   {
      UtInputBlock inputBlock(aInput, "end_pulse_repetition_frequencies");
      while (inputBlock.ReadCommand(command))
      {
         if ((command == "pulse_repetition_frequency") ||
             (command == "pulse_repetition_frequency_id") ||
             (command == "prf_id") ||
             (command == "prf"))
         {
            int prfId;
            aInput.ReadValue(prfId);
            aInput.ValueInClosedRange(prfId, 1, static_cast<int>(GetPulseRepetitionFrequencyCount() + 1));
            double prf;
            aInput.ReadValueOfType(prf, UtInput::cFREQUENCY);
            SetPulseRepetitionFrequency(prf, prfId);
         }
         else
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   else if (command == "pulse_width")
   {
      double pulseWidth;
      aInput.ReadValueOfType(pulseWidth, UtInput::cTIME);
      aInput.ValueGreater(pulseWidth, 0.0);
      SetPulseWidth(pulseWidth);
   }
   else if ((command == "duty_cycle") ||
            (command == "duty-cycle"))
   {
      double dutyCycle;
      aInput.ReadValue(dutyCycle);
      aInput.ValueInClosedRange(dutyCycle, 0.0, 1.0);
      SetDutyCycle(dutyCycle);
   }
   // Allow embedded antenna input if an antenna is attached.
   else if ((mAntennaPtr != nullptr) &&
            mAntennaPtr->ProcessInput(aInput))
   {
   }
   else if (mComponents.ProcessComponentInput(aInput))
   {
      // First try components already attached. If the input was not recognized by one of them then
      // try the component factories, which may result in the dynamic creation of a new component.
   }
   else if (WsfComponentFactoryList<WsfEM_Xmtr>::ProcessInput(WsfScenario::FromInput(aInput), aInput, *this))
   {
      // If not processed by an attached component try the component factories to see if a component
      // should be dynamically attached.
   }
   else
   {
      myCommand = WsfEM_XmtrRcvr::ProcessInput(aInput);
   }
   return myCommand;
}

// =================================================================================================
//! A convenience method to check for and process a 'transmitter/end_transmitter' block.
//virtual
bool WsfEM_Xmtr::ProcessInputBlock(UtInput& aInput)
{
   bool myCommand = false;
   if (aInput.GetCommand() == "transmitter")
   {
      myCommand = true;
      UtInputBlock inputBlock(aInput);
      while (inputBlock.ReadCommand())
      {
         if (!ProcessInput(aInput))
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   return myCommand;
}

// =================================================================================================
//! Define the receiver that is linked to this transmitter.
//! A receiver is typically linked to a transmitter when they share the same antenna.
//! When certain receiver attributes are defined they will also automatically be set
//! in the linked receiver.
//!
//! The attributes currently include:
//! - Frequency
//! - Antenna pattern
//!
//! @param aRcvrPtr The receiver to be linked to this transmitter.
void WsfEM_Xmtr::SetLinkedReceiver(WsfEM_Rcvr* aRcvrPtr)
{
   mLinkedRcvrPtr = aRcvrPtr;
   WsfEM_XmtrRcvr::AssociateSystems(this, aRcvrPtr);
}

// =================================================================================================
//! Set the bandwidth.
//! @param aBandwidth The bandwidth (Hertz)
//virtual
void WsfEM_Xmtr::SetBandwidth(double aBandwidth)
{
   mBandwidth = aBandwidth;
   if (mEM_ManagerPtr != nullptr)
   {
      mEM_ManagerPtr->UpdateXmtr(this);
   }
}

// =================================================================================================
//! 设置发射频率
//! @param aFrequency 频率值(Hz)
void WsfEM_Xmtr::SetFrequency(double aFrequency)
{
   mFrequency = aFrequency;
   if (mEM_ManagerPtr != nullptr)
   {
      mEM_ManagerPtr->UpdateXmtr(this); // 通知管理器更新
   }
   if (mLinkedRcvrPtr != nullptr)
   {
      mLinkedRcvrPtr->SetFrequency(mFrequency); // 同步更新关联接收器
   }
}

// =================================================================================================
//! Selects the alternate frequency with aId.
//! @note If the aId is not in the list then it is assumed that the list has been exhausted
//!       and it will return to the default (index 0) frequency value.
//! @param aId The Id of the alternate frequency to set; [0, ... N-1], where N is the size of the list.
void WsfEM_Xmtr::SelectAlternateFrequency(unsigned int aId)
{
   // Check if already selected
   if ((aId == mCurrentAltFreqId) || mAlternateFrequencyList.empty()) { return; }

   auto afli = mAlternateFrequencyList.find(aId);
   if (afli != mAlternateFrequencyList.end())
   {
      // Will set the mRcvr frequency also as it is linked to mXmtr
      SetFrequency(mAlternateFrequencyList[aId]);
      mCurrentAltFreqId = aId;
   }
   else
   {
      // Will set the mRcvr frequency also as it is linked to mXmtr
      SetFrequency(mAlternateFrequencyList[0]);
      mCurrentAltFreqId = 0;
   }
}

// =================================================================================================
//! Set the polarization of the transmitted signal.
//! @param aPolarization The polarization.
void WsfEM_Xmtr::SetPolarization(WsfEM_Types::Polarization aPolarization)
{
   if ((aPolarization >= WsfEM_Types::cPOL_DEFAULT) &&
       (aPolarization <  WsfEM_Types::cPOL_COUNT))
   {
      mPolarization = aPolarization;
      if (mLinkedRcvrPtr != nullptr)
      {
         mLinkedRcvrPtr->SetPolarization(mPolarization);
      }
   }
}

// =================================================================================================
//! Set the power.
//! @param aPower The power (Watts)
void WsfEM_Xmtr::SetPower(double aPower)
{
   mPowerList.clear();
   mPowerList.emplace_back(0.0, aPower);
}

// =================================================================================================
//! Set the power for the supplied frequency.
//! @param aPower The power (Watts).
//! @param aFrequency The lower frequency bound of the power input value.
//! @return True if the insert was successful, False if the input frequency was previously defined.
bool WsfEM_Xmtr::SetPower(double aPower,
                          double aFrequency)
{
   bool insertSuccess = true;
   PowerList::iterator pli = mPowerList.begin();

   while (pli !=  mPowerList.end())
   {
      //sort inserts by increasing frequency
      if (aFrequency < (*pli).first)
      {
         break;
      }
      else if (aFrequency == (*pli).first)
      {
         insertSuccess = false;
      }
      ++pli;
   }

   if (insertSuccess == true)
   {
      mPowerList.insert(pli, make_pair(aFrequency, aPower));
   }

   return insertSuccess;
}

// =================================================================================================
//! Set the pulse compression ratio.
//! @param aPulseCompressionRatio Pulse compression ratio (absolute ratio, NOT dB)
void WsfEM_Xmtr::SetPulseCompressionRatio(double aPulseCompressionRatio)
{
   mPulseCompressionRatio = aPulseCompressionRatio;
}

// =================================================================================================
//! 设置脉冲重复频率(PRF)
//! @param aPulseRepetitionFrequency 脉冲重复频率(Hz)
//! @param aPRF_Num PRF编号(默认为1)
void WsfEM_Xmtr::SetPulseRepetitionFrequency(double aPulseRepetitionFrequency, size_t aPRF_Num/* = 1*/)
{
    // 调整容器大小
   if (mPulseRepetitionFrequencies.size() != aPRF_Num + 1)
   {
      mPulseRepetitionIntervals.resize(aPRF_Num + 1, 0.0);
      mPulseRepetitionFrequencies.resize(aPRF_Num + 1, 0.0);
   }
   // 设置PRF和对应的PRI
   mPulseRepetitionFrequencies[aPRF_Num] = aPulseRepetitionFrequency;
   mPulseRepetitionIntervals[aPRF_Num] = 0.0;

   if (mPulseRepetitionFrequencies[aPRF_Num] > 0.0)
   {
      mPulseRepetitionIntervals[aPRF_Num] = 1.0 / mPulseRepetitionFrequencies[aPRF_Num];
   }

   // 重新计算平均值
   if (mPulseRepetitionFrequencies.size() > 1)
   {
      mPulseRepetitionFrequencies[0] = std::accumulate(mPulseRepetitionFrequencies.begin() + 1, mPulseRepetitionFrequencies.end(), 0.0) /
                                       (mPulseRepetitionFrequencies.size() - 1);
   }

   if (mPulseRepetitionFrequencies[0] > 0.0)
   {
      mPulseRepetitionIntervals[0] = 1.0 / mPulseRepetitionFrequencies[0];
   }
}

// =================================================================================================
//! Set the pulse repetition interval (PRI = 1 / PRF)
//! @param aPulseRepetitionInterval Pulse Repetition Interval (seconds)
//! @param aPRI_Num                 The PRI number in the index to set. Must be in increasing order.
void WsfEM_Xmtr::SetPulseRepetitionInterval(double aPulseRepetitionInterval, size_t aPRI_Num/* = 1*/)
{
   if (mPulseRepetitionIntervals.size() != (aPRI_Num + 1))
   {
      mPulseRepetitionFrequencies.resize(aPRI_Num + 1, 0.0);
      mPulseRepetitionIntervals.resize(aPRI_Num + 1, 0.0);
   }

   mPulseRepetitionIntervals[aPRI_Num] = aPulseRepetitionInterval;
   mPulseRepetitionFrequencies[aPRI_Num] = 0.0;

   if (mPulseRepetitionIntervals[aPRI_Num] > 0.0)
   {
      mPulseRepetitionFrequencies[aPRI_Num] = 1.0 / mPulseRepetitionIntervals[aPRI_Num];
   }

   // Recalculate averages
   if (mPulseRepetitionIntervals.size() > 1)
   {
      mPulseRepetitionIntervals[0] = std::accumulate(mPulseRepetitionIntervals.begin() + 1, mPulseRepetitionIntervals.end(), 0.0) /
                                     (mPulseRepetitionIntervals.size() - 1);
   }

   if (mPulseRepetitionIntervals[0] > 0.0)
   {
      mPulseRepetitionFrequencies[0] = 1.0 / mPulseRepetitionIntervals[0];
   }
}

//! Get the pulse repetition frequency
//! @param aIndex [input] The PRF index to lookup.
//! @return Pulse repetition frequency (Hertz).
//! @note If 0.0 then it is a continuous wave transmitter.
//! @note Default returns average PRF, i.e.index 0.
double WsfEM_Xmtr::GetPulseRepetitionFrequency(size_t aIndex/* = 0*/) const
{
   double prf(0.0);
   if ((! mPulseRepetitionFrequencies.empty()) &&
       (aIndex < mPulseRepetitionFrequencies.size()))
   {
      prf = mPulseRepetitionFrequencies[aIndex];
   }
   return prf;
}

std::vector<double> WsfEM_Xmtr::GetPulseRepetitionFrequencies() const
{
   vector<double> prfs(mPulseRepetitionFrequencies.size() - 1);
   copy(mPulseRepetitionFrequencies.begin() + 1,
        mPulseRepetitionFrequencies.end(),
        prfs.begin());
   return prfs;
}

//! Get the pulse repetition interval (1 / PRF).
//! @param aIndex [input] The PRF index to lookup.
//! @return Pulse repetition interval (seconds).
//! @note If 0.0 then it is a continuous wave transmitter.
//! @note Default returns average PRI, i.e. index 0.
double WsfEM_Xmtr::GetPulseRepetitionInterval(size_t aIndex/* = 0*/) const
{
   double pri(0.0);
   if ((!mPulseRepetitionIntervals.empty()) &&
       (aIndex < mPulseRepetitionIntervals.size()))
   {
      pri = mPulseRepetitionIntervals[aIndex];
   }
   return pri;
}

std::vector<double> WsfEM_Xmtr::GetPulseRepetitionIntervals() const
{
   vector<double> pris(mPulseRepetitionIntervals.size() - 1);
   copy(mPulseRepetitionIntervals.begin() + 1,
        mPulseRepetitionIntervals.end(),
        pris.begin());
   return pris;
}

// =================================================================================================
//! Set the pulse width
//! @param aPulseWidth Pulse width (seconds).
void WsfEM_Xmtr::SetPulseWidth(double aPulseWidth)
{
   mPulseWidth = aPulseWidth;
}

// =================================================================================================
//! Set the duty-cycle
//! @param aDutyCycle Duty cycle (absolute).
void WsfEM_Xmtr::SetDutyCycle(double aDutyCycle)
{
   mDutyCycle = aDutyCycle;
}

// =================================================================================================
//! 添加接收器到监听列表
//! @param aRcvrPtr 要添加的接收器指针
//! @return 添加成功返回true，已存在返回false
//virtual
bool WsfEM_Xmtr::AddListener(WsfEM_Rcvr* aRcvrPtr)
{
   bool added = false;
   if (std::find(mListeners.begin(), mListeners.end(), aRcvrPtr) == mListeners.end())
   {
      mListeners.push_back(aRcvrPtr);
      added = true;
   }
   return added;
}

// =================================================================================================
//! Remove a receiver from the list of those to be notified when the transmitter emits.
//! @param aRcvrPtr The receiver to be removed from the list.
//! @returns true if the receiver was removed from the list or false if it was not in the list.
//virtual
bool WsfEM_Xmtr::RemoveListener(WsfEM_Rcvr* aRcvrPtr)
{
   bool removed = false;
   Listeners::iterator lli = std::find(mListeners.begin(), mListeners.end(), aRcvrPtr);
   if (lli != mListeners.end())
   {
      mListeners.erase(lli);
      removed = true;
   }
   return removed;
}

// =================================================================================================
//! 通知监听器发射事件
//! @param aSimTime 当前仿真时间
//! @param aResult  交互结果对象
//virtual
void WsfEM_Xmtr::NotifyListeners(double             aSimTime,
                                 WsfEM_Interaction& aResult)
{
   for (auto& listener : mListeners)
   {
      listener->EmitterActiveCallback(aSimTime, aResult);
   }
}
// =================================================================================================
//! Add a receiver to the list of those to be notified when the transmitter changes parameters.
//! @param aRcvrPtr The receiver to be notified when a transmission prameter changes occur.
//! @returns true if the receiver was added to the list or false if it was already in the list.
//virtual
bool WsfEM_Xmtr::AddChangeListener(WsfEM_Rcvr* aRcvrPtr)
{
   bool added = false;
   if (std::find(mChangeListeners.begin(), mChangeListeners.end(), aRcvrPtr) == mChangeListeners.end())
   {
      mChangeListeners.push_back(aRcvrPtr);
      added = true;
   }
   return added;
}

// =================================================================================================
//! Remove a receiver from the list of those to be notified when the transmitter changes parameters.
//! @param aRcvrPtr The receiver to be removed from the list.
//! @returns true if the receiver was removed from the list or false if it was not in the list.
//virtual
bool WsfEM_Xmtr::RemoveChangeListener(WsfEM_Rcvr* aRcvrPtr)
{
   bool removed = false;
   Listeners::iterator lli = std::find(mChangeListeners.begin(), mChangeListeners.end(), aRcvrPtr);
   if (lli != mChangeListeners.end())
   {
      mChangeListeners.erase(lli);
      removed = true;
   }
   return removed;
}

// =================================================================================================
//! Notify listening receivers that this target's transmitters have changed parameters.
//! @param aSimTime     The current simulation time.
//! @param aTargetIndex The target platform index representing the transmission
//virtual
void WsfEM_Xmtr::NotifyChangeListeners(double aSimTime,
                                       size_t aTargetIndex)
{
   for (auto& changeListener : mChangeListeners)
   {
      changeListener->SignalChangeCallback(aSimTime, aTargetIndex);
   }
}
