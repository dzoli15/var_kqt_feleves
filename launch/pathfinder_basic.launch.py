from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 1. NODE: Az EREDETI pathfinder (A* és DFS logikával)
        Node(
            package='var_kqt_feleves',
            executable='pathfinder_node',
            name='pathfinder_node',
            output='screen',
            parameters=[
                {'map_size': 15},
                {'automatic_mode': True}   # Automata mód (hogy ne kelljen 3. terminál a triggereléshez)
            ]
        ),

        # 2. NODE: RViz2
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen'
        )
    ])
