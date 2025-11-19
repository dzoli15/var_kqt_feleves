#ifndef MAZE_SOLVER_HPP
#define MAZE_SOLVER_HPP

#include <vector>
#include <string>
#include <random>
#include <map>

// Előre deklarációk
struct Point {
    int x, y;
    bool operator==(const Point& other) const;
    bool operator<(const Point& other) const;
};

struct AStarNode {
    Point pos;
    double g, h;
    AStarNode* parent;
    double f() const;
    bool operator>(const AStarNode& other) const;
};

class MazeSolver
{
public:
    MazeSolver(int map_size);

    // 1. Labirintus generátor
    void generateMaze(std::vector<Point>& out_entrances, std::vector<Point>& out_exits);

    // 2. Legrövidebb út (A*)
    std::vector<Point> findShortestPathAStar(Point start, Point end);

    // 3. Összes út (DFS)
    std::vector<std::vector<Point>> findAllPathsDFS(Point start, Point end);

    // Getter a térképhez (vizualizációhoz)
    const std::vector<std::vector<int>>& getMapGrid() const;
    int getMapSize() const;

private:
    int map_size_;
    std::vector<std::vector<int>> map_grid_;
    std::mt19937 gen_; // Random generátor

    // DFS segédfüggvények
    std::vector<std::vector<Point>> all_paths_list_;
    void findAllPathsRecursive(Point current, Point end, std::vector<Point>& current_path, std::map<Point, bool>& visited);
    bool isValidForDFS(Point p, std::map<Point, bool>& visited);

    // A* segédfüggvény (heurisztika)
    double heuristic(Point a, Point b);
};

#endif // MAZE_SOLVER_HPP