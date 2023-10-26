#include "at.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static At_Err_t _paramClear(At_Param_t param)
{
    if (param == nullptr) return AT_ERROR;

    if (param->cmd != nullptr) {
        at_free(param->cmd);
        param->cmd = nullptr;
    }
    for (int i = 0; i < param->argc; i++) {
        if (param->argv[i]) {
            at_free(param->argv[i]);
            param->argv[i] = nullptr;
        }
    }
    size_t arg_num = sizeof(param->argv) / sizeof(char*);
    for (size_t i = param->argc + 1; i <= arg_num; i++) {
        param->argv[i] = nullptr;
    }
    param->argc = 0;

    return AT_EOK;
}

static At_Err_t _paramInit(At_Param_t param)
{
    if (param == nullptr) return AT_ERROR;

    _paramClear(param);

    return AT_EOK;
}

static At_Err_t _paramAddCmd(At_Param_t param, const char* cmd)
{
    if (param == nullptr) return AT_ERROR;

    if (param->cmd) {
        at_free(param->cmd);
        param->cmd = nullptr;
    }
    size_t cmd_len = strlen(cmd);
    param->cmd = (char*)at_malloc(cmd_len + 1);
    if (param->cmd == nullptr) return AT_ERROR;
    at_memset(param->cmd, 0, cmd_len + 1);
    at_memcpy(param->cmd, cmd, cmd_len);

    return AT_EOK;
}

static At_Err_t _paramAddArg(At_Param_t param, const char* arg)
{
    if (param == nullptr) return AT_ERROR;
    size_t arg_num = sizeof(param->argv) / sizeof(char*);
    if (param->argc >= arg_num) return AT_ERROR;

    if (param->argv[param->argc]) {
        at_free(param->argv[param->argc]);
        param->argv[param->argc] = nullptr;
    }
    size_t arg_len = strlen(arg);
    param->argv[param->argc] = (char*)at_malloc(arg_len + 1);
    if (param->argv[param->argc] == nullptr) return AT_ERROR;
    at_memset(param->argv[param->argc], 0, arg_len + 1);
    at_memcpy(param->argv[param->argc], arg, arg_len);

    param->argc++;
    return AT_EOK;
}

static At_Err_t _cutString(At* this, At_Param_t param, const char* atLable)
{
	char* str = (char*)atLable;
    char* str_temp = nullptr;

    _paramClear(param);
    _paramAddCmd(param, AT_LABLE_TAIL);

	// find at lable
	str_temp = strtok(str, " \r\n");
    _paramAddCmd(param, str_temp);
	// find at param
	for (int i = 0; i < this->getParamMaxNum(this); i++)
	{
		str_temp = strtok(NULL, " \r\n");
        _paramAddArg(param, str_temp);
		if (str_temp == nullptr)
			break;
		param->argc++;
	}

	return AT_EOK;
}

static At_State_t _checkString(At* this, At_Param_t param, const char* atLable)
{
	uint32_t i = 0;
	At_State_t target = nullptr;

    _paramInit(param);
	this->cutString(this, param, atLable);

	while (this->_atTable[i].atLable != AT_LABLE_TAIL)
	{
        if (!at_memcmp(this->_atTable[i].atLable, param->cmd, strlen(this->_atTable[i].atLable)))
		{
			target = &this->_atTable[i];
			break;
		}
		i++;
	}

    return target;
}

static size_t _getParamMaxNum(At* this)
{
    if (this == nullptr) return -1;
    return this->_param_max_num;
}

static At_State_t _getStateTable(At* this)
{
    if (this == nullptr) return nullptr;
    return this->_atTable;
}

static At_Err_t _setInputDevice(At* this, Stream* input_dev)
{
    if (this == nullptr) return AT_ERROR;
    if (input_dev == nullptr) return AT_ERROR;
    this->_input_dev = input_dev;
    return AT_EOK;
}

static At_Err_t _setOutputDevice(At* this, Stream* output_dev)
{
    if (this == nullptr) return AT_ERROR;
    if (output_dev == nullptr) return AT_ERROR;
    this->_output_dev = output_dev;
    return AT_EOK;
}

// dangerous
static const char* _errorToString(At_Err_t error)
{
    static char error_str[50];

    at_memset(error_str, 0, sizeof(error_str));
	switch (error)
	{
	case AT_ERROR:
        at_memcpy(error_str, "AT normal error", 16); break;
	case AT_ERROR_INPUT:
        at_memcpy(error_str, "AT input device error", 22); break;
	case AT_ERROR_OUTPUT:
        at_memcpy(error_str, "AT output device error", 23); break;
	case AT_ERROR_NOT_FIND:
        at_memcpy(error_str, "AT not find this string command", 32); break;
	case AT_ERROR_NO_ACT:
        at_memcpy(error_str, "AT this string command not have act", 36); break;
	case AT_ERROR_CANNOT_CUT:
        at_memcpy(error_str, "AT this string can't be cut", 28); break;
    default:
        at_memcpy(error_str, "AT no error", 12); break;
	}
    return error_str;
}

