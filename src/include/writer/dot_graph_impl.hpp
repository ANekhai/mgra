//
// Created by Nikita Kartashov on 16/03/2015.
//

#ifndef DOT_GRAPH_IMPL_HPP_
#define DOT_GRAPH_IMPL_HPP_

namespace writer {
  template <class graph_pack_t>
  void GraphDot<graph_pack_t>::save_bp_graph(graph_pack_t const& graph_pack, size_t stage) {
    std::string dotname = m_graphname + std::to_string(stage) + ".dot";
    std::ofstream dot(path::append_path(m_path, dotname));

    dot << "graph {" << std::endl;
    if (!cfg::get().colorscheme.empty()) {
      dot << "edge [colorscheme=" << cfg::get().colorscheme << "];" << std::endl;
    }

    std::unordered_set<vertex_t> processed;
    infinity_verteces = 0;
    for (vertex_t const& x : graph_pack.graph) {
      mularcs_t const& mularcs = graph_pack.get_all_adjacent_multiedges(x);

      if (mularcs.size() == 1 && mularcs.union_multicolors() == graph_pack.multicolors.get_complete_color()) {
        continue;
      }

      for (auto const& edge : mularcs) {
        if (processed.find(edge.first) != processed.end()) {
          continue;
        }

        if (graph_pack.multicolors.is_vec_T_consistent_color(edge.second)) {
          draw_multiedge(dot, x, edge.second, edge.first, "penwidth=3");
        } else if (graph_pack.multicolors.is_T_consistent_color(edge.second)) {
          draw_multiedge(dot, x, edge.second, edge.first, "");
        } else {
          auto split_colors = graph_pack.split_color(edge.second);
          if (split_colors.size() == 2) {
            draw_multiedge(dot, x, *split_colors.cbegin(), edge.first, "style=dashed");
            draw_multiedge(dot, x, *(++split_colors.cbegin()), edge.first, "");
          } else {
            size_t number_splits = graph_pack.split_color(edge.second).size();
            draw_multiedge(dot, x, edge.second, edge.first, "label=" + std::to_string(number_splits), true);
          }
        }
      }
      processed.insert(x);
    }

    for (int i = infinity_verteces; i < 0; ++i) {
      dot << "\t\"" << i << "\"\t[shape=point,color=black];" << std::endl;
    }

    dot << "}" << std::endl;
    dot.close();
  }

  template <class graph_pack_t>
  void GraphDot<graph_pack_t>::draw_multiedge(std::ofstream& dot, vertex_t const& x, mcolor_t const& color, vertex_t const& y,
      std::string const& properties, bool is_label_properties) {
    bool is_write_properties = false;

    for (auto const& match : color) {
      for (size_t i = 0; i < match.second; ++i) {
        dot << "\t\"" << x << "\"\t--\t\"";

        if (y == Infty) {
          if (match == *color.cbegin()) {
            --infinity_verteces;
          }
          dot << infinity_verteces << "\"\t[len=0.75,";
        } else {
          dot << y << "\"\t[";
        }

        if (is_label_properties) {
          if (is_write_properties) {
            dot << "color=" << cfg::get().get_RGBcolor(match.first) << "];" << std::endl;
          } else {
            is_write_properties = true;
            dot << "color=" << cfg::get().get_RGBcolor(match.first)
                << ", " << properties << "];" << std::endl;
          }
        } else {
          dot << "color=" << cfg::get().get_RGBcolor(match.first)
              << ", " << properties << "];" << std::endl;
        }
      }
    }
  }

/**
* Functions for output tree in legend.dot file
*/
  template <class graph_pack_t>
  std::string GraphDot<graph_pack_t>::get_branches_from_tree(std::shared_ptr<const node_t> const& current,
      std::vector<std::string>& info) const {
    auto const& left = current->get_left_child();
    auto const& right = current->get_right_child();

    if (!left && !right) {
      if (!current->get_children().empty()) {
        for (auto const& lc : current->get_children()) {
          info.push_back("\t\"" + current->get_name() + "\" -> \"" + lc + "\";");
        }
      }
      return current->get_name();
    }

    if (left) {
      std::string const& first = get_branches_from_tree(left, info);
      info.push_back("\t\"" + current->get_name() + "\" -> \"" + first + "\";");
    }

    if (right) {
      std::string const& second = get_branches_from_tree(right, info);
      info.push_back("\t\"" + current->get_name() + "\" -> \"" + second + "\";");
    }

    return current->get_name();
  }


  template <class graph_pack_t>
  void GraphDot<graph_pack_t>::save_subtrees() {
    std::ofstream output(path::append_path(m_path, "legend.dot"));
    save_subtrees(output, cfg::get().phylotrees);
    output.close();
  }

  template <class graph_pack_t>
  void GraphDot<graph_pack_t>::save_subtrees(std::vector<tree_t> const& trees) {
    std::ofstream output(path::append_path(m_path, "legend.dot"));
    save_subtrees(output, trees);
    output.close();
  }

  template <class graph_pack_t>
  void GraphDot<graph_pack_t>::save_subtrees(
      std::ostream& output,
      std::vector<tree_t> const& trees) {
    output << "digraph legend {" << std::endl;
    output << "\tnode [style=filled";
    if (!cfg::get().colorscheme.empty()) {
      output << ", colorscheme=" << cfg::get().colorscheme;
    }
    output << "];" << std::endl;

    for (size_t j = 0; j < cfg::get().get_count_genomes(); ++j) {
      output << "\t\"" << cfg::get().get_priority_name(j) << "\" [fillcolor="
          << cfg::get().get_RGBcolor(j) << "];" << std::endl;
    }

    std::vector<std::string> info;
    for (auto const& tree : trees) {
      get_branches_from_tree(tree.get_root(), info);
    }

    for (auto const& str : info) {
      output << str << std::endl;
    }

    output << "}" << std::endl;
  }
}

#endif //_MGRA_GRAPH_DOT_IMPL_H_
