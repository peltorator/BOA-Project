#include <bits/stdc++.h>

struct Distance {
    long long g1;
    long long h1;
    long long f1;
    long long g2;
    long long h2;
    long long f2;

    Distance(const long long g1, const long long h1, const long long g2, const long long h2): g1(g1), h1(h1), f1(g1 + h1), g2(g2), h2(h2), f2(g2 + h2) {}

    Distance(): g1(0), h1(0), f1(0), g2(0), h2(0), f2(0) {}

    bool operator<=(const Distance& other) const {
        return f1 <= other.f1 && f2 <= other.f2;
    }

    bool operator<(const Distance& other) const {
        return f1 <= other.f1 && f2 <= other.f2 && (f1 < other.f1 || f2 < other.f2);
    }

    bool operator>=(const Distance& other) const {
        return f1 >= other.f1 && f2 >= other.f2;
    }

    bool operator>(const Distance& other) const {
        return f1 >= other.f1 && f2 >= other.f2 && (f1 > other.f1 || f2 > other.f2);
    }
};

struct ParetoSet {
    std::list<std::pair<long long, long long>> paretoSet;
    long long g2min = std::numeric_limits<long long>::max();

    bool dominates(const long long val) const {
        return g2min <= val;
    }

    void add(const Distance& dist) {
        if (g2min > dist.g2) {
            paretoSet.push_back({dist.g1, dist.g2});
            g2min = dist.g2;
        }
    }
};

struct Edge {
    int from;
    int to;
    std::pair<long long, long long> cost;

    Edge(const int from, const int to, const std::pair<long long, long long>& cost): from(from), to(to), cost(cost) {}
};

struct Graph {
    std::vector<std::list<Edge>> adjacencyList;

    Graph(const int n) {
        adjacencyList.resize(n);
    }

    void addEdge(const Edge& edge) {
        adjacencyList[edge.from].emplace_back(edge);
    }

    const std::list<Edge>& getVertexAdjacencyList(const int v) const {
        return adjacencyList[v];
    }
};

struct Node {
    int index;
    Distance dist;

    Node(): index(0), dist(Distance(0, 0, 0, 0)) {}

    Node(const int index, const Distance& dist): index(index), dist(dist) {}

    bool operator<(const Node& other) const {
        return dist < other.dist;
    }

    bool operator<=(const Node& other) const {
        return dist <= other.dist;
    }

    bool operator>(const Node& other) const {
        return dist > other.dist;
    }

    bool operator>=(const Node& other) const {
        return dist >= other.dist;
    }
};

struct NodeComparator {
    bool operator() (const Node& first, const Node& second) const {
        return std::make_pair(first.dist.f1, first.dist.f2) > std::make_pair(second.dist.f1, second.dist.f2);
    }
};

ParetoSet BOAStar(const int n, const Graph& graph, const int source, const int target, const std::vector<std::pair<int, int>>& coordinates, std::function<int(std::pair<int, int>, std::pair<int, int>)> heuristic1, std::function<int(std::pair<int, int>, std::pair<int, int>)> heuristic2) {
    auto h1 = [&] (const int idx) { return heuristic1(coordinates[idx], coordinates[target]); };
    auto h2 = [&] (const int idx) { return heuristic2(coordinates[idx], coordinates[target]); };
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSet;
    openSet.push(Node(source, Distance(0, h1(source), 0, h2(source))));
    std::vector<ParetoSet> paretoSets(n);

    while (!openSet.empty()) {
        Node curNode = openSet.top();
        int v = curNode.index;
        Distance curDist = curNode.dist;
        openSet.pop();
        if (paretoSets[v].dominates(curDist.g2) || paretoSets[target].dominates(curDist.f2)) {
            continue;
        }
        paretoSets[v].add(curDist);
        for (const Edge& e : graph.getVertexAdjacencyList(v)) {
            int u = e.to;
            Distance newDist(curDist.g1 + e.cost.first, h1(u), curDist.g2 + e.cost.second, h2(u));
            if (!paretoSets[u].dominates(newDist.g2) && !paretoSets[target].dominates(newDist.f2)) {
                openSet.push(Node(u, newDist));
            }
        }
    }
    return paretoSets[target];
}

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}

