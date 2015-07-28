#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief MainWindow::MainWindow
 * @param parent
 * MainWindow constructor
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType< Mat >("Mat");
    connect(&vt, SIGNAL(flowRate(int,double)), this, SLOT(on_flowRate(int,double)));
    connect(&vt, SIGNAL(showResult(Mat)), this, SLOT(on_showResult(Mat)));
    connect(&vt, SIGNAL(videoProgress(double)), this, SLOT(on_videoProgress(double)));
    connect(&vt, SIGNAL(finished(bool)), this, SLOT(on_finished(bool)));
    ui->diameterLineEdit->setValidator(new QDoubleValidator(0, 5000, 2, this));
    ui->heightLineEdit->setValidator(new QDoubleValidator(0, 5000, 2, this));
    initPlot();
}



MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::on_finished
 * @param yes
 * Receive signal from videoThread. When video processing finishes, UI events:
 * 1. Destroy Result window
 * 2. Display a messagebox that notify user it's ended.
 * 3. Progress bar sets back to 0
 * 4. Save csv button becomes enabled.
 * Thread event:
 * 1. Stop video thread.
 */

void MainWindow::on_finished(bool yes)
{
    if(yes){
        //Stop video thread
        vt.Stop = true;

        //UI event 1
        destroyWindow("Result");
        //UI event 2
        QString info = "Overflow processing has finished. Resulting video saved to:\n\n" + QString::fromStdString(vt.videoSaveAddress);
        QMessageBox::information(
                    this,
                    tr(""),
                    info
                    );
        //UI event 3
        ui->progressBar->setValue(0);
        //UI event 4
        ui->savePushButton->setEnabled(true);
    }

}


/**
 * @brief MainWindow::on_videoProgress
 * @param progress
 * Receive signal from videothread about current progress.
 * UI event:
 * 1. Update progress bar in real-time.
 */
void MainWindow::on_videoProgress(double progress)
{
    ui->progressBar->setValue(progress);
}

/**
 * @brief MainWindow::on_showResult
 * @param result
 *
 */
void MainWindow::on_showResult(Mat result)
{
    imshow("Result", result);
    //waitKey(5);
}

void MainWindow::on_flowRate(int frameNumber, double flowRateAmount)
{
    double timeStamp = frameNumber * 1.0 / vt.fps; //5 is target fps

    this->x.push_back(timeStamp);
    this->y.push_back(flowRateAmount);
    if(timeStamp > this->xRng)
    {
        ui->plotWidget->xAxis->setRange(0, timeStamp);
        this->xRng = timeStamp;
    }
    if(flowRateAmount > this->yRng) {
        this->yRng = flowRateAmount;
        ui->plotWidget->yAxis->setRange(0, flowRateAmount);
    }
    ui->plotWidget->graph(0)->setData(this->x, this->y);
    ui->plotWidget->replot();
}

void MainWindow::initPlot()
{
    // create graph and assign data to it:
    ui->plotWidget->addGraph(0);
    x.resize(0);
    y.resize(0);
    ui->plotWidget->graph(0)->setData(this->x, this->y);
    // give the axes some labels:
    ui->plotWidget->xAxis->setLabel("Elapsed Time (s)");
    ui->plotWidget->yAxis->setLabel("Flow Rate (GPM)");
    // set axes ranges, so we see all data:
    ui->plotWidget->xAxis->setRange(0, this->xRng);
    ui->plotWidget->yAxis->setRange(0, this->yRng);
    ui->plotWidget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->plotWidget->replot();
}

void MainWindow::on_openPushButton_clicked()
{
    QString selectedVideoString =  QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "D://",
                "Video Files (*.avi && *.mp4 && *.mov && *.mkv)"
                );
    if (selectedVideoString.size()!=0)
    {
        this->vt.videoFileName = selectedVideoString.toStdString();
        //show filenames in table widget
        ui->tableWidget->insertRow(0);
        QTableWidgetItem *item = new QTableWidgetItem(selectedVideoString);
        ui->tableWidget->setItem(0, 0, item);
    }
}

void MainWindow::on_runPushButton_clicked()
{
    if (this->vt.videoFileName.size()!=0)
    {
        if (ui->diameterLineEdit->text().isEmpty() || ui->heightLineEdit->text().isEmpty())
        {
            QMessageBox::information(
                        this,
                        tr(""),
                        tr("Please specify pipe diameter and height.")
                        );
        } else {
            ui->savePushButton->setEnabled(false);
            this->initPlot();
            //Set stop flag to false
            vt.D = ui->diameterLineEdit->text().toDouble();
            vt.H = ui->heightLineEdit->text().toDouble();
            this->vt.Stop = false;
            //Run
            this->vt.start();
        }

    }
}

void MainWindow::on_stopPushButton_clicked()
{
    this->vt.Stop = true;
}

void MainWindow::on_actionOpen_triggered()
{
    this->on_openPushButton_clicked();
}

void MainWindow::on_savePushButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save to csv", "FlowRate.csv", "CSV files (*.csv);", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
        {
            QTextStream output(&data);
            for (int i = 0; i < x.size(); i++)
            {
                output << x[i] << "," << y[i] << endl;
            }
            //output << "'your csv content';'more csv content'";
    }
}

void MainWindow::on_actionRun_triggered()
{
    this->on_runPushButton_clicked();
}

void MainWindow::on_actionStop_triggered()
{
    this->on_stopPushButton_clicked();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();

}

void MainWindow::on_actionAbout_triggered()
{
    aboutDialog mDialog;
    mDialog.setModal(true);
    mDialog.exec();
}

void MainWindow::on_diameterPushButton_clicked()
{
    QMessageBox::information(
                this,
                tr(""),
                tr("Pipe diameter takes dimension of pipe diameter as argument, in inches.")
                );
}

void MainWindow::on_pushButton_clicked()
{
    QMessageBox::information(
                this,
                tr(""),
                tr("Height means the vertical distance between the bottom of the horizontal pipe and the water plane, in inches.")
                );
}


