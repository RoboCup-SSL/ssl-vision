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
  \file    capturedc1394v2.cpp
  \brief   C++ Implementation: CaptureDC1394v2
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "capturedc1394v2.h"

namespace {
int compareCameraGUID(const void* a, const void* b) {
  dc1394camera_id_t* cam1 = (dc1394camera_id_t*)a;
  dc1394camera_id_t* cam2 = (dc1394camera_id_t*)b;

  if(cam1->guid == cam2->guid)
    return 0;

  if(cam1->guid > cam2->guid)
    return 1;
  else
    return -1;
}
}  // namespace
GlobalCaptureDC1394instanceManager* GlobalCaptureDC1394instanceManager::pinstance = 0;

dc1394_t* GlobalCaptureDC1394instanceManager::obtainInstance() {
  if (pinstance==0) pinstance=new GlobalCaptureDC1394instanceManager();
  return pinstance->instance->obtainInstance();
}
bool GlobalCaptureDC1394instanceManager::removeInstance() {
  if (pinstance==0) pinstance=new GlobalCaptureDC1394instanceManager();
  return pinstance->instance->removeInstance();
}
GlobalCaptureDC1394instanceManager::GlobalCaptureDC1394instanceManager() {
  instance=new GlobalCaptureDC1394instance();
}
GlobalCaptureDC1394instanceManager::~GlobalCaptureDC1394instanceManager() {
  if (instance!=0) delete instance;
  instance=0;
}


