#include "./tabu.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <cstdio>
#include <set>
#include <cassert>

using namespace std;

void Tabu::initialization(bool is_he, const vector<set<unsigned int> >& config){
    // init run time data structures
    solutions = vector<unsigned int>(N + 1, 0);
    best_solution = vector<unsigned int>(N + 1, 0);
    tabu_tenure = vector<vector<unsigned int> >(N + 1, vector<unsigned int>(K, 0));
    color_conflict_num = vector<vector<unsigned int> >(N + 1, vector<unsigned int>(K, 0));
    conflict_num = 0;
    min_conflict_num = 0;

    // generate initial value
    if(is_he){
        for(size_t i = 0; i < config.size(); i ++){
            for(unsigned int v: config[i]){
                solutions[v] = i;
            }
        }
    }else{
        for(size_t i = 1; i <= N; i++)
            solutions[i] = (unsigned int)rand() % K;
    }

    for(unsigned int x = 1; x <= N; x++){
        for(int i = nodeHead[x]; i != -1; i = graph[i].head){
            int y = graph[i].y;
            color_conflict_num[x][solutions[y]] ++;
        }
    }


    for(size_t i = 1; i <= N; i++){
        conflict_num += color_conflict_num[i][solutions[i]];
    }
    if(is_he){
        printf("after cross over, conflict %d\n", conflict_num);
    }
    min_conflict_num =  conflict_num;
    // print_runtime_ds();
}


void Tabu::tabu_search(int K, int max_iter){
    this->max_iter = max_iter;
    this->K = K;
    vector<set<unsigned int> > t;
    initialization(false, t);
    cout << "initialization over, begin tabu search" << endl;

    int iter = 0;
    TabuMove tm;

    while(conflict_num != 0){
        assert(conflict_num >=0);
        find_move(tm, iter);
        make_move(tm, iter);
        iter ++;
        if(iter > max_iter) break;

        if(iter % 10000 == 0){
            printf("iter %u conflict %d\n", iter, conflict_num);
        }
    }
    archive_result();
}


void Tabu::archive_result(){
    if(conflict_num == 0){
        printf("K is %d\n", K);
        printf("the ans is\n");
        for(size_t i = 0; i < N; i++){
            printf("%d ", solutions[i]);
            if(i % 10 == 0 && i != 0) cout << endl;
        }
    }
}

void Tabu::tabu_search(const vector<set<unsigned int> >& config, int iter_time, int break_line){
    initialization(true, config);

    int iter = 0;
    TabuMove tm;
    while(conflict_num){
        find_move(tm, iter);
        make_move(tm, iter);
        iter ++;
        if(iter > iter_time) break;
        if(min_conflict_num < break_line) return;

        if(iter % 10000 == 0){
            printf("iter %u conflict %d\n", iter, conflict_num);
        }
    }
}


void Tabu::find_move(TabuMove& tabu_move, unsigned int iter){
    TabuMove tabu_best_move;
    TabuMove non_tabu_best_move;
    // 询问所有的顶点
    for(int i = 1; i <= N; i++){
        // 顶点i的现在的冲突不为0
        if(color_conflict_num[i][solutions[i]] != 0){
            // 选择最佳的切换颜色
            for(int k = 0; k < K; k++){
                if(k == solutions[i]) continue;

                int delta = (int)color_conflict_num[i][k] - (int)color_conflict_num[i][solutions[i]];
                if(iter < tabu_tenure[i][k]){
                    if(delta < tabu_best_move.delta)
                        tabu_best_move = TabuMove(i, solutions[i], k, delta);
                }else{
                    if(delta < non_tabu_best_move.delta)
                        non_tabu_best_move = TabuMove(i, solutions[i], k, delta);
                }
            }
        }
    }

    // when tabu is worse and non-tabu is better, use non-tabu
    bool is_aspiration = (tabu_best_move.delta < 0 && non_tabu_best_move.delta > 0);
    if(is_aspiration)
        tabu_move = tabu_best_move;
    else
        tabu_move = non_tabu_best_move;

}

void Tabu::make_move(TabuMove& tabu_move, unsigned int iter){
    int u = tabu_move.vertex;
    int vi = tabu_move.from;
    int vj = tabu_move.to;
    int delta = tabu_move.delta;

    // change the color
    solutions[u] = vj;
    // update current conflict nums
    conflict_num = conflict_num + delta * 2;
    // update the best conflict num
    if(conflict_num < min_conflict_num){
        min_conflict_num = conflict_num;
        best_solution = solutions;
    }
    // update tabu tenure
    tabu_tenure[u][vi] = iter + conflict_num + (unsigned int)rand()%10;
    // update the conflict_color_table;
    for(int i = nodeHead[u]; i != -1; i = graph[i].head){
        int y = graph[i].y;
        assert(color_conflict_num[y][vi]);
        color_conflict_num[y][vi] --;
        color_conflict_num[y][vj] ++;
    }
}



