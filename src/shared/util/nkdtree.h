//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    nkdtree.h
  \brief   NKD-Tree
  \author  James R. Bruce, (C) 1999-2002
*/
//========================================================================
#ifndef __NKD_TREE_H__
#define __NKD_TREE_H__

#include <stdio.h>

#include <queue>

#include "util.h"
#include "nvector.h"

template <class state_t>
class NKDTreeGetNext{
public:
  state_t *&operator()(state_t *s)
    {return(s->next);}
  const state_t * const &operator()(const state_t *s) const
    {return(s->next);}
};

#define NKD_TEM \
  template <class state_t,typename num_t,int dim,bool scaled,class get_next>
#define NKD_FUN NKDTree<state_t,num_t,dim,scaled,get_next>

template <class state_t,typename num_t,int dim,bool scaled=false,
          class get_next=NKDTreeGetNext<state_t> >
class NKDTree{
public:
  typedef Vec::NVector<num_t,dim> vec_t;

  struct BBox{
    vec_t min,max;
  };

  struct Node : public BBox{
    int split_dim;
    num_t threshold;

    // state storage (leaf nodes only)
    state_t *states;
    int num_states;

    // child pointers (internal nodes only)
    Node *child[2];
  };

  struct QueryNode{
    float dist;
    Node *node;
    state_t *state;

    bool operator <(const QueryNode &qn) const
      {return(dist > qn.dist);}
  };

protected:
  Node *root;
  int leaf_size,max_depth;
  int is_built;

  vec_t query_point;
  double query_max_dist;
  vec_t scale;

  typedef std::priority_queue<QueryNode> _PriorityQueue;
  class PriorityQueue : public _PriorityQueue{
  public:
    void clear() {_PriorityQueue::c.clear();}
  };
  PriorityQueue queue;

protected:
  double distFromQuery(Node *p);
  double distFromQuery(state_t *s);

  void initBBox(BBox &b,state_t &s);
  void updateBBox(BBox &b,state_t &s);
  void calcBBox(BBox &b,state_t *s);
  void calcBBox(BBox &b,BBox &c1,BBox &c2);

  void freeTree(Node *p);
  void add(Node **q,state_t *s,int level);
  void split(Node *p,int level);

  void addToSearchQueue(Node *p);
  void addToSearchQueue(state_t *s);

  state_t *getOnlyNearest(Node *t,state_t *nearest);

  void draw(const Node *t,int levels) const;

public:
  NKDTree() {root=NULL; leaf_size=16; max_depth=20; scale.set(1.0);}
  ~NKDTree() {clear();}

  void add(state_t *s) {add(&root,s,0);}
  void build();
  void clear() {freeTree(root); root=NULL; is_built=false;}
  void setScale(int dim_idx,num_t val)
    {scale[dim_idx]=val;}

  // multiple state iterative query
  void startQuery(const state_t &_query_point,double _query_max_dist);
  void startQuery(const vec_t &_query_point,double _query_max_dist);
  state_t *getNextNearest(double &dist);
  void endQuery() {queue.clear();}

  // single shot query
  state_t *getOnlyNearest(const state_t &s,double &max_dist);

  bool isEmpty() const
    {return(!root || root->num_states==0);}
  void draw(int levels) const
    {if(dim==2) draw(root,levels);}
};

NKD_TEM
double NKD_FUN::distFromQuery(Node *p)
{
  vec_t near;
  near.bound(query_point, p->min, p->max);
  if(!scaled){
    return(dist(query_point, near));
  }else{
    double d = 0.0;
    for(int i=0; i<dim; i++){
      d += sq((query_point[i] - near[i]) * scale[i]);
    }
    return(sqrt(d));
  }
}

