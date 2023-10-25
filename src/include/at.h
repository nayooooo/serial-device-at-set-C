#ifndef __AT_H__
#define __AT_H__

#include "at_type.h"
#include "at_stream_device.h"

struct At{
    At_State_t _atTable;
    Stream* _input_dev;
    Stream* _output_dev;
    size_t _param_max_num;
    char _terminator;
    char* _readString;
    size_t _readString_len;
};
typedef struct At At;

At_Err_t At_Init(
    At* at,
    const At_State_t atTable, Stream* input_dev, Stream* output_dev,
    size_t param_max_num, char terminator, size_t readString_len
);

#endif  // !__AT_H__
