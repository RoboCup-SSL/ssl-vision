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
  \file    CentralWindow.h
  \brief   C++ Implementation: CentralWindow
  \author  Ulfert Nehmiz, 2009
*/
//========================================================================

#include "CentralWindow.h"
#include <QtCore/QVariant>


CentralWindow::CentralWindow()
{
    drawMutex = new QMutex();
    centralwidget = new QWidget(this);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    gridLayout = new QGridLayout(centralwidget);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    Spielfeld = new QFrame(centralwidget);
    Spielfeld->setObjectName(QString::fromUtf8("Spielfeld"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(Spielfeld->sizePolicy().hasHeightForWidth());
    Spielfeld->setSizePolicy(sizePolicy);
    Spielfeld->setFrameShape(QFrame::Panel);
    Spielfeld->setFrameShadow(QFrame::Raised);
    Spielfeld->setLineWidth(1);
    gridLayout_7 = new QGridLayout(Spielfeld);
    gridLayout_7->setSpacing(0);
    gridLayout_7->setMargin(0);
    gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
    soccerView = new SoccerView(drawMutex);
    soccerView->setObjectName(QString::fromUtf8("soccerView"));
    soccerView->setEnabled(true);

    gridLayout_7->addWidget(soccerView, 1, 0, 1, 1);

    gridLayout->addWidget(Spielfeld, 0, 0, 1, 1);
    this->setCentralWidget(centralwidget);

    logControl = new QDockWidget(this);
    logControl->setObjectName(QString::fromUtf8("logControl"));
    logControl->setEnabled(true);
    logControl->setMinimumSize(QSize(79, 150));
    logControl->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    logControl->setAllowedAreas(Qt::BottomDockWidgetArea);
    logControlWidget = new QWidget();
    logControlWidget->setObjectName(QString::fromUtf8("logControlWidget"));
    horizontalSlider = new QSlider(logControlWidget);
    horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
    horizontalSlider->setGeometry(QRect(0, 0, 631, 22));
    horizontalSlider->setMaximum(1800);
    horizontalSlider->setPageStep(60);
    horizontalSlider->setOrientation(Qt::Horizontal);
    horizontalSlider->setTickPosition(QSlider::TicksAbove);
    horizontalSlider->setTickInterval(300);
    log_backward = new QPushButton(logControlWidget);
    log_backward->setObjectName(QString::fromUtf8("log_backward"));
    log_backward->setGeometry(QRect(0, 30, 150, 25));
    log_pause = new QPushButton(logControlWidget);
    log_pause->setObjectName(QString::fromUtf8("log_pause"));
    log_pause->setGeometry(QRect(160, 30, 150, 25));
    log_forward = new QPushButton(logControlWidget);
    log_forward->setObjectName(QString::fromUtf8("log_forward"));
    log_forward->setGeometry(QRect(320, 30, 150, 25));
    log_play = new QPushButton(logControlWidget);
    log_play->setObjectName(QString::fromUtf8("log_play"));
    log_play->setGeometry(QRect(480, 30, 150, 25));
    log_slower = new QPushButton(logControlWidget);
    log_slower->setObjectName(QString::fromUtf8("log_slower"));
    log_slower->setGeometry(QRect(0, 60, 150, 25));
    log_speed = new QLabel(logControlWidget);
    log_speed->setObjectName(QString::fromUtf8("log_speed"));
    log_speed->setGeometry(QRect(160, 60, 150, 25));
    log_speed->setAlignment(Qt::AlignCenter);
    log_faster = new QPushButton(logControlWidget);
    log_faster->setObjectName(QString::fromUtf8("log_faster"));
    log_faster->setGeometry(QRect(320, 60, 150, 25));
    log_frame_back = new QPushButton(logControlWidget);
    log_frame_back->setObjectName(QString::fromUtf8("log_frame_back"));
    log_frame_back->setGeometry(QRect(0, 90, 150, 25));
    log_frameNumber = new QLCDNumber(logControlWidget);
    log_frameNumber->setObjectName(QString::fromUtf8("log_frameNumber"));
    log_frameNumber->setGeometry(QRect(160, 90, 150, 25));
    log_frameNumber->setNumDigits(7);
    log_frameNumber->setSegmentStyle(QLCDNumber::Flat);
    log_frameNumber->setProperty("intValue", QVariant(0));
    log_frame_forward = new QPushButton(logControlWidget);
    log_frame_forward->setObjectName(QString::fromUtf8("log_frame_forward"));
    log_frame_forward->setGeometry(QRect(320, 90, 150, 25));
    log_totalFrames = new QLCDNumber(logControlWidget);
    log_totalFrames->setObjectName(QString::fromUtf8("log_totalFrames"));
    log_totalFrames->setGeometry(QRect(480, 90, 150, 25));
    log_totalFrames->setAutoFillBackground(false);
    log_totalFrames->setSmallDecimalPoint(false);
    log_totalFrames->setNumDigits(7);
    log_totalFrames->setSegmentStyle(QLCDNumber::Flat);
    logControl->setWidget(logControlWidget);
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(8), logControl);

    logControl->setWindowTitle(QApplication::translate("GuiControls", "LogControl", 0, QApplication::UnicodeUTF8));
    log_backward->setText(QApplication::translate("GuiControls", "backward", 0, QApplication::UnicodeUTF8));
    log_pause->setText(QApplication::translate("GuiControls", "pause", 0, QApplication::UnicodeUTF8));
    log_forward->setText(QApplication::translate("GuiControls", "forward", 0, QApplication::UnicodeUTF8));
    log_play->setText(QApplication::translate("GuiControls", "play", 0, QApplication::UnicodeUTF8));
    log_slower->setText(QApplication::translate("GuiControls", "slower", 0, QApplication::UnicodeUTF8));
    log_speed->setText(QApplication::translate("GuiControls", "Speed", 0, QApplication::UnicodeUTF8));
    log_faster->setText(QApplication::translate("GuiControls", "faster", 0, QApplication::UnicodeUTF8));
    log_frame_back->setText(QApplication::translate("GuiControls", "frame--", 0, QApplication::UnicodeUTF8));
    log_frame_forward->setText(QApplication::translate("GuiControls", "frame++", 0, QApplication::UnicodeUTF8));

    logControl->hide();
    thread = new ViewUpdateThread(soccerView, drawMutex);

    //connect all SLOTs and SIGNALs
    //Slider control
    connect(thread, SIGNAL(initializeSlider(int,int,int,int,int)), this, SLOT(initializeSlider(int, int, int, int, int)));
    connect(thread, SIGNAL(update_frame(int)), horizontalSlider,         SLOT(setValue(int)));
    connect(horizontalSlider, SIGNAL(valueChanged(int)), thread->log_control, SLOT(goto_frame(int)));
    connect(horizontalSlider, SIGNAL(sliderPressed()),   thread->log_control, SLOT(log_pause()));
    connect(horizontalSlider, SIGNAL(sliderReleased()),  thread->log_control, SLOT(log_play()));

    //Buttons for logfile control
    connect(log_forward,       SIGNAL(clicked()), thread->log_control, SLOT(log_forward()));
    connect(log_play,          SIGNAL(clicked()), thread->log_control, SLOT(log_play()));
    connect(log_backward,      SIGNAL(clicked()), thread->log_control, SLOT(log_backward()));
    connect(log_pause,         SIGNAL(clicked()), thread->log_control, SLOT(log_pause()));
    connect(log_faster,        SIGNAL(clicked()), thread->log_control, SLOT(log_faster()));
    connect(log_slower,        SIGNAL(clicked()), thread->log_control, SLOT(log_slower()));
    connect(log_frame_back,    SIGNAL(clicked()), thread->log_control, SLOT(log_frame_back()));
    connect(log_frame_forward, SIGNAL(clicked()), thread->log_control, SLOT(log_frame_forward()));

    //Log Control
    connect(thread, SIGNAL(showLogControl(bool)), logControl, SLOT(setVisible(bool)));

    //QLCDNumber control
    connect(thread, SIGNAL(update_frame(int)), log_frameNumber, SLOT(display(int)));
    connect(thread, SIGNAL(log_size(int)),     log_totalFrames, SLOT(display(int)));
    connect(thread->log_control, SIGNAL(update_speed(QString)), log_speed, SLOT(setText(QString)));

    thread->start(QThread::NormalPriority);

    //initialisation for nice start
    for(int i = 0; i < 14; i++)
    {
        soccerView->initView();
        soccerView->updateView();
    }
}

CentralWindow::~CentralWindow()
{
    thread->Terminate();
    while(thread->isRunning())
        ;
}

void CentralWindow::initializeSlider(int min, int max, int singleStep, int pageStep, int tickInterval)
{
    horizontalSlider->setRange(min, max);
    horizontalSlider->setSingleStep(singleStep);
    horizontalSlider->setPageStep(pageStep);
    horizontalSlider->setValue(min);
    horizontalSlider->setTickPosition(QSlider::TicksAbove);
    horizontalSlider->setTickInterval(tickInterval);
}
