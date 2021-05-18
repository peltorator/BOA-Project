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

    bool strictlyDominates(const Distance& dist) const {
        for (const Distance& paretoDist : paretoSet) {
            if (paretoDist < dist) {
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
    paretoSets[source].add(Distance(0, 0));

    while (!openSet.empty()) {
        Node curNode = openSet.top();
        int v = curNode.index;
        Distance curDist = curNode.dist;
        openSet.pop();
        if (paretoSets[v].strictlyDominates(curDist) || paretoSets[target].dominates(curDist)) {
            continue;
        }
        for (const Edge& e : graph.getVertexAdjacencyList(v)) {
            Distance newDist = curDist + e.cost;
            int u = e.to;
            if (!paretoSets[u].dominates(newDist) && !paretoSets[target].dominates(newDist)) {
                paretoSets[u].add(newDist);
                openSet.push(Node(u, newDist));
            }
        }
    }
    return paretoSets[target];
}

ParetoSet BBDijkstra(const int n, const Graph& graph, const Graph& revGraph, const int source, const int target) {
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSetLeft;
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openSetRight;
    openSetLeft.push(Node(source, Distance(0, 0)));
    openSetRight.push(Node(target, Distance(0, 0)));
    std::vector<ParetoSet> paretoSetsLeft(n), paretoSetsRight(n);
    paretoSetsLeft[source].add(Distance(0, 0));
    paretoSetsRight[target].add(Distance(0, 0));

    ParetoSet ans;

    for (int iteration = 0; !openSetLeft.empty() || !openSetRight.empty(); iteration++) {        
        if (iteration % 2 == 0 && !openSetLeft.empty()) {
            Node curNodeLeft = openSetLeft.top();
            int vleft = curNodeLeft.index;
            Distance curDistLeft = curNodeLeft.dist;
            openSetLeft.pop();
            if (paretoSetsLeft[vleft].strictlyDominates(curDistLeft) || ans.dominates(curDistLeft)) {
                continue;
            }
            for (const Distance& rightDist : paretoSetsRight[vleft].paretoSet) {
                ans.add(curDistLeft + rightDist);
            }
            for (const Edge& e : graph.getVertexAdjacencyList(vleft)) {
                Distance newDist = curDistLeft + e.cost;
                int u = e.to;
                if (!paretoSetsLeft[u].dominates(newDist) && !ans.dominates(newDist)) {
                    paretoSetsLeft[u].add(newDist);
                    openSetLeft.push(Node(u, newDist));
                }
            }
        } else {
            Node curNodeRight = openSetRight.top();
            int vright = curNodeRight.index;
            Distance curDistRight = curNodeRight.dist;
            openSetRight.pop();
            if (paretoSetsRight[vright].strictlyDominates(curDistRight) || ans.dominates(curDistRight)) {
                continue;
            }
            for (const Distance& leftDist : paretoSetsLeft[vright].paretoSet) {
                ans.add(curDistRight + leftDist);
            }
            for (const Edge& e : revGraph.getVertexAdjacencyList(vright)) {
                Distance newDist = curDistRight + e.cost;
                int u = e.to;
                if (!paretoSetsRight[u].dominates(newDist) && !ans.dominates(newDist)) {
                    paretoSetsRight[u].add(newDist);
                    openSetRight.push(Node(u, newDist));
                }
            }
        }
    }
    return ans;
}
 
int main() {
    int n, m;
    std::cin >> n >> m;
    int source, target;
    std::cin >> source >> target;
    source--;
    target--;

    for (int i = 0; i < n; i++) {
        int x, y;
        std::cin >> x >> y;
    }

    Graph graph(n), revGraph(n);
    for (int i = 0; i < m; i++) {
        int from, to, time, length;
        std::cin >> from >> to >> time >> length;
        from--;
        to--;
        graph.addEdge(Edge(from, to, Distance(time, length)));
        revGraph.addEdge(Edge(to, from, Distance(time, length)));
    }

    ParetoSet ansBDijkstra = BDijkstra(n, graph, source, target);
    ParetoSet ansBBDijkstra = BBDijkstra(n, graph, revGraph, source, target);

    std::cout << "Optimal distances for BDijstra:\n";
    for (const Distance& dist : ansBDijkstra.paretoSet) {
        std::cout << dist.time << " " << dist.length << '\n';
    }

    std::cout << "\nOptimal distances for BBDijstra:\n";
    for (const Distance& dist : ansBBDijkstra.paretoSet) {
        std::cout << dist.time << " " << dist.length << '\n';
    }

}
