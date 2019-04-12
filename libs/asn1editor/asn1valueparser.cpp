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

#include "asn1valueparser.h"

#include "asn1const.h"

#include <QVariant>

namespace asn1 {

Asn1ValueParser::Asn1ValueParser(QObject *parent)
    : QObject(parent)
{
}

QVariantMap Asn1ValueParser::parseAsn1Value(const QVariantMap &asn1Type, const QString &asn1Value, bool *valueOk) const
{
    QVariantMap valueMap;
    bool ok = true;

    if (asn1Type.empty() || asn1Value.isEmpty())
        return valueMap;

    QString value = asn1Value.trimmed();
    auto valueType = static_cast<ASN1Type>(asn1Type[ASN1_TYPE].toInt());

    valueMap["name"] = asn1Type[ASN1_NAME];

    switch (valueType) {
    case INTEGER: {
        int intVal = value.toInt(&ok);

        if (ok && (ok = checkRange(asn1Type, intVal)))
            valueMap["value"] = intVal;

        break;
    }
    case DOUBLE: {
        double doubleVal = value.toDouble(&ok);

        if (ok && (ok = checkRange(asn1Type, doubleVal)))
            valueMap["value"] = doubleVal;

        break;
    }
    case BOOL:
        if ((ok = value == "TRUE" || value == "FALSE"))
            valueMap["value"] = value == "TRUE" ? true : false;

        break;
    case SEQUENCE:
        ok = parseSequenceValue(asn1Type, value, valueMap);
        break;
    case SEQUENCEOF:
        ok = parseSequenceOfValue(asn1Type, value, valueMap)
                && checkRange(asn1Type, valueMap["seqofvalue"].toList().count());
        break;
    case ENUMERATED: {
        QVariantList values = asn1Type["values"].toList();

        // check enumerated value
        if ((ok = values.contains(value)))
            valueMap["value"] = value;

        break;
    }
    case CHOICE:
        ok = parseChoiceValue(asn1Type, value, valueMap);
        break;
    default:
        // take string between " "
        if (value.startsWith("\""))
            value = value.remove(0, 1);

        if (value.endsWith("\""))
            value.chop(1);

        if ((ok = checkRange(asn1Type, value.length())))
            valueMap["value"] = value;
        break;
    }

    if (!ok) {
        valueMap.clear();
        Q_EMIT parseError(QString(tr("Incorrect value for %1")).arg(asn1Type["name"].toString()));
    }

    if (valueOk != nullptr)
        *valueOk = ok;
    return valueMap;
}

bool Asn1ValueParser::checkFormat(const QString &asn1Value) const
{
    return asn1Value.startsWith("{") && asn1Value.endsWith("}");
}

bool Asn1ValueParser::parseSequenceValue(const QVariantMap &asn1Type, const QString &asn1Value,
                                         QVariantMap &valueMap) const
{
    /*
{ intVal 5,
  realVal 42.0,
  intArray {},
  boolVal TRUE,
  enumVal enum2,
  enumArray {enum1, enum3},
  label "Hello World",
  choiceVal choiceReal : 10.5,
  seqVal { iVal 5 }
}
*/
    QVariantList children;
    QVariantList parseList;
    QString value = asn1Value.trimmed();

    if (!checkFormat(value)) {
        Q_EMIT parseError(tr("Incorrect format for ASN.1 sequence value"));
        return false;
    }

    // delete '{' from begin and '}' from end
    value.remove(0, 1);
    value = value.remove(-1, 1).trimmed();

    int index;

    while ((index = value.indexOf(" ")) != -1) {
        QVariantMap fieldMap;

        fieldMap["name"] = value.mid(0, index);
        value = value.remove(0, index).trimmed();

        index = nextIndex(value);

        fieldMap["value"] = value.mid(0, index);
        parseList.append(fieldMap);

        if (index != -1) {
            value = value.remove(0, index).trimmed();
            value = value.remove(0, value.indexOf(",") + 1).trimmed();
        } else
            break;
    }

    for (const auto &field : parseList) {
        auto childType = getType(field.toMap()["name"].toString(), asn1Type);

        if (childType.size()) {
            auto childValue = parseAsn1Value(childType, field.toMap()["value"].toString());

            if (childValue.size())
                children.append(childValue);
            else
                return false;
        } else
            return false;
    }

    valueMap["children"] = children;

    return true;
}

bool Asn1ValueParser::parseSequenceOfValue(const QVariantMap &asn1Type, const QString &asn1Value,
                                           QVariantMap &valueMap) const
{
    /*
{enum1, enum3}
{ { intVal 5, realVal 42.0 }, { intVal 6, realVal 13.9 } )

*/
    QVariantList seqofValues;
    QString value = asn1Value.trimmed();

    if (!checkFormat(value)) {
        Q_EMIT parseError(tr("Incorrect format for ASN.1 sequence value"));
        return false;
    }

    // delete '{' from begin and '}' from end
    value.remove(0, 1);
    value = value.remove(-1, 1).trimmed();

    int index;
    while ((index = nextIndex(value))) {
        auto item = value.mid(0, index);

        auto itemValue = parseAsn1Value(asn1Type[ASN1_SEQOFTYPE].toMap(), item);
        if (itemValue.size())
            seqofValues.append(itemValue);
        else
            return false;

        if (index != -1) {
            value = value.remove(0, index).trimmed();
            value = value.remove(0, value.indexOf(",") + 1).trimmed();
        } else
            break;
    }

    valueMap["seqofvalue"] = seqofValues;

    return true;
}

bool Asn1ValueParser::parseChoiceValue(const QVariantMap &asn1Type, const QString &asn1Value,
                                       QVariantMap &valueMap) const
{
    /*
 choiceReal : 10.5
 */
    const QString name = asn1Value.left(asn1Value.indexOf(":")).trimmed();

    auto choiceType = getType(name, asn1Type);
    if (choiceType.size()) {
        const QString value = asn1Value.mid(asn1Value.indexOf(":") + 1).trimmed();
        valueMap["choice"] = parseAsn1Value(choiceType, value);

        return true;
    }

    return false;
}

QVariantMap Asn1ValueParser::getType(const QString &name, const QVariantMap &asn1Type) const
{
    QVariantMap result;

    if (asn1Type[ASN1_NAME] == name)
        return asn1Type;

    if (asn1Type.contains(ASN1_CHILDREN)) {
        for (const QVariant &child : asn1Type[ASN1_CHILDREN].toList()) {
            result = getType(name, child.toMap());
            if (result.size())
                break;
        }
    }

    if (asn1Type.contains(ASN1_CHOICES)) {
        for (const QVariant &choice : asn1Type[ASN1_CHOICES].toList()) {
            result = getType(name, choice.toMap());
            if (result.size())
                break;
        }
    }

    return result;
}

bool Asn1ValueParser::checkRange(const QVariantMap &asn1Type, const QVariant &value) const
{
    if (asn1Type.contains(ASN1_MIN) && value < asn1Type[ASN1_MIN])
        return false;
    else if (asn1Type.contains(ASN1_MAX) && value > asn1Type[ASN1_MAX])
        return false;

    return true;
}

int Asn1ValueParser::nextIndex(const QString &value) const
{
    int index;

    if (value.startsWith("{")) {
        /* possile formats:
         *  { ... }
         *  { { ...  }, { ... }, ... }
         *  { { { ...  }, { ... } }, { { ...  }, { ... } }, ... }
         */
        int countBracket = 0;
        for (index = 0; index < value.size(); ++index) {
            if (value[index] == '{')
                ++countBracket;
            else if (value[index] == '}')
                --countBracket;

            if (countBracket == 0)
                break;
        }

        ++index;
    } else {
        index = value.indexOf(",");
        if (index == -1)
            index = value.size();
    }

    return index;
}

} // namespace asn1