int main() {
    std::string mapName = "NY";
    
    std::ifstream coordf("maps/" + mapName + "/coordinates.txt"); 

    int n;
    coordf >> n;
    std::vector<std::pair<int, int>> coordinates(n);
    for (int i = 0; i < n; i++) {
        int index;
        coordf >> index;
        index--;
        assert(index == i);
        coordf >> coordinates[index].first >> coordinates[index].second;
    }
    coordf.close();

    Graph graph(n);
    long double maxspeed = 0;
    long double maxmult = 0;

    std::ifstream distf("maps/" + mapName + "/distances.txt");
    std::ifstream timef("maps/" + mapName + "/time.txt");
    int m;
    distf >> m;
    for (int i = 0; i < m; i++) {
        int from, to;
        long long length;
        distf >> from >> to >> length;
        int from2, to2;
        long long time;
        timef >> from2 >> to2 >> time;
        assert(from2 == from && to2 == to);

        from--;
        to--;
        long long dx = coordinates[from].first - coordinates[to].first;
        long long dy = coordinates[from].second - coordinates[to].second;
        if (length != 0) {
            maxmult = std::max(maxmult, sqrtl(dx * dx + dy * dy) / static_cast<long double>(length));
        }
        if (time != 0) {
            maxspeed = std::max(maxspeed, static_cast<long double>(length) / static_cast<long double>(time));
        }
        graph.addEdge(Edge(from, to, {length, time}));
    }
    distf.close();
    timef.close();
    //std::string heuristicName = "euclid";
    std::string heuristicName = "no_heurist";
    //std::string heuristicName = "chebyshev";

    auto h1 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return 0;
        //long double dx = a.first - b.first;
        //long double dy = a.second - b.second;
        //return floor(sqrtl(dx * dx + dy * dy) / maxmult);
        //return std::max(a.first - b.first, a.second - b.second) / maxmult;
    };
    auto h2 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    const int TESTCASES = 100;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/BOAStar_" + heuristicName + ".txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    std::vector<double> times;
    for (int i = 0; i < TESTCASES; i++) {
        int source = rnd() % n, target = rnd() % n;
        std::cerr << "Starting BOAStar search. Map: " << mapName << ", Heuristic: " << heuristicName << " Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansBOAStar = BOAStar(n, graph, source, target, coordinates, h1, h2);

        double workTime = GetCurTime() - startTime;
        times.push_back(workTime);
        std::cerr << "Current task work time = " << workTime << std::endl;
        sumTime += workTime;
        sumAnsSize += ansBOAStar.paretoSet.size();
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;
        std::cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << std::endl;

        ansBOAStar.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const std::pair<int, int>& dist : ansBOAStar.paretoSet) {
            outp << dist.first << " " << dist.second << '\n';
        }
        outp << "\n\n\n";
    }
    std::sort(times.begin(), times.end());
    std::cerr << "\n\nResults for BOAStar with heuristic '" << heuristicName << "' on map '" << mapName << "'\n";
    std::cerr << "Final average time per task: " << sumTime / TESTCASES << std::endl;
    std::cerr << "Min time per task: " << times[0] << std::endl;
    std::cerr << "Max time per task: " << times.back() << std::endl;
    std::cerr << "Median time per task: " << times[TESTCASES / 2] << std::endl;
    std::cerr << "95 time percentile: " << times[TESTCASES * 0.95] << std::endl;
    std::cerr << "90 time percentile: " << times[TESTCASES * 0.9] << std::endl;
    std::cerr << "80 time percentile: " << times[TESTCASES * 0.8] << std::endl;
    std::cerr << "Final average Pareto set size per task: " << sumAnsSize / TESTCASES << " (sum of sizes is " << sumAnsSize << ")" << std::endl;
}
