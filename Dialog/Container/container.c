#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "container.h"

typedef struct s_node{
    struct s_node*m_pPrev;
    struct s_node*m_pNext;
    void*         m_pElem;
}t_node;

t_node*NodeNew(t_node*pPrev, t_node*pNext, void*pElem){
    t_node*pNode=(t_node*)malloc(sizeof(t_node));
    assert(pNode);
    *pNode=(t_node){
        .m_pPrev = pPrev,
        .m_pNext = pNext,
        .m_pElem = pElem        
    };
    if(pPrev) pPrev->m_pNext=pNode;
    if(pNext) pNext->m_pPrev=pNode;
    return pNode;
}

t_node*NodeDelReturnNext(t_node*pNode, t_ptfV pDeleteFunc){
    assert(pNode);
    if(pDeleteFunc) pDeleteFunc(pNode->m_pElem);
    else free(pNode->m_pElem);
    t_node*pNext=pNode->m_pNext;
    if(pNode->m_pPrev) pNode->m_pPrev->m_pNext=pNode->m_pNext;
    if(pNode->m_pNext) pNode->m_pNext->m_pPrev=pNode->m_pPrev;
    free(pNode);
    return pNext;
}

struct s_container {
    t_node  *m_pHead;
    t_node  *m_pTail;
    t_ptfV  m_pDeleteFunc;
    size_t   m_szCard;
};

t_container*ContainerNew(t_ptfV pDeleteFunc){
    t_container*pContainer=(t_container*)malloc(sizeof(t_container));
    assert(pContainer);
    *pContainer=(t_container){
        .m_pDeleteFunc = pDeleteFunc,
    };
    return pContainer;
}

t_container*ContainerDel(t_container*pContainer){
    assert(pContainer);
    while(pContainer->m_pHead){
        pContainer->m_pHead=NodeDelReturnNext(pContainer->m_pHead, pContainer->m_pDeleteFunc);
        pContainer->m_szCard--;
    }
    assert(pContainer->m_szCard==0);
    free(pContainer);
    return NULL;
}

void*ContainerPushback(t_container*pContainer, void*pElem){
    assert(pContainer);
    pContainer->m_pTail=NodeNew(pContainer->m_pTail, NULL, pElem);
    if(!pContainer->m_pHead) pContainer->m_pHead=pContainer->m_pTail;
    pContainer->m_szCard++;
    assert(pContainer->m_pTail->m_pElem==pElem);
    return pContainer->m_pTail->m_pElem;
}

void*ContainerParse(t_container*pContainer, t_ptfVV pParseFunc, void*pParam){
    assert(pContainer);
    assert(pParseFunc);
    t_node*pParse=pContainer->m_pHead;
    while(pParse){
        if(pParseFunc(pParse->m_pElem, pParam)) return pParse->m_pElem;
        pParse=pParse->m_pNext;
    }
    return NULL;
}

void*ContainerParseAll(t_container*pContainer, t_ptfVV pParseFunc, void*pParam){
    assert(pContainer);
    assert(pParseFunc);
    t_node*pParse=pContainer->m_pHead;
    while(pParse){
        pParseFunc(pParse->m_pElem, pParam);
        pParse=pParse->m_pNext;
    }
    return NULL;
}

void*ContainerGetNextElemByCriteria(t_container*pContainer,t_ptfVV pParseFunc, void*pElem){
    assert(pContainer);
    assert(pParseFunc);
    assert(pElem);
    
    t_node*pParse=pContainer->m_pHead;
    while(pParse && pParse->m_pElem!=pElem) pParse=pParse->m_pNext;
    if(pParse==NULL) return NULL;
    pParse=pParse->m_pNext;
    if(pParse==NULL) pParse=pContainer->m_pHead;
    while(pParse->m_pElem!=pElem){
        if(pParseFunc(pParse->m_pElem, pElem)) return pParse->m_pElem;
        pParse=pParse->m_pNext;
        if(pParse==NULL) pParse=pContainer->m_pHead;
    }
    return pElem;
}

void*ContainerGetPrevElemByCriteria(t_container*pContainer,t_ptfVV pParseFunc, void*pElem){
    assert(pContainer);
    assert(pParseFunc);
    assert(pElem);
    
    t_node*pParse=pContainer->m_pTail;
    while(pParse && pParse->m_pElem!=pElem) pParse=pParse->m_pPrev;
    if(pParse==NULL) return NULL;
    pParse=pParse->m_pPrev;
    if(pParse==NULL) pParse=pContainer->m_pTail;
    while(pParse->m_pElem!=pElem){
        if(pParseFunc(pParse->m_pElem, pElem)) return pParse->m_pElem;
        pParse=pParse->m_pPrev;
        if(pParse==NULL) pParse=pContainer->m_pTail;
    }
    return pElem;
}