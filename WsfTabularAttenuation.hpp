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

#ifndef WSFTABULARATTENUATION_HPP
#define WSFTABULARATTENUATION_HPP

#include "wsf_export.h"

#include <iosfwd>
#include <string>
#include <vector>

#include "UtCloneablePtr.hpp"
#include "UtTable.hpp"
#include "WsfEM_Attenuation.hpp"

//! An attenuation model defined by externally-created tables.
class WSF_EXPORT WsfTabularAttenuation : public WsfEM_Attenuation
{
   public:
      WsfTabularAttenuation();
      WsfTabularAttenuation(const WsfTabularAttenuation& aSrc);
      WsfTabularAttenuation& operator=(const WsfTabularAttenuation&) = delete;
      WsfEM_Attenuation* Clone() const override;

      static WsfEM_Attenuation* ObjectFactory(const std::string& aTypeName);

      bool Initialize(WsfEM_XmtrRcvr* aXmtrRcvrPtr) override;

      bool ProcessInput(UtInput& aInput) override;

      bool AcceptsInlineBlockInput() const override         { return true; }

      double ComputeAttenuationFactor(WsfEM_Interaction&          aInteraction,
                                              WsfEnvironment&             aEnvironment,
                                              WsfEM_Interaction::Geometry aGeometry) override;

      //! @name Publicly accessible utility methods for processing MODTRAN spectral data.
      //@{

      static void BuildResponseVector(UtTable::Curve&            aReponseCurve,
                                      const std::vector<double>& aWavenumbers,
                                      std::vector<double>&       aResponseVector);

      static double ComputeAverageContrastTransmittance(const std::vector<double>& aWavenumbers,
                                                        const std::vector<double>& aSensorToTargetTransmittances,
                                                        const std::vector<double>& aTargetToBackgroundRadiances,
                                                        const std::vector<double>& aSensorResponse);

      static double ComputeAverageTransmittance(const std::vector<double>& aWavenumbers,
                                                const std::vector<double>& aTransmittances,
                                                const std::vector<double>& aSensorResponse);

      static bool ReadSpectralData(std::istream&        aFile,
                                   double&              aAltitude,
                                   double&              aElevation,
                                   double&              aRange,
                                   std::vector<double>& aWavenumbers,
                                   std::vector<double>& aValues);

      static void ReadSpectralHeaders(std::istream& aFile,
                                      std::string   aHeaders[3]);
      //@}
   private:

      void ProcessQueryCommand(UtInput& aInput);

      void SpectralDataConversion(UtInput& aInput);

      //! The attenuation table
      UtCloneablePtr<UtTable::Table>  mAttenuationPtr;

      //! An adjustment factor that can be applied to the results.
      double                          mAdjustmentFactor;

      //! True if the table provides two-way attenuation.
      bool                            mTwoWayAttenuation;

      //! True if table is a function of frequency
      bool                            mNeedFrequency;

      //! True if table is a function of 'ground_range'
      bool                            mNeedGroundRange;

      //! True if table is a function of 'slant_range'
      bool                            mNeedSlantRange;

};

#endif

