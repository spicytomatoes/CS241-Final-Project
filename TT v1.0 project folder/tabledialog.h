#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>
#include <QTableView>
#include "summedtablemodel.h"

namespace Ui {
class TableDialog;
}

class TableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableDialog(QWidget *parent = nullptr);
    explicit TableDialog(QWidget *parent = nullptr, SummedTableModel *table = nullptr);
    ~TableDialog();

private:
    Ui::TableDialog *ui;
};

#endif // TABLEDIALOG_H
