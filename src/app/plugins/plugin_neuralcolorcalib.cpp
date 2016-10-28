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
  \file    plugin_neuralcolorcalib.cpp
  \brief   C++ Implementation: plugin_neuralcolorcalib, based on the plugin_colorcalib
  \author  J. A. Gurzoni Jr and Gabriel Francischini, (C) 2011
*/
//========================================================================
#include "plugin_neuralcolorcalib.h"
#include <QStackedWidget>
#include <cmath>
#include "opencv/cxcore.h"
#include "opencv/ml.h" //OpenCV Machine Learning functions

const int sizeInputLayer = 3; //Number of neurons on the input layer. 3 because each example has Y, U and V values
const int sizeHidLayer = 20; //Number of neurons on the hidden layer
const int sizeOutLayer = 10; //Number of neurons on the output layer (also number of classes - in this case colors)


int PluginNeuralColorCalib::TrainNeuralopenCV() {
    CvANN_MLP_TrainParams netparams;
    CvTermCriteria term_crit;
    double * training;
    training= new double [n_samples * sizeInputLayer * sizeInputLayer];
    double * target;
    target= new double [n_samples * sizeOutLayer * sizeOutLayer];
    CvMat trainin;
    CvMat trainout;
    int interactions_ran=0;

    term_crit.max_iter =1000; //maximum number of epochs to train
    term_crit.epsilon  = 1e-5; //maximum tolerance to errors of the network during training
    term_crit.type = CV_TERMCRIT_ITER + CV_TERMCRIT_EPS; //indicates training stops either when performance or max. epochs is reached
    netparams=CvANN_MLP_TrainParams(term_crit, CvANN_MLP_TrainParams::BACKPROP,0.1,0.1); //specifies learning algorithm, learning rate and momentum

    for (unsigned int n=0; n<neuro_trainingset->size(); n++){
        training[n] = neuro_trainingset->at(n);
    }
    for (unsigned int n=0; n<neuro_targset->size(); n++){
        target[n] = neuro_targset->at(n);
    }

    trainin = cvMat(n_samples,3,CV_64F,training);
    trainout = cvMat(n_samples,sizeOutLayer,CV_64F,target);
    interactions_ran = neuronet->train (&trainin, &trainout,0,0,netparams,0); //call training functions of the object
    isNetSet=true;
    return interactions_ran;


}

int PluginNeuralColorCalib::RunNeuralopenCV(double *input) {
    double tmpout=0,maxout=0;
    int noutput=0;

    if (isNetSet){ //if network is not trained do nothing
            cvSetData(realinput,input,sizeof(double)*3); //inserts the input into the OpenCV matrix format
            neuronet->predict (realinput,netout); //call prediction (ask the output) of the object for a given input.

            for (int j=0; j<sizeOutLayer; j++){ //inserts the output in an output array
                tmpout=CV_MAT_ELEM(*netout,double,0,j);
                if  (tmpout > maxout){
                    noutput=j;
                    maxout=tmpout;
                }
            }

            return noutput;
    }
    else
        return -1;
}


PluginNeuralColorCalib::PluginNeuralColorCalib(FrameBuffer * _buffer, YUVLUT * _lut, LUTChannelMode _mode) : VisionPlugin(_buffer)
{
    mode=_mode;
    lut=_lut;
    lutw=0;
    settings=new VarList("Neural Calibrator");
    continuing_undo = false;

    int layersinfo[sizeInputLayer]={sizeInputLayer,sizeHidLayer,sizeOutLayer};
    CvMat layer_sizes= cvMat(1,3,CV_32SC1,layersinfo);
    neuronet = new CvANN_MLP(&layer_sizes,CvANN_MLP::SIGMOID_SYM,1,1);

    //variables used by OpenCV algorithm RunNeuralopenCV
    realinput = cvCreateMat(1,3,CV_64F);
    netout = cvCreateMat(1,sizeOutLayer,CV_64F);
    //

    isNetSet=false;
    n_samples=0;
    neuro_trainingset = new std::vector<double>;
    neuro_targset = new std::vector<double>;

}

ProcessResult PluginNeuralColorCalib::process(FrameData * data, RenderOptions * options) {

    if (lutw->pending_loadnn){
        if (isNetSet){
            this->CopyNeuraltoLUT();
            lutw->setStatusLabel("LUT Updated");
        }
        lutw->pending_loadnn = false;
    }else if(lutw->pending_train){
        if (n_samples > 0){
            this->epochs = this->TrainNeuralopenCV();
            lutw->setStatusLabel("NN trained in "+ QString::number(epochs)+" epochs");
        }else{
            lutw->setStatusLabel("Please capture samples.");
        }
        lutw->pending_train = false;
    }else if(lutw->pending_clear){
        this->Clear();
        lutw->setSamplesLabel(this->n_samples);
        lutw->setStatusLabel("NN training data cleared");
        lutw->pending_clear = false;
    }

    return ProcessingOk;
}


