#ifndef STATICRESULTMODEL_H
#define STATICRESULTMODEL_H

#include <QAbstractTableModel>
#include <QImage>
#include <QList>

class StaticResultModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StaticResultModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void clear();
    void addResultData(QString fileName,
                       QImage image,
                       int method,
                       double minVal,
                       double maxVal,
                       QPoint minLoc,
                       QPoint maxLoc);
    QString exportResults() const;

private:
    struct ResultData
    {
        QString fileName;
        QImage image;
        int method;
        double minVal;
        double maxVal;
        QPoint minLoc;
        QPoint maxLoc;
    };
    QList<ResultData> m_results;
};

#endif // STATICRESULTMODEL_H
