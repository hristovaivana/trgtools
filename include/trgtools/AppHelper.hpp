#ifndef TRGTOOLS_APPHELPER_HPP_
#define TRGTOOLS_APPHELPER_HPP_


#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>

#include <ranges>
#include <any>
//#include <thread>
//#include <chrono>
//#include <ctime>
#include <signal.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <iostream>
//#include <cstdlib>


#include "hdf5libs/HDF5RawDataFile.hpp"
#include "trgdataformats/TriggerPrimitive.hpp"
#include "fddetdataformats/WIBEthFrame.hpp"

#include "trgtools/TPGEmulator.hpp"
#include "trgtools/TPGEmulators.hpp"
#include "trgtools/detail/TriggerPrimitive_v4.hpp"

#include <TStyle.h>
#include <TMultiGraph.h>
#include <TGraph.h>
#include "THttpServer.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom3.h"
#include "TSystem.h"
#include "THStack.h"
#include "TVectorD.h"

#include "TFile.h"
#include "TMemFile.h"
#include "TNtuple.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TTimer.h"
#include "TROOT.h"



volatile sig_atomic_t gStopProcessing = false;

namespace dunedaq::trgtools
{


class AppHelper;

//https://root.cern.ch/doc/master/classTGraphPainter.html#GP03
class RootPlotter {

  enum tpg_type {
    RECORD = 0,
    SLICE = 1,
    TPGEMU = 2,
    TPGNAIVE = 3,
    TPGSIM = 4
  };

  public:
    RootPlotter();
    ~RootPlotter() = default;

    friend class AppHelper;

  public:
    static void signal_callback_handler(int signum) {
      std::cout << "Caught signal " << signum << std::endl;
      // Terminate program
      //exit(signum);
      gStopProcessing = true;
    }

  protected:
    // ROOT
    //THttpServer *serv;


    //auto cst = new TCanvas("cst", "stacked hists", 10, 10, 700, 700);
    TCanvas* cst = new TCanvas("cst", "stacked hists");
    THStack* hs = new THStack("hs", "Stacked 1D histograms");

 /*
    std::vector<TCanvas*> csts;
    std::vector<THStack*> hss;
    std::vector<TGraph*> grs;


    std::vector<TCanvas*> cmgs;
    std::vector<TMultiGraph*> mgrs{new TMultiGraph()};

    std::vector<TCanvas*> crse;  
*/
    
    std::map<TString, std::vector<TCanvas*>> csts;
    std::map<TString, std::vector<THStack*>> hss;
    std::map<TString, std::vector<TGraph*>> grs;


    std::map<TString, std::vector<TCanvas*>> cmgs;
    std::map<TString, std::vector<TMultiGraph*>> mgrs;
    std::map<TString, TString> m_canvas_titles;
    std::vector<TString> m_2d_xtitles;
    std::vector<TString> m_2d_ytitles;

    std::map<TString, std::vector<TCanvas*>> crse;


    //TCanvas *c1 = new TCanvas("c1", "Overlay Histograms", 800, 600);
    TCanvas *c1 = new TCanvas("c1", "Overlay Histograms");

    int max_ch{12284};
    TH1F *h1_ch = new TH1F("h1_ch", "This is the channel distribution", max_ch, 0, max_ch);

    TH1F *h1_tp = new TH1F("h1_tp", "This is the TP distribution", 500., 0., 1.0);
    //TH1F *h1_ch_r = new TH1F("h1_ch_r", "This is the channel distribution", max_ch, 0, max_ch);
    //TH1F *h1_ch_s = new TH1F("h1_ch_s", "This is the channel distribution", max_ch, 0, max_ch);
    TH2D* h2_tp = new TH2D("h2_tp", "h2_tp", 200, 0, 1, max_ch, 0, max_ch);

    bool pause{false};

  private:
    int m_current_runnum{0};
    uint64_t m_current_recnum{0};
    std::string m_current_folder{""};
    void plot_tps(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const std::string& type, bool pause=false);


    std::vector<TString> m_tp_titles;
    std::vector<TString> m_tp_subs;

