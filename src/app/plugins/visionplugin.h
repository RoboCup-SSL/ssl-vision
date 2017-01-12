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
  \file    visionplugin.h
  \brief   C++ Interface: VisionPlugin
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef VISIONPLUGIN_H
#define VISIONPLUGIN_H
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMutex>
#include <QObject>
#include <string>

#include "VarTypes.h"
#include "framedata.h"
#include "realtimedisplaywidget.h"
#include "pixelloc.h"
using namespace std;
using namespace VarTypes;

enum ProcessResult {
  ProcessingOk = 0,
  ProcessingFailed = 1,
};

/*!
  \class   VisionPlugin
  \brief   A base class for general vision processing plugin
  \author  Stefan Zickler, (C) 2008

  This is the class that should be extended when adding vision processing functions to the system.
  A VisionPlugin can be added to any part of a VisionStack

*/
class VisionPlugin : public QObject {
Q_OBJECT
protected:
    QMutex mutex;
    bool enabled;
    bool visualize;
    bool shared;
    FrameBuffer * buffer;
    double time_proc;
    double time_post;
public:


    VisionPlugin(FrameBuffer * _buffer);

    virtual ~VisionPlugin();

    /// you should overload this one to return the name of your plugin
    virtual string getName();

    /// this functions provides convenient access to the ring-buffer
    FrameBuffer * getFrameBuffer() const;

    /// these functions are called automatically by the parent-stack
    /// to ensure thread-safety
    /// you should *NOT* need to touch them
    void lock();
    void unlock();

    /// indicates whether this plugin will be used
    /// (e.g. whether process() will be called on it)
    virtual bool isEnabled() const;
    virtual void setEnabled(bool enable);

    /// indicates whether this plugin's visualization functions will be called
    /// (postProcess(...) and renderOverlay(...))
    virtual bool isVisualize() const;
    virtual void setVisualize(bool enable);

    /// indicates whether this plugin instance is used by multiple different stacks.
    /// if so, its control panels will be shown on the global pane, instead of the usual
    /// independent camera pane.
    virtual bool isSharedAmongStacks() const;
    virtual void setSharedAmongStacks(bool enable);

    /// this function will be called about many times/s on your plugin
    /// in most cases you might want to only trigger a render if frame_changed==true
    /// which should occur with the same frequency as your camera input
    virtual void displayLoopEvent(bool frame_changed, RenderOptions * opts);

    /// this is the main-processing function. It allows your plugin to operate on the
    /// current frame-data
    virtual ProcessResult process(FrameData * data, RenderOptions * options);

    /// any settings of your plugin should be returned here
    /// this will ensure a nice integration with the systems data-tree and automatic
    /// XML settings saving
    virtual VarList * getSettings();

    /// the following two functions should be used to visualize data
    /// postProcess(...) is used to modify the data in the FrameData buffer
    /// before it will hit the end of the rendering pipe
    /// this is useful if the only rendering we need to do can be done
    /// by directly modifying the output image (e.g. color some image-regions)
    ///
    /// Note that this will be called after all the processing of all modules
    /// has happened, so it should not affect processing, only visualization
    virtual void postProcess(FrameData * data, RenderOptions * options);

    /// renderOverlay(...) on the other hand is used to visualize anything that
    /// goes beyond just modifying the existing output buffer. It should be used
    /// if we need to draw additional stuff using OpenGL or a QPainter:
    /// renderOverlay will be called after all postProcess(...) have been called
    //void renderOverlay(FrameData * data, RenderOptions * options);

    /// if this plugin uses an extra widget to visualize data output
    /// it should be returned here:
    /// this is useful if visualization neeeds to go beyond the two above functions
    virtual QWidget * getVisualizationWidget();

    /// if this plugin has a control-widget (other than getSettings())
    /// it should be returned here:
    virtual QWidget * getControlWidget();

    /// this function will be called when the plugin's widgets are being closed
    /// it is up to the plugin whether to delete them from memory or not.
    /// If your plugin decides to delete them from memory (which is recommended if they
    /// are memory hogs, you just need to make sure to correctly reload them when
    /// one of the getControlWidget(...) or getVisualizationWidget(...) functions is called.
    virtual void closeWidgets();

    /// any QActions that should be displayed in a toolbar should be
    /// returned here
    virtual QList<QAction *> actions();

    /// These are event handlers that were called on the
    /// visualization window
    /// these events should provide a *locked* (meaning thread-safe) context.
    /// That is, they will never get triggered during a processing function.
    /// IMPORTANT, you should call event->accept() if you choose to handle it
    /// because non-accepted events will be forwarded to the next plugin
    /// and multiple plugins might be listening to the same events.
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );
    virtual void wheelEvent ( QWheelEvent * event, pixelloc loc );

    void setTimeProcessing(double val);
    void setTimePostProcessing(double val);
    double getTimeProcessing();
    double getTimePostProcessing();

public slots:
    void slotKeyPressEvent ( QKeyEvent * event );

};

#endif
