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
      : start_(start),
        end_(end),
        adj_list_{{start, std::vector<T>{}}, {end_, std::vector<T>{}}} {}

  bool insert(const T& v, const std::vector<T>& adj) {
    for (const auto& a : adj) {
      auto it = adj_list_.find(a);
      if (it == adj_list_.end()) {
        adj_list_.emplace(
            a, std::vector<T>{end_});  // speculatively mark as end node
      }
      ++in_deg_[a];
    }

    if (in_deg_.find(v) == in_deg_.end()) {
      in_deg_.emplace(v, 0);
    }

    auto [it, ins] = adj_list_.emplace(v, adj);
    if (!ins) {
      if (it->second == std::vector<T>{end_}) {
        it->second = adj;
      } else {
        return false;
      }
    }

    linkToStart();
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
  T start_;
  T end_;
  std::unordered_map<T, std::vector<T>> adj_list_;
  std::unordered_map<T, std::size_t> in_deg_;

  void linkToStart() {
    auto& start_nodes = adj_list_[start_];
    start_nodes.clear();

    for (const auto& [v, c] : in_deg_) {
      if (c == 0) {
        start_nodes.push_back(v);
      }
    }
  }
};
}  // namespace graph
