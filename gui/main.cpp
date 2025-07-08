#include <iostream>
#include <QApplication>
#include <QDir>
#include <QTcpServer>
#include <QDebug>
#include "mainwindow.h"
#include "lib/beam.h"

quint16 getFreePort()
{
  QTcpServer server;
  // Bind to port 0 to get an available port
  if (server.listen(QHostAddress::Any, 0))
  {
    quint16 freePort = server.serverPort();
    // Close as we just want to know the port number
    server.close();
    return freePort;
  }
  else
  {
    qDebug() << "Failed to find a free port.";
    return 0; // Return 0 if no free port found
  }
}

int main(int argc, char *argv[])
{
  quint16 port = 5555;
  bool devMode = false;
  std::cout << "Starting Tau5..." << std::endl;

  if (argc > 1 && std::strcmp(argv[1], "dev") == 0)
  {
    std::cout << "Development mode enabled." << std::endl;
    devMode = true;
  }
  else
  {
    std::cout << "Production mode enabled." << std::endl;
    port = getFreePort();
  }

#if defined(Q_OS_WIN)
  if (devMode)
  {
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
    {
      FILE *stream;
      freopen_s(&stream, "CONOUT$", "w", stdout);
      freopen_s(&stream, "CONOUT$", "w", stderr);
    }
  }
#endif

  // Set environment variables for Chromium
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
          "--disable-background-timer-throttling "
          "--disable-renderer-backgrounding "
          "--disable-backgrounding-occluded-windows "
          "--disable-features=AudioServiceOutOfProcess "
          "--autoplay-policy=no-user-gesture-required");

  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);

  QApplication app(argc, argv);

  Q_INIT_RESOURCE(Tau5);
  app.setApplicationName(QObject::tr("Tau5"));
  app.setStyle("gtk");

  // Get a free port

  qDebug() << "Using port: " << port;
  QString appDirPath = QCoreApplication::applicationDirPath();
  QDir dir(appDirPath);

 // TODO : Make less brittle wrt platform build dirs
#if defined(Q_OS_WIN)
  dir.cd("../../../server");
#elif defined(Q_OS_MACOS)
  dir.cd("../../../../../server");
#else
  dir.cd("../../server");
#endif

  QString basePath = dir.absolutePath();
  qDebug() << "Base path: " << basePath;
  qDebug() << dir.entryList();

  Beam *beam = new Beam(&app, basePath, "tau5", "0.1.0", port, devMode);
  MainWindow mainWindow(port);

#if defined(Q_OS_WIN)
  mainWindow.setWindowIcon(QIcon(":/images/app.ico"));
#endif

  mainWindow.show();
  return app.exec();
}
