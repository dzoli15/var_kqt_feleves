#include "pathfinder_node.hpp"
#include <sstream>
#include <iomanip>

using namespace std::chrono_literals;

PathfinderNode::PathfinderNode() : Node("pathfinder_node"), gen_(std::random_device{}())
{
    // Paraméterek
    this->declare_parameter<bool>("automatic_mode", true);
    this->declare_parameter<int>("map_size", 15);
    
    map_size_ = this->get_parameter("map_size").as_int();
    automatic_mode_ = this->get_parameter("automatic_mode").as_bool();

    // Labirintus megoldó inicializálása
    solver_ = std::make_unique<MazeSolver>(map_size_);
    // A solver kényszeríthette a páratlan méretet, kérdezzük le
    map_size_ = solver_->getMapSize(); 

    // Publisherek
    grid_pub_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("map_grid", 10);
    marker_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("visualization_markers", 10);

    // Módválasztó
    if (automatic_mode_) {
        timer_ = this->create_wall_timer(
            10000ms, std::bind(&PathfinderNode::timer_callback, this));
        RCLCPP_INFO(this->get_logger(), LOG_GREEN "Indítás AUTOMATA módban. (10 másodpercenként újratervezés)" LOG_RESET);
    } else {
        trigger_service_ = this->create_service<std_srvs::srv::Trigger>(
            "trigger_pathfinding",
            std::bind(&PathfinderNode::handle_trigger_service, this, std::placeholders::_1, std::placeholders::_2)
        );
        RCLCPP_INFO(this->get_logger(), LOG_YELLOW "Indítás MANUÁLIS módban." LOG_RESET);
        RCLCPP_INFO(this->get_logger(), LOG_YELLOW "Hívd a '/trigger_pathfinding' szolgáltatást a tervezéshez." LOG_RESET);
    }
    
    RCLCPP_INFO(this->get_logger(), LOG_GREEN "Pathfinder node elindult. Térkép méret: %d x %d" LOG_RESET , map_size_, map_size_);
}

void PathfinderNode::timer_callback()
{
    std::lock_guard<std::mutex> lock(run_mutex_);
    RCLCPP_INFO(this->get_logger(), " ");
    RCLCPP_INFO(this->get_logger(), "---------------------------------");
    RCLCPP_INFO(this->get_logger(), "--- Időzített újratervezés ---");
    RCLCPP_INFO(this->get_logger(), "---------------------------------");
    RCLCPP_INFO(this->get_logger(), " ");
    run_pathfinding_cycle();
}

void PathfinderNode::handle_trigger_service(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    std::lock_guard<std::mutex> lock(run_mutex_);
    (void)request;
    RCLCPP_INFO(this->get_logger(), " ");
    RCLCPP_INFO(this->get_logger(), "---------------------------------");
    RCLCPP_INFO(this->get_logger(), "--- Manuális tervezés kérése ---");
    RCLCPP_INFO(this->get_logger(), "---------------------------------");
    RCLCPP_INFO(this->get_logger(), " ");
    auto result = run_pathfinding_cycle();
    response->success = result.first;
    response->message = result.second;
}

std::pair<bool, std::string> PathfinderNode::run_pathfinding_cycle()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 1. Labirintus generálása
    std::vector<Point> valid_entrances, valid_exits;
    solver_->generateMaze(valid_entrances, valid_exits);

    if (valid_entrances.empty() || valid_exits.empty()) {
        RCLCPP_ERROR(this->get_logger(), LOG_RED "Nem sikerült be- vagy kijáratokat generálni!" LOG_RESET);
        return {false, "Generálási hiba"};
    }

    // 2. Random Start/End kiválasztása
    std::uniform_int_distribution<> entrance_dist(0, valid_entrances.size() - 1);
    std::uniform_int_distribution<> exit_dist(0, valid_exits.size() - 1);
    
    Point start_point = valid_entrances[entrance_dist(gen_)];
    Point end_point = valid_exits[exit_dist(gen_)];

    // 3. LEGRÖVIDEBB út keresése (A*)
    std::vector<Point> shortest_path = solver_->findShortestPathAStar(start_point, end_point);

    // 4. ÖSSZES út keresése (DFS)
    std::vector<std::vector<Point>> all_paths = solver_->findAllPathsDFS(start_point, end_point);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end_time - start_time;

    // 5. JAVÍTVA: Összesített, színes logolás (DFS NÉLKÜL)
    RCLCPP_INFO(this->get_logger(), "---------------------------------");
    RCLCPP_INFO(this->get_logger(), "--- Útvonal Tervezés LOG ---");
    RCLCPP_INFO(this->get_logger(), "Generálás és A* keresési idő: " LOG_CYAN "%.2f ms" LOG_RESET, duration.count());
    RCLCPP_INFO(this->get_logger(), "Kiválasztott pontok: " LOG_CYAN "Start(%d, %d)" LOG_RESET "," LOG_CYAN " Cél(%d, %d)" LOG_RESET, start_point.x, start_point.y, end_point.x, end_point.y);

    std::string result_message;
    bool success = false;

    // VISSZAÁLLÍTVA: Most már az all_paths-t ellenőrizzük
    if (!all_paths.empty()) {
        success = true;
        // Színes log kimenet
        std::stringstream ss_color_log;
        ss_color_log << "Siker! " << LOG_YELLOW << all_paths.size() << LOG_RESET << " lehetséges út közül a legrövidebb: "
                     << LOG_RED << shortest_path.size() << LOG_RESET << " lépés.";
        RCLCPP_INFO(this->get_logger(), "%s", ss_color_log.str().c_str());

        // Sima string a szerviz válaszhoz (színkódok nélkül)
        std::stringstream ss_raw_msg;
        ss_raw_msg << "Siker! " << all_paths.size() << " lehetséges út közül a legrövidebb: " << shortest_path.size() << " lépés.";
        result_message = ss_raw_msg.str();
    } else {
        RCLCPP_WARN(this->get_logger(), LOG_RED "Nem található útvonal!" LOG_RESET);
        result_message = "Nem található útvonal!";
    }

    RCLCPP_INFO(this->get_logger(), "---------------------------------");

    // 6. JAVÍTVA: Vizualizáció (CSAK a piros út)
    publish_visualizations(shortest_path, start_point, end_point);

    return {success, result_message};
}

