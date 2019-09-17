#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>

static const QString masterNode = "8";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fillNodeButtons();
    fillDeviceList();
    connect(&tmrRead, &QTimer::timeout, this, &MainWindow::onTimeToReadDevice);
    connect(ui->txtCustomCmd, &QLineEdit::returnPressed, this, &MainWindow::on_pbSendCustomCmd_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbDeviceRefreshList_clicked()
{
    fillDeviceList();
}

void MainWindow::onNodeButtonToggled(bool checked)
{
    if (!checked) return;
    currentNode = static_cast<QPushButton *>(sender())->text();
}

void MainWindow::onTimeToReadDevice()
{
    if (!serialPort.canReadLine())
        return;
    QByteArray lnArr = serialPort.readLine();
    QString ln = QString::fromLatin1(lnArr);
    logEvent(Receive, ln);
}

void MainWindow::on_pbDeviceOpen_clicked()
{
    QString portName = ui->cbDeviceList->currentText();
    QString msg = QString("Try to open port '%0'...").arg(portName);
    serialPort.close();
    serialPort.setBaudRate(QSerialPort::Baud9600);
    serialPort.setPortName(portName);
    bool success = serialPort.open(QIODevice::ReadWrite);
    if (success)
        logEvent(Info, msg + " Success");
    else
    {
        logEvent(Info, msg);
        logEvent(Error, "Failed. "+serialPort.errorString());
    }
    tmrRead.start(200);
}

void MainWindow::on_pbDeviceClose_clicked()
{
    tmrRead.stop();
    serialPort.close();
}

void MainWindow::on_pbClearLog_clicked()
{
    ui->txtLog->clear();
}

void MainWindow::fillNodeButtons()
{
    static const QStringList NodeButtonNames = { masterNode, "10", "20", "30", "40", "50", "60", "70", "80", "90" };
    for (int current = 0; current < NodeButtonNames.count(); current++)
    {
        int row = current / 3;
        int col = current % 3;
        QPushButton *btn = new QPushButton(NodeButtonNames[current]);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        connect(btn, &QAbstractButton::toggled, this, &MainWindow::onNodeButtonToggled);
        ui->wgtNodeButtonsLayout->addWidget(btn, row, col);
        nodeButtons.append(btn);
    }
    nodeButtons.first()->toggle();
}

void MainWindow::sendATCommand(const QString &cmd)
{
    QString fullCmd = QString("AT+%0").arg(cmd);
    doSendATCommand(fullCmd);
}

void MainWindow::sendATCommand(const QString &cmd, const QString &node)
{
    QString fullCmd = QString("AT+%0%1").arg(cmd, node);
    doSendATCommand(fullCmd);
}

void MainWindow::sendATCommand(const QString &cmd, const QString &node, const QString &num)
{
    QString fullCmd = QString("AT+%0%1,%2").arg(cmd, node, num);
    doSendATCommand(fullCmd);
}

void MainWindow::sendATCommand(const QString &cmd, const QString &node, const QString &num, const QString &state)
{
    QString fullCmd = QString("AT+%0%1,%2,%3").arg(cmd, node, num, state);
    doSendATCommand(fullCmd);
}

void MainWindow::doSendATCommand(const QString &fullCmd)
{
    QString theCmd = fullCmd + "\n";
    logEvent(Send, fullCmd);
    int res = serialPort.write(theCmd.toStdString().c_str());
    if (res < 0)
        logEvent(Error, "Error. "+serialPort.errorString());
}

void MainWindow::fillDeviceList()
{
    QString currentPortName = ui->cbDeviceList->currentText();
    ui->cbDeviceList->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, ports)
    {
        ui->cbDeviceList->addItem(info.portName());
    }
    ui->cbDeviceList->setCurrentText(currentPortName);
    if (ui->cbDeviceList->currentIndex() < 0 && ui->cbDeviceList->count() > 0)
        ui->cbDeviceList->setCurrentIndex(0);
}

void MainWindow::logEvent(MainWindow::LogType logType, const QString &text)
{
    QString finalText = "%0";
    switch (logType) {
    case MainWindow::Error:
        finalText = "<span style=\"color: red\">%0</span>";
        break;
    case MainWindow::Send:
        finalText = "<span style=\"color: green\">%0</span>";
        break;
    case MainWindow::Receive:
        finalText = "<span style=\"color: blue\">%0</span>";
        break;
    default:
        break;
    }
    ui->txtLog->append(finalText.arg(text));
}

void MainWindow::on_pbPing_clicked()
{
    sendATCommand("PING?", currentNode);
}

void MainWindow::on_pbVersion_clicked()
{
    sendATCommand("VERSION?", currentNode);
}

void MainWindow::on_pbCapabilities_clicked()
{
    sendATCommand("CAPABILITIES?", currentNode);
}

void MainWindow::on_pbWaterOn_clicked()
{
    sendATCommand("WATER=", currentNode, "0", "1");
}

void MainWindow::on_pbWaterOff_clicked()
{
    sendATCommand("WATER=", currentNode, "0", "0");
}

void MainWindow::on_pbWaterState_clicked()
{
    sendATCommand("WATER?", currentNode, "0");
}

void MainWindow::on_pbSoilState_clicked()
{
    sendATCommand("SOIL?", currentNode, "0");
}

void MainWindow::on_pbPingAll_clicked()
{
    sendATCommand("PING*");
}

void MainWindow::on_pbDcAdapterOn_clicked()
{
    sendATCommand("DCADAPTER=", masterNode, "0", "1");
}

void MainWindow::on_pbDcAdapterOff_clicked()
{
    sendATCommand("DCADAPTER=", masterNode, "0", "0");
}

void MainWindow::on_pbDcAdapterState_clicked()
{
    sendATCommand("DCADAPTER?", masterNode, "0");
}

void MainWindow::on_pbPumpOn_clicked()
{
    sendATCommand("PUMP=", masterNode, "0", "1");
}

void MainWindow::on_pbPumpOff_clicked()
{
    sendATCommand("PUMP=", masterNode, "0", "0");
}

void MainWindow::on_pbPumpState_clicked()
{
    sendATCommand("PUMP?", masterNode, "0");
}

void MainWindow::on_pbPressureState_clicked()
{
    sendATCommand("PRESSURE?", masterNode, "0");
}

void MainWindow::on_pbVoltageState_clicked()
{
    sendATCommand("VOLTAGE?", masterNode, "0");
}

void MainWindow::on_pbCurrentState_clicked()
{
    sendATCommand("CURRENT?", masterNode, "0");
}

void MainWindow::on_pbSendCustomCmd_clicked()
{
    doSendATCommand(ui->txtCustomCmd->text());
}
