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
  \file    cameracalibwidget.h
  \brief   C++ Interface: CameraCalibrationWidget
  \author  OB, (C) 2008
*/
//========================================================================

#ifndef CAMERACALIBWIDGET_H
#define CAMERACALIBWIDGET_H
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <camera_calibration.h>

/*!
  \file    cameracalibwidget.h
  \brief   A GUI for calibrating the cameras overviewing the field
  \author  OB, (C) 2008
*/
class CameraCalibrationWidget : public QWidget {
Q_OBJECT
public:
    void focusInEvent ( QFocusEvent * event );
    CameraCalibrationWidget(CameraParameters &cp);
    ~CameraCalibrationWidget();
    
    CameraParameters& camera_parameters;
    
    bool getDetectEdges() {return detectEdges;}
    
    void resetDetectEdges() {detectEdges = false;}
    
    void set_slider_from_vars();    
    
  protected:
    QSlider* lineSearchCorridorWidthSlider;
    QLabel* lineSearchCorridorWidthLabelRight;
    QSlider* cameraHeightSlider;
    QLabel* cameraHeightLabelRight;
    QSlider* distortionSlider;
    QLabel* distortionLabelRight;
    bool detectEdges;

    public slots:
    void is_clicked_initial();
    void is_clicked_full();
    void is_clicked_reset();
    void edges_is_clicked();
    void cameraheight_slider_changed(int val);
    void distortion_slider_changed(int val);
    void line_search_slider_changed(int val);
};

#endif
