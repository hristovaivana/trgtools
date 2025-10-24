#ifndef TRGTOOLS_TPGEMULATOR_HPP_
#define TRGTOOLS_TPGEMULATOR_HPP_


#include "trgdataformats/TriggerPrimitive.hpp"
#include "fddetdataformats/WIBEthFrame.hpp"
#include "detchannelmaps/TPCChannelMap.hpp"
#include "tpglibs/TPGenerator.hpp"
#include "daqdataformats/Fragment.hpp"


#include "tpglibs/NaiveTPGenerator.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

namespace dunedaq::trgtools
{

//template <typename T>
class TPGEmulator
{
  public:
    TPGEmulator();
    ~TPGEmulator() = default;

    void configure(nlohmann::json config);

    std::vector<std::pair<std::string, nlohmann::json>> get_configs() { return m_tpg_configs; };
    void set_configs(std::vector<std::pair<std::string, nlohmann::json>>& tpg_configs) { m_tpg_configs = tpg_configs; }

    //template<typename T, typename U>
    //std::vector<trgdataformats::TriggerPrimitive> emulate_using(const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>& records); 
    /*
    template<typename T>
    std::vector<trgdataformats::TriggerPrimitive> emulate_from(const std::unique_ptr<T>& tp_generator, const std::unique_ptr<daqdataformats::Fragment>& fragment);
    */
    std::vector<trgdataformats::TriggerPrimitive> emulate_from(const std::unique_ptr<daqdataformats::Fragment>& fragment); 
    std::vector<trgdataformats::TriggerPrimitive> emulate_from_naive(const std::unique_ptr<daqdataformats::Fragment>& fragment); 



    //std::vector<trgdataformats::TriggerPrimitive> emulate_vector(const std::vector<fddetdataformats::WIBEthFrame*>&& wibs);
    std::vector<trgdataformats::TriggerPrimitive> emulate_vector(std::vector<std::unique_ptr<fddetdataformats::WIBEthFrame*>>& wibs);

    void emulate_raw(const fddetdataformats::WIBEthFrame& wib, std::vector<trgdataformats::TriggerPrimitive>& tps);

    
    std::unique_ptr<tpglibs::NaiveTPGenerator> get_tp_generator_naive(const std::unique_ptr<daqdataformats::Fragment>& fragment);
    std::unique_ptr<tpglibs::TPGenerator> get_tp_generator(const std::unique_ptr<daqdataformats::Fragment>& fragment);
    //std::vector<std::unique_ptr<tpglibs::TPGenerator>> get_tp_generators(const auto& fragments);
    std::vector<std::unique_ptr<tpglibs::TPGenerator>> get_tp_generators(const std::vector<daqdataformats::Fragment>& fragments);

    void set_tp_generator(std::unique_ptr<tpglibs::TPGenerator>& tp_generator) { m_tp_generator = std::move(tp_generator); } 
    std::unique_ptr<tpglibs::TPGenerator> m_tp_generator;
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

};



#endif
