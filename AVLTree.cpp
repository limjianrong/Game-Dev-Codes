/*!*****************************************************************************
\file AVLTree.cpp
\author Lim Jian Rong (jianrong.lim@digipen.edu / 2201501@sit.singaporetech.edu.sg)
\par Course: CSD2183 Section B
\par Assignment: 3
\date 01/03/2024
\brief This file has definitions for for a AVL class template which inherits from
       a BSTree class template. It provides functionality unique to AVLTrees, such
       as automatic balancing of the tree after a insertion/deletion.
*******************************************************************************/

    /*!
      \brief
      Constructor for AVLTree object

      \param oa
        pointer to object allocator

      \param shareOA
        boolean to determine if other AVL will share OA with this AVL
    */
    template<typename T>
    AVLTree<T>::AVLTree(ObjectAllocator* oa, bool shareOA):BSTree<T>(oa, shareOA) {

    }

    /*!
      \brief
      function to calculate balance factor of a node

      \param node_ptr
        node to be calculated

      \return
      returns balance factor of the node
    */
    template<typename T>
    int AVLTree<T>::calculate_bf(typename AVLTree<T>::BinTreeNode* node_ptr){

            //there is left child and right child
            if(node_ptr->left && node_ptr->right){
                node_ptr->balance_factor = BSTree<T>::tree_height(node_ptr->left) - BSTree<T>::tree_height(node_ptr->right);
            }

            //only have left child
            else if(node_ptr->left && !node_ptr->right){
                node_ptr->balance_factor = BSTree<T>::tree_height(node_ptr->left);
            }

            //only have right child
            else if(!node_ptr->left && node_ptr->right){
                node_ptr->balance_factor = -BSTree<T>::tree_height(node_ptr->right);
            }

            //no child, bf = 0
            else{
                node_ptr->balance_factor = 0;
            }

            //return bf of node
            return node_ptr->balance_factor;
    }

    /*!
      \brief
      function to rotate right about a node

      \param node_ptr
        node to be rotated

      \return
      none
    */
    template<typename T>
    void AVLTree<T>::rotate_right(typename AVLTree<T>::BinTreeNode*& node_ptr){
        typename AVLTree<T>::BinTreeNode* temp = node_ptr;

        //shift node to be equal to its left child
        node_ptr = node_ptr->left;

        //original node's left child is now the shifted node's right child
        temp->left = node_ptr->right;

        //update count
        temp->count -= node_ptr->count;
        if(node_ptr->right){
            temp->count += node_ptr->right->count;
            node_ptr->count -= node_ptr->right->count;
        }

        node_ptr->right = temp;
        node_ptr->count += (temp->count);
    }

    /*!
      \brief
      function to rotate left about a node

      \param node_ptr
        node to be rotated

      \return
      none
    */
    template<typename T>
    void AVLTree<T>::rotate_left(typename AVLTree<T>::BinTreeNode*& node_ptr){
        typename AVLTree<T>::BinTreeNode* temp = node_ptr;

        //shift node to be equal to its right child
        node_ptr = node_ptr->right;

        //original node's right child is now the shifted node's left child
        temp->right = node_ptr->left;

        //update count values 
        temp->count -= node_ptr->count;
        if(node_ptr->left){
            temp->count += node_ptr->left->count;
            node_ptr->count -= node_ptr->left->count;
        }

        node_ptr->left = temp;
        node_ptr->count += (temp->count);
    }


    /*!
      \brief
      function to insert a node into AVLTree

      \param value
        value to be inserted

      \return
      none
    */
    template<typename T>
    void AVLTree<T>::insert(const T& value){
        try{

            //keep track of the visited nodes when we are inserting
            std::stack<typename AVLTree<T>::BinTreeNode**> visited_nodes;
            int number_of_recursions = 0;
            typename BSTree<T>::BinTreeNode*& root_node = BSTree<T>::get_root();

            //root is nullptr, create root node
            if(root_node == nullptr){
                root_node = BSTree<T>::make_node(value);
                root_node->count = 1;
            }
            
            //recursively call
            else if(value < root_node->data){
                insert_node(root_node, root_node->left, visited_nodes, value, number_of_recursions);
            }

            //recursively call
            else if(value > root_node->data){
                insert_node(root_node, root_node->right, visited_nodes, value, number_of_recursions);
            }
            

            //insert_node has successfully inserted a node, we go through the stack of visited nodes
            //and calculate each node's new balance factor
            while(visited_nodes.size()){
                typename AVLTree<T>::BinTreeNode**& top = visited_nodes.top();
                calculate_bf(*top);


                //bf >= -1 and bf <= 1, no need to do any balancing
                if((*top)->balance_factor >= -1 && (*top)->balance_factor <= 1){
                    visited_nodes.pop();
                    continue;
                }

                //balancing needed to be done
                else{

                    if((*top)->balance_factor>1){
                        
                        //case 1: left-left, right rotation
                        if(calculate_bf((*top)->left)>=0){
                            rotate_right((*top));
                        }

                        //case 2: left-right, left-right rotation
                        else{
                            rotate_left((*top)->left);
                            rotate_right((*top));
                        }
                        
                    }

                    else if((*top)->balance_factor< -1){

                        //case 3: right-right, left rotation
                        if(calculate_bf((*top)->right) <= 0){
                            rotate_left((*top));
                        }

                        //case 4: right-left, right-left rotation
                        else{
                            rotate_right((*top)->right);
                            rotate_left((*top));
                        }
                    }
                    
                    //once a node is rebalanced, we do not need to check subsequent visited nodes
                    break;
                }

            }

        } catch (const std::exception& e){
            throw(BSTException(OAException::OA_EXCEPTION::E_NO_MEMORY, e.what()));
        }
    }


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
    template<typename T>
    void AVLTree<T>::insert_node(typename AVLTree<T>::BinTreeNode*& parent, typename AVLTree<T>::BinTreeNode*& node_ptr,std::stack<typename AVLTree<T>::BinTreeNode**>& visited_nodes, T value, int& number_of_recursions){
            ++number_of_recursions;
            visited_nodes.push(&parent);
        
            //create node here
            if(node_ptr == nullptr){
                
                node_ptr = BSTree<T>::make_node(value);
                node_ptr->left = nullptr;
                node_ptr->right = nullptr;
                if(value < parent->data){
                    parent->left = node_ptr;
                }
                else if(value > parent->data){
                    parent->right = node_ptr;
                }

                node_ptr->count = 1;
                BSTree<T>::get_root()->count += 1;
            }

            //havent reached the spot to insert node at
            else if(value < node_ptr->data){
                node_ptr->count += 1;
                insert_node(node_ptr, node_ptr->left,visited_nodes, value, number_of_recursions);
            }

            //havent reached the spot to insert node at
            else if(value > node_ptr->data){
                node_ptr->count += 1;
                insert_node(node_ptr, node_ptr->right,visited_nodes, value, number_of_recursions);
            }
            else{
            }
    }


    /*!
      \brief
      function to remove a node

      \param value
      value to be removed from AVLTree

      \return
      none
    */
    template<typename T>
    void AVLTree<T>::remove(const T& value){

            //similarly to insert, we also keep track of traversed nodes
            std::stack<typename AVLTree<T>::BinTreeNode**> visited_nodes;
            
            typename BSTree<T>::BinTreeNode*& root_node = BSTree<T>::get_root();

            if(!root_node){
                return;
            }

            //found match right at the start
            if(value == root_node->data){
                BSTree<T>::free_node(root_node);
                BSTree<T>::get_root() = nullptr;

            }

            //recursively call
            else if(value < root_node->data){
                remove_node(root_node, root_node->left, visited_nodes, value);
            }

            //recursively call
            else if(value > root_node->data){
                remove_node(root_node, root_node->right, visited_nodes, value);
            }


            //if code reaches here, node to be deleted has been found
            while(visited_nodes.size()){
                typename AVLTree<T>::BinTreeNode**& top = visited_nodes.top();
                calculate_bf(*top);
                if((*top)->balance_factor >= -1 && (*top)->balance_factor <= 1){
                    visited_nodes.pop();
                    continue;
                }
                else{

                    if((*top)->balance_factor>1){
                        
                        //case 1: left-left, right rotation
                        if(calculate_bf((*top)->left)>=0){
                            rotate_right((*top));
                        }

                        //case 2: left-right, left-right rotation
                        else{
                            rotate_left((*top)->left);
                            rotate_right((*top));
                        }
                        
                    }

                    else if((*top)->balance_factor< -1){

                        //case 3: right-right, left rotation
                        if(calculate_bf((*top)->right) <= 0){
                            rotate_left((*top));
                        }

                        //case 4: right-left, right-left rotation
                        else{
                            rotate_right((*top)->right);
                            rotate_left((*top));
                        }
                    }   
                    visited_nodes.pop();
                }
            }
    }

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
    template<typename T>
    void AVLTree<T>::remove_node(typename AVLTree<T>::BinTreeNode*& parent, typename AVLTree<T>::BinTreeNode*& node_ptr,std::stack<typename AVLTree<T>::BinTreeNode**>& visited_nodes, T value) {

        visited_nodes.push(&parent);
        if(node_ptr == nullptr){
        //  std::cout <<"value " <<value <<" not found\n";
        }

        else if(node_ptr->data == value){
                //Case 1: Node to be deleted has 2 children, find node's predecessor
                if(node_ptr->left && node_ptr->right){
                    typename AVLTree<T>::BinTreeNode* root_node = BSTree<T>::root();
                    typename AVLTree<T>::BinTreeNode* predecessor = root_node->left;
                    typename AVLTree<T>::BinTreeNode* parent_of_predecessor=root_node;

                    while(predecessor->right != nullptr){
                        predecessor->count -= 1;
                        parent_of_predecessor = predecessor;
                        predecessor = predecessor->right;
                    }

                    

                    //predecessor has been found, replace node to be deleted value
                    node_ptr->data = predecessor->data;

                    //now, we need to delete the node that has the predecessor
                    //this node is guaranteed to either be leaf node, or node with empty right child
                        visited_nodes.push(&node_ptr);
                    //if node is leaf node, simply delete
                    if(predecessor->left ==nullptr && predecessor->right == nullptr){
                        if(parent_of_predecessor!=root_node){
                            if(predecessor->data < parent_of_predecessor->data){
                            parent_of_predecessor->left = nullptr;
                            }
                        
                            else{
                                parent_of_predecessor->right = nullptr;
                            }
                        }

                        if(node_ptr->right){
                            if(node_ptr->data > node_ptr->right->data){
                                node_ptr->right->left = node_ptr->left;
                                if(node_ptr->left){
                                    node_ptr->right->count += (node_ptr->left->count);
                                }
                                node_ptr->left = node_ptr->right;
                                node_ptr->right = nullptr;
                                
                            }
                        }

                        //free node
                        BSTree<T>::free_node(predecessor);

                        predecessor= nullptr;
                    }

                    //else if node has left child
                    else if(predecessor->left !=nullptr){
                        if(parent_of_predecessor!=root_node){
                            parent_of_predecessor->right = predecessor->left;
                        }

                        //if parent of predecessor is root node itself, it means we did not
                        //even enter the while loop, which means that the parent
                        //must be the root node itself, it also means that the predecessor
                        //would be the immediate left child of root node, and the next highest
                        //node would be the left child of the immediate left child 
                        else{
                            root_node->left = predecessor->left;
                        }
                        BSTree<T>::free_node(predecessor);
                        predecessor= nullptr;

                    }
                    root_node->count -= 1;
                }

                //node to be removed has left child and no right child
                else if(node_ptr->left && !node_ptr->right){

                    typename AVLTree<T>::BinTreeNode* tmp3 = BSTree<T>::root();
                    typename AVLTree<T>::BinTreeNode* parent_node3 =BSTree<T>::root();

                    //decrement count value of all parents of the node to be deleted
                    while(tmp3 != nullptr && value != tmp3->data){
                        if(value < tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->left;
                        }
                        else if(value > tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->right;
                        }
                        parent_node3->count -= 1;

                    }


                    typename AVLTree<T>::BinTreeNode* tmp2 = node_ptr;
                    if(node_ptr ==BSTree<T>::get_root()){
                        BSTree<T>::get_root() = node_ptr->left;
                    }
                    node_ptr = node_ptr->left;
                    BSTree<T>::free_node(tmp2);
                    if( node_ptr->data < parent->data){
                        parent->left = node_ptr;
                    }
                    else if(node_ptr->data > parent->data){
                        parent->right = node_ptr;
                    }

                    tmp2= nullptr;
                }

                //node to be removed has right child and no left child
                else if(node_ptr->right && !node_ptr->left){

                    typename AVLTree<T>::BinTreeNode* tmp3 = BSTree<T>::root();
                    typename AVLTree<T>::BinTreeNode* parent_node3 =BSTree<T>::root();

                    //decrement count value of all parents of the node to be deleted
                    while(tmp3 != nullptr && value != tmp3->data){
                        if(value < tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->left;
                        }
                        else if(value > tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->right;
                        }
                        parent_node3->count -= 1;
                    }

                    typename AVLTree<T>::BinTreeNode* tmp2 = node_ptr;
                    if(node_ptr == BSTree<T>::get_root()){
                        BSTree<T>::get_root() = node_ptr->right;
                    }
                    node_ptr = node_ptr->right;
                    BSTree<T>::free_node(tmp2);
                    if( node_ptr->data < parent->data){
                        parent->left = node_ptr;
                    }
                    else if(node_ptr->data > parent->data){
                        parent->right = node_ptr;
                    }

                    tmp2= nullptr;
                }

                //node to be removed is a leaf node
                else if(!node_ptr->left && !node_ptr->right){
                        //            std::cout <<"no child\n";
                    typename AVLTree<T>::BinTreeNode* tmp3 = BSTree<T>::root();
                    typename AVLTree<T>::BinTreeNode* parent_node3 =BSTree<T>::root();

                    //decrement count value of all parents of the node to be deleted
                    while(tmp3 != nullptr && value != tmp3->data){
                        if(value < tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->left;
                        }
                        else if(value > tmp3->data){
                            parent_node3 = tmp3;
                            tmp3 = tmp3->right;
                        }
                        parent_node3->count -= 1;

                    }

                    if( node_ptr->data < parent->data){
                        parent->left = nullptr;
                    }
                    else if(node_ptr->data > parent->data){
                        parent->right = nullptr;
                    }

                    BSTree<T>::free_node(node_ptr);

                    if(node_ptr == BSTree<T>::get_root()){
                        BSTree<T>::get_root() = nullptr;
                    }

                    node_ptr= nullptr;
                }
                else{
                }
        }

        //havent found value yet
        else if(value < node_ptr->data){
            remove_node(node_ptr, node_ptr->left, visited_nodes, value);
        }

        //havent found value yet
        else if(value > node_ptr->data){
            remove_node(node_ptr, node_ptr->right, visited_nodes, value);
        }
        else {
        }
    }


    /*!
      \brief
      function that returns true if efficiency is implemented

      \return
      returns true
    */
    template<typename T>
    bool AVLTree<T>::ImplementedBalanceFactor(){
        return true;
    }