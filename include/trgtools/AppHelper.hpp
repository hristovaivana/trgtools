#ifndef TRGTOOLS_APPHELPER_HPP_
#define TRGTOOLS_APPHELPER_HPP_

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include "hdf5libs/HDF5RawDataFile.hpp"

namespace dunedaq::trgtools
{

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
    AppHelper(std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> _input_files,
              bool _verbose);
    ~AppHelper() = default;

    /**
     * @brief Struct with available cli application options
     */
    struct Options
    {
      /// @brief vector of input filenames
      std::vector<std::string> input_files;
      /// @brief print more info to stdout? default: no
      bool verbose = false;
    };    

    /**
    * @brief Adds options to our CLI application
    * 
     * @param _app CLI application
     * @param _opts Struct with the available options
     */
    void parse_app(CLI::App& _app, Options& _opts);


  private:
    /// @brief A pointer to the input file
    std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> m_input_files;
    /// @brief Quiet down the cout output
    const bool m_verbose;

};

};

#endif
