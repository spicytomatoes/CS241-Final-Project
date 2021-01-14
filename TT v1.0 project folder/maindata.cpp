//#include "maindata.h"
//#include <QDebug>
//#include <QElapsedTimer>
//#include <QProgressDialog>

//MainData::MainData(QObject *parent)
//    : QAbstractTableModel(parent)
//{
//}

//MainData::MainData(const QList<QList<QString> > &t, QObject *parent)
//    : QAbstractTableModel(parent)
//{
//    setTable(t);
//}

//void MainData::setTable(const QList<QList<QString> > &t)
//{
//    if (t.size() == 0) return;
//    emit beginResetModel();
//    table = t;
//    QDateTime first, last;
//    first.setSecsSinceEpoch(table.first()[1].toInt());
//    last.setSecsSinceEpoch(table.last()[1].toInt());
//    firstDate = first.date();
//    lastDate = last.date();
//}

//void MainData::setFirstLastDate(const QDate &first, const QDate &last)
//{
//    firstDate = first;
//    lastDate = last;
//}

//int MainData::rowCount(const QModelIndex & /*parent*/) const
//{
//   return table.size();
//}

//int MainData::columnCount(const QModelIndex & /*parent*/) const
//{
//    if (table.size() > 0) return table[0].size();
//    return 0;
//}

//QVariant MainData::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant();
//    if (index.row() >= table.size() || index.row() < 0)
//        return QVariant();
//    if (index.column() >= table[index.row()].size() || index.column() < 0)
//        return QVariant();

//    if (role == Qt::DisplayRole)
//    {
//        return table[index.row()][index.column()];
//    }

//    return QVariant();
//}

//QString MainData::debugData(int i, int j)
//{
//    return table[i][j];
//}

////QList<int> mapTable(const QList<QString> &t)
////{

////    QList<int> mapped;
////    QDateTime dateTime;
////    dateTime.setSecsSinceEpoch(t[1].toInt());
////    QTime time = dateTime.time();
////    time.setHMS(time.hour(),0,0);
////    dateTime.setTime(time);
////    mapped.append(dateTime.toSecsSinceEpoch());
////    mapped.append(1);

////    return mapped;
////}

////void reduceTable(QList<QList<int> > &result, const QList<int> &row)
////{
////    for (int i = 0; i < result.size(); i++)
////    {
////        if (result[i][0] == row[0])
////        {
////            result[i][1]++;
////            return;
////        }
////    }
////    result.append(row);
//////    if (result.isEmpty()) { result.append(row); }
//////    else if (result.last()[0] == row[0])
//////    {
//////        result.last()[0]++;
//////    }
//////    else result.append(row);
////}

////QList<QList<int> > MainData::getSummedData()
////{
////    QList<QList<int> > summedData;

////    QElapsedTimer timer;
////    timer.start();

////    //Progress dialog
////    QProgressDialog pDialog;
////    pDialog.setLabelText("Summing data");

////    //Load data into 2d QList array using mappedReduced
////    QFutureWatcher<QList<QList<int> >> watcher;
////    QFuture<QList<QList<int> > > future = QtConcurrent::mappedReduced(table,mapTable,reduceTable,QtConcurrent::OrderedReduce);
////    watcher.setFuture(future);

////    connect(&pDialog, &QProgressDialog::canceled, &watcher, &QFutureWatcher<QList<QList<int> >>::cancel);
////    connect(&watcher, &QFutureWatcher<QList<QList<int> >>::finished, &pDialog, &QProgressDialog::reset);
////    connect(&watcher, &QFutureWatcher<QList<QList<int> >>::progressRangeChanged, &pDialog, &QProgressDialog::setRange);
////    connect(&watcher, &QFutureWatcher<QList<QList<int> >>::progressValueChanged, &pDialog, &QProgressDialog::setValue);

////    pDialog.exec();
////    watcher.waitForFinished();

////    summedData = future.result();
////    std::sort(summedData.begin(),summedData.end());

////    qDebug() << "generating summed data took " << timer.elapsed() << "ms";

////    return summedData;
////}
