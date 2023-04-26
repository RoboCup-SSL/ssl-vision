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
  \file    plugin_dvr.cpp
  \brief   C++ Implementation: plugin_dvr
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "plugin_dvr.h"

#include <google/protobuf/util/json_util.h>
#include <chrono>
#include <fstream>
#include <utility>
#include <memory>

PluginDVRWidget::PluginDVRWidget(PluginDVR * dvr, QWidget * parent) : QWidget(parent) {
  layout_main=new QVBoxLayout();

  int mode_icon_size=32;
  jog = new JogDial();
  connect(jog,SIGNAL(valueChanged(float)),dvr,SLOT(jogValueChanged(float)));
  btn_pause_refresh=new QToolButton();
  btn_pause_refresh->setToolTip("Refresh Paused Image");
  btn_pause_refresh->setIcon(QIcon(":/icons/view-refresh.png"));
  btn_pause_refresh->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_pause_refresh->setEnabled(false);
  connect(btn_pause_refresh,SIGNAL(clicked()),dvr,SLOT(slotPauseRefresh()));
  btn_mode_record=new QToolButton();
  btn_mode_record->setToolTip("DVR Recorder Mode");
  btn_mode_pause=new QToolButton();
  btn_mode_pause->setToolTip("Pause Mode");
  btn_mode_off=new QToolButton();
  btn_mode_off->setToolTip("Live Mode (DVR Off)");

  btn_mode_record->setIcon(QIcon(":/icons/tool-animator.png"));
  btn_mode_pause->setIcon(QIcon(":/icons/media-playback-pause.png"));
  btn_mode_off->setIcon(QIcon(":/icons/media-playback-start.png"));

  btn_mode_record->setCheckable(true);
  btn_mode_pause->setCheckable(true);
  btn_mode_off->setCheckable(true);

  btn_mode_record->setAutoExclusive(true);
  btn_mode_pause->setAutoExclusive(true);
  btn_mode_off->setAutoExclusive(true);

  btn_mode_off->setChecked(true);
  btn_mode_record->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_mode_pause->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_mode_off->setIconSize(QSize(mode_icon_size,mode_icon_size));

  connect(btn_mode_record,SIGNAL(toggled(bool)),dvr,SLOT(slotModeToggled()));
  connect(btn_mode_pause,SIGNAL(toggled(bool)),dvr,SLOT(slotModeToggled()));
  connect(btn_mode_off,SIGNAL(toggled(bool)),dvr,SLOT(slotModeToggled()));

  layout_mode = new QHBoxLayout();
  layout_mode->addWidget(btn_mode_record);
  layout_mode->addWidget(btn_mode_pause);
  layout_mode->addWidget(btn_pause_refresh);
  layout_mode->addWidget(btn_mode_off);

  layout_mode->addStretch();
  box_mode = new QGroupBox("Mode Selection");
  box_mode ->setLayout(layout_mode);

  btn_rec_rec = new QToolButton();
  btn_rec_rec->setToolTip("Record");
  btn_rec_continuous = new QToolButton();
  btn_rec_continuous->setToolTip("Continuously record and save");
  btn_rec_new = new QToolButton();
  btn_rec_new->setToolTip("New Recording");
  btn_rec_load = new QToolButton();
  btn_rec_load->setToolTip("Load Recording");
  btn_rec_save = new QToolButton();
  btn_rec_save->setToolTip("Save Recording");

  btn_rec_rec->setIcon(QIcon(":/icons/media-record.png"));
  btn_rec_rec->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_rec_continuous->setIcon(QIcon(":/icons/view-refresh.png"));
  btn_rec_continuous->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_rec_new->setIcon(QIcon(":/icons/document-new.png"));
  btn_rec_new->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_rec_load->setIcon(QIcon(":/icons/document-open.png"));
  btn_rec_load->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_rec_save->setIcon(QIcon(":/icons/document-save.png"));
  btn_rec_save->setIconSize(QSize(mode_icon_size,mode_icon_size));

  connect(btn_rec_continuous,SIGNAL(clicked(bool)),dvr,SLOT(slotRecordContinuousToggled()));
  connect(btn_rec_new,SIGNAL(clicked(bool)),dvr,SLOT(slotMovieNew()));
  connect(btn_rec_load,SIGNAL(clicked(bool)),dvr,SLOT(slotMovieLoad()));
  connect(btn_rec_save,SIGNAL(clicked(bool)),dvr,SLOT(slotMovieSave()));


  btn_seek_front = new QToolButton();
  btn_seek_front->setToolTip("Seek to First Frame");
  btn_seek_frame_back = new QToolButton();
  btn_seek_frame_back->setToolTip("Seek to Previous Frame");
  btn_seek_pause = new QToolButton();
  btn_seek_pause->setToolTip("Pause Playback");
  btn_seek_frame_forward = new QToolButton();
  btn_seek_frame_forward->setToolTip("Seek to Next Frame");
  btn_seek_end = new QToolButton();
  btn_seek_end->setToolTip("Seek to Last Frame");
  btn_seek_play = new QToolButton();
  btn_seek_play->setToolTip("Play Recording");
  btn_seek_live = new QToolButton();
  btn_seek_live->setToolTip("Live View");

  btn_seek_wrap = new QToolButton();
  btn_seek_wrap->setToolTip("Loop Playback");

  int seek_icon_size=24;
  btn_seek_front->setIcon(QIcon(":/icons/go-first-view.png"));
  btn_seek_front->setIconSize(QSize(seek_icon_size,seek_icon_size));

  btn_seek_frame_back->setIcon(QIcon(":/icons/go-previous-view.png"));
  btn_seek_frame_back->setIconSize(QSize(seek_icon_size,seek_icon_size));

  btn_seek_pause->setIcon(QIcon(":/icons/media-playback-pause.png"));
  btn_seek_pause->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_seek_frame_forward->setIcon(QIcon(":/icons/go-next-view.png"));
  btn_seek_frame_forward->setIconSize(QSize(seek_icon_size,seek_icon_size));

  btn_seek_end->setIcon(QIcon(":/icons/go-last-view.png"));
  btn_seek_end->setIconSize(QSize(seek_icon_size,seek_icon_size));

  btn_seek_play->setIcon(QIcon(":/icons/media-playback-start.png"));
  btn_seek_play->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_seek_live->setIcon(QIcon(":/icons/media-skip-forward.png"));
  btn_seek_live->setIconSize(QSize(mode_icon_size,mode_icon_size));

  btn_seek_wrap->setIcon(QIcon(":/icons/object-rotate-right.png"));
  btn_seek_wrap->setIconSize(QSize(16,16));

  layout_rec = new QHBoxLayout();
  layout_rec->addWidget(btn_rec_new);
  layout_rec->addWidget(btn_rec_load);
  layout_rec->addWidget(btn_rec_save);
  layout_rec->addStretch();

  QHBoxLayout * layout_subseek_buttons = new QHBoxLayout();
  layout_subseek_buttons->addWidget(btn_seek_front);
  layout_subseek_buttons->addWidget(btn_seek_frame_back);
  layout_subseek_buttons->addWidget(btn_seek_frame_forward);
  layout_subseek_buttons->addWidget(btn_seek_end);
  QWidget * box_subseek_buttons = new QWidget();
  box_subseek_buttons->setLayout(layout_subseek_buttons);

  QVBoxLayout * layout_subseek = new QVBoxLayout();
  layout_subseek->addWidget(box_subseek_buttons);
  layout_subseek->addWidget(jog);
  QWidget * box_subseek = new QWidget();
  box_subseek->setLayout(layout_subseek);

  btn_rec_rec->setCheckable(true);
  btn_rec_continuous->setCheckable(true);

  btn_seek_pause->setCheckable(true);
  btn_seek_play->setCheckable(true);
  btn_seek_live->setCheckable(true);

  btn_seek_pause->setAutoExclusive(true);
  btn_seek_play->setAutoExclusive(true);
  btn_seek_live->setAutoExclusive(true);

  btn_seek_wrap->setCheckable(true);

  btn_seek_live->setChecked(true);

  connect(btn_seek_pause,SIGNAL(toggled(bool)),dvr,SLOT(slotSeekModeToggled()));
  connect(btn_seek_play,SIGNAL(toggled(bool)),dvr,SLOT(slotSeekModeToggled()));
  connect(btn_seek_live,SIGNAL(toggled(bool)),dvr,SLOT(slotSeekModeToggled()));

  connect(btn_seek_front,SIGNAL(clicked(bool)),dvr,SLOT(slotSeekFrameFirst()));
  connect(btn_seek_end,SIGNAL(clicked(bool)),dvr,SLOT(slotSeekFrameLast()));
  connect(btn_seek_frame_back,SIGNAL(clicked(bool)),dvr,SLOT(slotSeekFrameBack()));
  connect(btn_seek_frame_forward,SIGNAL(clicked(bool)),dvr,SLOT(slotSeekFrameForward()));

  layout_seek = new QHBoxLayout();
  layout_seek->addWidget(btn_rec_rec);
  layout_seek->addSpacing(20);
  layout_seek->addWidget(btn_rec_continuous);
  layout_seek->addSpacing(20);
  layout_seek->addWidget(btn_seek_pause);
  layout_seek->addWidget(btn_seek_play);
  layout_seek->addWidget(btn_seek_wrap);
  layout_seek->addWidget(btn_seek_live);
  layout_seek->addSpacing(10);
  layout_seek->addWidget(box_subseek);
  layout_seek->addStretch();

  label_info = new QLabel();

  box_rec = new QGroupBox("Manage Recordings");
  box_rec ->setLayout(layout_rec);

  box_seek = new QGroupBox("Record and Playback");
  box_seek ->setLayout(layout_seek);

  layout_dvr= new QVBoxLayout();
  layout_dvr->addWidget(box_rec);
  layout_dvr->addWidget(box_seek);
  layout_dvr->addWidget(label_info);
  layout_dvr->addStretch();

  box_dvr = new QWidget();
  box_dvr ->setLayout(layout_dvr);

  layout_main->addWidget(box_mode);
  layout_main->addWidget(box_dvr);
  //layout_main->addStretch();
  this->setLayout(layout_main);


}