VarList * PluginNeuralColorCalib::getSettings() {
    return settings;
}

string PluginNeuralColorCalib::getName() {
    return "Neural Calib";
}

PluginNeuralColorCalib::~PluginNeuralColorCalib()
{
    delete settings;
}

QWidget * PluginNeuralColorCalib::getControlWidget() {
    if (lutw==0) {
        lutw=new NeuroWidget(lut,mode);
    //    connect(lutw->getGLLUTWidget(),SIGNAL(signalKeyPressEvent(QKeyEvent *)),this, SLOT(slotKeyPressEvent(QKeyEvent *)));
    }
    return (QWidget *)lutw;
}

void PluginNeuralColorCalib::mouseEvent( QMouseEvent * event, pixelloc loc) {
    QTabWidget* tabw = (QTabWidget*) lutw->parentWidget()->parentWidget();
    if (tabw->currentWidget() == lutw) {
        if (event->buttons()==Qt::LeftButton && lutw->getStateChannel() != -1) {
            FrameBuffer * rb=getFrameBuffer();
            if (rb!=0) {
                rb->lockRead();
                int idx=rb->curRead();
                FrameData * frame = rb->getPointer(idx);
                if (loc.x < frame->video.getWidth() && loc.y < frame->video.getHeight() && loc.x >=0 && loc.y >=0) {
                    if (frame->video.getWidth() > 1 && frame->video.getHeight() > 1) {
                        yuv color;
                        //if converting entire image then blanking is not needed
                        ColorFormat source_format=frame->video.getColorFormat();
                        if (source_format==COLOR_RGB8) {
                            //plain copy of data
                            rgbImage img(frame->video);
                            color=Conversions::rgb2yuv(img.getPixel(loc.x,loc.y));
                        } else if (source_format==COLOR_YUV444) {
                            yuvImage img(frame->video);
                            color=img.getPixel(loc.x,loc.y);
                        } else if (source_format==COLOR_YUV422_UYVY) {
                            uyvy color2 = *((uyvy*)(frame->video.getData() + (sizeof(uyvy) * (((loc.y * (frame->video.getWidth())) + loc.x) / 2))));
                            color.u=color2.u;
                            color.v=color2.v;
                            if ((loc.x % 2)==0) {
                                color.y=color2.y1;
                            } else {
                                color.y=color2.y2;
                            }
                        } else {
                            //blank it:
                            fprintf(stderr,"Unable to pick color from frame of format: %s\n",Colors::colorFormatToString(source_format).c_str());
                            fprintf(stderr,"Currently supported are rgb8, yuv444, and yuv422 (UYVY).\n");
                            fprintf(stderr,"(Feel free to add more conversions to plugin_neuralcolorcalib.cpp).\n");
                        }

                        neuro_trainingset->push_back( (double)color.y );
                        neuro_trainingset->push_back( (double)color.u );
                        neuro_trainingset->push_back( (double)color.v );

                        for (int j=0; j < sizeOutLayer; j++){
                            if (lutw->getStateChannel() == j)
                                neuro_targset->push_back( lutw->getStateChannel() );
                            else
                                neuro_targset->push_back( 0 );
                        }
                        this->n_samples++;
                        lutw->setSamplesLabel(this->n_samples);

                    }
                }
                rb->unlockRead();
            }
            event->accept();
        }

    }
    else
        event->ignore();
}

void PluginNeuralColorCalib::CopyNeuraltoLUT() {

    double input[3];
    int output;

    if (isNetSet)
    {
        lut->lock();
        for (int y = 0; y <= 255; y+= (0x1 << lut->X_SHIFT)) {
            for (int u = 0; u <= 255; u+= (0x1 << lut->Y_SHIFT)) {
                for (int v = 0; v <= 255; v+= (0x1 << lut->Z_SHIFT)) {
                    input[0]= (double)y;
                    input[1]= (double)u;
                    input[2]= (double)v;
                    output = RunNeuralopenCV(input);
                    if (output== -1)
                        output = 0;
                    lut->set(y,u,v,output); //Writes values to the camera's LUT table
                }
            }
        }
        lut->unlock();
    }
}

void PluginNeuralColorCalib::Clear() {
    n_samples=0;
    neuro_trainingset->resize(0);
    neuro_targset->resize(0);
    isNetSet=false;
}

void PluginNeuralColorCalib::keyPressEvent ( QKeyEvent * event ) {
    event->accept();
}

void PluginNeuralColorCalib::mousePressEvent ( QMouseEvent * event, pixelloc loc ) {
    mouseEvent(event,loc);
}

void PluginNeuralColorCalib::mouseReleaseEvent ( QMouseEvent * event, pixelloc loc ) {
    event->accept();
    //continuing_undo = false;
    //mouseEvent(event,loc);
}

void PluginNeuralColorCalib::mouseMoveEvent ( QMouseEvent * event, pixelloc loc ) {
    event->ignore();
}

