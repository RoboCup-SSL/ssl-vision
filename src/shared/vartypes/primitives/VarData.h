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
  \file    VarData.h
  \brief   C++ Interface: VarData
  \author  Stefan Zickler, (C) 2008
*/
#ifndef VDATATYPE_H_
#define VDATATYPE_H_
#include <string>
#include <vector>
#include <limits.h>
#include <float.h>

/*!
  \class  VarData
  \brief  The base class of the VarTypes system.
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  This is is the base-class of the VarType system.
  All other VarTypes must publicly inherit this class.

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/
#ifndef VDATA_NO_QT
  #include <QObject>
#endif

#ifndef VDATA_NO_XML
  #include "xml/xmlParser.h"
#endif

#ifndef VDATA_NO_THREAD_SAFETY
  #include <pthread.h>
#endif

using namespace std;

/*!
  \enum vDataFlags
  \brief bitflags for setting meta-properties of a VarType
*/
enum vDataFlags {
  DT_FLAG_NONE = 0x00,  /// no special properties
  DT_FLAG_READONLY= 0x01 << 0, /// this vartype is not editable in the GUI
  DT_FLAG_HIDDEN  = 0x01 << 1, /// this vartype will be hidden in the GUI
  DT_FLAG_AUTO_EXPAND = 0x01 << 2, /// whether to expand this node by default
  DT_FLAG_AUTO_EXPAND_TREE = 0x01 << 3, /// whether to expand this node and all of its sub-nodes by default
  DT_FLAG_NOSAVE = 0x01 << 4, /// don't store this subtree to XML
  DT_FLAG_NOLOAD = 0x01 << 5, /// don't load this subtree from XML
  DT_FLAG_PERSISTENT = 0x01 << 6, /// make the item editor be persistent (always open, not just when clicked on it)
  DT_FLAG_HIDE_CHILDREN = 0x01 << 7, /// hide the entire subtree (but not node itself)
  DT_FLAG_ENUM_COUNT = 0x01 << 8,
  DT_FLAG_NOSTORE = DT_FLAG_NOSAVE | DT_FLAG_NOLOAD
};

typedef int VarDataFlag;

enum vDataTypeEnum {
  DT_UNDEFINED,
  DT_BOOL,
  DT_INT,
  DT_DOUBLE,
  DT_STRING,
  DT_BLOB,
  DT_EXTERNAL,
  DT_VECTOR2D,
  DT_VECTOR3D,
  DT_TIMEVAR,
  DT_TIMELINE,
  DT_LIST,
  DT_STRINGENUM,
  DT_TRIGGER,
  DT_QWIDGET,
  //---IMPORTANT-----------
  // if adding a datatype here you will also need to integrate it
  // to the following places to achieve full functionality
  // in the functions
  //   stringToType() in VarData.cpp
  //   typeToString() in VarData.cpp
  //   newVarType() in VarTypes.cpp
  // also make sure that DT_COUNT is always last in the enum
  DT_COUNT,

};

#ifndef VDATA_NO_QT
  //if using QT, trigger the hasChanged() signal
  #define CHANGE_MACRO hasChanged();
#else
  //if not using QT, don't do anything
  #define CHANGE_MACRO
#endif

#ifndef VDATA_NO_THREAD_SAFETY
  #define DT_LOCK pthread_mutex_lock((pthread_mutex_t*)_mutex);
  #define DT_UNLOCK pthread_mutex_unlock((pthread_mutex_t*)_mutex);
#else
  #define DT_LOCK
  #define DT_UNLOCK
#endif

