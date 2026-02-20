#include <QtTest>

#include "Approximation.h"
#include "enum_macro.h"
#include "functions.h"
#include "Logger.h"
#include "macroses.h"
#include "MathParser.h"
#include "Profiler.h"
#include "templates.h"
#include "TestRunner.h"

#include "modbus_functions.h"
#include "ModbusDataConverter.h"
#include "ModbusRegisterSerializer.h"

#include "TestModbusIntegration.h"

int main(int argc, char *argv[])
{
    int status = 0;
    QApplication app(argc, argv);

    Profiler::_enable = true;
    TestRunner::_enable = true;

    qCDebug(logDebug) << "=============================================";
    qCDebug(logDebug) << "=================Core tests==================";
    qCDebug(logDebug) << "=============================================";

    TestRunner tr;
    RUN_TEST(tr, TemplatesTest);
    RUN_TEST(tr, FunctionsTest);
    RUN_TEST(tr, EnumMacroTest);
    RUN_TEST(tr, MacrosesTest);
    RUN_TEST(tr, ApproximationTest);
    RUN_TEST(tr, MathParserTest);

    RUN_TEST(tr, ModbusFunctionsTest);
    RUN_TEST(tr, ModbusDataConverterTest);
    RUN_TEST(tr, ModbusRegisterSerializerTest);

    TestModbusIntegration tmi;
    status |= QTest::qExec(&tmi, argc, argv);

    qCDebug(logDebug) << "=============================================";
    qCDebug(logDebug) << "=================Core tests done=============";
    qCDebug(logDebug) << "=============================================";

    return status;
}
