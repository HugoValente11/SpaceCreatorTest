#include <exceptions.h>
#include <mscchart.h>
#include <mscdocument.h>

#include <QtTest>

using namespace msc;

class tst_MscDocument : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testName();
    void testAddDocument();
    void DocumentInstance();
    void testNoNullPtrDocument();
    void testAddChart();
    void testNoDuplicateChart();
    void testNoNullPtrChart();

private:
    MscDocument *m_document = nullptr;
};

void tst_MscDocument::init()
{
    m_document = new MscDocument;
}

void tst_MscDocument::cleanup()
{
    delete m_document;
    m_document = nullptr;
}

void tst_MscDocument::testName()
{
    QCOMPARE(m_document->name(), QString());

    const QString chartName("Rosetta");
    m_document->setName(chartName);
    QCOMPARE(m_document->name(), chartName);
}

void tst_MscDocument::testAddDocument()
{
    QCOMPARE(m_document->documents().size(), 0);

    auto doc1 = new MscDocument("Doc01", m_document);
    m_document->addDocument(doc1);
    auto doc2 = new MscDocument("Doc02", m_document);
    m_document->addDocument(doc2);
    QCOMPARE(m_document->documents().size(), 2);
}

void tst_MscDocument::DocumentInstance()
{
    auto doc = new MscDocument("Doc", m_document);
    m_document->addDocument(doc);
    m_document->addDocument(doc);
    QCOMPARE(m_document->documents().size(), 1);
}

void tst_MscDocument::testNoNullPtrDocument()
{
    m_document->addDocument(nullptr);
    QCOMPARE(m_document->documents().size(), 0);
}

void tst_MscDocument::testAddChart()
{
    QCOMPARE(m_document->charts().size(), 0);

    auto doc1 = new MscChart("Chart01", m_document);
    m_document->addChart(doc1);
    auto doc2 = new MscChart("Chart02", m_document);
    m_document->addChart(doc2);
    QCOMPARE(m_document->charts().size(), 2);
}

void tst_MscDocument::testNoDuplicateChart()
{
    auto chart = new MscChart("Chart", m_document);
    m_document->addChart(chart);
    m_document->addChart(chart);
    QCOMPARE(m_document->charts().size(), 1);
}

void tst_MscDocument::testNoNullPtrChart()
{
    m_document->addChart(nullptr);
    QCOMPARE(m_document->charts().size(), 0);
}

QTEST_APPLESS_MAIN(tst_MscDocument)

#include "tst_mscdocument.moc"
