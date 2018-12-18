/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include <exceptions.h>
#include <mscchart.h>
#include <msccondition.h>
#include <mscdocument.h>
#include <mscfile.h>
#include <mscinstance.h>
#include <mscmessage.h>
#include <mscmodel.h>
#include <msctimer.h>

#include <QtTest>

using namespace msc;

class tst_MscFile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testFileOpenError();
    void testSyntaxError();
    void testEmptyDocument();
    void testComments();
    void testNestedDocuments();
    void testMscInDocument();
    void testInstance();
    void testInstanceWithKind();
    void testInstanceWithInheritance();
    void testMessage();
    void testSameMessageInTwoInstances();
    void testMessageWithParameters();
    void testGateMessage();
    void testCondition();
    void testTimer();
    void testSortedMessage();
    void testSortedMessageTwoCharts();
    void testSortedInstanceEvents();
    void testHierarchy();

private:
    MscFile *file = nullptr;
};

void tst_MscFile::init()
{
    file = new MscFile;
}

void tst_MscFile::cleanup()
{
    delete file;
    file = nullptr;
}

void tst_MscFile::testFileOpenError()
{
    QVERIFY_EXCEPTION_THROWN(file->parseFile("some_dummy_file_name"), FileNotFoundException);
}

void tst_MscFile::testSyntaxError()
{
    QString fileName = QFINDTESTDATA("syntax_error.msc");
    QVERIFY_EXCEPTION_THROWN(file->parseFile(fileName), ParserException);
}

void tst_MscFile::testEmptyDocument()
{
    MscModel *model = file->parseText("MSCDOCUMENT CU_level;\nENDMSCDOCUMENT;");
    QCOMPARE(model->documents().size(), 1);
    QCOMPARE(model->documents().at(0)->name(), QString("CU_level"));
    delete model;

    model = file->parseText("mscdocument inst_1_cu_nominal.cu_controller;\nendmscdocument;");
    QCOMPARE(model->documents().size(), 1);
    QCOMPARE(model->documents().at(0)->name(), QString("inst_1_cu_nominal.cu_controller"));
    delete model;

    model = file->parseText("  \n mscdocument \n  ABC  ; \n  \n  endmscdocument  ;\n  ");
    QCOMPARE(model->documents().size(), 1);
    QCOMPARE(model->documents().at(0)->name(), QString("ABC"));
    // no exception thrown
    delete model;
}

void tst_MscFile::testComments()
{
    MscModel *model;
    model = file->parseText("/* This is a comment*/MSCDOCUMENT CU_level;\nENDMSCDOCUMENT;");
    delete model;
    model = file->parseText("MSCDOCUMENT CU_level /*This is a comment */ ;\nENDMSCDOCUMENT;");
    delete model;
    model = file->parseText("MSCDOCUMENT CU_level;\n/* This is a comment*/\nENDMSCDOCUMENT;");
    // no exception thrown
    delete model;
}

void tst_MscFile::testNestedDocuments()
{
    MscModel *model = file->parseText("MSCDOCUMENT l1; MSCDOCUMENT l2;ENDMSCDOCUMENT; \nENDMSCDOCUMENT;");
    auto mainDocuments = model->documents();
    QCOMPARE(mainDocuments.size(), 1);
    QCOMPARE(mainDocuments.at(0)->documents().size(), 1);
    delete model;

    // documents in parallel
    model = file->parseText("MSCDOCUMENT l1; MSCDOCUMENT l2;ENDMSCDOCUMENT; MSCDOCUMENT l3;ENDMSCDOCUMENT; \nENDMSCDOCUMENT;");
    mainDocuments = model->documents();
    QCOMPARE(mainDocuments.size(), 1);
    QCOMPARE(mainDocuments.at(0)->documents().size(), 2);
    delete model;
}

