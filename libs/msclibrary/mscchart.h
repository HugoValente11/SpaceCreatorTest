#ifndef MSCCHART_H
#define MSCCHART_H

#include <QString>
#include <QVector>

namespace msc {
class MscInstance;
class MscMessage;

class MscChart
{
public:
    explicit MscChart(const QString &name = "");
    ~MscChart();

    const QString &name() const;
    void setName(const QString &name);

    const QVector<MscInstance *> &instances() const;
    void addInstance(MscInstance *instance);

    MscInstance *instanceByName(const QString &name);

    const QVector<MscMessage *> &messages() const;
    void addMessage(MscMessage *message);

    MscMessage *messageByName(const QString &name);

private:
    QString m_name;
    QVector<MscInstance *> m_instances;
    QVector<MscMessage *> m_messages;
};

} // namespace msc

#endif // MSCCHART_H
