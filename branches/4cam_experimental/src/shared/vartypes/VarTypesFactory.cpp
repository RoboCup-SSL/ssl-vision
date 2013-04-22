//
// C++ Implementation: VarTypesFactory
//
// Description: 
//
//
// Author: Stefan Zickler <szickler@cs.cmu.edu>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "VarTypesFactory.h"

namespace VarTypes {
  VarTypesFactory::VarTypesFactory()
  {
  }
  
  VarTypesFactory::~VarTypesFactory()
  {
  }
  
  VarType * VarTypesFactory::newUserVarType(VarTypeId t) {
    (void)t;
    return 0;
  }
  
  VarVal * VarTypesFactory::newUserVarVal(VarTypeId t) {
    (void)t;
    return 0;
  }
  
  VarTypeId VarTypesFactory::stringToUserType(const string & s) {
    (void)s;
    return VARTYPE_ID_UNDEFINED;
  }
  
  string VarTypesFactory::userTypeToString(VarTypeId t) {
    (void)t;
    return "undefined";
  } 
  
  VarType * VarTypesFactory::newVarType(VarTypeId t) {
    if (t==VARTYPE_ID_BOOL) {
      return new VarBool();
    } else if (t==VARTYPE_ID_INT) {
      return new VarInt();
    } else if (t==VARTYPE_ID_DOUBLE) {
      return new VarDouble();
    } else if (t==VARTYPE_ID_STRING) {
      return new VarString();
    } else if (t==VARTYPE_ID_BLOB) {
      return new VarBlob();
    } else if (t==VARTYPE_ID_EXTERNAL) {
      return new VarExternal();
    } else if (t==VARTYPE_ID_LIST) {
      return new VarList();
    } else if (t==VARTYPE_ID_STRINGENUM) {
      return new VarStringEnum();
    } else if (t==VARTYPE_ID_SELECTION) {
      return new VarSelection();
    //} else if (t==VARTYPE_ID_PROTO_BUFFER) {
    //  return new VarProtoBuffer();
    } else if (t==VARTYPE_ID_QWIDGET) {
      return new VarQWidget();
    } else if (t==VARTYPE_ID_TRIGGER) {
      return new VarTrigger();
    } else {
      if (t >= VARTYPE_ID_MIN_USERTYPE) {
        VarType * v = newUserVarType(t);
        if (v==0) {
          fprintf(stderr,"Error: failed to construct unknown user-defined VarType with VarTypeId: %d (type name: %s).\n",t,typeToString(t).c_str());
          fprintf(stderr,"Note, if you were trying to define your own VarType, you need to overload the newUserVarType(...) function from the VarTypesFactory class.\n");
        }
        return v;
      } else {
        fprintf(stderr,"Error: failed to construct unknown VarType with VarTypeId: %d (type name: %s).\n",t,typeToString(t).c_str());
        fprintf(stderr,"Note if you were trying to define your own VarType, you need to ensure ");
        fprintf(stderr,"that your VarTypeId is greater than %d, as all other values are reserved ",VARTYPE_ID_MIN_USERTYPE);
        fprintf(stderr,"for future native types.\n");
        fprintf(stderr,"Otherwise, this error could have been caused if you are trying to load a VarTypes file that was created with a newer version, containing types that were not yet implemented in this version.\n");
        return 0;
      }
    }
  }
  
  VarVal * VarTypesFactory::newVarVal(VarTypeId t) {
    //TODO: add VarVal creation.
    (void)t;
    return 0;
  }

  VarTypeId VarTypesFactory::stringToType(const string & s) {
    if (s=="bool") {
      return VARTYPE_ID_BOOL;
    } else if (s=="double") {
      return VARTYPE_ID_DOUBLE;
    } else if (s=="int") {
      return VARTYPE_ID_INT;
    } else if (s=="string") {
      return VARTYPE_ID_STRING;
    } else if (s=="blob") {
      return VARTYPE_ID_BLOB;
    } else if (s=="external") {
      return VARTYPE_ID_EXTERNAL;
    } else if (s=="vector2d") {
      return VARTYPE_ID_VECTOR2D;
    } else if (s=="vector3d") {
      return VARTYPE_ID_VECTOR3D;
    } else if (s=="timeline") {
      return VARTYPE_ID_TIMELINE;
    } else if (s=="timevar") {
      return VARTYPE_ID_TIMEVAR;
    } else if (s=="list") {
      return VARTYPE_ID_LIST;
    } else if (s=="stringenum") {
      return VARTYPE_ID_STRINGENUM;
    } else if (s=="selection") {
      return VARTYPE_ID_SELECTION;
    } else if (s=="trigger") {
      return VARTYPE_ID_TRIGGER;
    } else if (s=="qwidget") {
      return VARTYPE_ID_QWIDGET;
    } else {
      VarTypeId v=stringToUserType(s);
      if (v==VARTYPE_ID_UNDEFINED) return v;
      if (v < VARTYPE_ID_MIN_USERTYPE) {
        fprintf(stderr,"Warning! stringToUserType needs to return a typeId greater or equal than %d (was %d for label '%s')\n",VARTYPE_ID_MIN_USERTYPE ,v, s.c_str());
        return VARTYPE_ID_UNDEFINED;
      } else {
        return v;
      }
    }
  }
  
  string VarTypesFactory::typeToString(VarTypeId vt)
  {
    if (vt==VARTYPE_ID_BOOL) {
      return "bool";
    } else if (vt==VARTYPE_ID_DOUBLE) {
      return "double";
    } else if (vt==VARTYPE_ID_INT) {
      return "int";
    } else if (vt==VARTYPE_ID_STRING) {
      return "string";
    } else if (vt==VARTYPE_ID_EXTERNAL) {
      return "external";
    } else if (vt==VARTYPE_ID_BLOB) {
      return "blob";
    } else if (vt==VARTYPE_ID_VECTOR2D) {
      return "vector2d";
    } else if (vt==VARTYPE_ID_VECTOR3D) {
      return "vector3d";
    } else if (vt==VARTYPE_ID_TIMEVAR) {
      return "timevar";
    } else if (vt==VARTYPE_ID_TIMELINE) {
      return "timeline";
    } else if (vt==VARTYPE_ID_LIST) {
      return "list";
    } else if (vt==VARTYPE_ID_STRINGENUM) {
      return "stringenum";
    } else if (vt==VARTYPE_ID_SELECTION) {
      return "selection";
    } else if (vt==VARTYPE_ID_TRIGGER) {
      return "trigger";
    } else if (vt==VARTYPE_ID_QWIDGET) {
      return "qwidget";
    } else {
      string s = userTypeToString(vt);
      if (s=="" || s=="undefined") {
        printf("warning: unknown vartype: %d\n",vt);
        return "undefined";
      }
      return s;
    }
  }
};
