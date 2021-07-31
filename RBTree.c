/**
 * @file RBTree.c
 * @author  Guy Lev <thehatguy>
 * @version 1.0
 * @date 27 may 2020
 *
 * @brief A generic red black tree data structure.
 *
 * @section DESCRIPTION
 * Holds the implementation of an RBTree data structure that can add items, delete them, check for containment and run a
 * func on all of the items it contains, by order.
 */
// ------------------------------ includes ------------------------------
#include "RBTree.h"
#include <stdlib.h>
#include <string.h>
// -------------------------- const definitions -------------------------
#define NO_ITEMS (0)

#define FAILURE (0)
#define SUCCESS (1)


#define LEFT (-1)
#define EQUAL (0)
#define RIGHT (1)
// ------------------------------ functions -----------------------------

/**
 * @brief Connects two nodes.
 * @param parent The node that is connected as the parent.
 * @param child The node that is connected as the child.
 * @param side The side of the parent to connect child to.
 */
void connectNodes(RBTree *tree, Node *parent, Node *child, int side);

/**
 * @param node The node whose children are to be checked
 * @return A pointer to node's child if the other child is NULL, NULL otherwise.
 */
Node *getSingleChild(Node *node);

/**
 * @brief Solves recursively a problem of double black upon deletion.
 * @param tree The tree containing all of the nodes.
 * @param parent The parent of the node where the violation happened.
 * @param child The node that violates the RB rules.
 * @param childSide The side of the child as a child of parent.
 */
void solveDB(RBTree *tree, Node **parentP, Node **childP, int childSide);

/**
 * @brief Frees the data of a node as well as the node itself.
 * @param tree The tree that contains the toFree to be freed.
 * @param toFree The node to be freed.
 */
void freeNode(RBTree *tree, Node *toFree);

/**
 * constructs a new RBTree with the given CompareFunc.
 * comp: a function two compare two variables.
 */
RBTree *newRBTree(CompareFunc compFunc, FreeFunc freeFunc)
{
    RBTree *tree = (RBTree *) malloc(sizeof(RBTree));
    if (tree == NULL)
    {
        return NULL;
    }
    *tree = (RBTree) {.root = NULL, .compFunc = compFunc, .freeFunc = freeFunc, .size = NO_ITEMS};
    return tree;
}

/**
 * @brief Connects node as parent's child
 * @param node The node to insert (as a child)
 * @param parent The node directly above the new node
 * @param side The side of the parent the child connects to.
 */
void connectNode(RBTree *tree, Node *node, Node *parent, int side)
{
    if (node != NULL)
    {
        node->parent = parent;
    }
    if (parent == NULL)
    {
        tree->root = node;
    }
    else if (side == LEFT)
    {
        parent->left = node;
    }
    else
    {
        parent->right = node;
    }
}

/**
 * @param child The node to check whether it's right child or left
 * @return 0 if it has no parent, -1 if it is a left child, 1 if it is a right child
 */
int getSide(Node const *const child)
{
    if (child == NULL)
    {
        return FAILURE;
    }
    Node *parent = child->parent;
    if (parent == NULL)
    {
        return FAILURE;
    }
    if (child == parent->left)
    {
        return LEFT;
    }
    return RIGHT;
}

/**
 * @brief Rotates a sub-tree so that the child will become the parent
 * @param child A left or a right child of the parent.
 * @param parent A node which is the old root of the sub-tree.
 */
void rotate(RBTree *tree, Node *child, Node *parent)
{
    int childSide = getSide(child);
    if (parent->parent == NULL)
    {
        tree->root = child;
        child->parent = NULL;
    }
    else
    {
        int parentSide = getSide(parent);
        child->parent = parent->parent;
        if (parentSide == LEFT)
        {
            parent->parent->left = child;
        }
        else
        {
            parent->parent->right = child;
        }
    }
    if (childSide == LEFT)
    {
        connectNodes(tree, parent, child->right, LEFT);
        connectNodes(tree, child, parent, RIGHT);
    }
    else
    {
        connectNodes(tree, parent, child->left, RIGHT);
        connectNodes(tree, child, parent, LEFT);
    }
}

