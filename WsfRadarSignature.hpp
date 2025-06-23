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

#ifndef WSFRADARSIGNATURE_HPP
#define WSFRADARSIGNATURE_HPP

#include "wsf_export.h"

class     UtScriptTypes;
#include "WsfEM_Types.hpp"
class     WsfEM_Rcvr;
class     WsfEM_Xmtr;
class     WsfPlatform;
class     WsfScenario;
#include "WsfSignature.hpp"

//! An object that represent the radar signature of a platform.
class WSF_EXPORT WsfRadarSignature : public WsfSignature
{
    //提供雷达特征的基础功能
   public:
      enum
      {
         cSIGNATURE_INDEX = 0          //!< Index of signature within WsfSignatureList (must be unique)
      };

      WsfRadarSignature();
      WsfRadarSignature* Clone() const override = 0;

      // =================================================================================================
      //! Get the radar signature for a given set of conditions.
      //!
      //! @param aStateId      [input] The string ID representing the signature state to be used.
      //! @param aPolarization [input] The polarization of the signal.
      //! @param aFrequency    [input] The frequency of the signal (Hz).
      //! @param aTgtToXmtrAz  [input] The azimuth   of the transmitter with respect to the target.
      //! @param aTgtToXmtrEl  [input] The elevation of the transmitter with respect to the target.
      //! @param aTgtToRcvrAz  [input] The azimuth   of the receiver    with respect to the target.
      //! @param aTgtToRcvrEl  [input] The elevation of the receiver    with respect to the target.
      //! @param aXmtrPtr      [input] Optional pointer to the transmitter
      //! @param aRcvrPtr      [input] Optional pointer to the receiver
      //! @returns The radar cross section (m^2)
      //! 
      //! 
      //! 
      //! RadarSigState() - 获取当前特征状态

      //    SetRadarSigState() - 设置特征状态

      //    RadarSigScaleFactor() - 获取缩放因子

      //    SetRadarSigScaleFactor() - 设置缩放因子

      //    RadarCrossSection(两个重载) - 计算RCS
      virtual float GetSignature(WsfStringId               aStateId,
                                 WsfEM_Types::Polarization aPolarization,
                                 double                    aFrequency,
                                 double                    aTgtToXmtrAz,
                                 double                    aTgtToXmtrEl,
                                 double                    aTgtToRcvrAz,
                                 double                    aTgtToRcvrEl,
                                 WsfEM_Xmtr*               aXmtrPtr = nullptr,
                                 WsfEM_Rcvr*               aRcvrPtr = nullptr) = 0;

      //! @name Methods to support the actual interface on the platform.
      //! These methods provide the interface from the sensor model to the signature.
      //!
      //! The user makes calls like WsfOpticalReflectivity::GetSignature(...).
      //@{
      static WsfStringId GetInputType(WsfPlatform* aPlatformPtr);

      static WsfStringId GetState(WsfPlatform* aPlatformPtr);

      static bool SetState(WsfPlatform* aPlatformPtr,
                           WsfStringId  aState);

      static float GetScaleFactor(WsfPlatform* aPlatformPtr);

      static bool SetScaleFactor(WsfPlatform* aPlatformPtr,
                                 float        aScaleFactor);

      static float GetValue(WsfPlatform*              aPlatformPtr,
                            WsfEM_Types::Polarization aPolarization,
                            double                    aFrequency,
                            double                    aTgtToXmtrAz,
                            double                    aTgtToXmtrEl,
                            double                    aTgtToRcvrAz,
                            double                    aTgtToRcvrEl);

      static float GetValue(WsfPlatform*              aPlatformPtr,
                            WsfEM_Xmtr*               aXmtrPtr,
                            WsfEM_Rcvr*               aRcvrPtr,
                            double                    aTgtToXmtrAz,
                            double                    aTgtToXmtrEl,
                            double                    aTgtToRcvrAz,
                            double                    aTgtToRcvrEl);

      static WsfRadarSignature* GetSignature(WsfPlatform* aPlatformPtr);

      static bool SetSignature(WsfPlatform*       aPlatformPtr,
                               WsfRadarSignature* aSignaturePtr);
      //@}

      static void RegisterScriptMethods(UtScriptTypes& aScriptTypes);
      static void RegisterInterface(WsfScenario& aScenario);
};

#endif
