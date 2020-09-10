#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

#ifdef ENABLE_INTERACTIVE_VIEWER
#include <gtkmm-3.0/gtkmm/application.h>
#endif

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

int
main(int argc, char* argv[])
{
  po::options_description desc("Allowed options");
  po::positional_options_description posDesc;
  // clang-format off
  desc.add_options()
    ("help", "produce help message")
    ("input", po::value<std::string>(), "input file for code generation")
  ;
  posDesc.add("input", -1);
  // clang-format on
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
              .options(desc)
              .positional(posDesc)
              .allow_unregistered()
              .run(),
            vm);
  po::notify(vm);

  if(vm.count("help")) {
    return EXIT_SUCCESS;
  }

  if(!vm.count("trace")) {
    cerr << "!! Requires a trace file!" << endl;
    return EXIT_FAILURE;
  }

  std::string trace = vm["trace"].as<std::string>();
  if(!fs::exists(trace)) {
    cerr << "!! Trace file \"" << trace << "\" does not exist!" << endl;
    return EXIT_FAILURE;
  }
  if(fs::is_directory(trace)) {
    cerr << "!! File \"" << trace << "\" no file, but a directory!" << endl;
    return EXIT_FAILURE;
  }

#ifdef ENABLE_INTERACTIVE_VIEWER
  auto app = Gtk::Application::create("at.maxheisinger.distrac");

  MainWindow mainWindow(traceFile);

  return app->run(mainWindow);
#else
  cerr << "!! Interactive viewer not available in this build! Specify offline "
          "analysis method instead."
       << endl;
#endif
}