/**
 * @brief Modifies the RBTree in case the node with the DB violation has a black uncle.
 * @param tree The RBTree containing all of the nodes.
 * @param gParent The grand parent of the node with the DB violation.
 * @param parent The parent of the node with the DB violation.
 * @param node The node with the DB violation.
 * @param parentSide The side of gParent parent is connected to.
 * @return The of node.
 */
Color updateColorBlackUncle(RBTree *tree, Node *gParent, Node *parent, Node *node, int parentSide)
{
        int childSide = getSide(node);
        if (childSide != parentSide)
        {
            rotate(tree, node, parent);
            parent = node;
        }
        rotate(tree, parent, gParent);
        parent->color = BLACK, gParent->color = RED;
        if (node == parent)
        {
            return BLACK;
        }
        return RED;
}

/**
 * @brief Updates the node colors starting from node up
 * @param node The node to start the update from
 * @return The color of node.
 */
Color updateColors(RBTree *tree, Node *node)
{
    Node *parent = node->parent;
    if (parent == NULL)
    {
        return BLACK;
    }
    if (parent->color == BLACK)
    {
        return RED;
    }
    Node *gParent = parent->parent;
    Node *uncle;
    int parentSide = getSide(parent);
    if (parentSide == LEFT)
    {
        uncle = gParent->right;
    }
    else
    {
        uncle = gParent->left;
    }
    if (uncle == NULL || uncle->color == BLACK)
    {
        return updateColorBlackUncle(tree, gParent, parent, node, parentSide);
    }
    parent->color = BLACK, uncle->color = BLACK;
    gParent->color = updateColors(tree, gParent);
    return RED;
}

/**
 * @brief Inserts a new node to a RBtree in the right position.
 * @param tree The tree to insert the node to.
 * @param newNode The node to insert.
 * @return 1 upon success, 0 if there is a node with the same data as newNode's already in tree.
 */
int insertNode(RBTree *tree, Node *newNode)
{
    Node *curNode = tree->root;
    Node *parent;
    int side;
    int compRes;
    while (curNode != NULL)
    {
        compRes = tree->compFunc(newNode->data, curNode->data);
        if (compRes == EQUAL)
        {
            free(newNode);
            return FAILURE;
        }
        parent = curNode;
        if (compRes < EQUAL)
        {
            side = LEFT;
            curNode = curNode->left;
        }
        else
        {
            side = RIGHT;
            curNode = curNode->right;
        }
    }
    connectNode(tree, newNode, parent, side);
    return SUCCESS;
}

/**
 * add an item to the tree
 * @param tree: the tree to add an item to.
 * @param data: item to add to the tree.
 * @return: 0 on failure, other on success. (if the item is already in the tree - failure).
 */
int insertToRBTree(RBTree *tree, void *data)
{
    if (tree == NULL)
    {
        return FAILURE;
    }
    if (data == NULL)
    {
        return FAILURE;
    }
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (newNode == NULL)
    {
        return FAILURE;
    }
    *newNode = (Node) {.parent = NULL, .right = NULL, .left = NULL, .data = data, .color = BLACK};
    if (tree->root == NULL)
    {
        tree->root = newNode;
        tree->size++;
        return SUCCESS;
    }
    if (!insertNode(tree, newNode))
    {
        return FAILURE;
    }
    newNode->color = updateColors(tree, newNode);
    (tree->size)++;
    return SUCCESS;
}

/**
 * @brief Finds the node with the data matching the input
 * @param tree The RBTree to check
 * @param data The data to check a match for
 * @return The node matching data, NULL if not found
 */
Node *findNode(const RBTree *tree, const void *data)
{
    int compRes;
    Node *curNode = tree->root;
    while (curNode != NULL)
    {
        compRes = tree->compFunc(data, curNode->data);
        if (compRes == EQUAL)
        {
            return curNode;
        }
        if (compRes < EQUAL)
        {
            curNode = curNode->left;
        }
        else
        {
            curNode = curNode->right;
        }
    }
    return NULL;
}

