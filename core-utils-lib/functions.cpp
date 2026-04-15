#include "functions.h"

#include <algorithm>
#include <sstream>
#include <regex>

#include <QNetworkInterface>
#include <QScreen>
#include <QTableView>

#include "dimensions.h"
#include "MathParser.h"

//bool string_replace(std::string& str, const std::string& from, const std::string& to) {
//    size_t start_pos = str.find(from);
//    if(start_pos == std::string::npos)
//        return false;
//    str.replace(start_pos, from.length(), to);
//    return true;
//}

std::string to_string_exact(double x, bool fixed, int precision) {
    std::ostringstream os;
    if (fixed) {
        os << std::fixed;
    }
    os << std::setprecision(precision);
    os << x; //std::setprecision(std::numeric_limits<double>::max_digits10)
    return os.str();
}

bool string_replace(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return false;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return true;
}

int round_up_to_value(int value, int value_to_round) {
    return ((value + value_to_round - 1) / value_to_round) * value_to_round;
}

int round_down_to_value(int value, int value_to_round) {
    if (value_to_round != 0) {
        return (value / value_to_round) * value_to_round;
    } else {
        return 0;
    }
}

// Удаляет лишние нули после запятой. Например, "3.500" → "3.5", "3.000" → "3"
std::string trim_trailing_zeros(std::string s) {
    if ((s.find(',') != std::string::npos) ||
        (s.find('.') != std::string::npos)) {
        // Удаляем все нули справа после точки
        //s = std::regex_replace(s, std::regex("([0-9]+\\.[0-9]*[1-9])0+$"), "$1");
        std::regex reg(R"(([.,]\d*[1-9])0+$|[.,]0*$)");
        s = std::regex_replace(s, reg, "$1");

        // Дополнительная обработка особых случаев
        if (s.empty() || s == "-" || s == "-0") {
            return s = "0";
        }
    }
    return s;
}

std::string TRANSLATION_EN_RU(std::string EN) {
    std::string res;

    //check if already RU
    if (translations_RU_EN.count(EN) > 0) {
        res = EN;
    } else {
        auto result = std::find_if(
                  translations_RU_EN.begin(),
                  translations_RU_EN.end(),
                  [EN](const auto& mo) {return mo.second == EN; });

        if (result != translations_RU_EN.end())
            res = result->first;
    }

    //qCDebug(logDebug()) << "EN_RU" << EN << res;

    return res;
}

std::string TRANSLATION_RU_EN(std::string RU) {
    std::string res;

    //check if already EN
    auto result = std::find_if(
              translations_RU_EN.begin(),
              translations_RU_EN.end(),
              [RU](const auto& mo) {return mo.second == RU; });

    if (result != translations_RU_EN.end()) {
        res = RU;
    } else {
        auto result = translations_RU_EN.find(RU);

        if (result != translations_RU_EN.end())
            res = result->second;
    }

    //qCDebug(logDebug()) << "RU_EN" << RU << res;

    return res;
}

QString getPlatformSpecificIP() {
    QString ip;
    // используем стандартный Qt подход
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : ipAddressesList) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress(QHostAddress::LocalHost) &&
            address.toString().startsWith("192.168.")) {
            ip = address.toString();
        }
    }

    return ip;
}

bool startsWith(std::string mainStr, std::string toMatch) {
    // std::string::find returns 0 if toMatch is found at starting
    if(mainStr.find(toMatch) == 0)
        return true;
    else
        return false;
}

void buf_to_chunks(std::vector<char> rx_ba) {
    int PACKET_MAX_LENGTH = 5;
    char buffer_rx[3][5] = {{0}};
    for (size_t j = 0; j < (rx_ba.size()/PACKET_MAX_LENGTH) + 1; ++j) {
        //if (rx_ba.size() < PACKET_MAX_LENGTH) {
        size_t chunk_size = 0;
        if (j == rx_ba.size()/PACKET_MAX_LENGTH) {
            chunk_size = rx_ba.size() % PACKET_MAX_LENGTH;
        } else {
            chunk_size = PACKET_MAX_LENGTH;
        }
        for (size_t i = 0; i < chunk_size; ++i) {
            buffer_rx[j][i] = rx_ba.at(i + j*PACKET_MAX_LENGTH);
        }
    }
    Q_UNUSED(buffer_rx);
//    for (int j = 0; j < 3; ++j) {
//        for (int i = 0; i < 5; ++i) {
//            LOG((int)buffer_rx[j][i]);
//        }
//        LOG("---");
//    }
}

