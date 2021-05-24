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

    int remove_worse(const pair<int, int> dist) {
        int gmin = -1;
        list<pair<int, int>> newParetoSet;
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return -1;
            } else if (!(paretoDist.first >= dist.first
                         && paretoDist.second >= dist.second
                         && (paretoDist.first > dist.first || paretoDist.second > dist.second ))
                    ) {
                newParetoSet.push_back(paretoDist);
                if (gmin == -1 || gmin > paretoDist.second) {
                    gmin = paretoDist.second;
                }
            }
        }
        paretoSet = newParetoSet;
        return gmin;
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

ParetoSet NAMOA_star_dr(const int n,
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

    vector<int> Gcl_gmin(n, -1);
    int sols_gmin = -1;

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
        if (Gcl_gmin[v] == -1 || Gcl_gmin[v] > g_value.second) {
            Gcl_gmin[v] = g_value.second;
        }

        if (v == goal) {
            if (sols_gmin == -1 || sols_gmin > g_value.second) {
                sols_gmin = g_value.second;
                sols.add(g_value);
            }
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

            if (Gcl_gmin[u]!= -1 && Gcl_gmin[u] < newDist.g2) {
                continue;
            }

            if (sols_gmin != -1 && sols_gmin < newDist.f2) {
                continue;
            }

            Gop[u].push({newDist.g1, newDist.g2});

            Open.insert(Node(u, newDist));
        }
    }
    sols.print();
    return sols;
}

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}

int main()
{
    string mapName = "NY";

    ifstream coordf("maps/" + mapName + "/coordinates.txt");

    int n;
    coordf >> n;
    vector<pair<int, int>> coordinates(n);
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

    ifstream distf("maps/" + mapName + "/distances.txt");
    ifstream timef("maps/" + mapName + "/time.txt");
    int m;
    distf >> m;
    for (int i = 0; i < m; i++) {
        int from, to, length;
        distf >> from >> to >> length;
        int from2, to2, time;
        timef >> from2 >> to2 >> time;
        assert(from2 == from && to2 == to);

        from--;
        to--;
        long long dx = coordinates[from].first - coordinates[to].first;
        long long dy = coordinates[from].second - coordinates[to].second;
        if (length != 0) {
            maxmult = max(maxmult, sqrtl(dx * dx + dy * dy) / static_cast<long double>(length));
        }
        if (time != 0) {
            maxspeed = max(maxspeed, static_cast<long double>(length) / static_cast<long double>(time));
        }
        graph.addEdge(Edge(from, to, {length, time}));
    }
    distf.close();
    timef.close();
    string heuristicName = "euclid";
    //string heuristicName = "no_heurist";
    //string heuristicName = "chebyshev";

    auto h1 = [&](pair<int, int> a, pair<int, int> b) -> int {
        //return 0;
        long double dx = a.first - b.first;
        long double dy = a.second - b.second;
        return floor(sqrtl(dx * dx + dy * dy) / maxmult);
        //return max(a.first - b. first, a.second - b.second) / maxmult;
    };
    auto h2 = [&](pair<int, int> a, pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    const int TESTCASES = 1;
    mt19937 rnd(1234);
    ofstream outp("results/" + mapName + "/NAMOA*dr_" + heuristicName + ".txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    for (int i = 0; i < TESTCASES; i++) {
        int source = rnd() % n, target = rnd() % n;
//        int source = 0, target = 8;
        cerr << "Starting NAMOA*dr search. Map: " << mapName << ", Heuristic: " << heuristicName << " Test #" << i + 1 << endl;
        double startTime = GetCurTime();
        ParetoSet ansNAMOA_star_dr = NAMOA_star_dr(n, graph, source, target, coordinates, h1, h2);

        double workTime = GetCurTime() - startTime;
        cerr << "Current task work time = " << workTime << endl;
        sumTime += workTime;
        sumAnsSize += ansNAMOA_star_dr.paretoSet.size();
        cerr << "Current average time per task: " << sumTime / (i + 1) << endl;
        cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << endl;

        ansNAMOA_star_dr.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const pair<int, int>& dist : ansNAMOA_star_dr.paretoSet) {
            outp << dist.first << " " << dist.second << '\n';
        }
        outp << "\n\n\n";
    }
    cerr << "\n\nResults for NAMOA*dr with heuristic '" << heuristicName << "' on map '" << mapName << "'\n";
    cerr << "Final average time per task: " << sumTime / TESTCASES << endl;
    cerr << "Final average Pareto set size per task: " << sumAnsSize / TESTCASES << " (sum of sizes is " << sumAnsSize << ")" << endl;
}
