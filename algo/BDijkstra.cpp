#include <bits/stdc++.h>

struct Distance {
    long long time;
    long long length;

    Distance(const long long time, const long long length): time(time), length(length) {}

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

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}
 
void solveForMap(std::string mapName, std::ofstream& res) {
    std::ifstream coordf("maps/" + mapName + "/coordinates.txt");
 
    int n;
    coordf >> n;
    coordf.close();

    Graph graph(n);

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
        graph.addEdge(Edge(from, to, {length, time}));
    }
    distf.close();
    timef.close();

    const int TESTCASES = 40;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/BDijkstra.txt");
    std::ofstream runtimes("results/" + mapName + "/BDijkstra_runtimes.txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    std::vector<double> times;
    for (int i = 0; i < TESTCASES; i++) {
        //int source = rnd() % n, target = rnd() % n;
        int source = rnd() % (n - 2000) + 1000, target = source + rnd() % 2000 - 1000;
        std::cerr << "Starting BDijkstra search. Map: " << mapName << " Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansBDijkstra = BDijkstra(n, graph, source, target);

        double workTime = GetCurTime() - startTime;
        times.push_back(workTime);
        runtimes << i << ' ' << workTime << '\n';
        std::cerr << "Current task work time = " << workTime << std::endl;
        sumTime += workTime;
        sumAnsSize += ansBDijkstra.paretoSet.size();
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;
        std::cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << std::endl;

        ansBDijkstra.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const Distance& dist : ansBDijkstra.paretoSet) {
            outp << dist.length << " " << dist.time << '\n';
        }
        outp << "\n\n\n";
    }
    std::sort(times.begin(), times.end());
    res << "\n\nResults for BDijkstra on map '" << mapName << "'\n";
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
    std::ofstream res("results_bdijkstra.txt");
    for (std::string mapName : {"NY", "BAY", "COL"}) {
        solveForMap(mapName, res);
    }
}
