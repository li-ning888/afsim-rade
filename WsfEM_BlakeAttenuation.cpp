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

#include "WsfEM_BlakeAttenuation.hpp"

#include <iostream>

#include "UtMath.hpp"

using namespace std;

// statics used when Atmospheric Losses are computed
double WsfEM_BlakeAttenuation::sElevations[6]  = { 0.0, 0.5, 1.0, 2.0, 5.0, 10.0 };
double WsfEM_BlakeAttenuation::sFrequencies[7] = { 0.1e9, 0.2e9, 0.3e9, 0.6e9, 1.0e9, 3.0e9, 10.0e9 };

double WsfEM_BlakeAttenuation::sACoefTable[7][6] = { {  .2739, .1881,  .1605,  .1031, .07371, .04119 },
   {  .6848, .5533,  .4282,  .3193, .2158,  .1017 },
   { 1.199,  .9917,  .7498,  .5186, .3029,  .1522 },
   { 2.210, 1.830,  1.314,   .9499, .4724,  .2512 },
   { 2.758, 2.177,  1.798,  1.168,  .5732,  .3007 },
   { 3.484, 2.592,  1.964,  1.345,  .6478,  .3408 },
   { 4.935, 3.450,  2.601,  1.718,  .9130,  .4420 }
};

double WsfEM_BlakeAttenuation::sBCoefTable[7][6] = { { .008648, .008644, .01106, .01723, .02313, .04076 },
   { .008648, .008644, .01104, .01374, .02213, .04886 },
   { .006837, .008795, .01110, .01474, .03116, .05360 },
   { .008499, .009737, .01221, .01623, .03677, .07204 },
   { .01030,  .01223,  .01163, .01831, .03927, .08056 },
   { .009745, .01225,  .01455, .02055, .04500, .08280 },
   { .00999,  .01340,  .01620, .02240, .03750, .08470 }
};

// =================================================================================================
//•	功能 : 提供默认构造函数、拷贝构造函数和析构函数。
//•	设计 : 继承自 WsfEM_Attenuation，拷贝构造函数调用了基类的拷贝构造函数。
WsfEM_BlakeAttenuation::WsfEM_BlakeAttenuation()
{
}

// =================================================================================================
WsfEM_BlakeAttenuation::~WsfEM_BlakeAttenuation()
{
}

// =================================================================================================
WsfEM_BlakeAttenuation::WsfEM_BlakeAttenuation(const WsfEM_BlakeAttenuation& aSrc)
   : WsfEM_Attenuation(aSrc)
{
}

// =================================================================================================
//! Factory method called by WsfEM_AttenuationTypes.
//•	功能: 根据类型名称创建 WsfEM_BlakeAttenuation 对象。
//•	设计 : 静态方法，支持多种类型名称（如 "WSF_BLAKE_ATTENUATION" 和 "blake"）。
WsfEM_Attenuation* WsfEM_BlakeAttenuation::ObjectFactory(const std::string& aTypeName)
{
   WsfEM_Attenuation* instancePtr = nullptr;
   if ((aTypeName == "WSF_BLAKE_ATTENUATION") ||
       (aTypeName == "blake"))
   {
      instancePtr = new WsfEM_BlakeAttenuation();
   }
   return instancePtr;
}

// =================================================================================================
// •	功能: 创建当前对象的深拷贝。
//•	    设计 : 使用拷贝构造函数实现。
WsfEM_Attenuation* WsfEM_BlakeAttenuation::Clone() const
{
   return new WsfEM_BlakeAttenuation(*this);
}

// =================================================================================================
//protected virtual
//•	功能 : 计算信号的衰减因子。
//•	逻辑 :
//1.	调用 Calculate2WayAttenuation 计算双向衰减。
//2.	返回衰减因子的平方根。
//•	注意 : 该方法忽略了高度参数 aAltitude。
double WsfEM_BlakeAttenuation::ComputeAttenuationFactorP(double aRange,
                                                         double aElevation,
                                                         double aAltitude,
                                                         double aFrequency)
{
   // note: The altitude is assumed to be zero and is not used.
   return sqrt(Calculate2WayAttenuation(aRange, aElevation, aFrequency));
}

