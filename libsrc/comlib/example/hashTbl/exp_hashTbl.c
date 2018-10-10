#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#define MAX_NODE_CNT 5
#define BKT_CNT 8

typedef struct ExampleHashNode ExampleHashNode;

struct ExampleHashNode{
	UINT             id;
    ComlibHashNode   hNode;
};

int main()
{
    SINT ret = RC_OK;
    UINT i = 0;
    ComlibHashTbl hashTbl;
    ExampleHashNode *exampleNode = NULL;

    ret = comlib_hashTblInit(&hashTbl, BKT_CNT, RC_FALSE, COM_HASH_TYPE_MURMUR, NULL);
    if(ret != RC_OK){
        printf("hash table init failed(ret=%d)\n",ret);
        return -1;
    }

    { /* insert node */
        for(i=0;i<MAX_NODE_CNT;i++){
            exampleNode = comlib_memMalloc(sizeof(ExampleHashNode));

            exampleNode->id = i;
            exampleNode->hNode.data = exampleNode;

            exampleNode->hNode.key.key = &exampleNode->id;
            exampleNode->hNode.key.keyLen = sizeof(exampleNode->id);

            ret = comlib_hashTblInsertHashNode(&hashTbl, &exampleNode->hNode.key, &exampleNode->hNode);
            if(ret != RC_OK){
                printf("hash node insert failed(ret=%d)\n",ret);
                return -1;
            }
        }
    }

    {/* find node */
        ComlibHashKey hKey;
        ComlibHashNode *hNode = NULL;
        UINT findId = 3;

        hKey.key = &findId;
        hKey.keyLen = sizeof(findId);

        ret = comlib_hashTblFindHashNode(&hashTbl, &hKey, 0, &hNode);
        if(ret != RC_OK){
            printf("node find failed(ret=%d)\n",ret);
            return -1;
        }

        exampleNode = hNode->data;
        printf("FIND ID=%d\n",exampleNode->id);
    }

    {/* delete node */
        ComlibHashKey hKey;
        ComlibHashNode *hNode = NULL;

        for(i=0;i<MAX_NODE_CNT;i++){
            hKey.key = &i;
            hKey.keyLen = sizeof(i);

            ret = comlib_hashTblFindHashNode(&hashTbl, &hKey, 0, &hNode);
            if(ret != RC_OK){
                printf("node find failed(ret=%d)\n",ret);
                return -1;
            }

            ret = comlib_hashTblDelHashNode(&hashTbl, hNode);
            if(ret != RC_OK){
                printf("node delete failed(ret=%d)\n",ret);
                return -1;
            }

            exampleNode = hNode->data;

            printf("DELETE ID = %d\n", exampleNode->id);

            comlib_memFree(exampleNode);
        }
    }

    ret = comlib_hashTblDstry(&hashTbl);
    if(ret != RC_OK){
        printf("hash table destory failed(ret=%d)\n",ret);
        return -1;
    }

    printf("END\n");

    return 0;
}

