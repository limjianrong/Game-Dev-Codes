#include <pch.h>
#include "Projects/ProjectTwo.h"
#include "P2_Pathfinding.h"
#include <list>
#include <iterator>


const int num_portions = 30;
int active_portions[num_portions];

const int portion_size = open_list_size / num_portions;
const float inv_portion_size = 1.f / portion_size;

//int last_active_index = 0;
using xy = char[2];
xy arr_xy[8] = { {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1} };

Node node_map[40][40];
mini_arr open_list_arr[open_list_size];

float slices_per_interval{ 4.f };

int index_of_cheapest_bucket = -1;
int largest = 0;

float rfw_distances[40 * 40][40 * 40];
int rfw_closest_node_index[40 * 40][40 * 40];
#define root2 1.41f

#pragma region Extra Credit
bool ProjectTwo::implemented_floyd_warshall()
{
    return true;
}

bool ProjectTwo::implemented_goal_bounding()
{
    return false;
}

bool ProjectTwo::implemented_jps_plus()
{
    return false;
}
#pragma endregion


int gridpos_to_index(int row, int col) {
    return row * terrain->get_map_width() + col;
}

GridPos index_to_gridpos(int index) {
    int row = index / terrain->get_map_width();
    int col = index - row * terrain->get_map_width();
    return GridPos{ row,col };

}
void AStarPather::precompute_roy_floyd() {

    for (int i = 0; i < terrain->get_map_height() * terrain->get_map_width(); ++i) {
        for (int u = 0; u < terrain->get_map_height() * terrain->get_map_width(); ++u) {
            if (!terrain->is_valid_grid_position(index_to_gridpos(i)) ||
                !terrain->is_valid_grid_position(index_to_gridpos(u))) {
                continue;
            }
            if (terrain->is_wall(index_to_gridpos(i)) ||
                terrain->is_wall(index_to_gridpos(u))) {
                rfw_distances[i][u] = std::numeric_limits<float>::max();
                rfw_closest_node_index[i][u] = -1;
            }
            else if (i == u) {
                rfw_distances[i][u] = 0;
                rfw_closest_node_index[i][u] = i;
            }
            else {
                rfw_distances[i][u] = std::numeric_limits<float>::max();
                rfw_closest_node_index[i][u] = -1;
            }
        }
    }

    for (int i = 0; i < terrain->get_map_height(); ++i) {
        for (int u = 0; u < terrain->get_map_width(); ++u) {
            if (terrain->is_wall(i, u)) {
                continue;
            }
            int idx = gridpos_to_index(i, u);
            for (int y = -1; y <= 1; ++y) {
                for (int x = -1; x <= 1; ++x) {
                    int neighbor_row = i + y;
                    int neighbor_col = u + x;
                    int neighbour_index = gridpos_to_index(neighbor_row, neighbor_col);
                    if (neighbour_index < 0 || neighbour_index >= (terrain->get_map_height() * terrain->get_map_width())) {
                        continue;
                    }
                    if (!terrain->is_valid_grid_position(neighbor_row, neighbor_col)) {
                        continue;
                    }
                    if (terrain->is_wall(neighbor_row, neighbor_col)) {
                        continue;
                    }

                    else if (terrain->is_wall(neighbor_row, u) && (y == 1 || y == -1)) {
                        continue;

                    }
                    else if (terrain->is_wall(i, neighbor_col) && (x == 1 || x == -1)) {
                        continue;

                    }

                    if ((neighbor_row != i && neighbor_col == u) || (neighbor_row==i && neighbor_col!=u)){
                        rfw_distances[idx][neighbour_index] = 1;
                        rfw_closest_node_index[idx][neighbour_index] = idx;

                    }
                    else {
                        rfw_distances[idx][neighbour_index] = 1.41f;
                        rfw_closest_node_index[idx][neighbour_index] = idx;
                    }


                }
            }
        }
    }

        for (int k = 0; k < terrain->get_map_height() * terrain->get_map_height(); ++k) {
            for (int i = 0; i < terrain->get_map_height() * terrain->get_map_height(); ++i) {
                for (int u = 0; u < terrain->get_map_width() * terrain->get_map_width(); ++u) {
                    if (rfw_distances[i][k] < std::numeric_limits<float>::max() && rfw_distances[k][u] < std::numeric_limits<float>::max()) {
                        if (rfw_distances[i][u] > (rfw_distances[i][k] + rfw_distances[k][u])) {

                            rfw_distances[i][u] = rfw_distances[i][k] + rfw_distances[k][u];

                            rfw_closest_node_index[i][u] = rfw_closest_node_index[k][u];
                        }
                    }
                }
            }
        }

}

