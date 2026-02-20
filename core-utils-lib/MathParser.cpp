#include "MathParser.h"

#include "Logger.h"
#include "functions.h"
#include "TestRunner.h"

MathParser::MathParser() {

}

MathParser::~MathParser() {

}

QString MathParser::parse(QString expression) {
    mu::string_type expression_buffer;
    QByteArray expression_array = expression.toLocal8Bit();
#ifdef unix
    char *expression_chars = expression_array.data();
    expression_buffer = expression_chars;
#endif
#ifdef WIN32
    QString expression_str = expression_array.data();
    expression_buffer = expression_str.toStdWString();
#endif
    try {
        _p.SetExpr(expression_buffer);
        _result = _p.Eval();
        //char buffer[20]; //expression_array.size()+1
        //sprintf(buffer, "%f", _result);
        //_result_buffer = buffer;
        _result_buffer = QString::number(_result, 'g', 10);
    } catch(mu::Parser::exception_type &e) {
        _result_buffer = "error";
    }
    return _result_buffer;
}

void MathParserTest() {
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
}