// =================================================================================================
//protected
//•	功能 : 计算信号的双向衰减。
//•	逻辑 :
//1.	将输入参数（距离、仰角、频率）限制在有效范围内。
//2.	使用 SearchForValue 方法查找频率和仰角的插值索引和比例。
//3.	从系数表中获取衰减系数，并使用插值法计算衰减。
//4.	返回双向衰减的倒数。
double WsfEM_BlakeAttenuation::Calculate2WayAttenuation(double aRange,
                                                        double aElevation,
                                                        double aFrequency)
{
   double frequency;
   double elevation;
   double freqFrac = 0.0;
   double elFrac = 0.0;
   int freqIndex = 0;
   int elIndex = 0;

   double lossDB[2][2] = { { 0.0, 0.0 },
      { 0.0, 0.0 }
   };

   double loss[2][2] = { { 0.0, 0.0 },
      { 0.0, 0.0 }
   };

   // convert range from meters to nm
   double rangeNM = aRange / UtMath::cM_PER_NM;

   // limit range to 300 nm, data is flat beyond this point
   if (rangeNM > 300.0)
   {
      rangeNM = 300.0;
   }

   // convert Elevation into degrees
   elevation = aElevation * UtMath::cDEG_PER_RAD;

   // Frequency is already in hertz
   frequency = aFrequency;

   if (frequency < 0.1E9)
   {
      frequency = 0.10001E9;
   }
   else if (frequency > 10.0E+9)
   {
      frequency  = 9.99999E9;
   }

   SearchForValue(frequency, sFrequencies, 7, freqIndex, freqFrac);

   if (elevation <= sElevations[0])
   {
      elevation = sElevations[0];
   }
   else if (elevation >= sElevations[5])
   {
      elevation = sElevations[5];
   }

   SearchForValue(elevation, sElevations, 6, elIndex, elFrac);

   // Set-up the coefficient matrices to be used in the interpolation
   double Acoef[2][2] = { { sACoefTable[freqIndex][elIndex],   sACoefTable[freqIndex][elIndex + 1] },
      { sACoefTable[freqIndex + 1][elIndex], sACoefTable[freqIndex + 1][elIndex + 1] }
   };

   double Bcoef[2][2] = { { sBCoefTable[freqIndex][elIndex],   sBCoefTable[freqIndex][elIndex + 1] },
      { sBCoefTable[freqIndex + 1][elIndex], sBCoefTable[freqIndex + 1][elIndex + 1] }
   };

   for (int i = 0; i < 2; ++i)
   {
      for (int j = 0; j < 2; ++j)
      {
         lossDB[i][j] = Acoef[i][j] * (1.0 - exp(-Bcoef[i][j] * rangeNM));
         loss[i][j] = UtMath::DB_ToLinear(lossDB[i][j]);
      }
   }

   double x1 = loss[0][0] + elFrac * (loss[0][1] - loss[0][0]);
   double x2 = loss[1][0] + elFrac * (loss[1][1] - loss[1][0]);

   double loss2Way = x1 + freqFrac * (x2 - x1);

   return 1.0 / loss2Way;
}

// =================================================================================================
//•	功能 : 在给定的表中查找值的插值索引和比例。
//•	逻辑 :
//1.	使用二分查找法定位值所在的区间。
//2.	计算值在区间内的比例。
void WsfEM_BlakeAttenuation::SearchForValue(double  aVal,
                                            double  aTable[],
                                            int     aLength,
                                            int&    aLowIndex,
                                            double& aFrac)
{
   int lo = 0;
   int hi = aLength - 1;

   while (hi > lo + 1)
   {
      int mid = (lo + hi) / 2;

      if (aVal >= aTable[mid])
      {
         lo = mid;
      }
      else
      {
         hi = mid;
      }
   }

   aLowIndex = lo;
   aFrac = (aVal - aTable[lo]) / (aTable[hi] - aTable[lo]);
}