bool AStarPather::initialize()
{
    // handle any one-time setup requirements you have

    /*
        If you want to do any map-preprocessing, you'll need to listen
        for the map change message.  It'll look something like this:

        Callback cb = std::bind(&AStarPather::your_function_name, this);
        Messenger::listen_for_message(Messages::MAP_CHANGE, cb);

        There are other alternatives to using std::bind, so feel free to mix it up.
        Callback is just a typedef for std::function<void(void)>, so any std::invoke'able
        object that std::function can wrap will suffice.
    */


    Callback changeMapCallBack = std::bind(&AStarPather::precompute_neighbours, this);
   Messenger::listen_for_message(Messages::MAP_CHANGE, changeMapCallBack);


   Callback royFloydChangeMapCallBack = std::bind(&AStarPather::precompute_roy_floyd, this);
   Messenger::listen_for_message(Messages::MAP_CHANGE, royFloydChangeMapCallBack);

    for (int i = 0; i < 40; ++i) {
        for (int u = 0; u < 40; ++u) {
            node_map[i][u].final_cost = 0.f;
            node_map[i][u].given_cost = 0.f;
            node_map[i][u].list_type = list::no_list;
            node_map[i][u].parent_row = -1;
            node_map[i][u].parent_col = -1;
            
            node_map[i][u].grid_pos.row = i;
            node_map[i][u].grid_pos.col = u;
            node_map[i][u].neighbours = 0;

        }
    }

    for (int i = 0; i < open_list_size; ++i) {
        open_list_arr[i].second = -1;
        for (int u = 0; u < mini_arr_size; ++u) {
            open_list_arr[i].first[u] = nullptr;

        }
    }

    for (int i = 0; i < num_portions; ++i) {
        active_portions[i] = 0;
    }


    for (int i = 0; i < 40 * 40; ++i) {
        for (int j = 0; j < 40 * 40; ++j) {
                rfw_distances[i][j] = std::numeric_limits<float>::max();
                rfw_closest_node_index[i][j] = -1;
        }
    }

    return true; // return false if any errors actually occur, to stop engine initialization
}

void AStarPather::shutdown()
{
    /*
        Free any dynamically allocated memory or any other general house-
        keeping you need to do during shutdown.
    */
    for (int i = 0; i < 40; ++i) {
        for (int u = 0; u < 40; ++u) {
            node_map[i][u].final_cost =0.f;
            node_map[i][u].given_cost = 0.f;
            node_map[i][u].list_type = list::no_list;
            node_map[i][u].neighbours = 0;
           // node_map[i][u].parent = nullptr;
            node_map[i][u].parent_row = 0;
            node_map[i][u].parent_col = 0;
            node_map[i][u].grid_pos.row = 0;
            node_map[i][u].grid_pos.col = 0;
        }
    }

    for (int i = 0; i < open_list_size; ++i) {
        open_list_arr[i].second = 0;
        for (int u = 0; u < mini_arr_size; ++u) {
            open_list_arr[i].first[u] = nullptr;
        }
    }

    for (int i = 0; i < num_portions; ++i) {
        active_portions[i] = 0;
    }
}


