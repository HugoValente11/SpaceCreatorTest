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

#ifndef ASN1FILEVIEW_H
#define ASN1FILEVIEW_H

#include <QPointer>
#include <QWidget>

namespace Ui {
class ASN1FileView;
}

namespace msc {
class MscModel;
}

class ASN1FileView : public QWidget
{
    Q_OBJECT

public:
    explicit ASN1FileView(QWidget *parent = nullptr);
    ~ASN1FileView();

public Q_SLOTS:
    void setModel(msc::MscModel *model);
    void setCurrentDirectory(const QString &directory);

private Q_SLOTS:
    void updateView();
    void selectFile();
    void fillPreview();
    void fillPreview(const QString &filename);

private:
    Ui::ASN1FileView *ui;
    QPointer<msc::MscModel> m_model;
    QString m_currentDirectory; // directory of the current MSC file
};

#endif // ASN1FILEVIEW_H