//
// C++ Implementation: VarVal
//
// Description: 
//
//
// Author: Stefan Zickler <szickler@cs.cmu.edu>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "VarVal.h"
#include "VarTypesInstance.h"
namespace VarTypes {
  VarVal::VarVal()
  {
  }
  
  VarVal::~VarVal()
  {
  }
  
  VarVal * VarVal::clone() const {
    return new VarVal();
  }
  
  VarVal * VarVal::deepClone() const {
    return clone();
  }
  
  /// Get the type of this VarType node.
  VarTypeId VarVal::getType() const { return VARTYPE_ID_UNDEFINED; };
  
  /// Get the string label of the type of this VarType node.
  string VarVal::getTypeName() const { return VarTypesInstance::getFactory()->typeToString(getType()) ;};
  
  void VarVal::printdebug() const {
  };
  
  string VarVal::getString() const {
    return "";
  };
  
  bool VarVal::setString(const string & val) {
    (void)val;
    return false;
  };
  
  //the following two functions are accessors for serialization
  //if this is a binary type then base64 might be a good format for i/o
  void VarVal::getSerialString(string & val) const
  {
    val=getString();
  }

  //deprecated wrapper for the above reference-based version
  string VarVal::getSerialString() const
  {
    string result;
    getSerialString(result);
    return result;
  }

  void VarVal::setSerialString(const string & val)
  {
    setString(val);
  }
  
  void VarVal::getBinarySerialString(string & val) const {
    getSerialString(val);
  };

  //deprecated wrapper for the above reference-based version
  string VarVal::getBinarySerialString() const
  {
    string result;
    getBinarySerialString(result);
    return result;
  }

  void VarVal::setBinarySerialString(const string & val) {
    setSerialString(val);
  }
  
  
  //the following functions are especially useful for plotting:
  bool VarVal::hasValue() const
  {
    return false;
  }
  
  bool VarVal::hasMaxValue() const
  {
    return false;
  }
  
  bool VarVal::hasMinValue() const
  {
    return false;
  }
  
  double VarVal::getMinValue() const
  {
    return 0.0;
  }
  
  double VarVal::getMaxValue() const
  {
    return 1.0;
  }
  
  double VarVal::getValue() const
  {
    return 0.0;
  }
};
