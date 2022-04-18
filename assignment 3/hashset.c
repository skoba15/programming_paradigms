#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
  assert(elemSize>0 && numBuckets>0 && hashfn!=NULL && comparefn!=NULL);
  h->elemsize=elemSize;
  h->numbuckets=numBuckets;
  h->cmpfn=comparefn;
  h->freefn=freefn;
  h->hashfn=hashfn;
  h->elems=malloc(numBuckets*sizeof(vector));
  h->elemnumber=0;
  int i;
  for(i=0; i<h->numbuckets; ++i){
    vector * vp=(vector *)((char*)h->elems+i*sizeof(vector));
    VectorNew(vp, h->elemsize, h->freefn, 0);
  }
}

void HashSetDispose(hashset *h)
{
  if(h->freefn!=NULL){
    int i;
    for(i=0; i<h->numbuckets; ++i){
      vector * vp=(vector *)((char *)h->elems+i*sizeof(vector));
      VectorDispose(vp);
    }
  }
  free(h->elems);
}

int HashSetCount(const hashset *h)
{
  return h->elemnumber;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  assert(mapfn!=NULL);
  int i;
  for(i=0; i<h->numbuckets; ++i){
    vector * vp=(vector *)((char *)h->elems+i*sizeof(vector));
    VectorMap(vp, mapfn, auxData);
  }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  assert(elemAddr!=NULL);
 int index= h->hashfn(elemAddr, h->numbuckets);
 assert(index>=0 && index<h->numbuckets);
 vector * vp=(vector *)((char *)h->elems+index*sizeof(vector));
 int res=VectorSearch(vp, elemAddr, h->cmpfn, 0, false);
 if(res==-1)VectorAppend(vp, elemAddr);
 else{
   VectorReplace(vp, elemAddr, res);
 }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
  assert(elemAddr!=NULL);
  int index=h->hashfn(elemAddr, h->numbuckets);
  assert(index>=0 && index<h->numbuckets);
  vector * vp=(vector *)((char *)h->elems+index*sizeof(vector));
  int res=VectorSearch(vp, elemAddr, h->cmpfn, 0, false);
  if(res==-1)return NULL;
  else{
    void * elem=(char *)vp->elems+res*sizeof(h->elemsize);
    return elem;
  }
  return NULL;
}
