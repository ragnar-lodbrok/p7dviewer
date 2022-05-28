#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTableView>
#include "p7d_model.h"

namespace p7 {
namespace ui {

class CentralWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void importP7Dump(const QString & filename);

private:

    CentralWidget * _centralWidget;

    p7::P7DumpModel _model;
};

class CentralWidget : public QWidget
{
public:
    CentralWidget(p7::P7DumpModel * model,
                  QWidget *parent = nullptr);

    void showModelData();

protected:

    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:

    void createWidgets();

    QLabel * _hostNameLabel;
    QLabel * _hostNameValue;

    QLabel * _processNameLabel;
    QLabel * _processNameValue;

    QLabel * _processDateTimeValue;

    QTableView * _traceTable;

    p7::P7DumpModel * _model;
};

} // namespace ui
} // namespace p7

#endif // UI_MAINWINDOW_H
