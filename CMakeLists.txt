cmake_minimum_required(VERSION 3.8)
project(var_kqt_feleves)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Függőségek keresése
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(std_msgs REQUIRED)

# 1. LIBRARY LÉTREHOZÁSA (A kód logikája)
add_library(pathfinder_lib
  src/pathfinder_node.cpp
  src/maze_solver.cpp
)

target_include_directories(pathfinder_lib PUBLIC include)

# A library-t hozzálinkeljük a ROS függőségekhez
ament_target_dependencies(pathfinder_lib
  rclcpp
  nav_msgs
  visualization_msgs
  geometry_msgs
  std_srvs
  std_msgs
)

# 2. EXECUTABLE LÉTREHOZÁSA (A main)
add_executable(pathfinder_node
  src/pathfinder_node_main.cpp
)

# A futtatható állományt hozzálinkeljük a mi library-nkhez
target_link_libraries(pathfinder_node pathfinder_lib)

# 3. TELEPÍTÉS
install(TARGETS
  pathfinder_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  pathfinder_lib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Header fájlok telepítése
install(
  DIRECTORY include/
  DESTINATION include
)

# Tesztelés (opcionális, de jó gyakorlat)
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()