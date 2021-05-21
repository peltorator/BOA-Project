#include <bits/stdc++.h>

struct Distance {
    int time;
    int length;

    Distance(const int time, const int length): time(time), length(length) {}

    Distance(): time(0), length(0) {}

    Distance operator+(const Distance& other) const {
        return Distance(time + other.time, length + other.length);
    }

    bool operator<=(const Distance& other) const {
        return time <= other.time && length <= other.length;
    }

    bool operator<(const Distance& other) const {
        return time <= other.time && length <= other.length && (time < other.time || length < other.length);
    }

    bool operator>=(const Distance& other) const {
        return time >= other.time && length >= other.length;
    }

    bool operator>(const Distance& other) const {
        return time >= other.time && length >= other.length && (time > other.time || length > other.length);
    }
};

struct DistanceComparator {
    bool operator() (const Distance& first, const Distance& second) const {
        return std::make_pair(first.length, first.time) > std::make_pair(second.length, second.time);
    }
};


struct ParetoSet {
    std::list<Distance> paretoSet;

    bool dominates(const Distance& dist) const {
        for (const Distance& paretoDist : paretoSet) {
            if (paretoDist <= dist) {
                return true;
            }
        }
        return false;
    }

    void add(const Distance& dist) {
        std::list<Distance> newParetoSet;
        for (const Distance& paretoDist : paretoSet) {
            if (paretoDist <= dist) {
                return;
            } else if (!(dist <= paretoDist)) {
                newParetoSet.push_back(paretoDist);
            }
        }
        newParetoSet.push_back(dist);
        paretoSet = newParetoSet;
    }
};

struct Edge {
    int from;
    int to;
    Distance cost;

    Edge(const int from, const int to, const Distance& cost): from(from), to(to), cost(cost) {}
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

    Node(): index(0), dist(Distance(0, 0)) {}

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
        return std::make_pair(first.dist.time, first.dist.length) > std::make_pair(second.dist.time, second.dist.length);
    }
};

ParetoSet BDijkstra(const int n, const Graph& graph, const int source, const int target) {
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSet;
    openSet.push(Node(source, Distance(0, 0)));
    std::vector<ParetoSet> paretoSets(n);

    while (!openSet.empty()) {
        Node curNode = openSet.top();
        int v = curNode.index;
        Distance curDist = curNode.dist;
        openSet.pop();
        if (paretoSets[v].dominates(curDist) || paretoSets[target].dominates(curDist)) {
            continue;
        }
        paretoSets[v].add(curDist);
        for (const Edge& e : graph.getVertexAdjacencyList(v)) {
            Distance newDist = curDist + e.cost;
            int u = e.to;
            if (!paretoSets[u].dominates(newDist) && !paretoSets[target].dominates(newDist)) {
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
        graph.addEdge(Edge(from, to, {time, length}));
    }

    std::mt19937 rnd(1234);
    int source = rnd() % n, target = rnd() % n;
    //int source = 0, target = 1000;

    std::cerr << "Starting BDijkstra search" << std::endl;
    double startTime = clock() * 1.0 / CLOCKS_PER_SEC;
    ParetoSet ansBDijkstra = BDijkstra(n, graph, source, target);
    std::cerr << "Work time = " << clock() * 1.0 / CLOCKS_PER_SEC - startTime << std::endl;

    std::ofstream outp("results/" + mapName + "/BDijkstra.txt");
    ansBDijkstra.paretoSet.sort();
    outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
    for (const Distance& dist : ansBDijkstra.paretoSet) {
        outp << dist.length << " " << dist.time << '\n';
    }
    outp << "\n\n\n";
}
