/*!*****************************************************************************
\file ALGraph.h
\author Lim Jian Rong (jianrong.lim@digipen.edu / 2201501@sit.singaporetech.edu.sg)
\par Course: CSD2183 Section B
\par Assignment: 4
\date 19/03/2024
\brief This file has declarations for functions for a ALGraph class. It also implements
       the ALGraph, AdjacencyInfo, DijkstraInfo struct, as well as check_greater_than
       and check_smaller_than functor which is used for sorting and in the priority
       queue. ALGraph provides functionality to create a graph of nodes, add
       edges between these nodes, and use Dijkstra algorithm to calculate the paths
       to every other node, starting from an arbitrary source node.
*******************************************************************************/
//---------------------------------------------------------------------------
#ifndef ALGRAPH_H
#define ALGRAPH_H
//---------------------------------------------------------------------------
#include <vector> //used for ALIST 
#include <queue> //used for priority queue
#include <map> //used for Dijkstra Algorithm

//struct used to keep track of cost and the path from source node to another node
struct DijkstraInfo
{
  unsigned cost;
  std::vector<unsigned> path;
};

//struct used to keep track of information about adjacent nodes
struct AdjacencyInfo
{
  unsigned id;
  unsigned weight;
};


typedef std::vector<std::vector<AdjacencyInfo>> ALIST;

//functor used for priority queue
struct check_greater_than {
    bool operator()(const AdjacencyInfo& lhs, const AdjacencyInfo& rhs){
        return lhs.weight > rhs.weight;
    }
};

//functor used for sorting 
struct check_smaller_than {
    bool operator()(const AdjacencyInfo& lhs, const AdjacencyInfo& rhs){
        if(lhs.weight <= rhs.weight){
          if(lhs.weight < rhs.weight){
            return true;
          }
          else if((lhs.weight == rhs.weight) && (lhs.id < rhs.id)){
           return true;
          }

        }

        return false;
    }
};

class ALGraph
{
  public:

    /*!
      \brief
      constructor for ALGraph

      \param size
      number of nodes that the graph will have

      \return
      none
    */
    ALGraph(unsigned size);

    /*!
      \brief
      deconstructor for ALGraph

      \return
      none
    */
    ~ALGraph(void);

    /*!
      \brief
      function to add a directed edge between two nodes

      \param source
      node that the edge starts from

      \param destination
      node that the edge is moving towards

      \param weight
      weight of this edge

      \return
      none
    */
    void AddDEdge(unsigned source, unsigned destination, unsigned weight);

    /*!
      \brief
      function to add a undirected edge between two nodes

      \param node1
      first node

      \param node2
      second node

      \param weight
      weight of this edge

      \return
      none
    */
    void AddUEdge(unsigned node1, unsigned node2, unsigned weight);

    /*!
      \brief
      function that implements Dijkstra algorithm to calculate all paths to every
      node, starting from an arbitrary node

      \param start_node
      starting node for the algorithm

      \return
      returns a vector of DijkstraInfo, keeping track of the total cost of the path
      as well as the nodes that the path consist of
    */
    std::vector<DijkstraInfo> Dijkstra(unsigned start_node) const;

    /*!
      \brief
      function to get aList in ALGraph

      \return
      returns aList of type ALIST, which is a vector of vector of AdjacencyInfo
    */
    ALIST GetAList(void) const;
        
  private:
     ALIST aList;
     unsigned int total_num_of_nodes;
    mutable std::priority_queue<AdjacencyInfo, std::vector<AdjacencyInfo>, check_greater_than> priority_queue;
    mutable std::map<unsigned int, DijkstraInfo> src_to_node;
};
#endif