CaptureDC1394v2::CaptureDC1394v2(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
{
  dc1394_instance=GlobalCaptureDC1394instanceManager::obtainInstance();
  cam_list=0;
  cam_id=default_camera_id;
  camera=0;
  is_capturing=false;
    mutex.lock();

  settings->addChild(conversion_settings = new VarList("Conversion Settings"));
  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild(v_colorout=new VarStringEnum("convert to mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));

  conversion_settings->addChild(v_debayer=new VarBool("de-bayer",false));
  conversion_settings->addChild(v_debayer_pattern=new VarStringEnum("de-bayer pattern",colorFilterToString(DC1394_COLOR_FILTER_MIN)));
  for (int i = DC1394_COLOR_FILTER_MIN; i <= DC1394_COLOR_FILTER_MAX; i++) {
    v_debayer_pattern->addItem(colorFilterToString((dc1394color_filter_t)i));
  }
  conversion_settings->addChild(v_debayer_method=new VarStringEnum("de-bayer method",bayerMethodToString(DC1394_BAYER_METHOD_MIN)));
  for (int i = DC1394_BAYER_METHOD_MIN; i <= DC1394_BAYER_METHOD_MAX; i++) {
    v_debayer_method->addItem(bayerMethodToString((dc1394bayer_method_t)i));
  }
  conversion_settings->addChild(v_debayer_y16=new VarInt("de-bayer y16 bits",16));
  dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_fps              = new VarInt("framerate",60));
  capture_settings->addChild(v_width            = new VarInt("width",780));
  capture_settings->addChild(v_height           = new VarInt("height",580));
  capture_settings->addChild(v_left            = new VarInt("left",0));
  capture_settings->addChild(v_top           = new VarInt("top",0));
  capture_settings->addChild(v_colormode        = new VarStringEnum("capture mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB16));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW8));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW16));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO8));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO16));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV411));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV444));
  capture_settings->addChild(v_format           = new VarStringEnum("capture format",captureModeToString(CAPTURE_MODE_MIN)));
  for (int i = CAPTURE_MODE_MIN; i <= CAPTURE_MODE_MAX; i++) {
    v_format->addItem(captureModeToString((CaptureMode)i));
  }
  capture_settings->addChild(v_use1394B         = new VarBool("use 1394B"  ,true));
  capture_settings->addChild(v_use_iso_800      = new VarBool("use ISO800", true));
  capture_settings->addChild(v_buffer_size      = new VarInt("ringbuffer size",4));

  //=======================DCAM PARAMETERS===========================
  dcam_parameters->addChild(P_BRIGHTNESS = new VarList("brightness"));
  P_BRIGHTNESS->addChild(new VarBool("enabled"));
  P_BRIGHTNESS->addChild(new VarBool("auto"));
  P_BRIGHTNESS->addChild(new VarTrigger("one-push","Auto!"));
  P_BRIGHTNESS->addChild(new VarInt("value"));
  P_BRIGHTNESS->addChild(new VarBool("use absolute"));
  P_BRIGHTNESS->addChild(new VarDouble("absolute value"));
    mvc_connect(P_BRIGHTNESS);

  dcam_parameters->addChild(P_EXPOSURE = new VarList("exposure"));
  P_EXPOSURE->addChild(new VarBool("enabled"));
  P_EXPOSURE->addChild(new VarBool("auto"));
  P_EXPOSURE->addChild(new VarTrigger("one-push","Auto!"));
  P_EXPOSURE->addChild(new VarInt("value"));
  P_EXPOSURE->addChild(new VarBool("use absolute"));
  P_EXPOSURE->addChild(new VarDouble("absolute value"));
    mvc_connect(P_EXPOSURE);

  dcam_parameters->addChild(P_SHARPNESS = new VarList("sharpness"));
  P_SHARPNESS->addChild(new VarBool("enabled"));
  P_SHARPNESS->addChild(new VarBool("auto"));
  P_SHARPNESS->addChild(new VarTrigger("one-push","Auto!"));
  P_SHARPNESS->addChild(new VarInt("value"));
  P_SHARPNESS->addChild(new VarBool("use absolute"));
  P_SHARPNESS->addChild(new VarDouble("absolute value"));
    mvc_connect(P_SHARPNESS);

  dcam_parameters->addChild(P_WHITE_BALANCE = new VarList("white balance"));
  P_WHITE_BALANCE->addChild(new VarBool("enabled"));
  P_WHITE_BALANCE->addChild(new VarBool("auto"));
  P_WHITE_BALANCE->addChild(new VarTrigger("one-push","Auto!"));
  P_WHITE_BALANCE->addChild(new VarInt("value U"));
  P_WHITE_BALANCE->addChild(new VarInt("value V"));
  P_WHITE_BALANCE->addChild(new VarBool("use absolute"));
  P_WHITE_BALANCE->addChild(new VarDouble("absolute value"));
    mvc_connect(P_WHITE_BALANCE);

  dcam_parameters->addChild(P_HUE = new VarList("hue"));
  P_HUE->addChild(new VarBool("enabled"));
  P_HUE->addChild(new VarBool("auto"));
  P_HUE->addChild(new VarTrigger("one-push","Auto!"));
  P_HUE->addChild(new VarInt("value"));
  P_HUE->addChild(new VarBool("use absolute"));
  P_HUE->addChild(new VarDouble("absolute value"));
    mvc_connect(P_HUE);

  dcam_parameters->addChild(P_SATURATION = new VarList("saturation"));
  P_SATURATION->addChild(new VarBool("enabled"));
  P_SATURATION->addChild(new VarBool("auto"));
  P_SATURATION->addChild(new VarTrigger("one-push","Auto!"));
  P_SATURATION->addChild(new VarInt("value"));
  P_SATURATION->addChild(new VarBool("use absolute"));
  P_SATURATION->addChild(new VarDouble("absolute value"));
    mvc_connect(P_SATURATION);

  dcam_parameters->addChild(P_GAMMA = new VarList("gamma"));
  P_GAMMA->addChild(new VarBool("enabled"));
  P_GAMMA->addChild(new VarBool("auto"));
  P_GAMMA->addChild(new VarTrigger("one-push","Auto!"));
  P_GAMMA->addChild(new VarInt("value"));
  P_GAMMA->addChild(new VarBool("use absolute"));
  P_GAMMA->addChild(new VarDouble("absolute value"));
    mvc_connect(P_GAMMA);

  dcam_parameters->addChild(P_SHUTTER = new VarList("shutter"));
  P_SHUTTER->addChild(new VarBool("enabled"));
  P_SHUTTER->addChild(new VarBool("auto"));
  P_SHUTTER->addChild(new VarTrigger("one-push","Auto!"));
  P_SHUTTER->addChild(new VarInt("value"));
  P_SHUTTER->addChild(new VarBool("use absolute"));
  P_SHUTTER->addChild(new VarDouble("absolute value"));
    mvc_connect(P_SHUTTER);

  dcam_parameters->addChild(P_GAIN = new VarList("gain"));
  P_GAIN->addChild(new VarBool("enabled"));
  P_GAIN->addChild(new VarBool("auto"));
  P_GAIN->addChild(new VarTrigger("one-push","Auto!"));
  P_GAIN->addChild(new VarInt("value"));
  P_GAIN->addChild(new VarBool("use absolute"));
  P_GAIN->addChild(new VarDouble("absolute value"));
    mvc_connect(P_GAIN);

  dcam_parameters->addChild(P_IRIS = new VarList("iris"));
  P_IRIS->addChild(new VarBool("enabled"));
  P_IRIS->addChild(new VarBool("auto"));
  P_IRIS->addChild(new VarTrigger("one-push","Auto!"));
  P_IRIS->addChild(new VarInt("value"));
  P_IRIS->addChild(new VarBool("use absolute"));
  P_IRIS->addChild(new VarDouble("absolute value"));
    mvc_connect(P_IRIS);

  dcam_parameters->addChild(P_FOCUS = new VarList("focus"));
  P_FOCUS->addChild(new VarBool("enabled"));
  P_FOCUS->addChild(new VarBool("auto"));
  P_FOCUS->addChild(new VarTrigger("one-push","Auto!"));
  P_FOCUS->addChild(new VarInt("value"));
  P_FOCUS->addChild(new VarBool("use absolute"));
  P_FOCUS->addChild(new VarDouble("absolute value"));
    mvc_connect(P_FOCUS);

  dcam_parameters->addChild(P_TEMPERATURE = new VarList("temperature"));
  P_TEMPERATURE->addChild(new VarBool("enabled"));
  P_TEMPERATURE->addChild(new VarBool("auto"));
  P_TEMPERATURE->addChild(new VarTrigger("one-push","Auto!"));
  P_TEMPERATURE->addChild(new VarInt("value"));
  P_TEMPERATURE->addChild(new VarBool("use absolute"));
  P_TEMPERATURE->addChild(new VarDouble("absolute value"));
    mvc_connect(P_TEMPERATURE);

  dcam_parameters->addChild(P_TRIGGER = new VarList("trigger"));
  P_TRIGGER->addChild(new VarBool("enabled"));
  P_TRIGGER->addChild(new VarBool("auto"));
  P_TRIGGER->addChild(new VarTrigger("one-push","Auto!"));
  P_TRIGGER->addChild(new VarInt("value"));
  P_TRIGGER->addChild(new VarBool("use absolute"));
  P_TRIGGER->addChild(new VarDouble("absolute value"));
    mvc_connect(P_TRIGGER);

  dcam_parameters->addChild(P_TRIGGER_DELAY = new VarList("trigger delay"));
  P_TRIGGER_DELAY->addChild(new VarBool("enabled"));
  P_TRIGGER_DELAY->addChild(new VarBool("auto"));
  P_TRIGGER_DELAY->addChild(new VarTrigger("one-push","Auto!"));
  P_TRIGGER_DELAY->addChild(new VarInt("value"));
  P_TRIGGER_DELAY->addChild(new VarBool("use absolute"));
  P_TRIGGER_DELAY->addChild(new VarDouble("absolute value"));
    mvc_connect(P_TRIGGER_DELAY);

  dcam_parameters->addChild(P_WHITE_SHADING = new VarList("white shading"));
  P_WHITE_SHADING->addChild(new VarBool("enabled"));
  P_WHITE_SHADING->addChild(new VarBool("auto"));
  P_WHITE_SHADING->addChild(new VarTrigger("one-push","Auto!"));
  P_WHITE_SHADING->addChild(new VarInt("value R"));
  P_WHITE_SHADING->addChild(new VarInt("value G"));
  P_WHITE_SHADING->addChild(new VarInt("value B"));
  P_WHITE_SHADING->addChild(new VarBool("use absolute"));
  P_WHITE_SHADING->addChild(new VarDouble("absolute value"));
    mvc_connect(P_WHITE_SHADING);

  dcam_parameters->addChild(P_FRAME_RATE = new VarList("frame rate"));
  P_FRAME_RATE->addChild(new VarBool("enabled"));
  P_FRAME_RATE->addChild(new VarBool("auto"));
  P_FRAME_RATE->addChild(new VarTrigger("one-push","Auto!"));
  P_FRAME_RATE->addChild(new VarInt("value"));
  P_FRAME_RATE->addChild(new VarBool("use absolute"));
  P_FRAME_RATE->addChild(new VarDouble("absolute value"));
  /*P_WHITE_SHADING->addChild(new VarInt("value R"));
  P_WHITE_SHADING->addChild(new VarInt("value G"));
  P_WHITE_SHADING->addChild(new VarInt("value B"));
  P_WHITE_SHADING->addChild(new VarBool("use absolute"));
  P_WHITE_SHADING->addChild(new VarDouble("absolute value"));*/

  vector<VarType *> v=dcam_parameters->getChildren();
  for (unsigned int i=0;i<v.size();i++) {
    if (v[i]->getType()==VARTYPE_ID_LIST) {
       VarBool * temp;
       ((VarList *)v[i])->addChild(temp = new VarBool("was_read",false));
      temp->addFlags(VARTYPE_FLAG_HIDDEN);
    }
  }

    mvc_connect(P_FRAME_RATE);

    mutex.unlock();
}

  void CaptureDC1394v2::mvc_connect(VarList * group) {
    vector<VarType *> v=group->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
      connect(v[i],SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
    }
    connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
  }

  void CaptureDC1394v2::changed(VarType * group) {
    if (group->getType()==VARTYPE_ID_LIST) {
      writeParameterValues( (VarList *)group );
      readParameterValues( (VarList *)group );
    }
  }


void CaptureDC1394v2::readAllParameterValues() {
  vector<VarType *> v=dcam_parameters->getChildren();
  for (unsigned int i=0;i<v.size();i++) {
    if (v[i]->getType()==VARTYPE_ID_LIST) {
      readParameterValues((VarList *)v[i]);
    }
  }
}

void CaptureDC1394v2::readAllParameterProperties()
{
  vector<VarType *> v=dcam_parameters->getChildren();
  for (unsigned int i=0;i<v.size();i++) {
    if (v[i]->getType()==VARTYPE_ID_LIST) {
      readParameterProperty((VarList *)v[i]);
    }
  }
}

void CaptureDC1394v2::writeAllParameterValues() {
  vector<VarType *> v=dcam_parameters->getChildren();
  for (unsigned int i=0;i<v.size();i++) {
    if (v[i]->getType()==VARTYPE_ID_LIST) {
      writeParameterValues((VarList *)v[i]);
    }
  }
}

void CaptureDC1394v2::readParameterValues(VarList * item) {
    mutex.lock();
  dc1394bool_t dcb;
  bool valid=true;
  dc1394feature_t feature=getDC1394featureEnum(item,valid);
  if (valid==false) {
    printf("INVALID FEATURE: %s\n",item->getName().c_str());
    mutex.unlock();
    return;
  }
  VarDouble * vabs=0;
  VarInt * vint=0;
  VarBool * vuseabs=0;
  VarBool * venabled=0;
  VarBool * vauto=0;
  VarBool * vwasread=0;
  VarTrigger * vtrigger=0;
  VarInt * vint2=0;
  VarInt * vint3=0;

  bool has_abs=false;

  vector<VarType *> children=item->getChildren();
  for (unsigned int i=0;i<children.size();i++) {
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="was_read") vwasread=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="auto") vauto=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="use absolute") vuseabs=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    if (children[i]->getType()==VARTYPE_ID_DOUBLE && children[i]->getName()=="absolute value") vabs=(VarDouble *)children[i];
    if (children[i]->getType()==VARTYPE_ID_TRIGGER && children[i]->getName()=="one-push") vtrigger=(VarTrigger *)children[i];
    if (feature==DC1394_FEATURE_WHITE_BALANCE) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value U") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value V") vint2=(VarInt *)children[i];
    }
    if (feature==DC1394_FEATURE_WHITE_SHADING) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value R") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value G") vint2=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value B") vint3=(VarInt *)children[i];
    }
  }

  if (dc1394_feature_is_present(camera,feature,&dcb) == DC1394_SUCCESS) {
    if (dcb==false) {
      //feature doesn't exist
      if (vwasread!=0) vwasread->setBool(false);
    } else {
      //check for switchability:
      if (vwasread!=0) vwasread->setBool(true);
      if (dc1394_feature_is_switchable(camera,feature,&dcb) == DC1394_SUCCESS) {
        if (dcb==true) {
          dc1394switch_t sw;
          if (dc1394_feature_get_power(camera,feature,&sw) == DC1394_SUCCESS && sw==DC1394_ON) {
            venabled->setBool(true);
          } else {
            venabled->setBool(false);
          }
        } else {
          venabled->setBool(true);
        }
      } else {
        fprintf(stderr,"DC1394 ERROR: FAILURE DURING SWITCHABLE READOUT: %s\n", item->getName().c_str());
      }
      //feature exists
      if (dc1394_feature_is_readable(camera,feature,&dcb) == DC1394_SUCCESS && dcb==true) {
        //feature is readable
        if (feature==DC1394_FEATURE_WHITE_SHADING) {
          unsigned int val1;
          unsigned int val2;
          unsigned int val3;
          if (dc1394_feature_whiteshading_get_value(camera,&val1,&val2,&val3) == DC1394_SUCCESS) {
            vint->setInt(val1);
            if (vint2!=0) vint2->setInt(val2);
            if (vint3!=0) vint3->setInt(val3);
          }
        } else if (feature==DC1394_FEATURE_WHITE_BALANCE) {
          unsigned int val1;
          unsigned int val2;
          if (dc1394_feature_whitebalance_get_value(camera,&val1,&val2) == DC1394_SUCCESS) {
            vint->setInt(val1);
            if (vint2!=0) vint2->setInt(val2);
          }
        } else {
          unsigned int val;
          if (dc1394_feature_get_value(camera,feature,&val) == DC1394_SUCCESS) {
            vint->setInt(val);
          }
        }

        dc1394feature_mode_t mode;
        if (dc1394_feature_get_mode(camera,feature,&mode) ==DC1394_SUCCESS) {
          if (mode==DC1394_FEATURE_MODE_AUTO) {
            vauto->setBool(true);
          } else if (mode==DC1394_FEATURE_MODE_MANUAL) {
            vauto->setBool(false);
          }
        }


        //------------ABSOLUTE CONTROL READOUT:
        if (dc1394_feature_has_absolute_control(camera,feature,&dcb) ==DC1394_SUCCESS && dcb==true) {
          has_abs=true;
          dc1394switch_t abs_sw;
          if (dc1394_feature_get_absolute_control(camera,feature,&abs_sw) ==DC1394_SUCCESS) {
            vuseabs->setBool( abs_sw==DC1394_ON);
          }

          float valf;
          if (dc1394_feature_get_absolute_value(camera,feature,&valf) == DC1394_SUCCESS) {
              vabs->setDouble((double)valf);
          }
        } else {
          vuseabs->setBool(false);
        }
      }
    }




    //dc1394feature_mode_t camera_mode;
    //dc1394_feature_get_mode(camera, feature, &camera_mode);

    //dc1394bool_t auto_mode= ((camera_mode == DC1394_FEATURE_MODE_AUTO) ? DC1394_TRUE : DC1394_FALSE);
    //dc1394bool_t manual_mode= ((camera_mode == DC1394_FEATURE_MODE_MANUAL) ? DC1394_TRUE : DC1394_FALSE);

    dc1394feature_modes_t all_modes;
    bool has_manual_mode=false;
    bool has_auto_mode=false;
    bool has_one_push=false;
    if (dc1394_feature_get_modes(camera, feature, &all_modes) == DC1394_SUCCESS) {
      for (unsigned int i = 0; i < all_modes.num ; i++) {
        if (all_modes.modes[i]==DC1394_FEATURE_MODE_MANUAL) has_manual_mode=true;
        if (all_modes.modes[i]==DC1394_FEATURE_MODE_AUTO ) has_auto_mode=true;
        if (all_modes.modes[i]==DC1394_FEATURE_MODE_ONE_PUSH_AUTO ) has_one_push=true;
      }
    }

    //update render flags:
    if (venabled->getBool() && vauto->getBool()==false) {
      if (has_one_push) {
          vtrigger->removeFlags( VARTYPE_FLAG_READONLY);
          vtrigger->addFlags( VARTYPE_FLAG_PERSISTENT);
        }
      if (has_abs) {
        vuseabs->removeFlags( VARTYPE_FLAG_READONLY);
      } else {
        vuseabs->addFlags( VARTYPE_FLAG_READONLY);
      }
      if (has_abs && vuseabs->getBool()) {
        vint->addFlags( VARTYPE_FLAG_READONLY );
        if (vint2!=0) vint2->addFlags( VARTYPE_FLAG_READONLY );
        if (vint3!=0) vint3->addFlags( VARTYPE_FLAG_READONLY );
        if (vabs!=0) vabs->removeFlags( VARTYPE_FLAG_READONLY );
      } else {
        vint->removeFlags( VARTYPE_FLAG_READONLY );
        if (vint2!=0) vint2->removeFlags( VARTYPE_FLAG_READONLY );
        if (vint3!=0) vint3->removeFlags( VARTYPE_FLAG_READONLY );
        if (vabs!=0) vabs->addFlags( VARTYPE_FLAG_READONLY );
      }
      if (venabled->getBool()) {
        if (has_auto_mode && has_manual_mode) {
          vauto->removeFlags( VARTYPE_FLAG_READONLY );
        } else {
          vauto->addFlags( VARTYPE_FLAG_READONLY );
        }
      } else {
        vauto->addFlags( VARTYPE_FLAG_READONLY );
      }
    } else {
      if (has_one_push) {
        vtrigger->addFlags( VARTYPE_FLAG_READONLY);
      }
      vint->addFlags( VARTYPE_FLAG_READONLY );
      if (vint2!=0) vint2->addFlags( VARTYPE_FLAG_READONLY );
      if (vint3!=0) vint3->addFlags( VARTYPE_FLAG_READONLY );

      if (vabs!=0) vabs->addFlags( VARTYPE_FLAG_READONLY );
      vuseabs->addFlags( VARTYPE_FLAG_READONLY);
      if (venabled->getBool()) {
        if (has_auto_mode && has_manual_mode) {
          vauto->removeFlags( VARTYPE_FLAG_READONLY );
        } else {
          vauto->addFlags( VARTYPE_FLAG_READONLY );
        }
      } else {
        vauto->addFlags( VARTYPE_FLAG_READONLY );
      }
    }

  } else {
    fprintf(stderr,"DC1394 ERROR: FAILURE TO READ SETTING: %s\n", item->getName().c_str());
  }


    mutex.unlock();
}

