#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>

#include "graphs/graph.hh"

namespace {
namespace po = boost::program_options;

void help(char** argv, const po::options_description& desc) {
  std::cout << "Usage: " << argv[0] << " [options]\n";
  std::cout << desc << std::endl;
}
}  // namespace

int main(int argc, char** argv) try {
  po::options_description desc("Options");
  // clang-format off
  desc.add_options()
    ("help,h", "Print help message and exit");
  // clang-format on

  po::variables_map vm;
  po::command_line_parser cmdp(argc, argv);
  try {
    po::store(cmdp.options(desc).run(), vm);
  } catch (po::error& e) {
    std::cerr << e.what() << std::endl;
    help(argv, desc);
    return EXIT_FAILURE;
  }

  if (vm.count("help")) {
    help(argv, desc);
    return 0;
  }

  auto g = graph::readGraph<std::size_t>(std::cin);
  g.dump(std::cout);
  return 0;
} catch (std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
