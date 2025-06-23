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

#include "WsfRadarSignature.hpp"

#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "UtMath.hpp"
#include "UtScriptClassDefine.hpp"
#include "UtScriptTypes.hpp"
#include "WsfEM_Util.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfGeoPoint.hpp"
#include "WsfPlatform.hpp"
#include "WsfRadarSignatureTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSignatureInterface.hpp"
#include "WsfSignatureList.hpp"
#include "script/WsfScriptContext.hpp"

// =================================================================================================
WsfRadarSignature::WsfRadarSignature()
   : WsfSignature()
{
}

// =================================================================================================
// Definition of the default signature to be used if a signature is not defined on a platform
// and a sensor is present that requires the signature.
// =================================================================================================

namespace
{
    //默认雷达特征实现，当平台未定义特征时使用

    //    特点：
    //    固定返回1000平方米的RCS值

    //    实现Clone方法支持复制
   class DefaultSignature : public WsfRadarSignature
   {
      public:
         DefaultSignature()
            : WsfRadarSignature()
         { }
         WsfRadarSignature* Clone() const override                           { return new DefaultSignature(); }
         float GetSignature(WsfStringId               aStateId,
                            WsfEM_Types::Polarization aPolarization,
                            double                    aFrequency,
                            double                    aTgtToXmtrAz,
                            double                    aTgtToXmtrEl,
                            double                    aTgtToRcvrAz,
                            double                    aTgtToRcvrEl,
                            WsfEM_Xmtr*               aXmtrPtr,
                            WsfEM_Rcvr*               aRcvrPtr) override     { return 1000.0F; } // m^2

   };
}

// =================================================================================================
// Definition of the signature-specific interface class
// =================================================================================================

namespace
{
   std::string sClassName("radar_signature");
   std::string sShortName("radar");
   //特征接口实现，管理特征对象的生命周期

   //    关键功能：
   //    提供类名和短名称标识

   //    负责特征对象的克隆

   //    连接平台和特征对象
   class Interface : public WsfSignatureInterface
   {
      public:
         Interface()
            : WsfSignatureInterface()
         { }
         Interface(const Interface& aSrc)
            : WsfSignatureInterface(aSrc)
         { }
         std::string GetClassName() const override                              { return sClassName; }
         std::string GetShortName() const override                              { return sShortName; }
         WsfSignatureInterface* Clone() const override                          { return new Interface(*this); }
         WsfSignature* CloneSignature(const WsfScenario& aScenario) const override
         {
            return WsfRadarSignatureTypes::Get(aScenario).Clone(mInputType);
         }
   };
}

// =================================================================================================
// Static methods to support access to the signature on a platform.
// =================================================================================================

// =================================================================================================
//! Get the input type of the signature.
//! @param aPlatformPtr [input] The pointer to the platform containing the signature.
//! @returns The current signature state.
WsfStringId WsfRadarSignature::GetInputType(WsfPlatform* aPlatformPtr)
{
   return aPlatformPtr->GetSignatureList().GetInputType(cSIGNATURE_INDEX);
}

// =================================================================================================
//! Get the current signature state.
//! @param aPlatformPtr [input] The pointer to the platform containing the signature.
//! @returns The current signature state.
WsfStringId WsfRadarSignature::GetState(WsfPlatform* aPlatformPtr)
{
   return aPlatformPtr->GetSignatureList().GetState(cSIGNATURE_INDEX);
}

// =================================================================================================
//! Set the signatures state
//! @param aPlatformPtr [input] The pointer to the platform containing the signature.
//! @param aState       [input] The new signatures state.
//! @returns true if successful, false if and error occurred.
bool WsfRadarSignature::SetState(WsfPlatform* aPlatformPtr,
                                 WsfStringId  aState)
{
   return aPlatformPtr->GetSignatureList().SetState(cSIGNATURE_INDEX, aState);
}

// =================================================================================================
//! Get the signature scale factor.
//! @param aPlatformPtr [input] The pointer to the platform containing the signature.
//! @returns The signature scale factor.
float WsfRadarSignature::GetScaleFactor(WsfPlatform* aPlatformPtr)
{
   return aPlatformPtr->GetSignatureList().GetScaleFactor(cSIGNATURE_INDEX);
}

// =================================================================================================
//! Set the signature scale factor.
//! @param aPlatformPtr [input] The pointer to the platform containing the signature.
//! @param aScaleFactor [input] The new signature scale factor.
//! @returns true if successful, false if and error occurred.
bool WsfRadarSignature::SetScaleFactor(WsfPlatform* aPlatformPtr,
                                       float        aScaleFactor)
{
   return aPlatformPtr->GetSignatureList().SetScaleFactor(cSIGNATURE_INDEX, aScaleFactor);
}

