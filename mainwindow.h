#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QSignalMapper>
#include <windows.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    virtual bool winEvent(MSG *message, long *result);

private slots:
    void clicked(int index);

private:
    QList<QPushButton*> buttons;
    void muteMixer(UINT uMxId, LONG fValue);
    QStringList getMixers();
    QSignalMapper *signalMapper;
    void hotkey();
    int index;
    int numDevs;
};

#endif // MAINWINDOW_H
