#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <filesystem>
#include <format>
#include <unordered_map>
#include <vector>

namespace graph {

template <typename T>
class Graph final {
 public:
  bool insert(const T& v, const std::vector<T>& adj) {
    vertices_.insert(v);
    for (const auto& a : adj) {
      vertices_.insert(a);
    }

    return adj_list_.insert({v, adj}).second;
  }

  void dump() const {
    using namespace boost;

    using OutGraph = adjacency_list<vecS, vecS, directedS,
                                    property<vertex_name_t, std::string>,
                                    property<edge_name_t, std::string>>;
    OutGraph g;

    std::unordered_map<T, OutGraph::vertex_descriptor> vd;
    vd.reserve(adj_list_.size());
    for (const auto& v : vertices_) {
      auto d = add_vertex(g);
      put(vertex_name, g, d, std::to_string(v));
      vd.insert({v, d});
    }

    for (auto& [v, adj] : adj_list_) {
      for (const auto& a : adj) {
        add_edge(vd[v], vd[a], g);
      }
    }

    write_graphviz(std::cout, g, make_label_writer(get(vertex_name, g)));
  }

 private:
  std::unordered_map<T, std::vector<T>> adj_list_;
  std::unordered_set<T> vertices_;
};
}  // namespace graph
