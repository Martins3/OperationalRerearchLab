#ifndef TABU_H
#define TABU_H
#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include <vector>

class Edge {
public:
  int x;
  int y;
  int head;
  Edge() {}
  Edge(int _x, int _y, int _head) : x(_x), y(_y), head(_head) {}
};

class Person {
public:
  int conflict_num;
  std::vector<std::set<unsigned int>> solution; // 每一个颜色中间的节点数

  Person(){};

  /**
   * N 节点总数
   * K 颜色个数
   */
  Person(const unsigned int N, const unsigned int K,
         const std::vector<unsigned int> &solutions, const int _conflict_num)
      : conflict_num(_conflict_num) {

    solution = std::vector<std::set<unsigned int>>(K, std::set<unsigned int>());
    for (unsigned int i = 1; i <= N; i++) {
      solution[solutions[i]].insert(i);
    }
  }
};

/**
 * 用于在 find_move 和 make_move 之间交换数据
 * 这个设计很蠢
 */
class TabuMove {
public:
  int vertex; // 发生修改的节点
  int from;   // 原来的颜色
  int to;     // 修改的颜色
  int delta;  // 修改之后，冲突数的变化

  const static int INF = 0x3f3f3f3f;
  TabuMove(int _u, int _vi, int _vj, int _delta)
      : vertex(_u), from(_vi), to(_vj), delta(_delta) {}

  TabuMove() : vertex(0), from(0), to(0), delta(INF) {}
};

class Tabu {
private:
  // runtime data
  std::vector<unsigned int> solutions; // 节点的颜色
  std::vector<unsigned int> best_solution;
  std::vector<std::vector<unsigned int>> tabu_tenure;
  std::vector<std::vector<unsigned int>> color_conflict_num;
  int conflict_num;
  int min_conflict_num;

  // config data
  unsigned int N; // 节点数
  unsigned int K; // 颜色数量
  unsigned int max_iter; // 迭代次数限制，防止出现死循环

  void initialization(bool is_he,
                      const std::vector<std::set<unsigned int>> &config);
  void find_move(TabuMove &tabu_move, unsigned int iter);
  void make_move(TabuMove &tabu_move, unsigned int iter);
  void update_adjacent_table(int u, int vi, int vj);

  // graph
  std::vector<int> nodeHead;
  int nodePointer;
  std::vector<Edge> graph;

  void add_edge(int x, int y);
  void init_graph(int vertex_num, int edge_num);
  void make_graph();

  // Hybrid Evolutionary
  void cross_over(std::vector<std::set<unsigned int>> & config_one,
                  std::vector<std::set<unsigned int>> & config_two,
                  std::vector<std::set<unsigned int>> &offspring);
  void tabu_search(const std::vector<std::set<unsigned int>> &config,
                   int iter_times, int break_line);
  unsigned int population_size;
  std::vector<Person> populations;
  void solution_to_config(const std::vector<unsigned int> &solution,
                          std::vector<std::set<unsigned int>> &config);
  void config_to_solution(const std::vector<std::set<unsigned int>> &config,
                          std::vector<unsigned int> &solution);

  // some tools
  void print_runtime_ds() const;
  void print_graph();
  void archive_result();

  // save population and read populations
  void save_populations();
  void load_populations();

public:
  Tabu(std::string data_version);
  void tabu_search(int K, int max_iter);
  void hybrid_evolutionary(int K, bool load, int population_size);
};
#endif // !ABU_H