void Tabu::make_graph(){
    cout << "begin to make graph" << endl;
    size_t max_len = 100;
    char line[max_len];
    while(true){
        fgets(line, max_len, stdin);
        if(line[0] != 'c') break;
    }
    unsigned int vertex_num;
    unsigned int edge_num;
    sscanf(line + 7, "%u %u", &vertex_num, &edge_num);
    printf("vertex num %u\nedge num %u\n", vertex_num, edge_num);

    init_graph(vertex_num, edge_num);
    this->N = vertex_num;

    for(size_t i = 0; i < edge_num; i++){
        fgets(line, max_len, stdin);
        int x;
        int y;
        sscanf(line + 1, "%d %d", &x, &y);
        add_edge(x, y);
        add_edge(y, x);
    }
    // print_graph();
}

void Tabu::print_graph(){
    for(size_t x = 1; x <= N; x++){
        printf("%lu : ", x);
        for(int i = nodeHead[x]; i != -1; i = graph[i].head){
            printf("%d ", graph[i].y);
        }
        printf("\n");
    }
}

void Tabu::add_edge(int x, int y){
   graph[nodePointer] = Edge(x, y, nodeHead[x]);
   nodeHead[x] = nodePointer;
   nodePointer ++;
}

void Tabu::init_graph(int vertex_num, int edge_num) {
    vertex_num += 10; edge_num += 10; edge_num *= 2;
    this->nodeHead = vector<int>(vertex_num, -1);
    this->graph =  vector<Edge>(edge_num);
    nodePointer = 0;
    cout << "init graph data structure finished !" << endl;
}


Tabu::Tabu(std::string data_version){
    // choose a right version to make the graph
    string path = "./DSJC";
    path += data_version;
    string logpath = "HEA_Log_" + data_version + ".txt";
    path += ".col.txt";
    freopen(path.c_str(), "r", stdin);
    freopen(logpath.c_str(), "w", stdout);
    cout << "test file is  " << path << endl;
    make_graph();
}


void Tabu::cross_over(vector<set<unsigned int> > & solution_one, vector<set<unsigned int> > & solution_two,
    vector<set<unsigned int> >& offspring){
        int os_size = 0;

        // 对于所有的颜色
        for(size_t i = 0; i < K; i++){
            if(i % 2){
                unsigned int max_v = 0;
                int max_i = -1;
                // 找到一个节点数最多的点
                for(size_t j = 0; j < K; j++){
                    const set<unsigned int> & s = solution_one[j];
                    if(s.size() > max_v){
                        max_v = s.size();
                        max_i = j;
                    }
                }
                // 如果没有，那么不要加入节点
                if(max_i == -1){
                    offspring.push_back(set<unsigned int>());
                    continue;
                }
                set<unsigned int> & purge = solution_one[max_i];
                os_size += purge.size();
                offspring.push_back(purge);
                // 其他颜色就不要包含这些选中的点了
                for(unsigned int v:purge){
                    for(set<unsigned int>& s : solution_one){
                        s.erase(v);
                    }
                    for(set<unsigned int>& s : solution_two){
                        s.erase(v);
                    }
                }
                solution_one[max_i].clear();
            }else{
                unsigned int max_v = 0;
                int max_i = -1;
                for(size_t j = 0; j < K; j++){
                    const set<unsigned int> & s = solution_two[j];
                    if(s.size() > max_v){
                        max_v = s.size();
                        max_i = j;
                    }
                }

                if(max_i == -1){
                    offspring.push_back(set<unsigned int>());
                    continue;
                }

                set<unsigned int> & purge = solution_two[max_i];
                os_size += purge.size();
                offspring.push_back(purge);
                for(unsigned int v:purge){
                    for(set<unsigned int>& s : solution_one){
                        s.erase(v);
                    }
                    for(set<unsigned int>& s : solution_two){
                        s.erase(v);
                    }
                }
            }
        }

        // 将的conflg_one and config_two 剩余数值随机添加到其中
        set<unsigned int> residual;
        for(const set<unsigned int> & s : solution_one){
            residual.insert(s.begin(), s.end());
        }
        for(const set<unsigned int> & s : solution_two){
            residual.insert(s.begin(), s.end());
        }
        for(unsigned int num : residual){
            int k = rand() % K;
            offspring[k].insert(num);
        }
 }


