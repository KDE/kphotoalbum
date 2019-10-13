#include "MainWindow.h"
#include "PathMapper.h"
#include "PathMappingDialog.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QLabel>
#include <QToolButton>

MainWindow *MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_instance = this;
    m_executor = new Executor(this);
    connect(ui->quit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
    connect(&PathMapper::instance(), &PathMapper::setupChanged, this, &MainWindow::populateMappings);
    populateMappings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMessage(const QString &message)
{
    m_instance->ui->output->appendPlainText(message);
}

void MainWindow::populateMappings()
{
    qDeleteAll(ui->pathMappings->children());

    auto layout = new QGridLayout(ui->pathMappings);
    int row = -1;

    for (const PathMapper::Mapping &mapping : PathMapper::instance().mappings()) {
        layout->addWidget(new QLabel("<b>Linux Path:</b>"), ++row, 0);
        layout->addWidget(new QLabel(mapping.linuxPath), row, 1);
        layout->addWidget(new QLabel("<b>Host Path:</b>"), row, 2);
        layout->addWidget(new QLabel(mapping.hostPath), row, 3);

        auto edit = new QToolButton;
        edit->setText("Edit");
        connect(edit, &QToolButton::clicked, [linuxPath = mapping.linuxPath, hostPath = mapping.hostPath] { PathMapper::instance().configurePath(linuxPath, hostPath); });
        layout->addWidget(edit, row, 4);

        auto remove = new QToolButton;
        remove->setText("Remove");
        connect(remove, &QToolButton::clicked, [linuxPath = mapping.linuxPath, hostPath = mapping.hostPath] { PathMapper::instance().removeMapping(linuxPath, hostPath); });
        layout->addWidget(remove, row, 5);

        layout->setColumnStretch(6, 1);
    }
}
