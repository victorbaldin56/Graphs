#include <boost/program_options.hpp>
#include <format>
#include <fstream>
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
    ("help,h", "Print help message and exit")
    ("o,output-file", po::value<std::string>(), "Specify output file for graph")
    ("domtree", po::value<std::string>(), "Specify output file for dominators tree")
    ("pdomtree", po::value<std::string>(), "Specify output file for postdominators tree");
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

  if (vm.count("o")) {
    auto out = vm["o"].as<std::string>();
    std::ofstream os(out);
    if (!os.is_open()) {
      throw std::runtime_error(std::format("Couldnt open file {}", out));
    }
    g.dump(os, "Graph");
  } else {
    g.dump(std::cout, "Graph");
  }

  if (vm.count("domtree")) {
    auto domtree = vm["domtree"].as<std::string>();
    std::ofstream os(domtree);
    if (!os.is_open()) {
      throw std::runtime_error(std::format("Couldnt open file {}", domtree));
    }
    g.dominatorTree().dump(os, "Dominator Tree", false);
  }

  if (vm.count("pdomtree")) {
    auto pdomtree = vm["pdomtree"].as<std::string>();
    std::ofstream os(pdomtree);
    if (!os.is_open()) {
      throw std::runtime_error(std::format("Couldnt open file {}", pdomtree));
    }
    g.postDominatorTree().dump(os, "Post Dominator Tree", false);
  }

  return 0;
} catch (std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
