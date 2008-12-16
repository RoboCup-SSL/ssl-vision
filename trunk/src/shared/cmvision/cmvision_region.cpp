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
  \file    cmvision_region.cpp
  \brief   C++ Implementation: cmvision_region
  \author  Author Name, 2008
*/
//========================================================================
#include "cmvision_region.h"

CMVisionRegion::CMVisionRegion()
{
}


CMVisionRegion::~CMVisionRegion()
{
}


void CMVisionRegion::encodeRuns(Image<raw8> * tmap, CMVision::Runlist * runlist)
// Changes the flat array version of the thresholded image into a run
// length encoded version, which speeds up later processing since we
// only have to look at the points where values change.
{

  int max_runs = runlist->getMaxRuns();
  CMVision::Run * runs = runlist->getRunArrayPointer();
  raw8 * map = tmap->getPixelData();
  int width=tmap->getWidth();
  int height=tmap->getHeight();


  raw8 clear(0);
  raw8 m,save;
  raw8 *row;
  int x,y,j,l;
  CMVision::Run r;

  r.next = 0;

  // initialize terminator restore
  save = map[0];

  j = 0;
  for(y=0; y<height; y++){
    row = &map[y * width];

    // restore previous terminator and store next
    // one in the first pixel on the next row
    row[0] = save;
    save = row[width];
    row[width] = 255;
    r.y = y;

    x = 0;
    while(x < width){
      m = row[x];
      r.x = x;

      l = x;
      while(row[x] == m) x++;

      if(m != clear || x>=width){
	r.color = m; 
	r.width = x - l;
	r.parent = j;
	runs[j++] = r;

        // printf("run (%d,%d):%d %d\n",r.x,r.y,r.width,r.color);

	if(j >= max_runs){
          row[width] = save;
          runlist->setUsedRuns(j);
          return;
        }
      }
    }
  }

  runlist->setUsedRuns(j);
}




void CMVisionRegion::connectComponents(CMVision::Runlist * runlist)
// Connect components using four-connecteness so that the runs each
// identify the global parent of the connected region they are a part
// of.  It does this by scanning adjacent rows and merging where
// similar colors overlap.  Used to be union by rank w/ path
// compression, but now is just uses path compression as the global
// parent index is a simpler rank bound in practice.
// WARNING: This code is complicated.  I'm pretty sure it's a correct
//   implementation, but minor changes can easily cause big problems.
//   Read the papers on this library and have a good understanding of
//   tree-based union find before you touch it
{

  CMVision::Run * map=runlist->getRunArrayPointer();
  int num = runlist->getUsedRuns();
  int l1,l2;
  CMVision::Run r1,r2;
  int i,j,s;

  // l2 starts on first scan line, l1 starts on second
  l2 = 0;
  l1 = 1;
  while(map[l1].y == 0) l1++; // skip first line

  // Do rest in lock step
  r1 = map[l1];
  r2 = map[l2];
  s = l1;
  while(l1 < num){
    /*
    printf("%6d:(%3d,%3d,%3d) %6d:(%3d,%3d,%3d)\n",
	   l1,r1.x,r1.y,r1.width,
	   l2,r2.x,r2.y,r2.width);
    */

    if(r1.color==r2.color && r1.color.v!=0) {
      // case 1: r2.x <= r1.x < r2.x + r2.width
      // case 2: r1.x <= r2.x < r1.x + r1.width
      if((r2.x<=r1.x && r1.x<r2.x+r2.width) ||
        (r1.x<=r2.x && r2.x<r1.x+r1.width)){
        if(s != l1){
          // if we didn't have a parent already, just take this one
          map[l1].parent = r1.parent = r2.parent;
          s = l1;
        }else if(r1.parent != r2.parent){
          // otherwise union two parents if they are different

          // find terminal roots of each path up tree
          i = r1.parent;
          while(i != map[i].parent) i = map[i].parent;
          j = r2.parent;
          while(j != map[j].parent) j = map[j].parent;

          // union and compress paths; use smaller of two possible
          // representative indicies to preserve DAG property
          if(i < j){
            map[j].parent = i;
            map[l1].parent = map[l2].parent = r1.parent = r2.parent = i;
          }else{
            map[i].parent = j;
            map[l1].parent = map[l2].parent = r1.parent = r2.parent = j;
          }
        }
      }
    }

    // Move to next point where values may change
    i = (r2.x + r2.width) - (r1.x + r1.width);
    if(i >= 0) r1 = map[++l1];
    if(i <= 0) r2 = map[++l2];
  }

  // Now we need to compress all parent paths
  for(i=0; i<num; i++){
    j = map[i].parent;
    map[i].parent = map[j].parent;
  }
}



