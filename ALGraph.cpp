/*!*****************************************************************************
\file ALGraph.cpp
\author Lim Jian Rong (jianrong.lim@digipen.edu / 2201501@sit.singaporetech.edu.sg)
\par Course: CSD2183 Section B
\par Assignment: 4
\date 19/03/2024
\brief This file has definitions for functions for a ALGraph class. It implements 
       a constructor for a ALGraph object, helper functions to add directed or
       undirected edges, as well as using Dijkstra algorithm to find paths to all nodes
       from a single node.
*******************************************************************************/
#include "ALGraph.h"
#include <algorithm> //needed for std::sort to sort from small to large


const unsigned INFINITY_ = static_cast<unsigned>(-1);

    /*!
      \brief
      deconstructor for ALGraph

      \return
      none
    */
    ALGraph::~ALGraph(){
        total_num_of_nodes = 0;
        src_to_node.clear();
        aList.clear();
    }

    /*!
      \brief
      constructor for ALGraph

      \param size
      number of nodes that the graph will have

      \return
      none
    */
    ALGraph::ALGraph(unsigned size){
        total_num_of_nodes = size;

        //allocate memory for all nodes in the graph
        for(unsigned int i=0; i < size; ++i){
            std::vector<AdjacencyInfo> tmp;
            aList.push_back(tmp);
        }
    }

    /*!
      \brief
      function to get aList in ALGraph

      \return
      returns aList of type ALIST, which is a vector of vector of AdjacencyInfo
    */
    ALIST ALGraph::GetAList() const {
        return aList;
    }

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
    void ALGraph::AddDEdge(unsigned source, unsigned destination, unsigned weight) {
        AdjacencyInfo info;
        info.id = destination;
        info.weight = weight;

        //only store into node 1, not node 2
        aList[source-1].push_back(info);

        //sort each node's AdjacencyInfo by size
        check_smaller_than smaller_than;
        for(unsigned int i=0; i < aList.size(); ++i){
            std::sort(aList[i].begin(), aList[i].end(), smaller_than);
        }
    }

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
    void ALGraph::AddUEdge(unsigned node1, unsigned node2, unsigned weight) {
        AdjacencyInfo info;
        info.id = node2;
        info.weight = weight;
        //store into node 1
        aList[node1-1].push_back(info);

        AdjacencyInfo info2;
        info2.id = node1;
        info2.weight = weight;

        //store into node 2
        aList[node2-1].push_back(info2);

        //sort adjacency info for every node
        check_smaller_than smaller_than;
        for(unsigned int i=0; i < aList.size(); ++i){
            std::sort(aList[i].begin(), aList[i].end(), smaller_than);
        }
    }

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
    std::vector<DijkstraInfo> ALGraph::Dijkstra(unsigned start_node) const {

        //create vector of dist and previous
        std::vector<int> dist(total_num_of_nodes, INFINITY_);
        std::vector<int> previous(total_num_of_nodes, -1);

        //distance to start node, from the start node, is 0
        dist[start_node-1] = 0;
        AdjacencyInfo info;
        info.id = start_node;
        info.weight = 0;
        priority_queue.push(info);

        //while pq is not empty
        while(!priority_queue.empty()){
        unsigned int u = priority_queue.top().id;

            //extract the min, visit it
            priority_queue.pop();
            for(auto& neighbor : aList[u-1]){
                unsigned int v = neighbor.id;
                unsigned int weight = neighbor.weight;

                //if distance to node +weight is lesser than current stored distance, replace it
                if(dist[u-1] + weight < static_cast<unsigned>(dist[v-1])){
                    dist[v-1] = dist[u-1] + weight;
                    previous[v-1] = u;
                    AdjacencyInfo info2;
                    info2.id = v;

                    //store into pq
                    info2.weight = dist[v-1];
                    priority_queue.push(info2);
                }
            }
        }

        //store finalized values in dist and previous into src_to_node map
        for(unsigned int i=0; i < total_num_of_nodes; ++i){
            src_to_node[i].cost = dist[i];
                unsigned int curr_node =i;

                //if previous node of current node is -1, break while loop
                while(previous[curr_node] != (-1)){
                    src_to_node[i].path.insert(src_to_node[i].path.begin(),previous[curr_node]);
                    curr_node = previous[curr_node]-1;
                }

                //if distance to node is not equal to -1, push back
                if(dist[i] != (-1)){
                    src_to_node[i].path.push_back(i+1);
                }

        }

        //convert to vector only for the start node, and return
        std::vector<DijkstraInfo> tmp;
        for(unsigned int i=0; i < total_num_of_nodes;++i){
            tmp.push_back(src_to_node[i]);
        }
        return tmp;
    }

