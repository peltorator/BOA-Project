#include <bits/stdc++.h>

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

int main() {
    std::string mapName = "SMALL";
    
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
        int from, to, length;
        distf >> from >> to >> length;
        int from2, to2, time;
        timef >> from2 >> to2 >> time;
        assert(from2 == from && to2 == to);

        from--;
        to--;
        graph.addEdge(Edge(from, to, {length, time}));
    }
    distf.close();
    timef.close();

    std::ofstream outp("results/" + mapName + "/BruteForce.txt");

    for (int source = 0; source < n; source++) {
        std::vector<std::vector<std::pair<int, int>>> paths(n);
        std::vector<bool> used(n, false);
        std::function<void(int, int, int)> dfs = [&](int v, int curLen, int curTime) {
            used[v] = true;
            paths[v].emplace_back(curLen, curTime);
            for (const Edge& e : graph.getVertexAdjacencyList(v)) {
                int u = e.to;
                if (!used[u]) {
                    int eLen = e.cost.first;
                    int eTime = e.cost.second;
                    dfs(u, curLen + eLen, curTime + eTime);
                }
            }
            used[v] = false;
        };
        dfs(source, 0, 0);
        for (int i = 0; i < n; i++) {
            std::sort(paths[i].begin(), paths[i].end());
            std::vector<std::pair<int, int>> pareto;
            for (auto dist : paths[i]) {
                if (pareto.empty() || pareto.back().second > dist.second) {
                    pareto.push_back(dist);
                }
            }
            outp << "Optimal set for path from " << source + 1 << " to " << i + 1 << '\n';
            for (const std::pair<int, int>& dist : pareto) {
                outp << dist.first << " " << dist.second << '\n';
            }
            outp << "\n\n\n";
        }
    }
}
