#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  assert(elemSize>=0);
  v->elemsize=elemSize;
  assert(initialAllocation>=0);
  if(initialAllocation==0)v->alloclength=4;
  else v->alloclength=initialAllocation;
  v->free=freeFn;
  v->logicallength=0;
  v->grow=v->alloclength;
  v->elems=malloc(v->alloclength*v->elemsize);
  assert(v->elems!=NULL);
}

void VectorDispose(vector *v)
{
  if(v->free!=NULL){
   int i;
   for(i=0; i<v->logicallength; ++i){
     void * target=(char *)v->elems+v->elemsize*i;
     v->free(target);
    }
  }
  free(v->elems);
}

int VectorLength(const vector *v)
{
  return v->logicallength;
}

void *VectorNth(const vector *v, int position)
{
  assert(position>=0 && position<=v->logicallength-1);
  void * target=(char *)v->elems+position*v->elemsize;
  return target;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  assert(position>=0 && position<=v->logicallength-1);
  void * target=(char *)v->elems+v->elemsize*position;
  if(v->free!=NULL)v->free(target);
  memcpy(target, elemAddr, v->elemsize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position>=0 && position<=v->logicallength);
  if(v->alloclength==v->logicallength){
    v->alloclength+=v->grow;
    v->elems=realloc(v->elems, v->alloclength*v->elemsize);
    assert(v->elems!=NULL);
  }
  if(position==v->logicallength){
    void * target=(char *)v->elems+v->elemsize*position;
    memcpy(target, elemAddr, v->elemsize);
  }
  else{
    void * target=(char *)v->elems+v->elemsize*position;
    memmove((char *)target+v->elemsize, target, v->elemsize*(v->logicallength-position));
    memcpy(target, elemAddr, v->elemsize);
  }
  v->logicallength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
  if(v->alloclength==v->logicallength){
    v->alloclength+=v->grow;
    v->elems=realloc(v->elems, v->alloclength*v->elemsize);
    assert(v->elems!=NULL);
  }
  void * target=(char *)v->elems+v->logicallength*v->elemsize;
  memcpy(target, elemAddr, v->elemsize);
  v->logicallength++;
}

void VectorDelete(vector *v, int position)
{
  assert(position>=0 && position<=v->logicallength-1);
  void * target=(char *)v->elems+position*v->elemsize;
  if(v->free!=NULL)v->free(target);
  if(position!=v->logicallength-1){
    memmove(target, (char *)target+v->elemsize, v->elemsize*(v->logicallength-position-1));
  }
  v->logicallength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  qsort(v->elems, v->logicallength, v->elemsize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  assert(mapFn!=NULL);
  int i;
  for(i=0; i<v->logicallength; ++i){
    void * elem=(char *)v->elems+v->elemsize*i;
    mapFn(elem, auxData);
  }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
  assert(startIndex>=0 && startIndex<=v->logicallength);
  assert(key!=NULL && searchFn!=NULL);
  if(isSorted){
    void * start=(char *)v->elems+v->elemsize*startIndex;
    void * result= bsearch(key, start, v->logicallength-startIndex, v->elemsize, searchFn);
    if(result==NULL)return -1;
    else{
      int index=((char *)result-(char *)v)/v->elemsize;
      return index;
    }
  }
    else{
      int i;
      for(i=0; i<v->logicallength; ++i){
	void * elem=(char *)v->elems+v->elemsize*i;
	if(searchFn(key, elem)==0)return i;
      }
    }
  return -1;
} 