void CaptureDC1394v2::writeParameterValues(VarList * item) {
    mutex.lock();
  dc1394bool_t dcb;
  bool valid=true;
  dc1394feature_t feature=getDC1394featureEnum(item,valid);
  if (valid==false) {
    mutex.unlock();
    return;
  }
  VarDouble * vabs=0;
  VarInt * vint=0;
  VarBool * vuseabs=0;
  VarBool * venabled=0;
  VarBool * vauto=0;
  VarInt * vint2=0;
  VarInt * vint3=0;
  VarBool * vwasread=0;
  VarTrigger * vtrigger=0;

  vector<VarType *> children=item->getChildren();
  for (unsigned int i=0;i<children.size();i++) {
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="was_read") vwasread=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="auto") vauto=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="use absolute") vuseabs=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    if (children[i]->getType()==VARTYPE_ID_DOUBLE && children[i]->getName()=="absolute value") vabs=(VarDouble *)children[i];
    if (children[i]->getType()==VARTYPE_ID_TRIGGER && children[i]->getName()=="one-push") vtrigger=(VarTrigger *)children[i];
    if (feature==DC1394_FEATURE_WHITE_BALANCE) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value U") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value V") vint2=(VarInt *)children[i];
    }
    if (feature==DC1394_FEATURE_WHITE_SHADING) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value R") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value G") vint2=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value B") vint3=(VarInt *)children[i];
    }
  }

  if (vwasread!=0 && vwasread->getBool()==true) {
  //new: only apply parameters which were previously read from the camera
    if (dc1394_feature_is_present(camera,feature,&dcb) == DC1394_SUCCESS) {
      if (dcb==false) {
        //feature doesn't exist
      } else {
        //check for switchability:
        if (dc1394_feature_is_switchable(camera,feature,&dcb) ==DC1394_SUCCESS && dcb==true) {
          dc1394_feature_set_power(camera,feature,(venabled->getBool() ? DC1394_ON : DC1394_OFF));
        }


        //feature exists
        if (dc1394_feature_is_readable(camera,feature,&dcb) == DC1394_SUCCESS && dcb==true) {
          if (vtrigger!=0 && vtrigger->getCounter() > 0) {
            vtrigger->resetCounter();
            printf("ONE PUSH for %s!\n",item->getName().c_str());
            dc1394_feature_set_mode(camera,feature,DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
            dc1394_feature_set_mode(camera,feature,vauto->getBool() ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL);
          } else {
            dc1394_feature_set_mode(camera,feature,vauto->getBool() ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL);
            if (vauto->getBool()==false && vabs->getBool()==false) {
              //feature is readable
              if (feature==DC1394_FEATURE_WHITE_SHADING) {
                if (vint2!=0 && vint3!=0) {
                  dc1394_feature_whiteshading_set_value(camera,(unsigned int) (vint->getInt()),(unsigned int) (vint2->getInt()),(unsigned int) (vint3->getInt()));
                }
              } else if (feature==DC1394_FEATURE_WHITE_BALANCE) {
                if (vint2!=0) {
                  dc1394_feature_whitebalance_set_value(camera,(unsigned int) (vint->getInt()),(unsigned int) (vint2->getInt()));
                }
              } else {
                dc1394_feature_set_value(camera,feature, (unsigned int) (vint->getInt()));
              }
            }
          }

          //------------ABSOLUTE CONTROL READOUT:
          if (dc1394_feature_has_absolute_control(camera,feature,&dcb) ==DC1394_SUCCESS && dcb==true) {
            dc1394_feature_set_absolute_control(camera,feature,vuseabs->getBool() ? DC1394_ON : DC1394_OFF);
            if (vuseabs->getBool() == true && vauto->getBool()==false) {
              dc1394_feature_set_absolute_value(camera,feature,(float)vabs->getDouble());
            }
          }
        }
      }
    }
  } else {
    //printf("==============Not writing parameter %s!===========\n",item->getName().c_str());
  }
    mutex.unlock();
}

