#ifndef TRGTOOLS_TPGEMULATORS_HPP_
#define TRGTOOLS_TPGEMULATORS_HPP_


#include "trgdataformats/TriggerPrimitive.hpp"
#include "fddetdataformats/WIBEthFrame.hpp"
#include "detchannelmaps/TPCChannelMap.hpp"
#include "daqdataformats/Fragment.hpp"

//#include "tpglibs/TPGenerator.hpp"
//#include "tpglibs/NaiveTPGenerator.hpp"
#include "tpglibs/TPSimulator.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace dunedaq::trgtools
{

template <typename T>
class TPGEmulators
{
  public:
    TPGEmulators();
    ~TPGEmulators() = default;

    //static std::shared_ptr<TPGEmulators<T>> get_instance();

    void configure(nlohmann::json config);

    std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> emulate_using(const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>>& records); 

    std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> emulate_using(nlohmann::json config, const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>>& records);

  protected:
    std::vector<trgdataformats::TriggerPrimitive> emulate_from(const std::unique_ptr<daqdataformats::Fragment>& fragment); 
    std::unique_ptr<T> get_tp_generator(const std::unique_ptr<daqdataformats::Fragment>& fragment);


  private:
    std::vector<std::pair<std::string, nlohmann::json>> m_tpg_configs;
    //std::unique_ptr<tpglibs::TPGenerator> m_tp_generator;

    std::string m_offline_channel_map_name{"PD2HDTPCChannelMap"};
    //std::string m_offline_channel_map_name{"PD2VDTPCChannelMap"};

    //std::string m_offline_channel_map_name{"PD2VDTopTPCChannelMap"};
    //std::string m_offline_channel_map_name{"PD2VDBottomTPCChannelMap"};

    static const uint64_t SAMPLE_TICK = 32;
    static const uint64_t TICKS_PER_FRAME = 64;

};

} // namespace trgtools

#include "trgtools/TPGEmulators.hxx"

#endif
