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
  \brief   The ssl-vision graphicalClient application entry point.
  \author  Joydeep Biswas, Stefan Zickler, (C) 2008
  \edit    Ulfert Nehmiz (LogPlayer included) 2009
*/
//========================================================================


#include <stdio.h>
#include <QTime>
#include "CentralWindow.h"


QApplication *app;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;


    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));


    app = new QApplication(argc,argv);

    CentralWindow* centralWindow = new CentralWindow();
    centralWindow->show();

    app->exec();

    return 0;
}
