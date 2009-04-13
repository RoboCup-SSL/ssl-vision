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
  \file    affinity_manager.cpp
  \brief   C++ Implementation: affinity_manager
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "affinity_manager.h"

AffinityManager::AffinityManager()
{
  _mutex=new pthread_mutex_t;
  pthread_mutex_init((pthread_mutex_t*)_mutex, NULL);

  parseCpuInfo();
}

AffinityManager::~AffinityManager()
{
  pthread_mutex_destroy((pthread_mutex_t*)_mutex);
  delete _mutex;
}

void AffinityManager::demandCore(int core) {

  DT_LOCK;
  /*unsigned int tid=(long int)syscall(__NR_gettid);
  printf("The ID of this of this thread is: %d\n", tid);
  cpu_set_t cpu_set;
  sched_getaffinity(tid, sizeof(cpu_set), &cpu_set);
  

  for (int i=0;i<4;i++) {
  CPU_CLR(i, &cpu_set);
    printf("CPU %d is %s\n",i,CPU_ISSET(i,&cpu_set) ? "set": "Not set");
  }*/
  DT_UNLOCK;
  //void CPU_CLR(int cpu, cpu_set_t *set);
  //int CPU_ISSET(int cpu, cpu_set_t *set);

}

int AffinityManager::parseFileUpTo(FILE * f, char * output, int len, char end) {
  char c=0;
  char prev_c=0;
  int i=0;
  while (i<(len-1) && ((c=fgetc(f))!=EOF)) {
    if ((c==' ' || c=='\t') && prev_c==0) {
      //ignore any leading spaces
    } else if ((c==' ' || c=='\t') && (prev_c==' ' || prev_c=='\t')) {
      //automatically single space things.
    } else if (c==end || c=='\n') {
      break;
    } else {
      output[i]=c;
      i++;
    }
    prev_c=c;
  }
  if (c==EOF) return -1;
  //remove any trailing space:
  if (i > 0 && (output[i-1]==' ' || output[i-1]=='\t')) output[i-1]=0;
  output[i]=0;
  return i;
}

void AffinityManager::parseCpuInfo() {
  DT_LOCK;
  cores.clear();
  int n=0;
  FILE *f=0;
  char item[256];
  char value[256];
  int phys_id=0;
  int proc_id=0;
  f=fopen("/proc/cpuinfo","r");

  PhysicalCore core;

  while(true) {
    n=parseFileUpTo(f, item, 255, ':');
    if (n==EOF) {
      break;
    } else if (n>0) {
      //printf("item: %s\n",value);
      n=parseFileUpTo(f, value, 255, '\n');
      if (n==EOF) {
        break;
      } else {
        if (strcmp(item,"processor")==0) {
          sscanf(value,"%d",&proc_id);

        } else if (strcmp(item,"core id")==0) {
          if (sscanf(value,"%d",&phys_id) ==1) {
            if (phys_id >= (int)cores.size()) {
              cores.resize(phys_id+1);
            }
          }
          cores[phys_id].enabled=true;
          cores[phys_id].processor_ids.push_back(proc_id);
        }
      }
    }
  }
  fclose(f);
  printf("== Affinity Manager CPU Detection Results =========================\n");
  printf(" Found %d core(s):\n", cores.size());
  for (unsigned int i=0;i< cores.size(); i++) {
    if (cores[i].enabled) {
      printf(" - Core %d with %d HT Processor(s) (IDs: ",i,cores[i].processor_ids.size());
      for (unsigned j=0;j< cores[i].processor_ids.size(); j++) {
        printf("[%d] ",cores[i].processor_ids[j]);
      }
      printf(")\n");
    }
  }
  printf("==================================================================\n");
  DT_UNLOCK;
}
