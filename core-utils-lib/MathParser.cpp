#include "MathParser.h"

#include "functions.h"
#include "Logger.h"
#include "TestRunner.h"

#include <type_traits>
#include <string>

// supports
// https://beltoforion.de/en/muparser/features.php#idDef2
// https://beltoforion.de/en/muparserx/index.php
// build https://qna.habr.com/q/700874

namespace {

    // to_mu — шаблон, чтобы if constexpr действительно отбрасывал
    // неподходящую ветку. Иначе в нешаблонной функции компилируются обе ветки,
    // и std::wstring <-> std::string даёт ошибку конверсии.
    template<typename T = mu::string_type>
    inline T to_mu(const char* s) {
        if constexpr (std::is_same_v<T, std::wstring>) {
            std::wstring ws;
            for (; *s; ++s) ws.push_back(static_cast<wchar_t>(*s));
            return ws;
        } else {
            return T(s);
        }
    }

    template<typename T = mu::string_type>
    inline T to_mu(const QString& s) {
        if constexpr (std::is_same_v<T, std::wstring>) {
            return s.toStdWString();     // Unicode сохраняется полностью
        } else {
            return s.toStdString();      // UTF-8
        }
    }

    template<typename S>
    inline QString from_mu(const S& s) {
        if constexpr (std::is_same_v<S, std::wstring>) {
            return QString::fromStdWString(s);
        } else {
            return QString::fromStdString(s);
        }
    }

} // namespace

MathParser::MathParser() {
    this->define_parser();
}

MathParser::~MathParser() {

}

// void MathParser::define_parser() {
// #ifdef Q_OS_WIN
//     //    // Удаляем стандартный оператор << (если он определен как оператор вектора)
//     //    try {
//     //        _p.RemoveOprt("<<");
//     //    } catch(...) {
//     //        // Игнорируем, если оператор не был определен
//     //    }

//     //    try {
//     //        _p.RemoveOprt(L">>");
//     //    } catch(...) {
//     //        // Игнорируем, если оператор не был определен
//     //    }

//     // Определяем << как побитовый сдвиг влево
//     _p.DefineOprt(L"<<", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
//     }, 0, mu::oaLEFT);  // Приоритет, ассоциативность

//     // Определяем >> как побитовый сдвиг вправо
//     _p.DefineOprt(L">>", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
//     }, 0, mu::oaLEFT);

//     _p.DefineFun(L"shl", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
//     });

//     _p.DefineFun(L"shr", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
//     });

//     _p.DefineInfixOprt(L"~", [](double a) -> double {
//         return static_cast<double>(~static_cast<long long>(a));
//     }, 0);

//     _p.DefineOprt(L"&", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) & static_cast<long long>(b));
//     }, 1, mu::oaLEFT);

//     _p.DefineOprt(L"|", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) | static_cast<long long>(b));
//     }, 0, mu::oaLEFT);

//     _p.DefineOprt(L"and", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) && static_cast<long long>(b));
//     }, 1, mu::oaLEFT);

//     _p.DefineOprt(L"or", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) || static_cast<long long>(b));
//     }, 0, mu::oaLEFT);
// #else
//     // Определяем << как побитовый сдвиг влево
//     _p.DefineOprt("<<", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
//     }, 0, mu::oaLEFT);  // Приоритет, ассоциативность

//     // Определяем >> как побитовый сдвиг вправо
//     _p.DefineOprt(">>", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
//     }, 0, mu::oaLEFT);

//     _p.DefineFun("shl", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
//     });

//     _p.DefineFun("shr", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
//     });

//     _p.DefineInfixOprt("~", [](double a) -> double {
//         return static_cast<double>(~static_cast<long long>(a));
//     }, 0);

//     _p.DefineOprt("&", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) & static_cast<long long>(b));
//     }, 1, mu::oaLEFT);

//     _p.DefineOprt("|", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) | static_cast<long long>(b));
//     }, 0, mu::oaLEFT);

//     _p.DefineOprt("and", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) && static_cast<long long>(b));
//     }, 1, mu::oaLEFT);

//     _p.DefineOprt("or", [](double a, double b) -> double {
//         return static_cast<double>(static_cast<long long>(a) || static_cast<long long>(b));
//     }, 0, mu::oaLEFT);
// #endif
// }

