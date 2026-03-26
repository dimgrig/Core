#include "scpi_functions.h"

#include <cmath>

#include "Logger.h"

void parse_float_array(QByteArray &ba, QVector<double> &res_v) {
    size_t block_length_shift = 2;
    size_t block_length = 0;

    //qCDebug(logDebug) << res << res.size();
    if (ba.size() > 0) {
        if (ba.front() == '#') {
            if (ba.size() > (int)block_length_shift) {
                size_t digit_size = ba.at(1) - '0';
                //qCDebug(logDebug) << "digit_size" << digit_size;
                if (ba.size() > (int)(block_length_shift + digit_size)) {
                    int shift = digit_size;
                    --shift;
                    for (auto it = ba.begin() + block_length_shift; it < ba.begin() + block_length_shift + digit_size; ++it) {
                        //qCDebug(logDebug) << "digitValue" << (*it) << std::pow(10, shift) << (size_t)(((*it) - '0') * std::pow(10, shift));
                        block_length = block_length + ((size_t)(((*it) - '0') * std::pow(10, shift)));
                        --shift;
                    }
                    //qDebug(logDebug) << "block_length" << block_length;
                    if (ba.size() > (int)(block_length + block_length_shift + digit_size)) {
                        //int j = 0;
                        for (auto it = ba.begin() + block_length_shift + digit_size;
                            it < ba.begin() + block_length + block_length_shift + digit_size; ) {
                            float result = 0.0;
                            unsigned char *ptr = (unsigned char *) &result;
                            it = it + sizeof(float) - 1; //bigendian!
                            for (unsigned char i = 0; i < sizeof(float); ++i) {
                                *ptr++ = (*it);
                                --it;
                            }
                            it = it + sizeof(float) + 1;
                            res_v.push_back(result); //res_v[block*SCPI_BLOCK_SIZE + j] = result;
                            //++j;
                            //qCDebug(logDebug) << result;
                        }
                        //qCDebug(logDebug) << res_v;
                    }
                }
            }
        }
    }
}
