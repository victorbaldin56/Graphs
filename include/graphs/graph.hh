#pragma once

#include <iostream>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
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

  void dump(std::ostream& os, std::string graph_name = "",
            bool add_sentinel = true) const {
    os << "digraph {\n"
       << "label=\"" << graph_name << "\"\n";

    if (add_sentinel) {
      os << "Start[label=\"Start\"];\n"
         << "End[label=\"End\"];\n";

      for (const auto& [u, cnt] : in_deg_) {
        if (cnt == 0) {
          os << "\"Start\" -> " << u << ";\n";
        }
      }

      for (const auto& [u, cnt] : out_deg_) {
        if (cnt == 0) {
          os << u << " -> \"End\";\n";
        }
      }
    }

    for (auto& [u, vs] : adj_list_) {
      for (const auto& v : vs) {
        os << u << " -> " << v << ";\n";
      }
    }

    os << "}\n";
    os.flush();
  }

  // compute dominator sets using a virtual entry (internal only). Returned map
  // contains only real nodes.
  std::unordered_map<T, std::unordered_set<T>> computeDominatorsVirtualEntry()
      const {
    // build augmented adjacency keyed by Opt
    std::unordered_map<Opt, std::vector<Opt>, OptHash> adj_opt;
    adj_opt.reserve(in_deg_.size() + 1);

    // insert all real nodes (even those with empty outgoing)
    for (const auto& [v, _] : in_deg_) {
      Opt key = Opt(v);
      auto it = adj_list_.find(v);
      if (it != adj_list_.end()) {
        std::vector<Opt> outs;
        outs.reserve(it->second.size());
        for (auto const& w : it->second) outs.push_back(Opt(w));
        adj_opt.emplace(key, std::move(outs));
      } else
        adj_opt.emplace(key, std::vector<Opt>{});
    }

    // virtual entry connects to all zero-in-degree nodes
    std::vector<Opt> starts;
    for (const auto& [v, cnt] : in_deg_)
      if (cnt == 0) starts.push_back(Opt(v));
    adj_opt.emplace(std::nullopt, std::move(starts));

    // compute dominators on augmented graph
    auto dom_opt = computeDominatorsOnOptAdj(adj_opt, std::nullopt);

    // convert back to real-node dom sets (drop virtual node from sets)
    std::unordered_map<T, std::unordered_set<T>> dom;
    dom.reserve(dom_opt.size());
    for (auto const& [k, set_opt] : dom_opt) {
      if (!k) continue;  // skip virtual
      std::unordered_set<T> s;
      s.reserve(set_opt.size());
      for (auto const& x : set_opt)
        if (x) s.insert(*x);
      dom.emplace(*k, std::move(s));
    }
    return dom;
  }

  // immediate dominators from dom-sets (root nodes will map to nullopt)
  std::unordered_map<T, std::optional<T>> immediateDominatorsFromDomSets(
      const std::unordered_map<T, std::unordered_set<T>>& dom) const {
    std::unordered_map<T, std::optional<T>> idom;
    idom.reserve(dom.size());
    for (auto const& [n, dset] : dom) {
      if (dset.size() <= 1) {
        idom[n] = std::nullopt;
        continue;
      }
      // candidates = dset - {n}
      std::vector<T> cand;
      cand.reserve(dset.size());
      for (auto const& x : dset)
        if (!(x == n)) cand.push_back(x);
      if (cand.empty()) {
        idom[n] = std::nullopt;
        continue;
      }
      // choose deepest: largest dominator set
      auto size_of = [&](const T& v) { return dom.at(v).size(); };
      T best = cand[0];
      for (size_t i = 1; i < cand.size(); ++i)
        if (size_of(cand[i]) > size_of(best)) best = cand[i];
      idom[n] = best;
    }
    return idom;
  }

  Graph<T> dominatorTree() const {
    auto dom = computeDominatorsVirtualEntry();
    auto idom = immediateDominatorsFromDomSets(dom);

    std::unordered_map<T, std::vector<T>> children;
    for (auto const& [n, _] : dom) children.emplace(n, std::vector<T>{});
    for (auto const& [n, p] : idom) {
      if (!p.has_value())
        continue;  // parent was virtual => root in returned forest
      children[*p].push_back(n);
    }

    Graph<T> tree;
    for (auto const& [v, ch] : children) tree.insert(v, ch);
    return tree;
  }

  Graph<T> postDominatorTree() const {
    AdjMap real = fullAdjacency();

    // reversed adjacency as Opt
    std::unordered_map<Opt, std::vector<Opt>, OptHash> rev_opt;
    rev_opt.reserve(real.size() + 1);
    for (auto const& [v, _] : real) rev_opt.emplace(Opt(v), std::vector<Opt>{});
    for (auto const& [u, outs] : real)
      for (auto const& v : outs) rev_opt[Opt(v)].push_back(Opt(u));

    // virtual entry in reversed graph connects to original exits (nodes with
    // zero outgoing).
    std::vector<Opt> exits;
    for (auto const& [v, outs] : real)
      if (outs.empty()) exits.push_back(Opt(v));
    rev_opt.emplace(std::nullopt, std::move(exits));

    auto dom_opt = computeDominatorsOnOptAdj(rev_opt, std::nullopt);

    std::unordered_map<T, std::unordered_set<T>> pdom;
    pdom.reserve(dom_opt.size());
    for (auto const& [k, set_opt] : dom_opt) {
      if (!k) continue;
      std::unordered_set<T> s;
      s.reserve(set_opt.size());
      for (auto const& x : set_opt)
        if (x) s.insert(*x);
      pdom.emplace(*k, std::move(s));
    }

    auto idom = immediateDominatorsFromDomSets(pdom);

    // build tree
    std::unordered_map<T, std::vector<T>> children;
    for (auto const& [n, _] : pdom) children.emplace(n, std::vector<T>{});
    for (auto const& [n, p] : idom) {
      if (!p.has_value()) continue;
      children[*p].push_back(n);
    }

    Graph<T> tree;
    for (auto const& [v, ch] : children) tree.insert(v, ch);
    return tree;
  }

  std::vector<T> nodes() const {
    std::vector<T> ns;
    ns.reserve(in_deg_.size());
    for (const auto& [v, _] : in_deg_) ns.push_back(v);
    return ns;
  }

 private:
  AdjMap adj_list_;

  using DegreeMap = std::unordered_map<T, std::size_t>;
  DegreeMap in_deg_;
  DegreeMap out_deg_;

  // small alias for optional key used only internally
  using Opt = std::optional<T>;

  struct OptHash {
    size_t operator()(const Opt& o) const noexcept {
      if (!o) {
        return 0x9e3779b97f4a7c15ULL;
      }
      return std::hash<T>()(*o);
    }
  };

  // compute dominators on an adjacency map keyed by Opt (virtual node =
  // nullopt)
  static std::unordered_map<Opt, std::unordered_set<Opt, OptHash>, OptHash>
  computeDominatorsOnOptAdj(
      const std::unordered_map<Opt, std::vector<Opt>, OptHash>& adj,
      const Opt& entry) {
    // reachable
    std::unordered_set<Opt, OptHash> seen;
    std::deque<Opt> dq;
    dq.push_back(entry);
    seen.insert(entry);
    while (!dq.empty()) {
      auto u = dq.front();
      dq.pop_front();
      auto it = adj.find(u);
      if (it == adj.end()) continue;
      for (const auto& v : it->second) {
        if (seen.insert(v).second) {
          dq.push_back(v);
        }
      }
    }

    std::unordered_map<Opt, std::vector<Opt>, OptHash> preds;
    for (auto const& n : seen) {
      preds.emplace(n, std::vector<Opt>{});
    }

    for (auto const& [u, outs] : adj) {
      if (!seen.count(u)) {
        continue;
      }
      for (auto const& v : outs) {
        if (seen.count(v)) preds[v].push_back(u);
      }
    }

    // init dom sets
    std::unordered_map<Opt, std::unordered_set<Opt, OptHash>, OptHash> dom;
    dom.reserve(seen.size());
    for (auto const& n : seen) {
      if (n == entry) {
        dom[n] = {entry};
      } else {
        dom[n] = std::unordered_set<Opt, OptHash>(seen.begin(), seen.end());
      }
    }

    // iterate until fixed point
    bool changed = true;
    while (changed) {
      changed = false;
      for (auto const& n : seen) {
        if (n == entry) {
          continue;
        }
        // intersect predecessors' dom sets
        std::unordered_set<Opt, OptHash> newdom;
        bool first = true;
        for (auto const& p : preds[n]) {
          if (first) {
            newdom = dom[p];
            first = false;
          } else {
            // intersection
            for (auto it = newdom.begin(); it != newdom.end();) {
              if (!dom[p].count(*it)) {
                it = newdom.erase(it);
              } else {
                ++it;
              }
            }
          }
        }
        if (first) {
          newdom.clear();
        }  // no preds
        newdom.insert(n);  // rule: dom(n) = {n} U intersect(preds)
        if (newdom != dom[n]) {
          dom[n] = std::move(newdom);
          changed = true;
        }
      }
    }
    return dom;
  }

  AdjMap fullAdjacency() const {
    AdjMap adj;
    adj.reserve(in_deg_.size());
    for (const auto& [v, _] : in_deg_) {
      auto it = adj_list_.find(v);
      if (it != adj_list_.end()) {
        adj.emplace(v, it->second);
      } else {
        adj.emplace(v, std::vector<T>{});
      }
    }
    return adj;
  }
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
