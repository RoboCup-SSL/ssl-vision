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
  \file    plugin_dvr.h
  \brief   C++ Interface: plugin_dvr
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef plugin_dvr_H
#define plugin_dvr_H

#include <visionplugin.h>
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "VarTypes.h"
#include <QWidget>
//#include <QPushButton>
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <condition_variable>
#include <thread>
#include <optional>

#include "image.h"
#include "jog_dial.h"
#include "rawimage.h"
#include "timer.h"

class PluginDVR;
class DVRNonBlockingWriter;

class PluginDVRWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit PluginDVRWidget(PluginDVR * dvr, QWidget * parent = nullptr);
    JogDial * jog;
    QGroupBox * box_mode;
    QGroupBox * box_rec;
    QGroupBox * box_seek;
    QWidget * box_dvr;
    QHBoxLayout * layout_mode;
    QVBoxLayout * layout_main;
    QVBoxLayout * layout_dvr;
    QHBoxLayout * layout_rec;
    QHBoxLayout * layout_seek;
    QToolButton * btn_mode_record;
    QToolButton * btn_mode_pause;
    QToolButton * btn_mode_off;
    QToolButton * btn_pause_refresh;

    QToolButton * btn_rec_new;
    QToolButton * btn_rec_load;
    QToolButton * btn_rec_rec;
    QToolButton * btn_rec_continuous;
    QToolButton * btn_rec_save;

    QToolButton * btn_seek_front;
    QToolButton * btn_seek_frame_back;
    QToolButton * btn_seek_pause;
    QToolButton * btn_seek_frame_forward;
    QToolButton * btn_seek_end;
    QToolButton * btn_seek_play;
    QToolButton * btn_seek_live;
    QToolButton * btn_seek_wrap;

    QLabel * label_info;
};


class DVRFrame
{
  public:
  RawImage video;
  virtual ~DVRFrame();
  void getFromFrameData(FrameData * data);
};

class DVRStream
{
  //TODO: add partial memory buffering for long video streams.
  protected:
    QList<DVRFrame *> frames;
    QList<SSL_DetectionFrame *> detection_frames;
    int limit;
    int current;

  public:
    DVRStream();
    virtual ~DVRStream();
    int getLimit();
    void setLimit(int num_frames);
    bool loadStream(QString file);
    void newRecording(QString directory);
    void saveStream(QString directory);
    void clear();
    void appendFrame(FrameData * data, bool shift_stream_on_limit_exceed);
    void appendDetectionFrame(SSL_DetectionFrame * frame, bool shift_stream_on_limit_exceed);
    void seek(int frame);
    int getFrameCount();
    void advance(int frames, bool wrap);
    //void advance(double s, bool wrap);
    void advanceToMostRecent();
    int getCurrentFrameIndex();
    DVRFrame * getFrame(int i);
    DVRFrame * getCurrentFrame();

    SSL_DetectionFrame * getDetectionFrame(int i);
};


class DVRUtils {
 public:
  static void saveFrame(const DVRFrame& frame, const QString& dir, int index);
  static void saveDetectionFrame(const SSL_DetectionFrame& detection_frame, const QString& dir, int index);
};

struct DVRFrameData {
  std::unique_ptr<DVRFrame> frame_ptr;
  SSL_DetectionFrame detection_frame;
};


class DVRThreadSafeQueue
{
 protected:
  std::queue<DVRFrameData> queue;
  std::mutex queue_mutex;
  std::condition_variable broker;
  std::atomic<bool> running = {true};

 public:
  DVRFrameData dequeue();
  void enqueue(DVRFrameData data);
  void stop();
  int size();
};


class DVRNonBlockingWriter
{
 protected:
  const QString output_dir;
  std::thread writer_thread;
  std::atomic<bool> running = {true};
  DVRThreadSafeQueue data_buffer;
  int index{};
  void runWriterOnLoop();
  void write();

 public:
  explicit DVRNonBlockingWriter(QString output_dir);
  ~DVRNonBlockingWriter();

  void write(FrameData * frameData, SSL_DetectionFrame * frame);
};


/**
	@author Stefan Zickler
*/
class PluginDVR : public VisionPlugin
{
Q_OBJECT
protected slots:
  void slotModeToggled();
  void slotPauseRefresh();
  void slotSeekModeToggled();
  void slotSeekFrameFirst();
  void slotSeekFrameForward();
  void slotSeekFrameBack();
  void slotSeekFrameLast();
  void slotRecordContinuousToggled();
  void slotMovieNew();
  void slotMovieLoad();
  void slotMovieSave();
  void jogValueChanged(float val);

protected:

  enum DVRModeEnum {
    DVRModeOff,
    DVRModePause,
    DVRModeRecord
  };
  enum SeekModeEnum {
    SeekModePause,
    SeekModePlay,
    SeekModeLive,
  };
  DVRModeEnum mode;
  SeekModeEnum seek_mode;

  VarList * _settings;
  VarList * _settings_rec_continuous;
  VarInt * _max_frames;
  VarBool * _shift_on_exceed;
  VarBool * _fps_limit_enable;
  VarDouble * _fps_limit;
  PluginDVRWidget * w;

  double advance_last_t;

  bool trigger_pause_refresh;
  DVRFrame pause_frame;
  DVRStream stream;

  // Continuous recording variables
  bool is_recording_continuous = false;
  long rec_continuous_last_timestamp = 0;

  // std::variant<DVRNonBlockingWriter> would be preferable over allocating on heap,
  // but is not available in c++11
  std::unique_ptr<DVRNonBlockingWriter> frame_writer;

public:

  PluginDVR(FrameBuffer * fb);
  virtual VarList * getSettings();
  virtual ~PluginDVR();
  virtual string getName();
  virtual QWidget * getControlWidget();
  virtual ProcessResult process(FrameData * data, RenderOptions * options);
};

#endif
