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

int main() {
    int n, m;
    std::cin >> n >> m;
    int source, target;
    std::cin >> source >> target;
    source--;
    target--;

    std::vector<std::pair<int, int>> coordinates(n);
    for (int i = 0; i < n; i++) {
        std::cin >> coordinates[i].first >> coordinates[i].second;
    }

    Graph graph(n), revGraph(n);
    double maxspeed = 0;
    for (int i = 0; i < m; i++) {
        int from, to, time, length;
        std::cin >> from >> to >> time >> length;
        from--;
        to--;
        if (time != 0) {
            maxspeed = std::max(maxspeed, static_cast<double>(length) / static_cast<double>(time));
        }
        graph.addEdge(Edge(from, to, {time, length}));
        revGraph.addEdge(Edge(to, from, {time, length}));
    }
    auto h1 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
    };
    auto h2 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    ParetoSet ansBOAStar = BOAStar(n, graph, source, target, coordinates, h1, h2);

    std::cout << "\nOptimal distances for BOAStar:\n";
    for (const std::pair<int, int>& dist : ansBOAStar.paretoSet) {
        std::cout << dist.first << " " << dist.second << '\n';
    }

}
