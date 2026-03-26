#ifndef CORECONFIG_H
#define CORECONFIG_H

#include <QColor>
#include <qnamespace.h>

#include "enum_macro.h"

#if (QT_VERSION > 0x050F00)
#define QT_HEX Qt::hex
#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define QT_HEX ""
#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

#define _US 1000000

#define PARAMS_DELIMETER    ", "            // ",\n" = not working due to test raw delimeter - cant copy to txt
#define ELEMENTS_DELIMETER  ";"
#define ROW_DELIMETER       "\r\n"

#define LOG_DT_FORMAT "yyyy-MM-dd HH:mm:ss.zzz"

#define TIME_FORMAT "HH:mm:ss"
#define TIME_FORMAT_SHORT "mm:ss"
#define TIME_FORMAT_MS "HH:mm:ss.zzz"
#define ISO_DATE_FORMAT "yyyy-MM-dd"
#define DT_FORMAT "yyyy-MM-dd HH:mm:ss.zzz"
#define DT_FORMAT_WITHOUT_MS "yyyy-MM-dd HH-mm-ss"
#define DT_FORMAT_SHORT "HH:mm:ss"
#define DT_FORMAT_WRAP "yyyy-MM-dd\nHH:mm:ss.zzz"
#define DATE_FORMAT "dd.MM.yyyy"
#define DATE_TIME_FORMAT_WITHOUT_MS "dd.MM.yyyy HH:mm:ss"
#define DATE_TIME_FORMAT_WITHOUT_MS_WRAP "dd.MM.yyyy\nHH:mm:ss"

#define SETTINGS_PATH           "settings/"
#define __ORGANIZATION_NAME        "ACS"

#define DEGREE_SYMBOL QChar(176)

#define PXI_ERROR_VALUE -999.999

#define CONTROL_MINIMUM_WIDTH 150
#define CONTROL_MINIMUM_HEIGHT 25

#define _TRANSPARENT QColor(0, 0, 0, 0)
#define BLACK_COLOR "#000000"
#define DGRAY_COLOR "#333333"
#define GRAY_COLOR "#999999"
#define LGRAY_COLOR "#CCCCCC"
#define WHITE_COLOR "#FFFFFF"

#define R_COLOR "#FF3E3E" //#"#CD5C5C"
#define G_COLOR "#32CD32"
#define B_COLOR "#4169E1"

#define LY_COLOR "#FFFF4C"
#define Y_COLOR "#FFF200"
#define DY_COLOR "#FEE12B"

#define LO_COLOR "#FFBE00"
#define O_COLOR "#FFA500" //#for TextItems Avaria
#define DO_COLOR "#FF8C00" //#FF953E

#define LG_COLOR "#90EE90" //#for TextItems
#define DG_COLOR "#006400"

#define LB_COLOR "#ADD8E6" //#for TextItems
#define DB_COLOR "#00008B"

#define _LR_COLOR "#FA8072" //#for TextItems
#define DR_COLOR "#B22222" //#8B0000

#define R_BG_COLOR QColor(255, 0, 0, 20) //"#FF6666"


#define V_COLOR "#EE82EE" //for VP
#define DV_COLOR "#9400D3"

#define SB_COLOR "#87CEEB" //for O2
#define LSB_COLOR "#B0E0E6" //87CEFA
#define DSB_COLOR "#00BFFF"


#define COLOR_Silver	"#C0C0C0"
#define COLOR_Gray	    "#808080"
#define COLOR_Black	    "#000000"
#define COLOR_Red	    "#FF0000"
#define COLOR_Maroon	"#800000"
#define COLOR_Yellow	"#FFFF00"
#define COLOR_Olive	    "#808000"
#define COLOR_Lime	    "#00FF00"
#define COLOR_Green	    "#008000"
#define COLOR_Aqua	    "#00FFFF"
#define COLOR_Teal	    "#008080"
#define COLOR_Blue	    "#0000FF"
#define COLOR_Navy	    "#000080"
#define COLOR_Fuchsia	"#FF00FF"
#define COLOR_Purple	"#800080"
#define COLOR_Orange	    "#FFA500"
#define COLOR_DeepPink	    "#FF1493"
#define COLOR_Chocolate	    "#D2691E"
#define COLOR_DarkSlateGray	"#2F4F4F"
#define COLOR_SandyBrown	"#F4A460"
#define COLOR_LightSteelBlue "#B0C4DE"
#define COLOR_Aquamarine	"#7FFFD4"
#define COLOR_PaleGreen	    "#98FB98"
#define COLOR_Violet	    "#EE82EE"

ENUM_MACRO2(ConnectionType, ConnectionTypeSerial, ConnectionTypeTCP,
            Serial, TCP,
            Serial, TCP,
            0, 1);

ENUM_MACRO8(SerialBaudRate, SerialBaud1200 = 1200, SerialBaud2400 = 2400, SerialBaud4800 = 4800, SerialBaud9600 = 9600,
            SerialBaud19200 = 19200, SerialBaud38400 = 38400, SerialBaud57600 = 57600, SerialBaud115200 = 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200);

ENUM_MACRO4(SerialDataBits, SerialDataBits5 = 5, SerialDataBits6 = 6, SerialDataBits7 = 7, SerialDataBits8 = 8,
            5, 6, 7, 8,
            5, 6, 7, 8,
            5, 6, 7, 8);

ENUM_MACRO3(SerialStopBits, SerialStopBits_OneStop = 1, SerialStopBits_TwoStop = 2, SerialStopBits_OneAndHalfStop = 3,
            1, 2, 1.5,
            1, 2, 1.5,
            1, 2, 3);

ENUM_MACRO5(SerialParity, SerialParity_NoParity = 0, SerialParity_EvenParity = 2, SerialParity_OddParity = 3,
            SerialParity_SpaceParity = 4, SerialParity_MarkParity = 5,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            0, 2, 3, 4, 5);

// big-endian
// big-endian слова, но байты в словах переставлены
// little-endian слова, но байты в словах не переставлены
// little-endian
ENUM_MACRO4(BytesFormat, BigEndian, BigEndianByteSwap, LittleEndianByteSwap, LittleEndian,
            ABCD, BADC, CDAB, DCBA,
            ABCD, BADC, CDAB, DCBA,
            0, 1, 2, 3);

extern const QStringList ConnectionType_sl;
extern const QStringList SerialBaudRate_sl;
extern const QStringList SerialDataBits_sl;
extern const QStringList SerialStopBits_sl;
extern const QStringList SerialParity_sl;
extern const QStringList BytesFormat_sl;

#endif // CORECONFIG_H
