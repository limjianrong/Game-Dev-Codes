/*!*****************************************************************************
\file AVLTree.h
\author Lim Jian Rong (jianrong.lim@digipen.edu / 2201501@sit.singaporetech.edu.sg)
\par Course: CSD2183 Section B
\par Assignment: 3
\date 01/03/2024
\brief This file has declarations for for a AVL class template which inherits from
       a BSTree class template. It provides functionality unique to AVLTrees, such
       as constructor, automatic balancing of the tree after a insertion/deletion, rotation
       of node, calculation of balance factor of node, and more.
*******************************************************************************/

//---------------------------------------------------------------------------
#ifndef AVLTREE_H
#define AVLTREE_H
//---------------------------------------------------------------------------
#include <stack>
#include "BSTree.h"

/*!
  Definition for the AVL Tree
*/
template <typename T>
class AVLTree : public BSTree<T>
{
  public:
    /*!
      \brief
      Constructor for AVLTree object

      \param oa
        pointer to object allocator

      \param shareOA
        boolean to determine if other AVL will share OA with this AVL
    */
     AVLTree(ObjectAllocator *oa = 0, bool ShareOA = false);
     virtual ~AVLTree() = default; // DO NOT IMPLEMENT

    /*!
      \brief
      function to insert a node into AVLTree

      \param value
        value to be inserted

      \return
      none
    */
     virtual void insert(const T& value) override;

    /*!
      \brief
      function to remove a node

      \param value
      value to be removed from AVLTree

      \return
      none
    */
     virtual void remove(const T& value) override;

    //   // Returns true if efficiency implemented
    /*!
      \brief
      function that returns true if efficiency is implemented

      \return
      returns true
    */
     static bool ImplementedBalanceFactor(void);

  private:
    // private stuff...

    /*!
      \brief
      function that is recursively called by insert()

      \param parent
      parent ndoe

      \param node_ptr
      child node

      \param visited_nodes
      reference to stack of visited nodes

      \param number_of_recursions
      number of times function is recursively called

      \return
      none
    */
    void insert_node(typename AVLTree<T>::BinTreeNode*& parent, typename AVLTree<T>::BinTreeNode*& node_ptr,std::stack<typename AVLTree<T>::BinTreeNode**>& visited_nodes, T value, int& number_of_recursions);

    /*!
      \brief
      function to calculate balance factor of a node

      \param node_ptr
        node to be calculated

      \return
      returns balance factor of the node
    */
    int calculate_bf(typename AVLTree<T>::BinTreeNode* node_ptr);

    /*!
      \brief
      function that is recursively called by remove()

      \param parent
      parent ndoe

      \param node_ptr
      child node

      \param visited_nodes
      reference to stack of visited nodes

      \param number_of_recursions
      number of times function is recursively called

      \return
      none
    */
    void remove_node(typename AVLTree<T>::BinTreeNode*& parent, typename AVLTree<T>::BinTreeNode*& node_ptr,std::stack<typename AVLTree<T>::BinTreeNode**>& visited_nodes, T value);

    /*!
      \brief
      function to rotate right about a node

      \param node_ptr
        node to be rotated

      \return
      none
    */
    void rotate_right(typename AVLTree<T>::BinTreeNode* &node_ptr);

    /*!
      \brief
      function to rotate left about a node

      \param node_ptr
        node to be rotated

      \return
      none
    */
    void rotate_left(typename AVLTree<T>::BinTreeNode*& node_ptr);

};

#include "AVLTree.cpp"

#endif
//---------------------------------------------------------------------------