std::vector<std::string> split(std::string strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter))
    {
       splittedStrings.push_back(item);
    }
    return splittedStrings;
}

double extract_double(QString str) {
    str = str.split("errlmt").front();
    QRegularExpression rx("[+-]?([0-9]+([(,|.)][0-9]*)?|[(,|.)][0-9]+)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        //qCDebug(logDebug()) << res;
    }
    char separator_comma = ',';
    char separator_point = '.';
    return res.replace(separator_comma, separator_point).toDouble();
}

double extract_double_scientific(QString str) {
    str = str.split("errlmt").front();
    QRegularExpression rx("[+-]?([0-9]+([(,|.)][0-9]*)?|[(,|.)][0-9]+)((e|E)?([+-]?)[0-9]+)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        //qCDebug(logDebug()) << res;
    }
    char separator_comma = ',';
    char separator_point = '.';
    return res.replace(separator_comma, separator_point).toDouble();
}



unsigned int extract_id(const QString str) {
    QRegularExpression rx("[0-9]+");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

unsigned int extract_outcome_id(const QString str) {
    QRegularExpression rx("[o][0-9]+");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        res.remove(0, 1);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

////|-Uзи ут||Uзи ут|Uзи изм|Uзи|-Iз изм|Iз изм|Uпр|Iпр|Uобр|Iобр|Iс изм|Iс max огр|Iс инд огр|Iс max|Iс инд|Iс|Rси|S|Uси изм|Uси|Uис
//|Uзи ут|Uзи изм|Uзи|Iз изм|Uпр|Iпр|Uобр|Iобр|Iс изм|Iс max|Iс инд|Iс|Rси|S|Uси изм|Uси|Uис
QString extract_name(const QStringList SignalName_sl_re, const QString str) {
//    QStringList sn_sl_re;
//    for (auto s : SignalName_sl_re()) {
//        s.replace("|", "[|]");
//        sn_sl_re.push_back(s);
//    }
//    //qCDebug(logDebug) << sn_sl_re;
    QString signal_names = SignalName_sl_re.join("|");
    QRegularExpression rx("(" + signal_names + ")");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        //qCDebug(logDebug()) << res;
    }
    return res;
}

QString extract_signal_type(const QString str) {
    QRegularExpression rx("(U|I|R|S)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match_su = i.next();
        res = match_su.captured(0);
        //qCDebug(logDebug()) << res;
    }
    return res;
}

QString extract_testType(const QStringList TestTypes_re, const QString str) {
    QString test_types = TestTypes_re.join("|");
    QRegularExpression rx("(" + test_types + ")"); //Контактирование|Обратный ток|Прямое напряжение //Импульсный ток стока Iс(и) max
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        //qCDebug(logDebug()) << res;
    }
    return res;
}

QString extract_standardUnit(const QString str) {
    QRegularExpression rx("([0-9][ ])(нс|мкс|мс|с|"
                                        "нА|мкА|мА|А|кА|МА|ГА|"
                                        "нВ|мкВ|мВ|В|кВ|МА|ГА|"
                                        "нОм|мкОм|мОм|Ом|кОм|МОм|ГОм|"
                                        "нСм|мкСм|мСм|См|кСм|МСм|ГСм|"
                                        "ns|us|ms|s|"                                        
                                        "nA|uA|mA|A|kA|MA|GA|"
                                        "nV|uV|mV|V|kV|MV|GV|"
                                        "nOhm|uOhm|mOhm|Ohm|kOhm|MOhm|GOhm|"
                                        "nS|uS|mS|kS|MS|GS|S)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QRegularExpression rx_su("(нс|мкс|мс|с|"
                                    "нА|мкА|мА|А|кА|МА|ГА|"
                                    "нВ|мкВ|мВ|В|кВ|МА|ГА|"
                                    "нОм|мкОм|мОм|Ом|кОм|МОм|ГОм|"
                                    "нСм|мкСм|мСм|См|кСм|МСм|ГСм|"
                                    "ns|us|ms|s|"
                                    "nA|uA|mA|A|kA|MA|GA|"
                                    "nV|uV|mV|V|kV|MV|GV|"
                                    "nOhm|uOhm|mOhm|Ohm|kOhm|MOhm|GOhm|"
                                    "nS|uS|mS|kS|MS|GS|S)");
        QRegularExpressionMatchIterator j = rx_su.globalMatch(match.captured(0));
        if (j.hasNext()) {
            QRegularExpressionMatch match_su = j.next();
            res = match_su.captured(0);
            //qCDebug(logDebug()) << res;
        }
    }
    return res;
}