void CaptureDC1394v2::readParameterProperty(VarList * item) {
    mutex.lock();
  bool debug=false;
  dc1394bool_t dcb;
  bool valid=true;
  dc1394feature_t feature=getDC1394featureEnum(item,valid);
  if (valid==false) {
    fprintf(stderr,"ERROR: INVALID PROPERTY ENCOUNTERED DURING READOUT: %s\n", item->getName().c_str());
    mutex.unlock();
    return;
  }
  VarDouble * vabs=0;
  VarInt * vint=0;
  VarBool * vuseabs=0;
  VarBool * venabled=0;
  VarBool * vauto=0;
  VarInt * vint2=0;
  VarInt * vint3=0;
  VarTrigger * vtrigger=0;

  vector<VarType *> children=item->getChildren();
  for (unsigned int i=0;i<children.size();i++) {
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="auto") vauto=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="use absolute") vuseabs=(VarBool *)children[i];
    if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    if (children[i]->getType()==VARTYPE_ID_DOUBLE && children[i]->getName()=="absolute value") vabs=(VarDouble *)children[i];
    if (children[i]->getType()==VARTYPE_ID_TRIGGER && children[i]->getName()=="one-push") vtrigger=(VarTrigger *)children[i];
    if (feature==DC1394_FEATURE_WHITE_BALANCE) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value U") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value V") vint2=(VarInt *)children[i];
    }
    if (feature==DC1394_FEATURE_WHITE_SHADING) {
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value R") vint=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value G") vint2=(VarInt *)children[i];
      if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value B") vint3=(VarInt *)children[i];
    }
  }

  if (dc1394_feature_is_present(camera,feature,&dcb) == DC1394_SUCCESS) {
    if (dcb==false) {
      if (debug) fprintf(stderr,"DC1394 PROP FEATURE IS *NOT* PRESENT: %s\n", item->getName().c_str());
      //feature doesn't exist
      item->addFlags(VARTYPE_FLAG_READONLY|VARTYPE_FLAG_HIDE_CHILDREN);
    } else {
      if (debug) fprintf(stderr,"DC1394 PROP FEATURE IS PRESENT: %s\n", item->getName().c_str());
      item->removeFlags(VARTYPE_FLAG_READONLY|VARTYPE_FLAG_HIDE_CHILDREN);
      //check for switchability:
      if (dc1394_feature_is_switchable(camera,feature,&dcb) ==DC1394_SUCCESS) {
        if (dcb==true) {
          venabled->removeFlags( VARTYPE_FLAG_READONLY);
        } else {
          venabled->setBool(true);
          venabled->addFlags( VARTYPE_FLAG_READONLY);
        }
      } else {
         fprintf(stderr,"DC1394 PROP READOUT ERROR - SWITCHABLE: %s\n", item->getName().c_str());
      }

      //feature exists
      if (dc1394_feature_is_readable(camera,feature,&dcb) == DC1394_SUCCESS && dcb==true) {
        //get feature modes:
        bool has_manual_mode=false;
        bool has_one_push=false;
        dc1394feature_modes_t all_modes;
        if (dc1394_feature_get_modes(camera, feature, &all_modes) == DC1394_SUCCESS) {
          for (unsigned int i = 0; i < all_modes.num ; i++) {
            if (all_modes.modes[i]==DC1394_FEATURE_MODE_MANUAL) has_manual_mode=true;
            if (all_modes.modes[i]==DC1394_FEATURE_MODE_ONE_PUSH_AUTO) has_one_push=true;
          }
        }
        //feature is readable
        if (has_one_push==false) {
          vtrigger->addFlags(VARTYPE_FLAG_READONLY);
        }
        if (has_manual_mode) {
          vint->removeFlags( VARTYPE_FLAG_READONLY );
          if (vint2!=0) vint2->removeFlags( VARTYPE_FLAG_READONLY );
          if (vint3!=0) vint3->removeFlags( VARTYPE_FLAG_READONLY );
          vauto->removeFlags( VARTYPE_FLAG_READONLY );
        } else {
          vint->addFlags( VARTYPE_FLAG_READONLY );
          if (vint2!=0) vint2->addFlags( VARTYPE_FLAG_READONLY );
          if (vint3!=0) vint3->addFlags( VARTYPE_FLAG_READONLY );
          vauto->addFlags( VARTYPE_FLAG_READONLY );
        }

        //------------ABSOLUTE CONTROL READOUT:
        if (dc1394_feature_has_absolute_control(camera,feature,&dcb) ==DC1394_SUCCESS && dcb==true) {
          vabs->removeFlags( VARTYPE_FLAG_READONLY );
          vuseabs->removeFlags( VARTYPE_FLAG_READONLY);
          float minv,maxv;
          if (dc1394_feature_get_absolute_boundaries(camera,feature,&minv,&maxv)==DC1394_SUCCESS) {
            vabs->setMin((double)minv);
            vabs->setMax((double)maxv);
          }
        } else {
          vabs->addFlags( VARTYPE_FLAG_READONLY );
          vuseabs->addFlags( VARTYPE_FLAG_READONLY );
        }


          //-----------STANDARD VALUE READOUT:
          unsigned int minv,maxv;
          if (dc1394_feature_get_boundaries(camera,feature,&minv,&maxv)==DC1394_SUCCESS) {
            vint->setMin(minv);
            vint->setMax(maxv);
          }



      }
    }
  } else {
    fprintf(stderr,"DC1394 PROP READOUT ERROR: %s\n", item->getName().c_str());
  }
    mutex.unlock();
}


