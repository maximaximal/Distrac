#include <boost/program_options/value_semantic.hpp>
#include <cstdlib>
#include <iostream>
#include <iterator>
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
using std::cout;
using std::endl;

int
main(int argc, char* argv[]) {
  bool print_summary = true;
  bool print_node_count = true;
  bool print_start_ns = true;
  bool print_end_ns = true;
  bool omit_csv_header = false;
  bool force_causal_sync = false;

  po::variables_map vm;
  po::options_description desc("Allowed options");
  try {
    po::positional_options_description posDesc;
    // clang-format off
  desc.add_options()
    ("help", "produce help message")
    ("trace", po::value<std::string>(), "input file for tracing")
    ("force-causal-sync", po::bool_switch(&force_causal_sync), "force synchronizing causal relations")
    ("omit-csv-header", po::bool_switch(&omit_csv_header), "omit the header of CSV output")
    ("print-summary,s", po::bool_switch(&print_summary), "print summary of tracefile")
    ("print-event,e", po::value<std::vector<std::string>>()->multitoken(), "print all occurences of given event names")
    ("print-node-count", po::bool_switch(&print_node_count), "print node count")
    ("print-start-ns", po::bool_switch(&print_start_ns), "print start time (ns) in trace-local time")
    ("print-end-ns", po::bool_switch(&print_end_ns), "print end time (ns) in trace-local time")
    ("select-nodes", po::value<std::vector<size_t>>()->multitoken(), "limit selection to given nodes (indexing based on internal node numbering, [0, N[)")
  ;
  posDesc.add("trace", -1);
    // clang-format on
    po::store(po::command_line_parser(argc, argv)
                .options(desc)
                .positional(posDesc)
                .allow_unregistered()
                .run(),
              vm);
    po::notify(vm);
  } catch(std::exception& e) {
    std::cerr << "!! Error during parameter parsing: " << e.what() << endl;
    return EXIT_FAILURE;
  }

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

  distrac::tracefile tracefile(trace, force_causal_sync);

  if(print_node_count) {
    cout << tracefile.node_count() << endl;
  }

  if(print_start_ns) {
    cout << tracefile.trace_internal_begin_time_ns() << endl;
  }

  if(print_end_ns) {
    cout << tracefile.trace_internal_end_time_ns() << endl;
  }

  if(print_summary) {
    tracefile.print_summary();
  }

  std::set<size_t> selected_nodes;
  if(vm.count("select-nodes")) {
    auto s = vm["select-nodes"].as<std::vector<size_t>>();
    selected_nodes = std::set<size_t>(s.begin(), s.end());

    for(auto n : selected_nodes) {
      if(n >= tracefile.node_count()) {
        cerr << "!! Node number " << n
             << " does not exist, as the node count is only "
             << tracefile.node_count() << "!" << endl;
      }
    }
  }

  if(vm.count("print-event")) {
    std::vector<std::string> event_names =
      vm["print-event"].as<std::vector<std::string>>();
    std::set<uint8_t> event_filter;

    for(auto& ev : event_names) {
      ssize_t event_id = tracefile.get_event_id(ev);
      if(event_id == -1) {
        cerr << "!! Event \"" << ev << "\" does not exist!" << endl;
        return EXIT_FAILURE;
      }
      assert(event_id >= 0 && event_id < 255);
      event_filter.insert(event_id);
    }

    bool first = !omit_csv_header;
    for(const auto& ev : tracefile.filtered(event_filter)) {
      if(first) {
        ev.csv_header_out(std::cout) << endl;
        first = false;
      }

      if(selected_nodes.size()) {
        if(!selected_nodes.count(ev.node_tracefile_location_index())) {
          continue;
        }
      }

      if(event_names.size() > 1) {
        std::cout << ev.ev_name() << " ";
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
