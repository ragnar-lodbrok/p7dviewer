////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2020 (c) Baical                                                        /
// 2022 (c) Ragnar Lodbrok                                                     /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////


#include <QApplication>
#include "main_window.h"

#ifdef Q_OS_WIN
    #include <Windows.h> // SetProcessDPIAware()
#endif // Q_OS_WIN


int main(int argc, char *argv[])
{

#ifdef Q_OS_WIN
    // fix scaling issues on win (TODO: rm when upgrade from Qt 5.6 ?)
    ::SetProcessDPIAware();
#endif // Q_OS_WIN

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // before app!

    QApplication a(argc, argv);
    a.addLibraryPath(QCoreApplication::applicationDirPath() + "/imageformats");

    QCoreApplication::setOrganizationName("p7dviewer");
    QCoreApplication::setApplicationName("p7dviewer");

    p7::ui::MainWindow mainWindow;
    mainWindow.showMaximized();

    return a.exec();
}
