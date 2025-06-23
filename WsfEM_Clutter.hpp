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

#ifndef WSFEM_CLUTTER_HPP
#define WSFEM_CLUTTER_HPP

#include "wsf_export.h"

class     UtInput;
class     WsfEM_Interaction;
class     WsfEM_Rcvr;
class     WsfEnvironment;
#include "WsfObject.hpp"

//! An abstract class for computing clutter power.
class WSF_EXPORT WsfEM_Clutter : public WsfObject
{
   public:

      WsfEM_Clutter();
      ~WsfEM_Clutter() override = default;

      WsfEM_Clutter* Clone() const override = 0;

      virtual bool Initialize(WsfEM_Rcvr* aRcvrPtr);

      bool ProcessInput(UtInput& aInput) override;

      //! Is 'debug' enabled?
      bool DebugEnabled() const                            { return mDebugEnabled; }

      //! Compute the clutter power for an interaction.
      //! @param aInteraction      The current interaction.
      //! @param aEnvironment      The environment (sea state, land formation, land coverage).
      //! @param aProcessingFactor The factor to be applied to the raw computed clutter power to produce
      //! the clutter power as seen by the radar AFTER is has performed clutter suppression. This value
      //! (in the range [0..1] represents the ability for the radar to attenuate (suppress) clutter returns.
      //! A value of zero indicates the clutter signal is completely attenuated while a value of one indicates
      //! that none of the clutter signal could be suppressed.
      //!
      //! For the non-table models, the computed clutter power will be multiplied by this value to
      //! produce the return value. For table models the factor has probably already been included
      //! in the table and the value will probably be ignored.
      //!
      //! @return The clutter power (watts).
      virtual double ComputeClutterPower(WsfEM_Interaction& aInteraction,
                                         WsfEnvironment&    aEnvironment,
                                         double             aProcessingFactor) = 0;

      //! Is the object a 'null' (no-effect) clutter model?
      //! No operational model needs to worry about this method.
      //! @note See the code in WsfEM_ClutterTypes as to why this is present.
      virtual bool IsNullModel() const                     { return false; }

   protected:
      WsfEM_Clutter(const WsfEM_Clutter& aSrc);
      WsfEM_Clutter& operator=(const WsfEM_Clutter& aRhs);

   private:

      //static WsfEM_Clutter* CreateInstance(const std::string& aTypeName);
       //•	类型 : bool
           //•	默认值 : false
           //•	功能 : 表示是否启用了调试模式。
          // •	用法 :
       //•	在 ProcessInput 方法中通过 "debug" 命令启用。
          // •	可能会在其他方法中用于输出调试信息（当前代码中未体现）。
      bool mDebugEnabled;
};

#endif
