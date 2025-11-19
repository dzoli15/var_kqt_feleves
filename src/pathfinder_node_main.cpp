#include "pathfinder_node.hpp"
#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <rclcpp/executors/multi_threaded_executor.hpp>

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto pathfinder_node = std::make_shared<PathfinderNode>();
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
  executor.add_node(pathfinder_node);
  //RCLCPP_INFO(pathfinder_node->get_logger(), "MultiThreadedExecutor indul...");
  executor.spin(); // Ez most már több szálon fogadja a hívásokat
  rclcpp::shutdown();
  return 0;
}