void tst_MscFile::testMscInDocument()
{
    MscModel *model = file->parseText("MSC msc1;ENDMSC;");
    QCOMPARE(model->charts().size(), 1);
    QCOMPARE(model->charts().at(0)->name(), QString("msc1"));
    delete model;

    model = file->parseText("MSCDOCUMENT doc1; MSC msc1;ENDMSC; \nENDMSCDOCUMENT;");
    QCOMPARE(model->documents().size(), 1);
    auto mainDocument = model->documents().at(0);
    QCOMPARE(mainDocument->charts().size(), 1);
    QCOMPARE(mainDocument->charts().at(0)->name(), QString("msc1"));
    delete model;
}

void tst_MscFile::testInstance()
{
    MscModel *model = file->parseText("MSC msc1;\nINSTANCE inst1;ENDINSTANCE;ENDMSC;");
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 1);
    QCOMPARE(chart->instances().at(0)->name(), QString("inst1"));
    delete model;
}

void tst_MscFile::testInstanceWithKind()
{
    MscModel *model = file->parseText("MSC msc1;INSTANCE foo : Process;ENDINSTANCE;ENDMSC;");
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 1);
    MscInstance *instance = chart->instances().at(0);
    QCOMPARE(instance->name(), QString("foo"));
    QCOMPARE(instance->kind(), QString("Process"));
    delete model;
}

void tst_MscFile::testInstanceWithInheritance()
{
    MscModel *model = file->parseText("MSC msc1;INSTANCE foo : PROCESS satellite;ENDINSTANCE;ENDMSC;");
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 1);
    MscInstance *instance = chart->instances().at(0);
    QCOMPARE(instance->name(), QString("foo"));
    QCOMPARE(instance->kind(), QString("PROCESS"));
    QCOMPARE(instance->inheritance(), QString("satellite"));
    delete model;
}

void tst_MscFile::testMessage()
{
    MscModel *model = file->parseText("MSC msc1;\nINSTANCE inst1;in ICONreq from env;out ICON to Responder;ENDINSTANCE;ENDMSC;");
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 1);
    MscInstance *instance = chart->instances().at(0);

    QCOMPARE(chart->instanceEvents().size(), 2);
    MscMessage *message1 = dynamic_cast<MscMessage *>(chart->instanceEvents().at(0));
    QVERIFY(message1 != nullptr);
    QCOMPARE(message1->name(), QString("ICONreq"));
    QCOMPARE(message1->sourceInstance(), static_cast<MscInstance *>(nullptr));
    QCOMPARE(message1->targetInstance(), instance);

    MscMessage *message2 = dynamic_cast<MscMessage *>(chart->instanceEvents().at(1));
    QVERIFY(message2 != nullptr);
    QCOMPARE(message2->name(), QString("ICON"));
    QCOMPARE(message2->sourceInstance(), instance);
    QCOMPARE(message2->targetInstance(), static_cast<MscInstance *>(nullptr));
    delete model;
}

void tst_MscFile::testSameMessageInTwoInstances()
{
    QString msc = "MSC msc1; \
                  INSTANCE Initiator;in ICON from Responder;ENDINSTANCE; \
                  INSTANCE Responder;out ICON to Initiator;ENDINSTANCE; \
                  ENDMSC;";
    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 2);
    QCOMPARE(chart->instanceEvents().size(), 1);
    delete model;
}

void tst_MscFile::testMessageWithParameters()
{
    QString msc = "MSC msc1; \
                   INSTANCE Initiator; \
                      in gui_send_tm,a(longitude:-174.0) from env; \
                      out gui_int_send_tm(12) to env; \
                   ENDINSTANCE; \
                   ENDMSC;";

    MscModel *model = file->parseText(msc);

    QVERIFY(model->charts().size() == 1);
    MscChart *chart = model->charts().at(0);

    QVERIFY(chart->instanceEvents().size() == 2);

    auto *message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(0));
    QVERIFY(message != nullptr);
    QCOMPARE(message->parameters().name, QString("a"));
    QCOMPARE(message->parameters().expression, QString("longitude:-174.0"));