// =================================================================================================
//! Get the radar signature for a given set of conditions.
//! @param aPlatformPtr  [input] The pointer to the platform containing the signature.
//! @param aPolarization [input] The polarization of the signal.
//! @param aFrequency    [input] The frequency of the signal (Hz).
//! @param aTgtToXmtrAz  [input] The azimuth   of the transmitter with respect to the target.
//! @param aTgtToXmtrEl  [input] The elevation of the transmitter with respect to the target.
//! @param aTgtToRcvrAz  [input] The azimuth   of the receiver    with respect to the target.
//! @param aTgtToRcvrEl  [input] The elevation of the receiver    with respect to the target.
//! @returns The radar cross section (m^2)
//基本参数版
//使用显式参数传递
//发射器和接收器指针为nullptr
float WsfRadarSignature::GetValue(WsfPlatform*              aPlatformPtr,
                                  WsfEM_Types::Polarization aPolarization,
                                  double                    aFrequency,
                                  double                    aTgtToXmtrAz,
                                  double                    aTgtToXmtrEl,
                                  double                    aTgtToRcvrAz,
                                  double                    aTgtToRcvrEl)
{
   // 获取平台的特征接口(Interface)
   Interface* interfacePtr = static_cast<Interface*>(aPlatformPtr->GetSignatureList().GetInterface(cSIGNATURE_INDEX));
   WsfRadarSignature* signaturePtr = static_cast<WsfRadarSignature*>(interfacePtr->GetSignature());
  //检查特征对象是否存在
   if (signaturePtr == nullptr)
   {
      // Signature not defined... use the default signature.
      { // RAII block
         auto out = ut::log::warning() << "Undefined radar signature.";
         out.AddNote() << "Platform: " << aPlatformPtr->GetName();
         out.AddNote() << "Platform Type: " << aPlatformPtr->GetType();
         out.AddNote() << "Using default.";
      }
      signaturePtr = new DefaultSignature();
      interfacePtr->SetSignature(signaturePtr);
   }
   //调用特征对象的GetSignature()计算原始RCS
   float value = signaturePtr->GetSignature(interfacePtr->GetState(), aPolarization, aFrequency,
                                            aTgtToXmtrAz, aTgtToXmtrEl, aTgtToRcvrAz, aTgtToRcvrEl, nullptr);
   //应用缩放因子(ScaleFactor)，返回最终RCS值
   return value * interfacePtr->GetScaleFactor();
}

// =================================================================================================
//! Get the radar signature for a given set of conditions.
//! @param aPlatformPtr  [input] The pointer to the platform containing the signature.
//! @param aXmtrPtr      [input] The pointer to the transmitter.
//! @param aRcvrPtr      [input] The pointer to the receiver.
//! @param aTgtToXmtrAz  [input] The azimuth   of the transmitter with respect to the target.
//! @param aTgtToXmtrEl  [input] The elevation of the transmitter with respect to the target.
//! @param aTgtToRcvrAz  [input] The azimuth   of the receiver    with respect to the target.
//! @param aTgtToRcvrEl  [input] The elevation of the receiver    with respect to the target.
//! @returns The radar cross section (m^2)
//EM对象版
//从EM对象获取极化和频率
//支持更完整的特征计算
float WsfRadarSignature::GetValue(WsfPlatform* aPlatformPtr,
                                  WsfEM_Xmtr*  aXmtrPtr,
                                  WsfEM_Rcvr*  aRcvrPtr,
                                  double       aTgtToXmtrAz,
                                  double       aTgtToXmtrEl,
                                  double       aTgtToRcvrAz,
                                  double       aTgtToRcvrEl)
{
   Interface* interfacePtr = static_cast<Interface*>(aPlatformPtr->GetSignatureList().GetInterface(cSIGNATURE_INDEX));
   WsfRadarSignature* signaturePtr = static_cast<WsfRadarSignature*>(interfacePtr->GetSignature());
   if (signaturePtr == nullptr)
   {
      // Signature not defined... use the default signature.
      { // RAII block
         auto out = ut::log::warning() << "Undefined radar signature.";
         out.AddNote() << "Platform: " << aPlatformPtr->GetName();
         out.AddNote() << "Platform Type: " << aPlatformPtr->GetType();
         out.AddNote() << "Using default.";
      }
      signaturePtr = new DefaultSignature();
      interfacePtr->SetSignature(signaturePtr);
   }
   float value = signaturePtr->GetSignature(interfacePtr->GetState(), aXmtrPtr->GetPolarization(), aXmtrPtr->GetFrequency(),
                                            aTgtToXmtrAz, aTgtToXmtrEl, aTgtToRcvrAz, aTgtToRcvrEl, aXmtrPtr, aRcvrPtr);
   return value * interfacePtr->GetScaleFactor();
}

// =================================================================================================
//! Get the radar signature object for a platform.
//! @param aPlatformPtr  [input] The pointer to the platform containing the signature.
//static
WsfRadarSignature* WsfRadarSignature::GetSignature(WsfPlatform* aPlatformPtr)
{
   Interface* interfacePtr = static_cast<Interface*>(aPlatformPtr->GetSignatureList().GetInterface(cSIGNATURE_INDEX));
   return static_cast<WsfRadarSignature*>(interfacePtr->GetSignature());
}

