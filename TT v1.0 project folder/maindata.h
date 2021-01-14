//#ifndef MAINDATA_H
//#define MAINDATA_H

//#include <QObject>
//#include <QAbstractTableModel>
////Date
//#include <QDate>
//#include <QDateTime>
////Threads
//#include <QtConcurrent>
//#include <QThread>
//#include <QFutureWatcher>

//class MainData : public QAbstractTableModel
//{
//    Q_OBJECT

//private:
//    QList<QList<QString> > table;
//    //first and last date to create array of time intervals
//    QDate firstDate;
//    QDate lastDate;

//public:
//    MainData(QObject *parent=NULL);
//    MainData(const QList<QList<QString> > &t, QObject *parent=NULL);

//    void setTable(const QList<QList<QString> > &t);
//    void setFirstLastDate(const QDate &first, const QDate &last);
//    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//    //get 2d QList array of summed data
//    static void sumData(QList<int> &row);
//    QList<QList<int> > getSummedData();

//    //for debugging
//    QString debugData(int i, int j);

//};

//#endif // MAINDATA_H
