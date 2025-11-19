#include "maze_solver.hpp"
#include <stack>
#include <queue>
#include <algorithm>
#include <cmath>

// --- Point struktúra ---
bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}
bool Point::operator<(const Point& other) const {
    if (x == other.x) return y < other.y;
    return x < other.x;
}

// --- AStarNode struktúra ---
double AStarNode::f() const { return g + h; }
bool AStarNode::operator>(const AStarNode& other) const {
    return f() > other.f();
}

// --- MazeSolver osztály ---

MazeSolver::MazeSolver(int map_size) : map_size_(map_size), gen_(std::random_device{}())
{
    // A páratlan méret kényszerítése
    if (map_size_ % 2 == 0) {
        map_size_++;
    }
    // Azonnal inicializáljuk a grid-et, hogy a getMapSize() helyes legyen
    map_grid_.resize(map_size_, std::vector<int>(map_size_, 100));
}

const std::vector<std::vector<int>>& MazeSolver::getMapGrid() const
{
    return map_grid_;
}

int MazeSolver::getMapSize() const
{
    return map_size_;
}

void MazeSolver::generateMaze(std::vector<Point>& out_entrances, std::vector<Point>& out_exits)
{
    // Térkép alaphelyzetbe állítása (mindenhol fal)
    map_grid_.assign(map_size_, std::vector<int>(map_size_, 100));
    std::stack<Point> stack;
    Point maze_start = {1, 1};
    map_grid_[maze_start.y][maze_start.x] = 0; // 0 = út
    stack.push(maze_start);

    int dx[] = {0, 0, 2, -2};
    int dy[] = {2, -2, 0, 0};
    std::vector<int> directions = {0, 1, 2, 3};

    while (!stack.empty()) {
        Point current = stack.top();
        std::vector<int> valid_neighbors;
        for(int i = 0; i < 4; ++i) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            if (nx > 0 && nx < map_size_ - 1 && ny > 0 && ny < map_size_ - 1 && map_grid_[ny][nx] == 100) {
                valid_neighbors.push_back(i);
            }
        }

        if (!valid_neighbors.empty()) {
            std::shuffle(valid_neighbors.begin(), valid_neighbors.end(), gen_);
            int dir = valid_neighbors[0];
            Point next = {current.x + dx[dir], current.y + dy[dir]};
            Point wall_between = {current.x + dx[dir]/2, current.y + dy[dir]/2};
            map_grid_[next.y][next.x] = 0;
            map_grid_[wall_between.y][wall_between.x] = 0;
            stack.push(next);
        } else {
            stack.pop();
        }
    }

    // Belső falak kiütése (több útvonal)
    int num_walls_to_break = map_size_ * 2;
    std::uniform_int_distribution<> pos_dist(1, map_size_ - 2);
    
    for (int i = 0; i < num_walls_to_break; ++i) {
        int x = pos_dist(gen_);
        int y = pos_dist(gen_);
        if (map_grid_[y][x] == 100 && (x % 2 == 0 || y % 2 == 0)) {
            map_grid_[y][x] = 0;
        }
    }

    // Több be- és kijárat generálása
    std::vector<Point> potential_entrances, potential_exits;
    for(int i = 1; i < map_size_ - 1; i += 2) {
        potential_entrances.push_back({i, 0}); // Felső
        potential_entrances.push_back({0, i}); // Bal
        potential_exits.push_back({i, map_size_ - 1}); // Alsó
        potential_exits.push_back({map_size_ - 1, i}); // Jobb
    }

    std::shuffle(potential_entrances.begin(), potential_entrances.end(), gen_);
    std::shuffle(potential_exits.begin(), potential_exits.end(), gen_);

    int num_openings = std::min(3, (int)potential_entrances.size());
    
    for(int i = 0; i < num_openings; ++i) {
        Point p_in = potential_entrances[i];
        map_grid_[p_in.y][p_in.x] = 0;
        out_entrances.push_back(p_in);

        Point p_out = potential_exits[i];
        map_grid_[p_out.y][p_out.x] = 0;
        out_exits.push_back(p_out);
    }
}

