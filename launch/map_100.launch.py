from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 1. NODE: A Generátor programod (100x100)
        Node(
            package='var_kqt_feleves',
            executable='pathfinder_node_100',
            name='maze_generator',
            output='screen',  # Hogy lásd a logokat (printelést) a terminálban
            parameters=[
                {'map_size': 100},
                {'automatic_mode': True}
            ]
        ),

        # 2. NODE: Az RViz2 (A látvány)
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen'
        )
    ])