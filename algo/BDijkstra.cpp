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

ParetoSet BDijkstra(const int n, const Graph& graph, const int source, const int target) {
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
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

int main() {
    int n, m;
    std::cin >> n >> m;
    int source, target;
    std::cin >> source >> target;
    source--;
    target--;

    Graph graph(n);
    for (int i = 0; i < m; i++) {
        int from, to, time, length;
        std::cin >> from >> to >> time >> length;
        from--;
        to--;
        graph.addEdge(Edge(from, to, Distance(time, length)));
    }

    ParetoSet ans = BDijkstra(n, graph, source, target);

    std::cout << "Optimal distances:\n";
    for (const Distance& dist : ans.paretoSet) {
        std::cout << dist.time << " " << dist.length << '\n';
    }
}
