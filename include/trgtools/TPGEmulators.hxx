#ifndef TRGTOOLS_TPGEmulators_CPP_
#define TRGTOOLS_TPGEmulators_CPP_

namespace dunedaq::trgtools
{

template <typename T>
TPGEmulators<T>::TPGEmulators() {
  // default configuration 
  std::string name = "AVXFrugalPedestalSubtractProcessor"; 
  nlohmann::json json_pedsub;
  json_pedsub["accum_limit"] = 10;
  m_tpg_configs.push_back(std::make_pair(name, json_pedsub));
  //name = "AVXAbsRunSumProcessor";
  name = "AVXThresholdProcessor";
  nlohmann::json json;
  json["plane0"] = 500; 
  json["plane1"] = 500;
  json["plane2"] = 500;
  m_tpg_configs.push_back(std::make_pair(name, json));
};

template <typename T>
void TPGEmulators<T>::configure(nlohmann::json config) {
  // Extract offline channel map name from json configuration
  m_offline_channel_map_name = config["offline_channel_map"][0];
  fmt::print("Using offline channel map: {}\n", m_offline_channel_map_name);

  nlohmann::json tpg_configs_json = config["tpg_config"][0];
  m_tpg_configs.clear();
  for (const auto& it : tpg_configs_json.items()) {
    m_tpg_configs.push_back(std::make_pair(it.key(), it.value()));
  }
  std::cout << "DBG json config: " << m_tpg_configs << "\n";
  std::cout << "DBG tpg pipeline 0: " << m_tpg_configs[0].first << "\n";
  std::cout << "DBG tpg pipeline 1: " << m_tpg_configs[1].first << "\n";

}

template <typename T>
std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> TPGEmulators<T>::emulate_using(nlohmann::json config, const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>>& records) {

  using generator_t = T;
  std::unique_ptr<TPGEmulators<generator_t>> tes = std::make_unique<TPGEmulators<generator_t>>();
  tes->configure(config);
  return tes->emulate_using(records);
}

template <typename T>
std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> TPGEmulators<T>::emulate_using(const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>>& records) {

  std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> res;

  for (const auto& record : records) {

    std::vector<trgdataformats::TriggerPrimitive> output_buffer;
    std::vector<trgdataformats::TriggerPrimitive> temp_buffer;

    auto& fragments = record->get_fragments_ref();

    for (const auto& fragment : fragments) {
      if (fragment->get_data_size() == 0) {
         continue;
      }
      if (fragment->get_fragment_type() != dunedaq::daqdataformats::FragmentType::kWIBEth) {
        continue;
      }

      temp_buffer = emulate_from(fragment);

      if (temp_buffer.size() != 0) {
        output_buffer.insert(output_buffer.end(), temp_buffer.begin(), temp_buffer.end());
        temp_buffer.clear();
      }
    }

    fmt::print("DBG found tps using {}, {}: \n", record->get_header_ref().get_trigger_number(), output_buffer.size());
    res.insert({record->get_header_ref().get_trigger_number(), output_buffer});

  }

  return res;
}

template <typename T>
std::vector<trgdataformats::TriggerPrimitive> TPGEmulators<T>::emulate_from(const std::unique_ptr<daqdataformats::Fragment>& fragment) {

  // Create the output.
  std::vector<trgdataformats::TriggerPrimitive> output_buffer;
  std::vector<trgdataformats::TriggerPrimitive> temp_buffer;

  std::unique_ptr<T> tp_generator = get_tp_generator(fragment);


  int16_t num_frames = fragment->get_data_size() / sizeof(dunedaq::fddetdataformats::WIBEthFrame);

  //std::cout << "DBG TPG NUM frames " << num_frames << "\n";
  for (int16_t ifr = 0; ifr < num_frames; ifr++) {

    auto fp = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data() + ifr * sizeof(dunedaq::fddetdataformats::WIBEthFrame));


    auto wfptr = reinterpret_cast<dunedaq::fddetdataformats::WIBEthFrame*>((uint8_t*)fp); // NOLINT

    std::vector<trgdataformats::TriggerPrimitive> temp_buffer = (*tp_generator)(wfptr);


    if (temp_buffer.size() != 0) {
      //fmt::print("DETAIL number of tps {}\n", temp_buffer.size());
      output_buffer.insert(output_buffer.end(), temp_buffer.begin(), temp_buffer.end());
      temp_buffer.clear();
    }

  }

  return output_buffer;

}

template <typename T>
std::unique_ptr<T> TPGEmulators<T>::get_tp_generator(const std::unique_ptr<daqdataformats::Fragment>& fragment) {

    fddetdataformats::WIBEthFrame* wib_array = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data());

    auto det_id = wib_array->daq_header.det_id;
    auto crate_id = wib_array->daq_header.crate_id;
    auto slot_id = wib_array->daq_header.slot_id;
    auto stream_id = wib_array->daq_header.stream_id;

    std::shared_ptr<detchannelmaps::TPCChannelMap> offline_channel_map =
            dunedaq::detchannelmaps::make_tpc_map(m_offline_channel_map_name);
    std::vector<std::pair<trgdataformats::channel_t, int16_t>> channel_plane_numbers;
    channel_plane_numbers.reserve(TICKS_PER_FRAME);

    std::vector<uint16_t> pedestals;
    pedestals.reserve(TICKS_PER_FRAME);

    for (int chan = 0; chan < TICKS_PER_FRAME; chan++) {
      trgdataformats::channel_t off_channel = offline_channel_map->get_offline_channel_from_det_crate_slot_stream_chan(det_id, crate_id, slot_id, stream_id, chan);
      int16_t plane = offline_channel_map->get_plane_from_offline_channel(off_channel);
      channel_plane_numbers.push_back(std::make_pair(off_channel, plane));
      pedestals.push_back(wib_array->get_adc(chan, 0));
    }

    for (auto& name_config : m_tpg_configs) {
      if (name_config.second.contains("pedestals")) {
        name_config.second["pedestals"] = pedestals;
      }
    }

    std::unique_ptr<T> tp_generator = std::make_unique<T>();
    tp_generator->configure(m_tpg_configs, channel_plane_numbers, SAMPLE_TICK);

    return tp_generator;
}


};

#endif