QString extract_dimension(const QString str) {
    QRegularExpression rx("(с|А|В|Ом|См|c|A|V|Ohm|S)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match_su = i.next();
        res = match_su.captured(0);
        //qCDebug(logDebug()) << res;
    }
    return res;
}

unsigned int extract_connectorPin(const QString str) {
    QRegularExpression rx("(A1|A2|G|D|S)"); //|C
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        res.remove(0, 1);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

double extract_tolerance(const QString str) {
    QRegularExpression rx("[+-][ ]([0-9]+([(,|.)][0-9]*)?|[(,|.)][0-9]+)[%]");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        if (res.size() > 3)
            res.remove(0, 2);
        if (res.size() > 0)
            res.remove(res.size() - 1, 1);
    }
    char separator_comma = ',';
    char separator_point = '.';
    return res.replace(separator_comma, separator_point).toDouble();
}

std::string su_from_name(const std::string name) {
    std::string su;
//    if ((name == "Iпр") || (name == "Iобр")) {
//        su = "А";
//    } else if ((name == "Uпр") || (name == "Uобр")) {
//        su = "В";
//    }
    if (name.find("I") != std::string::npos) {
        su = "А";
    } else if (name.find("U") != std::string::npos) {
        su = "В";
    } else if (name.find("R") != std::string::npos) {
        su = "Ом";
    } else if (name.find("S") != std::string::npos) {
        su = "См";
    }
    return su;
}

double extract_scale_factor(const std::string su) {
    double scale_factor = 1.0;
    for (const auto &dimension : dimension_v) {
        if (startsWith(su, dimension)) {
            if (dimension_m.count(dimension) > 0) {
                scale_factor = dimension_m.at(dimension).first / dimension_m.at(dimension).second;
            }
            break;
        }
    }
    return scale_factor;
}