/**
 * Gets the node which it's data is the successor of node, if node has right a child
 * @param node The node whose successor needed to be found
 * @return A pointer to the Node pointer of node's successor
 */
Node *findSuccessor(Node *node)
{
    Node *child = node->right;
    while (child != NULL)
    {
        if (child->left != NULL)
        {
            child = child->left;
        }
        else
        {
            break;
        }
    }
    return child;
}

/**
 * @brief Frees the data of a node as well as the node itself.
 * @param tree The tree that contains the toFree to be freed.
 * @param toFree The node to be freed.
 */
void freeNode(RBTree *tree, Node *toFree)
{
    if (toFree == NULL)
    {
        return;
    }
    freeNode(tree, toFree->left);
    freeNode(tree, toFree->right);
    (tree->freeFunc)(toFree->data);
    free(toFree);
}

/**
 * @param node The node whose children are to be checked
 * @return A pointer to node's child if the other child is NULL, NULL otherwise.
 */
Node *getSingleChild(Node *node)
{
    if (node->left == NULL && node->right != NULL)
    {
        return node->right;
    }
    else if (node->left != NULL && node->right == NULL)
    {
        return node->left;
    }
    return NULL;
}

/**
 * @brief Connects two nodes.
 * @param parent The node that is connected as the parent.
 * @param child The node that is connected as the child.
 * @param side The side of the parent to connect child to.
 */
void connectNodes(RBTree *tree, Node *parent, Node *child, int side)
{
    if (parent == NULL)
    {
        tree->root = child;
    }
    else
    {
        if (side == LEFT)
        {
            parent->left = child;
        }
        else
        {
            parent->right = child;
        }
    }
    if (child != NULL)
    {
        child->parent = parent;
    }
}

/**
 * @brief Switches the place and color of two nodes.
 * @param highNode The node which is higher in the tree.
 * @param lowNode The node which is lower in the tree.
 */
void switchNodes(RBTree *tree, Node *highNode, Node *lowNode)
{
    Node *highParent = highNode->parent;
    Node *highRight = highNode->right;
    Node *highLeft = highNode->left;
    Color highCol = highNode->color;
    int highSide = getSide(highNode);
    int lowSide = getSide(lowNode);
    connectNodes(tree, highNode, lowNode->left, LEFT);
    connectNodes(tree, highNode, lowNode->right, RIGHT);
    if (lowNode->parent != highNode)
    {
        connectNodes(tree, lowNode->parent, highNode, lowSide);
        connectNodes(tree, lowNode, highRight, RIGHT);
        connectNodes(tree, lowNode, highLeft, LEFT);
    }
    else
    {
        connectNodes(tree, lowNode, highNode, lowSide);
        if (lowSide == LEFT)
        {
            connectNodes(tree, lowNode, highRight, RIGHT);
        }
        else
        {
            connectNodes(tree, lowNode, highLeft, LEFT);
        }
    }
    connectNodes(tree, highParent, lowNode, highSide);
    highNode->color = lowNode->color;
    lowNode->color = highCol;
}

/**
 * @brief Solves DB in case the violating node has a black sibling and its close nephew is red.
 * @param tree The RBTree containing all of the nodes.
 * @param siblingP A pointer to a pointer to the sibling of the violating node.
 * @param childSide The side of parent the violating node is connected to.
 */
void closeRedNephewDB(RBTree *tree, Node **siblingP, int childSide)
{
    if (childSide == LEFT && (*siblingP)->left != NULL && (*siblingP)->left->color == RED)
    {
        (*siblingP)->left->color = BLACK;
        (*siblingP)->color = RED;
        rotate(tree, (*siblingP)->left, (*siblingP));
    }
    else if (childSide == RIGHT && (*siblingP)->right != NULL && (*siblingP)->right->color == RED)
    {
        (*siblingP)->right->color = BLACK;
        (*siblingP)->color = RED;
        rotate(tree, (*siblingP)->right, (*siblingP));
    }
}

