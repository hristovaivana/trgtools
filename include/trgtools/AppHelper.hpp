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

#include <TGraph.h>
#include "THttpServer.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom3.h"
#include "TSystem.h"
#include "THStack.h"

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


class RootPlotter {
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

    int max_ch{12284};

    //auto cst = new TCanvas("cst", "stacked hists", 10, 10, 700, 700);
    TCanvas* cst = new TCanvas("cst", "stacked hists");
    THStack* hs = new THStack("hs", "Stacked 1D histograms");

    //TCanvas *c1 = new TCanvas("c1", "Overlay Histograms", 800, 600);
    TCanvas *c1 = new TCanvas("c1", "Overlay Histograms");


    TH1F *h1_ch_r = new TH1F("h1_ch_r", "This is the channel distribution", max_ch, 0, max_ch);
    TH1F *h1_ch_s = new TH1F("h1_ch_s", "This is the channel distribution", max_ch, 0, max_ch);

  private:
    void plot_tps(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const std::string& type);
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



    /**
     * @brief Struct with available cli application options
     */
    struct Options
    {
      /// @brief vector of input filenames
      std::vector<std::string> input_files;

      std::vector<int> input_records;
      std::vector<int> input_slices;
      //int bytes_per_tp;

      int helper;
      std::vector<int> helpers;
      /// @brief print more info to stdout? default: no
      bool assert = false;
      bool verbose = false;
      bool verbose2 = false;

      friend std::ostream& operator<<(std::ostream& os, const Options& o)
      {
	os << "Job options:\n";
	os << "TriggerRecords: "; for (auto& i : o.input_records) os << i << ", "; os << "\n";
	os << "TimeSlices: "; for (auto& i : o.input_slices)  os << i << ", "; os << "\n";
	//os << "-b, --bytes-per-tp: " << o.bytes_per_tp  << "\n";
	os << "-e, --helper: " << o.helper  << "\n";
	os << "-ee (helpers): "; for (auto& i : o.helpers)  os << i << ", "; os << "\n";
        os << "-a, --assert: " << o.assert  << "\n"
	   << "-v, --verbose: " << o.verbose  << "\n"
	   << "-vv, --verbose2: " << o.verbose2  << "\n";

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


    /// @brief A pointer to the input file
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files;
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files_raw;
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files_tp;
    std::vector<int> m_input_records;
    std::vector<int> m_input_slices;
    /// @brief Quiet down the cout output
    //const bool m_verbose;
    bool m_verbose;
    std::unique_ptr<Helper> m_helper;

    std::string dts_to_datetime(uint64_t& ts, std::string s);
    //void get_tps(auto& fragments, TH1F *h1_ch, TString s);
    void get_tps(auto& fragments);
    void get_valid_sourceids(auto& fragments);
    void app_test_trts(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file, bool a, bool v, bool vv);
    void app_test_trts_info(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file);



    void helper_0();
    void helper_1();
      void helper_1_0(const auto& fragment, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames);
      void helper_1_1(auto& fragments, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames);
      void helper_1_2(auto& record, uint64_t& first_ts, uint64_t& last_ts, uint64_t& n_frames);
    void helper_2();
    void helper_3();
    //#pragma GCC diagnostic push
    //#pragma GCC diagnostic ignored "-Wpedantic"
    // code with flexible array member
    //void (*helpers[])();
    //void (AppHelper::*helpers[])();      
    //#pragma GCC diagnostic pop
    void (AppHelper::*helpers[4])() = { &AppHelper::helper_0, &AppHelper::helper_1, &AppHelper::helper_2, &AppHelper::helper_3 };      

    static const size_t SIZE_WIB_FRAME  = sizeof(fddetdataformats::WIBEthFrame);
    static const uint64_t SAMPLE_TICK = 32;
    static const uint64_t TICKS_PER_FRAME = 64;
    int BYTES_PER_TP = 0;
    std::vector<std::unique_ptr<daqdataformats::TriggerRecord>> m_record_buffer;
    std::vector<std::unique_ptr<daqdataformats::TimeSlice>> m_slice_buffer;

    //std::vector<std::shared_ptr<std::vector<uint64_t>>> m_record_window;
    std::vector<std::vector<uint64_t>> m_record_window;
    std::map<uint64_t, std::vector<daqdataformats::SourceID>> m_record_sourceids;
    std::map<uint64_t, std::vector<daqdataformats::SourceID>> m_slice_sourceids;
    std::vector<trgdataformats::TriggerPrimitive> m_tp_buffer;

    // ROOT
    RootPlotter* m_rp;

    /*
    int max_ch{12284};
    //TH1F *h1_ch = new TH1F("h1_ch", "This is the channel distribution", max_ch, 0, max_ch);

    //TCanvas *c1 = new TCanvas("c1", "Overlay Histograms", 800, 600);
    TCanvas *c1 = new TCanvas("c1", "Overlay Histograms");
    TH1F *h1_ch_r = new TH1F("h1_ch_r", "This is the channel distribution", max_ch, 0, max_ch);
    TH1F *h1_ch_s = new TH1F("h1_ch_s", "This is the channel distribution", max_ch, 0, max_ch);
    */

};

};

#endif
