/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QTextStream>
#include "Highlighter.h"

using namespace qdev;

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // preprocessor keywords starting with #
	rule.pattern = QRegularExpression("^\\s*#[a-zA-Z]+\\s*");
	rule.format.setForeground(Qt::blue);
	highlightingRules.append(rule);

	// special case of #include <what> with lookbehind regex
	rule.pattern = QRegularExpression("(?<=#include)\\s*<[a-zA-Z]+>");
	rule.format.setForeground(Qt::red);
	highlightingRules.append(rule);

	// class/namespace scope with lookahead regex
	rule.pattern = QRegularExpression("[a-zA-Z0-9_]*(?=::)");
	rule.format.setForeground(QColor(192, 0, 192));
	highlightingRules.append(rule);

	// class/struct name with lookbehind regex
	rule.pattern = QRegularExpression("(?<=class )\\s*[a-zA-Z0-9_]+");
	rule.format.setForeground(QColor(192, 0, 192));
	highlightingRules.append(rule);
	rule.pattern = QRegularExpression("(?<=struct )\\s*[a-zA-Z0-9_]+");
	rule.format.setForeground(QColor(192, 0, 192));
	highlightingRules.append(rule);

    // quotations
	rule.pattern = QRegularExpression("([\"\'])(?:(?=(\\\\?))\\2.)*?\\1");
	rule.format.setForeground(Qt::red);
    highlightingRules.append(rule);

    // functions
	rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format.setForeground(Qt::darkGreen);
    highlightingRules.append(rule);

    // C++ keywords
	QFile cppKeywords(":syntax/cpp_keywords.txt");
	if (cppKeywords.open(QIODevice::ReadOnly))
	{
		QTextStream in(&cppKeywords);
        while (!in.atEnd())
        {
			QString keyword = in.readLine();
			rule.pattern = QRegularExpression("(?<![a-zA-Z0-9_])" + keyword + "(?![a-zA-Z0-9_])");
			rule.format.setForeground(Qt::blue);
			rule.format.setFontWeight(QFont::Bold);
			highlightingRules.append(rule);
        }
		cppKeywords.close();
	}

	// C++ classes
	rule.format.setFontWeight(QFont::Normal);
	QFile cppClasses(":syntax/cpp_classes.txt");
	if (cppClasses.open(QIODevice::ReadOnly))
	{
		QTextStream in(&cppClasses);
		while (!in.atEnd())
		{
			QString class_name = in.readLine();
			rule.pattern = QRegularExpression("(?<![a-zA-Z0-9_<>])" + class_name + "(?![a-zA-Z0-9_<>])");
			rule.format.setForeground(QColor(192,0,192));
			highlightingRules.append(rule);
		}
		cppClasses.close();
	}

	// single-line comments
	rule.pattern = QRegularExpression("//[^\n]*");
	rule.format.setForeground(Qt::gray);
	highlightingRules.append(rule);

	// multi-line comments
	commentStartExpression = QRegularExpression("/\\*");
	commentEndExpression = QRegularExpression("\\*/");
	multiLineCommentFormat.setForeground(Qt::gray);
}


void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) 
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) 
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);
    while (startIndex >= 0) 
    {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) 
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } 
        else 
        {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