    std::unique_ptr<THttpServer> m_serv;
    void book(dunedaq::trgtools::AppHelper* ah);
    void fill_helper_2(TH1F* h1i, TH1F* h1, const tpg_type& type, const TString* info);
    TGraph* fill_helper_1(const tpg_type& type);
    void fill(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const tpg_type& type);
    void show(dunedaq::trgtools::AppHelper* ah);
};


class AppHelper
{
  public:
    /**
     * @brief Constructor, takes file input path & configuration
     * 
     * Each TAFileHandler will create its own thread, so all TAFileHandlers are
     * run on separate threads
     * 
     * @param _input_files a vector of input HDF5 shared pointers to process
     * @param _config TAMaker configuration
     * @param _sliceid_range range of sliceids to process
     * @param _run_parallel run each TAMaker (one per SourceID) in parallel
     * @param _verbose quiet down the cout
     */
    AppHelper();
    ~AppHelper() = default;
    //~AppHelper();

    friend class RootPlotter;

    /**
     * @brief Struct with available cli application options
     */
    struct Options
    {
      /// @brief vector of input filenames
      std::vector<std::string> input_files;
      std::string config_name;

      std::vector<uint64_t> input_records;
      std::vector<uint64_t> input_slices;
      std::string tp_format = "v5";

      int helper;
      std::vector<int> helpers;
      /// @brief print more info to stdout? default: no
      bool assert = false;
      bool verbose = false;
      bool verbose2 = false;
      std::string port = "8081";

      friend std::ostream& operator<<(std::ostream& os, const Options& o)
      {
	os << "Job options:\n";
	os << "-j, --json-config: " << o.config_name << "\n";
	os << "TriggerRecords: "; for (auto& i : o.input_records) os << i << ", "; os << "\n";
	os << "TimeSlices: "; for (auto& i : o.input_slices)  os << i << ", "; os << "\n";
	os << "-f, --tp-format: " << o.tp_format  << "\n";
	os << "-e, --helper: " << o.helper  << "\n";
	os << "-ee (helpers): "; for (auto& i : o.helpers)  os << i << ", "; os << "\n";
	os << "-p, --port: " << o.port  << "\n";
        os << "-a, --assert: " << o.assert  << "\n"
	   << "-v, --verbose: " << o.verbose  << "\n"
	   << "-vv, --verbose2: " << o.verbose2  << "\n"
	   ;

        return os;
      }

    }; 

    struct Helper
    {
      Helper(Options& opts) : m_opts(opts) 
      {
        //fmt::print("INFO going to start helper {}\n", m_opts.helper);
      };
      void start_message(const int& id) {
        fmt::print("{:.<30}\n", "");
        fmt::print("INFO going to start helper {}\n", id);
      }
      Options m_opts;
    };

    /**
    * @brief Adds options to our CLI application
    * 
     * @param _app CLI application
     * @param _opts Struct with the available options
     */
    void parse_app(CLI::App& _app, Options& _opts);

    void config_app(Options& _opts);


  private:
    void dummy(auto& fragment);

    /// @brief A pointer to the input file
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files;
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files_raw;
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files_tp;
    std::set<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_matched_files_tp;
    //std::vector<uint32_t> m_valid_tp_sourceids; 
    std::vector<daqdataformats::SourceID> m_valid_tp_sourceids; 
    std::vector<uint64_t> m_input_records;
    std::vector<uint64_t> m_input_slices;
      std::vector<std::vector<uint64_t>> m_input_records_slices; // no stitching
      std::map<uint64_t, uint64_t> m_input_slices_records; 
    int m_num_records{0};
    int m_num_slices{0};
    /// @brief Quiet down the cout output
    //const bool m_verbose;
    bool m_verbose;
    std::unique_ptr<Helper> m_helper;

    trgdataformats::TriggerPrimitive convert_tp(auto& tp);
    template <typename T>
    void  get_tps(const std::vector<std::unique_ptr<dunedaq::daqdataformats::Fragment>>& fragments, const std::vector<uint64_t>& rw, const T& TriggerPrimitive);
    void  get_tps_wrapper(const std::vector<std::unique_ptr<dunedaq::daqdataformats::Fragment>>& fragments, const std::vector<uint64_t>& rw);


