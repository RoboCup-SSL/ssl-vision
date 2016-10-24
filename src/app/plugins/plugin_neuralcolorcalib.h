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
  \file    plugin_neuralcolorcalib.h
  \brief   C++ Interface: plugin_neuralcolorcalib
  \author J Angelo Gurzoni Jr <jgurzoni@ieee.org>
          Based on the PLuginColorCalyb by Stefan Zickler
*/
//========================================================================
#ifndef PLUGIN_NEURALCOLORCALIB_H
#define PLUGIN_NEURALCOLORCALIB_H
#include <QWidget>
#include <QPushButton>
#include "framedata.h"
#include "VarTypes.h"
#include "visionplugin.h"
#include "lut3d.h"
#include "neurowidget.h"
#include "opencv/ml.h" //OpenCV Machine Learning functions


class PluginNeuralColorCalib : public VisionPlugin {
protected:
    YUVLUT * lut;
    VarList * settings;
    NeuroWidget * lutw;
    LUTChannelMode mode;
    bool continuing_undo;

    CvANN_MLP * neuronet; //Creation of the ANN object (OpenCV)
    //variables used by OpenCV algorithm RunNeuralopenCV
    CvMat *realinput;
    CvMat *netout;
    std::vector<double> * neuro_trainingset; //size of training set.
    std::vector<double> * neuro_targset; //size of target set (outputs of the training set)

protected:
    void mouseEvent ( QMouseEvent * event, pixelloc loc );
    void CopyNeuraltoLUT();

public:
    bool isNetSet; //flag indicating if the network is trained
    int n_samples; //number of captured samples
    int epochs;

public:
    PluginNeuralColorCalib(FrameBuffer * _buffer, YUVLUT * _lut, LUTChannelMode _mode=LUTChannelMode_Numeric);
    ~PluginNeuralColorCalib();
    ProcessResult process(FrameData * data, RenderOptions * options);
    int TrainNeuralopenCV();
    int RunNeuralopenCV(double *input);
    void Clear();

    virtual VarList * getSettings();
    virtual string getName();
    virtual QWidget * getControlWidget();

    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );


};

#endif
