#include "main_window.h"
#include <QtWidgets>

namespace p7 {
namespace ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(QSize(700, 400));
    setWindowTitle("P7D Viewer");

    _centralWidget = new CentralWidget(&_model);
    setCentralWidget(_centralWidget);
}

void MainWindow::importP7Dump(const QString & filename)
{
    p7::p7DumpImporter importer;
    p7::p7DumpData data = importer.import(filename.toStdString());

    _model.setDumpData(std::move(data));
    _centralWidget->showModelData();
}

CentralWidget::CentralWidget(p7::P7DumpModel * model,
                             QWidget *parent)
    : QWidget(parent)
    , _model(model)
{
    createWidgets();

    setAcceptDrops(true);
}

void CentralWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl url : urls) {
            if (url.fileName().endsWith(".p7d")) {
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void CentralWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl url : urls) {
            if (url.fileName().endsWith(".p7d")) {
                event->acceptProposedAction();
                static_cast<MainWindow *>(parent())->importP7Dump(url.path());
                break;
            }
        }
    }

    event->acceptProposedAction();
}

void CentralWidget::createWidgets()
{
    QVBoxLayout * mainLayout = new QVBoxLayout();

    QHBoxLayout * processDataLayout = new QHBoxLayout();

    _hostNameLabel = new QLabel(tr("Host:"));
    _hostNameValue = new QLabel();

    _processNameLabel = new QLabel(tr("Process:"));
    _processNameValue = new QLabel();

    _processDateTimeValue = new QLabel();

    processDataLayout->addWidget(_hostNameLabel);
    processDataLayout->addWidget(_hostNameValue);
    processDataLayout->addStretch(1);

    processDataLayout->addWidget(_processNameLabel);
    processDataLayout->addWidget(_processNameValue);
    processDataLayout->addStretch(1);

    processDataLayout->addWidget(_processDateTimeValue);
    processDataLayout->addStretch(10);

    _traceTable = new QTableView();
    _traceTable->verticalHeader()->hide();
    _traceTable->horizontalHeader()->setHighlightSections(false);
    _traceTable->verticalHeader()->setDefaultSectionSize(24);
    _traceTable->setSortingEnabled(false);
    _traceTable->setModel(_model);

    for (int i=0; i<_model->columnCount(); ++i) {
        _traceTable->setColumnWidth(i, _model->columnWidth(i));
    }

    mainLayout->addLayout(processDataLayout);
    mainLayout->addWidget(_traceTable);

    mainLayout->setAlignment(processDataLayout, Qt::AlignTop | Qt::AlignLeft);

    setLayout(mainLayout);
}

void CentralWidget::showModelData()
{
    _hostNameValue->setText(_model->hostName());
    _processNameValue->setText(_model->processName());
    _processDateTimeValue->setText(_model->processDateTimeAsString());
}

} // namespace ui
} // namespace p7d

