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

ParetoSet BBDijkstra(const int n, const Graph& graph, const Graph& revGraph, const int source, const int target) {
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSetLeft;
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSetRight;
    openSetLeft.push(Node(source, Distance(0, 0)));
    openSetRight.push(Node(target, Distance(0, 0)));
    std::vector<ParetoSet> paretoSetsLeft(n), paretoSetsRight(n);

    ParetoSet ans;

    for (int iteration = 0; !openSetLeft.empty() && !openSetRight.empty(); iteration++) {        
        if (iteration % 2 == 0) {
            Node curNodeLeft = openSetLeft.top();
            int vleft = curNodeLeft.index;
            Distance curDistLeft = curNodeLeft.dist;
            openSetLeft.pop();
            if (paretoSetsLeft[vleft].dominates(curDistLeft) || ans.dominates(curDistLeft)) {
                continue;
            }
            paretoSetsLeft[vleft].add(curDistLeft);
            for (const Distance& rightDist : paretoSetsRight[vleft].paretoSet) {
                ans.add(curDistLeft + rightDist);
            }
            for (const Edge& e : graph.getVertexAdjacencyList(vleft)) {
                Distance newDist = curDistLeft + e.cost;
                int u = e.to;
                if (!paretoSetsLeft[u].dominates(newDist) && !ans.dominates(newDist)) {
                    openSetLeft.push(Node(u, newDist));
                }
            }
        } else {
            Node curNodeRight = openSetRight.top();
            int vright = curNodeRight.index;
            Distance curDistRight = curNodeRight.dist;
            openSetRight.pop();
            if (paretoSetsRight[vright].dominates(curDistRight) || ans.dominates(curDistRight)) {
                continue;
            }
            paretoSetsRight[vright].add(curDistRight);
            for (const Distance& leftDist : paretoSetsLeft[vright].paretoSet) {
                ans.add(curDistRight + leftDist);
            }
            for (const Edge& e : revGraph.getVertexAdjacencyList(vright)) {
                Distance newDist = curDistRight + e.cost;
                int u = e.to;
                if (!paretoSetsRight[u].dominates(newDist) && !ans.dominates(newDist)) {
                    openSetRight.push(Node(u, newDist));
                }
            }
        }
    }
    return ans;
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
    Graph revGraph(n);

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
        revGraph.addEdge(Edge(to, from, {length, time}));
    }
    distf.close();
    timef.close();

    const int TESTCASES = 5;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/BBDijkstra.txt");
    std::ofstream runtimes("results/" + mapName + "/BBDijkstra_runtimes.txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    std::vector<double> times;
    for (int i = 0; i < TESTCASES; i++) {
        //int source = rnd() % (n - 60) + 30, target = source + rnd() % 60 - 30;
        int source = rnd() % (n - 2000) + 1000, target = source + rnd() % 2000 - 1000;
        std::cerr << "Starting BBDijkstra search. Map: " << mapName << " Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansBBDijkstra = BBDijkstra(n, graph, revGraph, source, target);

        double workTime = GetCurTime() - startTime;
        times.push_back(workTime);
        runtimes << i << ' ' << workTime << '\n';
        std::cerr << "Current task work time = " << workTime << std::endl;
        sumTime += workTime;
        sumAnsSize += ansBBDijkstra.paretoSet.size();
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;
        std::cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << std::endl;

        ansBBDijkstra.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const Distance& dist : ansBBDijkstra.paretoSet) {
            outp << dist.length << " " << dist.time << '\n';
        }
        outp << "\n\n\n";
    }
    std::sort(times.begin(), times.end());
    res << "\n\nResults for BBDijkstra on map '" << mapName << "'\n";
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
    std::ofstream res("results_bbdijkstra.txt");
    for (std::string mapName : {"NY", "BAY", "COL"}) {
        solveForMap(mapName, res);
    }
}