// --- A* Algoritmus ---

double MazeSolver::heuristic(Point a, Point b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<Point> MazeSolver::findShortestPathAStar(Point start, Point end)
{
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> open_list;
    std::map<Point, bool> closed_list;
    std::vector<std::vector<AStarNode*>> all_nodes(map_size_, std::vector<AStarNode*>(map_size_, nullptr));

    AStarNode* start_node = new AStarNode{start, 0.0, heuristic(start, end), nullptr};
    open_list.push(*start_node);
    all_nodes[start.y][start.x] = start_node;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    std::vector<Point> path;

    while (!open_list.empty()) {
        AStarNode current_node_copy = open_list.top();
        open_list.pop();
        Point current_pos = current_node_copy.pos;
        if (closed_list.count(current_pos)) continue;
        closed_list[current_pos] = true;

        if (current_pos == end) {
            AStarNode* temp = all_nodes[current_pos.y][current_pos.x];
            while (temp != nullptr) {
                path.push_back(temp->pos);
                temp = temp->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        for (int i = 0; i < 4; ++i) {
            Point neighbor_pos = {current_pos.x + dx[i], current_pos.y + dy[i]};
            if (neighbor_pos.x < 0 || neighbor_pos.x >= map_size_ ||
                neighbor_pos.y < 0 || neighbor_pos.y >= map_size_ ||
                map_grid_[neighbor_pos.y][neighbor_pos.x] == 100 ||
                closed_list.count(neighbor_pos)) {
                continue;
            }
            double new_g = all_nodes[current_pos.y][current_pos.x]->g + 1.0;
            AStarNode* neighbor_node = all_nodes[neighbor_pos.y][neighbor_pos.x];
            if (neighbor_node == nullptr || new_g < neighbor_node->g) {
                if (neighbor_node == nullptr) {
                     neighbor_node = new AStarNode();
                     all_nodes[neighbor_pos.y][neighbor_pos.x] = neighbor_node;
                }
                neighbor_node->pos = neighbor_pos;
                neighbor_node->g = new_g;
                neighbor_node->h = heuristic(neighbor_pos, end);
                neighbor_node->parent = all_nodes[current_pos.y][current_pos.x];
                open_list.push(*neighbor_node);
            }
        }
    }
    for (auto& row : all_nodes) {
        for (auto* node_ptr : row) {
            if (node_ptr != nullptr) delete node_ptr;
        }
    }
    return path;
}

// --- DFS Algoritmus (Összes Út) ---

bool MazeSolver::isValidForDFS(Point p, std::map<Point, bool>& visited) {
    return p.x >= 0 && p.x < map_size_ &&
           p.y >= 0 && p.y < map_size_ &&
           map_grid_[p.y][p.x] == 0 &&
           !visited[p];
}

void MazeSolver::findAllPathsRecursive(Point current, Point end, std::vector<Point>& current_path, std::map<Point, bool>& visited)
{
    visited[current] = true;
    current_path.push_back(current);

    if (current == end) {
        all_paths_list_.push_back(current_path);
    } else {
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};
        for (int i = 0; i < 4; ++i) {
            Point neighbor = {current.x + dx[i], current.y + dy[i]};
            if (isValidForDFS(neighbor, visited)) {
                findAllPathsRecursive(neighbor, end, current_path, visited);
            }
        }
    }
    current_path.pop_back();
    visited[current] = false;
}

std::vector<std::vector<Point>> MazeSolver::findAllPathsDFS(Point start, Point end) {
    all_paths_list_.clear();
    std::vector<Point> current_path;
    std::map<Point, bool> visited;
    findAllPathsRecursive(start, end, current_path, visited);
    return all_paths_list_;
}