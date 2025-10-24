#ifndef TRGTOOLS_TRIGGERPRIMITIVE_V4_HXX_
#define TRGTOOLS_TRIGGERPRIMITIVE_V4_HXX_

#include "trgdataformats/TriggerPrimitive.hpp"

namespace dunedaq {
namespace trgtools {

enum class Type
{
  kUnknown = 0,
  kTPC = 1,
  kPDS = 2,
};

enum class Algorithm
{
  kUnknown = 0,
  kSimpleThreshold = 1,
  kAbsRunningSum = 2,
  kRunningSum = 3
};



struct TriggerPrimitive_v4 {

  uint8_t version = { 1 }; // NOLINT(build/unsigned)
  uint64_t time_start = { 0 };
  uint64_t time_peak = { 0 };
  uint64_t time_over_threshold = { 0 };
  int32_t channel = { 0 };
  uint32_t adc_integral = { 0 }; // NOLINT(build/unsigned)
  uint16_t adc_peak = { 0 };     // NOLINT(build/unsigned)
  uint16_t detid = { 0 }; // NOLINT(build/unsigned)
  Type type = Type::kUnknown;
  Algorithm algorithm = Algorithm::kUnknown;  
  uint16_t flag = { 0 };


  void convert(const TriggerPrimitive_v4& tp_v4, trgdataformats::TriggerPrimitive& tp) {
  }
};

}
}
#endif
