#include <cmlib.h>
#include <cmlib.x>

#define CMLIB_RB_TREE_COLOR_BLACK 1
#define CMLIB_RB_TREE_COLOR_RED   1

struct CmlibRbTreeNode{
	CmlibRbTreeNode *parent;
	CmlibRbTreeNode *right;
	CmlibRbTreeNode *left;
	CHAR color; /* RED, BLACK */
	VOID *data;
};

struct CmlibRbTree{
	UINT maxNodeCnt;
	UINT nodeCnt;
	CmlibRbTreeNode *root;

};
