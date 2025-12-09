#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <queue>
#include <unordered_map>
#include <vector>

namespace graph {

template <typename T>
class Graph final {
 public:
  using AdjMap = std::unordered_map<T, std::vector<T>>;

  Graph() noexcept = default;

  Graph(
      std::initializer_list<std::pair<T, std::initializer_list<T>>> adj_list) {
    for (const auto& [v, a] : adj_list) {
      insert(v, std::vector(a));
    }
  }

  bool insert(const T& v, const std::vector<T>& adj) {
    auto it = adj_list_.find(v);
    if (it != adj_list_.end()) {
      return false;
    }

    for (const auto& a : adj) {
      auto it = adj_list_.find(a);
      ++in_deg_[a];
      out_deg_.emplace(a, 0);
    }

    in_deg_.emplace(v, 0);
    out_deg_[v] = adj.size();
    adj_list_.emplace(v, adj);
    return true;
  }

  std::vector<T> topologicalSort() const {
    std::vector<T> order;
    std::queue<T> q;

    for (auto& [u, cnt] : in_deg_) {
      if (cnt == 0) {
        q.push(u);
      }
    }

    order.reserve(in_deg_.size());
    auto id = in_deg_;

    while (!q.empty()) {
      T u = q.front();
      q.pop();

      order.push_back(u);

      auto it = adj_list_.find(u);

      if (it != adj_list_.end()) {
        for (auto& v : it->second) {
          if (--id[v] == 0) {
            q.push(v);
          }
        }
      }
    }

    if (order.size() != in_deg_.size()) {
      throw std::runtime_error("Graph has cycle");
    }
    return order;
  }

  void dump(std::ostream& os) const {
    using namespace boost;

    using OutGraph = adjacency_list<vecS, vecS, directedS,
                                    property<vertex_name_t, std::string>,
                                    property<edge_name_t, std::string>>;

    OutGraph g;

    auto start = add_vertex(g);
    auto end = add_vertex(g);
    put(vertex_name, g, start, "Start");
    put(vertex_name, g, end, "End");

    std::unordered_map<T, OutGraph::vertex_descriptor> vd;
    vd.reserve(in_deg_.size());
    for (const auto& [v, _] : in_deg_) {
      auto d = add_vertex(g);
      put(vertex_name, g, d, std::to_string(v));
      vd.emplace(v, d);
    }

    for (const auto& [v, cnt] : in_deg_) {
      if (cnt == 0) {
        add_edge(start, vd[v], g);
      }
    }

    for (const auto& [v, cnt] : out_deg_) {
      if (cnt == 0) {
        add_edge(vd[v], end, g);
      }
    }

    for (const auto& [v, adj] : adj_list_) {
      for (const auto& a : adj) {
        add_edge(vd[v], vd[a], g);
      }
    }

    write_graphviz(os, g, make_label_writer(get(vertex_name, g)));
  }

 private:
  AdjMap adj_list_;

  using DegreeMap = std::unordered_map<T, std::size_t>;
  DegreeMap in_deg_;
  DegreeMap out_deg_;
};

template <typename T>
inline Graph<T> readGraph(std::istream& is) {
  graph::Graph<T> g;

  std::string line;
  while (std::getline(is, line)) {
    if (line.empty()) {
      break;
    }

    std::istringstream ss(line);
    T u;
    ss >> u;

    std::vector<T> adj;
    T v;
    while (ss >> v) {
      adj.push_back(v);
    }
    g.insert(u, adj);
  }

  return g;
}
}  // namespace graph
