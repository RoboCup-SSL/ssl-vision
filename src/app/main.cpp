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
#include <QString>
#include "mainwindow.h"
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "qgetopt.h"

MainWindow* mainWinPtr = NULL;

// Signal handler for breaks (Ctrl-C)
void HandleStop(int i) {
  if (mainWinPtr != NULL) {
    printf("\nExiting.\n");
    fflush(stdout);
    mainWinPtr->Quit();
  }
}

// Print a path warning if the running directory is NOT one level above the binary directory
void printPathWarning() {
  char cwd[PATH_MAX];
  char* result = getcwd(cwd, PATH_MAX);
  if (!result) {
    // We failed getting the cwd, abort
    return;
  }

  std::string currentWorkingDir = cwd;

  char binaryPathRaw[PATH_MAX];
  int count = readlink("/proc/self/exe", binaryPathRaw, PATH_MAX);

  if (count == -1) {
    // We failed getting the binary path, quit
    return;
  }
  std::string binaryPath = std::string(binaryPathRaw, count);

  // Get the offset to check
  unsigned int offset = currentWorkingDir.size();
  if (offset == 0 || offset >= binaryPath.size()){
    return;
  }

  if (binaryPath.substr(offset, string::npos) != "/bin/vision") {
    std::string warningMsg = "[WARNING] You are running vision from a non-standard directory.\n"
      "Please run ssl-vision from the root of the git repo unless you know what you are doing.\n"
      "(run with: ./bin/vision)\n";

    std::cout << std::endl << warningMsg << std::endl;

    // display a message box
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8(warningMsg.c_str()));
    msgBox.exec();
  }
}

int main(int argc, char *argv[])
{
  signal(SIGINT,HandleStop);
  QApplication app(argc, argv);

  GetOpt opts(argc, argv);
  bool help=false;
  bool start=false;
  bool enforce_affinity=false;
  QString camera_count;
  int ecode=0;
  opts.addSwitch("help",&help);
  opts.addShortOptSwitch( 'a',QString("Enforce Processor Affinity"),&enforce_affinity, false);
  opts.addShortOptSwitch( 's',QString("Start Capturing Immediately"),&start, false);
  opts.addOptionalOption( 'c',QString("Camera Count"),&camera_count, QString("4"));
  if (!opts.parse()) {
    fprintf(stderr,"Invalid command line parameters!\n");
    help=true;
    ecode=1;
  }

  bool camera_count_ok = false;
  int num_cameras = camera_count.toInt(&camera_count_ok);
  if(!camera_count_ok) {
    fprintf(stderr,"Invalid number of cameras!\n");
    help=true;
    ecode=1;
  }

  if (help) {
    printf("SSL-Vision command line options:\n");
    printf(" -s        Start capture immediately\n");
    printf(" -a        Set Processor Affinity\n");
    printf(" -c <n>    Set Number of Cameras\n");
    printf(" --help    Show this help\n");
    exit(ecode);
  }

  printPathWarning();

  MainWindow mainWin(start, enforce_affinity, num_cameras);
  mainWinPtr = &mainWin;
  mainWin.show();
  mainWin.init();

  int retval = app.exec();

  return retval;
}
