#include "summedtablemodel.h"

SummedTableModel::SummedTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    locationFilter.clear();
    //fill location filter
    for (int i = 0; i < 100; i++) locationFilter.append(i);

}

SummedTableModel::SummedTableModel(const QHash<QString, QList<int> > &t, QObject *parent)
    : QAbstractTableModel(parent)
{
    locationFilter.clear();
    //fill location filter
    for (int i = 0; i < 100; i++) locationFilter.append(i);
    setTable(t);
}

void SummedTableModel::setTable(const QHash<QString, QList<int> > &t)
{
    QHash<QString, QList<int> >::const_iterator i = t.find("dateRange");

    startTime = i.value()[0];
    endTime = i.value()[1];

    table = t;
    startFilter.setSecsSinceEpoch(startTime);
    endFilter.setSecsSinceEpoch(endTime);

    updateSummedTable();
}

//row count equal to number of hours in the selected range
int SummedTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    int s = startFilter.toSecsSinceEpoch();
    int e = endFilter.toSecsSinceEpoch();

    int rows = (e-s) / 3600 ;

    return rows;
}

int SummedTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    if (summedTable.size() > 1) return summedTable[0].size();
    return 0;
}

QVariant SummedTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();   
    if (index.column() >= summedTable[index.row()].size() || index.column() < 0)
        return QVariant();

    int row = index.row() + startRow;

    if (row >= summedTable.size() || row < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        //return corresponding types
        if (index.column() == 0)
        {
            QDateTime dateTime;
            dateTime.setSecsSinceEpoch(summedTable[row][0]);
            return dateTime.toMSecsSinceEpoch();
        }
        if (index.column() == 3 || index.column() == 4)
        {
            return (double)summedTable[row][index.column()] / 100;
        }
        return summedTable[row][index.column()];
    }

    return QVariant();
}

int SummedTableModel::getStartTime()
{
    return startTime;
}

int SummedTableModel::getEndTime()
{
    return endTime;
}

int SummedTableModel::getMaxVal(const int &col)
{
    if (col <= 0) return 0;
    if (summedTable.size() == 0) return 0;
    if (summedTable[0].size() <= col) return 0;

    int max = INT32_MIN;
    for (int i = 0; i < summedTable.size(); i++)
    {
        if (summedTable[i][col] > max) max = summedTable[i][col];
    }
    if (col == 3 || col == 4)
    {
        return (double)max / 100;
    }
    return max;
}

int SummedTableModel::getMinVal(const int &col)
{
    if (col <= 0) return 0;
    if (summedTable.size() == 0) return 0;
    if (summedTable[0].size() <= col) return 0;

    int min = INT32_MAX;
    for (int i = 0; i < summedTable.size(); i++)
    {
        if (summedTable[i][col] < min) min = summedTable[i][col];
    }
    if (col == 3 || col == 4)
    {
        return (double)min / 100;
    }
    return min;
}

//heat map filtered by time
QVariantList SummedTableModel::getHeatMap(bool isStart)
{
    QElapsedTimer timer;
    timer.start();

    QVariantList heatMap;
    int c = isStart ? 0 : 100;

    int max = 0;
    for (int i = 0+c; i < 100+c; i++)
    {
        //heat is the sum of orders
        int heat = 0;
        for (int timeID = startFilter.toSecsSinceEpoch(); timeID < endFilter.toSecsSinceEpoch(); timeID+=3600)
        {
            //generate key
            QString key = QString("%1").arg(i,3,10,QChar('0'));
            key.append(QString("%1").arg(timeID));

            if (table.contains(key))
            {
                QHash<QString, QList<int> >::const_iterator it = table.find(key);
                heat += it.value()[0];
            }
        }
        if (max < heat) max = heat;
        heatMap.append(heat);
    }
    heatMap.append(max);

    qDebug() << "getHeatMap took " << timer.elapsed() << "ms";

    return heatMap;
}

//get number of orders from gridID (if gridID is between 100-199, then get orders TO gridID)
int SummedTableModel::getDemand(int gridID)
{
    int demand = 0;
    for (int timeID = startFilter.toSecsSinceEpoch(); timeID < endFilter.toSecsSinceEpoch(); timeID+=3600)
    {
        //generate key
        QString key = QString("%1").arg(gridID,3,10,QChar('0'));
        key.append(QString("%1").arg(timeID));

        if (table.contains(key))
        {
            QHash<QString, QList<int> >::const_iterator it = table.find(key);
            demand += it.value()[0];
        }
    }
    return demand;
}

