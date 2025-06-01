#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <QMainWindow>
#include "ui_widget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow()
    {
        QIcon icon("../Aufgabe5/thi_icon.png");
        setWindowTitle("UFO");
        setWindowIcon(icon);
        //resize(200, 25);
        mainwidget = new MainWidget(this);
        setCentralWidget(mainwidget);
    }

    ~MainWindow()
    {
        delete mainwidget;
    }

private:
    MainWidget *mainwidget;
};

#endif // MAINWINDOW_H