#ifndef __clang_analyzer__
    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(1));
    QVERIFY(message != nullptr);
    QVERIFY(message->parameters().name.isEmpty());
    QCOMPARE(message->parameters().pattern, QString("12"));

    delete model;
#endif
}

void tst_MscFile::testGateMessage()
{
    QString msc = "MSC msc1; \
                   gate out Msg_4 to Inst_1; \
                   INSTANCE Inst_1;in Msg_4 from env;out Msg_3 to Inst_2;ENDINSTANCE; \
                   INSTANCE Inst_2;in Msg_3 from Inst_1;ENDINSTANCE; \
                   ENDMSC;";
    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 2);
    QCOMPARE(chart->instanceEvents().size(), 2);
    delete model;
}

void tst_MscFile::testCondition()
{
    QString msc = "MSC msc1; \
                      INSTANCE Inst_1; \
                         condition Con_1 shared all; \
                         condition Con_2; \
                       ENDINSTANCE; \
                   ENDMSC;";

    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);

    QCOMPARE(chart->instances().size(), 1);
    QCOMPARE(chart->instanceEvents().size(), 2);

    auto *condition = static_cast<MscCondition *>(chart->instanceEvents().at(0));
    QCOMPARE(condition->name(), QString("Con_1"));
    QVERIFY(condition->messageName().isEmpty());
    QCOMPARE(condition->shared(), true);

    condition = static_cast<MscCondition *>(chart->instanceEvents().at(1));
    QCOMPARE(condition->name(), QString("Con_2"));
    QVERIFY(condition->messageName().isEmpty());
    QCOMPARE(condition->shared(), false);

    delete model;
}

void tst_MscFile::testTimer()
{
    QString msc = "MSC msc1; \
                      INSTANCE Inst_1; \
                         starttimer T1; \
                         stoptimer T2; \
                         timeout T3; \
                       ENDINSTANCE; \
                   ENDMSC;";

    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);
    MscChart *chart = model->charts().at(0);

    QCOMPARE(chart->instances().size(), 1);
    QCOMPARE(chart->instanceEvents().size(), 3);

    auto event = chart->instanceEvents().at(0);
    QVERIFY(event->entityType() == MscEntity::EntityType::Timer);
    auto timer = static_cast<MscTimer *>(event);
    QCOMPARE(timer->name(), QString("T1"));
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Start);

    event = chart->instanceEvents().at(1);
    QVERIFY(event->entityType() == MscEntity::EntityType::Timer);
    timer = static_cast<MscTimer *>(event);
    QCOMPARE(timer->name(), QString("T2"));
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Stop);

    event = chart->instanceEvents().at(2);
    QVERIFY(event->entityType() == MscEntity::EntityType::Timer);
    timer = static_cast<MscTimer *>(event);
    QCOMPARE(timer->name(), QString("T3"));
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Timeout);

    delete model;
}

void tst_MscFile::testSortedMessage()
{
    QString msc = "msc connection; \
                      instance Initiator; \
                          in ICONreq from env; \
                          out ICON to Responder; \
                          in ICONF from Responder; \
                          out ICONconf to env; \
                      endinstance; \
                      instance Responder; \
                          in ICON from Initiator; \
                          out ICONind to env; \
                          in ICONresp from env; \
                          out ICONF to Initiator; \
                      endinstance; \
                  endmsc;";

    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);

    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 2);
    MscInstance *initiator = chart->instances().at(0);
    MscInstance *responder = chart->instances().at(1);

    QCOMPARE(chart->instanceEvents().size(), 6);

    MscMessage *message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(0));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONreq"));
    QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
    QCOMPARE(message->targetInstance(), initiator);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(1));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICON"));
    QCOMPARE(message->sourceInstance(), initiator);
    QCOMPARE(message->targetInstance(), responder);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(2));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONind"));
    QCOMPARE(message->sourceInstance(), responder);
    QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(3));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONresp"));
    QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
    QCOMPARE(message->targetInstance(), responder);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(4));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONF"));
    QCOMPARE(message->sourceInstance(), responder);
    QCOMPARE(message->targetInstance(), initiator);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(5));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONconf"));
    QCOMPARE(message->sourceInstance(), initiator);
    QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));

    delete model;
}

