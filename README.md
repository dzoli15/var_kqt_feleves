##### [INFO] [1763366882.038912313] [pathfinder_node]: Siker! 1114 lehetséges út közül a legrövidebb: 43 lépés.

##### Az 1114 nem azt jelenti, hogy 1114 teljesen különböző, logikus főcsapás van. Azt jelenti, hogy ha a robot minden egyes elágazásnál dönthetne, és minden kis hurkot megkerülhetne így is meg úgy is, akkor 1114 különböző kombinációban tudna eljutni a kéktől a zöldig anélkül, hogy önmagába harapna. ______________________________________________________________________________________________________________________________________________________________________________

Nulladik lépés: könyvtárba navigálás:
```bash
cd ~/ros2_ws
```

## Futtatás:

Első lépés:		terminal 1:
 ```bash
 colcon build --packages-select var_kqt_feleves
 ```
 Második lépés:	terminal 2:
 ```bash
 source ~/ros2_ws/install/setup.bash rviz2
 ```
 Harmadik lépés:	terminal 3:
 ```bash
 source ~/ros2_ws/install/setup.bash
 ```
{Automatikus} Az RViz-ben 10 másodpercenként frissül a kép, a terminálban pedig pörögnek a logok.
```bash
ros2 run var_kqt_feleves pathfinder_node
```
{Manuális mód}
```bash
ros2 run var_kqt_feleves pathfinder_node --ros-args -p automatic_mode:=false
```
terminal 4:
```bash
source ~/ros2_ws/install/setup.bash
ros2 service call /trigger_pathfinding std_srvs/srv/Trigger "{}"
```
Negyedik lépés:	{RViz beállítás}
- Fixed frame ->> map
- add /map_grid
- add /visualization_markers


## Log példa automatikus:

##### [INFO] [1763367653.752137217] [pathfinder_node]: Indítás AUTOMATA módban. (10 másodpercenként újratervezés)

##### [INFO] [1763367653.752390557] [pathfinder_node]: Pathfinder node elindult. Térkép méret: 15 x 15

##### [INFO] [1763367653.752460572] [pathfinder_node]: MultiThreadedExecutor indul...

##### [INFO] [1763367663.752323627] [pathfinder_node]:  

#### [INFO] [1763367663.752418070] [pathfinder_node]: ---------------------------------

##### [INFO] [1763367663.752446114] [pathfinder_node]: --- Időzített újratervezés ---

##### [INFO] [1763367666.189127684] [pathfinder_node]: ---------------------------------

##### [INFO] [1763367663.752323627] [pathfinder_node]:  

##### [INFO] [1763367666.189127684] [pathfinder_node]: ---------------------------------

##### [INFO] [1763367666.189222838] [pathfinder_node]: --- Útvonal Tervezés LOG ---

##### [INFO] [1763367666.189262094] [pathfinder_node]: Generálás és A* keresési idő: 2436.67 ms

##### [INFO] [1763367666.189299296] [pathfinder_node]: Kiválasztott pontok: Start(0, 17), Cél(20, 1)

##### [INFO] [1763367666.189351086] [pathfinder_node]: Siker! 16952 lehetséges út közül a legrövidebb: 37 lépés.

##### [INFO] [1763367666.189369041] [pathfinder_node]: ---------------------------------


## Log példa manuális:

##### [INFO] [1763367886.253602811] [pathfinder_node]: Indítás MANUÁLIS módban.

##### [INFO] [1763367886.253815473] [pathfinder_node]: Hívd a '/trigger_pathfinding' szolgáltatást a tervezéshez.

##### [INFO] [1763367886.253848757] [pathfinder_node]: Pathfinder node elindult. Térkép méret: 15 x 15

##### [INFO] [1763367886.253896419] [pathfinder_node]: MultiThreadedExecutor indul...

##### [INFO] [1763367663.752323627] [pathfinder_node]:

##### [INFO] [1763366881.429620688] [pathfinder_node]: ---------------------------------

##### [INFO] [1763366881.429681956] [pathfinder_node]: --- Manuális tervezés kérése ---

##### [INFO] [1763366882.038769527] [pathfinder_node]: ---------------------------------

##### [INFO] [1763367663.752323627] [pathfinder_node]:  

##### [INFO] [1763367666.189127684] [pathfinder_node]: ---------------------------------

##### [INFO] [1763366882.038854291] [pathfinder_node]: --- Útvonal Tervezés LOG ---

##### [INFO] [1763366882.038861555] [pathfinder_node]: Generálás és A* keresési idő: 609.02 ms

##### [INFO] [1763366882.038870542] [pathfinder_node]: Kiválasztott pontok: Start(7, 0), Cél(20, 9)

##### [INFO] [1763366882.038912313] [pathfinder_node]: Siker! 1114 lehetséges út közül a legrövidebb: 43 lépés.

##### [INFO] [1763366882.038926220] [pathfinder_node]: ---------------------------------
