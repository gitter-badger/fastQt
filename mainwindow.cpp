#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mTabWidget = new QTabWidget;
    setCentralWidget(mTabWidget);

    setMenuBar(new QMenuBar());

    menuBar()->addMenu("File")->addAction("Open file");

    addFile("/home/sacha/Bioinfo/data/Mucobiome/raw/small.fastq");

    addToolBar("test")->addAction("run", this, SLOT(run()));


}

MainWindow::~MainWindow()
{

}

void MainWindow::addFile(const QString &filename)
{

    MainAnalyseWidget * w = new MainAnalyseWidget(filename);
    mainList.append(w);
    mTabWidget->addTab(w, w->windowIcon(), w->windowTitle());




}

void MainWindow::run()
{

    for (MainAnalyseWidget * widget : mainList)
    {
        QtConcurrent::run(widget, &MainAnalyseWidget::runAllAnalysis);
    }


}