/* ===== PluginDVR ===== */

void PluginDVR::slotSeekModeToggled() {
  lock();
    if (w->btn_seek_pause->isChecked()) {
      seek_mode=SeekModePause;
    } else if (w->btn_seek_play->isChecked()) {
      seek_mode=SeekModePlay;
    } else {
      seek_mode=SeekModeLive;
    }
  unlock();
}

void PluginDVR::slotModeToggled() {
  lock();
  w->btn_pause_refresh->setEnabled(w->btn_mode_pause->isChecked());

  DVRModeEnum old_mode = mode;
  if (w->btn_mode_pause->isChecked()) {
    mode=DVRModePause;
  } else if (w->btn_mode_record->isChecked()) {
    mode=DVRModeRecord;
  } else {
    mode=DVRModeOff;
  }

  w->box_dvr->setEnabled(mode==DVRModeRecord);

  if (mode==DVRModePause && mode!=old_mode) {
    trigger_pause_refresh=true;
  }
  //update mode:
  unlock();
}

DVRFrame::~DVRFrame() {
  video.setData(0);
}

void PluginDVR::slotRecordContinuousToggled(){
  is_recording_continuous = false;

  // Destruct the frame writer if it exists
  if(frame_writer){
    frame_writer.reset();
  }

  // Return if continuous recording is disabled
  if(!w->btn_rec_continuous->isChecked()) return;

  // If the FPS limiter is not enabled, inform the user
  if(!_fps_limit_enable->getBool()){
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(w, "Warning! FPS limiter not enabled!", "The FPS limiter is not enabled. Writing all "
                                  "frames could take a toll on your computer. You can enable the FPS limiter on the "
                                  "left under DVR Settings. Do you still want to continue?", QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::No){
      w->btn_rec_continuous->setChecked(false);
      return;
    }
  }

  // Ask the user where to store the recordings
  const auto dir = QFileDialog::getExistingDirectory(nullptr, "Select Directory to store", "", QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);

  // Check if 'cancel' was pressed
  if(dir.isEmpty()){
    w->btn_rec_continuous->setChecked(false);
    return;
  }

  // Create the frame writer with the given directory
  frame_writer = std::unique_ptr<DVRNonBlockingWriter>(new DVRNonBlockingWriter(dir));
  is_recording_continuous = true;
}

