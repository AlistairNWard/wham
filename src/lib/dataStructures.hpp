#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <string>

#include "api/BamMultiReader.h"
#include "fastahack/Fasta.h"
#include "ssw_cpp.h"


struct regionDat{
  int seqidIndex ;
  int start      ;
  int end        ;
};

struct readPair{
  int          flag;
  int          count;
  BamTools::BamAlignment al1;
  BamTools::BamAlignment al2;
};

struct cigDat{
    int  Length;
    char Type;
};

struct saTag{
    int seqid;
    int pos;
    bool strand;
    int mapQ;
    int NM;
    std::vector<cigDat> cig;
};

struct node;
struct edge;

struct edge{
  node * L;
  node * R;
  std::map<char,int> support;
};

struct node{
  int   seqid                  ;
  int   pos                    ;
  std::vector <edge *>     eds ;
  std::map<std::string,int> sm ;
};

struct graph{
  std::map< int, std::map<int, node *> > nodes;
  std::vector<edge *>   edges;
};

//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : a node
 Function does   : visits all the edges and counts the support
 Function returns: int (count of support)
*/

int getSupport(node * N){

    int support = 0;

    for(std::vector<edge *>::iterator ed = N->eds.begin() ;
        ed != N->eds.end(); ed++){
        support += (*ed)->support['L'];
        support += (*ed)->support['H'];
        support += (*ed)->support['S'];
        support += (*ed)->support['I'];
        support += (*ed)->support['D'];
        support += (*ed)->support['V'];
        support += (*ed)->support['M'];
        support += (*ed)->support['R'];
        support += (*ed)->support['X'];
        support += (*ed)->support['A'];
    }
    return support;
}


class breakpoint{
private:

    std::map<char, std::string > typeMap;
    std::vector<std::string> refs    ;
    std::vector<std::string> seqNames;

    char type;
    std::string typeName;

    int totalGraphWeight ;

    bool paired;
    bool bnd   ;
    bool masked;

    long int length;

    node * nodeL;
    node * nodeR;

    double totalCount;
    double delCount  ;
    double insCount  ;
    double dupCount  ;
    double invCount  ;
    double traCount  ;

    void _count(node * n){
        for(std::vector<edge *>::iterator ed = n->eds.begin() ;
            ed != n->eds.end(); ed++){
            if((*ed)->L->seqid == (*ed)->R->seqid){
                delCount += (*ed)->support['H'];
                delCount += (*ed)->support['D'];
                delCount += (*ed)->support['S'];
                insCount += (*ed)->support['I'];
                insCount += (*ed)->support['R'];
                insCount += (*ed)->support['L'];
                invCount += (*ed)->support['M'];
                invCount += (*ed)->support['A'];
                invCount += (*ed)->support['V'];
                dupCount += (*ed)->support['V'];
                dupCount += (*ed)->support['S'];
                dupCount += (*ed)->support['X'];
            }
            else{
                traCount += (*ed)->support['H'];
                traCount += (*ed)->support['D'];
                traCount += (*ed)->support['S'];
                traCount += (*ed)->support['I'];
                traCount += (*ed)->support['R'];
                traCount += (*ed)->support['L'];
                traCount += (*ed)->support['M'];
                traCount += (*ed)->support['A'];
                traCount += (*ed)->support['V'];
                traCount += (*ed)->support['S'];
                traCount += (*ed)->support['X'];
            }
            totalCount += (*ed)->support['H'];
            totalCount += (*ed)->support['D'];
            totalCount += (*ed)->support['S'];
            totalCount += (*ed)->support['I'];
            totalCount += (*ed)->support['R'];
            totalCount += (*ed)->support['L'];
            totalCount += (*ed)->support['M'];
            totalCount += (*ed)->support['A'];
            totalCount += (*ed)->support['V'];
            totalCount += (*ed)->support['V'];
            totalCount += (*ed)->support['S'];
            totalCount += (*ed)->support['X'];
        }
    }

