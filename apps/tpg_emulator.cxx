/**
 * @file tpg_emulator.cxx
 *
 * Developer(s) of this DAQ application have yet to replace this line with a 
brief description of the application.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "trgtools/AppHelper.hpp"

using namespace dunedaq;
using namespace trgtools;

int
main(int argc, char* argv[])
{

  CLI::App app{"tpg_emulator"};
  AppHelper ah;
  AppHelper::Options opts{};
  ah.parse_app(app, opts);

  try {
    app.parse(argc, argv);
  }
  catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  ah.config_app(opts);



  //if (opts.verbose) 
  fmt::print("{:.<77}\n", "");
  fmt::print("Done! \n");

  return 0;
}
