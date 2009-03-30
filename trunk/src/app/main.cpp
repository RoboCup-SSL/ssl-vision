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
  \file    main.cpp
  \brief   The ssl-vision application entry point.
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include <QApplication>
#include <QCleanlooksStyle>
#include <QPlastiqueStyle>
#include <QString>
#include "mainwindow.h"
#include <stdio.h>
#include "qgetopt.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  GetOpt opts(argc, argv);
  bool start=false;
  opts.addShortOptSwitch( 's',QString("Start Capturing Immediately"),&start, false);
  if (!opts.parse()) {
    fprintf(stderr,"Invalid command line parameters!\n");
    exit(1);
  } 

  MainWindow mainWin(start);
  //if desired, launch a particular style:
  // app.setStyle(new QPlastiqueStyle());
  // app.setStyle(new QCleanlooksStyle());
  mainWin.show();
  mainWin.init();

  int retval = app.exec();

  return retval;
}