void CMVisionRegion::extractRegions(CMVision::Regionlist * reglist, CMVision::Runlist * runlist)
// Takes the list of runs and formats them into a region table,
// gathering the various statistics along the way.  num is the number
// of runs in the rmap array, and the number of unique regions in
// reg[] (bounded by max_reg) is returned.  Implemented as a single
// pass over the array of runs.
{
  int b,i,n,a;
  CMVision::Run r;
  CMVision::Region * reg = reglist->getRegionArrayPointer();
  CMVision::Run * rmap = runlist->getRunArrayPointer();
  int max_reg=reglist->getMaxRegions();
  int num = runlist->getUsedRuns();

  n = 0;

  for(i=0; i<num; i++){
    if(rmap[i].color.v!=0){
      r = rmap[i];
      if(r.parent == i){
        // Add new region if this run is a root (i.e. self parented)
        rmap[i].parent = b = n;  // renumber to point to region id
        reg[b].color = r.color;
        reg[b].area = r.width;
        reg[b].x1 = r.x;
        reg[b].y1 = r.y;
        reg[b].x2 = r.x + r.width;
        reg[b].y2 = r.y;
        reg[b].cen_x = rangeSum(r.x,r.width);
        reg[b].cen_y = r.y * r.width;
        reg[b].run_start = i;
        reg[b].iterator_id = i; // temporarily use to store last run
        n++;
        if(n >= max_reg) {
          reglist->setUsedRegions(max_reg);
          return;
        }
      }else{
        // Otherwise update region stats incrementally
        b = rmap[r.parent].parent;
        rmap[i].parent = b; // update parent to identify region id
        reg[b].area += r.width;
        reg[b].x2 = max(r.x + r.width,reg[b].x2);
        reg[b].x1 = min((int)r.x,reg[b].x1);
        reg[b].y2 = r.y; // last set by lowest run
        reg[b].cen_x += rangeSum(r.x,r.width);
        reg[b].cen_y += r.y * r.width;
        // set previous run to point to this one as next
        rmap[reg[b].iterator_id].next = i;
        reg[b].iterator_id = i;
      }
    }
  }

  // calculate centroids from stored sums
  for(i=0; i<n; i++){
    a = reg[i].area;
    reg[i].cen_x = (float)reg[i].cen_x / a;
    reg[i].cen_y = (float)reg[i].cen_y / a;
    rmap[reg[i].iterator_id].next = 0; // -1;
    reg[i].iterator_id = 0;
    reg[i].x2--; // change to inclusive range
  }

  reglist->setUsedRegions(n);
  return;
}





#if 0
template <class color_class_state_t,class region_t>
int SeparateRegions(color_class_state_t *color,int colors,
		    region_t *reg,int num)
// Splits the various regions in the region table a separate list for
// each color.  The lists are threaded through the table using the
// region's 'next' field.  Returns the maximal area of the regions,
// which can be used later to speed up sorting.
{
  region_t *p;
  int i; // ,l;
  int c;
  int area,max_area;

  // clear out the region list head table
  for(i=0; i<colors; i++){
    color[i].list = NULL;
    color[i].num  = 0;
  }

  // step over the table, adding successive
  // regions to the front of each list
  max_area = 0;
  for(i=0; i<num; i++){
    p = &reg[i];
    c = p->color;
    area = p->area;

    if(area >= color[c].min_area){
      if(area > max_area) max_area = area;
      color[c].num++;
      p->next = color[c].list;
      color[c].list = p;
    }
  }

  return(max_area);
}

// These are the tweaking values for the radix sort given below
// Feel free to change them, though these values seemed to work well
// in testing.  Don't worry about extra passes to get all 32 bits of
// the area; the implementation only does as many passes as needed to
// touch the most significant set bit (MSB of largest region's area)
#define CMV_RBITS 6
#define CMV_RADIX (1 << CMV_RBITS)
#define CMV_RMASK (CMV_RADIX-1)

template <class region_t>
region_t *SortRegionListByArea(region_t *list,int passes)
// Sorts a list of regions by their area field.
// Uses a linked list based radix sort to process the list.
{
  region_t *tbl[CMV_RADIX],*p,*pn;
  int slot,shift;
  int i,j;

  // Handle trivial cases
  if(!list || !list->next) return(list);

  // Initialize table
  for(j=0; j<CMV_RADIX; j++) tbl[j] = NULL;

  for(i=0; i<passes; i++){
    // split list into buckets
    shift = CMV_RBITS * i;
    p = list;
    while(p){
      pn = p->next;
      slot = ((p->area) >> shift) & CMV_RMASK;
      p->next = tbl[slot];
      tbl[slot] = p;
      p = pn;
    }

    // integrate back into partially ordered list
    list = NULL;
    for(j=0; j<CMV_RADIX; j++){
      p = tbl[j];
      tbl[j] = NULL; // clear out table for next pass
      while(p){
        pn = p->next;
        p->next = list;
        list = p;
        p = pn;
      }
    }
  }

  return(list);
}

template <class color_class_state_t>
void SortRegions(color_class_state_t *color,int colors,int max_area)
// Sorts entire region table by area, using the above
// function to sort each threaded region list.
{
  int i,p;

  // do minimal number of passes sufficient to touch all set bits
  p = 0;
  while(max_area != 0){
    max_area >>= CMV_RBITS;
    p++;
  }

  // sort each list
  for(i=0; i<colors; i++){
    color[i].list = SortRegionListByArea(color[i].list,p);
  }
}

template <class region_t,class rle_t>
int FindStart(rle_t *rmap,int left,int right,int x)
// This function uses binary search to find the leftmost run whose
// interval either contains or is greater than x.
{
  int m;

  while(right > left){
    m = (left + right) / 2;
    if(x > rmap[m].x+rmap[m].width){
      left = m + 1;
    }else if(x < rmap[m].x){
      right = m;
    }else{
      return(m);
    }
  }

  return(m);
}

#endif



















