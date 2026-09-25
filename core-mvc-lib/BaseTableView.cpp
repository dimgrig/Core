#include "BaseTableView.h"
#include "Profiler.h"

//#include <QWebEnginePage>
//#include <QWebEngineView>

#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QPrinter>
#include <QSettings>
#include <QTextDocument>

#include "aqp.h"
#include "CoreConfig.h"
#include "templates.h"

#include "BaseTableModel.h"

void get_html_header_stream(QStringList sl, QStringList sl_class, BaseTableView* tw, QTextStream &stream) {
    stream << "<thead><tr bgcolor=#f0f0f0>";
    for (int column = 0; column < sl.size(); column++) {
        if (tw ? (!tw->isColumnHidden(column)) : true) {
            QString data;
            QString style;
            QString class_str;
            if (sl.size() > column) {
                data = sl.at(column);
            }
            if (sl_class.size() > column) {
                class_str = sl_class.at(column);
            }
            stream << QString("<th><div") + class_str + style + QString(">%1</div></th>").arg(data);
        }
    }
    stream << "</tr></thead>";
}

void get_html_row_stream(QStringList sl, BaseTableView* tw, QTextStream &stream) {
    stream << "<tr>";
    for (int column = 0; column < sl.size(); column++) {
        if (tw ? (!tw->isColumnHidden(column)) : true) {
            QString data = sl.at(column);
            data.replace("<", "&lt;").replace(">", "&gt;");
            stream << QString("<td>%1</td>").arg(data);
        }
    }
    stream << "</tr>";
}

void get_csv_row_stream(QStringList sl, BaseTableView* tw, QTextStream &stream) {
    QStringList visible_sl;
    for (int column = 0; column < sl.size(); column++) {
        if (tw ? (!tw->isColumnHidden(column)) : true) {
            visible_sl << sl.at(column);
        }
    }

    stream << "\"";
    stream << visible_sl.join("\";\"");
    stream << "\"";
    stream << "\n";
}

BaseTableView::BaseTableView(QWidget *parent) :
    QTableView(parent)
{
    //ContextMenuPolicy CHANGE!!!
    connect(this->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &BaseTableView::slotHeaderCustomMenuRequested);
}

BaseTableView::~BaseTableView()
{

}

void BaseTableView::setModel(QAbstractItemModel *model) {
    QTableView::setModel(model);
    this->view_repaint();
}

void BaseTableView::closeEvent(QCloseEvent *event) {
    // qCDebug(logDebug()) << "closeEvent" << this->objectName();
    // QSettings settings(cfg.ORGANIZATION_NAME, cfg.APPLICATION_NAME);
    // for (int i = 0; i < this->model()->columnCount(); ++i) {
    //     if (this->objectName().contains("sensor_values")) {
    //         qCDebug(logDebug) << i << this->objectName() + "/" + this->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() << this->isColumnHidden(i);
    //     }
    //     settings.setValue(this->objectName() + "/" + this->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), QVariant(this->isColumnHidden(i)));
    //     QApplication::processEvents();
    // }

    //QSettings settings(cfg.ORGANIZATION_NAME, cfg.APPLICATION_NAME);
    QSettings settings(__ORGANIZATION_NAME, qApp->applicationName());

    // {
    //     //очистка настроек
    //     settings.beginGroup(this->objectName());
    //     settings.remove(""); // Удаляет всю группу
    //     settings.endGroup();
    // }

    QByteArray state = horizontalHeader()->saveState();

    if (!state.isEmpty()) {
        QString key = QString("%1/header_state").arg(objectName());
        settings.setValue(key, state);
        //settings.setValue(key + "/timestamp", QDateTime::currentDateTime().toString());
        settings.sync(); // Принудительная запись

        //qCDebug(logDebug()) << "Saved state for" << objectName() << "size:" << state.size();
    }

    QTableView::closeEvent(event);
}

