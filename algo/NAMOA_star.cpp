#include <bits/stdc++.h>

using namespace std;

struct Distance {
    int g1;
    int h1;
    int f1;
    int g2;
    int h2;
    int f2;

    Distance(const int g1, const int h1, const int g2, const int h2): g1(g1), h1(h1), f1(g1 + h1), g2(g2), h2(h2), f2(g2 + h2) {}

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
    list<pair<int, int>> paretoSet;

    bool dominates(const pair<int, int> dist) const {
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return true;
            }
        }
        return false;
    }

    void add(const pair<int, int> dist) {
        list<pair<int, int>> newParetoSet;
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return;
            } else if (!(paretoDist.first >= dist.first && paretoDist.second >= dist.second)) {
                newParetoSet.push_back(paretoDist);
            }
        }
        newParetoSet.push_back(dist);
        paretoSet = newParetoSet;
    }

    void push(const pair<int, int> g_value) {
        paretoSet.push_back(g_value);
    }

    void remove(const pair<int, int> g_value) {
        paretoSet.remove(g_value);
    }

    void remove_worse(const pair<int, int> dist) {
        list<pair<int, int>> newParetoSet;
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return;
            } else if (!(paretoDist.first >= dist.first
                         && paretoDist.second >= dist.second
                         && (paretoDist.first > dist.first || paretoDist.second > dist.second ))
                    ) {
                newParetoSet.push_back(paretoDist);
            }
        }
        paretoSet = newParetoSet;
    }

    void print() {
        cerr << "ParetoSet" << endl << "ParetoSet size: " << paretoSet.size() << endl << "Paths:" << endl;
        for (const auto paretoDist : paretoSet) {
            cerr << paretoDist.first << ' ' << paretoDist.second << endl;
        }
        cerr << endl;
    }
};

struct Edge {
    int from;
    int to;
    pair<int, int> cost;

    Edge(const int from, const int to, const pair<int, int>& cost): from(from), to(to), cost(cost) {}
};

struct Graph {
    vector<list<Edge>> adjacencyList;

    Graph(const int n) {
        adjacencyList.resize(n);
    }

    void addEdge(const Edge& edge) {
        adjacencyList[edge.from].emplace_back(edge);
    }

    const list<Edge>& getVertexAdjacencyList(const int v) const {
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
        return make_pair(first.dist.f1, first.dist.f2) <= make_pair(second.dist.f1, second.dist.f2);
    }
};

ParetoSet NAMOA_star(const int n,
                     const Graph& graph,
                     const int start,
                     const int goal,
                     const vector<pair<int, int>>& coords,
                     function<int(pair<int, int>, pair<int, int>)> heuristic1,
                     function<int(pair<int, int>, pair<int, int>)> heuristic2) {

    auto h1 = [&] (const int idx) { return heuristic1(coords[idx], coords[goal]); };
    auto h2 = [&] (const int idx) { return heuristic2(coords[idx], coords[goal]); };

    ParetoSet sols;
    vector<ParetoSet> Gop(n);
    vector<ParetoSet> Gcl(n);
    Gop[start].add({0, 0});

    set<Node, NodeComparator> Open;
    Open.insert(Node(start, Distance(0, h1(start), 0, h2(start))));

    while (!Open.empty()) {
        Node s = *Open.begin();
        int v = s.index;

        Distance curDist = s.dist;
        pair<int, int> g_value = {s.dist.g1, s.dist.g2};
        Open.erase(Open.begin());

        Gop[v].remove(g_value);
        Gcl[v].push(g_value);

        if (v == goal) {
            sols.add(g_value);
            set<Node, NodeComparator> newOpen;
            for (auto u: Open) {
                if (!(s < u)) {
                    newOpen.insert(u);
                }
            }
            Open = newOpen;
            continue;
        }

        for (const Edge& e : graph.getVertexAdjacencyList(v)) {
            int u = e.to;
            Distance newDist(curDist.g1 + e.cost.first, h1(u), curDist.g2 + e.cost.second, h2(u));

            if (Gop[u].dominates({newDist.g1, newDist.g2})) {
                continue;
            }

            if (Gcl[u].dominates({newDist.g1, newDist.g2})) {
                continue;
            }

            if (sols.dominates({newDist.f1, newDist.f2})) {
                continue;
            }

            Gcl[u].remove_worse({newDist.g1, newDist.g2});
            Gop[u].add({newDist.g1, newDist.g2});

            Open.insert(Node(u, newDist));
        }
    }
//    sols.print();
    return sols;
}

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}

