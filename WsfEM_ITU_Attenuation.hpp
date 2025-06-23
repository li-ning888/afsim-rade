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

#ifndef WSFEM_ITU_ATTENUATION_HPP
#define WSFEM_ITU_ATTENUATION_HPP

#include "wsf_export.h"

#include "WsfEM_Attenuation.hpp"

#include <vector>
#include <functional>

#include "UtAtmosphere.hpp"
#include "WsfEM_Types.hpp"

//! An atmospheric attenuation model based on standards from the
//! International Telecommunications Union (ITU) Radio Communication Sector.
//!
//! The following ITU recommendations are used in this model.
//!
//! 1) "Recommendation ITU-R P.676-9, Attenuation by atmospheric gases".
//! 2) "Recommendation ITU-R P.835-4, Reference standard atmospheres".
//! 3) "Recommendation ITU-R P.838-3, Specific attenuation model for rain for use in prediction methods".
//! 4) "Recommendation ITU-R P.840-4, Attenuation due to clouds and fog".
class WSF_EXPORT WsfEM_ITU_Attenuation : public WsfEM_Attenuation
{
    //主要工作流程
    //    1、初始化：

    //    通过构造函数设置大气模型

    //    准备衰减系数表

    //    2、计算衰减：

    //    根据输入参数(频率、高度、仰角等)

    //    生成或更新衰减系数表

    //    沿传播路径积分计算总衰减

    //    3、结果输出：

    //    返回线性衰减因子(0 - 1之间)
   public:
      WsfEM_ITU_Attenuation(const UtAtmosphere& aAtm);
      WsfEM_ITU_Attenuation(const WsfEM_ITU_Attenuation& aSrc);
      WsfEM_ITU_Attenuation& operator=(const WsfEM_ITU_Attenuation& aRhs) = delete;
      WsfEM_Attenuation* Clone() const override;

      static std::function<WsfEM_Attenuation*(const std::string&)> GetObjectFactory(WsfScenario& aScenario);

      bool ProcessInput(UtInput& aInput) override;

      bool AcceptsInlineBlockInput() const override         { return true; }

      // Override of base class method.
      double ComputeAttenuationFactor(WsfEM_Interaction&          aInteraction,
                                              WsfEnvironment&             aEnvironment,
                                              WsfEM_Interaction::Geometry aGeometry) override;

      double ComputeAttenuationFactor(double                    aRange,
                                      double                    aElevation,
                                      double                    aAltitude,
                                      double                    aFrequency,
                                      WsfEM_Types::Polarization aPolarization,
                                      double                    aEarthRadiusMultiplier,
                                      WsfEnvironment&           aEnvironment);

      static double ComputeGasSpecificAttenuation(double aFrequency,
                                                  double aPressure,
                                                  double aTemperature,
                                                  double aWaterVaporDensity);

      static double ComputeCloudSpecificAttenuation(double aFrequency,
                                                    double aTemperature,
                                                    double aWaterDensity);

      static double ComputeRainSpecificAttenuation(double                    aFrequency,
                                                   WsfEM_Types::Polarization aPolarization,
                                                   double                    aRainRate);

      static void PlotGasFigure1();
      static void PlotGasFigure2();
      static void PlotGasFigure3();

      static void PlotCloudFigure1();

   private:

      static void ComputeAtmosphereData(UtAtmosphere& aAtmosphere,
                                        double        aAltitude,
                                        double&       aPressure,
                                        double&       aTemperature,
                                        double&       aWaterVaporDensity);

      void GenerateTable(double                    aFrequency,
                         WsfEM_Types::Polarization aPolarization,
                         WsfEnvironment&           aEnvironment);

      //! The atmosphere for computing pressure, temperature and water vapor density.
      UtAtmosphere                     mAtmosphere;

      //! The frequency at which the tables were last calculated
      double                           mFrequency;

      //! Specific attenuation as a function of altitude.
      struct Point
      {
         double mAltitude;             //!< meters
         double mGamma;                //!< dB/km
      };
      std::vector<Point>               mGammaTable;
};

#endif
