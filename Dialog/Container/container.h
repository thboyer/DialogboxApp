#ifndef __CONTAINER_H
#define __CONTAINER_H

#ifdef __cpluplus
extern "C" {
#endif

typedef void*(*t_ptfV)(void*);
typedef void*(*t_ptfVV)(void*,void*);

typedef struct s_container t_container;

t_container*ContainerNew(t_ptfV pDeleteFunc);
t_container*ContainerDel(t_container*pContainer);
void*ContainerPushback(t_container*pContainer, void*pElem);
void*ContainerParse(t_container*pContainer, t_ptfVV pParseFunc, void*pParam);
void*ContainerParseAll(t_container*pContainer, t_ptfVV pParseFunc, void*pParam);
void*ContainerGetNextElemByCriteria(t_container*pContainer,t_ptfVV pParseFunc, void*pElem);
void*ContainerGetPrevElemByCriteria(t_container*pContainer,t_ptfVV pParseFunc, void*pElem);

#ifdef __cpluplus
}
#endif

#endif