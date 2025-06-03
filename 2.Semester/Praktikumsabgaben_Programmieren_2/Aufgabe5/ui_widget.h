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
#include "vertical.h"
#include "ballistic.h"

using namespace std;


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


        //Objekte anlegen
        ufo = new Ballistic("ufo1", 20.0, 20.0);
        uthread = new UfoThread(ufo);

        connect(start_button, SIGNAL(clicked()), this, SLOT(startUfo()));
        connect(uthread, SIGNAL(stopped(vector<float>)), this, SLOT(updateWindow(vector<float>)));
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
        delete ufo;
        delete uthread;
    }

private slots:    
    void startUfo(){
        QString x_eingabewert = edit_x->text();
        QString y_eingabewert = edit_y->text();
        QString height_eingabewert = edit_height->text();
        QString speed_eingabewert = edit_speed->text();

        bool xok = Numbercontrol(x_eingabewert);
        bool yok = Numbercontrol(y_eingabewert);
        bool heightok = Numbercontrol(height_eingabewert);
        bool speedok = Numbercontrol(speed_eingabewert);


        if((xok == true) && (yok == true)&&(heightok == true)&&(speedok == true)){
            float x_wert = x_eingabewert.toFloat();
            float y_wert = y_eingabewert.toFloat();
            float height_wert = height_eingabewert.toFloat();
            int speed_wert = speed_eingabewert.toInt();

            //flieg los
            uthread->startUfo(x_wert,y_wert,height_wert,speed_wert);
            vector<float> pos = ufo->getPosition();

            //Ausgabepart
            QString labelcontent;
            labelcontent += "Started at\n";
            labelcontent += "Position:\n";
            labelcontent += QString::number(pos[0], 'f',2) + "|"
                            +  QString::number(pos[1], 'f', 2) + "|"
                            +  QString::number(pos[2], 'f', 2) + "meter";
            label->setText(labelcontent);

            start_button->setText("Flying");
            start_button->setEnabled(false);
        }

        if(xok == false){
            edit_x->setText("Ungültig");
            label->setText("\n\n\n");
        }
        if(yok == false){
            edit_y->setText("Ungültig");
            label->setText("\n\n\n");
        }
        if(heightok == false){
            edit_height->setText("Ungültig");
            label->setText("\n\n\n");
        }
        if(speedok == false){
            edit_speed->setText("Ungültig");
            label->setText("\n\n\n");
        }
    }

    void updateWindow(vector<float> pos){

        //Ausgabepart
        QString labelcontent;
        labelcontent += "Flight completed at\n";
        labelcontent += "Position:\n";
        labelcontent += QString::number(pos[0], 'f',2) + " | "
                        +  QString::number(pos[1], 'f', 2) + " | "
                        +  QString::number(pos[2], 'f', 2) + "meter";
        label->setText(labelcontent);

        start_button->setText("Start");
        start_button->setEnabled(true);
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
    string text;
    Ufo *ufo;
    UfoThread *uthread;

    bool Numbercontrol(const QString& input){
        if(input.isEmpty()){
            return false;
        }

        int punkte = 0;
        int minuse = 0;

        for(int i=0; i<input.length(); i++){
            QChar buchstabe = input[i];
            if(buchstabe == '.'){
                punkte++;
                if(punkte >1){
                    return false;
                }
            }else if(buchstabe == '-'){
                minuse ++;
                if(minuse > 1){
                    return false;
                }
            }else if(buchstabe.isDigit() == false){
                return false;
            }
        }
        return true;
    }
};

#endif // UI_WIDGET_H
