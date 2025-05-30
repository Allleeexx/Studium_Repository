#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <string>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QMainWindow *parent = nullptr): QWidget(parent)
    {
        edit = new QLineEdit();
        button = new QPushButton("Click");
        label = new QLabel();
        label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        grid = new QGridLayout();
        grid->addWidget(edit, 0, 0);
        grid->addWidget(button, 1, 0);
        grid->addWidget(label, 2, 0);
        setLayout(grid);
        connect(button, SIGNAL(clicked()), this, SLOT(changeText()));
    }

    ~MainWidget()
    {
        delete grid;
        delete edit;
        delete button;
        delete label;
    }

private slots:
    void changeText()
    {
        label->setText(edit->text());
    }

private:
    QGridLayout *grid;
    QLineEdit *edit;
    QPushButton *button;
    QLabel *label;
    std::string text;
};

#endif // UI_WIDGET_H
