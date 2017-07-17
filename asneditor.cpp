/****************************************************************************
**
** Copyright (C) 2017 N7 Mobile sp. z o. o.
** Contact: http://n7mobile.com/Space
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a programme and funded by
** European Space Agency.
**
** This Plugin is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This Plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "asneditor.h"

#include <QApplication>
#include <QTextDocumentFragment>

#include <coreplugin/editormanager/documentmodel.h>

#include <texteditor/texteditoractionhandler.h>

#include "asn1acnconstants.h"
#include "asndocument.h"
#include "asnautocompleter.h"
#include "asncompletionassist.h"
#include "indenter.h"
#include "tr.h"

using namespace Asn1Acn::Internal;

using namespace Core;
using namespace TextEditor;

AsnEditor::AsnEditor()
{
    addContext(Constants::LANG_ASN1);
}

AsnEditorFactory::AsnEditorFactory()
{
    setId(Constants::ASNEDITOR_ID);
    setDisplayName(Tr::tr("ASN.1 Editor"));
    addMimeType(Constants::ASN1_MIMETYPE);

    setDocumentCreator([]() { return new AsnDocument; });
    setEditorWidgetCreator([]() { return new AsnEditorWidget; });
    setEditorCreator([]() { return new AsnEditor; });

    setCompletionAssistProvider(new AsnCompletionAssistProvider);
    setAutoCompleterCreator([]() { return new AsnAutoCompleter; });
    setIndenterCreator([]() { return new Indenter; });

    setCodeFoldingSupported(true);
    setMarksVisible(true);
    setParenthesesMatchingEnabled(true);
    setUseGenericHighlighter(true);

    setEditorActionHandlers(TextEditorActionHandler::Format
                          | TextEditorActionHandler::UnCommentSelection
                          | TextEditorActionHandler::UnCollapseAll
                          | TextEditorActionHandler::FollowSymbolUnderCursor);

  // TODO      addHoverHandler(new CppHoverHandler);
}

AsnEditorWidget::Link AsnEditorWidget::findLinkAt(const QTextCursor &cursor,
                                                  bool resolveTarget,
                                                  bool inNextSplit)
{
    Q_UNUSED(inNextSplit);

    TextDocument *document = textDocument();
    if (document->characterAt(cursor.position()).isSpace())
        return Link();

    QTextCursor tc = cursor;
    tc.select(QTextCursor::WordUnderCursor);
    if (!tc.hasSelection())
        return Link();

    Link link(document->filePath().toString());
    if (resolveTarget) {
        QTextDocumentFragment selectedText(tc);
        if (selectedText.isEmpty())
            return Link();

        QString docText = document->plainText();
        int targetPos = docText.indexOf(selectedText.toPlainText());
        if (targetPos == -1)
            return Link();

        convertPosition(targetPos, &link.targetLine, &link.targetColumn);
    } else {
        link.linkTextStart = tc.selectionStart();
        link.linkTextEnd = tc.selectionEnd();
    }

    return link;
}
