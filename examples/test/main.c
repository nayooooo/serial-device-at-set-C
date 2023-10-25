#include "at.h"

#include <stdio.h>

struct At_State _at_table[] = {
    { AT_LABLE_TAIL, AT_TYPE_NULL,  },
};

int main()
{
    At at;
    Stream dev;
    At_Init(&at, _at_table, &dev, &dev, AT_PARAM_MAX_NUM, '\n', 200);
    printf("_atTable:        %p\r\n", at._atTable);
    printf("_input_dev:      %p\r\n", at._input_dev);
    printf("_output_dev:     %p\r\n", at._output_dev);
    printf("_param_max_num:  %d\r\n", (int)at._param_max_num);
    printf("_terminator:     %c\r\n", at._terminator);
    printf("_readString:     %s\r\n", at._readString);
    printf("_readString_len: %d\r\n", (int)at._readString_len);

    return 0;
}