PathResult AStarPather::compute_path(PathRequest &request)
{
    
    /*
        This is where you handle pathing requests, each request has several fields:

        start/goal - start and goal world positions
        path - where you will build the path upon completion, path should be
            start to goal, not goal to start
        heuristic - which heuristic calculation to use
        weight - the heuristic weight to be applied
        newRequest - whether this is the first request for this path, should generally
            be true, unless single step is on

        smoothing - whether to apply smoothing to the path
        rubberBanding - whether to apply rubber banding
        singleStep - whether to perform only a single A* step
        debugColoring - whether to color the grid based on the A* state:
            closed list nodes - yellow
            open list nodes - blue

            use terrain->set_color(row, col, Colors::YourColor);
            also it can be helpful to temporarily use other colors for specific states
            when you are testing your algorithms

        method - which algorithm to use: A*, Floyd-Warshall, JPS+, or goal bounding,
            will be A* generally, unless you implement extra credit features

        The return values are:
            PROCESSING - a path hasn't been found yet, should only be returned in
                single step mode until a path is found
            COMPLETE - a path to the goal was found and has been built in request.path
            IMPOSSIBLE - a path from start to goal does not exist, do not add start position to path
    */

    // WRITE YOUR CODE HERE

    
    // Just sample code, safe to delete
    const GridPos start = terrain->get_grid_position(request.start);
    const GridPos goal = terrain->get_grid_position(request.goal);

    if (request.newRequest) {
        if (request.settings.debugColoring) {
            terrain->set_color(start, Colors::Blue);
        }

        request.path.clear();
        index_of_cheapest_bucket = -1;
        for (int i = 0; i < num_portions; ++i) {
            active_portions[i] = 0;
        }
        for (int i = 0; i < terrain->get_map_height(); ++i) {
            for (int u = 0; u < terrain->get_map_width(); ++u) {
                node_map[i][u].list_type = list::no_list;
                node_map[i][u].parent_row = -1;
                node_map[i][u].parent_col = -1;
            }
        }

        for (int i = 0; i < open_list_size; ++i) {
            open_list_arr[i].second = -1;
        }

        if (request.settings.method == Method::FLOYD_WARSHALL) {
            int start_1d_index = start.row * terrain->get_map_width() + start.col;
            int end_1d_index = goal.row * terrain->get_map_width() + goal.col;
            if (rfw_distances[start_1d_index][end_1d_index] == std::numeric_limits<float>::max()) {
                return PathResult::IMPOSSIBLE;
            }

            request.path.push_front(terrain->get_world_position(goal));
            for (int curr = end_1d_index; curr != start_1d_index; curr = rfw_closest_node_index[start_1d_index][curr]) {
                GridPos cell_grid_pos{};

                cell_grid_pos.row = rfw_closest_node_index[start_1d_index][curr] / terrain->get_map_width();
                cell_grid_pos.col = rfw_closest_node_index[start_1d_index][curr] - (terrain->get_map_width()*cell_grid_pos.row);
                request.path.push_front(terrain->get_world_position(cell_grid_pos));
            }

            return PathResult::COMPLETE;

        }

        float hx{};
        float xdiff = static_cast<float>(std::abs(goal.col - start.col));
       // float xdiff = (goal.col - start.col) < 0 ?  -static_cast<float>(goal.col-start.col) : static_cast<float>(goal.col - start.col);
        float ydiff = static_cast<float>(std::abs(goal.row - start.row));
       // float ydiff = (goal.row-start.row) < 0? -static_cast<float>(goal.row - start.row) : static_cast<float>(goal.row - start.row);
        float min = xdiff > ydiff ? ydiff : xdiff;
        float max = xdiff > ydiff ? xdiff : ydiff;

        switch (request.settings.heuristic) {
        case Heuristic::OCTILE:
            hx = min* 0.41f + max;
            break;

        case Heuristic::EUCLIDEAN:
            hx = (sqrt(xdiff * xdiff + ydiff*ydiff));
            break;

        case Heuristic::INCONSISTENT:
            if ((start.row + start.col) % 2 > 0) {
                hx = (sqrt(xdiff * xdiff + ydiff * ydiff));
                break;
            }
            else {
                break;
            }

        case Heuristic::MANHATTAN:
            hx = xdiff+ydiff;
            break;

        case Heuristic::CHEBYSHEV:
            hx = max;
            break;

        default:
            break;
        }

        Node& start_node = node_map[start.row][start.col];
        start_node.given_cost = 0.f;
       start_node.final_cost = hx* request.settings.weight;

       index_of_cheapest_bucket = static_cast<int>(start_node.final_cost * slices_per_interval);

       open_list_arr[index_of_cheapest_bucket].second = 0;
        open_list_arr[index_of_cheapest_bucket].first[0] = (&(node_map[start.row][start.col]));
        start_node.list_type = list::on_open_list;
        active_portions[static_cast<int>(index_of_cheapest_bucket * (inv_portion_size))]++;



    }

    for( ;  index_of_cheapest_bucket >=0  ; ) {

        int index_of_cheapest_in_mini_arr = 0;
        Node* cheapest_node = open_list_arr[index_of_cheapest_bucket].first[0];

        for (int i = 1; i <= open_list_arr[index_of_cheapest_bucket].second; ++i) {
            if (open_list_arr[index_of_cheapest_bucket].first[i]->final_cost < cheapest_node->final_cost) {
                cheapest_node = open_list_arr[index_of_cheapest_bucket].first[i];
                index_of_cheapest_in_mini_arr = i;
            }
        }

        const Node& copy_of_cheapest_node = *cheapest_node;
        if (copy_of_cheapest_node.grid_pos == goal) {
            
            request.path.push_front(terrain->get_world_position(goal));
            if (request.settings.rubberBanding) {
                while (cheapest_node->parent_row >= 0 && cheapest_node->parent_col >= 0) {
                    if (node_map[cheapest_node->parent_row][cheapest_node->parent_col].parent_row >= 0 &&
                        node_map[cheapest_node->parent_row][cheapest_node->parent_col].parent_col >= 0) {
                        
                        int temp_goal_node_row = cheapest_node->grid_pos.row;
                        int temp_goal_node_col = cheapest_node->grid_pos.col;

                        int temp_start_node_row = node_map[cheapest_node->parent_row][cheapest_node->parent_col].parent_row;
                        int temp_start_node_col = node_map[cheapest_node->parent_row][cheapest_node->parent_col].parent_col;

                        int minx = temp_goal_node_col > temp_start_node_col ? temp_start_node_col : temp_goal_node_col;
                        int miny = temp_goal_node_row > temp_start_node_row ? temp_start_node_row : temp_goal_node_row;

                        int maxx = temp_goal_node_col > temp_start_node_col ? temp_goal_node_col : temp_start_node_col;
                        int maxy = temp_goal_node_row > temp_start_node_row ? temp_goal_node_row : temp_start_node_row;
                        bool wall_found = false;
                        
                        for (int i = miny; i <= maxy; ++i) {
                            for (int u = minx; u <= maxx; ++u) {
                                if (terrain->is_wall(i, u)) {
                                    wall_found = true;
                                    break;
                                }
                            }
                            if (wall_found) {
                                break;
                            }
                        }
                        if (!wall_found) {
                            cheapest_node->parent_row = temp_start_node_row;
                            cheapest_node->parent_col = temp_start_node_col;

                        }
                        else {
                            request.path.push_front(terrain->get_world_position((node_map[cheapest_node->parent_row][cheapest_node->parent_col].grid_pos)));
                            cheapest_node = &node_map[cheapest_node->parent_row][cheapest_node->parent_col];
                        }
                    }

                    // cheapest_node->parent->parent does not exist, simply push cheapest_node->parent
                    // set cheapest = cheapest->parent, and in the next while loop, the cheapest->parent will
                    // return false, and the while loop breaks
                    else {
                        request.path.push_front(terrain->get_world_position((node_map[cheapest_node->parent_row][cheapest_node->parent_col].grid_pos)));
                        cheapest_node = &node_map[cheapest_node->parent_row][cheapest_node->parent_col];
                    }

                }
            } //end of if (rubberbanding)

            //if no rubber banding, enter here
            else {
                while (cheapest_node->parent_row >=0 && cheapest_node->parent_col >=0) {

                    request.path.push_front(terrain->get_world_position((node_map[cheapest_node->parent_row][cheapest_node->parent_col].grid_pos)));
                    cheapest_node = &node_map[cheapest_node->parent_row][cheapest_node->parent_col];
                } 
            }
            if (request.settings.smoothing) {
                int inserted_count = 0;

                if (request.settings.rubberBanding) {
                    for (auto iter = request.path.begin(); iter != request.path.end(); ) {
                        auto next_node = iter;
                        ++next_node;
                        if (next_node == request.path.end()) {
                            break;
                        }
                        GridPos pos1{ terrain->get_grid_position(*iter) };
                        GridPos pos2{ terrain->get_grid_position(*next_node) };

                        float distance = static_cast<float>(sqrt((pos1.row - pos2.row) * (pos1.row - pos2.row) + (pos1.col - pos2.col) * (pos1.col - pos2.col)));
                        if (distance > 1.5f) {
                            request.path.insert(next_node, (*iter + *next_node) * 0.5f);
                            iter = request.path.begin();
                        }
                        else {
                            ++iter;
                        }
                    }
                }

                const std::list<Vec3> copy_of_original_path = request.path;
                auto it_0 = request.path.begin();

                auto it_1 = copy_of_original_path.begin();
                auto it_2 = copy_of_original_path.begin();
                std::advance(it_2, 1);

                auto it_3 = copy_of_original_path.begin();
                std::advance(it_3, 2);

                auto it_4 = copy_of_original_path.begin();
                std::advance(it_4, 3);

                for (int count = 0; count < 3; ++count) {
                      Vec3 new_pos = Vec3::CatmullRom(*it_1, *it_1, *it_2, *it_3, count * 0.25f + 0.25f);
                      ++inserted_count;
                      it_0 = request.path.begin();
                      std::advance(it_0, inserted_count);
                      request.path.insert(it_0, new_pos);

                }

                for (int i = 1; i < copy_of_original_path.size()-1; ++i) {
                      if (i == copy_of_original_path.size() - 2) {
                          for (int count = 0; count < 3; ++count) {
                              Vec3 new_pos = Vec3::CatmullRom(*it_1, *it_2, *it_3, *it_3, count * 0.25f + 0.25f);
                              ++inserted_count;
                              it_0 = request.path.begin();
                              std::advance(it_0, inserted_count + i);
                              request.path.insert(it_0, new_pos);

                          }
                          break;
                      }

                      for (int count = 0; count < 3; ++count) {
                          Vec3 new_pos = Vec3::CatmullRom(*it_1, *it_2, *it_3, *it_4, count * 0.25f + 0.25f);
                          ++inserted_count;
                          it_0 = request.path.begin();
                          std::advance(it_0, inserted_count+i);
                          request.path.insert(it_0, new_pos);

                      }
                      std::advance(it_1, 1);
                      std::advance(it_2, 1);
                      std::advance(it_3, 1);
                      std::advance(it_4, 1);
                }

                
            } //end of if (smoothing)
            return PathResult::COMPLETE;
        }


        (cheapest_node)->list_type = list::on_closed_list;
        open_list_arr[index_of_cheapest_bucket].first[index_of_cheapest_in_mini_arr] = &(*(open_list_arr[index_of_cheapest_bucket].first[open_list_arr[index_of_cheapest_bucket].second--]));

            active_portions[static_cast<int>(index_of_cheapest_bucket* (inv_portion_size))]--;

        if (request.settings.debugColoring) {
            terrain->set_color(cheapest_node->grid_pos, Colors::Yellow);
        }
        int cheapest_node_row = copy_of_cheapest_node.grid_pos.row;
        int cheapest_node_col = copy_of_cheapest_node.grid_pos.col;

        for (int i = 0; i < 8; ++i) {
            int neighbour_row{};
            int neighbour_col{};
            if (copy_of_cheapest_node.neighbours & (1u << i)) {
                neighbour_row = cheapest_node_row + arr_xy[i][1];
                neighbour_col = cheapest_node_col + arr_xy[i][0];
            }

            else {
                continue;
            }

            float given_cost{};
            if ((neighbour_row != cheapest_node_row && neighbour_col == cheapest_node_col) || (neighbour_row == cheapest_node_row && neighbour_col != cheapest_node_col)) {
                given_cost = copy_of_cheapest_node.given_cost + 1;
            }
            else {
                given_cost = copy_of_cheapest_node.given_cost + 1.41f;
            }

            float neighbour_node_hx{0.f};
            //float xdiff = (goal.col - neighbour_col) < 0 ? -static_cast<float>(goal.col - neighbour_col) : static_cast<float>(goal.col - neighbour_col);
            float xdiff = static_cast<float>(std::abs(goal.col - neighbour_col));
            //float ydiff = (goal.row - neighbour_row) < 0 ? -static_cast<float>(goal.row - neighbour_row) : static_cast<float>(goal.row - neighbour_row);
            float ydiff = static_cast<float>(std::abs(goal.row - neighbour_row));
            float min = xdiff > ydiff ? ydiff : xdiff;
            float max = xdiff > ydiff ? xdiff : ydiff;

            switch (request.settings.heuristic) {
            case Heuristic::OCTILE:
                neighbour_node_hx = min * 0.41f + max;
                break;

            case Heuristic::EUCLIDEAN:

                neighbour_node_hx = (sqrt(xdiff * xdiff + ydiff * ydiff));
                break;

            case Heuristic::INCONSISTENT:
                if(static_cast<int>(neighbour_row + neighbour_col )%2 > 0){
                    neighbour_node_hx = sqrt(xdiff * xdiff + ydiff * ydiff);
                    break;
                }
                else {
                    break;
                }

            case Heuristic::MANHATTAN:
                neighbour_node_hx = xdiff + ydiff;
                break;

            case Heuristic::CHEBYSHEV:
                neighbour_node_hx = max;
                break;

            default:
                break;
            }
            float new_final_cost = given_cost + neighbour_node_hx * request.settings.weight;

            if (node_map[neighbour_row][neighbour_col].list_type == list::no_list) {

                node_map[neighbour_row][neighbour_col].list_type = list::on_open_list;
                node_map[neighbour_row][neighbour_col].parent_row = cheapest_node->grid_pos.row;
                node_map[neighbour_row][neighbour_col].parent_col = cheapest_node->grid_pos.col;
                node_map[neighbour_row][neighbour_col].given_cost = given_cost;
                node_map[neighbour_row][neighbour_col].final_cost = new_final_cost;

                if (open_list_arr[static_cast<int>(new_final_cost * slices_per_interval)].second <= -2) {
                    open_list_arr[static_cast<int>(new_final_cost * slices_per_interval)].second = -1;
                }
                 open_list_arr[static_cast<int>(new_final_cost * slices_per_interval)].first[++(open_list_arr[static_cast<int>(new_final_cost * slices_per_interval)]).second] = &(node_map[neighbour_row][neighbour_col]);
                 
                 active_portions[(static_cast<int>(new_final_cost * slices_per_interval* inv_portion_size))]++;

                    if (static_cast<int>(new_final_cost * slices_per_interval) < index_of_cheapest_bucket) {
                        index_of_cheapest_bucket = static_cast<int>(new_final_cost * slices_per_interval);
                    }
                    if (request.settings.debugColoring) {
                        terrain->set_color(node_map[neighbour_row][neighbour_col].grid_pos, Colors::Blue);
                    }

                }

                /*
                 if code reaches here, list_type is definitely open_list or closed list
                */
                else if (new_final_cost < node_map[neighbour_row][neighbour_col].final_cost) {

                    int new_bucket_index = static_cast<int>(new_final_cost * slices_per_interval);

                    if (node_map[neighbour_row][neighbour_col].list_type == list::on_open_list) {
                        int old_bucket_index = static_cast<int>(node_map[neighbour_row][neighbour_col].final_cost * slices_per_interval);
                        for (int i = 0; i < open_list_arr[old_bucket_index].second; ++i) {
                            Node* tmp = open_list_arr[old_bucket_index].first[i];
                                if (tmp->grid_pos == node_map[neighbour_row][neighbour_col].grid_pos) {
                                    open_list_arr[old_bucket_index].first[i] = open_list_arr[old_bucket_index].first[open_list_arr[old_bucket_index].second--];
                                        active_portions[static_cast<int>(old_bucket_index * inv_portion_size)]--;

                                    break;
                                }
                            
                        }
                    }


                    node_map[neighbour_row][neighbour_col].final_cost = new_final_cost;
                    node_map[neighbour_row][neighbour_col].parent_row = cheapest_node->grid_pos.row;
                    node_map[neighbour_row][neighbour_col].parent_col = cheapest_node->grid_pos.col;
                    node_map[neighbour_row][neighbour_col].given_cost = given_cost;
                    node_map[neighbour_row][neighbour_col].list_type = list::on_open_list;

                   
                    open_list_arr[new_bucket_index].first[++(open_list_arr[new_bucket_index].second)] = &node_map[neighbour_row][neighbour_col];
                    active_portions[static_cast<int>(new_bucket_index *inv_portion_size)]++;

                }
            
        }
        if (open_list_arr[index_of_cheapest_bucket].second < 0) {
            index_of_cheapest_bucket = -1;
            for (int i = 0; i < num_portions; ++i) {
                if (active_portions[i] > 0) {
                    for (int u = 0; u < portion_size; ++u) {
                        if (open_list_arr[i * portion_size + u].second >= 0) {
                            index_of_cheapest_bucket = i * portion_size + u;
                            break;
                        }
                    }
                }
                else {
                    continue;
                }
                break;
                
                
            }
        }
        if (request.settings.singleStep) {
            return PathResult::PROCESSING;
        }
    }

    if(index_of_cheapest_bucket <0){
        return PathResult::IMPOSSIBLE;
    }

    return PathResult::COMPLETE;
}

