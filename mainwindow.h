#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "program.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_cmdLineEdit_editingFinished();

private:
    Ui::MainWindow *ui;
    Program *program;

    void setUIForDebugMode();
    void setUIExitDebugMode();

public:
    friend class Program;
    bool parseCommand(const QString& s);
    bool askAndLoadProgram();
    bool loadProgram(const QString& filename);
    bool executeProgram();
    bool clearProgram();
};
#endif // MAINWINDOW_H