void tst_MscFile::testSortedMessageTwoCharts()
{
    QString msc = "mscdocument doc1; \
                      msc connection1; \
                          instance Initiator1; \
                              in ICONreq1 from env; \
                              out ICON1 to Responder1; \
                              in ICONF1 from Responder1; \
                              out ICONconf1 to env; \
                          endinstance; \
                          instance Responder1; \
                              in ICON1 from Initiator1; \
                              out ICONind1 to env; \
                              in ICONresp1 from env; \
                              out ICONF1 to Initiator1; \
                          endinstance; \
                      endmsc; \
                      msc connection2; \
                          instance Initiator2; \
                              in ICONreq2 from env; \
                              out ICON2 to Responder2; \
                              in ICONF2 from Responder2; \
                              out ICONconf2 to env; \
                          endinstance; \
                          instance Responder2; \
                              in ICON2 from Initiator2; \
                              out ICONind2 to env; \
                              in ICONresp2 from env; \
                              out ICONF2 to Initiator2; \
                          endinstance; \
                      endmsc; \
                  endmscdocument;";

    MscModel *model = file->parseText(msc);
    QCOMPARE(model->documents().size(), 1);

    MscDocument *document = model->documents().at(0);
    QCOMPARE(document->charts().size(), 2);

    for (int x = 0; x < document->charts().size(); ++x) {
        MscChart *chart = document->charts().at(x);
        QCOMPARE(chart->instances().size(), 2);

        MscInstance *initiator = chart->instances().at(0);
        MscInstance *responder = chart->instances().at(1);

        QCOMPARE(chart->instanceEvents().size(), 6);

        MscMessage *message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(0));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICONreq%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
        QCOMPARE(message->targetInstance(), initiator);

        message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(1));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICON%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), initiator);
        QCOMPARE(message->targetInstance(), responder);

        message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(2));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICONind%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), responder);
        QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));

        message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(3));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICONresp%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
        QCOMPARE(message->targetInstance(), responder);

        message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(4));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICONF%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), responder);
        QCOMPARE(message->targetInstance(), initiator);

        message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(5));
        QVERIFY(message != nullptr);
        QCOMPARE(message->name(), QString("ICONconf%1").arg(x + 1));
        QCOMPARE(message->sourceInstance(), initiator);
        QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));
    }

    delete model;
}

