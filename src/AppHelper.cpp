#ifndef TRGTOOLS_APPHELPER_CPP_
#define TRGTOOLS_APPHELPER_CPP_

#include "trgtools/AppHelper.hpp"



namespace dunedaq::trgtools
{

RootPlotter::RootPlotter() {	
  h1_ch_r->GetYaxis()->SetCanExtend(TH1::kAllAxes);
  h1_ch_s->GetYaxis()->SetCanExtend(TH1::kAllAxes);

  h1_ch_r->SetCanExtend(TH1::kAllAxes); 
  h1_ch_s->SetCanExtend(TH1::kAllAxes);

  h1_ch_r->GetYaxis()->SetCanExtend(kTRUE);
  h1_ch_s->GetYaxis()->SetCanExtend(kTRUE);

};

void RootPlotter::plot_tps(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const std::string& type ) {
  // 3072 * 4 = 12284
  //int max_ch = 12284;
  //TH1F *h1_ch = new TH1F("h1_ch", "This is the channel distribution", max_ch, 0, max_ch);


  // Create two histograms
   TH1F *hpx = new TH1F("hpx", "This is the px distribution", 100, -4, 4);
   TH2F *hpxpy = new TH2F("hpxpy", "py vs px", 40, -4, 4, 40, -4, 4);

   // http server with port 8080, use jobname as top-folder name
   THttpServer *serv = new THttpServer("http:8081");
   //if (!serv) serv = new THttpServer("http:8081");

   // Fill histograms randomly
   TRandom3 random;
   Float_t px, py;

    // Set a timer to call TimerFunction after a delay
    //gSystem->AddTimer(30000, Timer); // Stops after 5 seconds

  TH1* h1_ch;
  TString c1_opt = "";
  bool pause = false;
  if (type == "record") {
    h1_ch_s->SetLineColor(kBlue);
    h1_ch = h1_ch_r;
  } else if (type == "slice") {
    h1_ch_s->SetLineColor(kRed);
    h1_ch = h1_ch_s;
    c1_opt = "SAME";
    pause = true;
  }

      for (auto& tp: tp_buffer) {
	
        h1_ch->Fill(tp.channel);
      } 
      //hs->Add((TH1*)h1_ch->Clone());
      hs->Add(h1_ch);


   gStopProcessing = false; 
   fmt::print("DBG signal {} \n", gStopProcessing);
   signal(SIGINT, signal_callback_handler);
   // press Ctrl-C to stop macro
  while (pause && !gSystem->ProcessEvents() && !gStopProcessing) {
   //if (!gStopProcessing) {
      random.Rannor(px,py);
      hpx->Fill(px);
      hpxpy->Fill(px,py);

      
      c1->cd();
      h1_ch->Draw(c1_opt);
      c1->Modified();
      c1->Update();
      
      cst->cd();
      cst->cd();
      gPad->SetGrid();
      //hs->Draw("nostack,e1p");
      hs->Draw("");
      cst->Modified();
      cst->Update();

      /*
      c1->cd();
      h1_ch->Draw(c1_opt);
      c1->Modified();
      c1->Update();
      */
  }
  //c1->SaveAs("out.png");
  


   //serv->ProcessRequests();
   //serv->SetTimer(0, kTRUE);

  //TGraph* gr = new TGraph(10);
  //gr->SetName("gr1"); 
  //auto serv = new THttpServer("http:8081");
  //serv->Register("graphs/subfolder", gr);
 

  //std::cout << "Press Enter to continue..." << std::endl;
  //std::cin.get();


}


AppHelper::AppHelper() {

  m_rp = new RootPlotter();

};
//AppHelper::~AppHelper() {
  //delete m_rh;
//}


void AppHelper::app_test_trts_info(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file) {
  std::vector<std::string> str{
	    "application_name"
	  , "closing_timestamp"
	  , "creation_timestamp"
	  , "record_type"
	  , "operational_environment"
	  , "offline_data_stream"
	  , "run_was_for_test_purposes"
	  , "filelayout_params"    // json
	  , "source_id_geo_id_map" // json
  };
  std::vector<std::string> num{
	    "file_index"
	  , "recorded_size"
	  , "filelayout_version"
	  , "run_number"
  };
  std::vector<std::string> attrs = input_file->get_attribute_names();
  for (const auto& a : attrs) {
    //fmt::print("INFO attr {} : \n", a);
    if (std::find(std::begin(str), std::end(str), a) != std::end(str)) {
      fmt::print("INFO attr {} : {}\n", a, input_file->get_attribute_if_exists<std::string>(a, "N/A"));
    }
    if (std::find(std::begin(num), std::end(num), a) != std::end(num)) {
      fmt::print("INFO attr {} : {}\n", a, input_file->get_attribute<size_t>(a));
    }
  }
}

std::string AppHelper::dts_to_datetime(uint64_t& ts, std::string s) {
  std::time_t sec = ts*16/1000000000;
  int ns = ts*16%1000000000;
  //std::cout << s << ns << " " << std::asctime(std::localtime(&sec));
  return s + std::to_string(ns) + " " + std::asctime(std::localtime(&sec));

}

//void AppHelper::get_tps(auto& fragments, TH1F *h1_ch, TString c1_opt) {
void AppHelper::get_tps(auto& fragments) {

  //std::vector<trgdataformats::TriggerPrimitive> tp_buffer;	

  // can be reused for raw files too 
  int n_frags = 0;
  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive) {
      continue;
    } else {
      if (n_frags > 0) break; // test
      BYTES_PER_TP = sizeof(dunedaq::trgdataformats::TriggerPrimitive);

      uint64_t begin = fragment->get_header().window_begin;
      uint64_t end = fragment->get_header().window_end;
      fmt::print("DETAIL -- window [ {} : {} ]\n", begin, end);
      int num_tps = fragment->get_data_size() / BYTES_PER_TP;
      if (num_tps == 0) continue;
      fmt::print("DETAIL -- tp size {}\n", BYTES_PER_TP);
      fmt::print("DETAIL -- tps {}\n", num_tps);

      trgdataformats::TriggerPrimitive* tp_array = static_cast<trgdataformats::TriggerPrimitive*>(fragment->get_data());
      for(size_t tpid(0); tpid<num_tps; ++tpid) {
          auto& tp = tp_array[tpid];

      //for (int itp = 0; itp < num_tps; itp++) {
      //  auto tp = reinterpret_cast<dunedaq::trgdataformats::TriggerPrimitive*>(
      //    static_cast<char*>(fragment->get_data()) + itp * BYTES_PER_TP);

	m_tp_buffer.push_back(tp);
	if (tpid < 10) {
	  uint64_t ts = tp.time_start;  
          int channel = (int)tp.channel;
	  std::cout << "TP INFO " << channel << ", " << dts_to_datetime(ts, "(ts).");
        }
      }
    }
    n_frags++;
  }
  fmt::print("DETAIL -- number of TP fragments {}\n", n_frags);
}

