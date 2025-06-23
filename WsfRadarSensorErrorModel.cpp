// ****************************************************************************
// UNCLASSIFIED//FOUO
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
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

#include "WsfRadarSensorErrorModel.hpp"

#include <string>

#include "UtInput.hpp"
#include "UtLog.hpp"

#include "WsfRadarSensor.hpp"
#include "WsfSensorMode.hpp"
#include "WsfSensorResult.hpp"

using namespace std;

namespace wsf
{

    //?	功能 : 创建当前误差模型的副本。
    //?	用途 : 支持误差模型的复制操作，便于在不同传感器实例中复用。
WsfSensorErrorModel* RadarSensorErrorModel::Clone() const
{
   return new RadarSensorErrorModel(*this);
}

//! Factory method for WsfSensorErrorModelTypes.
//! This is called by WsfSensorErrorModelTypes::LoadType to determine if a pattern
//! represented by this class is being requested.
//?	功能 : 根据传入的类型名称创建 RadarSensorErrorModel 实例。
//?	用途 : 支持动态加载和实例化误差模型。
WsfSensorErrorModel* RadarSensorErrorModel::ObjectFactory(const std::string& aTypeName)
{
   RadarSensorErrorModel* instancePtr = nullptr;
   if ((aTypeName == "WSF_RADAR_SENSOR_ERROR") ||
       (aTypeName == "RADAR_SENSOR_ERROR") ||
       (aTypeName == "radar_sensor_error"))
   {
      instancePtr = new RadarSensorErrorModel();
   }
   return instancePtr;
}
//•	功能 : 初始化误差模型。
//•	逻辑 :
//1.	调用基类的 Initialize 方法。
//2.	检查传感器是否为 WsfRadarSensor 类型。
//3.	如果不是雷达传感器，记录错误日志并返回 false。
bool RadarSensorErrorModel::Initialize(WsfSensorMode* aSensorModePtr)
{
   bool ok = WsfSensorErrorModel::Initialize(aSensorModePtr);
   if (dynamic_cast<WsfRadarSensor*>(aSensorModePtr->GetSensor()) == nullptr)
   {
      auto out = ut::log::error() << "Unable to use 'error_model' on sensor types that are note WSF_RADAR_SENSORs.";
      out.AddNote() << "Model: " << GetName();
      ok = false;
   }
   return ok;
}
//•	功能 : 计算雷达传感器的测量误差。
// 计算测量误差，具体包括方位角、俯仰角、距离和速度的误差。
void RadarSensorErrorModel::GetSphericalErrorStdDev(ut::Random&      aRandom,
                                                    WsfSensorResult& aResult,
                                                    Sigmas&          aSigmas,
                                                    Errors&          aErrors)
{
   // Compute the measurement errors using techniques that are specific to a radar sensor.
   
   int n = 1; // Could be > 1 for non-coherent integration....
   //1.	信噪比计算:
    //根据信噪比（SNR）计算误差缩放因子。
   double temp = sqrt(2.0 * n * aResult.mSignalToNoise);

   WsfSensorMode* snsrModePtr = GetSensorMode();
   WsfRadarSensor::RadarBeam* beamPtr = dynamic_cast<WsfRadarSensor::RadarBeam*>(snsrModePtr->GetBeamEntry(aResult.mBeamIndex));
   WsfEM_Rcvr* rcvrPtr = aResult.GetReceiver();
   WsfEM_Xmtr* xmtrPtr = aResult.GetTransmitter();

  //2.	方位角误差计算:
   //-如果波束的方位角宽度未定义，则从接收机获取默认值。
   //根据信噪比计算方位角误差的标准差。
   double azBeamwidth = beamPtr->mErrorModelAzBeamwidth;
   if (azBeamwidth < 0.0)
   {
      azBeamwidth = rcvrPtr->GetAzimuthBeamwidth();
   }
   double azErrorSigma = azBeamwidth / temp;

   //3.	俯仰角误差计算:
   //类似方位角误差的计算逻辑。
   double elBeamwidth = beamPtr->mErrorModelElBeamwidth;
   if (elBeamwidth < 0.0)
   {
      elBeamwidth = rcvrPtr->GetElevationBeamwidth();
   }
   double elErrorSigma = elBeamwidth / temp;

   // 4.	距离误差计算:
/*   -根据脉冲宽度和信噪比计算距离误差的标准差。
       - 如果脉冲宽度未定义，则尝试从接收机的带宽或发射机的脉冲压缩比中推导*/
   double pulseWidth = beamPtr->mErrorModelPulseWidth;
   if (pulseWidth < 0.0)
   {
      pulseWidth = xmtrPtr->GetPulseWidth();
      if (pulseWidth > 0.0)
      {
         // Pulse width is defined.
      }
      else if (rcvrPtr->GetBandwidth() > 0.0)
      {
         // Assume a matched filter.
         pulseWidth = 1.0 / rcvrPtr->GetBandwidth();
      }
      else
      {
         pulseWidth = 0.0;
      }
      // Account for processing gains due to pulse compression.
      pulseWidth /= xmtrPtr->GetPulseCompressionRatio();
   }
   double rangeErrorSigma = 0.0;
   if (pulseWidth > 0.0)
   {
      rangeErrorSigma = (pulseWidth * UtMath::cLIGHT_SPEED) / (2.0 * temp);
   }

   //5.	速度误差计算:
  // - 根据多普勒分辨率和信噪比计算速度误差的标准差。
   double dopplerResolution = beamPtr->mErrorModelDopplerResolution;
   if (dopplerResolution < 0.0)
   {
      dopplerResolution = beamPtr->GetDopplerResolution();
   }
   double rangeRateErrorSigma = 0.0;
   if (dopplerResolution > 0.0)
   {
      rangeRateErrorSigma = dopplerResolution / (2.0 * temp);
   }
  /* 6.	误差赋值:
   •	将计算的误差值赋给 aErrors，并通过高斯分布生成随机误差值。*/
   WsfMeasurement& measurement = aResult.mMeasurement;

   measurement.SetSensorAzimuthError(0.0);
   if ((snsrModePtr->ReportsBearing() || snsrModePtr->ReportsLocation()) &&
       (azErrorSigma != 0.0))
   {
      measurement.SetSensorAzimuthError(azErrorSigma);
      *aErrors.mAzError = aRandom.Gaussian() * azErrorSigma;
   }
   measurement.SetSensorElevationError(0.0);
   if ((snsrModePtr->ReportsElevation() || snsrModePtr->ReportsLocation()) &&
       (elErrorSigma != 0.0))
   {
      measurement.SetSensorElevationError(elErrorSigma);
      *aErrors.mElError = aRandom.Gaussian() * elErrorSigma;
   }
   measurement.SetRangeError(0.0);
   if ((snsrModePtr->ReportsRange() || snsrModePtr->ReportsLocation()) &&
       (rangeErrorSigma != 0.0))
   {
      measurement.SetRangeError(rangeErrorSigma);
      *aErrors.mRangeError = aRandom.Gaussian() * rangeErrorSigma;
   }
   measurement.SetRangeRateError(0.0);
   if (snsrModePtr->ReportsRangeRate() &&
       (rangeRateErrorSigma != 0.0))
   {
      measurement.SetRangeRateError(rangeRateErrorSigma);
      *aErrors.mRangeRateError = aRandom.Gaussian() * rangeRateErrorSigma;
   }
}

}