// safer
static const char* _errorToString_s(At_Err_t error)
{
    static char error_str[12];
    at_memset(error_str, 0, sizeof(error_str));
    at_memcpy(error_str, "building...", 11);
    return error_str;
}

static At_Err_t _handle(At* this, const char* atLable)
{
    if (this == nullptr) return AT_ERROR;

	struct At_Param param;
    At_State_t target = this->checkString(this, &param, atLable);

	if (target == nullptr)
		return AT_ERROR_NOT_FIND;
	if (target->act == nullptr)
		return AT_ERROR_NO_ACT;

    At_Err_t ret = target->act(&param);
    _paramClear(&param);
    return ret;
}

static size_t _printf(struct At* this, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	char temp[64] = { 0 };
	char *buffer = temp;
	size_t len = vsnprintf(temp, sizeof(temp), format, arg);
	va_end(arg);
	if (len > sizeof(temp) - 1) {
        buffer = at_malloc((len + 1) * sizeof(char));
		if (!buffer) return 0;
		at_memset(buffer, 0, len + 1);
		va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
	}
	this->print(this, buffer);
    if (buffer != temp) {
        at_free(buffer);
        buffer = nullptr;
    }

	return len;
}

static size_t _print(struct At* this, const char* message)
{
    if (this == nullptr) return 0;
    if (this->_output_dev == nullptr) return 0;
    return this->_output_dev->print(this->_output_dev, message);
}

static At_Err_t _At_Init(
    At* this,
    const At_State_t atTable, Stream* input_dev, Stream* output_dev,
    size_t param_max_num, char terminator, size_t readString_len
)
{
    if (this == nullptr) return AT_ERROR;
    if (atTable == nullptr) return AT_ERROR;
    if (input_dev == nullptr) return AT_ERROR;
    if (output_dev == nullptr) return AT_ERROR;
    if (readString_len <= 3) return AT_ERROR;  // AT+

    this->_atTable = atTable;
    this->_input_dev = input_dev;
    this->_output_dev = output_dev;
    this->_param_max_num = param_max_num;
    this->_terminator = terminator;
    this->_readString_len = readString_len;
    this->_readString = (char*)at_malloc(readString_len * sizeof(char));
    if (this->_readString == nullptr) return AT_ERROR;
    at_memset(this->_readString, 0, this->_readString_len);
    this->_readString_used = 0;

    this->cutString = _cutString;
    this->checkString = _checkString;

    this->getParamMaxNum = _getParamMaxNum;
    this->getStateTable = _getStateTable;

    this->setInputDevice = _setInputDevice;
    this->setOutputDevice = _setOutputDevice;

    this->errorToString = _errorToString;

    this->handle = _handle;

    this->printf = _printf;
    this->print = _print;

    return AT_EOK;
}

At_Err_t At_Init(
    At* this,
    const At_State_t atTable, Stream* input_dev, Stream* output_dev,
    size_t argc, ...
)
{
    if (argc == 0) {
        return _At_Init(this, atTable, input_dev, output_dev, AT_PARAM_MAX_NUM, AT_TERMINATOR_DEFAULT, AT_READSTRING_LEN_DEFAULT);
    } else if (argc > 3) return AT_ERROR;
    va_list args;
    va_start(args, argc);
    size_t temp = argc;
    size_t param_max_num; char terminator; size_t readString_len;
    param_max_num = va_arg(args, size_t); if (--temp) {
        va_end(args);
        return _At_Init(this, atTable, input_dev, output_dev, param_max_num, AT_TERMINATOR_DEFAULT, AT_READSTRING_LEN_DEFAULT);
    }
    terminator = va_arg(args, char); if (--temp) {  // dangerous at "va_arg(args, char)", the "char"
        va_end(args);
        return _At_Init(this, atTable, input_dev, output_dev, param_max_num, terminator, AT_READSTRING_LEN_DEFAULT);
    }
    readString_len = va_arg(args, size_t); if (--temp) {
        va_end(args);
        return _At_Init(this, atTable, input_dev, output_dev, param_max_num, terminator, readString_len);
    }
    va_end(args);

    return AT_ERROR;
}

At_Err_t At_Init_s(
    At* this,
    const At_State_t atTable, Stream* input_dev, Stream* output_dev,
    size_t param_max_num, char terminator, size_t readString_len
)
{
    return _At_Init(this, atTable, input_dev, output_dev, param_max_num, terminator, readString_len);
}

At_Err_t At_Deinit(At* this)
{
    if (this == nullptr) return AT_ERROR;

    if (this->_readString) {
        at_free(this->_readString);
        this->_readString = nullptr;
    }

    return AT_EOK;
}
