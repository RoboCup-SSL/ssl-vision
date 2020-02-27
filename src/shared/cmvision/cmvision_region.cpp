
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
  \author  James Bruce (Original CMVision implementation and algorithms),
           SSL-Vision code restructuring, and data structure changes: Stefan Zickler 2008
*/
//========================================================================
#include "cmvision_region.h"

namespace CMVision {

RegionProcessing::RegionProcessing()
{
}


RegionProcessing::~RegionProcessing()
{
}


void RegionProcessing::encodeRuns(Image<raw8> * tmap, CMVision::RunList * runlist)
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
  raw8 m;
  raw8 *row;
  int x,y,j,l;
  CMVision::Run r;

  r.next = 0;

  j = 0;
  for(y=0; y<height; y++){
    row = &map[y * width];

    r.y = y;

    x = 0;
    while(x < width){
      m = row[x];
      r.x = x;

      l = x;

      //fix by Stefan: stop if x==row-width
      //(and don't access the row array in that case as it could cause a segfault)
      //Note that the left argument of the && operator is always evaluated first and as
      //such this expression should be safe.
      while(x != width && row[x] == m) x++;

      if(m != clear || x==width) {
        r.color = m;
        r.width = x - l;
        r.parent = j;
        runs[j++] = r;

        if(j >= max_runs){
          runlist->setUsedRuns(j);
          return;
        }
      }
    }
  }

  runlist->setUsedRuns(j);
}




void RegionProcessing::connectComponents(CMVision::RunList * runlist)
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



void RegionProcessing::extractRegions(CMVision::RegionList * reglist, CMVision::RunList * runlist)
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





int RegionProcessing::separateRegions(CMVision::ColorRegionList * colorlist, CMVision::RegionList * reglist, int min_area)
// Splits the various regions in the region table a separate list for
// each color.  The lists are threaded through the table using the
// region's 'next' field.  Returns the maximal area of the regions,
// which can be used later to speed up sorting.
{
  CMVision::Region * p;
  int i; // ,l;
  uint8_t c;
  int area,max_area;
  int num_regions=reglist->getUsedRegions();
  CMVision::Region * reg = reglist->getRegionArrayPointer();
  int num_colors=colorlist->getNumColorRegions();
  CMVision::RegionLinkedList * color=colorlist->getColorRegionArrayPointer();

  // clear out the region list head table
  for(i=0; i<num_colors; i++){
    color[i].reset();
  }

  // step over the table, adding successive
  // regions to the front of each list
  max_area = 0;
  for(i=0; i<num_regions; i++){
    p = &reg[i];
    c = p->color.v;
    area = p->area;
    if (c >= num_colors) {
      printf("Found a color of index %d...but colorlist is only allocated for a max index of %d\n",c,num_colors-1);
    } else {
      if(area >= min_area){
        if(area > max_area) max_area = area;
        color[c].insertFront(p);
      }
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

CMVision::Region * RegionProcessing::sortRegionListByArea(CMVision::Region *list,int passes)
// Sorts a list of regions by their area field.
// Uses a linked list based radix sort to process the list.
{
  CMVision::Region *tbl[CMV_RADIX],*p,*pn;
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

void RegionProcessing::sortRegions(CMVision::ColorRegionList * colors,int max_area)
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

  int num_colors=colors->getNumColorRegions();
  CMVision::RegionLinkedList * color = colors->getColorRegionArrayPointer();
  // sort each list
  for(i=0; i<num_colors; i++){
    color[i].setFront(sortRegionListByArea(color[i].getInitialElement(),p));
  }
}


#if 0
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


ImageProcessor::ImageProcessor(YUVLUT * _lut, int _max_regions, int _max_runs) {
  lut=_lut;
  max_regions=_max_regions;
  max_runs=_max_runs;
  img_thresholded = new Image<raw8>();
  runlist = new CMVision::RunList(max_runs);
  reglist = new CMVision::RegionList(max_regions);
  colorlist = new CMVision::ColorRegionList(lut->getChannelCount());
}

ImageProcessor::~ImageProcessor() {
  delete reglist;
  delete colorlist;
  delete runlist;
  delete img_thresholded;
}

void ImageProcessor::processYUV422_UYVY(const RawImage * image, int min_blob_area) {
  img_thresholded->allocate(image->getWidth(),image->getHeight());
  Image<raw8> mask;
  mask.allocate(image->getWidth(),image->getHeight());
  memset(mask.getData(),-1,mask.getNumBytes());
  CMVisionThreshold::thresholdImageYUV422_UYVY(img_thresholded,image,lut,&mask);
  processThresholded(img_thresholded,min_blob_area);
}

void ImageProcessor::processYUV444(const ImageInterface * image, int min_blob_area) {
  img_thresholded->allocate(image->getWidth(),image->getHeight());
  Image<raw8> mask;
  mask.allocate(image->getWidth(),image->getHeight());
  memset(mask.getData(),-1,mask.getNumBytes());
  CMVisionThreshold::thresholdImageYUV444(img_thresholded,image,lut,&mask);
  processThresholded(img_thresholded,min_blob_area);
}

void ImageProcessor::processThresholded(Image<raw8> * _img_thresholded, int min_blob_area) {
  CMVision::RegionProcessing::encodeRuns(_img_thresholded, runlist);
  if (runlist->getUsedRuns() == runlist->getMaxRuns()) {
    printf("Warning: runlength encoder exceeded current max run size of %d\n",runlist->getMaxRuns());
  }
  //Connect the components of the runlength map:
  CMVision::RegionProcessing::connectComponents(runlist);

  //Extract Regions from runlength map:
  CMVision::RegionProcessing::extractRegions(reglist, runlist);

  if (reglist->getUsedRegions() == reglist->getMaxRegions()) {
    printf("Warning: Region: extract regions exceeded maximum number of %d regions\n",reglist->getMaxRegions());
  }

  //Separate Regions by colors:
  int max_area = CMVision::RegionProcessing::separateRegions(colorlist, reglist, min_blob_area);

  CMVision::RegionProcessing::sortRegions(colorlist,max_area);
}

ColorRegionList * ImageProcessor::getColorRegionList() {
  return colorlist;
}

}














