#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "program.h"
#include <QFileDialog>
#include <QMessageBox>
#include "config.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setUIExitDebugMode();

    connect(ui->btnDebugMode, &QPushButton::clicked, this, &MainWindow::setUIForDebugMode);
    connect(ui->btnExitDebugMode, &QPushButton::clicked, this, &MainWindow::setUIExitDebugMode);
    
    program = new Program(this);

    connect(ui->btnLoadCode, &QPushButton::clicked, this, &MainWindow::askAndLoadProgram);
    connect(ui->btnRunCode, &QPushButton::clicked, this, &MainWindow::executeProgram);
    connect(ui->btnClearCode, &QPushButton::clicked, this, &MainWindow::clearProgram);
    
    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text();
    ui->cmdLineEdit->setText("");

    if(waitInput){
        bool ok;
        if (cmd.startsWith("?")) {
            cmd = cmd.mid(1);
            qDebug() << "cmd: " << cmd;
        }
        int value = cmd.toInt(&ok);
        if (ok) {
            inputValue = value;
            waitInput = false;
            return ;
        } else {
            QMessageBox::warning(this, "Input Error", "The input is not a valid integer.");
            ui->cmdLineEdit->setText("?");
            return ;
        }
    }
    //ui->CodeDisplay->append(cmd);
    parseCommand(cmd);
}

void MainWindow::setUIForDebugMode(){
    ui->btnClearCode->setVisible(false);
    ui->btnLoadCode->setVisible(false);
    ui->btnDebugMode->setVisible(false);

    ui->btnExitDebugMode->setVisible(true);
    ui->btnDebugResume->setVisible(true);

    ui->labelSyntaxTree->setVisible(false);
    ui->treeDisplay->setVisible(false);

    ui->labelMonitor->setVisible(true);
    ui->monitorDisplay->setVisible(true);
    ui->labelBreakPoints->setVisible(true);
    ui->breakPointsDisplay->setVisible(true);
}

void MainWindow::setUIExitDebugMode(){
    ui->btnClearCode->setVisible(true);
    ui->btnLoadCode->setVisible(true);
    ui->btnDebugMode->setVisible(true);

    ui->btnExitDebugMode->setVisible(false);
    ui->btnDebugResume->setVisible(false);

    ui->labelSyntaxTree->setVisible(true);
    ui->treeDisplay->setVisible(true);

    ui->labelMonitor->setVisible(false);
    ui->monitorDisplay->setVisible(false);
    ui->labelBreakPoints->setVisible(false);
    ui->breakPointsDisplay->setVisible(false);
}

/* MainWindow::parseCommand
* Parse string s and execute corresponding command.
* Valid commands:
* 1. LOAD: open a window to load a program
* 2. RUN: execute the program
* 3. CLEAR: clear the program
* 4. QUIT: exit the program
* 5. LIST: do nothing
* 6. <number> <statement>: update the statement of the program

*/
bool MainWindow::parseCommand(const QString& s)
{
    //split the command into two parts,seperated by the first space
    //store in argv0 and argv1.
    QString trimmed = s.trimmed();
    int firstSpaceIndex = trimmed.indexOf(' ');
    QString argv0,argv1;
    argv0 = trimmed.left(firstSpaceIndex);
    if(firstSpaceIndex != -1) argv1 = trimmed.mid(firstSpaceIndex + 1).trimmed();
    else argv1 = "";
    
    if (QString::compare(argv0, "LOAD") == 0) {
        // Handle LOAD command
        if(askAndLoadProgram()) return true;
    } else if (QString::compare(argv0, "RUN") == 0) {
        // Handle RUN command
        if(program->execute()) return true;
    } else if (QString::compare(argv0, "CLEAR") == 0) {
        // Handle CLEAR command
        program->clear();
        return true;
    } else if (QString::compare(argv0, "QUIT") == 0) {
        // Handle QUIT command
        exit(0);
        return true;
    } else if (QString::compare(argv0, "LIST") == 0) {
        // Handle LIST command(Abandoned)
        return true;
    } else if (argv0.toInt()) {
        // Handle <number> <statement> command
        if(program->updateStatement(argv0.toInt(), argv1)) 
            return true;
    } 
    // Default:Handle invalid command
    return false;
}

bool MainWindow::askAndLoadProgram(){
    if(debugMode) {
        loadProgram(testFilename);
        executeProgram();
        return true;
    }
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Program"), "", tr("Program Files (*.txt *.bas)"));
    if (!filename.isEmpty()) {
        return loadProgram(filename);
    }
    return false;
}

bool MainWindow::loadProgram(const QString& filename){
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Failed to open file: "<<filename;
        return false;
    }
    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        if(!parseCommand(line)){
            QMessageBox::information(this, "错误", "选中文件无法解析");
            program->clear();
            return false;
        }
    }
    return true;
}

bool MainWindow::executeProgram(){
    return program->execute();
}

bool MainWindow::clearProgram(){
    program->clear();
    return true;
}

