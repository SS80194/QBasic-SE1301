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
    Program *program_temp;

    void setUIForDebugMode();
    void setUIExitDebugMode();

public:
    volatile bool waitInput = false;
    int inputValue = 0;
    void askForInput(const QString& s);

    friend class Program;
    bool parseCommand(const QString& s);
    bool askAndLoadProgram();
    bool loadProgram(const QString& filename);
    bool executeProgram();
    bool clearProgram();
    void updateBreakPoint(const QString& s);
    void updateVariables(const QString& s);
    void updateOutput(const QString& s);
    void resumeProgram();
    void ExitDebugMode();
};
#endif // MAINWINDOW_H
