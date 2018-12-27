#include <QTextBlock>
#include <QHostInfo>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

void menuAction(QMenu* fileMenu, MainWindow* win, QString name,
                  QObject* ptr, std::function<void()> slot){
    QAction* act = new QAction(name, win);
    QObject::connect(act, &QAction::triggered, ptr, slot);
    fileMenu->addAction(act);
}


#define ADDTEST(NAME) ::menuAction(m_testMenu, this, "&" #NAME, _test, std::bind(&MiniClient::test ## NAME, _test))

QStringList IPInfo(){
    QStringList addressInfo;
    QHostInfo info;
    addressInfo.append(info.localHostName());
    QHostInfo info2(QHostInfo::fromName(addressInfo[0]));
    QList<QHostAddress>hostaddr = info2.addresses();
    for(auto addr : hostaddr)
        addressInfo.append(addr.toString());
    return addressInfo;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    m_testMenu = ui->menuBar->addMenu(tr("&Test"));

    ::menuAction(m_testMenu, this, "&Tell more", this, [this]{addLine("Modbus test application");});

    connect(ui->clientBtn, &QPushButton::released, this, &MainWindow::clientSelected);
    connect(ui->serverBtn, &QPushButton::released, this, &MainWindow::serverSelected);
 }



void MainWindow::showEvent(QShowEvent* event){
    QMainWindow::showEvent(event);
    if(ui->consoleText->document()->lineCount() <= 1){
        QFontMetrics fm(ui->consoleText->font());
        const int rows = (ui->consoleText->height()) / (fm.lineSpacing());
        ui->consoleText->appendPlainText(QString(rows - 1, '\n'));
        addLine("Qt ModBus Test, libmodbus:" + ModBUS::ModBusLibrary::version());
        auto ips = IPInfo();
        for(auto b = ips.begin(); b != ips.end(); b++)
            addLine(*b);
    }
}

MainWindow::~MainWindow(){
    if(m_conn != nullptr)
        m_conn->close();
    delete ui;
}

void MainWindow::addLine (QString line) {
    QTextCursor cursor(ui->consoleText->document()->begin().next());
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
    ui->consoleText->appendPlainText(line);
}

void MainWindow::addLine (QString line) const {
   const_cast<MainWindow*>(this)->addLine(line);
}


void MainWindow::enableUi(bool enable){
    ui->serverBtn->setEnabled(enable);
    ui->clientBtn->setEnabled(enable);
    ui->ipaddress->setEnabled(enable);
    ui->ipport->setEnabled(enable);
}

bool MainWindow::address(QHostAddress& addr, int& port) const {
    QString ps = ui->ipport->text();
    bool ok = false;
    port = ps.toInt(&ok);
    if(!ok)
        port = ModBUS::ModbusTcp::TCP_DEFAULT_PORT;

    QString str = ui->ipaddress->text();
    return (str.length() > 0 && addr.setAddress(str));
}

void MainWindow::clientSelected(){
    QHostAddress addr;
    int port;
    if(address(addr, port)){
        enableUi(false);
        addLine("Client mode");
        MiniClient* test = new MiniClient(addr, port, *this, this);
        m_conn = test;
        connect(test, &MiniClient::connected, this, &MainWindow::clientTestUi);
        connect(test, &MiniClient::error, this, &MainWindow::clientError);
    }
}

void MainWindow::clientError(QString reason){
    addLine(reason + " had errors.");
    delete m_conn;
    m_conn = nullptr;
    enableUi(true);
}

void MainWindow::clientTestUi(MiniClient* _test){
    ADDTEST(Calculator);
    ADDTEST(123);
    ADDTEST(Read3);
    ADDTEST(AllRegisters);
}


void MainWindow::serverSelected(){
    enableUi(false);
    addLine("Server mode");
    QHostAddress addr;
    int port;
    const bool validAddr = address(addr, port);
    MiniServer* ms = new MiniServer(validAddr ? &addr : NULL, port, *this, this);
    connect(qApp, &QCoreApplication::aboutToQuit, ms, &MiniServer::aboutToQuit);
}



