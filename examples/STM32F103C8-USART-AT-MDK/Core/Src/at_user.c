#include "at_user.h"

#include "usart.h"
#include "string.h"
extern UART_HandleTypeDef huart1;
static Stream at_user_stream_device;
static Stream* sdev = &at_user_stream_device;

int _at_user_sdev_print(Stream* this, const char* message)
{
	if (this == nullptr) return -1;
	int len = strlen(message);
	HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart1, (uint8_t*)message, len, 0xFFFF);
	if (ret != HAL_OK) return -2;
	return len;
}

int _at_user_sdev_available(Stream* this)
{
	if (this == nullptr) return -1;
}

int _at_user_sdev_read(Stream* this)
{
	if (this == nullptr) return -1;
}

static At_Err_t _at_user_AT(At_Param_t param);
static struct At_State _atTable[] = {
	{ "AT", AT_TYPE_CMD, _at_user_AT },
	{ AT_LABLE_TAIL, AT_TYPE_NULL, nullptr },
};
At at;

static At_Err_t _at_user_AT(At_Param_t param)
{
	at.sendInfor(&at, AT_USER_OK);
	return AT_EOK;
}

At_Err_t at_user_init(void)
{
	At_Err_t err = Stream_Init(sdev, 3, , , );
	if (err != AT_EOK) return err;
	err = At_Init(&at, _atTable, sdev, sdev, 0);
	return err;
}
