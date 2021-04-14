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

#include "ivcommonprops.h"
#include "ivfunction.h"
#include "ivinterface.h"
#include "ivlibrary.h"
#include "ivmodel.h"
#include "ivnamevalidator.h"
#include "ivtestutils.h"
#include "propertytemplateconfig.h"

#include <QTest>

class tst_AADLIface : public QObject
{
    Q_OBJECT

private:
    ivm::PropertyTemplateConfig *cfg { nullptr };

private Q_SLOTS:
    void initTestCase();
    void tst_setAttr_Autoname();
    void tst_reqIfaceAutorename();
};

void tst_AADLIface::initTestCase()
{
    ivm::initIVLibrary();
    cfg = ivm::PropertyTemplateConfig::instance();
    cfg->init(QLatin1String("default_attributes.xml"));
}

void tst_AADLIface::tst_setAttr_Autoname()
{
    static const QString autonamedPropName = ivm::meta::Props::token(ivm::meta::Props::Token::Autonamed);

    ivm::IVModel model(cfg);

    std::unique_ptr<ivm::IVFunction> func = std::make_unique<ivm::IVFunction>();
    model.addObject(func.get());

    ivm::IVInterface::CreationInfo ci;
    ci.id = shared::createId();
    ci.type = ivm::IVInterface::InterfaceType::Required;
    ci.function = func.get();
    ci.model = &model;
    std::unique_ptr<ivm::IVInterfaceRequired> reqIface = std::make_unique<ivm::IVInterfaceRequired>(ci);
    func->addChild(reqIface.get());
    model.addObject(reqIface.get());

    QVERIFY(reqIface->isInheritPI());
    QVERIFY(reqIface->hasProperty(autonamedPropName, true));

    reqIface->setTitle(QLatin1String("SomeNameForReqIface"));
    QCOMPARE(reqIface->hasProperty(autonamedPropName, false), true);

    reqIface->setAttr(ivm::meta::Props::token(ivm::meta::Props::Token::name), QString());
    QVERIFY(ivm::IVNameValidator::isAutogeneratedName(reqIface.get(), reqIface->title()));
    QVERIFY(reqIface->hasProperty(autonamedPropName));

    ci.id = shared::createId();
    ci.type = ivm::IVInterface::InterfaceType::Provided;
    std::unique_ptr<ivm::IVInterfaceProvided> provIface = std::make_unique<ivm::IVInterfaceProvided>(ci);
    model.addObject(provIface.get());
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
    ivm::IVModel model(cfg);

    std::unique_ptr<ivm::IVFunction> func1 = std::make_unique<ivm::IVFunction>();
    func1->setTitle(QLatin1String("F1"));
    model.addObject(func1.get());
    std::unique_ptr<ivm::IVInterfaceProvided> provIface1 { ivm::testutils::createProvidedIface(
            func1.get(), QLatin1String("P1")) };
    func1->addChild(provIface1.get());
    model.addObject(provIface1.get());

    std::unique_ptr<ivm::IVFunction> func2 = std::make_unique<ivm::IVFunction>();
    func2->setTitle(QLatin1String("F2"));
    model.addObject(func2.get());
    std::unique_ptr<ivm::IVInterfaceProvided> provIface2 { ivm::testutils::createProvidedIface(
            func2.get(), QLatin1String("P1")) };
    func2->addChild(provIface2.get());
    model.addObject(provIface2.get());

    ivm::IVInterface::CreationInfo ci3;
    std::unique_ptr<ivm::IVFunction> func3 = std::make_unique<ivm::IVFunction>();
    model.addObject(func3.get());
    func3->setTitle(QLatin1String("F3"));
    std::unique_ptr<ivm::IVInterfaceRequired> reqIface1 { ivm::testutils::createRequiredIface(func3.get()) };
    func3->addChild(reqIface1.get());
    model.addObject(reqIface1.get());
    reqIface1->setPrototype(provIface1->as<ivm::IVInterfaceProvided *>());
    QCOMPARE(reqIface1->title(), provIface1->title());

    std::unique_ptr<ivm::IVInterfaceRequired> reqIface2 { ivm::testutils::createRequiredIface(func3.get()) };
    func3->addChild(reqIface2.get());
    model.addObject(reqIface2.get());
    reqIface2->setPrototype(provIface2.get());
    const QString reqIface2Name = func2->title() + QLatin1Char('_') + provIface2->title();
    QCOMPARE(reqIface2Name, reqIface2->title());

    provIface2->setTitle(QLatin1String("P2"));
    QCOMPARE(provIface2->title(), reqIface2->title());

    QString title = QLatin1String("P1");
    std::unique_ptr<ivm::IVInterfaceProvided> provIface3 { ivm::testutils::createProvidedIface(func3.get(), title) };
    func3->addChild(provIface3.get());
    model.addObject(provIface3.get());
    QCOMPARE(provIface3->title(), title);

    std::unique_ptr<ivm::IVInterfaceRequired> reqIface3 { ivm::testutils::createRequiredIface(func2.get(), title) };
    func2->addChild(reqIface3.get());
    model.addObject(reqIface3.get());
    reqIface3->setPrototype(provIface3.get());
    QCOMPARE(provIface3->title(), reqIface3->title());

    title = QLatin1String("P5");
    std::unique_ptr<ivm::IVInterfaceProvided> provIface5 { ivm::testutils::createProvidedIface(func3.get(), title) };
    func3->addChild(provIface5.get());
    model.addObject(provIface5.get());
    QCOMPARE(provIface5->title(), title);

    title = QLatin1String("P1");
    provIface5->setTitle(title);
    QEXPECT_FAIL("", "It needs to be decided where name validation should be implemented", Continue);
    QVERIFY(provIface5->title() != title);
}

QTEST_APPLESS_MAIN(tst_AADLIface)

#include "tst_aadliface.moc"