void tst_MscFile::testSortedInstanceEvents()
{
    QString msc = "msc connection; \
                      instance Initiator; \
                          starttimer T1; \
                          in ICONreq from env; \
                          out ICON to Responder; \
                          stoptimer T1; \
                          in ICONF from Responder; \
                          out ICONconf to env; \
                          timeout T1; \
                      endinstance; \
                      instance Responder; \
                          in ICON from Initiator; \
                          out ICONind to env; \
                          starttimer T2; \
                          in ICONresp from env; \
                          out ICONF to Initiator; \
                      endinstance; \
                  endmsc;";

    MscModel *model = file->parseText(msc);
    QCOMPARE(model->charts().size(), 1);

    MscChart *chart = model->charts().at(0);
    QCOMPARE(chart->instances().size(), 2);
    MscInstance *initiator = chart->instances().at(0);
    MscInstance *responder = chart->instances().at(1);

    QCOMPARE(chart->instanceEvents().size(), 10);

    int eventNr = -1;

    MscTimer *timer = dynamic_cast<MscTimer *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(timer != nullptr);
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Start);
    QCOMPARE(timer->name(), QString("T1"));

    MscMessage *message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONreq"));
    QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
    QCOMPARE(message->targetInstance(), initiator);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICON"));
    QCOMPARE(message->sourceInstance(), initiator);
    QCOMPARE(message->targetInstance(), responder);

    timer = dynamic_cast<MscTimer *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(timer != nullptr);
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Stop);
    QCOMPARE(timer->name(), QString("T1"));

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONind"));
    QCOMPARE(message->sourceInstance(), responder);
    QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));

    timer = dynamic_cast<MscTimer *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(timer != nullptr);
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Start);
    QCOMPARE(timer->name(), QString("T2"));

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONresp"));
    QCOMPARE(message->sourceInstance(), static_cast<MscInstance *>(nullptr));
    QCOMPARE(message->targetInstance(), responder);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONF"));
    QCOMPARE(message->sourceInstance(), responder);
    QCOMPARE(message->targetInstance(), initiator);

    message = dynamic_cast<MscMessage *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(message != nullptr);
    QCOMPARE(message->name(), QString("ICONconf"));
    QCOMPARE(message->sourceInstance(), initiator);
    QCOMPARE(message->targetInstance(), static_cast<MscInstance *>(nullptr));

    timer = dynamic_cast<MscTimer *>(chart->instanceEvents().at(++eventNr));
    QVERIFY(timer != nullptr);
    QCOMPARE(timer->timerType(), MscTimer::TimerType::Timeout);
    QCOMPARE(timer->name(), QString("T1"));

    delete model;
}

void tst_MscFile::testHierarchy()
{
    QString msc = "\
        mscdocument test_is /* MSC IS */; \
            /* MSC AND */ \
            mscdocument test_and; \
                /* MSC OR */ \
                MSCDOCUMENT test_or; \
                    MSC connection; \
                    ENDMSC; \
                endmscdocument; \
                /* MSC PARALLEL */ \
                MSCDOCUMENT test_parallel; \
                    MSC connection; \
                    ENDMSC; \
                endmscdocument; \
                /* MSC REPEAT */ \
                MSCDOCUMENT test_repeat; \
                    MSC connection; \
                    ENDMSC; \
                endmscdocument; \
                MSCDOCUMENT test_exception /* MSC EXCEPTION */; \
                    MSCDOCUMENT test_leaf; \
                        MSC connection; \
                        ENDMSC; \
                    endmscdocument; \
                endmscdocument; \
            endmscdocument; \
        endmscdocument;";

    auto model = file->parseText(msc);
    QCOMPARE(model->documents().size(), 1);

    auto documentIs = model->documents().at(0);
    QCOMPARE(documentIs->hierarchyType(), MscDocument::HierarchyIs);
    QCOMPARE(documentIs->documents().size(), 1);

    auto documentAnd = documentIs->documents().at(0);
    QCOMPARE(documentAnd->hierarchyType(), MscDocument::HierarchyAnd);
    QCOMPARE(documentAnd->documents().size(), 4);

    auto documentOr = documentAnd->documents().at(0);
    QCOMPARE(documentOr->hierarchyType(), MscDocument::HierarchyOr);
    QCOMPARE(documentOr->documents().size(), 0);

    auto documentParallel = documentAnd->documents().at(1);
    QCOMPARE(documentParallel->hierarchyType(), MscDocument::HierarchyParallel);
    QCOMPARE(documentParallel->documents().size(), 0);

    auto documentRepeat = documentAnd->documents().at(2);
    QCOMPARE(documentRepeat->hierarchyType(), MscDocument::HierarchyRepeat);
    QCOMPARE(documentRepeat->documents().size(), 0);

    auto documentException = documentAnd->documents().at(3);
    QCOMPARE(documentException->hierarchyType(), MscDocument::HierarchyException);
    QCOMPARE(documentException->documents().size(), 1);

    auto documentLeaf = documentException->documents().at(0);
    QCOMPARE(documentLeaf->hierarchyType(), MscDocument::HierarchyLeaf);

    delete model;
}

QTEST_APPLESS_MAIN(tst_MscFile)

#include "tst_mscfile.moc"