void MathParser::define_parser() {
    _p.DefineOprt(to_mu("<<"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
    }, 0, mu::oaLEFT);

    _p.DefineOprt(to_mu(">>"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
    }, 0, mu::oaLEFT);

    _p.DefineFun(to_mu("shl"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) << static_cast<long long>(b));
    });

    _p.DefineFun(to_mu("shr"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) >> static_cast<long long>(b));
    });

    _p.DefineInfixOprt(to_mu("~"), [](double a) -> double {
        return static_cast<double>(~static_cast<long long>(a));
    }, 0);

    _p.DefineOprt(to_mu("&"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) & static_cast<long long>(b));
    }, 1, mu::oaLEFT);

    _p.DefineOprt(to_mu("|"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) | static_cast<long long>(b));
    }, 0, mu::oaLEFT);

    _p.DefineOprt(to_mu("and"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) && static_cast<long long>(b));
    }, 1, mu::oaLEFT);

    _p.DefineOprt(to_mu("or"), [](double a, double b) -> double {
        return static_cast<double>(static_cast<long long>(a) || static_cast<long long>(b));
    }, 0, mu::oaLEFT);
}

QString MathParser::parse(QString expression) {
//    mu::string_type expression_buffer;
//    QByteArray expression_array = expression.toLocal8Bit();
// #ifdef Q_OS_UNIX
//     char *expression_chars = expression_array.data();
//     expression_buffer = expression_chars;
// #endif
// #ifdef Q_OS_WIN
//     QString expression_str = expression_array.data();
//     expression_buffer = expression_str.toStdWString();
// #endif
    mu::string_type expression_buffer = to_mu(expression);
    try {
        _p.SetExpr(expression_buffer);
        _result = _p.Eval();
        _result_buffer = QString::number(_result, 'g', 10);
    } catch (mu::Parser::exception_type &e) {
        qCDebug(logWarning) << "MathParser:" << from_mu(e.GetMsg())
        << "| code:" << int(e.GetCode())
        << "| expr:" << expression;
        _result_buffer = "error";
    }
    return _result_buffer;
}

void MathParserTest() {
    //TODO works only with c++14???

    MathParser mp;
    {
        QString expression = "1+2-3*4^5-6";
        double result_ref = 1+2-3*std::pow(4, 5)-6;
        QString result_str = mp.parse(expression);
        double result = mp.get_result();
        ASSERT_EQUAL(result, result_ref);
    }
    {
        QString expression = "(1+4)*4";
        double result_ref = 20;
        QString result_str = mp.parse(expression);
        double result = mp.get_result();
        ASSERT_EQUAL(result, result_ref);
    }
    {
        QString expression = "4*(1+4)";
        double result_ref = 20;
        QString result_str = mp.parse(expression);
        double result = mp.get_result();
        ASSERT_EQUAL(result, result_ref);
    }
    {
        QString expression = "(1-4)*4";
        double result_ref = -12;
        QString result_str = mp.parse(expression);
        double result = mp.get_result();
        ASSERT_EQUAL(result, result_ref);
    }
    {
        QString expression = "4*(1-4)";
        double result_ref = -12;
        QString result_str = mp.parse(expression);
        double result = mp.get_result();
        ASSERT_EQUAL(result, result_ref);
    }

    //bool
    {
        {
            QString expression = "1 && 0";
            double result_ref = 0;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "1 && 1";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "0 || 0";
            double result_ref = 0;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "1 || 0";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "((1 && 0) || (0 || 0)) || (1 || 0)";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }

    //bool
    {
        {
            QString expression = "1 and 0";
            double result_ref = 0;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "1 and 1";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "0 or 0";
            double result_ref = 0;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "1 or 0";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "((1 and 0) or (0 or 0)) or (1 or 0)";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }

    //int
    {
        {
            QString expression = "-(1+2)*(3-4)*(2^3)/(4/2)";
            double result_ref = 12;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT_EQUAL(result, result_ref);
        }
    }
    //double
    {
        {
            QString expression = "-(1.5+2.5)*(3.5-4.5)*(2.5^3.5)/(4.5/2.5)";
            double result_ref = 54.9006538224;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }
    {
        // cuff
        {
            QString expression = "shl(1, 4)";
            double result_ref = 16;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "1 << 4";
            double result_ref = 16;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(661 & ~(1 << 4)) | ((0 & 1) << 4)";
            double result_ref = 645;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(661 & ~(1 << 4)) | ((1 & 1) << 4)";
            double result_ref = 661;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(645 & ~(1 << 4)) | ((0 & 1) << 4)";
            double result_ref = 645;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(645 & ~(1 << 4)) | ((1 & 1) << 4)";
            double result_ref = 661;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(0 | ((1 & 1) << 4)) | (1 << 7) | (1 << 9)";
            double result_ref = 656;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }
    {
        {
            QString expression = "abs(100)";
            double result_ref = 100;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "abs(-100)";
            double result_ref = 100;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }
    {
        {
            QString expression = "(1 == 1)? 1 : 0";
            double result_ref = 1;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
        {
            QString expression = "(1 == 0)? 1 : 0";
            double result_ref = 0;
            QString result_str = mp.parse(expression);
            double result = mp.get_result();
            ASSERT(equal_to_double(result, result_ref));
        }
    }
}
