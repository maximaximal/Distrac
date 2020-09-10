#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

int
main(int argc, char* argv[])
{
  po::options_description desc("Codegen options");
  po::positional_options_description posDesc;
  // clang-format off
  desc.add_options()
    ("help", "produce help message")
    ("output", po::value<std::string>(), "output file for code generation")
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

  if(!vm.count("input")) {
    cerr << "!! Requires an input file!" << endl;
    return EXIT_FAILURE;
  }

  std::string input = vm["input"].as<std::string>();
  if(!fs::exists(input)) {
    cerr << "!! Input file \"" << input << "\" does not exist!" << endl;
    return EXIT_FAILURE;
  }
  if(fs::is_directory(input)) {
    cerr << "!! File \"" << input << "\" no file, but a directory!" << endl;
    return EXIT_FAILURE;
  }

}
