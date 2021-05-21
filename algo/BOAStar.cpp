#include <bits/stdc++.h>

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
    std::list<std::pair<int, int>> paretoSet;
    int g2min = std::numeric_limits<int>::max();

    bool dominates(const int val) const {
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
    std::pair<int, int> cost;

    Edge(const int from, const int to, const std::pair<int, int>& cost): from(from), to(to), cost(cost) {}
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
    //paretoSets[source].add(Distance(0, h1(source), 0, h2(source)));

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

std::string GetMapName() {
    std::string mapName;
    std::cin >> mapName;
    return mapName;
}

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}

int main() {
    std::string mapName = "NY";// GetMapName();
    
    std::ifstream coordf("maps/" + mapName + "/coordinates.txt");
    std::ifstream distf("maps/" + mapName + "/distances.txt");
    std::ifstream timef("maps/" + mapName + "/time.txt");

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

    Graph graph(n);
    long double maxspeed = 0;
    long double maxmult = 0;

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
            maxmult = std::max(maxmult, sqrtl(dx * dx + dy * dy) / static_cast<long double>(length));
        }
        if (time != 0) {
            maxspeed = std::max(maxspeed, static_cast<long double>(length) / static_cast<long double>(time));
        }
        graph.addEdge(Edge(from, to, {time, length}));
    }
    auto h1 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        long double dx = a.first - b.first;
        long double dy = a.second - b.second;
        return floor(sqrtl(dx * dx + dy * dy) / maxmult);
        //return std::max(a.first - b.first, a.second - b.second) / maxmult;
    };
    auto h2 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    const int TESTCASES = 100;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/BOAStar.txt");
    long double sumTime = 0;
    for (int i = 0; i < TESTCASES; i++) {
        int source = rnd() % n, target = rnd() % n;
        std::cerr << "Starting BOAStar search. Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansBOAStar = BOAStar(n, graph, source, target, coordinates, h1, h2);

        double workTime = GetCurTime() - startTime;
        std::cerr << "Work time = " << workTime << std::endl;
        sumTime += workTime;
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;

        ansBOAStar.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const std::pair<int, int>& dist : ansBOAStar.paretoSet) {
            outp << dist.second << " " << dist.first << '\n';
        }
        outp << "\n\n\n";
    }
    std::cerr << "Final average time per task: " << sumTime / TESTCASES << std::endl;
}