CaptureDC1394v2::~CaptureDC1394v2()
{
  GlobalCaptureDC1394instanceManager::removeInstance();
  //FIXME: delete all of dcam_parameters children
}

bool CaptureDC1394v2::resetBus() {
    mutex.lock();

  //delete any old camera data structures:
  if (camera != 0) {
    dc1394_camera_free(camera);
    camera=0;
  }

  if (cam_list != 0) {
    dc1394_camera_free_list(cam_list);
    cam_list=0;
  }

  if (dc1394_camera_enumerate(dc1394_instance, &cam_list)!=DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: can't find cameras");
      mutex.unlock();
    return false;
  }

  if (cam_list==0) {
    fprintf(stderr,"CaptureDC1394v2 Error: Camera List was a null pointer");
      mutex.unlock();
    return false;
  }

  if (cam_list->num==0) {
    fprintf(stderr,"CaptureDC1394v2 Error: 0 cameras found");
      mutex.unlock();
    return false;
  }

  for (unsigned int i=0; i < cam_list->num ; i++) {
    camera = dc1394_camera_new(dc1394_instance, cam_list->ids[i].guid);
    dc1394_reset_bus(camera);
    dc1394_camera_free(camera);
    camera=0;
  }

  if (cam_list != 0) {
    dc1394_camera_free_list(cam_list);
    cam_list=0;
  }


  /*


  dc1394camera_t *local_camera, **local_cameras=NULL;
  uint32_t numCameras, i;

  // Find local_cameras
  int err=dc1394_find_cameras(&local_cameras, &numCameras);

  if (err!=DC1394_SUCCESS && err != DC1394_NO_CAMERA) {
    fprintf( stderr, "Unable to look for local_cameras\n\n"
             "On Linux, please check \n"
             "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded \n"
             "  - if you have read/write access to /dev/raw1394\n\n");
  }

  //-----------------------------------------------------------------------
  //  get the camera nodes and describe them as we find them
  //-----------------------------------------------------------------------
  if (numCameras<1) {
    fprintf(stderr, "no cameras found :(\n");
    return;
  }
  local_camera=local_cameras[0];

  // free the other local_cameras
  for (i=1;i<numCameras;i++)
    dc1394_free_camera(local_cameras[i]);
  free(local_cameras);

  printf ("Reseting bus...\n");
  dc1394_reset_bus (local_camera);

  dc1394_free_camera(local_camera);
  */
    mutex.unlock();
  return true;

}

bool CaptureDC1394v2::stopCapture() {

  if (isCapturing()) {
    readAllParameterValues();
  }
  cleanup();

  vector<VarType *> tmp = capture_settings->getChildren();
  for (unsigned int i=0; i < tmp.size();i++) {
    tmp[i]->removeFlags( VARTYPE_FLAG_READONLY );
  }
  dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
  return true;
}


void CaptureDC1394v2::cleanup()
{
    mutex.lock();

  if (camera!=0) {
    dc1394_capture_stop(camera);
    dc1394_video_set_transmission(camera,DC1394_OFF);
    dc1394_camera_free(camera);
  }
  camera=0;
  //TODO: cleanup/free any memory buffers.

  is_capturing=false;
    mutex.unlock();

}

/// This function converts a local dcam_parameters-manager variable ID
/// to a DC1394 feature enum
dc1394feature_t CaptureDC1394v2::getDC1394featureEnum(VarList * val, bool & valid)
{
  dc1394feature_t res=DC1394_FEATURE_MIN;
  if (val== P_BRIGHTNESS) {
    res= DC1394_FEATURE_BRIGHTNESS;
  } else if (val== P_EXPOSURE) {
    res= DC1394_FEATURE_EXPOSURE;
  } else if (val== P_SHARPNESS) {
    res= DC1394_FEATURE_SHARPNESS;
  } else if (val== P_WHITE_BALANCE) {
    res= DC1394_FEATURE_WHITE_BALANCE;
  } else if (val== P_HUE) {
    res= DC1394_FEATURE_HUE;
  } else if (val== P_SATURATION) {
    res= DC1394_FEATURE_SATURATION;
  } else if (val== P_GAMMA) {
    res= DC1394_FEATURE_GAMMA;
  } else if (val== P_SHUTTER) {
    res= DC1394_FEATURE_SHUTTER;
  } else if (val== P_GAIN) {
    res= DC1394_FEATURE_GAIN;
  } else if (val== P_IRIS) {
    res= DC1394_FEATURE_IRIS;
  } else if (val== P_FOCUS) {
    res= DC1394_FEATURE_FOCUS;
  } else if (val== P_TEMPERATURE) {
    res= DC1394_FEATURE_TEMPERATURE;
  } else if (val== P_TRIGGER) {
    res= DC1394_FEATURE_TRIGGER;
  } else if (val== P_TRIGGER_DELAY) {
    res= DC1394_FEATURE_TRIGGER_DELAY;
  } else if (val== P_WHITE_SHADING) {
    res= DC1394_FEATURE_WHITE_SHADING;
  } else if (val== P_FRAME_RATE) {
    res= DC1394_FEATURE_FRAME_RATE;
  } else {
    valid=false;
  }
  return res;
}

/// This function converts a DC1394 feature into a
/// dcam_parameters-manager variable ID
VarList * CaptureDC1394v2::getVariablePointer(dc1394feature_t val)
{
  VarList * res=0;
  switch (val) {
    case DC1394_FEATURE_BRIGHTNESS: res=P_BRIGHTNESS; break;
    case DC1394_FEATURE_EXPOSURE: res=P_EXPOSURE; break;
    case DC1394_FEATURE_SHARPNESS: res=P_SHARPNESS; break;
    case DC1394_FEATURE_WHITE_BALANCE: res=P_WHITE_BALANCE; break;
    case DC1394_FEATURE_HUE: res=P_HUE; break;
    case DC1394_FEATURE_SATURATION: res=P_SATURATION; break;
    case DC1394_FEATURE_GAMMA: res=P_GAMMA; break;
    case DC1394_FEATURE_SHUTTER: res=P_SHUTTER; break;
    case DC1394_FEATURE_GAIN: res=P_GAIN; break;
    case DC1394_FEATURE_IRIS: res=P_IRIS; break;
    case DC1394_FEATURE_FOCUS: res=P_FOCUS; break;
    case DC1394_FEATURE_TEMPERATURE: res=P_TEMPERATURE; break;
    case DC1394_FEATURE_TRIGGER: res=P_TRIGGER; break;
    case DC1394_FEATURE_TRIGGER_DELAY: res=P_TRIGGER_DELAY; break;
    case DC1394_FEATURE_WHITE_SHADING: res=P_WHITE_SHADING; break;
    case DC1394_FEATURE_FRAME_RATE: res=P_FRAME_RATE; break;
    default: res=0; break;
  }
  return res;
}