void AppHelper::get_valid_sourceids(auto& fragments) {

  std::vector<daqdataformats::SourceID> ret;
  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive && fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
      continue;
    }

    daqdataformats::SourceID sourceid = fragment->get_element_id();

    ret.push_back(sourceid);
  }

  //uint64_t trn = record->get_header_ref().get_trigger_number();
  //m_record_sourceids[trn].reserve(m_record_sourceids[trn].size() + ret.size());
  //m_record_sourceids[trn].insert(m_record_sourceids[trn].end(), std::make_move_iterator(ret.begin()), std::make_move_iterator(ret.end()));

  fmt::print("INFO sourceids \n");
  for (auto& it: ret) {
    fmt::print("{}  ", it.id);
  }
  fmt::print("\n");

}
void AppHelper::app_test_trts(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file, bool a, bool v, bool vv) {

  if (vv) app_test_trts_info(input_file);

  auto records = input_file->get_all_record_ids();
  auto first_rec = *(records.begin());
  auto all_rh_paths = input_file->get_record_header_dataset_paths();

  int rec_first = 0, rec_last = 0, rec_second = 0, rec_gap = 1;
  if (input_file->is_trigger_record_type()) {
    auto trh_ptr = input_file->get_trh_ptr(first_rec);
    rec_first = trh_ptr->get_header().trigger_number;
    rec_last = input_file->get_trh_ptr(all_rh_paths.back())->get_header().trigger_number;
    // gap
    for (const auto& record : records) {
      rec_second++;
      if (rec_second == 2) {
        auto trh_ptr = input_file->get_trh_ptr(record);
        rec_second = trh_ptr->get_header().trigger_number;
        rec_gap = rec_second - rec_first;
        break;
      }
    }
  } else if (input_file->is_timeslice_type()) {
    auto tsh_ptr = input_file->get_tsh_ptr(first_rec);
    rec_first = (*tsh_ptr).timeslice_number;
    rec_last = (*(input_file->get_tsh_ptr(all_rh_paths.back()))).timeslice_number;
  }

  size_t num_records = (rec_last - rec_first)/rec_gap + 1;
  if (v) {
    fmt::print("Number of records {} vs {}: [ {} : {} ], gap: {}\n", num_records, all_rh_paths.size(), rec_first, rec_last, rec_gap);
  }

  if (a) {
    assert(num_records == all_rh_paths.size() && "checking number of records");
  }

}


