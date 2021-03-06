#ifndef BRUTEFORCE_RECOVER_TREE_ALGORITHM_HPP_
#define BRUTEFORCE_RECOVER_TREE_ALGORITHM_HPP_

#include <unordered_set>

#include "recover_tree_algorithm.hpp"
#include "structures/branch.hpp"
#include "graph/graph_pack.hpp"
#include "structures/mcolor_hash.hpp"
#include "statistics_based_recover_tree_algorithm.hpp"

namespace algo {

  template <class graph_pack_t>
  struct BruteforceRecoverTreeAlgorithm :
      StatisticsBasedRecoverTreeAlgorithm<graph_pack_t> {
    using mcolor_t = typename RecoverTreeAlgorithm<graph_pack_t>::mcolor_t;
    using tree_t = typename RecoverTreeAlgorithm<graph_pack_t>::tree_t;
    using node_t = typename tree_t::colored_node_t;
    using node_ptr = std::shared_ptr<node_t>;
    using node_vector = std::vector<node_ptr>;
    using tree_ptr = typename RecoverTreeAlgorithm<graph_pack_t>::tree_ptr;
    using tree_vector = typename RecoverTreeAlgorithm<graph_pack_t>::tree_vector;
    using branch_t = std::pair<mcolor_t, mcolor_t>;
    using branch_vector = std::vector<branch_t>;
    using class_t = std::pair<branch_vector, size_t>;
    using class_vector = std::vector<class_t>;
    using statistic_t = std::pair<branch_t, size_t>;
    using statistic_vector = std::vector<statistic_t>;
    using BranchHelper = structure::Branch<mcolor_t>;
    using color_set = std::unordered_set<mcolor_t>;
    using statistic_producer_ptr = std::shared_ptr<StatisticsProducer<graph_pack_t> > const&;


    BruteforceRecoverTreeAlgorithm(graph_pack_t& graph_pack,
                                   statistic_producer_ptr statistic_producer) :
        StatisticsBasedRecoverTreeAlgorithm<graph_pack_t>(graph_pack, statistic_producer) {
    }

    tree_vector recover_trees() {
      auto statistics = this->m_statistic_producer->make_statistics();
      return build_trees(statistics);
    }

  private:
    /**
    * Checks if the branch conflicts the class of branches, adds it if not
    * @return true if the branch was added
    */
    bool screen_branch(class_t& cls, statistic_t const& statistic) {
      auto const& new_branch = statistic.first;
      bool new_branch_conflicts = false;
      for (auto& branch: cls.first) {
        if (BranchHelper::do_intersect(branch, new_branch)) {
          new_branch_conflicts = true;
          break;
        }
      }
      if (!new_branch_conflicts) {
        cls.first.push_back(new_branch);
        cls.second += statistic.second;
        return true;
      }
      return false;
    }

    tree_vector build_trees(statistic_vector& branch_statistics) {
      class_vector tree_classes;

      for (size_t i = 0; i != branch_statistics.size(); ++i) {
        auto& statistic = branch_statistics[i];
        bool has_been_added = false;
        for (auto& cls: tree_classes) {
          bool screening_result = screen_branch(cls, statistic);
          has_been_added = has_been_added || screening_result;
        }
        if (!has_been_added) {
          tree_classes.push_back(class_t({statistic.first}, statistic.second));
          class_t& new_class = tree_classes.back();

          // So we won't forget to add the branches already screened, but consistent with new class
          for (size_t j = 0; j != i; ++j) {
            screen_branch(new_class, branch_statistics[j]);
          }
        }
      }

      if (cfg::get().is_debug) {
        for (size_t i = 0; i != tree_classes.size(); ++i) {
          auto cls = tree_classes[i];
          std::clog << "Class " << i << " : " << cls.second << std::endl;
          for (auto& branch: cls.first) {
            std::clog << BranchHelper::branch_to_string(branch) << std::endl;
          }
        }
      }

      std::sort(std::begin(tree_classes), std::end(tree_classes),
          // Descending by class score
                [](class_t const& left, class_t const& right) {
                  return left.second > right.second;
                });

      // Collect the colors appearing in the tree to speed up the strictness checking
      color_set appearing_colors;
      for (auto const& statistic: branch_statistics) {
        auto left_color = statistic.first.first;
        appearing_colors.insert(left_color);
      }

      tree_vector results;
      results.reserve(tree_classes.size());
      std::transform(std::begin(tree_classes), std::end(tree_classes), std::back_inserter(results),
                     [&appearing_colors](class_t& cls_to_fold) {
                       auto root_node = BranchHelper::fold_into_root_node(cls_to_fold.first);
                       // FIXME: add pruning!
                       does_need_to_prune_node(root_node, appearing_colors);
                       return std::make_shared<tree_t>(root_node);
                     });

      return results;
    }

    /**
    * Removes the unnecessary children from the node
    * @return true if the node needs to be pruned itself
    */
    static bool does_need_to_prune_node(node_ptr const& node, color_set const& appearing_colors) {
      // Node needs to be pruned in two cases:
      // 1. It doesn't appear in statistics set
      // 2. All its children have to be pruned

      if (node->is_leaf()) {
        if (node->is_simple()) {
          return appearing_colors.count(node->get_data()) == 0;
        }
        return true;
      }
      bool both_children_appear_in_statistics =
          appearing_colors.count(node->get_left_child()->get_data()) != 0 &&
          appearing_colors.count(node->get_right_child()->get_data()) != 0;
      bool needs_to_be_pruned =
          !both_children_appear_in_statistics &&
          does_need_to_prune_node(node->get_left_child(), appearing_colors) &&
          does_need_to_prune_node(node->get_right_child(), appearing_colors);
      return needs_to_be_pruned;
    }
  };
}

#endif