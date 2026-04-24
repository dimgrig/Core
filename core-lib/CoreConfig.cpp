#include "CoreConfig.h"

const QStringList ConnectionType_sl = {{ConnectionType_to_text(ConnectionTypeSerial),
                                         ConnectionType_to_text(ConnectionTypeTCP)}};
const QStringList SerialBaudRate_sl = {{SerialBaudRate_to_text(SerialBaud1200),
                                         SerialBaudRate_to_text(SerialBaud2400),
                                         SerialBaudRate_to_text(SerialBaud4800),
                                         SerialBaudRate_to_text(SerialBaud9600),
                                         SerialBaudRate_to_text(SerialBaud19200),
                                         SerialBaudRate_to_text(SerialBaud38400),
                                         SerialBaudRate_to_text(SerialBaud57600),
                                         SerialBaudRate_to_text(SerialBaud115200)}};
const QStringList SerialDataBits_sl = {{SerialDataBits_to_text(SerialDataBits5),
                                         SerialDataBits_to_text(SerialDataBits6),
                                         SerialDataBits_to_text(SerialDataBits7),
                                         SerialDataBits_to_text(SerialDataBits8)}};
const QStringList SerialStopBits_sl = {{SerialStopBits_to_text(SerialStopBits_OneStop),
                                         SerialStopBits_to_text(SerialStopBits_TwoStop),
                                         SerialStopBits_to_text(SerialStopBits_OneAndHalfStop)}};
const QStringList SerialParity_sl = {{SerialParity_to_text(SerialParity_NoParity),
                                       SerialParity_to_text(SerialParity_EvenParity),
                                       SerialParity_to_text(SerialParity_OddParity),
                                       SerialParity_to_text(SerialParity_SpaceParity),
                                       SerialParity_to_text(SerialParity_MarkParity)}};
const QStringList BytesFormat_sl = {{BytesFormat_to_text(BigEndian),
                                      BytesFormat_to_text(BigEndianByteSwap),
                                      BytesFormat_to_text(LittleEndianByteSwap),
                                      BytesFormat_to_text(LittleEndian)}};
