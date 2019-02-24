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

// Unset 'interface' from pylon/api_autoconf.h which conflicts with variables in ssl-vision
#undef interface

#ifndef VDATA_NO_QT
#include <QMutex>
#endif

class BaslerInitManager {
public:
	static void register_capture();
	static void unregister_capture();
//private:
	static int count;
};

#ifdef VDATA_NO_QT
class CaptureBasler: public CaptureInterface

#else
class CaptureBasler: public QObject, public CaptureInterface {
public:
	Q_OBJECT

    public slots:
    void changed(VarType * group);
private:
	QMutex mutex;
#endif

public:
#ifndef VDATA_NO_QT
	CaptureBasler(VarList* _settings=0, QObject* parent=0);
    void mvc_connect(VarList * group);
#else
	CaptureBasler(VarList * _settings=0);
#endif
	~CaptureBasler();

	bool startCapture();

	bool stopCapture();

	bool isCapturing() { return is_capturing; };

	RawImage getFrame();

	void releaseFrame();

	string getCaptureMethodName() const;

	bool copyAndConvertFrame(const RawImage & src, RawImage & target);

	void readAllParameterValues();

	void writeParameterValues(VarList* vars);

private:
	bool is_capturing;
	bool ignore_capture_failure;
	Pylon::CBaslerGigEInstantCamera* camera;
	Pylon::CGrabResultPtr grab_result;
	Pylon::CImageFormatConverter converter;
	int current_id;
  	unsigned char* last_buf;

  	VarList* vars;
  	VarInt* v_camera_id;
  	VarInt* v_balance_ratio_red;
  	VarInt* v_balance_ratio_green;
  	VarInt* v_balance_ratio_blue;
  	VarBool* v_auto_gain;
  	VarDouble* v_gain;
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
#ifdef OPENCV
  	static const double blur_sigma;
  	void gaussianBlur(RawImage& img);
    void contrast(RawImage& img, double factor);
    void sharpen(RawImage& img);
#endif
};

#endif
