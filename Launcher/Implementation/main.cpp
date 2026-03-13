#include <QApplication>
#include <HomepageWindow.h>

int main(int argc, char*argv[]) {
    QApplication launcherApp(argc, argv);
    HomepageWindow homeWindow;
    homeWindow.show();

    return launcherApp.exec();
}
