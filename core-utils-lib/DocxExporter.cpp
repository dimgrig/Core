#include "DocxExporter.h"

#include <QTextList>

#include "Logger.h"
#include "qtextcodec.h"

DocxExporter::DocxExporter() {

}

bool DocxExporter::write(const QString &fileName, QTextDocument &document) {
    // Сохраняем документ в формате .docx
    QTextDocumentWriter writer(fileName);
    writer.setFormat("ODF");
    bool res = writer.write(&document);
    if (!res) {
        qCDebug(logDebug) << writer.device()->errorString();
    }
    return res;
}

void DocxExporter::style(QTextDocument &document) {
    //блок - по сути абзац, margin - каждого абзаца
    QTextCursor style_cursor(&document);
    do{
        QTextBlockFormat bf = style_cursor.blockFormat();

        if (style_cursor.currentTable() != nullptr) {
            bf.setAlignment(Qt::AlignCenter);
            bf.setIndent(0);
            bf.setLineHeight(10, QTextBlockFormat::LineDistanceHeight); // interval
            //bf.setNonBreakableLines(true);
        } else {
            bf.setAlignment(Qt::AlignJustify);
            bf.setIndent(0);
            bf.setTextIndent(47.5); // first line in the block (38 - 1 cm???, 47.5 - 1.26cm)
            bf.setLineHeight(10, QTextBlockFormat::LineDistanceHeight); // interval - working in docx
            //bf.setLineHeight(150, QTextBlockFormat::ProportionalHeight); // interval - works in pdf, not working in docx
            //bf.setNonBreakableLines(true);
        }
        style_cursor.setBlockFormat(bf);
    } while(style_cursor.movePosition(QTextCursor::NextBlock));
}

void DocxExporter::exportTextToDocx(QTextDocument &document, const QString &text) {
    // Добавляем текст в документ
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat tcf;
    tcf.setFont(QFont("Arial", 14));
    cursor.insertText(text, tcf);
}

void DocxExporter::exportTableToDocx(QTextDocument &document, const QString &caption, const QVector<QVector<QString>> &tableData) {
    // Добавляем текст в документ
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat tcf;
    tcf.setFont(QFont("Arial", 14));
    cursor.insertText(caption, tcf);
    // //Добавляем таблицу в документ
    if (!tableData.isEmpty()) {
        insertTable(cursor, tableData);
    }
}

void DocxExporter::insertTable(QTextCursor &cursor, const QVector<QVector<QString>> &tableData) {
    // Создаем таблицу
    QTextCharFormat tcf;
    tcf.setFont(QFont("Arial", 14));
    QTextTableFormat ttf;
    ttf.setAlignment(Qt::AlignHCenter);
    ttf.setCellPadding(5);
    ttf.setCellSpacing(0);
    ttf.setBorder(1);
    ttf.setHeaderRowCount(1);
    //ttf.setWidth(QTextLength(QTextLength::PercentageLength, 100)); // Ширина 100% страницы
    //ttf.setWidth(400);

    int rows = tableData.size();
    int columns = tableData.isEmpty() ? 0 : tableData[0].size();

    QTextTable *table = cursor.insertTable(rows, columns, ttf);

    // Заполняем таблицу данными
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            QTextTableCell cell = table->cellAt(row, col);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(tableData[row][col], tcf);
        }
    }

    // Перемещаем курсор после таблицы
    cursor.movePosition(QTextCursor::End);
}

void DocxExporter::exportFormulaToDocx(QTextDocument &document, const QString &caption, const QString &formula) {
    // Добавляем текст в документ
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat tcf;
    tcf.setFont(QFont("Arial", 14));
    cursor.insertText(caption, tcf);
    cursor.insertHtml(formula);
    cursor.insertText(".\n", tcf);
}
