////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "ScopedTimer.hpp"

#include "Logger.hpp"
#include <chrono>
#include <iostream>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::ScopedTimer(std::string const& name)
  : mName(name)
  , mStartTime(getNow())
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::~ScopedTimer()
{
  double now = getNow();
  ILLUSION_DEBUG << mName << ": " << now - mStartTime << " ms " << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double ScopedTimer::getNow()
{
  auto time        = std::chrono::system_clock::now();
  auto since_epoch = time.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count() * 0.001;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
