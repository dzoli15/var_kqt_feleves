#ifndef PATHFINDER_NODE_HPP
#define PATHFINDER_NODE_HPP

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <visualization_msgs/msg/marker_array.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <memory> // std::unique_ptr
#include <random>
#include <mutex>
#include "maze_solver.hpp"

// ANSI Színkódok a logoláshoz
#define LOG_RESET "\033[0m"
#define LOG_RED "\033[1;31m"
#define LOG_GREEN "\033[1;32m"
#define LOG_YELLOW "\033[1;33m"
#define LOG_CYAN "\033[1;36m"

class PathfinderNode : public rclcpp::Node
{
public:
    PathfinderNode();

private:
    // --- ROS Visszahívó függvények ---
    void timer_callback();
    void handle_trigger_service(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    // --- Fő Logikai Ciklus ---
    std::pair<bool, std::string> run_pathfinding_cycle();

    // --- Vizualizáció ---
    void publish_visualizations(
        const std::vector<Point>& shortest_path,
        Point start, Point end);

    // --- ROS Tagváltozók ---
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr trigger_service_;
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr grid_pub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;

    // --- Belső Állapot ---
    std::unique_ptr<MazeSolver> solver_;
    std::mt19937 gen_; // Random be/kijárat választáshoz
    int map_size_;
    bool automatic_mode_;
    std::mutex run_mutex_;
};

#endif // PATHFINDER_NODE_HPP