/**
 * @brief Solves DB in case the violating node has a black sibling and its far nephew is red.
 * @param tree The RBTree containing all of the nodes.
 * @param parentP A pointer to a pointer to the parent of the violating node.
 * @param siblingP A pointer to a pointer to the sibling of the violating node.
 * @param childSide The side of parent the violating node is connected to.
 */
void farRedNephewDB(RBTree *tree, Node **parentP, Node **siblingP, int childSide)
{
    if ((childSide == LEFT && (*siblingP)->right != NULL && (*siblingP)->right->color == RED) ||
        (childSide == RIGHT && (*siblingP)->left != NULL && (*siblingP)->left->color == RED))
    {
        Color temp = (*siblingP)->color;
        (*siblingP)->color = (*parentP)->color;
        (*parentP)->color = temp;
        if ((*siblingP)->right != NULL && (*siblingP)->right->color == RED)
        {
            (*siblingP)->right->color = BLACK;
        }
        else
        {
            (*siblingP)->left->color = BLACK;
        }
        rotate(tree, *siblingP, *parentP);
    }
}

/**
 * @brief Solves DB in case the violating node has a black sibling.
 * @param tree The RBTree containing all of the nodes.
 * @param parentP A pointer to a pointer to the parent of the violating node.
 * @param siblingP A pointer to a pointer to the sibling of the violating node.
 * @param childP A pointer to a pointer to the violating node.
 * @param childSide The side of parent the violating node is connected to.
 */
void blackSiblingDB(RBTree *tree, Node **parentP, Node **siblingP, Node **childP, int childSide)
{
    if ((*siblingP)->left == NULL || (*siblingP)->left->color == BLACK)
    {
        if ((*siblingP)->right == NULL || (*siblingP)->right->color == BLACK)
        {
            (*siblingP)->color = RED;
            if ((*parentP)->color == RED)
            {
                (*parentP)->color = BLACK;
            }
            else
            {
                solveDB(tree, &((*parentP)->parent), parentP, getSide((*parentP)));
                return;
            }
        }
    }
    closeRedNephewDB(tree, siblingP, childSide);
    *siblingP = (*parentP)->left == (*childP) ? (*parentP)->right : (*parentP)->left;
    farRedNephewDB(tree, parentP, siblingP, childSide);
}

/**
 * @brief Solves recursively a problem of double black upon deletion.
 * @param tree The tree containing all of the nodes.
 * @param parent The parent of the node where the violation happened.
 * @param child The node that violates the RB rules.
 * @param childSide The side of the child as a child of parent.
 */
void solveDB(RBTree *tree, Node **parentP, Node **childP, int childSide)
{
    if (*parentP == NULL)
    {
        return;
    }
    Node **siblingP = (*parentP)->left == (*childP) ? &((*parentP)->right) : &((*parentP)->left);
    if ((*siblingP)->color == RED)
    {
        (*siblingP)->color = BLACK, (*parentP)->color = RED;
        rotate(tree, *siblingP, *parentP);
        solveDB(tree, parentP, childP, childSide);
    }
    else
    {
        blackSiblingDB(tree, parentP, siblingP, childP, childSide);
    }
}

/**
 * @brief Deals with the first stage of deletion - switches the node to be deleted with it's succesor if needed.
 * @param tree The RBTree
 * @param toSwitch The node to be switched
 * @param childPtr A pointer to the place to hold toSwitch's child
 * @return 0 upon failure, 1 otherwise.
 */
int placeBeforeDeletion(RBTree *tree, Node *toSwitch, Node **childPtr)
{
    if (toSwitch == NULL)
    {
        return FAILURE;
    }
    *childPtr = getSingleChild(toSwitch);
    if (*childPtr != NULL)
    {
        switchNodes(tree, toSwitch, *childPtr);
    }
    else if (toSwitch->right != NULL && toSwitch->left != NULL)
    {
        Node *suc = findSuccessor(toSwitch);
        switchNodes(tree, toSwitch, suc);
    }
    return SUCCESS;
}

