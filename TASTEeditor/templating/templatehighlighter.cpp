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

#include "templatehighlighter.h"

#include <QDebug>

namespace taste3 {
namespace templating {

/**
 * @brief TemplateHighlighter::TemplateHighlighter ctor
 * @param parent
 */
TemplateHighlighter::TemplateHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // keyword rule
    HighlightingRule keywordRule;
    keywordRule.format.setForeground(Qt::blue);
    keywordRule.format.setFontWeight(QFont::Bold);
    const QString keywords = QStringLiteral("\\b(include|extends|for|in|endfor|comment|endcomment|if|elif|else|endif|firstof|block|endblock|"
                                            "autoescape\\s+(on|off)|endautoescape|cycle|as|silent|filter|endfilter|debug|ifchanged|endifchanged|"
                                            "ifequal|endifequal|ifnotequal|endifnotequal|load|from|media_finder|now|range|endrange|by|regroup|"
                                            "spaceless|endspaceless|templatetag|widthratio|with|endwith)\\b");
    keywordRule.pattern = QRegularExpression(keywords, QRegularExpression::InvertedGreedinessOption);

    // block.super keyword rule
    HighlightingRule blockSuperKeywordRule;
    blockSuperKeywordRule.format = keywordRule.format;
    blockSuperKeywordRule.pattern = QRegularExpression(QStringLiteral("\\bblock.super\\b"), QRegularExpression::InvertedGreedinessOption);

    // variable rule
    HighlightingRule variableRule;
    variableRule.format.setForeground(Qt::darkGreen);
    variableRule.format.setFontWeight(QFont::Bold);
    variableRule.pattern = QRegularExpression(QStringLiteral("(?<!\\|)\\b[a-zA-Z_]\\w*(\\.\\w+)*\\b"));

    // filter rule
    HighlightingRule filterRule;
    filterRule.format.setForeground(Qt::darkRed);
    filterRule.format.setFontWeight(QFont::Bold);
    const QString filters = QStringLiteral("(?<=\\||filter\\s)\\b(add|addslashes|capfirst|center|cut|date|default|default_if_none|dictsort|divisibleby|"
                                           "escape|escapejs|first|fix_ampersands|floatformat|force_escape|get_digit|join|last|"
                                           "length|length_is|linebreaks|linebreaksbr|linenumbers|ljust|lower|make_list|random|"
                                           "removetags|rjust|safe|safeseq|slice|slugify|stringformat|striptags|time|timesince|"
                                           "timeuntil|title|truncatewords|unordered_list|upper|wordcount|wordwrap|yesno)\\b");
    filterRule.pattern = QRegularExpression(filters, QRegularExpression::InvertedGreedinessOption);

    // filter delimiter rule
    HighlightingRule filterDelimiterRule;
    filterDelimiterRule.format = filterRule.format;
    filterDelimiterRule.format.setFontWeight(QFont::Bold);
    filterDelimiterRule.pattern = QRegularExpression(QStringLiteral("(?<=\\w)(\\||:)"));

    // string rule
    HighlightingRule stringRule;
    stringRule.format.setForeground(Qt::magenta);
    stringRule.format.setFontWeight(QFont::Bold);
    stringRule.pattern = QRegularExpression(QStringLiteral("(\".*\")|(\'.*\')"), QRegularExpression::InvertedGreedinessOption);

    // brackets highlighting
    Highlighting bracketHighlighting;
    bracketHighlighting.baseFormat.setForeground(Qt::red);
    bracketHighlighting.basePattern = QRegularExpression(QStringLiteral("({{|}}|{%|%}|{#|#})"));
    m_highlightings.append(bracketHighlighting);

    // keywords highlighting
    Highlighting keywordHighlighting;
    keywordHighlighting.basePattern = QRegularExpression(QStringLiteral("(?<={%).*(?=%})"), QRegularExpression::InvertedGreedinessOption);
    keywordHighlighting.highlightingRules.append(variableRule);
    keywordHighlighting.highlightingRules.append(filterRule);
    keywordHighlighting.highlightingRules.append(filterDelimiterRule);
    keywordHighlighting.highlightingRules.append(keywordRule);
    keywordHighlighting.highlightingRules.append(stringRule);
    m_highlightings.append(keywordHighlighting);

    // variables highlighting
    Highlighting variableHighlighting;
    variableHighlighting.basePattern = QRegularExpression(QStringLiteral("(?<={{).*(?=}})"), QRegularExpression::InvertedGreedinessOption);
    variableHighlighting.highlightingRules.append(variableRule);
    variableHighlighting.highlightingRules.append(blockSuperKeywordRule);
    variableHighlighting.highlightingRules.append(filterRule);
    variableHighlighting.highlightingRules.append(filterDelimiterRule);
    variableHighlighting.highlightingRules.append(stringRule);
    m_highlightings.append(variableHighlighting);

    // comments highlighting
    m_commentFormat.setForeground(Qt::darkGray);
    m_commentStartExpression.setPattern(QStringLiteral("{%\\s*comment\\s*%}"));
    m_commentEndExpression.setPattern(QStringLiteral("{%\\s*endcomment\\s*%}"));

    Highlighting commentHighlighting;
    commentHighlighting.basePattern = QRegularExpression(QStringLiteral("(?<={#).+(?=#})"), QRegularExpression::InvertedGreedinessOption);
    commentHighlighting.baseFormat = m_commentFormat;
    m_highlightings.append(commentHighlighting);
}

/**
 * @brief TemplateHighlighter::highlightBlock highlights text
 * @param text
 */
void TemplateHighlighter::highlightBlock(const QString &text)
{
    for (const Highlighting &highlighting : m_highlightings) {
        QRegularExpression expression = highlighting.basePattern;
        if (!expression.isValid()) {
            qWarning() << Q_FUNC_INFO << expression.errorString() << expression.pattern();
            continue;
        }

        QRegularExpressionMatchIterator it = expression.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int startIndex = match.capturedStart();
            if (highlighting.highlightingRules.isEmpty()) {
                int length = match.capturedLength();
                setFormat(startIndex, length, highlighting.baseFormat);
                continue;
            }

            const QString &captured = match.captured();
            for (const HighlightingRule &rule : highlighting.highlightingRules) {
                QRegularExpression pattern = rule.pattern;
                if (!pattern.isValid()) {
                    qWarning() << Q_FUNC_INFO << pattern.errorString() << pattern.pattern();
                    continue;
                }
                QRegularExpressionMatchIterator i = pattern.globalMatch(captured);
                while (i.hasNext()) {
                    QRegularExpressionMatch m = i.next();
                    int index = m.capturedStart();
                    int length = m.capturedLength();
                    setFormat(startIndex + index, length, rule.format);
                }
            }
        }
    }

    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1) {
        QRegularExpressionMatch startMatch = m_commentStartExpression.match(text);
        startIndex = startMatch.hasMatch() ? startMatch.capturedEnd() : -1;
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(m_commentEndExpression, startIndex, &endMatch);
        int commentLength;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex;
        }

        setFormat(startIndex, commentLength, m_commentFormat);
        QRegularExpressionMatch startMatch = m_commentStartExpression.match(text, startIndex + commentLength);
        startIndex = startMatch.hasMatch() ? startMatch.capturedEnd() : -1;;
    }
}

} // ns templating
} // ns taste3