void PluginDVR::slotSeekFrameFirst() {
  lock();
    stream.seek(0);
  unlock();
}

void PluginDVR::slotSeekFrameForward() {
  lock();
    stream.advance(1,w->btn_seek_wrap->isChecked());
  unlock();
}

void PluginDVR::slotSeekFrameBack() {
  lock();
    stream.advance(-1,w->btn_seek_wrap->isChecked());
  unlock();
}

void PluginDVR::slotSeekFrameLast() {
  lock();
    stream.seek(stream.getFrameCount()-1);
  unlock();
}

void PluginDVR::slotPauseRefresh() {
  lock();
  trigger_pause_refresh=true;
  unlock();
}

void PluginDVR::jogValueChanged(float val) {

}

void PluginDVR::slotMovieNew() {
  lock();
  stream.clear();
  unlock();
}

void PluginDVR::slotMovieLoad() {
  lock();
  // Do not use native dialog since some platforms like Windows XP and
  // Kubuntu 12.04 have very slow directory listing dialogs. The QT version is
  // pretty responsive.
  QString dirstr = QFileDialog::getExistingDirectory(
      0,"Select Directory to Load", "",
      QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
  if (dirstr!="") {

    int fnum=0;
    stream.clear();
    QDir dir(dirstr);
    QFileInfoList list = dir.entryInfoList ( QDir::Files | QDir::Readable, QDir::Name );
    QProgressDialog * dlg = new QProgressDialog("Loading Movie from Files...","Cancel", 1,list.size());
    dlg->setWindowModality(Qt::WindowModal);
    for (int i = 0 ; i < list.size(); i ++) {
      dlg->setValue(i+1);
      QFileInfo info = list[i];
      if (info.suffix().compare("png",Qt::CaseInsensitive) == 0 ||
          info.suffix().compare("bmp",Qt::CaseInsensitive) == 0 ||
          info.suffix().compare("jpg",Qt::CaseInsensitive) == 0 ||
          info.suffix().compare("jpeg",Qt::CaseInsensitive) == 0) {
          int w,h;
          rgb * data = ImageIO::readRGB(w,h,info.filePath().toStdString().c_str());
          FrameData fdata;
          fdata.time = 0.0;
          fdata.number = fnum;
          fnum++;
          fdata.video.setColorFormat(COLOR_RGB8);
          fdata.video.setData((unsigned char*)data);
          fdata.video.setWidth(w);
          fdata.video.setHeight(h);
          stream.appendFrame(&fdata,false);
          delete[] data;
      }
      if (dlg->wasCanceled()) break;
    }
    delete dlg;
  }
  unlock();
}

void PluginDVR::slotMovieSave() {
  lock();
  QString dir = QFileDialog::getExistingDirectory(0,"Select Directory to Save");
  QProgressDialog * dlg = new QProgressDialog("Saving Movie to PNG Files...","Cancel", 1,stream.getFrameCount());
  dlg->setWindowModality(Qt::WindowModal);

  if (dir!="") {
    for (int i = 0; i < stream.getFrameCount(); i++) {
      DVRFrame * f = stream.getFrame(i);
      dlg->setValue(i+1);
      if (f!=0) {
        DVRUtils::saveFrame(*f, dir, i);
      }

      SSL_DetectionFrame* detection_frame = stream.getDetectionFrame(i);
      if(detection_frame){
        DVRUtils::saveDetectionFrame(*detection_frame, dir, i);
      }

      if (dlg->wasCanceled()) break;
    }
  }
  delete dlg;
  unlock();
}

PluginDVR::PluginDVR(FrameBuffer * fb)
 : VisionPlugin(fb)
{
  mode = DVRModeOff;
  advance_last_t=0;
  seek_mode = SeekModeLive;
  w=new PluginDVRWidget(this);
  trigger_pause_refresh=false;
  stream.clear();
  _settings = new VarList("DVR Settings");
  _max_frames = new VarInt("Max Frames",250);
  _max_frames->setMin(0);
  _shift_on_exceed = new VarBool("Shift Video On Exceeding",true);

  // Options to limit FPS recording, as to not overload the PC with write operations
  _settings_rec_continuous = new VarList("DVR Record Continuous");
  _fps_limit_enable = new VarBool("Enable DVR FPS Limiter");
  _fps_limit_enable->setBool(true);
  _fps_limit = new VarDouble("DVR FPS Limit");
  _fps_limit->setInt(1.0);
  _settings_rec_continuous->addChild(_fps_limit_enable);
  _settings_rec_continuous->addChild(_fps_limit);

  _settings->addChild(_max_frames);
  _settings->addChild(_shift_on_exceed);
  _settings->addChild(_settings_rec_continuous);

  slotModeToggled();
  slotSeekModeToggled();
}

PluginDVR::~PluginDVR()
{
  delete _settings;
  delete w;
}

VarList * PluginDVR::getSettings() {
  return _settings;
}

string PluginDVR::getName() {
  return "DVR";
}

void DVRFrame::getFromFrameData(FrameData * data) {
  video.deepCopyFromRawImage(data->video,true);
}

ProcessResult PluginDVR::process(FrameData * data, RenderOptions * options) {

  using namespace std::chrono;

  (void)options;
  QString status;
  QString stream_info;
  stream_info = "";
  status = "";
  bool is_recording = w->btn_rec_rec->isChecked();
  //capture data:
  if (mode == DVRModePause) {
    if (trigger_pause_refresh) {
      pause_frame.getFromFrameData(data);
      trigger_pause_refresh=false;
    }
    status = "Pausing.";
  } else if (mode == DVRModeRecord) {
    if (is_recording || is_recording_continuous) {
      status = "Recording mode. " + QString::number(stream.getFrameCount()) + " frames in buffer. ";

      stream.setLimit(_max_frames->getInt());

      // Get detection frame connected to frame
      SSL_DetectionFrame* detection_frame = (SSL_DetectionFrame *)data->map.get("ssl_detection_frame");

      // If recording is on, store the frame and possible detection_frame in the ringbuffers
      if (is_recording) {
        status = status + "Currently storing all frames. ";
        stream.appendFrame(data, _shift_on_exceed->getBool());
        if(detection_frame)
          stream.appendDetectionFrame(detection_frame, _shift_on_exceed->getBool());
      }

      // If continuous recording is on, store the frame and possible detection_frame on the disk
      if(is_recording_continuous) {
        double fps_limit = _fps_limit->getDouble();
        status += "Currently writing " + QString::number(fps_limit) + " frames per second to disk. ";
        // Check if enough time has passed to store the next frame and detection_frame
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        double interval_ms = 1000 / fps_limit;

        if (!_fps_limit_enable->getBool() || interval_ms < now - rec_continuous_last_timestamp) {
            rec_continuous_last_timestamp = now;
            
            // using namespace std::chrono;
            // high_resolution_clock::time_point t1 = high_resolution_clock::now();
            // std::cout << std::endl;
            frame_writer->write(data, detection_frame);
            // high_resolution_clock::time_point t2 = high_resolution_clock::now();
            // duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            // std::cout << "[plugin_dvr.cpp][process] Writing frame took " << time_span.count() * 1000 << " ms in main thread" << std::endl;
            
        }
      }

      // Update status text
      if (stream.getFrameCount() >= stream.getLimit()) {
        if (_shift_on_exceed->getBool()) {
          status = status + " Past Max Frame Limit! Now Shift-Recoding!";
        } else {
          status = status + " Past Max Frame Limit! Dropping Frames!";
        }
      }
    } else {
      status = "Recorded " + QString::number(stream.getFrameCount()) + " Frame(s)";
    }
  } else {
    status = "Live Pass-Through.";
  }

  if (mode==DVRModeRecord) {
    //move stream:
    double t=GetTimeSec();
    double advance_passed=t-advance_last_t;
    double advance_fps=(double)w->jog->offset() * 20.0;

    int frames_advance_int=0;
    if (advance_fps != 0.0) {
      double advance_period=1.0/advance_fps;
      float frames_behind=advance_passed/advance_period;
      if (fabs(frames_behind) > 1.0) {
        frames_advance_int=(int)frames_behind;
      }
    }
    if (frames_advance_int!=0) {
      w->btn_seek_pause->setChecked(true);
      seek_mode=SeekModePause;
    }


    if (seek_mode==SeekModePlay) {
      stream.advance(1,w->btn_seek_wrap->isChecked());
    } else if (seek_mode==SeekModePause) {
      if (frames_advance_int!=0) {
        stream.advance(frames_advance_int,w->btn_seek_wrap->isChecked());
        advance_last_t=t;
      }
    }
    if (stream.getFrameCount() <= 0 || seek_mode==SeekModeLive) {
      stream_info = "Showing Live-View.";
    } else {
      double percent = 100.0 * ((double)stream.getCurrentFrameIndex()+1) / ((double)(stream.getFrameCount()));
      if (seek_mode==SeekModePause) {
        stream_info = "Paused Playback at Frame " + QString::number(stream.getCurrentFrameIndex()+1) + "/" + QString::number(stream.getFrameCount()) + " (" + QString::number(percent,'f',2) + "%).";
      } else {
        stream_info = "Running Playback at Frame " + QString::number(stream.getCurrentFrameIndex()+1) + "/" + QString::number(stream.getFrameCount()) + " (" + QString::number(percent,'f',2) + "%).";
      }
    }
  }

  //output data:
  if (mode==DVRModePause) {
    data->video.deepCopyFromRawImage(pause_frame.video,false);
  } else if (mode==DVRModeRecord) {
    if (seek_mode!=SeekModeLive && stream.getFrameCount() > 0) {
      DVRFrame * f = stream.getCurrentFrame();
      if (f!=0) {
        data->video.deepCopyFromRawImage(f->video,false);
      }
    }
  }

  //update GUI:
  w->label_info->setText(status + "\n" + stream_info);

  return ProcessingOk;
}

QWidget * PluginDVR::getControlWidget() {
  return w;
}





/* ===== DVRStream ===== */

bool DVRStream::loadStream(QString file) {
  return false;
}

void DVRStream::newRecording(QString directory) {

}

void DVRStream::saveStream(QString directory) {

}

void DVRStream::clear() {
  for (int i = 0; i < frames.size(); i++) {
    delete frames[i];
  }
  frames.clear();
  current=0;
}

void DVRStream::appendFrame(FrameData * data, bool shift_stream_on_limit_exceed) {
  if (limit!=0 && ((getFrameCount() + 1) > limit)) {
    if (shift_stream_on_limit_exceed) {
      DVRFrame * f = frames[0];
      delete f;
      frames.removeFirst();
      if (current > 0) current--;
    } else {
      return;
    }
  }
  DVRFrame * f = new DVRFrame();
  f->getFromFrameData(data);
  frames.append(f);
}

void DVRStream::appendDetectionFrame(SSL_DetectionFrame * frame, bool shift_stream_on_limit_exceed){
  if (limit!=0 && ((getFrameCount() + 1) > limit)) {
    if (shift_stream_on_limit_exceed) {
      SSL_DetectionFrame * f = detection_frames[0];
      delete f;
      frames.removeFirst();
      if (current > 0) current--;
    } else {
      return;
    }
  }
  SSL_DetectionFrame * f = new SSL_DetectionFrame();
  f->CopyFrom(*frame);
  detection_frames.append(f);
}

void DVRStream::seek(int frame) {
  if (frame >= getFrameCount()) frame=getFrameCount()-1;
  if (frame < 0) frame=0;
  current=frame;
}

int DVRStream::getFrameCount() {
  return frames.size();
}

void DVRStream::setLimit(int num_frames) {
  limit=num_frames;
}

int DVRStream::getLimit() {
  return limit;
}

DVRStream::DVRStream() {
  limit=0;
  clear();
}

DVRStream::~DVRStream() {
 clear();
}

void DVRStream::advance(int frames, bool wrap) {
  if (getFrameCount() == 0) return;
  int new_current=current+frames;
  int max_idx=getFrameCount()-1;
  if (new_current < 0) {
    if (wrap) {
      new_current=mod(new_current,getFrameCount());
    } else {
      new_current=0;
    }
  }
  if (new_current > max_idx) {
    if (wrap) {
      new_current=mod(new_current,getFrameCount());
    } else {
      new_current=max_idx;
    }
  }
  current=new_current;
}

void DVRStream::advanceToMostRecent() {
  current=getFrameCount()-1;
}

int DVRStream::getCurrentFrameIndex() {
  return current;
}

DVRFrame * DVRStream::getCurrentFrame() {
  if (current >= frames.size()) return 0;
  return frames[current];
}

DVRFrame * DVRStream::getFrame(int i) {
  if (current >= frames.size()) return 0;
  if (i < 0) return 0;
  return frames[i];
}

SSL_DetectionFrame * DVRStream::getDetectionFrame(int i) {
  if (current >= detection_frames.size()) return 0;
  if (i < 0) return 0;
  return detection_frames[i];
}





/* ===== DVRUtils ===== */

void DVRUtils::saveFrame(const DVRFrame& frame, const QString& dir, int index){
  rgbImage output;

  ColorFormat fmt = frame.video.getColorFormat();
  output.allocate(frame.video.getWidth(), frame.video.getHeight());
  if (fmt == COLOR_YUV422_UYVY) {
    Conversions::uyvy2rgb(frame.video.getData(), output.getData(), frame.video.getWidth(), frame.video.getHeight());
  } else if (fmt == COLOR_RGB8) {
    memcpy(output.getData(), frame.video.getData(), frame.video.getNumBytes());
  } else {
    std::cout << "[PluginDVR::saveFrame] Warning! Unknown color format" << std::endl;
    output.allocate(0,0);
  }
  if (output.getNumBytes() > 0) {
    // write file:
    QString num = QString::number(index);
    num = "00000" + num;
    num = num.right(5);
    QString filename = dir + "/" + num + ".png";
    output.save(filename.toStdString());
  }
}

void DVRUtils::saveDetectionFrame(const SSL_DetectionFrame& detection_frame, const QString& dir, int index){
  std::string json_string;
  google::protobuf::util::MessageToJsonString(detection_frame, &json_string);

  QString num = QString::number(index);
  num = "00000" + num;
  num = num.right(5);
  QString filename = dir + "/" + num + ".json";
  std::ofstream json_file(filename.toStdString());
  json_file << json_string;
  json_file.close();
}





/* ===== DVRThreadSafeQueue ===== */

DVRFrameData DVRThreadSafeQueue::dequeue() {
  std::unique_lock<std::mutex> lock(queue_mutex);

  // Releases the lock and wait until there is new element
  broker.wait(lock, [&] {
    return !queue.empty() || !running;
  });

  // Early return since queue might be empty
  if (!running) return {};

  auto data = std::move(queue.front());
  queue.pop();
  return data;
}

void DVRThreadSafeQueue::enqueue(DVRFrameData data) {
  const std::lock_guard<std::mutex> lock(queue_mutex);
  queue.push(std::move(data));
  broker.notify_one();
}

void DVRThreadSafeQueue::stop() {
  running = false;
  broker.notify_one();
}

int DVRThreadSafeQueue::size() {
  return queue.size();
}





/* ===== DVRNonBlockingWriter ===== */

DVRNonBlockingWriter::DVRNonBlockingWriter(QString output_dir): output_dir(std::move(output_dir)){
  std::cout << "[DVRNonBlockingWriter] New instance" << std::endl;
  writer_thread = std::thread(&DVRNonBlockingWriter::runWriterOnLoop, this);
}

DVRNonBlockingWriter::~DVRNonBlockingWriter() {
  if (writer_thread.joinable()) {
    running = false;
    data_buffer.stop();
    writer_thread.join();
  }
  std::cout << "[DVRNonBlockingWriter] Instance destructed" << std::endl;
}

void DVRNonBlockingWriter::runWriterOnLoop() {
  while(running) write();
}

/**
 * Called from secondary thread. Writes the frame to disk
*/
void DVRNonBlockingWriter::write() {
  const auto data = data_buffer.dequeue();

  // Early return in case, thread was stopped, since dequeue might return {} in that case
  if (!running) return;

  // using namespace std::chrono;
  // high_resolution_clock::time_point t1 = high_resolution_clock::now();
  DVRUtils::saveFrame(*data.frame_ptr, output_dir, index);
  DVRUtils::saveDetectionFrame(data.detection_frame, output_dir, index);
  // high_resolution_clock::time_point t2 = high_resolution_clock::now();
  // duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  // std::cout << "[DVRNonBlockingWriter] Writing frame " << index << " took " << time_span.count() * 1000 << " ms in secondary thread" << std::endl;

  index++;
}

/**
 * Called from main thread. Deepcopies the frame and adds it to the queue to be written in the secondary thread
*/
void DVRNonBlockingWriter::write(FrameData* frameData, SSL_DetectionFrame* detectionFrame) {
    std::unique_ptr<DVRFrame> frame_ptr = std::unique_ptr<DVRFrame>(new DVRFrame());
    // Limit the number of frames in the queue to 1. This prevents memory from overflowing
    if(data_buffer.size() == 0){
      frame_ptr->getFromFrameData(frameData);
      data_buffer.enqueue({std::move(frame_ptr), *detectionFrame });
    }else{
      std::cout << "[DVRNonBlockingWriter] Warning: a previous frame is already queued to be written. Please lower the 'DVR FPS Limit' in the settings. Skipping current frame.." << std::endl;
    }
}
