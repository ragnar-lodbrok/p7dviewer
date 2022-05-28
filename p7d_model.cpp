#include "p7d_model.h"

#include <QGuiApplication>
#include <QPalette>

namespace p7 {

void P7DumpModel::setDumpData(const p7DumpData & data)
{
    size_t oldRowsCount = _data.traceDataCount();
    if (oldRowsCount) {
        beginRemoveRows(QModelIndex(), 0, oldRowsCount-1);
        _data = {};
        endRemoveRows();
    }

    int newRowsCount = data.traceDataCount();
    if (newRowsCount) {
        beginInsertRows(QModelIndex(), 0, newRowsCount-1);
        _data = data;
        endInsertRows();
    } else {
        _data = data;
    }
}

QString P7DumpModel::hostName() const
{
    return _data.hostName();
}

QString P7DumpModel::processName() const
{
    return _data.processName();
}

QString P7DumpModel::processDateTimeAsString() const
{
    return _data.processDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

int P7DumpModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(Columns::Count);
}

int P7DumpModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _data.traceDataCount();
}

Qt::ItemFlags P7DumpModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled;
}

QVariant P7DumpModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (static_cast<Columns>(section)) {
        case Columns::Number:
            return QString(tr("#"));
        case Columns::ID:
            return QString(tr("ID"));
        case Columns::Level:
            return QString(tr("Level"));
        case Columns::Module:
            return QString(tr("Module"));
        case Columns::CPUNumber:
            return QString(tr("CPU#"));
        case Columns::Thread:
            return QString(tr("Thread"));
        case Columns::File:
            return QString(tr("File"));
        case Columns::Line:
            return QString(tr("Line"));
        case Columns::Function:
            return QString(tr("Function"));
        case Columns::Time:
            return QString(tr("Time"));
        case Columns::Text:
            return QString(tr("Text"));
        default:
            break;
        }
    }

    return QVariant();
}

QVariant P7DumpModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= (int)_data.traceDataCount()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {

        const p7TraceDataInfo & data = _data.traceDataAt((size_t)index.row());

        switch (static_cast<Columns>(index.column())) {

        case Columns::Number:
            return index.row() + 1;

        case Columns::ID:
            return data.id;

        case Columns::Level:
            return traceLevelAsString(data.verbosity);

        case Columns::Module:
            return data.moduleName.isEmpty()
                    ? QString::number(data.moduleId)
                    : data.moduleName
                        + "(" + QString::number(data.moduleId) + ")";

        case Columns::CPUNumber:
            return data.processorNumber;

        case Columns::Thread:
            return data.threadName.isEmpty()
                    ? "0x" + QString::number(data.threadId, 16)
                    : data.threadName
                        + "(0x" + QString::number(data.threadId, 16) + ")";

        case Columns::File:
            return data.filename;

        case Columns::Line:
            return data.line;

        case Columns::Function:
            return data.function;

        case Columns::Time:
            return data.time.toString("HH:mm:ss.zzz");

        case Columns::Text:
            return data.message;

        default:
            break;
        }

    } else if (role == Qt::BackgroundRole) {

        const p7TraceDataInfo & data = _data.traceDataAt((size_t)index.row());

        switch (data.verbosity) {

        case eP7Trace_Level::EP7TRACE_LEVEL_WARNING: {

                QColor bgColor = QGuiApplication::palette()
                    .color(QPalette::Window);

                // R+G = yellow
                bgColor.setRedF(qMin(bgColor.redF() + 0.1, 1.0));
                bgColor.setGreenF(qMin(bgColor.greenF() + 0.1, 1.0));

                return bgColor;
            }

        case eP7Trace_Level::EP7TRACE_LEVEL_ERROR:
        case eP7Trace_Level::EP7TRACE_LEVEL_CRITICAL: {

                QColor bgColor = QGuiApplication::palette()
                    .color(QPalette::Window);

                // Red
                bgColor.setRedF(qMin(bgColor.redF() + 0.1, 1.0));

                return bgColor;
            }

        default:
            break;
        }

    }

    return QVariant();
}

int P7DumpModel::columnWidth(int columnIndex) const
{
    switch (static_cast<Columns>(columnIndex)) {
    case Columns::Module:
        return 200;
    case Columns::Thread:
        return 150;
    case Columns::File:
        return 800;
    case Columns::Function:
        return 700;
    case Columns::Time:
        return 120;
    case Columns::Text:
        return 1000;
    default:
        return 100;
    }
}

}
