#ifndef TRGTOOLS_APPHELPER_CPP_
#define TRGTOOLS_APPHELPER_CPP_

#include "trgtools/AppHelper.hpp"


namespace dunedaq::trgtools
{

AppHelper::AppHelper(std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>> input_files,
                     bool verbose)
  : m_input_files(input_files),
    m_verbose(verbose)
{
};

void AppHelper::parse_app(CLI::App& _app, Options& _opts)
{
  _app.add_option("-i,--input-files", _opts.input_files, "List of input files (required)")
    ->required()
    ->check(CLI::ExistingFile); // Validate that each file exists

  _app.add_flag("-v, --verbose", _opts.verbose, "Don't print outputs.");

}  

};

#endif