std::ostream& operator<<(std::ostream& out, const GridPos& rhs) {
    out << "("<< rhs.row <<"," << rhs.col <<")";
    return out;
}



void AStarPather::precompute_neighbours() {

    for (int row = 0; row < terrain->get_map_height(); ++row) {
        for (int col = 0; col < terrain->get_map_width(); ++col) {
            node_map[row][col].neighbours = 0;

            for (int y = -1; y <= 1; ++y) {
                if (row + y < 0 || row + y >= terrain->get_map_height()) {
                    continue;
                }
                for (int x = -1; x <= 1; ++x) {
                    if (x == 0 && y == 0) {
                        continue;
                    }
                    else if (col + x < 0 || col + x >= terrain->get_map_width()) {
                        continue;
                    }
                    int neighbour_row = row + y;
                    int neighbour_col = col + x;

                    if (terrain->is_wall(neighbour_row, neighbour_col)) {
                        continue;
                    }

                    else if (terrain->is_wall(neighbour_row, col) && (y == 1 || y == -1)) {
                        continue;

                    }
                    else if (terrain->is_wall(row, neighbour_col) && (x == 1 || x == -1)) {
                        continue;

                    }
                    else {
                        if (y == 1 && x == 0) {
                            node_map[row][col].neighbours |= (1u << 7);
                        }
                        else if (y == 1 && x == 1) {
                            node_map[row][col].neighbours |= (1u << 6);
                        }
                        else if (y == 0 && x == 1) {
                            node_map[row][col].neighbours |= (1u << 5);
                        }
                        else if (y == -1 && x == 1) {
                            node_map[row][col].neighbours |= (1u << 4);
                        }
                        else if (y == -1 && x == 0) {
                            node_map[row][col].neighbours |= (1u << 3);

                        }
                        else if (y == -1 && x == -1) {
                            node_map[row][col].neighbours |= (1u << 2);

                        }
                        else if (y == 0 && x == -1) {
                            node_map[row][col].neighbours |= (1u << 1);

                        }
                        else if (y == 1 && x == -1) {
                            node_map[row][col].neighbours |= 1u;
                        }


                    }

                }
            }
        }
    }
}
