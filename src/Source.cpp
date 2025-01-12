#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QVector>
#include "Source.h"
#include "MainWindow.h"

using namespace qdev;

// create source from (existing) file or to a new file
Source::Source(QString path, bool fromFile, bool helloworld) 
	: QPlainTextEdit(0){

	// set attributes
	_isUndoAvailable = false;
	_isRedoAvailable = false;
	_isSaved = true;

	_filePath = path;

	QFileInfo sourceInfo(path);

	_linesArea = new LineNumberArea(this);

	// install syntax highlighter BEFORE inserting text 
	// to avoid textChanged() signal to be emitted the first time
	if (_filePath.endsWith(".c") || _filePath.endsWith(".cpp") || _filePath.endsWith(".h") \
		|| _filePath.endsWith(".hpp") || _filePath.endsWith(".cc"))
		_syntaxHighlighter = new Highlighter(document());

	// insert code from file or helloworld, or leave empty
	if (fromFile)
		read();
	else{
	
		if (helloworld){
			QString content =
				"#include <iostream>\n"
				"\n"
				"int main()\n"
				"{\n"
				"\tstd::cout<<\"Hello QDevelop!\";\n"
				"\n"
				"\treturn EXIT_SUCCESS;\n"
				"}";
			setPlainText(content);
		}

		write();
	}

	// set font and tab size
	setFont(MainWindow::instance()->session().font());
	setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
	setWordWrapMode(QTextOption::NoWrap);

	// set left margin (leave space for lines area)
	updateLeftMargin();

	// connections
	connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(undoAvailable(bool)));
	connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(redoAvailable(bool)));
	connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
	connect(_linesArea, SIGNAL(clicked(int)), this,
	       SLOT(onBreakpointClicked(int)));
}


void Source::highlightLine(unsigned int lineIndex, QColor highlightColor){

	QList<QTextEdit::ExtraSelection> extraSelectionsLst = extraSelections();	
	
	for (auto& sel : extraSelectionsLst) 
		if (sel.cursor.blockNumber() + 1 == lineIndex) {
			// If already has warning overwrite with error, then return
			if (sel.format.background().color() == SRC_WARNING_COLOR && highlightColor == SRC_ERROR_COLOR) {
				sel.format.setBackground(SRC_ERROR_COLOR);
				setExtraSelections(extraSelectionsLst);
			}
			return;
		}

	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(highlightColor);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	setTextCursor(QTextCursor(document()->findBlockByLineNumber(lineIndex - 1)));
	selection.cursor = textCursor();
	selection.cursor.clearSelection();

	extraSelectionsLst.append(selection);

	setExtraSelections(extraSelectionsLst);

}


void Source::textChanged(){

	_isSaved = false;
	
	if (!extraSelections().isEmpty()) {
		QList<QTextEdit::ExtraSelection> exselLst = extraSelections();
		exselLst.clear();
		setExtraSelections(exselLst);
	}


}

void Source::removeHighlight(){

	QList<QTextEdit::ExtraSelection> extraSelectionsLst = extraSelections();
	extraSelectionsLst.clear();
	setExtraSelections(extraSelectionsLst); 
}

void Source::gotoLine(unsigned int lineIndex){

	QTextBlock textBlock = document()->findBlockByLineNumber(lineIndex - 1);
	QTextCursor textCursor_ = textCursor();

	textCursor_.setPosition(textBlock.position());

	setFocus();
	setTextCursor(textCursor_);
}

// destroyer
Source::~Source(){

	if (_syntaxHighlighter)
		delete _syntaxHighlighter;
}

QString Source::name(){

	QFileInfo info(_filePath);
	return info.fileName();
}

bool Source::read(){

	QFile f(_filePath);
	if (!f.open(QIODevice::ReadOnly))
		return false;

	setPlainText(f.readAll());
	f.close();

	return true;
}


void Source::updateDirPath(QString newPath) {
	_filePath = newPath + "/" + name();
}