void AppHelper::parse_app(CLI::App& _app, Options& _opts)
{
  _app.add_option("-i,--input-files", _opts.input_files, "List of input files (required)")
    ->required()
    ->check(CLI::ExistingFile); // Validate that each file exists

  _app.add_option("-r,--input-records", _opts.input_records, "List of input trigger records to process");
  _app.add_option("-s,--input-slices", _opts.input_slices, "List of input time slices to process");
  //_app.add_option("-b,--bytes-per-tp", _opts.bytes_per_tp, "Size of trigger primitive in bytes");

  _app.add_option("-e, --helper", _opts.helper, "Select helper, e.g. 0, 1, ...");
  _app.add_option("--ee", _opts.helpers, "Select helpers, e.g. 0 1 ...");

  _app.add_flag("-a, --assert", _opts.assert, "Don't crash on assert failure.");
  _app.add_flag("-v, --verbose", _opts.verbose, "Don't print outputs.");
  _app.add_flag("--vv", _opts.verbose2, "Don't print more outputs.");

}  

void AppHelper::config_app(Options& _opts) {

  if (_opts.verbose) {

    fmt::print("{:-<77}\n", "");
    fmt::print("{}\n", _opts);
    //std::cout << _opts;

    fmt::print("{:-<77}\n", "");
    fmt::print("Input files to process:\n");
    for (const std::string& file : _opts.input_files) {
      fmt::print("- {}\n", file);
    }
    fmt::print("Number of input files: {}\n", _opts.input_files.size());
    fmt::print("{:-<77}\n", "");
  }

  for (auto& input_file : _opts.input_files) {
    m_input_files.push_back(std::make_shared<hdf5libs::HDF5RawDataFile>(input_file));
  }

  for (const auto& input_file : m_input_files) {
    app_test_trts(input_file, _opts.assert, _opts.verbose, _opts.verbose2);
    if (input_file->is_trigger_record_type()) {
      m_input_files_raw.push_back(input_file);
    } else if (input_file->is_timeslice_type()) {
      m_input_files_tp.push_back(input_file);
    } else {
      fmt::print("ERROR File has unknown record type.\n"); 
    }
  }

  //helpers[0] = &AppHelper::helper_0;

  //helper_0();
  m_helper = std::make_unique<Helper>(_opts);
  //(this->*helpers[_opts.helper])();
  for (const auto& h : _opts.helpers) {
    m_helper->start_message(h);
    (this->*helpers[h])();
  }
}