void BaseTableView::restore_settings() {
    // qCDebug(logDebug()) << "restore_settings" << this->objectName() << this->model()->columnCount();
    // QSettings settings(cfg.ORGANIZATION_NAME, cfg.APPLICATION_NAME);
    // if (this->model()) {
    //     for (int i = 0; i < this->model()->columnCount(); ++i) {
    //         bool hidden = settings.value(this->objectName() + "/" + this->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), true).toBool();
    //         this->setColumnHidden(i, hidden);
    //         qCDebug(logDebug) << i << this->objectName() + "/" + this->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() << this->isColumnHidden(i);
    //     }
    // }

    //QSettings settings(cfg.ORGANIZATION_NAME, cfg.APPLICATION_NAME);
    QSettings settings(__ORGANIZATION_NAME, qApp->applicationName());

    QString key = QString("%1/header_state").arg(objectName());
    QByteArray state = settings.value(key).toByteArray();

    if (!state.isEmpty()) {
        // Проверяем, что состояние совместимо с текущей моделью
        if (horizontalHeader()->restoreState(state)) {
            //qCDebug(logDebug) << settings.value(key + "/timestamp").toString();
            //qCDebug(logDebug()) << "Restored header state for" << objectName() << "size:" << state.size();
        } else {
            //qCDebug(logDebug()) << "Failed to restore header state for" << objectName();
        }
    } else {
        //qCDebug(logDebug()) << "No saved header state for" << objectName();
    }
}
void BaseTableView::slotHeaderCustomMenuRequested(QPoint pos) {
    //qCDebug(logDebug()) << "logicalIndexAt" << this->horizontalHeader()->logicalIndexAt(pos);

    QMenu * menu = new QMenu(this);

    int index = this->horizontalHeader()->logicalIndexAt(pos);
    if (index != -1) {
        QString name;
        QString title = "Скрыть столбец";
        if (index < this->model()->columnCount()) {
            name = this->model()->headerData(index, Qt::Horizontal, Qt::DisplayRole).toString();
            name.replace("\n", " ");
            title += " \"" + name + "\"";
        }

        QAction * Hide = new QAction(title, this);
        menu->addAction(Hide);
        Hide->setData(index);
        connect(Hide, &QAction::triggered, this, &BaseTableView::hide_column);
    }

    QVector<int> hidden_index_v;
    QVector<QString> hidden_name_sl;
    for (int i = 0; i < this->model()->columnCount(); ++i) {
        if (this->isColumnHidden(i)) {
            hidden_index_v.push_back(i);
            hidden_name_sl.push_back(this->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        }
    }

    for (auto&[index, name] : qzip(hidden_index_v, hidden_name_sl)) {
        name.replace("\n", " ");
        QAction * Show = new QAction("Показать столбец \"" + name + "\"", this);
        Show->setData(index);
        menu->addAction(Show);
        connect(Show, &QAction::triggered, this, &BaseTableView::show_column);
    }

    /* Вызываем контекстное меню */
    menu->popup(this->mapToGlobal(pos));
}

void BaseTableView::copy_to_clipboard(QItemSelectionModel *select) {
    QModelIndex index = select->currentIndex();
    if (!index.isValid())
        return;

    QModelIndexList indexes = select->selectedRows();
    QString selected_text;

    foreach(const QModelIndex &current, indexes)
    {
        for (int col = 0; col < this->model()->columnCount(); ++col) {
            QVariant data = model()->data(model()->index(current.row(), col, current.parent()), Qt::DisplayRole);
            QString text = data.toString();
            selected_text.append(text);
            selected_text.append(';');
        }
        size_t size_to_remove = QString(ELEMENTS_DELIMETER).size();
        selected_text.remove(selected_text.size() - size_to_remove, size_to_remove);
        selected_text.append(ROW_DELIMETER);
    }
    size_t size_to_remove = QString(ROW_DELIMETER).size();
    selected_text.remove(selected_text.size() - size_to_remove, size_to_remove);
    QApplication::clipboard()->setText(selected_text);

    //qCDebug(logDebug()) << this->get_header();
    //qCDebug(logDebug()) << this->get_text();
}

QString BaseTableView::get_header() {
    QStringList headers;
    for(int column = 0; column < this->model()->columnCount(); column++) {
        if (!this->isColumnHidden(column)) {
            headers.append(this->model()->headerData(column, Qt::Horizontal).toString().replace("\n", " ").toUtf8());
        }
    }
    QString headers_row = "\"";
    headers_row += headers.join("\";\"");
    headers_row += "\"";
    headers_row += "\n";
    return headers_row;
}

QString BaseTableView::get_text(int row) {
    QString text;
    if (row == -1) {
        for (int row_ = 0; row_ < this->model()->rowCount(); ++row_) {
            QStringList row_sl;
            for (int column = 0; column < this->model()->columnCount(); ++column) {
                if (!this->isColumnHidden(column)) {
                    QVariant data = model()->data(model()->index(row_, column));
                    QString text = "\"";
                    text += data.toString();
                    text += "\"";
                    row_sl << text;
                }
            }
            text += row_sl.join(ELEMENTS_DELIMETER);
            text += "\n";
        }
    } else {
        QStringList row_sl;
        for (int column = 0; column < this->model()->columnCount(); ++column) {
            if (!this->isColumnHidden(column)) {
                QVariant data = model()->data(model()->index(row, column));
                QString text = "\"";
                text += data.toString();
                text += "\"";
                row_sl << text;
            }
        }
        text += row_sl.join(ELEMENTS_DELIMETER);
        text += "\n";
    }

    return text;
}

void BaseTableView::view_repaint() {
    this->view_repaint(-1, -1);
}

void BaseTableView::view_repaint(int row, int col) {
    //qCDebug(logDebug()) << row << col;

    bool was_stretched = horizontalHeader()->stretchLastSection();

    {
        //Profiler p1("BaseTableView resizeColumnsToContents");
        if (col != -1) {
            if ((col >= 0) && (col < this->model()->columnCount())) {
                this->resizeColumnToContents(col);
            }
        } else {
            this->resizeColumnsToContents();
        }
    }
    {
        //Profiler p1("BaseTableView resizeRowsToContents");
        if (row != -1) {
            if ((row >= 0) && (row < this->model()->rowCount())) {
                this->resizeRowToContents(row);
            }
        } else {
            this->resizeRowsToContents();
        }
    }

    this->horizontalHeader()->setStretchLastSection(was_stretched);
    // Если столбцы есть, принудительно растягиваем последний
    if (this->model()->columnCount() > 0) {
        int last_column = model()->columnCount() - 1;
        this->horizontalHeader()->setSectionResizeMode(last_column, QHeaderView::Stretch);
    }

    {
        //Profiler p1("BaseTableView repaint");
        //this->repaint();
    }
}

void BaseTableView::set_row_height(int row, int height) {
    if ((row > -1) && (row < this->model()->rowCount())) {
        if (height > this->rowHeight(row)) {
            this->setRowHeight(row, height);
        }
    }
}

void BaseTableView::set_col_width(int col, int width) {
    if ((col > -1) && (col < this->model()->columnCount())) {
        if (width > this->columnWidth(col)) {
            this->setColumnWidth(col, width);
        }
    }
}
void BaseTableView::hide_column() {
    QAction *Hide = qobject_cast<QAction *>(sender());
    int index = Hide->data().toInt();

    if (index < this->model()->columnCount()) {
        this->setColumnHidden(index, true);
    }
}

void BaseTableView::show_column() {
    QAction *Show = qobject_cast<QAction *>(sender());
    int index = Show->data().toInt();

    if (index < this->model()->columnCount()) {
        this->setColumnHidden(index, false);
    }
}

void BaseTableView::get_html_head(QTextStream &stream) {
    QString page_title = "Заголовок страницы";

    stream << "<html>"
           "<head>"
           "<meta name='qrichtext' content='1' charset='UTF-8'>"
           << QString("<title>%1</title>").arg(page_title);
    stream << "<style type='text/css' media='screen'>"
           "table {width: 100%; font-size: 12px; border-color: black}"
           "table th div.shrink {white-space:nowrap}"
           "table th div.small_expand {white-space:nowrap; width: 15%;}"
           "table th div.expand {white-space:nowrap; width: 25%;}"
           "</style>";
    stream << "</head>"
           "<body bgcolor=#ffffff link=#5000A0>";
//    "table th {word-break: break-word;}"
//    "table th.shrink {white-space:nowrap;}"
//    "table th.expand {white-space:nowrap;width: 25%;}"

//    "table {width: 100%; font-size: 12px; border-color: black}"
//    "table th div {transform: rotate(180deg); writing-mode:tb-rl;}" //not working - not supported https://doc.qt.io/qt-5/richtext-html-subset.html
//    "table td {white-space: pre-wrap;}"
}

void BaseTableView::get_html_table(TableType table_type, QString table_title, QString table_subtitle, QTextStream &stream, int header, int row) {
    Q_UNUSED(table_type);

    auto model = this->model();
    const int rowCount = model->rowCount();
    const int columnCount = model->columnCount();

    QStringList column_html;
    QStringList column_class_html;

    stream << QString("<h3>%1</h3>").arg(table_title);
    stream << QString("<h4>%1</h4>").arg(table_subtitle);
    stream << "<table border=1 cellspacing=0 cellpadding=2>"; // style='white-space:nowrap'

    // headers
    // switch (table_type) {
    //     case TableType_MainTable:
    //         // column_html = this->get_test_column_html();
    //         // column_class_html = this->get_test_column_class_html();
    //     break;
    //     case TableType_Measurements:

    //         // column_html = TestResultColumnRus_html;
    //         // column_class_html = TestResultColumnRus_class_html;
    //     break;
    //     default:

    //     break;
    // }

    for (int col = 0; col < this->model()->columnCount(); ++col) {
        auto header_text = this->model()->headerData(col, Qt::Horizontal).toString();
        header_text.replace("\n", " ");
        column_html << header_text;
        column_class_html << "";
    }

    if (header) {
        stream << "<thead><tr bgcolor=#f0f0f0>";
        for (int column = 0; column < columnCount; column++) {
            if (!this->isColumnHidden(column)) {
                QString data;
                QString style;
                QString class_str;
                if (column_html.size() > column) {
                    data = column_html.at(column); //model->headerData(column, Qt::Horizontal).toString().simplified();
                }
                if (column_class_html.size() > column) {
                    class_str = column_class_html.at(column);
                }
                //data.replace(" ", "<br>");
                stream << QString("<th><div") + class_str + style + QString(">%1</div></th>").arg(data);
            }
        }
        stream << "</tr></thead>";
    }

    if (row == -1) {
        // switch (this->get_table_type()) {
        //     case TableType_MainTable:
        //         // data table selected
        //         stream << "<tr>";
        //         for (const auto &index : this->selectedIndexes()) {
        //             if (!this->isColumnHidden(index.column())) {
        //                 QString data = model->data(model->index(index.row(), index.column())).toString().simplified();
        //                 data.replace("<", "&lt;").replace(">", "&gt;");
        //                 //data.replace(", ", ",<br>");
        //                 //data.replace(" = ", "&nbsp;=&nbsp;");
        //                 //                if (column > 1) {
        //                 //                   data.replace(", ", ",<br>");
        //                 //                   data.replace(" ", "&nbsp;");
        //                 //                }
        //                 //out << QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
        //                 stream << QString("<td>%1</td>").arg(data);
        //             }
        //         }
        //         stream << "</tr>";
        //     break;
        //     default:
                // data table
                for (int row = 0; row < rowCount; row++) {
                    stream << "<tr>";
                    for (int column = 0; column < columnCount; column++) {
                        if (!this->isColumnHidden(column)) {
                            QString data = model->data(model->index(row, column)).toString().simplified();
                            data.replace("<", "&lt;").replace(">", "&gt;");
                            //data.replace(", ", ",<br>");
                            //data.replace(" = ", "&nbsp;=&nbsp;");
                            //                if (column > 1) {
                            //                   data.replace(", ", ",<br>");
                            //                   data.replace(" ", "&nbsp;");
                            //                }
                            //out << QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
                            stream << QString("<td>%1</td>").arg(data);
                        }
                    }
                    stream << "</tr>";
                }
        //     break;
        // }
    } else {
        if (rowCount > row) {
            stream << "<tr>";
            for (int column = 0; column < columnCount; column++) {
                if (!this->isColumnHidden(column)) {
                    QString data = model->data(model->index(row, column)).toString().simplified();
                    data.replace("<", "&lt;").replace(">", "&gt;");
                    stream << QString("<td>%1</td>").arg(data);
                }
            }
            stream << "</tr>";
        }
    }
    stream << "</table>"
              "</body>"
              "</html>";
}

void BaseTableView::get_html(QTextStream &stream) {
    int header = 1;
    int row = -1;
    switch (this->get_table_type()) {
        case TableType_MainTable:
        case TableType_CalibrationRanges:
            if (!this->selectedIndexes().empty()) {
                row = this->selectedIndexes().front().row();
            }
        break;
        default:

        break;
    }

    this->set_table_title();
    this->set_table_subtitle();

    this->get_html_head(stream);
    this->get_html_table(this->get_table_type(), this->_table_title, this->_table_subtitle, stream, header, row);
}

void BaseTableView::get_html_stream(QTextStream &stream) {
    bool points_export = false;
    this->export_options_dialog(points_export);

    switch (this->get_table_type()) {
        case TableType_MainTable:
        case TableType_CalibrationRanges:
            if (points_export) { //complex
                this->get_html(stream);
                if (points_export) {
                    this->get_points_html(stream, 0);
                }
            } else { //simple
                this->get_html(stream);
            }
        break;
        case TableType_Measurements:
        case TableType_Calibration:
            //always simple export
            this->get_html(stream);
        break;
        default:

        break;
    }
}

void BaseTableView::get_csv_stream(QTextStream &stream) {
    bool points_export = false;
    this->export_options_dialog(points_export);

    int row = -1;
    switch (this->get_table_type()) {
        case TableType_MainTable:
        case TableType_CalibrationRanges:
            if (!this->selectedIndexes().empty()) {
                row = this->selectedIndexes().front().row();
            }
        break;
        default:

        break;
    }

    switch (this->get_table_type()) {
        case TableType_MainTable:
        case TableType_CalibrationRanges:
            if (points_export) { //complex
                stream << this->get_header();
                stream << this->get_text(row);
                if (points_export) {
                    this->get_points_csv(stream, 0);
                }
            } else { //simple
                stream << this->get_header();
                stream << this->get_text(row);
            }
        break;
        case TableType_Measurements:
        case TableType_Calibration:
            //always simple export
            stream << this->get_header();
            stream << this->get_text(row);
        break;
        default:

        break;
    }
}

void BaseTableView::slot_print() {
    bool empty = this->check_empty();
    if (!empty) {
        QScopedPointer<QPrinter> printer(new QPrinter());
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setPageOrientation(QPageLayout::Landscape);
    //    qreal topMargin = 15;
    //    qreal bottomMargin = 15;
    //    qreal leftMargin = 20;
    //    qreal rightMargin = 15;
    //    QPrinter::Unit units = QPrinter::Millimeter;
    //    printer->setPageMargins(leftMargin, topMargin, rightMargin, bottomMargin, units);
        if (!QPrinterInfo::availablePrinters().empty()) {
            QPrintDialog dialog = QPrintDialog(printer.get(), NULL);
            if (dialog.exec() == QDialog::Accepted) {
                QString print_string;
                QTextStream stream(&print_string);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                 stream.setCodec("UTF-8");
#endif
                this->get_html_stream(stream); //this->get_html(stream);

                QScopedPointer<QTextDocument> document(new QTextDocument());
                document->setHtml(print_string);
                document->print(printer.get());
            }
        } else {
            AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Нет настроенных принтеров"), 0, 10000);
        }
    }
}

void BaseTableView::slot_export_html() {
    bool empty = this->check_empty();
    if (!empty) {
        QString filename = this->file_dialog("html");
        if (filename != "") {
            QScopedPointer<QFile> file(new QFile(filename));
            if (file->open(QFile::WriteOnly | QFile::Text)){
                QTextStream stream(file.get());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                stream.setCodec("UTF-8");
#endif
                this->get_html_stream(stream);
                file->close();
            }
        }
    }
}

void BaseTableView::slot_export_pdf() {
    bool empty = this->check_empty();
    if (!empty) {
        QString filename = this->file_dialog("pdf");
        if (filename != "") {
            QScopedPointer<QPrinter> printer(new QPrinter());
            printer->setOutputFormat(QPrinter::PdfFormat);
            printer->setPageOrientation(QPageLayout::Landscape);
            printer->setOutputFileName(filename);

            QString print_string;
            QTextStream stream(&print_string);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            stream.setCodec("UTF-8");
#endif
            this->get_html_stream(stream); //this->get_html(stream);

            QScopedPointer<QTextDocument> document(new QTextDocument());
            document->setHtml(print_string);
            document->print(printer.get());
        }
    }
}

void BaseTableView::slot_export_csv() {
    bool empty = this->check_empty();
    if (!empty) {
        QString filename = this->file_dialog("csv");
        if (filename != "") {
            QScopedPointer<QFile> file(new QFile(filename));
            if (file->open(QFile::WriteOnly | QFile::Text)){
                QTextStream stream(file.get());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                stream.setCodec("UTF-8");
#endif
                this->get_csv_stream(stream);
                file->close();
            }
        }
    }
}

void BaseTableView::slot_export_txt() {
    bool empty = this->check_empty();
    if (!empty) {
        QString filename = this->file_dialog("txt");
        //same as csv!!!
        if (filename != "") {
            QScopedPointer<QFile> file(new QFile(filename));
            if (file->open(QFile::WriteOnly | QFile::Text)) {
                QTextStream out(file.get());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                out.setCodec("UTF-8");
#endif
                QString text = this->get_header();
                text += this->get_text();
                text.replace(ELEMENTS_DELIMETER, "\t");
                out << text;
                file->close();
            }
        }
    }
}

void BaseTableView::add_print_export_actions(QMenu *menu) {
    switch (this->get_table_type()) {
        case TableType_MainTable:
            if (!this->selectedIndexes().empty()) {
                QAction * print_action = new QAction("Распечатать", this);
                connect(print_action, &QAction::triggered, this, &BaseTableView::slot_print);
                menu->addAction(print_action);
                QAction * export_html_action = new QAction("Экспортировать в html", this);
                connect(export_html_action, &QAction::triggered, this, &BaseTableView::slot_export_html);
                menu->addAction(export_html_action);
                QAction * export_pdf_action = new QAction("Экспортировать в pdf", this);
                connect(export_pdf_action, &QAction::triggered, this, &BaseTableView::slot_export_pdf);
                menu->addAction(export_pdf_action);
                QAction * export_csv_action = new QAction("Экспортировать в csv", this);
                connect(export_csv_action, &QAction::triggered, this, &BaseTableView::slot_export_csv);
                menu->addAction(export_csv_action);
            }
        break;
        case TableType_Measurements:
        case TableType_Calibration:
        {
            QAction * print_action = new QAction("Распечатать", this);
            connect(print_action, &QAction::triggered, this, &BaseTableView::slot_print);
            menu->addAction(print_action);
            QAction * export_html_action = new QAction("Экспортировать в html", this);
            connect(export_html_action, &QAction::triggered, this, &BaseTableView::slot_export_html);
            menu->addAction(export_html_action);
            QAction * export_pdf_action = new QAction("Экспортировать в pdf", this);
            connect(export_pdf_action, &QAction::triggered, this, &BaseTableView::slot_export_pdf);
            menu->addAction(export_pdf_action);
            QAction * export_csv_action = new QAction("Экспортировать в csv", this);
            connect(export_csv_action, &QAction::triggered, this, &BaseTableView::slot_export_csv);
            menu->addAction(export_csv_action);
        }
        break;
        default:

        break;
    }
}

bool BaseTableView::check_empty() const {
    bool empty = !(this->model()->rowCount() > 0);
    if (empty) {
        QString title = "Ошибка";
        QString text = "Нет данных";
        QString detailed_text = "";
        AQP::warning(nullptr, title, text, detailed_text, 0, 10000);
    }
    return empty;
}

QString BaseTableView::file_dialog(QString file_suffix) {
    // auto test_model = qobject_cast<BaseTableModel*>(this->model());
    // QString filename = test_model->filename().replace(".csv", "." + file_suffix).replace(".xml", "." + file_suffix);

    // if (this->get_table_type() == TableType_TestsResult) {
    //     if (file_suffix == "csv") {
    //         QFileInfo fileinfo = QFileInfo(filename);
    //         QString filename_export = fileinfo.baseName() + "_export";
    //         fileinfo.setFile(fileinfo.dir(), filename_export + QString(".") + fileinfo.completeSuffix());
    //         filename = fileinfo.absoluteFilePath();
    //     }
    // }

    QString filename = "";

    QString caption(filename.isEmpty() ? QApplication::applicationDisplayName() : QFileInfo(filename).fileName());
    QString dir(filename.isEmpty() ? QString(".") : filename);
    QString filter(filename.isEmpty() ? file_suffix : QFileInfo(filename).completeSuffix());

    //qCDebug(logDebug) << file_suffix << filename << caption << dir << filter;

    filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Экспортировать").arg(caption),
            dir,
            tr("Файлы *.%1 (*.%1)").arg(filter));

    //qCDebug(logDebug) << filename;

    return filename;
}

void BaseTableView::export_options_dialog(bool &points_export) {
    QString title;
    QString text;
    QString detailed_text;
    switch (this->get_table_type()) {
        case TableType_MainTable:
        case TableType_CalibrationRanges:
            title = "Параметры экспорта";
            text = "Экспортировать дополнительно измерения?";
            detailed_text = "";
            points_export = AQP::question(this, title, text, detailed_text);
        break;
        case TableType_Measurements:
        case TableType_Calibration:
            //always simple export
        break;
        default:

        break;
    }
}