    std::string dts_to_datetime(uint64_t& ts, std::string s);
    //void get_tps(auto& fragments, TH1F *h1_ch, TString s);
    void get_tps_old(auto& fragments, const std::vector<uint64_t>& rw);
    void get_valid_sourceids(auto& fragments);
    void get_valid_tp_sourceids(auto& fragments, const std::string& type);
    void app_test_trts(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file, bool a, bool v, bool vv);
    void app_test_trts_info(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file);



    void helper_0();
      void helper_0_1();
    void helper_1();
      void helper_1_0(const auto& fragment, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames);
      void helper_1_1(auto& fragments, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames);
      void helper_1_2(auto& record, uint64_t& first_ts, uint64_t& last_ts, uint64_t& n_frames);
    void helper_2();
    void helper_3(); void helper_3_old();
    void helper_4();
    void helper_5();
    void helper_6();
    void helper_7();
    void helper_8();
    void helper_9();
    void helper_10();
    //#pragma GCC diagnostic push
    //#pragma GCC diagnostic ignored "-Wpedantic"
    // code with flexible array member
    //void (*helpers[])();
    //void (AppHelper::*helpers[])();      
    //#pragma GCC diagnostic pop
    void (AppHelper::*helpers[11])() = { &AppHelper::helper_0
	    , &AppHelper::helper_1, &AppHelper::helper_2, &AppHelper::helper_3
	    , &AppHelper::helper_4, &AppHelper::helper_5, &AppHelper::helper_6
	    , &AppHelper::helper_7, &AppHelper::helper_8, &AppHelper::helper_9
	    , &AppHelper::helper_10
    };      

    static const size_t SIZE_WIB_FRAME  = sizeof(fddetdataformats::WIBEthFrame);
    static const uint64_t SAMPLE_TICK = 32;
    static const uint64_t TICKS_PER_FRAME = 64;
    int BYTES_PER_TP = 0;
    std::vector<std::unique_ptr<daqdataformats::TriggerRecord>> m_record_buffer;
    std::vector<std::unique_ptr<daqdataformats::TimeSlice>> m_slice_buffer;

    std::set<uint64_t> m_trigger_numbers;
    std::map<uint64_t, std::vector<uint64_t>> m_record_window_map;
    std::vector<std::vector<uint64_t>> m_record_window;
    std::map<uint64_t, std::vector<uint64_t>> m_slice_window_map;
      std::map<std::pair<uint64_t, uint16_t>, std::vector<uint64_t>> m_slice_window_buffer;
    std::map<uint64_t, std::vector<daqdataformats::SourceID>> m_record_sourceids;
    std::map<uint64_t, std::vector<daqdataformats::SourceID>> m_slice_sourceids;
    std::vector<trgdataformats::TriggerPrimitive> m_record_tp_buffer;
    std::vector<trgdataformats::TriggerPrimitive> m_slice_tp_buffer;
    //std::vector<std::variant<trgdataformats::TriggerPrimitive, TriggerPrimitive_v4>> m_record_tp_buffer;
    //std::vector<std::variant<trgdataformats::TriggerPrimitive, TriggerPrimitive_v4>> m_slice_tp_buffer;
    std::vector<std::any> m_record_tp_buffer_any;
    std::vector<std::any> m_slice_tp_buffer_any;

    // ROOT
    //RootPlotter* m_rp;
    std::unique_ptr<RootPlotter> m_rp;
    std::unique_ptr<TPGEmulator> m_te;

    /*
    int max_ch{12284};
    //TH1F *h1_ch = new TH1F("h1_ch", "This is the channel distribution", max_ch, 0, max_ch);

    //TCanvas *c1 = new TCanvas("c1", "Overlay Histograms", 800, 600);
    TCanvas *c1 = new TCanvas("c1", "Overlay Histograms");
    TH1F *h1_ch_r = new TH1F("h1_ch_r", "This is the channel distribution", max_ch, 0, max_ch);
    TH1F *h1_ch_s = new TH1F("h1_ch_s", "This is the channel distribution", max_ch, 0, max_ch);
    */

    template <typename T>
    std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> emulate_using(const nlohmann::json& config, const std::vector<std::unique_ptr<daqdataformats::TriggerRecord>>& records) {
     using generator_t = T;
    std::unique_ptr<TPGEmulators<generator_t>> tes = std::make_unique<TPGEmulators<generator_t>>();
    tes->configure(config);
    return tes->emulate_using(records);
}


};

};

#endif
