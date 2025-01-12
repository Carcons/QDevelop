#pragma once

#include <QPlainTextEdit>
#include <QFileInfo>
#include <QVector>
#include "Highlighter.h"


namespace qdev{
	class Source;
	class LineNumberArea;
}



class qdev::Source : public QPlainTextEdit{

	Q_OBJECT

	protected:

		const QColor _NUMBER_AREA_RECT_COLOR = QColor(230, 230, 230);
		const QColor _NUMBER_AREA_COLOR = QColor(169, 183, 198);

		// object attributes
		QString _filePath;

		Highlighter* _syntaxHighlighter = 0;

		bool _isUndoAvailable;
		bool _isRedoAvailable;
		bool _isSaved;
		QWidget* _linesArea;

		// lines area constants and utilities
		const int _LINES_AREA_MARGIN = 15;
		const int _LINES_AREA_DIGITS = 4;

		int linesAreaWidth();
		
		// Breakpoints
		QVector<int> _breakpoints;
		const int _CIRCLE_WIDTH = 14;	
	
	public:


		~Source();

		// create source from (existing) file or to a new file
		Source(QString path, bool fromFile = true, bool helloworld = false);

		//Highlighter
		void highlightLine(unsigned int lineIndex, QColor highlightColor);
		void removeHighlight();

		// getters
		QString name();
		QString filePath() { return _filePath; }
		bool isUndoAvailable() { return _isUndoAvailable; }
		bool isRedoAvailable() { return _isRedoAvailable; }
		bool isSaved() { return _isSaved; }
		QVector<int> breakpoints(){ return _breakpoints; }
	
		// I/O methods
		bool read();
		void write();
		void remove();
		void rename(QString newName);

		// event handlers
		virtual void wheelEvent(QWheelEvent* e) override;

		//Utility
		void updateDirPath(QString newPath);
		void addBreakpoint(int b) { _breakpoints.append(b); }
		void addBreakpoints(QVector<int> bs) { _breakpoints = bs; }
		void removeBreakpoint(int b) { _breakpoints.removeAt(b); }
		void gotoLine(unsigned int lineIndex);

		friend class LineNumberArea;

	public slots:

		void undoAvailable(bool);
		void redoAvailable(bool);
		void textChanged();

		// lines area methods
		void resizeEvent(QResizeEvent* event) override;
		void lineNumberAreaPaintEvent(QPaintEvent* event);
		void updateLeftMargin();
		void updateLineNumberArea(const QRect&, int);
		void onBreakpointClicked(int h);	
		void emitBreakpointChanged() { emit breakpointChanged(this); }
	
	signals:

		void breakpointChanged(Source* src);


};


class qdev::LineNumberArea : public QWidget {

	Q_OBJECT

	private:

		Source* codeEditor;
	
	public:

		LineNumberArea(Source* editor) : 
			QWidget(editor), codeEditor(editor) {}

		QSize sizeHint() const override {
		
			return QSize(codeEditor->linesAreaWidth(), 0);
		}

	protected:

		void paintEvent(QPaintEvent* event) override {
		
			codeEditor->lineNumberAreaPaintEvent(event);
		}
	
		void mousePressEvent(QMouseEvent *event){
			emit clicked(event->pos().y());
		}

	signals:
		
		void clicked(int y);
};
