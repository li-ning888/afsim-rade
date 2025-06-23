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

#ifndef WSFEM_SIMPLEATTENUATION_HPP
#define WSFEM_SIMPLEATTENUATION_HPP

#include "wsf_export.h"

#include "WsfEM_Attenuation.hpp"

//! A trivial attenuation model that is uses a fixed dB/length.
class WSF_EXPORT WsfEM_SimpleAttenuation : public WsfEM_Attenuation
{
   public:
      WsfEM_SimpleAttenuation();
      WsfEM_SimpleAttenuation(const WsfEM_SimpleAttenuation& aSrc);
      WsfEM_SimpleAttenuation& operator=(const WsfEM_SimpleAttenuation& aRhs) = delete;
      WsfEM_Attenuation* Clone() const override;
      ~WsfEM_SimpleAttenuation() override = default;

      static WsfEM_Attenuation* ObjectFactory(const std::string& aTypeName);

      bool ProcessInput(UtInput& aInput) override;

      bool AcceptsInlineBlockInput() const override { return true; }

   private:

      double ComputeAttenuationFactorP(double aRange,
                                       double aElevation,
                                       double aAltitude,
                                       double aFrequency) override;

      //! @name Attenuation values.
      //! The attenuation factor will be used if it is non-zero, otherwise the specific
      //! attenuation will be used.
      //@{
      double       mAttenuationFactor;           //!< absolute
      double       mSpecificAttenuation;         //!< dB/m
      //@}
};

#endif