/**
 * @brief Disconnects the node to be deleted and balances the tree.
 * @param tree: the tree to remove an item from.
 * @param parentP A pointer to a pointer to the parent of the violating node.
 * @param toDeleteP A pointer to a pointer the node to be deleted.
 * @param childP A pointer to a pointer to the child of the node to be deleted.
 * @param toDeleteSide The side of the toDelete as a child of parent.
 */
void balanceTree(RBTree *tree, Node **parentP, Node **toDeleteP, Node **childP, int toDeleteSide)
{
    if ((*toDeleteP)->color == RED)
    {
        connectNode(tree, NULL, *parentP, toDeleteSide);
    }
    else if ((*toDeleteP)->color == BLACK)
    {
        *childP = getSingleChild((*toDeleteP));
        connectNode(tree, *childP, *parentP, toDeleteSide);
        if (*childP != NULL && (*childP)->color == RED)
        {
            (*childP)->color = BLACK;
        }
        else
        {
            solveDB(tree, parentP, childP, toDeleteSide);
        }
    }
}

/**
 * remove an item from the tree
 * @param tree: the tree to remove an item from.
 * @param data: item to remove from the tree.
 * @return: 0 on failure, other on success. (if data is not in the tree - failure).
 */
int deleteFromRBTree(RBTree *tree, void *data)
{
    if (tree == NULL)
    {
        return FAILURE;
    }
    Node *toDelete = findNode(tree, data);
    Node *child = NULL;
    if (!placeBeforeDeletion(tree, toDelete, &child))
    {
        return FAILURE;
    }
    Node *parent = toDelete->parent;
    int toDeleteSide = getSide(toDelete);
    balanceTree(tree, &parent, &toDelete, &child, toDeleteSide);
    toDelete->right = NULL, toDelete->left = NULL;
    freeNode(tree, toDelete);
    (tree->size)--;
    return SUCCESS;
}


/**
 * check whether the tree RBTreeContains this item.
 * @param tree: the tree to add an item to.
 * @param data: item to check.
 * @return: 0 if the item is not in the tree, other if it is.
 */
int RBTreeContains(const RBTree *tree, const void *data)
{
    if (tree == NULL)
    {
        return FAILURE;
    }
    return (findNode(tree, data) != NULL);
}

/**
 * Activate a function on each item of the sub-tree whose root is node. The order is an ascending order. if one of the activations of the
 * function returns 0, the process stops.
 * @param node The root of the sub-tree to check.
 * @param func: the function to activate on all items.
 * @param args: more optional arguments to the function (may be null if the given function support it).
 * @return: 0 on failure, 1 on success.
 */
int forEachNode(const Node *node, forEachFunc func, void *args)
{
    if (node == NULL)
    {
        return SUCCESS;
    }
    if (forEachNode(node->left, func, args) == FAILURE)
    {
        return FAILURE;
    }
    if (func(node->data, args) == FAILURE)
    {
        return FAILURE;
    }
    if (forEachNode(node->right, func, args) == FAILURE)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * Activate a function on each item of the tree. the order is an ascending order. if one of the activations of the
 * function returns 0, the process stops.
 * @param tree: the tree with all the items.
 * @param func: the function to activate on all items.
 * @param args: more optional arguments to the function (may be null if the given function support it).
 * @return: 0 on failure, other on success.
 */
int forEachRBTree(const RBTree *tree, forEachFunc func, void *args)
{
    if (tree == NULL)
    {
        return FAILURE;
    }
    if (tree->root == NULL)
    {
        return SUCCESS;
    }
    return forEachNode(tree->root, func, args);
}

/**
 * free all memory of the data structure.
 * @param tree: pointer to the tree to free.
 */
void freeRBTree(RBTree **tree)
{
    Node *root = (*tree)->root;
    freeNode(*tree, root);
    free(*tree);
    *tree = NULL;
}
