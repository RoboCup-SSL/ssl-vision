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
  \file    affinity_manager.h
  \brief   C++ Interface: affinity_manager
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef AFFINITY_MANAGER_H
#define AFFINITY_MANAGER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <asm/unistd.h>
#include <syscall.h>
#include <sched.h>
#include "pthread.h"
#define DT_LOCK pthread_mutex_lock((pthread_mutex_t*)_mutex);
#define DT_UNLOCK pthread_mutex_unlock((pthread_mutex_t*)_mutex);

using namespace std;

/**
	@author Stefan Zickler
*/
class AffinityManager{
public:
  class PhysicalCore {
    public:
    bool enabled;
    vector<int> processor_ids;
    PhysicalCore() {
      enabled=false;
      processor_ids.clear();
    }
  };
protected:
    pthread_mutex_t * _mutex;
    vector<PhysicalCore> cores;
    int max_cpu_id;
    int parseFileUpTo(FILE * f, char * output, int len, char end);
    void parseCpuInfo();
public:

    void demandCore(int core);
    AffinityManager();

    ~AffinityManager();

};

#endif
