/*
   Copyright (C) 2018-2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "asn1const.h"
#include "asn1xmlparser.h"
#include "definitions.h"
#include "file.h"
#include "typeassignment.h"
#include "types/builtintypes.h"

#include <QDomElement>
#include <QDomNodeList>
#include <QSignalSpy>
#include <QtTest>

using namespace asn1;

class tst_Asn1XMLParser : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testFileOpenError();
    void testInvalidXMLFormat();
    void testEmptyFile();
    void testIntRealTypes();
    void testBoolEnumTypes();
    void testChoiceType();
    void testSequenceType();

private:
    Asn1XMLParser *xmlParser = nullptr;
    Asn1Acn::Types::Type::ASN1Type toAsn1Type(const QVariant &value)
    {
        return static_cast<Asn1Acn::Types::Type::ASN1Type>(value.toInt());
    }
};

void tst_Asn1XMLParser::init()
{
    xmlParser = new Asn1XMLParser;
}

void tst_Asn1XMLParser::cleanup()
{
    delete xmlParser;
    xmlParser = nullptr;
}

void tst_Asn1XMLParser::testFileOpenError()
{
    QSignalSpy spy(xmlParser, SIGNAL(parseError(const QString &)));

    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile("some_dummy_file_name.xml");
    QVERIFY(!asn1Types);

    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), tr("File not found"));
}

void tst_Asn1XMLParser::testInvalidXMLFormat()
{
    QSignalSpy spy(xmlParser, SIGNAL(parseError(const QString &)));
    xmlParser->parseAsn1XmlFile(QFINDTESTDATA("invalid_format.xml"));
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), tr("Invalid XML format"));
}

void tst_Asn1XMLParser::testEmptyFile()
{
    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile(QFINDTESTDATA("empty.xml"));
    QCOMPARE(asn1Types->definitionsList().size(), 1);
    QCOMPARE(asn1Types->definitionsList().at(0)->types().size(), 0);
}

void tst_Asn1XMLParser::testIntRealTypes()
{
    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile(QFINDTESTDATA("number_type.xml"));
    QVERIFY(asn1Types);

    const Asn1Acn::Definitions *definitions = asn1Types->definitions("ModuleTest");
    QVERIFY(definitions != nullptr);

    QCOMPARE(definitions->types().size(), 3);

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign1 = definitions->types().at(0);
    QCOMPARE(typeAssign1->name(), QString("MyInt"));
    QCOMPARE(typeAssign1->type()->typeName(), QString("INTEGER"));
    const QVariantMap &data1 = typeAssign1->type()->parameters();
    QCOMPARE(data1.size(), 2);
    QCOMPARE(data1[ASN1_MIN].toLongLong(), static_cast<qlonglong>(0));
    QCOMPARE(data1[ASN1_MAX].toLongLong(), static_cast<qlonglong>(20));

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign2 = definitions->types().at(1);
    QCOMPARE(typeAssign2->name(), QString("MyReal"));
    QCOMPARE(typeAssign2->type()->typeName(), QString("REAL"));
    const QVariantMap &data2 = typeAssign2->type()->parameters();
    QCOMPARE(data2.size(), 2);
    QCOMPARE(data2[ASN1_MIN].toDouble(), 0.0);
    QCOMPARE(data2[ASN1_MAX].toLongLong(), 1000.0);

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign3 = definitions->types().at(2);
    QCOMPARE(typeAssign3->name(), QString("T-UInt32"));
    QCOMPARE(typeAssign3->type()->typeName(), QString("INTEGER"));
    const QVariantMap &data3 = typeAssign3->type()->parameters();
    QCOMPARE(data3.size(), 2);
    QCOMPARE(data3[ASN1_MIN].toLongLong(), static_cast<qlonglong>(0));
    QCOMPARE(data3[ASN1_MAX].toLongLong(), static_cast<qlonglong>(4294967295));
}

void tst_Asn1XMLParser::testBoolEnumTypes()
{
    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile(QFINDTESTDATA("boolenum_type.xml"));
    const Asn1Acn::Definitions *definitions = asn1Types->definitions("ModuleTest");

    QCOMPARE(definitions->types().size(), 2);

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign1 = definitions->types().at(0);
    QCOMPARE(typeAssign1->name(), QString("MyBOOL"));
    QCOMPARE(typeAssign1->type()->typeName(), QString("BOOLEAN"));

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign2 = definitions->types().at(1);
    QCOMPARE(typeAssign2->name(), QString("TypeEnumerated"));
    QCOMPARE(typeAssign2->type()->typeName(), QString("ENUMERATED"));
    const QVariantMap &data2 = typeAssign2->type()->parameters();
    QCOMPARE(data2.size(), 1);
    QVariantList enumValues = data2[ASN1_VALUES].toList();
    QCOMPARE(enumValues.count(), 3);
    QCOMPARE(enumValues.at(0).toString(), QString("red"));
    QCOMPARE(enumValues.at(1).toString(), QString("green"));
    QCOMPARE(enumValues.at(2).toString(), QString("blue"));
}

void tst_Asn1XMLParser::testChoiceType()
{
    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile(QFINDTESTDATA("choice_type.xml"));
    const Asn1Acn::Definitions *definitions = asn1Types->definitions("ModuleTest");
    QCOMPARE(definitions->types().size(), 1);

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign1 = definitions->types().at(0);
    QCOMPARE(typeAssign1->name(), QString("MyChoice"));
    QCOMPARE(typeAssign1->type()->typeName(), QString("CHOICE"));

    auto choice = dynamic_cast<const Asn1Acn::Types::Choice *>(typeAssign1->type());
    QCOMPARE(choice->children().size(), 2);

    const std::unique_ptr<Asn1Acn::Types::Type> &choice1 = choice->children().at(0);
    QCOMPARE(choice1->typeName(), QString("BOOLEAN"));

    const std::unique_ptr<Asn1Acn::Types::Type> &choice2 = choice->children().at(1);
    QCOMPARE(choice2->typeName(), QString("REAL"));
    const QVariantMap &data = choice2->parameters();
    QCOMPARE(data.size(), 2);
    QCOMPARE(data[ASN1_MIN].toDouble(), -90.0);
    QCOMPARE(data[ASN1_MAX].toLongLong(), 90.0);
}

void tst_Asn1XMLParser::testSequenceType()
{
    std::unique_ptr<Asn1Acn::File> asn1Types = xmlParser->parseAsn1XmlFile(QFINDTESTDATA("sequence_type.xml"));
    const Asn1Acn::Definitions *definitions = asn1Types->definitions("ModuleTest");
    QCOMPARE(definitions->types().size(), 1);

    const std::unique_ptr<Asn1Acn::TypeAssignment> &typeAssign1 = definitions->types().at(0);
    QCOMPARE(typeAssign1->name(), QString("MySeq"));
    QCOMPARE(typeAssign1->type()->typeName(), QString("SEQUENCE"));

    auto sequence = dynamic_cast<const Asn1Acn::Types::Sequence *>(typeAssign1->type());
    QCOMPARE(sequence->children().size(), 2);

    const std::unique_ptr<Asn1Acn::Types::Type> &sequence1 = sequence->children().at(0);
    QCOMPARE(sequence1->typeName(), QString("BOOLEAN"));

    const std::unique_ptr<Asn1Acn::Types::Type> &sequence2 = sequence->children().at(1);
    QCOMPARE(sequence2->typeName(), QString("INTEGER"));
    const QVariantMap &data = sequence2->parameters();
    QCOMPARE(data.size(), 2);
    QCOMPARE(data[ASN1_MIN].toInt(), -10);
    QCOMPARE(data[ASN1_MAX].toInt(), 10);
}

QTEST_APPLESS_MAIN(tst_Asn1XMLParser)

#include "tst_asn1xmlparser.moc"