bool CaptureDC1394v2::startCapture()
{
    mutex.lock();
  //grab current parameters:
  cam_id=v_cam_bus->getInt();
  width=v_width->getInt();
  height=v_height->getInt();
  left=v_left->getInt();
  top=v_top->getInt();
  capture_format=Colors::stringToColorFormat(v_colormode->getString().c_str());
  int fps=v_fps->getInt();
  CaptureMode mode=stringToCaptureMode(v_format->getString().c_str());
  ring_buffer_size=v_buffer_size->getInt();
  bool use_1394B=v_use1394B->getBool();
  dc1394speed_t iso_speed=(v_use_iso_800->getBool() ? DC1394_ISO_SPEED_800 : DC1394_ISO_SPEED_400);

  //Check configuration parameters:
  if (fps <= 2 ) {
    dcfps=DC1394_FRAMERATE_1_875;
  } else if (fps <= 4) {
    dcfps=DC1394_FRAMERATE_3_75;
  } else if (fps <= 8) {
    dcfps=DC1394_FRAMERATE_7_5;
  } else if (fps <= 15) {
    dcfps=DC1394_FRAMERATE_15;
  } else if (fps <= 30) {
    dcfps=DC1394_FRAMERATE_30;
  } else if (fps <= 60) {
    dcfps=DC1394_FRAMERATE_60;
  } else if (fps <= 120) {
    dcfps=DC1394_FRAMERATE_120;
  } else if (fps <= 240) {
    dcfps=DC1394_FRAMERATE_240;
  } else {
    fprintf(stderr,"CaptureDC1394v2 Error: The library does not support framerates higher than 240 fps (does your camera?).");
      mutex.unlock();
    return false;
  }

  if (dc1394_camera_enumerate(dc1394_instance, &cam_list)!=DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: can't find cameras");
      mutex.unlock();
    return false;
  }

  qsort(cam_list->ids, cam_list->num, sizeof(dc1394camera_id_t), &compareCameraGUID);

  if (cam_list==0) {
    fprintf(stderr,"CaptureDC1394v2 Error: Camera List was a null pointer");
      mutex.unlock();
    return false;
  }

  if (cam_list->num==0) {
    fprintf(stderr,"CaptureDC1394v2 Error: 0 cameras found");
      mutex.unlock();
    return false;
  }

  if (cam_id >= cam_list->num) {
    fprintf(stderr,"CaptureDC1394v2 Error: no camera found with index %d. Max index is: %d\n",cam_id,cam_list->num-1);
      mutex.unlock();
    return false;
  }

  //free up other cameras:
  //for (unsigned i=0;i<cam_list->num;i++) {
  //  if (i!=camID) dc1394_free_camera(cameras[i]);
  //}

  camera = dc1394_camera_new(dc1394_instance, cam_list->ids[cam_id].guid);

  // Acquire free iso channel on firewire bus; allows to run two cameras on one bus
  int channel;
  if(dc1394_iso_allocate_channel(camera, 0xffff , &channel) != DC1394_SUCCESS)
    printf("CaptureDC1394v2 Info: Could not allocate channel\n");
  else {
    if(dc1394_video_set_iso_channel(camera, channel) != DC1394_SUCCESS)
      printf("CaptureDC1394v2 Info: Could not set channel\n");
    else
      printf("CaptureDC1394vc Info: got iso channel: %d\n", channel);
  }

  //disable any previous activity on that camera:
  dc1394_capture_stop(camera);
  dc1394_video_set_transmission(camera,DC1394_OFF);


  //dc1394_reset_bus(camera);
  //dc1394_camera_reset(camera);

  if (dc1394_video_set_operation_mode(camera, use_1394B ? DC1394_OPERATION_MODE_1394B : DC1394_OPERATION_MODE_LEGACY) != DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394 Error: unable to set operation mode. Resuming nevertheless...\n");
  }

  dc1394operation_mode_t opmode_tmp;
  if (dc1394_video_get_operation_mode(camera,&opmode_tmp) == DC1394_SUCCESS) {
    if (opmode_tmp==DC1394_OPERATION_MODE_1394B) {
      printf("CaptureDC1394v2 Info: using 1394B\n");
    } else {
      printf("CaptureDC1394v2 Info: using legacy 1394A\n");
    }
  }

  //setup camera for capture
  if(dc1394_feature_get_all(camera, &features) !=DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: unable to get feature set\n");
  } else {
    //dc1394_print_feature_set(&features);
  }



  if (dc1394_video_set_iso_speed(camera, iso_speed) !=  DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: unable to set ISO speed!\n");
      mutex.unlock();
    cleanup();
    return false;
  }

  dc1394speed_t iso_speed2;
  if (dc1394_video_get_iso_speed(camera, &iso_speed2) == DC1394_SUCCESS) {
    if (iso_speed!=iso_speed2) {
      fprintf(stderr,"CaptureDC1394v2 Warning: unable to set desired ISO speed!\n");
    }
    if (iso_speed2==DC1394_ISO_SPEED_800) {
      printf("CaptureDC1394v2 Info: running at ISO_SPEED_800!\n");
    } else {
      printf("CaptureDC1394v2 Info: running at ISO_SPEED_400!\n");
    }
  }

  dc1394video_modes_t supported_modes;
  bool know_modes=false;
  if (dc1394_video_get_supported_modes(camera,&supported_modes) == DC1394_SUCCESS) {
    know_modes=true;
  } else {
    fprintf(stderr,"CaptureDC1394v2 Warning: unable to query supported camera modes!\n");
  }

  bool native_unavailable = false;
  if (mode==CAPTURE_MODE_AUTO || mode==CAPTURE_MODE_NATIVE) {
    if (width==160 && height==120) {
      if (capture_format==COLOR_YUV444) {
        dcformat=DC1394_VIDEO_MODE_160x120_YUV444;
      } else {
        native_unavailable = true;
      }
    } else if (width==320 && height==240) {
      if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_320x240_YUV422;
      } else {
        native_unavailable = true;
      }
    } else if (width==640 && height==480) {
      if (capture_format==COLOR_YUV411) {
        dcformat=DC1394_VIDEO_MODE_640x480_YUV411;
      } else if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_640x480_YUV422;
      } else if (capture_format==COLOR_RGB8) {
        dcformat=DC1394_VIDEO_MODE_640x480_RGB8;
      } else if (capture_format==COLOR_MONO8) {
        dcformat=DC1394_VIDEO_MODE_640x480_MONO8;
      } else if (capture_format==COLOR_MONO16) {
        dcformat=DC1394_VIDEO_MODE_640x480_MONO16;
      } else {
        native_unavailable = true;
      }
    } else if (width==800 && height==600) {
      if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_800x600_YUV422;
      } else if (capture_format==COLOR_RGB8) {
        dcformat=DC1394_VIDEO_MODE_800x600_RGB8;
      } else if (capture_format==COLOR_MONO8) {
        dcformat=DC1394_VIDEO_MODE_800x600_MONO8;
      } else if (capture_format==COLOR_MONO16) {
        dcformat=DC1394_VIDEO_MODE_800x600_MONO16;
      } else {
        native_unavailable = true;
      }
    } else if (width==1024 && height==768) {
      if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_1024x768_YUV422;
      } else if (capture_format==COLOR_RGB8) {
        dcformat=DC1394_VIDEO_MODE_1024x768_RGB8;
      } else if (capture_format==COLOR_MONO8) {
        dcformat=DC1394_VIDEO_MODE_1024x768_MONO8;
      } else if (capture_format==COLOR_MONO16) {
        dcformat=DC1394_VIDEO_MODE_1024x768_MONO16;
      } else {
        native_unavailable = true;
      }
    } else if (width==1280 && height==960) {
      if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_1280x960_YUV422;
      } else if (capture_format==COLOR_RGB8) {
        dcformat=DC1394_VIDEO_MODE_1280x960_RGB8;
      } else if (capture_format==COLOR_MONO8) {
        dcformat=DC1394_VIDEO_MODE_1280x960_MONO8;
      } else if (capture_format==COLOR_MONO16) {
        dcformat=DC1394_VIDEO_MODE_1280x960_MONO16;
      } else {
        native_unavailable = true;
      }
    } else if (width==1600 && height==1200) {
      if (capture_format==COLOR_YUV422_UYVY) {
        dcformat=DC1394_VIDEO_MODE_1600x1200_YUV422;
      } else if (capture_format==COLOR_RGB8) {
        dcformat=DC1394_VIDEO_MODE_1600x1200_RGB8;
      } else if (capture_format==COLOR_MONO8) {
        dcformat=DC1394_VIDEO_MODE_1600x1200_MONO8;
      } else if (capture_format==COLOR_MONO16) {
        dcformat=DC1394_VIDEO_MODE_1600x1200_MONO16;
      } else {
        native_unavailable = true;
      }
    } else {
      native_unavailable = true;
    }
    if (native_unavailable==false && know_modes) {
      bool found_mode=false;
      for (unsigned int i=0;i<supported_modes.num;i++) {
        if (supported_modes.modes[i]==dcformat) {
          found_mode=true;
          break;
        }
      }
      if (found_mode==false) {
        native_unavailable=true;
      }
    }

    if (native_unavailable==true) {
      if (mode==CAPTURE_MODE_AUTO) {
        printf("CaptureDC1394v2 Info: Selected format/resolution not supported as FORMAT 0\n");
        printf("CaptureDC1394v2 Info: Selecting lowest available Format 7 mode\n");
        if (know_modes) {
          bool found=false;
          for (unsigned int i=0;i<supported_modes.num;i++) {
            if (supported_modes.modes[i] >= DC1394_VIDEO_MODE_FORMAT7_MIN && supported_modes.modes[i]<=DC1394_VIDEO_MODE_FORMAT7_MAX) {
              if (found==false || supported_modes.modes[i] < dcformat) {
                found=true;
                dcformat=supported_modes.modes[i];
              }
            }
          }
          if (found==false) {
            fprintf(stderr,"CaptureDC1394v2 Error: No Format 7 modes available!");
            fprintf(stderr,"CaptureDC1394v2 Error: Maybe try selecting a supported Format 0 resolution/framerate?");
              mutex.unlock();
            return false;
          }
        } else {
          dcformat=DC1394_VIDEO_MODE_FORMAT7_0;
        }

      } else {
        fprintf(stderr,"CaptureDC1394v2 Error: Selected color format/resolution not natively supported!");
        fprintf(stderr,"CaptureDC1394v2 Error: Maybe try switching to auto or a format7 mode.");
          mutex.unlock();
        return false;
      }
    }
  } else {
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_0) dcformat = DC1394_VIDEO_MODE_FORMAT7_0;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_1) dcformat = DC1394_VIDEO_MODE_FORMAT7_1;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_2) dcformat = DC1394_VIDEO_MODE_FORMAT7_2;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_3) dcformat = DC1394_VIDEO_MODE_FORMAT7_3;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_4) dcformat = DC1394_VIDEO_MODE_FORMAT7_4;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_5) dcformat = DC1394_VIDEO_MODE_FORMAT7_5;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_6) dcformat = DC1394_VIDEO_MODE_FORMAT7_6;
    if (mode==CAPTURE_MODE_FORMAT_7_MODE_7) dcformat = DC1394_VIDEO_MODE_FORMAT7_7;
  }


  if (know_modes) {
    //check whether capture mode is supported:
    bool found=false;
    for (unsigned int i=0;i<supported_modes.num;i++) {
      if (supported_modes.modes[i]==dcformat) {
        found=true;
        break;
      }
    }
    if (found==false) {
      fprintf(stderr,"CaptureDC1394v2 Error: Selected mode (format/resolution/framerate) is not supported by camera!\n");
        mutex.unlock();
      cleanup();
      return false;
    } else {
    }
  }


  if (dc1394_video_set_mode(camera,dcformat) !=  DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: unable to set capture mode\n");
      mutex.unlock();
    cleanup();
    return false;
  }

  //very mode:
  dc1394video_mode_t check_mode;
  if (dc1394_video_get_mode(camera,&check_mode) ==  DC1394_SUCCESS) {
    if (check_mode!=dcformat) {
      fprintf(stderr,"CaptureDC1394v2 Error: Unable to set selected mode\n");
        mutex.unlock();
      cleanup();
      return false;
    }
  }

  if (dcformat >= DC1394_VIDEO_MODE_FORMAT7_MIN && dcformat <= DC1394_VIDEO_MODE_FORMAT7_MAX) {
    dc1394color_coding_t color_coding=DC1394_COLOR_CODING_MONO8;
    if (capture_format==COLOR_YUV411) {
        color_coding=DC1394_COLOR_CODING_YUV411;
    } else if (capture_format==COLOR_YUV422_UYVY) {
        color_coding=DC1394_COLOR_CODING_YUV422;
    } else if (capture_format==COLOR_YUV444) {
        color_coding=DC1394_COLOR_CODING_YUV444;
    } else if (capture_format==COLOR_RGB8) {
        color_coding=DC1394_COLOR_CODING_RGB8;
    } else if (capture_format==COLOR_RGB16) {
        color_coding=DC1394_COLOR_CODING_RGB16;
    } else if (capture_format==COLOR_MONO8) {
        color_coding=DC1394_COLOR_CODING_MONO8;
    } else if (capture_format==COLOR_MONO16) {
        color_coding=DC1394_COLOR_CODING_MONO16;
    } else if (capture_format==COLOR_YUV411) {
        color_coding=DC1394_COLOR_CODING_YUV411;
    } else if (capture_format==COLOR_YUV411) {
        color_coding=DC1394_COLOR_CODING_YUV411;
    } else if (capture_format==COLOR_RAW8) {
        color_coding=DC1394_COLOR_CODING_RAW8;
    } else if (capture_format==COLOR_RAW16) {
        color_coding=DC1394_COLOR_CODING_RAW16;
    } else {
        fprintf(stderr,"CaptureDC1394v2 Error: System does not know selected Format 7 Color Coding Format\n");
          mutex.unlock();
        cleanup();
        return false;
    }

    if (  dc1394_format7_set_color_coding(camera, dcformat, color_coding) != DC1394_SUCCESS) {
      fprintf(stderr,"CaptureDC1394v2 Error: unable to set Format 7 Color Coding Format\n");
        mutex.unlock();
      cleanup();
      return false;
    }

    if (dc1394_format7_set_image_size(camera, dcformat, width, height) != DC1394_SUCCESS) {
      fprintf(stderr,"CaptureDC1394v2 Error: unable to set Format 7 image size\n");
        mutex.unlock();
      cleanup();
      return false;
    }

    if (dc1394_format7_set_image_position(camera, dcformat, left, top) != DC1394_SUCCESS) {
      fprintf(stderr,"CaptureDC1394v2 Error: unable to set Format 7 image size\n");
        mutex.unlock();
      cleanup();
      return false;
    }



    uint32_t unit=0;
    uint32_t maxp=0;

    dc1394_format7_get_packet_parameters(camera, dcformat, &unit, &maxp);

    //printf("unit size: %d     max size: %d\n",unit,maxp);
    //maximum frame rate (in fps) = 10^6 / (packets per frame x 125μs)
    //where the 125us is the frame transmission time.
    uint32_t tmpsize=0;
    if (dc1394_format7_get_packet_size(camera, dcformat, &tmpsize) == DC1394_SUCCESS) {
      printf("CaptureDC1394v2 Info: Previous Packet Size: %d\n",tmpsize);
    }

    uint32_t recommended_size=0;
    if (dc1394_format7_get_recommended_packet_size(camera, dcformat, &recommended_size) == DC1394_SUCCESS) {
      printf("CaptureDC1394v2 Info: Recommended Packet Size: %d\n",recommended_size);
    }

    uint64_t bytes=0;
    dc1394_format7_get_total_bytes(camera, dcformat, &bytes);
    //printf("bytes per frame: %d\n",bytes);

    uint32_t target_size=recommended_size;//;
    if (target_size!=0) {
      if (dc1394_format7_set_packet_size(camera, dcformat, recommended_size) != DC1394_SUCCESS) {
        fprintf(stderr,"CaptureDC1394v2 Error: unable to set Format 7 packet size / framerate!\n");

          mutex.unlock();
        cleanup();
        return false;
      }
    }

    tmpsize=0;
    if (dc1394_format7_get_packet_size(camera, dcformat, &tmpsize) == DC1394_SUCCESS) {
      printf("CaptureDC1394v2 Info: Final Packet Size: %d\n",tmpsize);
    }

  } else {

    dc1394framerates_t sup_framerates;
    if (dc1394_video_get_supported_framerates(camera,dcformat,&sup_framerates)== DC1394_SUCCESS) {
      bool found=false;
      for (unsigned int i=0;i<sup_framerates.num;i++) {
        if (sup_framerates.framerates[i]==dcfps) {
          found=true;
          break;
        }
      }
      if (found==false) {
        fprintf(stderr,"CaptureDC1394v2 Error: selected framerate is not supported!\n");
        mutex.unlock();
        cleanup();
        return false;
      }
    }

    if (dc1394_video_set_framerate(camera,dcfps) !=  DC1394_SUCCESS) {
      fprintf(stderr,"CaptureDC1394v2 Error: unable to set framerate!\n");
        mutex.unlock();
      cleanup();
      return false;
    }
  }