// -------------------------------------------------------------------------
void AppHelper::helper_1_0(const auto& fragment, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames) {
  assert(fragment->get_fragment_type() == daqdataformats::FragmentType::kWIBEth && "WIBEth fragment check");

  n_frames = fragment->get_data_size()/SIZE_WIB_FRAME;
  //if (m_helper->m_opts.verbose) {
  //  fmt::print("  WIBEth fragment size: {}\n", fragment->get_data_size());
  //  fmt::print("  Num WIB frames: {}\n", n_frames);
  //}
  fddetdataformats::WIBEthFrame* wib_array = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data());
  first_ts = wib_array[0].get_timestamp();
  last_ts = wib_array[n_frames-1].get_timestamp();
  uint64_t ts_diff = last_ts - first_ts;
  uint64_t last_ts_expected = first_ts + SAMPLE_TICK * TICKS_PER_FRAME * (n_frames - 1); 
  uint64_t ts_diff_expected = last_ts_expected - first_ts;
  //if (m_helper->m_opts.verbose) {
  //  fmt::print("INFO timestamp check: {} vs {}, diff = 0 vs {}\n", ts_diff, ts_diff_expected, ts_diff-ts_diff_expected);
  //}
  assert(ts_diff-ts_diff_expected == 0 && "timestamp check");
}
void AppHelper::helper_1_1(auto& fragments, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames) {
  if (m_helper->m_opts.verbose) fmt::print("DETAIL -- number of fragments {}\n", fragments.size());

  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
      continue;
    } else {
        helper_1_0(fragment, first_ts, last_ts, n_frames);
        break;
    }
  }
}
void AppHelper::helper_1_2(auto& record, uint64_t& first_ts, uint64_t& last_ts, uint64_t& n_frames) {

    last_ts += SAMPLE_TICK * TICKS_PER_FRAME;
    uint64_t last_ts_expected = first_ts + n_frames * SAMPLE_TICK * TICKS_PER_FRAME;
    if (m_helper->m_opts.verbose) {
      //fmt::print("INFO n_frags: {} vs {}\n", n_frames, fragments.size());
      fmt::print("INFO check last timestamp: {} vs {}\n", last_ts, last_ts_expected);
    }
    assert(last_ts == last_ts_expected && "quick timestamp check");


    uint64_t ts_window = last_ts - first_ts;
    if (m_helper->m_opts.verbose) {
      fmt::print("INFO trigger record window: {}\n", ts_window); 
      fmt::print("INFO first timestamp: {}\n", first_ts); 
      fmt::print("INFO last timestamp: {}\n", last_ts); 
    }

    std::vector<uint64_t> v = {record->get_header_ref().get_trigger_number(), first_ts, last_ts};
    m_record_window.push_back(v);

    //uint64_t tt = record->get_header_ref().get_trigger_timestamp();
    //assert(tt >= first_ts && tt <= last_ts && "check record header timestamp");

}
// -------------------------------------------------------------------------
void AppHelper::helper_0() {

  //auto records = m_input_files.front()->get_all_record_ids();
  //std::vector<daqdataformats::TriggerRecord> record_buffer;
  //std::vector<std::unique_ptr<daqdataformats::TriggerRecord>> record_buffer;
  // eventually separate into m_input_files_raw and m_input_files_tp
  //for (auto& input_file: m_input_files) {
  for (auto& input_file: m_input_files_raw) {

    auto records = input_file->get_all_record_ids();
    fmt::print("INFO records: {}\n", records.size());
    fmt::print("INFO records to process: {}\n", m_helper->m_opts.input_records.size());
    for (const auto& record : records) {


      if (m_helper->m_opts.input_records.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_records.begin(), m_helper->m_opts.input_records.end(), record.first) == m_helper->m_opts.input_records.end()) {
          continue;
        }
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL trigger record number {}\n", record.first);

      m_record_buffer.push_back(std::make_unique<daqdataformats::TriggerRecord>(input_file->get_trigger_record(record)));


      daqdataformats::TriggerRecord trigger_record = input_file->get_trigger_record(record);      
      auto& fragments = trigger_record.get_fragments_ref(); 
      get_tps(fragments);
      m_rp->plot_tps(m_tp_buffer, "record");
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments {}\n", fragments.size());
	//get_valid_sourceids(fragments);
      }

    }

  }


}