void PathfinderNode::publish_visualizations(
    const std::vector<Point>& shortest_path,
    Point start, Point end)
{
    auto now = this->get_clock()->now();
    double map_resolution = 0.1;
    std::string frame_id = "map";

    visualization_msgs::msg::MarkerArray marker_array;

    // 1. OccupancyGrid (Labirintus)
    nav_msgs::msg::OccupancyGrid grid_msg;
    grid_msg.header.stamp = now;
    grid_msg.header.frame_id = frame_id;
    grid_msg.info.resolution = map_resolution;
    grid_msg.info.width = map_size_;
    grid_msg.info.height = map_size_;
    grid_msg.info.origin.position.x = 0.0;
    grid_msg.info.origin.position.y = 0.0;
    grid_msg.data.resize(map_size_ * map_size_);

    const auto& map_grid = solver_->getMapGrid(); // Térkép lekérése a solver-től
    for (int y = 0; y < map_size_; ++y) {
        for (int x = 0; x < map_size_; ++x) {
            grid_msg.data[y * map_size_ + x] = map_grid[y][x];
        }
    }
    grid_pub_->publish(grid_msg);

    // 2. Előző útvonalak törlése (Minden névtérből, ami kell)
    visualization_msgs::msg::Marker delete_marker;
    delete_marker.header.frame_id = frame_id;
    delete_marker.header.stamp = now;
    delete_marker.action = visualization_msgs::msg::Marker::DELETEALL;
    
    delete_marker.ns = "path";
    marker_array.markers.push_back(delete_marker);
    delete_marker.ns = "points";
    marker_array.markers.push_back(delete_marker);
    delete_marker.ns = "other_paths";
    marker_array.markers.push_back(delete_marker);


    // 3. Legrövidebb útvonal (Vastag piros)
    visualization_msgs::msg::Marker shortest_path_marker;
    shortest_path_marker.header.frame_id = frame_id;
    shortest_path_marker.header.stamp = now;
    shortest_path_marker.ns = "path";
    shortest_path_marker.id = 1;
    shortest_path_marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
    shortest_path_marker.action = visualization_msgs::msg::Marker::ADD;
    shortest_path_marker.scale.x = map_resolution * 0.5; // Vastagság
    shortest_path_marker.color.r = 1.0;
    shortest_path_marker.color.g = 0.0;
    shortest_path_marker.color.b = 0.0;
    shortest_path_marker.color.a = 1.0;
    
    for (const auto& p : shortest_path) {
        geometry_msgs::msg::Point gp;
        gp.x = (p.x + 0.5) * map_resolution;
        gp.y = (p.y + 0.5) * map_resolution;
        gp.z = 0.01; // Legfelül
        shortest_path_marker.points.push_back(gp);
    }
    marker_array.markers.push_back(shortest_path_marker);

    // 4. Bejárat (zöld) és Kijárat (kék)
    auto create_point_marker = [&](Point p, int id, float r, float g, float b) {
        visualization_msgs::msg::Marker marker;
        marker.header.frame_id = frame_id;
        marker.header.stamp = now;
        marker.ns = "points";
        marker.id = id;
        marker.type = visualization_msgs::msg::Marker::CUBE;
        marker.action = visualization_msgs::msg::Marker::ADD;
        marker.pose.position.x = (p.x + 0.5) * map_resolution;
        marker.pose.position.y = (p.y + 0.5) * map_resolution;
        marker.pose.position.z = 0.01;
        marker.scale.x = map_resolution;
        marker.scale.y = map_resolution;
        marker.scale.z = 0.01;
        marker.color.r = r;
        marker.color.g = g;
        marker.color.b = b;
        marker.color.a = 1.0;
        return marker;
    };
    marker_array.markers.push_back(create_point_marker(start, 2, 0.0, 1.0, 0.0));
    marker_array.markers.push_back(create_point_marker(end, 3, 0.0, 0.0, 1.0));

    marker_pub_->publish(marker_array);
}