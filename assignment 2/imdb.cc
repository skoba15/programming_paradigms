using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "imdb.h"
#include <stdlib.h>
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct actor{
    char *c;
    void * info;
  };

  struct mov{
    film movi;
    void * info;
  };


imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
  
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) ||
	    (movieInfo.fd == -1) );
}

int cmp1(const void * a, const void * b){
    actor * act=(actor *)a;
    int offset=*(int *)b;
    string s1=act->c;
    string s2=(char *)((char *)act->info+offset);
    return s1.compare(s2);
}

// you should be implementing these two methods right here...
bool imdb::getCredits(const string& player, vector<film>& films)const {
  int siz=*(int *)actorFile;
  int * start=(int *)actorFile+1;
  actor act;
  act.c=(char *)player.c_str();
  act.info=(void *)actorFile;
  void * result=bsearch(&act, start, siz, sizeof(int), cmp1);
  bool p=false;
  if(result!=NULL){
    p=true;
    int offset=*(int *)result;
    void * from=(char *)actorFile+offset;
    char * n=(char *)from;
    string name=n;
    int extra1=0;
    int extra2=0;
    if(name.length()%2==0)++extra1;
    short numberOfMovies=*(short *)((char *)from+name.length()+1+extra1);
    if((name.length()+1+extra1+sizeof(short))%4!=0)extra2=2;
    int * movies=(int *)((char *)from+name.length()+1+extra1+extra2+sizeof(short));
    for(int i=0; i<numberOfMovies; ++i){
        void * movie=((char *)movieFile+movies[i]);
        string title=(char *)movie;
        int delta=*(char *)((char *)movie+title.length()+1);
        int year=1900+delta;
        film m;
        m.title=title;
        m.year=year;
        films.push_back(m);
    }
  }
  if(p)return true;
  else return false;
}

int cmp2(const void * a, const void * b){
    film m1;
    film m2;
    mov * p=(mov *)a;
    m1=p->movi;
    int off=*(int *)b;
    string name=(char *)((char *)p->info+off);
    int year=*(char *)((char *)p->info+off+name.length()+1);
    year+=1900;
    m2.title=name;
    m2.year=year;
    if(m1<m2)return -1;
    else if(m1==m2)return 0;
    else return 1;
}



bool imdb::getCast(const film& movie, vector<string>& players) const {
    int siz=*(int *)movieFile;
    int * start=(int *)movieFile+1;
    mov filmi;
    filmi.movi=movie;
    bool p=false;
    filmi.info=(void *)movieFile;
    void * result=bsearch(&filmi, start, siz, sizeof(int), cmp2);
    if(result!=NULL){
        p=true;
        int offset=*(int *)result;
        void * from=(char *)movieFile+offset;
        string name=(char *)from;
        int yearoffset=*(char *)((char *)from+name.length()+1);
        int year=1900+yearoffset;
        int extra1=0;
        if((name.length()+1+sizeof(char))%2==1)extra1++;
        short actorNumber=*(short *)((char *)from+name.length()+1+extra1+sizeof(char));
        int extra2=0;
        if((name.length()+1+extra1+sizeof(char)+sizeof(short))%4!=0)extra2=2;
        int * from1=(int *)((char *)from+name.length()+1+extra1+sizeof(char)+extra2+sizeof(short));
        for(int i=0; i<actorNumber; ++i){
            int off=from1[i];
            void * actorr=(char *)actorFile+off;
            char * c=(char *)actorr;
            string actorname=c;
            players.push_back(actorname);
        }
    }
 if(p)return true;
 return false;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM..
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