void AppHelper::helper_1() {
  // DETAILED TIMESTAMP CHECK - check each fragment separately 
  /* for each trigger record get
   * 1. start timestamp
   * 2. end timestamp
   * 3. check for missing timestamps
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
  for (const auto& record : m_record_buffer) {
    auto& fragments = record->get_fragments_ref();
    int n_frags = fragments.size();
    size_t n_frames;
    if (m_helper->m_opts.verbose) fmt::print("DETAIL -- number of fragments {}\n", fragments.size());


    //const auto& first_fragment = fragments.front();
    //const auto& last_fragment = fragments.back();
    //helper_1_0(first_fragment);
    //helper_1_0(last_fragment);
    bool first = false;
    uint64_t first_ts;
    uint64_t last_ts;
    for (const auto& fragment : fragments) {
      if (fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
        continue;
      } else {
	if (!first) {
	  first = true;
	  helper_1_0(fragment, first_ts, last_ts, n_frames);
        }
	helper_1_0(fragment, first_ts, last_ts, n_frames);
      }
    }
    helper_1_2(record, first_ts, last_ts, n_frames);

  }

}




void AppHelper::helper_2() {
  // QUICK TIMESTAMP CHECK Don't check each fragment
  /* for each trigger record get
   * 1. start timestamp
   * 2. end timestamp
   * 3. check for missing timestamps
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
  for (const auto& record : m_record_buffer) { 
    uint64_t first_ts;
    uint64_t last_ts;
    size_t n_frames;
    auto& fragments = record->get_fragments_ref();
    helper_1_1(fragments, first_ts, last_ts, n_frames);


    std::ranges::reverse_view rv{fragments};
    helper_1_1(rv, first_ts, last_ts, n_frames);
    helper_1_2(record, first_ts, last_ts, n_frames);

  }

}





void AppHelper::helper_3() {
  /* for each trigger record window get the corresponding timeslice window
   *
   */

  fmt::print("INFO trigger record: \n");
  fmt::print("INFO trigger record {}: \n", m_record_window.size());
  for (auto& i: m_record_window) {
    fmt::print("INFO trigger record {}: [ {} : {} ]\n", i.at(0), i.at(1), i.at(2));

    std::cout << dts_to_datetime(i.at(1), std::string("(beg)."));
    std::cout << dts_to_datetime(i.at(2), std::string("(end)."));

  }

  fmt::print("INFO timeslice {}: \n", m_input_files_tp.size());

  for (auto& input_file: m_input_files_tp) {

    auto records = input_file->get_all_record_ids();
    fmt::print("INFO slices: {}\n", records.size());
    fmt::print("INFO slices to process: {}\n", m_helper->m_opts.input_slices.size());
    for (const auto& record : records) {

      if (m_helper->m_opts.input_slices.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_slices.begin(), m_helper->m_opts.input_slices.end(), record.first) == m_helper->m_opts.input_slices.end()) {
          continue;
        }
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL timeslice number {}\n", record.first);

      m_slice_buffer.push_back(std::make_unique<daqdataformats::TimeSlice>(input_file->get_timeslice(record)));

      daqdataformats::TimeSlice timeslice = input_file->get_timeslice(record);      
      auto& fragments = timeslice.get_fragments_ref();
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments {}\n", fragments.size());
	//get_valid_sourceids(fragments);
      }
      get_tps(fragments);
      m_rp->plot_tps(m_tp_buffer, "slice");
    }

  }
}





};

#endif