#ifndef VDATA_NO_QT
//if using QT, inherit QObject as a base
class VarData : public QObject
#else
class VarData
#endif
{

#ifndef VDATA_NO_QT
  //if using QT, enable signals by employing the q_object macro
  Q_OBJECT
signals:
  /// This signal is triggered when any data or flag of this VarType has changed.
  /// This includes changes that were done programmatically, as well through a GUI.
  /// It also includes changes to render-flags or other meta-data.
  void hasChanged();

  /// This signal is triggered when data of this VarType has been edited by a user through an MVC viewer.
  /// Unlike /c hasChanged(), this signal is not necessarily triggered if this data was internally changed.
  /// So, if you would like to catch all events, you should use /c hasChanged() instead.
  void wasEdited(VarData *); //this signal is only triggered when a value was edited by a user through a MVC view-instance.

public slots:
  /// A slot to receive signals from a model-view system that editing of this item was just completed.
  void mvcEditCompleted() {
    wasEdited(this);
  }
#endif
protected:
  #ifndef VDATA_NO_THREAD_SAFETY
    pthread_mutex_t * _mutex;
  #endif
  VarDataFlag _flags;
  string _name;
  static string fixString(const char * cst);
  static string intToString(int val);
  static string doubleToString(double val);

public:
  /// A conversion from a string-label to the actual type enum of the VarData type.
  /// \see typeToString(vDataTypeEnum vt) for its inverse.
  static vDataTypeEnum stringToType(string stype);

  /// A conversion from the actual type enum of the VarData type to a string label.
  /// \see stringToType(string stype) for its inverse.
  static string typeToString(vDataTypeEnum vt);

  /// A constructor function which will create a new vartype instance, based on the desired type t.
  static VarData * newVarType(vDataTypeEnum t);

  /// The constructor of a VarData type. If inheriting this class, you should forward the name argument
  /// to this constructor.
  ///
  /// \param name The string label of this VarData node
  VarData(string name="");
  virtual ~VarData();

  /// Return a list of children nodes of this VarData type.
  /// This can be an empty vector if the node does not have any children.
  virtual vector<VarData *> getChildren() const;

  /// Return the current render option bitflags
  /// \see VarDataFlag
  virtual VarDataFlag getRenderFlags() const;

  /// Set the current render option bitflags
  /// \see VarDataFlag
  virtual void setRenderFlags(VarDataFlag f);

  /// Add (bitwise-or) some renderflags f to the currently set renderflags.
  /// \see VarDataFlag
  virtual void addRenderFlags(VarDataFlag f);

  /// Remove some renderflags f from the currently set renderflags.
  /// \see VarDataFlag
  virtual void removeRenderFlags(VarDataFlag f);

  /// Check whether all the flags specified in f are currently set.
  /// \see VarDataFlag  
  virtual bool areRenderFlagsSet(VarDataFlag f) const;

  /// Get the type of this VarData node.
  virtual vDataTypeEnum getType() const;

  /// Get the string label of the type of this VarData node.
  virtual string getTypeName() const;

  /// Set the string label of this node.
  virtual void setName(string name);

  /// Get the string label of this node.
  virtual string getName() const;


  /// Reset the values of this node to default.
  virtual void resetToDefault();

  /// Print out debugging information of this node to console.
  /// Usually this means the actually data of the node will be printed.
  virtual void printdebug() const;

  /// Get a human-readable string representation of this node's data.
  virtual string getString() const;

  /// Set this node's data to some string representation.
  virtual bool setString(const string & val);

  /// Get a full canonical string representation of this node.
  /// This will be used to store the node to XML.
  virtual const char * getSerialString() const;

  /// Set the node to some full canonical string representation.
  /// This will be used to load the node from XML.
  virtual void setSerialString(const string & val);

  /// This function indicates whether this nodes represents a numeric value.
  /// This is useful for e.g. determining whether this item is plottable.
  /// (e.g. whether this node has some kind of y-value)
  virtual bool hasValue() const;

  /// This function indicates whether this node's value range has a pre-specified
  /// maximum
  /// This is useful for plotting.
  /// \see getMaxValue()
  virtual bool hasMaxValue() const; //indicates whether the value range has
  // a pre-specified maximum

  /// This function indicates whether this node's value range has a pre-specified
  /// minimum.
  /// This is useful for plotting.
  /// \see getMinValue()
  virtual bool hasMinValue() const; //indicates whether the value range has
  // a pre-specified minimum

  /// This function returns the pre-specified minimum of this node's value range.
  /// To first check whether this node has such a minimum, please see \c hasMinValue() .
  ///
  /// \see hasMinValue()
  virtual double getMinValue() const; // returns the pre-specified minimum

  /// This function returns the pre-specified maximum of this node's value range.
  /// To first check whether this node has such a maximum, please see \c hasMaxValue() .
  ///
  /// \see hasMaxValue()
  virtual double getMaxValue() const; // returns the pre-specified maximum

  /// Returns a single, numeric value of this node.
  /// To first check whether this node has a numeric value, please see \c hasValue() .
  ///
  /// \see hasValue()
  virtual double getValue() const; //returns the numeric value


#ifndef VDATA_NO_XML
  //XML LOAD/STORE functions
protected:

  //The following 6 functions are the ones to be reimplemented by subclasses to perform the correct
  //XML store/load actions
  virtual void updateAttributes(XMLNode & us) const;
  virtual void updateText(XMLNode & us) const;
  virtual void updateChildren(XMLNode & us) const;

  virtual void readAttributes(XMLNode & us);
  virtual void readText(XMLNode & us);
  virtual void readChildren(XMLNode & us);

  static XMLNode findOrAppendChild(XMLNode & parent, string key, string val);
public:
  /// Clear an XMLNode's list of children.
  static void deleteAllVarChildren(XMLNode & node);

  /// A helper function to read a list of children from XML and convert it to a vector of VarData nodes.
  static vector<VarData *> readChildrenHelper(XMLNode & parent , vector<VarData *> existing_children, bool only_update_existing, bool blind_append);

  /// Write the contents of this VarData node to an XMLNode.
  void writeXML(XMLNode & parent, bool blind_append=true) const;

  /// Let this VarData node load the contents of an XMLNode.
  void readXML(XMLNode & us);
#endif
};


#endif /*VDATATYPE_H_*/
