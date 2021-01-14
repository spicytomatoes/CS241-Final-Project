#ifndef BARMODEL_H
#define BARMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QDateTime>

class BarModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    //this stores time interval - number of orders - avg time - avg rev - total rev
    QList<QList<int> > barTable;
    //this stores key: (gridid, time interval) value: number of orders - total time taken - total revenue
    QHash<QString, QList<int> > table;
    //to generate summedTable
    int startTime, endTime;
    //filters
    QDateTime startFilter, endFilter;
    int startRow = 0;
    QList<int> locationFilter;
    bool isStartLocation = true;

public:
    BarModel(QObject *parent=NULL);
    BarModel(const QList<QList<int> >, QObject *parent=NULL);

    void setTable(const QHash<QString, QList<int> > &t);
    void updateSummedTable();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int getStartTime();
    int getEndTime();
    int getMaxVal(const int &col);
    int getMinVal(const int &col);

    QString debugData(int i, int j);

//public slots:
    void updateStartFilter(const QDateTime &datetime);
    void updateEndFilter(const QDateTime &datetime);
    void updateLocationFilter(int gridID);
    void flipLocationFilter(bool isStart);
    void fillLocationFilter();
    void emptyLocationFilter();
};

#endif // BARMODEL_H
