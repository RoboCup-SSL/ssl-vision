//
// C++ Implementation: VarTypesInstance
//
// Description: 
//
//
// Author: Stefan Zickler <szickler@cs.cmu.edu>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "VarTypesInstance.h"
namespace VarTypes {
  
  VarTypesInstance* VarTypesInstance::pinstance = 0;// initialize pointer
  
  VarTypesFactory* VarTypesInstance::getFactory ()
  {
    if (pinstance == 0)  // is it the first call?
    {
      pinstance = new VarTypesInstance(); // create sole instance
    }
    if (pinstance->_factory==0) {
      pinstance->_factory= new VarTypesFactory();
    }
    return pinstance->_factory; // address of sole instance
  }
  
  bool VarTypesInstance::setFactory (VarTypesFactory* factory)
  {
    if (pinstance == 0)  // is it the first call?
    {
      pinstance = new VarTypesInstance(); // create sole instance
    }
    if (pinstance->_factory==0) {
      pinstance->_factory= factory;
      return true;
    } else {
      fprintf(stderr,"ERROR: Unable to set VarTypes factory with a VarTypesInstance::setFactory() call because getFactory() has already been called!\n");
      fprintf(stderr,"Please call VarTypesInstance::setFactory() before any other VarType usage (e.g. at the beginning of your program!)\n");
      return false;
    }
  }
  
  VarTypesInstance::VarTypesInstance()
  {
    _factory = 0;
  }
};
