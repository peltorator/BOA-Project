#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef double ld;

struct Node {
    ll id;
    pair<ld, ld> g;
    pair<ld, ld> f;
    Node * parent = nullptr;
};

bool operator==(const Node& lhs, const Node& rhs) {
    return (lhs.id == rhs.id) && (lhs.g == rhs.g);
}

bool operator<(const Node& lhs, const Node& rhs) {
    if (lhs.f.first == rhs.f.first) {
        return lhs.f.second < rhs.f.second;
    }
    return lhs.f.first < rhs.f.first;
}

ld diagonal_distance(ld i1, ld j1, ld i2, ld j2) {
    ld dx = abs(i1 - i2);
    ld dy = abs(j1 - j2);
    return (double)abs(dx - dy) + sqrt(2) * min(dx, dy);
}

ld euclid_distance(ld i1, ld j1, ld i2, ld j2) {
    ld dx = abs(i1 - i2);
    ld dy = abs(j1 - j2);
    return sqrt(dx * dx + dy * dy);
}

bool check_dominance(pair<ld, ld> a, pair<ld, ld> b) {
    return (a.first <= b.first && a.second <= b.second && a != b);
}

vector<Node> Close;

pair<vector<Node>, vector<Node>> NAMOA_star(map<ll, vector<ll>> graph,
                                            map<ll, pair<ld, ld>> coords,
                                            map<pair<ll, ll>, pair<ld, ld>> c,
                                            ll start, ll goal) {
    auto [i_start, j_start] = coords[start];
    auto [i_goal, j_goal] = coords[goal];

    vector<Node> sols;
    map<ll, vector<pair<ld, ld>>> Gop;
    map<ll, vector<pair<ld, ld>>> Gcl;
    Gop[start].push_back({0, 0});

    Node start_node = Node();
    start_node.id = start;
    start_node.g = {0, 0};
    ld h_start = euclid_distance(i_start, j_start, i_goal, j_goal);
    start_node.f = {h_start, h_start};

    set<Node> Open;
    Open.insert(start_node);

    while (!Open.empty()) {
        Node s = *Open.begin();
        Close.push_back(s);
        Open.erase(Open.begin());
        cout << "s.id " << s.id << ' ' << s.g.first << ' ' << s.g.second << endl;
        if (s.parent != nullptr) {
            cout << "PARENT s " << s.parent->id << endl;
        }
        Gop[s.id].erase(remove(Gop[s.id].begin(), Gop[s.id].end(), s.g), Gop[s.id].end());
        Gcl[s.id].push_back(s.g);

        if (s.id == goal) {
            sols.push_back(s);
            for (const Node& u: Open) {
                if (check_dominance(s.g, u.g)) {
                    Open.erase(u);
                }
            }
            continue;
        }

        for (ll i: graph[s.id]) {
            Node t = Node();
            t.id = i;
            t.g = {s.g.first + c[{s.id, t.id}].first, s.g.second + c[{s.id, t.id}].second};
            cout << "t.id " << t.id << ' ' << t.g.first << ' ' << t.g.second << endl;

            bool worse_way = false;
            for (auto g: Gop[t.id]) {
                if (check_dominance(g, t.g)) {
                    worse_way = true;
                    break;
                }
                if (g == t.g) {
                    t.parent = &s;
                    worse_way = true;
                    break;
                }
            }
            if (worse_way) {
                continue;
            }

            for (auto g: Gcl[t.id]) {
                if (check_dominance(g, t.g)) {
                    worse_way = true;
                    break;
                }
                if (g == t.g) {
                    t.parent = &s;
                    worse_way = true;
                    break;
                }
            }
            if (worse_way) {
                continue;
            }

            ld h = euclid_distance(coords[t.id].first, coords[t.id].first, i_goal, j_goal);
            t.f = {t.g.first + h, t.g.second + h};

            for (auto v: sols) {
                if (check_dominance(v.g, t.f)) {
                    worse_way = true;
                    break;
                }
            }
            if (worse_way) {
                continue;
            }

            vector<pair<ld, ld>> Gop_new;
            for (auto g: Gop[t.id]) {
                if (check_dominance(t.g, g)) {
                    auto iter = find_if(Open.begin(), Open.end(),
                                        [t, g] (const Node& v) { return (v.id == t.id && v.g == g); });
                    while (iter != Open.end()) {
                        Open.erase(iter);
                        iter = find_if(Open.begin(), Open.end(),
                                       [t, g] (const Node& v) { return (v.id == t.id && v.g == g); });
                    }
                    continue;
                }
                Gop_new.push_back(g);
            }
            Gop[t.id] = Gop_new;
            Gop[t.id].push_back(t.g);

            vector<pair<ld, ld>> Gcl_new;
            for (auto g: Gcl[t.id]) {
                if (check_dominance(t.g, g)) {
                    continue;
                }
                Gcl_new.push_back(g);
            }
            Gcl[t.id] = Gcl_new;

            t.parent = &s;
            cout << "parent...... " << t.parent->id << endl;
            Open.insert(t);
        }
    }
    return {sols, Close};
}

int main()
{
    map<ll, vector<ll>> graph;
    map<ll, pair<ld, ld>> coords;
    map<pair<ll, ll>, pair<ld, ld>> c;
    ll n;
    cin >> n;
    ll id;
    ld x, y;
    for (ll i = 0; i < n; i++) {
        cin >> id >> x >> y;
        coords[id] = {x, y};
    }
    ll m;
    cin >> m;
    ll id1, id2;
    ld dist, time;
    for (ll i = 0; i < m; i++) {
        cin >> id1 >> id2 >> dist;
        graph[id1].push_back(id2);
        c[{id1, id2}].first = dist;
    }
    for (ll i = 0; i < m; i++) {
        cin >> id1 >> id2 >> time;
        c[{id1, id2}].second = time;
    }
    ll start, goal;
    cin >> start >> goal;

    auto [sols, close] = NAMOA_star(graph, coords, c, start, goal);
    cout << "size " << sols.size() << endl;
    for (auto way: sols) {
        cout << way.id << ' ' << way.g.first << ' ' << way.g.second << endl;
        cout << "parent " << way.parent->id << endl;
        cout << endl;
    }
}