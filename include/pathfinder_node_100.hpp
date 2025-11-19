#ifndef PATHFINDER_NODE_100_HPP
#define PATHFINDER_NODE_100_HPP

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <visualization_msgs/msg/marker_array.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <memory>
#include <random>
#include <mutex>
#include "maze_solver.hpp" // A régi solvert használjuk!

// ANSI Színkódok
#define LOG_RESET "\033[0m"
#define LOG_RED "\033[1;31m"
#define LOG_GREEN "\033[1;32m"
#define LOG_YELLOW "\033[1;33m"
#define LOG_CYAN "\033[1;36m"

class PathfinderNode100 : public rclcpp::Node
{
public:
    PathfinderNode100();

private:
    void timer_callback();
    void handle_trigger_service(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void run_generation_cycle();
    void publish_visualizations();

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr trigger_service_;
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr grid_pub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;

    std::unique_ptr<MazeSolver> solver_;
    std::mt19937 gen_; 
    int map_size_;
    bool automatic_mode_;
    std::mutex run_mutex_;
};

#endif // PATHFINDER_NODE_100_HPP