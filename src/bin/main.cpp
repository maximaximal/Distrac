#include <boost/program_options/value_semantic.hpp>
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

#include <distrac/analysis/event_iterator.hpp>
#include <distrac/analysis/tracefile.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

int
main(int argc, char* argv[]) {
  bool print_summary = true;

  po::options_description desc("Allowed options");
  po::positional_options_description posDesc;
  // clang-format off
  desc.add_options()
    ("help", "produce help message")
    ("trace", po::value<std::string>(), "input file for tracing")
    ("print-summary,s", po::bool_switch(&print_summary), "print summary of tracefile")
    ("print-event,e", po::value<std::string>(), "print all occurences of a given event name")
  ;
  posDesc.add("trace", -1);
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
    cout << desc << endl;
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

  distrac::tracefile tracefile(trace);

  if(print_summary) {
    tracefile.print_summary();
  }

  if(vm.count("print-event")) {
    std::string event_name = vm["print-event"].as<std::string>();
    ssize_t event_id = tracefile.get_event_id(event_name);
    if(event_id == -1) {
      cerr << "!! Event \"" << event_name << "\" does not exist!" << endl;
      return EXIT_FAILURE;
    }

    assert(event_id >= 0 && event_id < 255);
    bool first = true;
    for(const auto& ev :
        tracefile.filtered({ static_cast<uint8_t>(event_id) })) {
      if(first) {
        ev.csv_header_out(std::cout) << endl;
        first = false;
      }
      ev.csv_out(std::cout) << endl;
    }
  }

#ifdef ENABLE_INTERACTIVE_VIEWER
  auto app = Gtk::Application::create("at.maxheisinger.distrac");

  MainWindow mainWindow(traceFile);

  return app->run(mainWindow);
#else
#endif

  return EXIT_SUCCESS;
}
