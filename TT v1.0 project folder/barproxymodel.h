//proxy model for bar charts
#ifndef BARPROXYMODEL_H
#define BARPROXYMODEL_H

#include <QObject>
#include <QIdentityProxyModel>
#include "summedtablemodel.h"

class BarProxyModel : public QIdentityProxyModel
{
private:
    QDateTime startTime;
public:
    BarProxyModel(QObject* parent = 0);
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void setStartTime(QDateTime t);
};

#endif // BARPROXYMODEL_H