void Tabu::hybrid_evolutionary(int K, bool load, int population_size){
    // 设置数值 K
    this->K = K;
    printf("hybrid evolutionary start\n");
    this->population_size = population_size;
    vector<set<unsigned int> > t_config;
    int best_person = TabuMove::INF; // 记录population 中间的 最佳的数值

    if(load){
        load_populations();
    }else{
        for(int i = 0; i < population_size; i++){
            tabu_search(K, 5 * 10000);
            populations.emplace_back(N, K, solutions, conflict_num);
            best_person = min(best_person, conflict_num);
            printf("the person %i has conflicts %d\n\n", i, conflict_num);
        }
        save_populations();
    }

    // tabu search 的中间　solutions 只有在进入到 tabu_search 的使用才会使用的
    while(best_person){
        int s1 = (unsigned int)rand() % population_size;
        int s2 = (unsigned int)rand() % population_size;
        while(s1 == s2) s2 = (unsigned int)rand() % population_size;

        // 使用 break_line 
        int break_line = 0;
        for(int  i = 0; i < population_size; i++){
            break_line = max(break_line, populations[i].conflict_num);
        }

        vector<set<unsigned int> > offspring;
        cross_over(populations[s1].solution, populations[s2].solution, offspring);
        tabu_search(offspring, 50 * 10000, break_line);


        Person s0 = Person(N, K, best_solution, min_conflict_num);

        best_person = min(best_person, s0.conflict_num);

        int worst_person = s0.conflict_num;
        int worst_index = -1;
        for(size_t i = 0; i < population_size; i++){
            if(populations[i].conflict_num > worst_person){
                worst_index = i;
                worst_person = populations[i].conflict_num;
            }
        }

        for(size_t i = 0; i < population_size; i++){
            printf("%d ", populations[i].conflict_num);
        }
        printf("\n");

        printf("%d is out, conflict is %d \n", worst_index, best_person);
        if(worst_index != -1) populations[worst_index] = s0;
    }

    // 记录最佳的输出
    if(best_person == 0){
        printf("K is %d\n", K);
        vector<unsigned int> solution(N);
        config_to_solution(populations[0].solution, solution);
        for(size_t i = 0; i < N; i++){
            printf("%d ", solutions[i]);
        }
        cout << endl;
    }

}

void Tabu::solution_to_config(const vector<unsigned int> & solution, vector<set<unsigned int> >& config){
    for(size_t i = 1; i <= N; i++){
        config[solution[i]].insert(i);
    }
}

void Tabu::config_to_solution(const vector<set<unsigned int> >& config, vector<unsigned int>& solutions){
    for(size_t i = 0; i < K; i++){
        const set<unsigned int> & s = config[i];
        for(int n : s){
            solutions[n] = i;
        }
    }
}

void Tabu::print_runtime_ds() const{
    printf("conflict num %d\n", conflict_num);
    printf("solutions\n");
    for(size_t i = 0; i < solutions.size(); i++){
        printf("%u ", solutions[i]);
    }
    printf("\n");

    printf("adjacent color table\n");
    for(size_t i = 0; i < color_conflict_num.size(); i++){
        for(size_t j = 0; j < color_conflict_num[i].size(); j++){
            printf("%u ", color_conflict_num[i][j]);
        }
        printf("\n");
    }

    printf("tabu_tenure\n");
    int I = tabu_tenure.size();
    int J = tabu_tenure[0].size();
    for(int i = 1; i < I; i++){
        int j;
        for(j = 0; j < J; j++){
            printf("%u ", tabu_tenure[i][j]);
        }
        printf("\n");
    }
    printf("log finished !\n");
}

void Tabu::save_populations(){
    ofstream file;
    file.open ("./populations.txt");
    for(const Person & person : populations){
        file << person.conflict_num << " ";
        vector<unsigned int> sol = vector<unsigned int>(N + 1);
        config_to_solution(person.solution,  sol);
        for(size_t i = 0; i < N; i++){
            file << sol[i] << " ";
        }
        file << endl;
    }
    file.close();
}

void Tabu::load_populations(){
    freopen("./populations.txt", "r", stdin);
    vector<unsigned int> sol;
    int conf;
    for(size_t i = 0; i < population_size; i++){
        scanf("%d", &conf);
        for(size_t i = 0; i < N; i++){
            int t;
            scanf("%d", &t);
            sol.push_back(t);
        }
        populations.emplace_back(N, K, sol, conf);
        sol.clear();
    }
}
