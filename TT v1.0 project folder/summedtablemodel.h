#ifndef SUMMEDDATA_H
#define SUMMEDDATA_H

#include <QObject>
#include <QAbstractTableModel>

//Threads
#include <QtConcurrent>
#include <QThread>
#include <QFutureWatcher>

class SummedTableModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    //this stores time interval - number of orders - avg time - avg rev - total rev
    //this is generated from table
    QList<QList<int> > summedTable;
    //this stores key: (gridid, time interval) value: number of orders - total time taken - total revenue
    //also stores key: (dateRange) start date and end date,
    //and key: (gridID) list of orders, travel time and fees for each destination
    QHash<QString, QList<int> > table;
    //to generate summedTable
    int startTime, endTime;
    //filter stuff
    QDateTime startFilter, endFilter;
    int startRow = 0;
    QList<int> locationFilter;
    bool isStartLocation = true;

public:
    SummedTableModel(QObject *parent=NULL);
    SummedTableModel(const QHash<QString, QList<int> > &t, QObject *parent=NULL);

    void setTable(const QHash<QString, QList<int> > &t);
    void updateSummedTable();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int getStartTime();
    int getEndTime();
    int getMaxVal(const int &col);
    int getMinVal(const int &col);
    QVariantList getHeatMap(bool isStart);
    int getDemand(int gridID);
    QVariantList getDestinationsHeatMap(int gridID);
    int getRouteRemand(int startGridID, int endGridID);
    QTime getRouteAverageTime(int startGridID, int endGridID);
    double getRouteAveragePrice(int startGridID, int endGridID);

    QString debugData(int i, int j);

    void updateStartFilter(const QDateTime &datetime);
    void updateEndFilter(const QDateTime &datetime);
    void updateLocationFilter(int gridID);
    void flipLocationFilter(bool isStart);
    void fillLocationFilter();
    void emptyLocationFilter();
};
#endif // SUMMEDDATA_H
