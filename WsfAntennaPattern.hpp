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

#ifndef WSFANTENNAPATTERN_HPP
#define WSFANTENNAPATTERN_HPP

#include <memory>
#include <mutex>

#include "wsf_export.h"

#include "TblLookup.hpp"
class     UtInput;
#include "UtReferenceTracked.hpp"

#include "WsfObject.hpp"
class     WsfSimulation;

//! Represents the gain of an antenna as a function of azimuth and elevation.
//!
//! Antenna patterns are used by transmitters and receivers of electromagnetic
//! radiation to compute the gain of the transmitted or received signal.
//!
//! A given antenna pattern object will be shared amongst all objects that
//! utilize the pattern.

class WSF_EXPORT WsfAntennaPattern : public WsfObject
{
   public:
      static const char* cTYPE_KIND;

      class WSF_EXPORT GainAdjustmentTable
      {
         public:
            TblIndVarU<double> mFrequency;
            TblDepVar1<double> mAdjustment;
      };

      class WSF_EXPORT BaseData : public UtReferenceCounted
      {
         public:

            BaseData();
            BaseData(const BaseData& aSrc) = default;
            ~BaseData() override = default;

            virtual bool ProcessInput(WsfAntennaPattern& aPattern,
                                      UtInput&           aInput);
            virtual bool Initialize(WsfAntennaPattern& aAntennaPattern);

            virtual double GetGain(double aFrequency,
                                   double aTargetAz,
                                   double aTargetEl,
                                   double aEBS_Az,
                                   double aEBS_El);

            virtual double PerformGainAdjustment(double aFrequency,
                                                 double aGain);

            virtual void InitializeAverageGain(double aFrequency);

            //! The minimum gain that should be returned by any antenna pattern.
            double                     mMinimumGain;

            //! A simple gain adjustment.
            double                     mGainAdjustment;

            //! Frequency-dependent gain adjustment table.
            GainAdjustmentTable        mGainAdjustmentTable;

            //! Initialization flag for the shared data that is to be initialized only once to speed up
            //! the overall initialization of the simulation.
            bool                       mInitialized;

            //! @name Data used by the GetGainThreshold function.
            //@{
            bool                       mAvgGainInitialized;
            bool                       mShowAvgGain;
            std::recursive_mutex       mAvgGainMutex;
            std::vector<double>        mAvgGain;
            //! The peak gain of the sample. This *SHOULD* be the peak gain of the pattern.
            double                     mSampledPeakGain;
            //@}
      };

      WsfAntennaPattern();
      WsfAntennaPattern(BaseData* aBasePtr);
      ~WsfAntennaPattern() override;

      WsfAntennaPattern* Clone() const override;

      const char* GetScriptClassName() const override { return "WsfAntennaPattern"; }

      virtual double GetGain(double aFrequency,
                             double aTargetAz,
                             double aTargetEl,
                             double aEBS_Az,
                             double aEBS_El);
      virtual double GetAzimuthBeamwidth(double aFrequency) const;
      virtual double GetElevationBeamwidth(double aFrequency) const;
      virtual double GetMinimumGain() const;
      virtual double GetPeakGain(double aFrequency) const;
      virtual const  GainAdjustmentTable& GetGainAdjustmentTable() const;
      virtual double GetGainAdjustment() const;

      virtual bool Initialize(WsfSimulation* aSimulationPtr);

      bool ProcessInput(UtInput& aInput) override;

      virtual double GetGainThresholdFraction(double aGainThreshold,
                                              double aPeakGain,
                                              double aMinAz,
                                              double aMaxAz,
                                              double aFrequency);

      virtual void SetAntennaBeamCount(unsigned int aBeamCount) {};

   protected:

      WsfAntennaPattern(const WsfAntennaPattern& aSrc);
      WsfAntennaPattern& operator=(const WsfAntennaPattern& aRhs);

      //! A shared pointer to the data shared between this and derived classes.
      //! @note Provides a method for sharing data among pattern instances via pointers within each derived class.
      //std::shared_ptr<BaseData>  mSharedDataPtr;

      BaseData* mSharedDataPtr;

};

#endif
