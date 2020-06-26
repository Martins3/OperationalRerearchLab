#include <iostream>
#include <vector>
#include <cmath>
#include <stack>
#include <sstream>
#include <climits>
#include <deque>
#include <set>
#include <utility>
#include <queue>
#include <map>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <string>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include "./tabu.h"


using namespace std;
int main(){
    std::clock_t start;
    double duration;
    start = std::clock();

    // 测试编译
    Tabu tabu("1000.1");
    // tabu.tabu_search(21, 1000 * 10000);

    tabu.hybrid_evolutionary(25, false, 5);
    // tabu.hybrid_evolutionary(49, false, 5);


    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Time cost in seconds: "<< duration <<'\n';
    return 0;
}