NKD_TEM
double NKD_FUN::distFromQuery(state_t *s)
{
  int i;
  double d = 0.0;
  for(i=0; i<dim; i++){
    if(!scaled){
      d += sq(query_point[i] - (*s)[i]);
    }else{
      d += sq(query_point[i] - (*s)[i]) * scale[i];
    }
  }
  return(sqrt(d));
}

NKD_TEM
void NKD_FUN::initBBox(BBox &b,state_t &s)
// initializes bounding box to contain a state
{
  for(int i=0; i<dim; i++){
    b.min[i] = b.max[i] = s[i];
  }
}

NKD_TEM
void NKD_FUN::updateBBox(BBox &b,state_t &s)
// Updates an existing bbox for a new state
{
  int i;
  num_t v;

  for(i=0; i<dim; i++){
    v = s[i];
    if(v < b.min[i]) b.min[i] = v;
    if(v > b.max[i]) b.max[i] = v;
  }
}

NKD_TEM
void NKD_FUN::calcBBox(BBox &b,state_t *s)
// Calculates bounding box for a list of states
// Note: assumes s!=NULL (otherwise bbox would be undefined anyway)
{
  get_next next;

  // set bbox equal to first state
  initBBox(*s);
  s = next(s);

  // enlarge using additional states
  while(s){
    updateBBox(*s);
    s = next(s);
  }
}

NKD_TEM
void NKD_FUN::calcBBox(BBox &b,BBox &c1,BBox &c2)
// Calculates bounding box (b) for two child bounding boxes (c1,c2)
{
  b.min.min(c1.min, c2.min);
  b.max.max(c1.max, c2.max);
}

NKD_TEM
void NKD_FUN::freeTree(Node *p)
{
  if(p){
    freeTree(p->child[0]); p->child[0]=NULL;
    freeTree(p->child[1]); p->child[1]=NULL;
    delete(p);
  }
}

NKD_TEM
void NKD_FUN::add(Node **q,state_t *s,int level)
{
  get_next next;
  Node *p = *q;

  // check if p is a leaf node
  if(!p || !p->child[0]){
    // we're at a leaf, so add state to this node

    // make a new node if none exists here
    if(!p){
      *q = p = new Node;
      mzero(*p);
      initBBox(*p,*s);
    }

    // add point
    next(s) = p->states;
    p->states = s;
    p->num_states++;
    updateBBox(*p,*s);

    // if tree has already been built, split leaf if necessary
    if(is_built) split(p,level);
  }else{
    // there is an existing tree and we're at an internal node, so
    // just update out bounding box and recurse through this node
    updateBBox(*p,*s);
    int i = (*s)[p->split_dim] > p->threshold;
    add(&p->child[i],s,level+1);
  }
}

NKD_TEM
void NKD_FUN::split(Node *p,int level)
{
  get_next next;
  state_t *s,*sn;
  int sd;
  double sum;
  int i;

  // check if we actually want to split
  if(!p) return;
  if((p->num_states <= leaf_size) || (level >= max_depth)) return;
  if(sqdist(p->min,p->max) < 1E-9) return;

  // choose split dimension as round-robin
  p->split_dim = sd = level % dim;

  // calculate mean for split threshold
  sum = 0.0;
  s = p->states;
  while(s){
    sum += (*s)[sd];
    s = next(s);
  }
  p->threshold = sum / p->num_states;

  // split the points based on the plane
  s = p->states;
  while(s){
    sn = next(s);

    i = (*s)[sd] > p->threshold;
    add(&(p->child[i]),s,level+1);

    s = sn;
  }

  // this state no longer stores points
  p->states = 0;
  p->num_states = 0;

  if(!is_built){
    split(p->child[0],level+1);
    split(p->child[1],level+1);
  }
}

NKD_TEM
void NKD_FUN::build()
{
  if(!is_built){
    if(root) split(root,0);
    is_built = true;
  }
}

NKD_TEM
void NKD_FUN::startQuery(const state_t &_query_point,double _query_max_dist)
{
  endQuery();
  for(int i=0; i<dim; i++) query_point[i] = _query_point[i];
  query_max_dist = _query_max_dist;
  if(root) addToSearchQueue(root);
}