//heat map of destinations of gridID, not filtered by time
QVariantList SummedTableModel::getDestinationsHeatMap(int gridID)
{
    QVariantList heatMap;
    QString key = QString("%1").arg(gridID,3,10,QChar('0'));
    int max = 0;
    if (table.contains(key))
    {
        QHash<QString, QList<int> >::const_iterator it = table.find(key);
        for (int i = 0; i < 100; i++)
        {
            if (it.value()[i] > max) max = it.value()[i];
            heatMap.append(it.value()[i]);
        }
    } else qDebug() << "Can't find Key: " << key;

    heatMap.append(max);

    return heatMap;
}

//get orders, average time and price in the given route
int SummedTableModel::getRouteRemand(int startGridID, int endGridID)
{
    QString key = QString("%1").arg(startGridID,3,10,QChar('0'));

    if (table.contains(key))
    {
        QHash<QString, QList<int> >::const_iterator it = table.find(key);
        return it.value()[endGridID];
    } else qDebug() << "Can't find Key: " << key;

    return -1;
}

QTime SummedTableModel::getRouteAverageTime(int startGridID, int endGridID)
{
    QString key = QString("%1").arg(startGridID,3,10,QChar('0'));

    if (table.contains(key))
    {
        QHash<QString, QList<int> >::const_iterator it = table.find(key);
        int demand = it.value()[endGridID];
        int seconds = it.value()[endGridID+100];
        if (demand != 0) seconds /= demand;

        QTime time = QTime(0,0,0).addSecs(seconds);

        return time;
    } else qDebug() << "Can't find Key: " << key;

    return QTime();
}

double SummedTableModel::getRouteAveragePrice(int startGridID, int endGridID)
{
    QString key = QString("%1").arg(startGridID,3,10,QChar('0'));

    if (table.contains(key))
    {
        QHash<QString, QList<int> >::const_iterator it = table.find(key);
        int demand = it.value()[endGridID];
        double price = it.value()[endGridID+200];
        if (demand != 0) price /= (100*demand);
        return price;
    } else qDebug() << "Can't find Key: " << key;

    return -1;
}

void SummedTableModel::updateStartFilter(const QDateTime &datetime)
{
    beginResetModel();
    startFilter = QDateTime(datetime);
    startRow = (startFilter.toSecsSinceEpoch() - startTime) / 3600;
    endResetModel();
}

void SummedTableModel::updateEndFilter(const QDateTime &datetime)
{
    beginResetModel();
    endFilter = QDateTime(datetime);
    endResetModel();
}

void SummedTableModel::updateLocationFilter(int gridID)
{
    bool isRemoved;
    if (!isStartLocation) gridID+=100;
    int index = locationFilter.indexOf(gridID);
    if (index == -1) {
        locationFilter.append(gridID);
        isRemoved = false;
    } else {
        locationFilter.removeAt(index);
        isRemoved = true;
    }
    updateSummedTable();
}

void SummedTableModel::flipLocationFilter(bool isStart)
{
    if (isStartLocation == isStart) return;
    isStartLocation = isStart;
    if (isStartLocation) {
        for (int i = 0; i < locationFilter.size(); i++)
            locationFilter[i] -= 100;
    } else {
        for (int i = 0; i < locationFilter.size(); i++)
            locationFilter[i] += 100;
    }
    updateSummedTable();
}

void SummedTableModel::fillLocationFilter()
{
    locationFilter.clear();
    int c = isStartLocation ? 0 : 100;
    for (int i = 0; i < 100; i++) locationFilter.append(i+c);
    updateSummedTable();
}

void SummedTableModel::emptyLocationFilter()
{
    locationFilter.clear();
    updateSummedTable();
}

// generate summed table from table
void SummedTableModel::updateSummedTable()
{
    QElapsedTimer timer;
    timer.start();

    beginResetModel();

    summedTable.clear();

    for (int timeID = startTime; timeID < endTime; timeID+=3600)
    {
        QList<int> row = {timeID, 0, 0, 0, 0};
        //sum all locations filter in result;
        for (int i = 0; i < locationFilter.size(); i++)
        {
            //generate key
            QString key = QString("%1").arg(locationFilter[i],3,10,QChar('0'));
            key.append(QString("%1").arg(timeID));

            if (table.contains(key))
            {
                QHash<QString, QList<int> >::const_iterator it = table.find(key);
                row[1] += it.value()[0];    //demand
                row[2] += it.value()[1];    //total time, to be divided by demand
                row[3] += it.value()[2];    //revenue

            }    
        }
        row[4] = row[3];            //total revenue, to be summed
        //get avg time
        if(row[1]!= 0) row[2] /= row[1];

        summedTable.append(row);
    }

    for (int i = 1; i < summedTable.size(); i++)
    {
        summedTable[i][4] += summedTable[i-1][4];
    }

    qDebug() << "updateSummedTable took " << timer.elapsed() << "ms";

    endResetModel();

}



