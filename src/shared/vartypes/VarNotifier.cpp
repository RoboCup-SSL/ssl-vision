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
  \file    varnotifier.cpp
  \brief   C++ Implementation: varnotifier
  \author  Author Name, 2009
*/
//========================================================================
#include "VarNotifier.h"

VarNotifier::VarNotifier()
{
  changed=false;
}


VarNotifier::~VarNotifier()
{
}

void VarNotifier::clear() {
  mutex.lock();
  QHash<VarData *, VarNotificationType>::const_iterator iter;
  for (iter = senders.constBegin();iter != senders.constEnd();iter++) {
    internalNonMutexedRemoveItem(iter.key());
  }
  senders.clear();
  mutex.unlock();
}

bool VarNotifier::hasChanged() {
  bool result=false;
  mutex.lock();
  if (changed) {
    result=true;
    changed=false;
  }
  mutex.unlock();
  return result;
}

bool VarNotifier::hasChangedNoReset() {
  bool result;
  mutex.lock();
  result=changed;
  mutex.unlock();
  return result;
}

void VarNotifier::changeSlotOtherChange() {
  mutex.lock();
  changed=true;
  mutex.unlock();
  emit changeOccured(0);
}

void VarNotifier::changeSlot(VarData * item) {
  mutex.lock();
  changed=true;
  mutex.unlock();
  emit changeOccured(item);
}

void VarNotifier::setChanged(bool value) {
  mutex.lock();
  changed=value;
  mutex.unlock();
}


void VarNotifier::internalNonMutexedRemoveItem(VarData * item) {
  if (item==0) return;
  QHash<VarData *, VarNotificationType>::iterator iter;
  iter = senders.find(item);
  if (iter != senders.constEnd()) {
    disconnect(item, 0, this, SLOT(changeSlot(VarData *)));
    senders.erase(iter);
  }
}

void VarNotifier::internalNonMutexedAddItem(VarData * item, VarNotificationType notification_type) {
  if (item==0) return;
  QHash<VarData *, VarNotificationType>::iterator iter;
  iter = senders.find(item);
  if (iter != senders.constEnd()) {
    if (iter.value() == notification_type) {
      //this exact item already exists.
      return;
    } else {
      //item exists, but with wrong notification type:
      //fix it:
      disconnect(item, 0, this, SLOT(changeSlot(VarData *)));
      iter.value() = notification_type;
    }
  } else {
    iter=senders.insert(item,notification_type);
  }
  //connect it with correct notification type:
  if (notification_type==VarNotificationChanged) {
    connect(item, SIGNAL(hasChanged(VarData *)), this, SLOT(changeSlot(VarData *)));
  } else {
    connect(item, SIGNAL(wasEdited(VarData*)), this, SLOT(changeSlot(VarData *)));
  }
}

void VarNotifier::addItem(VarData * item, VarNotificationType notification_type) {
  mutex.lock();
  internalNonMutexedAddItem(item, notification_type);
  mutex.unlock();
}

void VarNotifier::addRecursive(VarData * item, VarNotificationType notification_type, bool include_root) {
  mutex.lock();
  QQueue<VarData *> queue;
  if (item!=0) queue.enqueue(item);
  while(queue.isEmpty()==false) {
    VarData * d = queue.dequeue();
    if ((d!=item) || include_root) internalNonMutexedAddItem(d,notification_type);
    vector<VarData *> children = d->getChildren();
    int s=children.size();
    for (int i=0;i<s;i++) {
      if (children[i]!=0) queue.enqueue(children[i]);
    }
  }
  mutex.unlock();
}

void VarNotifier::removeItem(VarData *item) {
  mutex.lock();
  internalNonMutexedRemoveItem(item);
  mutex.unlock();
}

void VarNotifier::removeRecursive(VarData * item, bool include_root) {
  mutex.lock();
  QQueue<VarData *> queue;
  if (item!=0) queue.enqueue(item);
  while(queue.isEmpty()==false) {
    VarData * d = queue.dequeue();
    if ((d!=item) || include_root) internalNonMutexedRemoveItem(item);
    vector<VarData *> children = d->getChildren();
    int s=children.size();
    for (int i=0;i<s;i++) {
      if (children[i]!=0) queue.enqueue(children[i]);
    }
  }
  mutex.unlock();
}