/*
  dc1394error_t dc1394_format7_get_packet_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *packet_size);

dc1394error_t dc1394_format7_set_packet_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t packet_size);

dc1394error_t dc1394_format7_get_recommended_packet_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *packet_size);
dc1394error_t dc1394_format7_get_packets_per_frame(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *ppf);
*/

  uint32_t bandwidth;
  if (dc1394_video_get_bandwidth_usage(camera, &bandwidth) == DC1394_SUCCESS) {
    if (bandwidth > 4915) {
      fprintf(stderr,"CaptureDC1394v2 Warning: Selected combination of video mode/format/resolution/framerate will likely exceed ");
      fprintf(stderr,"available bus bandwidth (available=4915, needed=%d). ",bandwidth);
      fprintf(stderr,"If capture hangs, then this is why!\n");
      fflush(stderr);
    }
  }

  //DC1394_CAPTURE_FLAGS_CHANNEL_ALLOC /*
  if (dc1394_capture_setup(camera, ring_buffer_size, DC1394_CAPTURE_FLAGS_DEFAULT ) != DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: unable to setup capture. Maybe selected combination of Format/Resolution is not supported?\n");
      mutex.unlock();
    cleanup();
    return false;
  }

  if (dc1394_video_set_transmission(camera,DC1394_ON) !=DC1394_SUCCESS) {
    fprintf(stderr,"CaptureDC1394v2 Error: Unable to start iso transmission on camera %d.\n",cam_id);
      mutex.unlock();
    cleanup();
    return false;
  }

  vector<VarType *> l = capture_settings->getChildren();
  for (unsigned int i = 0; i < l.size(); i++) {
    l[i]->addFlags(VARTYPE_FLAG_READONLY);
  }

  is_capturing=true;

  vector<VarType *> tmp = capture_settings->getChildren();
  for (unsigned int i=0; i < tmp.size();i++) {
    tmp[i]->addFlags( VARTYPE_FLAG_READONLY );
  }

  dcam_parameters->removeFlags( VARTYPE_FLAG_HIDE_CHILDREN );
    mutex.unlock();
  readAllParameterProperties();
  printf("CaptureDC1394v2 Info: Restoring Previously Saved Camera Parameters\n");
  writeAllParameterValues();
  readAllParameterValues();
  return true;
}