NKD_TEM
void NKD_FUN::startQuery(const vec_t &_query_point,double _query_max_dist)
{
  endQuery();
  query_point = _query_point;
  query_max_dist = _query_max_dist;
  if(root) addToSearchQueue(root);
}

NKD_TEM
void NKD_FUN::addToSearchQueue(Node *p)
{
  QueryNode qn;
  qn.dist = distFromQuery(p);
  if(qn.dist < query_max_dist){
    qn.node = p;
    qn.state = NULL;
    queue.push(qn);
  }
}

NKD_TEM
void NKD_FUN::addToSearchQueue(state_t *s)
{
  QueryNode qn;
  qn.dist = distFromQuery(s);
  if(qn.dist < query_max_dist){
    qn.node = NULL;
    qn.state = s;
    queue.push(qn);
  }
}

NKD_TEM
state_t *NKD_FUN::getNextNearest(double &dist)
{
  get_next next;
  QueryNode qn;
  state_t *s;

  while(queue.size() > 0){
    // get head of priority queue
    qn = queue.top();
    queue.pop();

    // if its a raw state, return it
    if(qn.state){
      dist = qn.dist;
      return(qn.state);
    }

    // otherwise it is a node, so we have to expand it

    // expand nodes
    if(qn.node->child[0]) addToSearchQueue(qn.node->child[0]);
    if(qn.node->child[1]) addToSearchQueue(qn.node->child[1]);

    // expand states
    s = qn.node->states;
    while(s){
      addToSearchQueue(s);
      s = next(s);
    }
  }

  // ran out of states
  return(NULL);
}

NKD_TEM
state_t *NKD_FUN::getOnlyNearest(Node *t,state_t *nearest)
{
  get_next next;
  state_t *s;
  double d;

  // pruning
  if(t==NULL || distFromQuery(t)>query_max_dist) return(nearest);

  // we either store states (leaf) or are have children (interal node)
  if(t->states){
    // leaf node, check the states stored here to update the nearest
    s = t->states;
    while(s){
      d = distFromQuery(s);
      if(d < query_max_dist){
        nearest = s;
        query_max_dist = d;
      }
      s = next(s);
    }
  }else{
    // internal node, find out which side of the plane our query is on
    // and descend the tree on that side first, followed by the
    // further side.
    int i = query_point[t->split_dim] > t->threshold;
    nearest = getOnlyNearest(t->child[ i],nearest);
    nearest = getOnlyNearest(t->child[!i],nearest);
  }

  return(nearest);
}

NKD_TEM
state_t *NKD_FUN::getOnlyNearest(const state_t &s,double &max_dist)
{
  state_t *sn;

  /*
  endQuery();
  startQuery(s,max_dist);
  sn = getNextNearest(max_dist);
  endQuery();
  */

  for(int i=0; i<dim; i++) query_point[i] = s[i];
  query_max_dist = max_dist;
  sn = getOnlyNearest(root,NULL);
  max_dist = query_max_dist;
  return(sn);
}

#ifdef DRAWING
#include "draw.h"
NKD_TEM
void NKD_FUN::draw(const Node *t,int levels) const
{
  get_next next;
  state_t *s;

  if(t && levels>0){
    vector2f c,r,p;
    c.set((t->max[0] + t->min[0])/2,
          (t->max[1] + t->min[1])/2);
    r.set((t->max[0] - t->min[0])/2,
          (t->max[1] - t->min[1])/2);
    DrawRectangle(c,r);

    if(s = t->states){
      while(s){
        p.set(s[0],s[1]);
        DrawLine(c,p);
        s = next(s);
      }
    }else{
      draw(t->child[0],levels-1);
      draw(t->child[1],levels-1);
    }
  }
}
#endif

#endif