    void _processInternal(void){
        if(nodeL->seqid != nodeR->seqid ){
            if(nodeL->seqid > nodeR->seqid){
                node * tmp;
                tmp   = nodeL;
                nodeL = nodeR;
                nodeR = tmp;
            }
        }
        else{
            if(nodeL->pos > nodeR->pos){
                node * tmp;
                tmp   = nodeL;
                nodeL = nodeR;
                nodeR = tmp;
            }
            this->length = this->nodeR->pos - nodeL->pos;
        }
    }

public:
    breakpoint(void): type('D')
                    , totalGraphWeight(0)
                    , paired(false)
                    , bnd(false)
                    , masked(false)
                    , length(0)
                    , nodeL(NULL)
                    , nodeR(NULL)
                    , totalCount(0)
                    , delCount(0)
                    , insCount(0)
                    , dupCount(0)
                    , invCount(0)
                    , traCount(0){

        typeMap['D'] = "DEL";
        typeMap['U'] = "DUP";
        typeMap['T'] = "BND";
        typeMap['I'] = "INS";
        typeMap['V'] = "INV";

    }

     long int getLength(void){
        return length;
    }
    void setGoodPair(bool t){
        this->paired = t;
    }
    bool IsMasked(void){
        return this->masked;
    }
    void unsetMasked(void){
        this->masked = false;
    }
    void setMasked(void){
        this->masked = true;
    }
    void setBadPair(void){
        this->paired = true;
    }
    void setTotalSupport(int t){
        this->totalCount = t;
    }

    bool IsBadPair(void){
        return this->paired;
    }
    int getTotalSupport(void){
        return this->totalCount;
    }

    bool add(node * n){
        if(nodeL != NULL && nodeR != NULL){
            return false;
        }
        if( nodeL == NULL ){
            nodeL = n;
            return true;
        }
        else{
            nodeR = n;
            _processInternal();
            return true;
        }
    }
    bool countSupportType(void){
        if(nodeL == NULL || nodeR == NULL){
            return false;
        }
        _count(nodeL);
        _count(nodeR);
        return true;
    }
    void calcType(void){

        int maxN = delCount;

        if(dupCount > maxN){
            type = 'U';
            maxN = dupCount;
        }
        if(invCount > maxN){
            type = 'V';
            maxN = invCount;
        }
        if(insCount > maxN){
            type = 'I';
            maxN = insCount;
        }
        if(traCount > maxN){
            type = 'T';
            maxN = traCount;
        }
        typeName = typeMap[type];
    }

    void getRefBases(std::string & reffile, std::map<int, string> & lookup){


        FastaReference rs;
        rs.open(reffile);

        refs.push_back(rs.getSubSequence(lookup[nodeL->seqid],
                                          nodeL->pos,  1));
        if(nodeL->seqid != nodeR->seqid){
            refs.push_back(rs.getSubSequence(lookup[nodeR->seqid],
                                              nodeR->pos,  1));
        }

        seqNames.push_back(lookup[nodeL->seqid]);
        seqNames.push_back(lookup[nodeR->seqid]);

    }


    friend std::ostream& operator<<(std::ostream& out,
                                    const breakpoint& foo);
};

std::ostream& operator<<(std::ostream& out, const breakpoint & foo){


    long int start = foo.nodeL->pos ;
    long int end   = foo.nodeR->pos ;

    long int len = foo.length;

    if(foo.type == 'I'){
        end = start;
    }
    if(foo.type == 'D'){
        len = -1*len;
    }

    if(foo.type != 'T' && (foo.nodeL->seqid == foo.nodeR->seqid)){
        out << foo.seqNames.front() << "\t"
            << start << "\t"
            << end   << "\t"
            << "D="  << foo.delCount
            << ";U=" << foo.dupCount
            << ";V=" << foo.invCount
            << ";I=" << foo.insCount
            << ";T=" << foo.traCount
            << ";REF=" << foo.refs.front();


        out << ";" << foo.typeName << ";SVLEN=" << len ;
        out << ";BW=" << double(getSupport(foo.nodeL)
                                + getSupport(foo.nodeR))
            / double(foo.totalCount);
    }
    else{
        out << foo.seqNames.front() << "\t"
            << foo.nodeL->pos   << "\t"
            << foo.nodeL->pos   << "\t"
            << "D="  << foo.delCount
            << ";U=" << foo.dupCount
            << ";V=" << foo.invCount
            << ";I=" << foo.insCount
            << ";T=" << foo.traCount
            << ";REF=" << foo.refs.front();

        out << ";SVTYPE=" << foo.typeName << ";SVLEN=." ;
        out << ";BW=" << double(getSupport(foo.nodeL)
                                + getSupport(foo.nodeR))
            / double(foo.totalCount) ;

        out << ";ALT=" << foo.refs.front() << "]"
            << foo.seqNames.back() << ":"
            << foo.nodeR->pos << "]";

        out << std::endl;

        out << foo.seqNames.back() << "\t"
            << foo.nodeR->pos      << "\t"
            << foo.nodeR->pos      << "\t"
            << "D="  << foo.delCount
            << ";U=" << foo.dupCount
            << ";V=" << foo.invCount
            << ";I=" << foo.insCount
            << ";T=" << foo.traCount
            << ";REF=" << foo.refs.back();

        out << ";SVTYPE=" << foo.typeName << ";SVLEN=.";
        out << ";BW=" << double(getSupport(foo.nodeL)
                                + getSupport(foo.nodeR))
            / double(foo.totalCount);
        out << ";ALT=" << foo.refs.back() << "]"
            << foo.seqNames.front() << ":"
            << foo.nodeL->pos << "]";
    }

    return out;
}


