#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QComboBox>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_serialPort = new QSerialPort();

    m_portNameList = getPortNameList();

    ui->comboBoxPortName->addItems(m_portNameList);

    QStringList bautRatesList;
    bautRatesList << "1200" << "2400" << "4800" << "9600" << "19200" << "57600" << "115200";
    ui->comboBoxBaudRate->addItems(bautRatesList);
    ui->comboBoxBaudRate->setCurrentIndex(3);

    //ui->txtSend->setText("发送命令");
}

MainWindow::~MainWindow()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
    }
    delete m_serialPort;

    delete ui;
}

QStringList MainWindow::getPortNameList()
{
    QStringList m_serialPortName;
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        m_serialPortName << info.portName();
        qDebug()<<"serialPortName:"<<info.portName();
    }
    return m_serialPortName;
}

void MainWindow::on_btnOpenCOM_clicked()
{
    if (ui->btnOpenCOM->text()=="打开串口")
    {
        if(m_serialPort->isOpen())
        {
            m_serialPort->clear();
            m_serialPort->close();
        }

        m_serialPort->setPortName(m_portNameList[ui->comboBoxPortName->currentIndex()]);

        if(!m_serialPort->open(QIODevice::ReadWrite))
        {
            qDebug()<<m_portNameList[ui->comboBoxPortName->currentIndex()]<<"打开失败!";
            return;
        }

        //打开成功
        m_serialPort->setBaudRate(ui->comboBoxBaudRate->currentText().toInt(),QSerialPort::AllDirections);//设置波特率和读写方向
        m_serialPort->setDataBits(QSerialPort::Data8);              //数据位为8位
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);   //无流控制
        m_serialPort->setParity(QSerialPort::NoParity);             //无校验位
        m_serialPort->setStopBits(QSerialPort::OneStop);            //一位停止位

        connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(receiveInfo()));

        ui->btnOpenCOM->setText("关闭串口");
    } else
    {
        m_serialPort->close();
        ui->btnOpenCOM->setText("打开串口");
    }
}

void MainWindow::receiveInfo()
{
    qDebug()<<"receiveInfo()";
    QByteArray info = m_serialPort->readAll();

    QString strReceiveData = "";
    if(ui->checkBoxHexReceive->isChecked())
    {
        QByteArray hexData = info.toHex();
        strReceiveData = hexData.toUpper();

        qDebug()<<"接收到串口数据: "<<strReceiveData;

        for(int i=0; i<strReceiveData.size(); i+=2+1)
            strReceiveData.insert(i, QLatin1String(" "));
        strReceiveData.remove(0, 1);

        qDebug()<<"处理后的串口数据: "<<strReceiveData;

        ui->txtReceiveData->append(strReceiveData);
    }
    else
    {
        strReceiveData = info;

        //避免中文乱码
        QTextCodec *tc = QTextCodec::codecForName("GBK");
        QString tmpQStr = tc->toUnicode(info);

        ui->txtReceiveData->append(tmpQStr);
    }

    //ui->txtReceiveData->append("\r\n");
}

//另一个 函数 char 转为 16进制
char convertCharToHex(char ch)
{
    /*
    0x30等于十进制的48，48也是0的ASCII值，，
    1-9的ASCII值是49-57，，所以某一个值－0x30，，
    就是将字符0-9转换为0-9
    */
    if((ch >= '0') && (ch <= '9'))
         return ch-0x30;
     else if((ch >= 'A') && (ch <= 'F'))
         return ch-'A'+10;
     else if((ch >= 'a') && (ch <= 'f'))
         return ch-'a'+10;
     else return (-1);
}

//基本和单片机交互 数据 都是16进制的 我们这里自己写一个 Qstring 转为 16进制的函数
void convertStringToHex(const QString &str, QByteArray &byteData)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    byteData.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = convertCharToHex(hstr);
        lowhexdata = convertCharToHex(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        byteData[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    byteData.resize(hexdatalen);
}

void MainWindow::on_btnSendData_clicked()
{
    QString m_strSendData = ui->txtSend->text();

    if(ui->checkBoxHexSend->isChecked())
    {
        if (m_strSendData.contains(" "))
        {
            m_strSendData.replace(QString(" "),QString(""));    //把空格去掉
        }

        QByteArray sendBuf;

        convertStringToHex(m_strSendData, sendBuf);             //把QString 转换 为 hex

        m_serialPort->write(sendBuf);
    }
    else
    {
        m_serialPort->write(m_strSendData.toLocal8Bit());
    }
}
