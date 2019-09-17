#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

namespace Ui {
class MainWindow;
}
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onNodeButtonToggled(bool checked);

    void on_pbDeviceRefreshList_clicked();

    void on_pbDeviceOpen_clicked();

    void on_pbClearLog_clicked();

    void on_pbDeviceClose_clicked();

    void on_pbPing_clicked();

    void on_pbVersion_clicked();

    void on_pbCapabilities_clicked();

    void on_pbWaterOn_clicked();

    void on_pbWaterOff_clicked();

    void on_pbWaterState_clicked();

    void on_pbSoilState_clicked();

    void on_pbPingAll_clicked();

    void on_pbDcAdapterOn_clicked();

    void on_pbDcAdapterOff_clicked();

    void on_pbDcAdapterState_clicked();

    void on_pbPumpOn_clicked();

    void on_pbPumpOff_clicked();

    void on_pbPumpState_clicked();

    void on_pbPressureState_clicked();

    void on_pbVoltageState_clicked();

    void on_pbCurrentState_clicked();

private:
    Ui::MainWindow *ui;
    QList<QPushButton *> nodeButtons;
    QString currentNode;
    QSerialPort serialPort;

    void fillDeviceList();
    void fillNodeButtons();
    void sendATCommand(const QString &cmd);
    void sendATCommand(const QString &cmd, const QString &node);
    void sendATCommand(const QString &cmd, const QString &node, const QString &num);
    void sendATCommand(const QString &cmd, const QString &node, const QString &num, const QString &state);
    void doSendATCommand(const QString &fullCmd);

    enum LogType
    {
        Info, Error, Send, Receive
    };
    void logEvent(LogType logType, const QString &text);
};

#endif // MAINWINDOW_H