struct libraryStats{
  std::map<std::string, double> mus ; // mean of insert length for each indvdual across 1e6 reads
  std::map<std::string, double> sds ;  // standard deviation
  std::map<std::string, double> low ;  // 25% of data
  std::map<std::string, double> upr ;  // 75% of the data
  std::map<std::string, double> swm ;
  std::map<std::string, double> sws ;
  std::map<std::string, double> avgD;
  double overallDepth;
} ;

//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : left and right node
 Function does   : return true if the left is less than the right
 Function returns: bool
*/

bool sortNodesBySupport(node * L, node * R){
  return (getSupport(L) > getSupport(R)) ;
}

//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : a left and right node
 Function does   : provides the rule for the sort
 Function returns: bool
*/

bool sortNodesByPos(node * L, node * R){
  return (L->pos < R->pos);
}

//------------------------------- SUBROUTINE --------------------------------


//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : edge pointer
 Function does   : init
 Function returns: void

*/

void initEdge(edge * e){
  e->L = NULL;
  e->R = NULL;

  // mate too close
  e->support['L'] = 0;
  // mate too far
  e->support['H'] = 0;
  // split read
  e->support['S'] = 0;
  // split read different strand (split)
  e->support['V'] = 0;
  // insertion
  e->support['I'] = 0;
  // deletion
  e->support['D'] = 0;
  // same strand too far
  e->support['M'] = 0;
  // same strand too close
  e->support['R'] = 0;

  // everted read pairs
  e->support['X'] = 0;
  e->support['K'] = 0;
  // same strand
  e->support['A'] = 0;

}

//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : refID (int), left position (int), graph
 Function does   : determine if a node is in the graph
 Function returns: bool
*/

inline bool isInGraph(int refID, int pos, graph & lc){

  if(lc.nodes.find(refID) == lc.nodes.end()){
    return false;
  }

  if(lc.nodes[refID].find(pos) != lc.nodes[refID].end()){
    return true;
  }
  return false;
}


//------------------------------- SUBROUTINE --------------------------------
/*
 Function input  : two node pointer
 Function does   : finds if nodes are directly connected
 Function returns: bool
*/

bool connectedNode(node * left, node * right){

  for(std::vector<edge *>::iterator l = left->eds.begin();
      l != left->eds.end(); l++){

    if(((*l)->L->pos == right->pos
    || (*l)->R->pos == right->pos)
       && (*l)->R->seqid == right->seqid ){
      return true;
    }
  }
  return false;
}

//------------------------------- SUBROUTINE --------------------------------
/*
  Function input  : a vector of edge pointers
  Function does   : does a tree terversal and joins nodes
  Function returns: NA

*/


bool findEdge(std::vector<edge *> & eds, edge ** e, int pos){

  for(std::vector<edge *>::iterator it = eds.begin(); it != eds.end(); it++){
    if((*it)->L->pos == pos || (*it)->R->pos == pos ){
      (*e) = (*it);
      return true;
    }
  }
  return false;
}

//------------------------------- SUBROUTINE --------------------------------
/*
  Function input  : a vector of node pointers, and a postion
  Function does   : removes edges that contain a position
  Function returns: NA

*/


void removeEdges(std::vector<node *> & tree, int pos){

    for(std::vector<node *>::iterator rm = tree.begin();
        rm != tree.end(); rm++){

      std::vector<edge *> tmp;

      for(std::vector<edge *>::iterator e = (*rm)->eds.begin();
        e != (*rm)->eds.end(); e++){
      if( (*e)->L->pos != pos && (*e)->R->pos != pos  ){
        tmp.push_back((*e));
      }
      else{

      }
    }
    (*rm)->eds.clear();
    (*rm)->eds.insert((*rm)->eds.end(), tmp.begin(), tmp.end());
  }
}

#endif