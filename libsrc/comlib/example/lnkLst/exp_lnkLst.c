#include <stdio.h> 

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#define MAX_NODE_CNT 3

typedef struct ExampleNode ExampleNode;
typedef struct Example Example;

struct ExampleNode{
    UINT id;
    ComlibLnkNode lnkNode;
};

struct Example{
    ComlibLnkLst exampleLl;
};

Example g_exp;

int main()
{
    SINT ret = RC_OK;
    UINT i = 0;
    ComlibLnkNode *lnkNode = NULL;
    ExampleNode *node = NULL;

    ret = comlib_lnkLstInit(&g_exp.exampleLl, ~0);
    if(ret != RC_OK){
        printf("linked list init failed(ret=%d)\n",ret);
        return -1;
    }

    { /* Insert test */
        for(i=0;i<MAX_NODE_CNT;i++){
            node = comlib_memMalloc(sizeof(ExampleNode));

            node->id = i;
            node->lnkNode.data = node;

            ret = comlib_lnkLstInsertTail(&g_exp.exampleLl, &node->lnkNode);
            if(ret != RC_OK){
                printf("linked list insert failed(ret=%d)\n",ret);
                return -1;
            }
        }

        printf("NODE CNT : %d\n",g_exp.exampleLl.nodeCnt);
    }

    {/* display node */
        COM_GET_LNKLST_FIRST(&g_exp.exampleLl, lnkNode);
        if(lnkNode != NULL){ /* node exist */
            while(1){
                node = lnkNode->data;

                printf("SAVE NODE_ID = %d\n", node->id);

                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){ /* end of linked list */
                    break;
                }
            }
        }
        else {
            printf("first node not exist\n");
            return -1;
        }
    }

    {/* delete all node */
        while(1){
            lnkNode = comlib_lnkLstGetFirst(&g_exp.exampleLl);
            if(lnkNode == NULL){
                printf("node not exist\n");
                break;
            }

            node = lnkNode->data;

            printf("DELETE NODE_ID=%d\n",node->id);

            comlib_memFree(node);
        }
    }

    return 0;
}

