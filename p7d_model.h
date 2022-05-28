#ifndef P7_DUMP_MODEL
#define P7_DUMP_MODEL

#include "importer.h"
#include <QAbstractTableModel>
#include <QString>

namespace p7 {

class P7DumpModel : public QAbstractTableModel
{
public:

    enum class Columns {
        Number = 0,
        ID,
        Level,
        Module,
        CPUNumber,
        Thread,
        File,
        Line,
        Function,
        Time,
        Text,
        Count
    };

    void setDumpData(const p7DumpData & data);

    QString hostName() const;
    QString processName() const;
    QString processDateTimeAsString() const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnWidth(int columnIndex) const;

private:

    p7DumpData _data;
};

}

#endif
