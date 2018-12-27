#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "miniconnection.h"

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow, Printer
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void clientTestUi(MiniClient* _test);
    void clientError(QString reason);
private:
    void clientSelected();
    void serverSelected();
    void addLine(QString text);
    void addLine(QString text) const;
    void enableUi(bool enable);
    bool address(QHostAddress& addr, int& port) const;
    void showEvent(QShowEvent* event);
    void print(QString string) const {addLine(string);}
private:
private:
    Ui::MainWindow *ui;
    MiniConnection* m_conn;
    QMenu* m_testMenu;
};

#endif // MAINWINDOW_H