// =================================================================================================
//! Get the radar signature object for a platform.
//! @param aPlatformPtr  [input] The pointer to the platform containing the signature.
//! @param aSignaturePtr [input] The pointer to the signature. The system assumes ownership if the
//! method completes successfully.
//static
bool WsfRadarSignature::SetSignature(WsfPlatform*       aPlatformPtr,
                                     WsfRadarSignature* aSignaturePtr)
{
   bool ok(true);
   Interface* interfacePtr = static_cast<Interface*>(aPlatformPtr->GetSignatureList().GetInterface(cSIGNATURE_INDEX));
   if (aSignaturePtr != interfacePtr->GetSignature())
   {
      ok = (aSignaturePtr->InitializeType() &&
            aSignaturePtr->Initialize(aPlatformPtr->GetSimTime(), aPlatformPtr));
      if (ok)
      {
         interfacePtr->SetSignature(aSignaturePtr);
      }
   }
   return ok;
}

// =================================================================================================
// Script Interface
// =================================================================================================

namespace
{
   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, RadarSigState, 0, "string", "")
   {
      aReturnVal.SetString(WsfRadarSignature::GetState(aObjectPtr));
   }

   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, SetRadarSigState, 1, "void", "string")
   {
      WsfStringId stateId = aVarArgs[0].GetString();
      WsfRadarSignature::SetState(aObjectPtr, stateId);
   }

   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, RadarSigScaleFactor, 0, "double", "")
   {
      aReturnVal.SetDouble(WsfRadarSignature::GetScaleFactor(aObjectPtr));
   }

   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, SetRadarSigScaleFactor, 1, "void", "double")
   {
      float scaleFactor = static_cast<float>(aVarArgs[0].GetDouble());
      WsfRadarSignature::SetScaleFactor(aObjectPtr, scaleFactor);
   }

   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, RadarCrossSection_1, 4, "double", "WsfGeoPoint, double, string, double")
   {
      WsfGeoPoint* viewPointPtr = (WsfGeoPoint*)aVarArgs[0].GetPointer()->GetAppObject();
      double earthRadiusScale = aVarArgs[1].GetDouble();
      earthRadiusScale = (earthRadiusScale > 0.0) ? earthRadiusScale : (4.0 / 3.0);       // Use default if requested

      WsfEM_Types::Polarization polarization{WsfEM_Types::cPOL_DEFAULT};
      WsfEM_Util::StringToEnum(polarization, aVarArgs[2].GetString());

      double frequency = aVarArgs[3].GetDouble();

      // Compute the azimuth and elevation of the viewpoint with respect to the target (this platform).
      double viewerLocWCS[3];
      viewPointPtr->GetLocationWCS(viewerLocWCS);
      double simTime = WsfScriptContext::GetTIME_NOW(aContext);
      aObjectPtr->Update(simTime);
      double targetToViewerAz, targetToViewerEl;
      WsfEM_Util::ComputeViewerAspect(aObjectPtr, viewerLocWCS, earthRadiusScale, targetToViewerAz, targetToViewerEl);

      double value = WsfRadarSignature::GetValue(aObjectPtr, polarization, frequency,
                                                 targetToViewerAz, targetToViewerEl,
                                                 targetToViewerAz, targetToViewerEl);
      aReturnVal.SetDouble(value);
   }

   // RadarCrossSection(double aAzimuth, double aElevation, polarization, frequency)
   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, RadarCrossSection_2, 4, "double", "double, double, string, double")
   {
      double targetToViewerAz = aVarArgs[0].GetDouble() * UtMath::cRAD_PER_DEG;
      double targetToViewerEl = aVarArgs[1].GetDouble() * UtMath::cRAD_PER_DEG;

      WsfEM_Types::Polarization polarization{WsfEM_Types::cPOL_DEFAULT};
      WsfEM_Util::StringToEnum(polarization, aVarArgs[2].GetString());

      double frequency = aVarArgs[3].GetDouble();

      double value = WsfRadarSignature::GetValue(aObjectPtr, polarization, frequency, targetToViewerAz,
                                                 targetToViewerEl, targetToViewerAz, targetToViewerEl);
      aReturnVal.SetDouble(value);
   }

} // end of anonymous namespace

// =================================================================================================
//! Register the script methods associated with this class.
//! @note The WsfPlatform script class must be defined before calling this method.
//static
void WsfRadarSignature::RegisterScriptMethods(UtScriptTypes& aScriptTypes)
{
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<RadarSigState>());
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<SetRadarSigState>());
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<RadarSigScaleFactor>());
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<SetRadarSigScaleFactor>());
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<RadarCrossSection_1>("RadarCrossSection"));
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<RadarCrossSection_2>("RadarCrossSection"));
}

// =================================================================================================
//! Register the platform interface object on the prototype signature list in the specified scenario.
//static
void WsfRadarSignature::RegisterInterface(WsfScenario& aScenario)
{
   aScenario.GetSignatureListPrototype().AddInterface(new Interface(), cSIGNATURE_INDEX);
}
