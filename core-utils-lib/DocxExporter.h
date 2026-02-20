#ifndef DOCXEXPORTER_H
#define DOCXEXPORTER_H

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextDocumentWriter>
#include <QFileDialog>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextTableFormat>

#include "core-utils-lib-global.h"

class CORE_UTILS_LIB_EXPORT DocxExporter {
public:
    DocxExporter();

    bool write(const QString &fileName, QTextDocument &document);
    void style(QTextDocument &document);
    void exportTextToDocx(QTextDocument &document, const QString &text);
    void exportTableToDocx(QTextDocument &document, const QString &caption, const QVector<QVector<QString>> &tableData);
    void exportFormulaToDocx(QTextDocument &document, const QString &caption, const QString &formula);
private:
    void insertTable(QTextCursor &cursor, const QVector<QVector<QString>> &tableData);
};

#endif // DOCXEXPORTER_H
