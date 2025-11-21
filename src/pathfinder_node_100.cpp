#include "pathfinder_node_100.hpp"
#include <sstream>
#include <iomanip>

using namespace std::chrono_literals;

// Konstruktor
PathfinderNode100::PathfinderNode100() : Node("pathfinder_node_100"), gen_(std::random_device{}())
{
    // Paraméterek: Alapértelmezett méret 100
    this->declare_parameter<bool>("automatic_mode", true);
    this->declare_parameter<int>("map_size", 100); 
    
    map_size_ = this->get_parameter("map_size").as_int();
    automatic_mode_ = this->get_parameter("automatic_mode").as_bool();

    // Labirintus megoldó inicializálása
    solver_ = std::make_unique<MazeSolver>(map_size_);
    // A solver kényszeríthette a páratlan méretet, kérdezzük le
    map_size_ = solver_->getMapSize(); 

    // Publisherek
    grid_pub_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("map_grid_100", 10);
    
    // Marker publisher-t meghagyjuk, hogy törölni tudjuk a régi elemeket, ha maradtak
    marker_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("visualization_markers_100", 10);

    // Módválasztó
    if (automatic_mode_) {
        timer_ = this->create_wall_timer(
            5000ms, std::bind(&PathfinderNode100::timer_callback, this)); // 5 másodpercenként
        RCLCPP_INFO(this->get_logger(), LOG_GREEN "Indítás AUTOMATA módban (100x100). 5 másodpercenként új labirintus." LOG_RESET);
    } else {
        trigger_service_ = this->create_service<std_srvs::srv::Trigger>(
            "trigger_generation_100",
            std::bind(&PathfinderNode100::handle_trigger_service, this, std::placeholders::_1, std::placeholders::_2)
        );
        RCLCPP_INFO(this->get_logger(), LOG_YELLOW "Indítás MANUÁLIS módban (100x100)." LOG_RESET);
        RCLCPP_INFO(this->get_logger(), LOG_YELLOW "Hívd a '/trigger_generation_100' szolgáltatást a generáláshoz." LOG_RESET);
    }
    
    RCLCPP_INFO(this->get_logger(), LOG_GREEN "PathfinderNode100 (Generator) elindult. Térkép méret: %d x %d" LOG_RESET , map_size_, map_size_);
}

void PathfinderNode100::timer_callback()
{
    std::lock_guard<std::mutex> lock(run_mutex_);
    RCLCPP_INFO(this->get_logger(), "--- [100x100] Új labirintus generálása... ---");
    run_generation_cycle();
}

void PathfinderNode100::handle_trigger_service(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    std::lock_guard<std::mutex> lock(run_mutex_);
    (void)request;
    RCLCPP_INFO(this->get_logger(), "--- [100x100] Manuális generálás kérése ---");
    run_generation_cycle();
    response->success = true;
    response->message = "Nagy labirintus legenerálva.";
}

void PathfinderNode100::run_generation_cycle()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 1. Csak a Labirintus generálása fut le
    std::vector<Point> valid_entrances, valid_exits;
    solver_->generateMaze(valid_entrances, valid_exits);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end_time - start_time;

    RCLCPP_INFO(this->get_logger(), "Generálási idő: " LOG_CYAN "%.2f ms" LOG_RESET, duration.count());

    // 2. Vizualizáció (Csak a térkép)
    publish_visualizations();
}

void PathfinderNode100::publish_visualizations()
{
    auto now = this->get_clock()->now();
    double map_resolution = 0.1; // 1 pixel = 10 cm
    std::string frame_id = "map";

    // 1. OccupancyGrid (Maga a Labirintus)
    nav_msgs::msg::OccupancyGrid grid_msg;
    grid_msg.header.stamp = now;
    grid_msg.header.frame_id = frame_id;
    grid_msg.info.resolution = map_resolution;
    grid_msg.info.width = map_size_;
    grid_msg.info.height = map_size_;
    
    // Középre igazítás
    grid_msg.info.origin.position.x = 0.0; 
    grid_msg.info.origin.position.y = 0.0;
    
    grid_msg.data.resize(map_size_ * map_size_);

    const auto& map_grid = solver_->getMapGrid();
    for (int y = 0; y < map_size_; ++y) {
        for (int x = 0; x < map_size_; ++x) {
            // A solver 100-at használ falnak, 0-t útnak.
            grid_msg.data[y * map_size_ + x] = map_grid[y][x];
        }
    }
    grid_pub_->publish(grid_msg);

    // 2. Takarítás: Töröljük a régi markereket (útvonalakat)
    visualization_msgs::msg::MarkerArray marker_array;
    visualization_msgs::msg::Marker delete_marker;
    delete_marker.header.frame_id = frame_id;
    delete_marker.header.stamp = now;
    delete_marker.action = visualization_msgs::msg::Marker::DELETEALL;
    
    marker_array.markers.push_back(delete_marker);
    marker_pub_->publish(marker_array);
}

// --- A MAIN FÜGGVÉNY ---
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<PathfinderNode100>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
