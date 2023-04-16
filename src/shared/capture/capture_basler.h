/*
 * capture_basler.h
 *
 *  Created on: Nov 21, 2016
 *      Author: root
 */

#ifndef CAPTURE_BASLER_H_
#define CAPTURE_BASLER_H_

#include "captureinterface.h"
#include <pylon/PylonIncludes.h>
#include <pylon/PylonBase.h>
#include <pylon/PylonImage.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <sys/time.h>
#include "VarTypes.h"
#include "TimeSync.h"

// Unset 'interface' from pylon/api_autoconf.h which conflicts with variables in ssl-vision
#undef interface

#include <QMutex>

class BaslerInitManager {
public:
	static void register_capture();
	static void unregister_capture();
//private:
	static int count;
};

class CaptureBasler: public QObject, public CaptureInterface {
public:
	Q_OBJECT

    public slots:
    void changed(VarType * group);
private:
	QMutex mutex;

public:
	CaptureBasler(VarList* _settings=0, int default_camera_id=0, QObject* parent=0);
    void mvc_connect(VarList * group);
	~CaptureBasler();

	bool startCapture();

	bool stopCapture();

	bool isCapturing() { return is_capturing; };

	RawImage getFrame();

	void releaseFrame();

	string getCaptureMethodName() const;

	bool copyAndConvertFrame(const RawImage & src, RawImage & target);

	void readAllParameterValues();

	void writeParameterValues(VarList* varList);

private:
	bool is_capturing;
        TimeSync timeSync;
        bool ignore_capture_failure;
	Pylon::CBaslerGigEInstantCamera* camera;
	Pylon::CBaslerGigEGrabResultPtr grab_result;
	Pylon::CImageFormatConverter converter;
	unsigned int current_id;
  	unsigned char* last_buf;

        // freq should always be 125 MHz for Basler-ace-1300-75gc
        int camera_frequency = 125e6;
  	VarList* vars;
  	VarInt* v_camera_id;
  	VarDouble* v_framerate;
  	VarInt* v_balance_ratio_red;
  	VarInt* v_balance_ratio_green;
  	VarInt* v_balance_ratio_blue;
  	VarBool* v_auto_gain;
  	VarInt* v_gain;
  	VarBool* v_gamma_enable;
  	VarDouble* v_gamma;
  	VarDouble* v_black_level;
  	VarBool* v_auto_exposure;
  	VarDouble* v_manual_exposure;
  	VarStringEnum* v_color_mode;

  	void resetCamera(unsigned int new_id);
  	bool _stopCapture();
  	bool _buildCamera();

    // A slight blur helps to reduce noise and improve color recognition.
  	static const double blur_sigma;
  	void gaussianBlur(RawImage& img);
    void contrast(RawImage& img, double factor);
    void sharpen(RawImage& img);
};

#endif