void solveForMap(std::string mapName, std::string heuristicName, std::ofstream& res) {
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

    std::function<int(std::pair<int, int>, std::pair<int, int>)> euclidHeuristic = [&](std::pair<int, int> a, std::pair<int, int> b) {
        long double dx = a.first - b.first;
        long double dy = a.second - b.second;
        return floor(sqrtl(dx * dx + dy * dy) / maxmult);
    };
    std::function<int(std::pair<int, int>, std::pair<int, int>)> chebyshevHeuristic = [&](std::pair<int, int> a, std::pair<int, int> b) {
        return std::max(a.first - b.first, a.second - b.second) / maxmult;
    };
    std::function<int(std::pair<int, int>, std::pair<int, int>)> noHeuristic = [&](std::pair<int, int> a, std::pair<int, int> b) {
        return 0;
    };

    auto h1 = (heuristicName == "euclid" ? euclidHeuristic : (heuristicName == "chebyshev" ? chebyshevHeuristic : noHeuristic));

    auto h2 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    const int TESTCASES = 40;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/NAMOA_star_" + heuristicName + ".txt");
    std::ofstream runtimes("results/" + mapName + "/NAMOA_star_" + heuristicName + "_runtimes.txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    std::vector<double> times;
    for (int i = 0; i < TESTCASES; i++) {
        int source = rnd() % n, target = rnd() % n;
        std::cerr << "Starting NAMOA_star search. Map: " << mapName << ", Heuristic: " << heuristicName << " Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansNAMOA_star = NAMOA_star(n, graph, source, target, coordinates, h1, h2);

        double workTime = GetCurTime() - startTime;
        times.push_back(workTime);
        runtimes << i << ' ' << workTime << '\n';
        std::cerr << "Current task work time = " << workTime << std::endl;
        sumTime += workTime;
        sumAnsSize += ansNAMOA_star.paretoSet.size();
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;
        std::cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << std::endl;

        ansNAMOA_star.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const std::pair<long long, long long>& dist : ansNAMOA_star.paretoSet) {
            outp << dist.first << " " << dist.second << '\n';
        }
        outp << "\n\n\n";
    }
    std::sort(times.begin(), times.end());
    res << "\n\nResults for NAMOA_star with heuristic '" << heuristicName << "' on map '" << mapName << "'\n";
    res << "Final average time per task: " << sumTime / TESTCASES << std::endl;
    res << "Min time per task: " << times[0] << std::endl;
    res << "Max time per task: " << times.back() << std::endl;
    res << "Median time per task: " << times[TESTCASES / 2] << std::endl;
    res << "95 time percentile: " << times[TESTCASES * 0.95] << std::endl;
    res << "90 time percentile: " << times[TESTCASES * 0.9] << std::endl;
    res << "80 time percentile: " << times[TESTCASES * 0.8] << std::endl;
    res << "Final average Pareto set size per task: " << sumAnsSize / TESTCASES << " (sum of sizes is " << sumAnsSize << ")" << std::endl << std::endl;
}

int main() {
    std::ofstream res("results_namoa_star_cal.txt");
    for (std::string mapName : {"NY", "BAY", "COL"}) {
        for (std::string heuristicName : {"euclid", "chebyshev", "no_heuristic"}) {
            solveForMap(mapName, heuristicName, res);
        }
    }
}