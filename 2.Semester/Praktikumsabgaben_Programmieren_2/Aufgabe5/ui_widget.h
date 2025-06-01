#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <string>
#include "ufo_thread.h"
#include "ufo.h"

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QMainWindow *parent = nullptr): QWidget(parent){



        //eingabefelder
        edit_x = new QLineEdit();
        edit_y = new QLineEdit();
        edit_height = new QLineEdit();
        edit_speed = new QLineEdit();

        //labels
        text_x = new QLabel("X: ");
        text_y = new QLabel("Y: ");
        text_height = new QLabel("Height: ");
        text_speed = new QLabel("Speed: ");

        //Start Button
        start_button = new QPushButton("Start");

        //Großes Label
        label = new QLabel();
        label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        label->setFrameShape(QFrame:: Box);
        label->setFrameShadow(QFrame::Plain);
        label->setMinimumHeight(80);


        //Layout
        grid = new QGridLayout();
        grid->addWidget(text_x, 0, 0);
        grid->addWidget(edit_x, 0, 1);

        grid->addWidget(text_y, 1, 0);
        grid->addWidget(edit_y, 1, 1);

        grid->addWidget(text_height, 2, 0);
        grid->addWidget(edit_height, 2, 1);

        grid->addWidget(text_speed, 3, 0);
        grid->addWidget(edit_speed, 3, 1);

        grid->addWidget(start_button, 4,0,1,2);

        grid->addWidget(label, 5,0,3,2);

        setLayout(grid);

        connect(start_button, SIGNAL(clicked()), this, SLOT(startUfo()));
    }

    ~MainWidget(){
        delete grid;
        delete edit_x;
        delete edit_y;
        delete edit_height;
        delete edit_speed;
        delete start_button;
        delete label;
        delete text_x;
        delete text_y;
        delete text_height;
        delete text_speed;
    }

private slots:    
    void startUfo(){
        start_button->
    }

private:
    QGridLayout *grid;
    QLineEdit *edit_x;
    QLineEdit *edit_y;
    QLineEdit *edit_height;
    QLineEdit *edit_speed;
    QPushButton *start_button;
    QLabel *label;
    QLabel *text_x;
    QLabel *text_y;
    QLabel *text_height;
    QLabel *text_speed;
    std::string text;
    Ufo *ufo;
    UfoThread *uthread;
};

#endif // UI_WIDGET_H