bool CaptureDC1394v2::copyFrame(const RawImage & src, RawImage & target)
{
  return convertFrame(src,target,src.getColorFormat());
}

bool CaptureDC1394v2::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
  return convertFrame(src,target,
                      Colors::stringToColorFormat(v_colorout->getSelection().c_str()),
                      v_debayer->getBool(),
                      stringToColorFilter(v_debayer_pattern->getSelection().c_str()),
                      stringToBayerMethod(v_debayer_method->getSelection().c_str()),
                      v_debayer_y16->getInt());
}


bool CaptureDC1394v2::convertFrame(const RawImage & src, RawImage & target, ColorFormat output_fmt,
                         bool debayer, dc1394color_filter_t bayer_format,dc1394bayer_method_t bayer_method, int y16bits)
{
    mutex.lock();
  ColorFormat src_fmt=src.getColorFormat();
  if (target.getData()==0) {
    //allocate target, if it does not exist yet
    target.allocate(output_fmt,src.getWidth(),src.getHeight());
  } else {
    target.ensure_allocation(output_fmt,src.getWidth(),src.getHeight());
    //target.setWidth(src.getWidth());
    //target.setHeight(src.getHeight());
    //target.setFormat(output_fmt);
  }
  target.setTime(src.getTime());
  target.setTimeCam ( src.getTimeCam() );
  if (output_fmt==src_fmt) {
    //just do a memcpy
    memcpy(target.getData(),src.getData(),src.getNumBytes());
  } else {
    //do some more fancy conversion
    if ((src_fmt==COLOR_MONO8 || src_fmt==COLOR_RAW8) && output_fmt==COLOR_RGB8) {
      //check whether to debayer or simply average to a grey rgb image
      if (debayer) {
        //de-bayer
        if ( dc1394_bayer_decoding_8bit( src.getData(), target.getData(), src.getWidth(), src.getHeight(), bayer_format, bayer_method) != DC1394_SUCCESS ) {
            mutex.unlock();
          return false;
        }
      } else {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_MONO8, 8);
        //Conversions::y2rgb (src.getData(), target.getData(), src.getNumPixels());
      }
    } else if ((src_fmt==COLOR_MONO16 || src_fmt==COLOR_RAW16)) {
      //check whether to debayer or simply average to a grey rgb image
      if (debayer && output_fmt==COLOR_RGB16) {
        //de-bayer
        if ( dc1394_bayer_decoding_16bit( (uint16_t *)src.getData(), (uint16_t *)target.getData(), src.getWidth(), src.getHeight(), bayer_format, bayer_method, y16bits) != DC1394_SUCCESS ) {
          fprintf(stderr,"Error in 16bit DC1394 Bayer Conversion");

            mutex.unlock();
          return false;
        }
      } else if (debayer==false && output_fmt==COLOR_RGB8) {

       dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       ((src_fmt==COLOR_MONO16) ? DC1394_COLOR_CODING_MONO16 : DC1394_COLOR_CODING_RAW16), y16bits);
        //Conversions::y162rgb (src.getData(), target.getData(), src.getNumPixels(), y16bits);
      } else {
      fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",Colors::colorFormatToString(src_fmt).c_str(),Colors::colorFormatToString(output_fmt).c_str());
        mutex.unlock();
      return false;
      }
    } else if (src_fmt==COLOR_YUV411 && output_fmt==COLOR_RGB8) {
      dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_YUV411, 8);
      //Conversions::uyyvyy2rgb (src.getData(), target.getData(), src.getNumPixels());
    } else if (src_fmt==COLOR_YUV422_UYVY && output_fmt==COLOR_RGB8) {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, DC1394_BYTE_ORDER_UYVY,
                       DC1394_COLOR_CODING_YUV422, 8);
    } else if (src_fmt==COLOR_YUV422_YUYV && output_fmt==COLOR_RGB8) {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, DC1394_BYTE_ORDER_YUYV,
                       DC1394_COLOR_CODING_YUV422, 8);
      //Conversions::uyvy2rgb (src.getData(), target.getData(), src.getNumPixels());
    } else if (src_fmt==COLOR_YUV444 && output_fmt==COLOR_RGB8) {
      dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_YUV444, 8);
      //Conversions::uyv2rgb (src.getData(), target.getData(), src.getNumPixels());
    } else {
      fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",Colors::colorFormatToString(src_fmt).c_str(),Colors::colorFormatToString(output_fmt).c_str());
        mutex.unlock();
      return false;
    }
  }
    mutex.unlock();
  return true;
}

RawImage CaptureDC1394v2::getFrame()
{
  mutex.lock();
  RawImage result;
  result.setColorFormat(capture_format);
  result.setWidth(width);
  result.setHeight(height);

  if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame)!=DC1394_SUCCESS) {
    fprintf (stderr, "CaptureDC1394v2 Error: Failed to capture from camera %d\n", cam_id);
    is_capturing=false;
    result.setData(0);
  } else {
    /*
    //
    // Returns DC1394_TRUE if the given frame (previously dequeued) has been
    // detected to be corrupt (missing data, corrupted data, overrun buffer, etc.).
    // Note that certain types of corruption may go undetected in which case
    // DC1394_FALSE will be returned.  The ability to detect corruption also
    // varies between platforms.  Note that corrupt frames still need to be
    // enqueued with dc1394_capture_enqueue() when no longer needed by the user.
    //
    if (dc1394_capture_is_frame_corrupt (camera, frame) == DC1394_TRUE) {
      printf("============================CORRUPT!\n"); fflush(stdout); exit(1);
    }*/
    result.setData(frame->image);
  }
  mutex.unlock();
  return result;
}

void CaptureDC1394v2::releaseFrame() {
    mutex.lock();
  if (dc1394_capture_enqueue (camera, frame) !=DC1394_SUCCESS) {
    fprintf (stderr, "CaptureDC1394v2 Error: Failed to release frame from camera %d\n", cam_id);
  }
    mutex.unlock();
}

string CaptureDC1394v2::getCaptureMethodName() const {
  return "DC1394";
}
