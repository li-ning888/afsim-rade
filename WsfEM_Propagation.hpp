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

#ifndef WSFEM_PROPAGATION_HPP
#define WSFEM_PROPAGATION_HPP

#include "wsf_export.h"

class     UtInput;
class     WsfEM_Interaction;
class     WsfEM_Xmtr;
class     WsfEM_XmtrRcvr;
class     WsfEnvironment;
#include "WsfObject.hpp"

//! An abstract base class for computing pattern propagation factors.
class WSF_EXPORT WsfEM_Propagation : public WsfObject
{
   public:

      WsfEM_Propagation();
      ~WsfEM_Propagation() override = default;

      WsfEM_Propagation* Clone() const override = 0;

      const char* GetScriptClassName() const override      { return "WsfEM_Propagation"; }

      //! Is 'debug' enabled?
      bool DebugEnabled() const                            { return mDebugEnabled; }

      //! Compute the propagation factor for the current interaction.
      //! @param aInteraction The current interaction.
      //! @param aEnvironment The environment (sea state, land formation, land coverage).
      //! @return The pattern propagation factor (F40) as a linear value (not dB).
      virtual double ComputePropagationFactor(WsfEM_Interaction& aInteraction,
                                              WsfEnvironment&    aEnvironment) = 0;

      virtual bool Initialize(WsfEM_XmtrRcvr* aXmtrRcvrPtr);

      virtual bool Initialize(WsfEM_Xmtr* aXmtrPtr);

      bool ProcessInput(UtInput& aInput) override;

      //! Is the object a 'null' (no-effect) propagation model?
      //! No operational model needs to worry about this method.
      //! @note See the code in WsfEM_PropagationTypes as to why this is present.
      virtual bool IsNullModel() const                     { return false; }


   protected:
      WsfEM_Propagation(const WsfEM_Propagation& aSrc);

      bool             mDebugEnabled;

   private:

      //! Assignment operator declared but not defined to prevent use.
      WsfEM_Propagation& operator=(const WsfEM_Propagation& aRhs) = delete;
};

#endif
