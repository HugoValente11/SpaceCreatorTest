/*
   Copyright (C) 2021 European Space Agency - <maxime.perrotin@esa.int>

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

#include "aadlcommonprops.h"
#include "aadlfunction.h"
#include "aadliface.h"
#include "aadlnamevalidator.h"

#include <QTest>

class tst_AADLIface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void tst_setAttr_Autoname();
    void tst_reqIfaceAutorename();
};

void tst_AADLIface::tst_setAttr_Autoname()
{
    static const QString autonamedPropName = ivm::meta::Props::token(ivm::meta::Props::Token::Autonamed);

    ivm::AADLIface::CreationInfo ci;
    std::unique_ptr<ivm::AADLFunction> func = std::make_unique<ivm::AADLFunction>();
    ci.function = func.get();
    std::unique_ptr<ivm::AADLIfaceRequired> reqIface = std::make_unique<ivm::AADLIfaceRequired>(ci);
    func->addChild(reqIface.get());

    QVERIFY(reqIface->isInheritPI());
    QVERIFY(reqIface->hasProperty(autonamedPropName, true));

    reqIface->setTitle(QLatin1String("SomeNameForReqIface"));
    QCOMPARE(reqIface->hasProperty(autonamedPropName, false), true);

    reqIface->setAttr(ivm::meta::Props::token(ivm::meta::Props::Token::name), QString());
    QVERIFY(ivm::AADLNameValidator::isAutogeneratedName(reqIface.get(), reqIface->title()));
    QVERIFY(reqIface->hasProperty(autonamedPropName));

    std::unique_ptr<ivm::AADLIfaceProvided> provIface = std::make_unique<ivm::AADLIfaceProvided>(ci);
    reqIface->setPrototype(provIface.get());
    QVERIFY(!reqIface->hasProperty(autonamedPropName));

    provIface->setTitle(QLatin1String("ProvIfaceName"));
    QCOMPARE(provIface->title(), reqIface->title());

    reqIface->setTitle(QLatin1String("NewNameForReqIface"));
    QVERIFY(provIface->title() != reqIface->title());
    QVERIFY(!reqIface->hasProperty(autonamedPropName));

    reqIface->unsetPrototype(provIface.get());
    QVERIFY(provIface->title() != reqIface->title());
    QVERIFY(reqIface->hasProperty(autonamedPropName, false));
}

void tst_AADLIface::tst_reqIfaceAutorename()
{
    ivm::AADLIface::CreationInfo ci1;
    std::unique_ptr<ivm::AADLFunction> func1 = std::make_unique<ivm::AADLFunction>();
    func1->setTitle(QLatin1String("F1"));
    ci1.name = QLatin1String("P1");
    ci1.function = func1.get();

    std::unique_ptr<ivm::AADLIfaceProvided> provIface1 = std::make_unique<ivm::AADLIfaceProvided>(ci1);
    func1->addChild(provIface1.get());

    ivm::AADLIface::CreationInfo ci2;
    std::unique_ptr<ivm::AADLFunction> func2 = std::make_unique<ivm::AADLFunction>();
    func2->setTitle(QLatin1String("F2"));
    ci2.name = QLatin1String("P1");
    ci2.function = func2.get();

    std::unique_ptr<ivm::AADLIfaceProvided> provIface2 = std::make_unique<ivm::AADLIfaceProvided>(ci2);
    func2->addChild(provIface2.get());

    ivm::AADLIface::CreationInfo ci3;
    std::unique_ptr<ivm::AADLFunction> func3 = std::make_unique<ivm::AADLFunction>();
    func3->setTitle(QLatin1String("F3"));
    ci3.function = func3.get();

    std::unique_ptr<ivm::AADLIfaceRequired> reqIface1 = std::make_unique<ivm::AADLIfaceRequired>(ci3);
    func3->addChild(reqIface1.get());
    reqIface1->setPrototype(provIface1.get());
    QCOMPARE(reqIface1->title(), provIface1->title());

    std::unique_ptr<ivm::AADLIfaceRequired> reqIface2 = std::make_unique<ivm::AADLIfaceRequired>(ci3);
    func3->addChild(reqIface2.get());
    reqIface2->setPrototype(provIface2.get());
    const QString reqIface2Name = func2->title() + QLatin1Char('_') + provIface2->title();
    QCOMPARE(reqIface2Name, reqIface2->title());

    provIface2->setTitle(QLatin1String("P2"));
    QCOMPARE(reqIface2Name, reqIface2->title());
}

QTEST_APPLESS_MAIN(tst_AADLIface)

#include "tst_aadliface.moc"