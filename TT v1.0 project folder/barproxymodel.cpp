#include "barproxymodel.h"


BarProxyModel::BarProxyModel(QObject *parent) : QIdentityProxyModel(parent)
{

}
//sums total filtered data according to the hour of the day (index)
QVariant BarProxyModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
          return QIdentityProxyModel::data(index, role);

    double sum = 0;
    int count = 0;
    QModelIndex index2 = QIdentityProxyModel::index(0,0);
    int k =  QIdentityProxyModel::data(index2, role).toDateTime().secsTo(startTime) / 3600;
    for (int i = index.row() + k; i < QIdentityProxyModel::rowCount(); i+=24)
    {
        index2 = QIdentityProxyModel::index(i,index.column());
        sum += QIdentityProxyModel::data(index2, role).toDouble();
        count++;
    }
    if (count!=0) sum/=count;

    return sum;
}

int BarProxyModel::rowCount(const QModelIndex &parent) const
{
    return 24;
}

void BarProxyModel::setStartTime(QDateTime t)
{
    startTime = t;
}
