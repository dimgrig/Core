#ifndef MATHPARSER_H
#define MATHPARSER_H

#include <QString>
#include "muParser.h"

#include "core-utils-lib-global.h"

CORE_UTILS_LIB_EXPORT void MathParserTest();

class CORE_UTILS_LIB_EXPORT MathParser {
public:
    explicit MathParser();
    ~MathParser();

    QString parse(QString expression);
    double get_result() {return _result;};
private:
    void define_parser();

    mu::Parser _p;
    double _result;
    QString _result_buffer;
    QString _expression;
};

#endif // MATHPARSER_H