int extract_controller_id(const QString str) {
    QRegularExpression rx("CONTROLLER(\\d+)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        res.remove(0, 10);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

int extract_sensor_id(const QString str) {
    QRegularExpression rx("SENSOR(\\d+)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        res.remove(0, 6);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

int extract_device_id(const QString str) {
    QRegularExpression rx("DEVICE(\\d+)");
    QString res = "";
    QRegularExpressionMatchIterator i = rx.globalMatch(str);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        res = match.captured(0);
        res.remove(0, 6);
        //qCDebug(logDebug()) << res;
    }
    return res.toInt();
}

int bit2(int x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x - (x >> 1);
}

int bit3(int x) {
    if (x == 0) return 0;
    int t = 1;
    if (x >= t << 16) t <<= 16;
    if (x >= t << 8) t <<= 8;
    if (x >= t << 4) t <<= 4;
    if (x >= t << 2) t <<= 2;
    if (x >= t << 1) t <<= 1;
    return t;
}

void delegate_selection_paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              QStyleOptionViewItem &opt,
                              const QModelIndex &index) {
    const QWidget *widget = option.widget;
    QString str = qvariant_cast<QString>(index.data())+ "\n";
    str += QString::number(qvariant_cast<float>(index.data(Qt::UserRole)));
    opt.text = "";
    QStyle *style = widget ? widget->style() : QApplication::style();

    if (option.state & QStyle::State_Selected) {
        // White pen while selection
        painter->setPen(Qt::white);
        painter->setBrush(option.palette.highlightedText());
        // This call will take care to draw, dashed line while selecting
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    } else {
        painter->setPen(QPen(option.palette.foreground(), 0));
        painter->setBrush(qvariant_cast<QBrush>(index.data(Qt::ForegroundRole)));
    }
}

std::string polynom_to_string(const std::vector<double>& polynom_v, int precision) {
    if (polynom_v.empty()) {
        return "0"; // Возвращаем "0" для пустого вектора
    }

    std::ostringstream oss;
    bool first_term = true; // Флаг для первого члена

    for (size_t i = 0; i < polynom_v.size(); ++i) {
        const double coeff = polynom_v[i];
        if (coeff == 0.0) continue; // Пропускаем нулевые коэффициенты

        // Определяем знак члена
        if (!first_term) {
            oss << (coeff > 0 ? " + " : " - ");
        } else if (coeff < 0) {
            oss << "-";
        }

        // Форматируем коэффициент (без знака)
        const double abs_coeff = std::abs(coeff);
        if (abs_coeff != 1.0 || i == 0) {
            // Для коэффициентов, отличных от 1, или для X^0
            oss << std::fixed << std::setprecision(precision) << abs_coeff;
            if (i != 0) {
                oss << "*";
            }
        }

        // Добавляем X и степень
        if (i > 0) {
            oss << "X";
            if (i > 1) {
                oss << "^" << i;
            }
        }

        first_term = false;
    }

    std::string result = oss.str();
    return result.empty() ? "0" : result; // Если все коэффициенты нулевые
}

int determine_polynom_order(const std::string& polynom_str) {
    if (polynom_str == "0") {
        return 0; // Нулевой полином
    }

    if (polynom_str == "X") {
        return 1;
    }

    std::vector<int> degrees;
    //std::regex termRegex(R"(([+-]?[\d\.]+\*X\^\d+)|([+-]?[\d\.]+\*X)|([+-]?[\d\.]+))");
    //std::regex termRegex(R"((([+-]?X\^\d+))|([+-]?[\d\.]+\*X\^\d+)|([+-]?[\d\.]+\*X)|([+-]?[\d\.]+))");
    std::regex termRegex(R"(([+-]?(?:\d+(?:[.,]\d+)?\*)?X(?:\^(\d+))?|[+-]?\d+(?:[.,]\d+)?))");
    std::sregex_iterator it(polynom_str.begin(), polynom_str.end(), termRegex);
    std::sregex_iterator end;

    while (it != end) {
        std::smatch match = *it;
        std::string term = match.str();

        // Определяем степень члена
        if (term.find("X^") != std::string::npos) {
            size_t caretPos = term.find('^');
            int degree = std::stoi(term.substr(caretPos + 1));
            degrees.push_back(degree);
        } else if (term.find('X') != std::string::npos) {
            degrees.push_back(1); // X^1
        } else {
            degrees.push_back(0); // Константа
        }
        ++it;
    }

    if (degrees.empty()) {
        return 0;
    }

    return *std::max_element(degrees.begin(), degrees.end());
}

std::string format_polynom_for_export(const std::string& input) {
    // Создаем регулярное выражение для поиска шаблона X^число
    std::regex pattern(R"(X\^(\d+))");

    // Заменяем каждое вхождение X^число на X<sup>число</sup>
    std::string result = std::regex_replace(
        input,
        pattern,
        "X<sup>$1</sup>"
        );

    return result;
}

void calculate(double &value, const QString &polynom_str) {
    MathParser mp;
    if (polynom_str != 0) {
        QString expression = polynom_str;
        expression.replace(",", ".");
        expression.replace("X", QString::number(value));
        QString result_str = mp.parse(expression);
        value = mp.get_result();
    }
}

void center_dialog(QDialog* dialog) {
    if (!dialog) return;

    QTableView* table_view = nullptr;
    QWidget* current_parent = dynamic_cast<QWidget*>(dialog->parent());
    while (current_parent && !table_view) {
        table_view = qobject_cast<QTableView*>(current_parent);
        current_parent = current_parent->parentWidget();
    }

    // Если есть родительский виджет (например, таблица), центрируем относительно него
    if (table_view != nullptr) {
        QPoint parentCenter = table_view->mapToGlobal(table_view->rect().center());
        dialog->move(
            parentCenter.x() - dialog->width() / 2,
            parentCenter.y() - dialog->height() / 2
            );
    }
    // Иначе центрируем относительно курсора
    else {
        QPoint cursorPos = QCursor::pos();
        dialog->move(
            cursorPos.x() - dialog->width() / 2,
            cursorPos.y() - dialog->height() / 2
            );
    }
}

void FunctionsTest() {
    {
        {
            int res_ref = 0;
            int res = round_up_to_value(0, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 1024;
            int res = round_up_to_value(750, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 1024;
            int res = round_up_to_value(1, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 1024;
            int res = round_up_to_value(1024, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 2048;
            int res = round_up_to_value(1025, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 2048;
            int res = round_up_to_value(2000, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 4096;
            int res = round_up_to_value(4000, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            int res_ref = 10240;
            int res = round_up_to_value(10200, 1024);
            ASSERT_EQUAL(res, res_ref);
        }
    }
    {
        {
            std::string res_ref = "A";
            std::string str = "А";
            std::string res = TRANSLATION_RU_EN(str);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            std::string res_ref = "us";
            std::string str = "мкс";
            std::string res = TRANSLATION_RU_EN(str);
        }
        {
            std::string res_ref = "мА";
            std::string str = "mA";
            std::string res = TRANSLATION_EN_RU(str);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            std::string res_ref = "мСм";
            std::string str = "mS";
            std::string res = TRANSLATION_EN_RU(str);
        }
        {
            std::string res_ref = "S";
            std::string str = "См";
            std::string res = TRANSLATION_RU_EN(str);
        }
    }
    {
        {
            double d_ref = 123.456;
            QString str = QStringLiteral("ABC 123,456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = -123.456;
            QString str = QStringLiteral("ABC -123,456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = 0.456;
            QString str = QStringLiteral("ABC 0,456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = -0.456;
            QString str = QStringLiteral("ABC -0,456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = 456;
            QString str = QStringLiteral("ABC 456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = -123;
            QString str = QStringLiteral("ABC -123 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = 0;
            QString str = QStringLiteral("ABC 0 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
        {
            double d_ref = 123.456;
            QString str = QStringLiteral("ABC = 123,456 DEG");
            double d = extract_double(str);
            ASSERT_EQUAL(d, d_ref);
        }
    }
    {
        {
            int id_ref = 1;
            QString str = QStringLiteral("test1");
            int id = extract_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 1;
            QString str = QStringLiteral("test 1");
            int id = extract_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 123;
            QString str = QStringLiteral("test123");
            int id = extract_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 123;
            QString str = QStringLiteral("test123asf");
            int id = extract_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
    }
    {
        {
            int id_ref = 1;
            QString str = QStringLiteral("o1");
            int id = extract_outcome_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 10;
            QString str = QStringLiteral("o10");
            int id = extract_outcome_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 123;
            QString str = QStringLiteral("test123o123");
            int id = extract_outcome_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            int id_ref = 123;
            QString str = QStringLiteral("test123asfo123asfas");
            int id = extract_outcome_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
    }
    {
        QString SignalName = QStringLiteral("-Uзи ут|Uзи ут|Uзи изм|Uзи|-Iз изм|Iз изм|Uпр|Iпр|Uобр|Iобр|Iс изм|Iс max огр|Iс инд огр|Iс max|Iс инд|Iс|Rси|S|Uси изм|Uси|Uис");
        QStringList SignalName_sl_re = SignalName.split("|");
        {
            QString name_ref = QStringLiteral("Uпр");
            QString str = QStringLiteral("Uпр1");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iпр");
            QString str = QStringLiteral("Iпрasf");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Uобр");
            QString str = QStringLiteral("Uобр2");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iобр");
            QString str = QStringLiteral("Iобр2");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }


        {
            QString name_ref = QStringLiteral("Iс");
            QString str = QStringLiteral("Iс = 10 А");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iс изм");
            QString str = QStringLiteral("Iс изм = 10 А");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iс max");
            QString str = QStringLiteral("Iс max = 10 А");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iс инд");
            QString str = QStringLiteral("Iс инд = 10 А");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Rси");
            QString str = QStringLiteral("Rси = 1 Ом");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("S");
            QString str = QStringLiteral("S = 10 См");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Uси изм");
            QString str = QStringLiteral("Uси изм = 10 В");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Uси");
            QString str = QStringLiteral("Uси = 10 В");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Uзи изм");
            QString str = QStringLiteral("Uзи изм = 10 В");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Uзи");
            QString str = QStringLiteral("Uзи = 10 В");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
        {
            QString name_ref = QStringLiteral("Iз изм");
            QString str = QStringLiteral("Iз изм = 10 А");
            QString name = extract_name(SignalName_sl_re, str);
            ASSERT_EQUAL(name, name_ref);
        }
    }
    {
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("5000 мкс");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("5000 мкс errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("aasd 5000 мкс 6000 А");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("aasd 500,0 мкс 6000 А");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("aasd 500.0 мкс 6000 А");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мкс");
            QString str = QStringLiteral("aasd 500.99 мкс 6000 А");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("");
            QString str = QStringLiteral("aasd 5000мкс asf");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мА");
            QString str = QStringLiteral("500 мА errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мВ");
            QString str = QStringLiteral("100 мВ errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("А");
            QString str = QStringLiteral("500 А errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("В");
            QString str = QStringLiteral("100 В errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }        
        {
            QString su_ref = QStringLiteral("мСм");
            QString str = QStringLiteral("100 мСм errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("mS");
            QString str = QStringLiteral("100 mS errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("мСм");
            QString str = QStringLiteral("S = 100 мСм errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
        {
            QString su_ref = QStringLiteral("mS");
            QString str = QStringLiteral("S = 100 mS errlimit");
            QString su = extract_standardUnit(str);
            ASSERT_EQUAL(su, su_ref);
        }
    }
    {
        {
            double tolerance_ref = 2;
            QString str = QStringLiteral("100 В errlimit +- 2%");
            double tolerance = extract_tolerance(str);
            ASSERT_EQUAL(tolerance, tolerance_ref);
        }
        {
            double tolerance_ref = 2;
            QString str = QStringLiteral("100 В errlimit +- 2.0%");
            double tolerance = extract_tolerance(str);
            ASSERT_EQUAL(tolerance, tolerance_ref);
        }
    }
    // {
    //     {
    //         QString test_name = QStringLiteral("Импульсный ток стока Iс(и) max1");
    //         TestType test_type_ref = Test_I_DM;
    //         TestType test_type = TestType_from_text(extract_testType(test_name).toStdString().c_str());
    //         ASSERT_EQUAL(test_type, test_type_ref);
    //     }
    // }
    {
        {
            std::string su_ref("А");
            std::string su = su_from_name("Iпр");
            ASSERT_EQUAL(su, su_ref);
        }
        {
            std::string su_ref("В");
            std::string su = su_from_name("Uзи");
            ASSERT_EQUAL(su, su_ref);
        }
        {
            std::string su_ref("Ом");
            std::string su = su_from_name("Rси");
            ASSERT_EQUAL(su, su_ref);
        }
        {
            std::string su_ref("См");
            std::string su = su_from_name("S");
            ASSERT_EQUAL(su, su_ref);
        }
    }
    {
        {
            QString str = QStringLiteral("CONTROLLER1_SENSOR1_Метка времени");
            int id_ref = 1;
            int id = extract_sensor_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
        {
            QString str = QStringLiteral("CONTROLLER1_SENSOR2_Статус");
            int id_ref = 2;
            int id = extract_sensor_id(str);
            ASSERT_EQUAL(id, id_ref);
        }
    }
//    {
//        LOG("1_____");
//        std::vector<char> rx_ba = {};
//        buf_to_chunks(rx_ba);
//    }
//    {
//        LOG("2_____");
//        std::vector<char> rx_ba = {1,2,3};
//        buf_to_chunks(rx_ba);
//    }
//    {
//        LOG("3_____");
//        std::vector<char> rx_ba = {1,2,3,4,5};
//        buf_to_chunks(rx_ba);
//    }
//    {
//        LOG("4_____");
//        std::vector<char> rx_ba = {1,2,3,4,5,6,7};
//        buf_to_chunks(rx_ba);
//    }
//    {
//        LOG("5_____");
//        std::vector<char> rx_ba = {1,2,3,4,5,6,7,8,9,10};
//        buf_to_chunks(rx_ba);
//    }
//    {
//        LOG("6_____");
//        std::vector<char> rx_ba = {1,2,3,4,5,6,7,8,9,10,11,12};
//        buf_to_chunks(rx_ba);
//    }
    {
        {
            std::string str_zeroes = "123,45000";
            std::string str_ref = "123,45";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "123,000";
            std::string str_ref = "123";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "123,001020300";
            std::string str_ref = "123,0010203";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "0,45000";
            std::string str_ref = "0,45";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "10,000";
            std::string str_ref = "10";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "0,000";
            std::string str_ref = "0";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "-123,450";
            std::string str_ref = "-123,45";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
        {
            std::string str_zeroes = "-0,12300";
            std::string str_ref = "-0,123";
            std::string str = trim_trailing_zeros(str_zeroes);
            ASSERT_EQUAL(str, str_ref);
        }
    }
    {
        {
            int x = 90;
            int ref = 64;
            int res = bit2(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 250;
            int ref = 128;
            int res = bit2(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 1;
            int ref = 1;
            int res = bit2(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 24;
            int ref = 16;
            int res = bit2(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 0;
            int ref = 0;
            int res = bit2(x);
            ASSERT_EQUAL(res, ref);
        }

        {
            int x = 90;
            int ref = 64;
            int res = bit3(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 250;
            int ref = 128;
            int res = bit3(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 1;
            int ref = 1;
            int res = bit3(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 24;
            int ref = 16;
            int res = bit3(x);
            ASSERT_EQUAL(res, ref);
        }
        {
            int x = 0;
            int ref = 0;
            int res = bit3(x);
            ASSERT_EQUAL(res, ref);
        }
    }
    {
        {
            {
                std::string polynom_str_ref = "0";
                std::vector<double> polynom_v({});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "X";
                std::vector<double> polynom_v({0, 1});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "1.234 + 6.789*X";
                std::vector<double> polynom_v({1.2345, 6.7891});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "0.124 - 1.245*X";
                std::vector<double> polynom_v({0.12421512, -1.2451254});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "3.123 - 2.456*X + 1.789*X^2";
                std::vector<double> polynom_v({3.123, -2.456, 1.789});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "4.123*X - 5.456*X^3";
                std::vector<double> polynom_v({0, 4.123, 0, -5.456});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
            {
                std::string polynom_str_ref = "-5.789*X^13";
                std::vector<double> polynom_v({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -5.789});
                std::string polynom_str = polynom_to_string(polynom_v);
                ASSERT_EQUAL(polynom_str, polynom_str_ref);
            }
        }
        {
            {
                int order_ref = 0;
                std::string polynom_str = "0";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 1;
                std::string polynom_str = "X";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 1;
                std::string polynom_str = "0.12421512 - 1.2451254*X";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 2;
                std::string polynom_str = "3.123 - 2.456*X + 1.789*X^2";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 3;
                std::string polynom_str = "4.123*X - 5.456*X^3";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 13;
                std::string polynom_str = "- 5.789*X^13";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
            {
                int order_ref = 3;
                std::string polynom_str = "4.123*X - X^3";
                int order = determine_polynom_order(polynom_str);
                ASSERT_EQUAL(order, order_ref);
            }
        }
        {
            std::string polynom_str = "0.123 - 4.123*X + 123*X^2 - X^3";
            std::string formatted_str_ref = "0.123 - 4.123*X + 123*X<sup>2</sup> - X<sup>3</sup>";
            std::string formatted_str = format_polynom_for_export(polynom_str);
            ASSERT_EQUAL(formatted_str, formatted_str_ref);
        }
    }
    {
        {
            int order_ref = 1;
            std::string polynom_str = "0,12421512 - 1,2451254*X";
            int order = determine_polynom_order(polynom_str);
            ASSERT_EQUAL(order, order_ref);
        }
        {
            int order_ref = 2;
            std::string polynom_str = "3,123 - 2,456*X + 1,789*X^2";
            int order = determine_polynom_order(polynom_str);
            ASSERT_EQUAL(order, order_ref);
        }
        {
            int order_ref = 1;
            std::string polynom_str = "-1,000 + X";
            int order = determine_polynom_order(polynom_str);
            ASSERT_EQUAL(order, order_ref);
        }
    }
}