void Source::write(){
	
	QFile f(_filePath);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::critical(this, "Internal error", "Cannot save text", QMessageBox::StandardButton::Abort);
		return;
	}

	f.write(toPlainText().toUtf8());
	f.close();

	_isSaved = true;
	
}


void Source::remove(){

	if(!QFile(_filePath).remove())
		QMessageBox::critical(this, "Internal error", "Cannot remove file", QMessageBox::StandardButton::Abort);
}

void Source::rename(QString newName){

	QString newPath = QFileInfo(_filePath).path() + "/" + newName;
	
	if (!QFile::rename(_filePath, newPath))
		QMessageBox::critical(this, "Internal error", "Cannot rename source", QMessageBox::StandardButton::Abort);
	else
		_filePath = newPath;
}

void Source::undoAvailable(bool available){
	_isUndoAvailable = available;
}

void Source::redoAvailable(bool available){

	_isRedoAvailable = available;
}



void Source::wheelEvent(QWheelEvent* e){

	if (e->modifiers() == Qt::ControlModifier){
	
		QFont newFont = font();
		if (e->angleDelta().y() > 0)
			newFont.setPointSize(newFont.pointSize() + 1);
		else
			newFont.setPointSize(newFont.pointSize() - 1);
		
		MainWindow::instance()->setFont(newFont);

		e->ignore();
	}else 
		QPlainTextEdit::wheelEvent(e);
}

int Source::linesAreaWidth(){

	// left margin + lines area width + right margin
	return 2*_LINES_AREA_MARGIN + fontMetrics().horizontalAdvance('0') * _LINES_AREA_DIGITS;

}


void Source::updateLeftMargin(){

	setViewportMargins(linesAreaWidth(), 0, 0, 0);
}

void Source::updateLineNumberArea(const QRect& rect, int dy){

	if (dy)
		_linesArea->scroll(0, dy);
	else
		_linesArea->update(0, rect.y(), _linesArea->width(), rect.height());

	updateLeftMargin();
}

void Source::resizeEvent(QResizeEvent* e){

	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	_linesArea->setGeometry(QRect(cr.left(), cr.top(), linesAreaWidth(), cr.height()));
}

void Source::onBreakpointClicked(int h){

	if (!(_filePath.endsWith(".c") || _filePath.endsWith(".cpp") || _filePath.endsWith(".h") \
		|| _filePath.endsWith(".hpp") || _filePath.endsWith(".cc")))
		return;

	QTextBlock block = firstVisibleBlock();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();
	int blockNumber = block.blockNumber();
	while (block.isValid()){
		if (block.isVisible()){
			if (h > top && h < bottom) {
				int index = _breakpoints.indexOf(blockNumber);
				if (index != -1)
					_breakpoints.remove(index);
				else 
					_breakpoints << blockNumber;
				emitBreakpointChanged();
				update();
				return;
			}
		}
		blockNumber++;
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
	}
	
	
}

void Source::lineNumberAreaPaintEvent(QPaintEvent* event){

	QPainter painter(_linesArea);
	painter.fillRect(event->rect(), _NUMBER_AREA_RECT_COLOR);
	
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();


	while (block.isValid() && top <= event->rect().bottom()) {

		if (block.isVisible() && bottom >= event->rect().top()) {
			if (_breakpoints.indexOf(blockNumber) != -1){
				painter.setBrush(Qt::red);
				painter.drawEllipse(0, top + (fontMetrics().height() - _CIRCLE_WIDTH) / 2, _CIRCLE_WIDTH, _CIRCLE_WIDTH);
			}
			QString number = QString::number(blockNumber + 1);
			painter.setPen(_NUMBER_AREA_COLOR);
			painter.drawText(0, top, _linesArea->width() - _LINES_AREA_MARGIN, fontMetrics().height(), Qt::AlignRight, number);
		}
		
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		++blockNumber;
	}

	if (!_breakpoints.isEmpty() && blockCount() <= _breakpoints.last()) {
		_breakpoints.pop_back();
		emit breakpointChanged(this);
	}
}


