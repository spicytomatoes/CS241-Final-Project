#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "summedtablemodel.h"
#include "summedchart.h"

#include <QMainWindow>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
//debug
#include <QDebug>
#include <QElapsedTimer>
#include "tabledialog.h"
//location
#include <QGeoRectangle>
#include <QGeoLocation>
#include <QGeoAddress>
#include <QGeoCodingManager>
#include <QGeoCoordinate>
#include <QGeoServiceProvider>
//data view & widgets
#include <QTableView>
//Files
#include <QDirIterator>
#include <QFile>
//Dialogs
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
//Threads
#include <QtConcurrent>
#include <QThread>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QHash<QString, QList<int> > getSummedData(const QList<QString> &fileNames);

public slots:
    void enableGraphButtons(bool flag);
    void updateStartFiler(const QDateTime &datetime);
    void updateEndFilter(const QDateTime &datetime);
    void timeFilterUpdated();
    void updateGridFilter(const int &gridID);
    void updateConstantInfo();
    void updateLocationInfoToUi(QGeoCodeReply *reply);
    void updateVariableInfo();
    void updateGridInfo(const int &gridID);
    void updateGridInfoToUi();
    void updateRouteInfo(const int &gridID);

private slots:
    void on_actionOpen_triggered();

    void on_actionview_table_triggered();

    void on_startEndSelector_currentIndexChanged(int index);

    void on_SelectAllButton_clicked();

    void on_clearButton_clicked();

    void on_selectGraphComboBox_currentIndexChanged(int index);

    void on_tabWidget_currentChanged(int index);

    void on_hMstartButton_clicked();

    void on_hMEndButton_clicked();

    void on_hMNoneButton_clicked();

private:
    Ui::MainWindow *ui;
    QQmlApplicationEngine *engine;
    //geocoding
    QGeoServiceProvider *geoSrv;
    QGeoCodingManager *geoCoder;
    //model and view
    SummedTableModel *summedTableModel;
    SummedChart *summedChart;
    //filters
    QDateTimeEdit *startDTFilter;
    QDateTimeEdit *endDTFilter;
    QObject *gridFilterItem;
    //map
    QObject *mapViewerItem;
};

#endif // MAINWINDOW_H
