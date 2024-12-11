#pragma once
#include "Misc/PathfindingDetails.hpp"

class AStarPather
{
public:
    /* 
        The class should be default constructible, so you may need to define a constructor.
        If needed, you can modify the framework where the class is constructed in the
        initialize functions of ProjectTwo and ProjectThree.
    */

    /* ************************************************** */
    // DO NOT MODIFY THESE SIGNATURES
    bool initialize();
    void shutdown();
    PathResult compute_path(PathRequest &request);
    /* ************************************************** */

    /*
        You should create whatever functions, variables, or classes you need.
        It doesn't all need to be in this header and cpp, structure it whatever way
        makes sense to you.
    */
    void precompute_neighbours();
    void precompute_roy_floyd();

};




enum list:unsigned char {
    no_list =0, on_open_list, on_closed_list
};


struct Node {
    //Node* parent;
    GridPos grid_pos;
    float given_cost;
    float final_cost;
    list list_type;
    unsigned char neighbours;

    char parent_row;
    char parent_col;

};

int const open_list_size = 600;
int const mini_arr_size = 80;

using mini_arr = std::pair<std::array<Node*, mini_arr_size>, int>;


extern Node node_map[40][40];
extern mini_arr open_list_arr[open_list_size];

std::ostream& operator<<(std::ostream& out, const GridPos& rhs);
