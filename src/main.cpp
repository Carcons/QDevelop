
#include "MainWindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <chrono>
#include <thread>


const unsigned int SLEEP = 4000;


int main(int argc, char** argv) {


    QApplication app(argc, argv);

    //Splashscreen
	QSplashScreen splash(QPixmap(":/icons/app.png"), Qt::WindowStaysOnTopHint);
    splash.show();
	std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP));

    qdev::MainWindow* window = qdev::MainWindow::instance();
    window->show();
	splash.finish(window);

    return app.exec();

}


