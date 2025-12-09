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
  Graph(const T& start = T(), const T& end = T())
      : end_(end), adj_list_{{end_, std::vector<T>{}}} {}

  bool insert(const T& v, const std::vector<T>& adj) {
    for (const auto& a : adj) {
      auto it = adj_list_.find(a);
      if (it == adj_list_.end()) {
        adj_list_.emplace(
            a, std::vector<T>{end_});  // speculatively mark as end node
      }
    }

    auto [it, ins] = adj_list_.emplace(v, adj);
    if (!ins) {
      if (it->second == std::vector<T>{end_}) {
        it->second = adj;
      } else {
        return false;
      }
    }
    return true;
  }

  void dump() const {
    using namespace boost;

    using OutGraph = adjacency_list<vecS, vecS, directedS,
                                    property<vertex_name_t, std::string>,
                                    property<edge_name_t, std::string>>;
    OutGraph g;

    std::unordered_map<T, OutGraph::vertex_descriptor> vd;
    vd.reserve(adj_list_.size());
    for (const auto& [v, _] : adj_list_) {
      auto d = add_vertex(g);
      put(vertex_name, g, d, v);
      vd.emplace(v, d);
    }

    for (auto& [v, adj] : adj_list_) {
      for (const auto& a : adj) {
        add_edge(vd[v], vd[a], g);
      }
    }

    write_graphviz(std::cout, g, make_label_writer(get(vertex_name, g)));
  }

 private:
  T end_;
  std::unordered_map<T, std::vector<T>> adj_list_;
};
}  // namespace graph
