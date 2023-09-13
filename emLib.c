/**
 * @file emLib.c
 * @author Peng (Peng2333)(Peng2333@vip.qq.com.com)
 * @authors Peng emmitt (Peng-22)(emmitt2333@qq.com)
 *
 * @brief 一些常用的C函数
 * @version 0.3
 * @date 2020-09-01
 *
 * @encoding UTF-8
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "emLib.h"

// C库
#include "stdarg.h"

#include "usart.h"

// 串口打印库暂时可以使用AC6编译器，但是无法使用gnu模式，只能是C99模式
// 本意是为了不去重定义printf函数的。就算使用自己的printf，也需要引入这一段重定义，但是使用这一段之后，printf也可以用了

/* ------------------通过重定向将printf函数映射到串口1上-------------------*/
#if !defined(__MICROLIB)

// #pragma import(__use_no_semihosting)
__asm(".global __use_no_semihosting\n\t");
void _sys_exit(int x) // 避免使用半主机模式
{
	x = x;
}
//__use_no_semihosting was requested, but _ttywrch was
void _ttywrch(int ch)
{
	ch = ch;
}
struct __FILE // AC6，不用这个
{
	int handle;
};
FILE __stdout;

#endif

#if defined(__GNUC__) && !defined(__clang__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
	unsigned int TimeOut = 1000000;
	USART_TypeDef *USARTx = USART_DEBUG_PROT;

	/* 实现串口发送一个字节数据的函数 */
	// serial_write(&serial1, (uint8_t)ch); //发送一个自己的数据到串口
	//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

#if (defined STM32H743xx) // 目前这些型号只适配了遇见过的芯片，对于新芯片只需查看"USART_TypeDef"的定义即可
	while (((USARTx->ISR & 0X40) == 0) && --TimeOut)
		;
	USARTx->TDR = (uint8_t)ch;
#elif (defined STM32F10X_HD) || (defined STM32F10X_MD) || (defined STM32F103xE) || (defined STM32F411xE) || (defined STM32F407xx) || (defined STM32F40_41xxx)
	while (((USARTx->SR & 0X40) == 0) && --TimeOut)
		; // 等待发送完成
	USARTx->DR = (uint8_t)ch;
#else
#error "检查上面的设置，可能没有定义目前相关的芯片"
#endif
	if (TimeOut == 0 || (TimeOut + 1 == 0))	// 如果这个超时，多数情况就是初始化串口之前使用了打印函数
	{
		while (1)
			;
	}

	return ch;
}

// char Printf_buff[2048];

/**
 * @brief 额，没有区别和sprintf
 * 
 * @param buf 
 * @param format 
 * @param ... 
 * @return int 
 */
int Em_sprintf(char *buf, const char *format, ...)
{
	va_list args;
	int ret = 0;

	va_start(args, format);
	ret = vsprintf(buf, format, args);
	va_end(args);

	return ret;
}

// 返回发送的字符串长度，如果失败则返回-1
__IO int USART_printf(USART_TypeDef *USARTx, char *format, ...)
{
	va_list args;
	char Char_Buff[1024]; // 最多只能发送1023个字符,
	unsigned int TimeOut = 1000000;
	char *pStr = Char_Buff;
	int ret = 0;

	memset(Char_Buff, 0, sizeof Char_Buff); // 为了快速可以不加这一句

	va_start(args, format);
	ret = vsprintf(Char_Buff, format, args);
	va_end(args);

	while (*pStr != 0)
	{
#if (defined STM32H743xx) // 目前这些型号只适配了遇见过的芯片，对于新芯片只需查看"USART_TypeDef"的定义即可
		while (((USARTx->ISR & 0X40) == 0) && --TimeOut)
			;
		USARTx->TDR = (uint8_t)*pStr++;
#elif (defined STM32F10X_HD) || (defined STM32F10X_MD) || (defined STM32F411xE)
		while (((USARTx->SR & 0X40) == 0) && --TimeOut)
			; // 等待发送完成
		USARTx->DR = (uint8_t)*pStr++;
#endif
	}

	if (TimeOut == 0) // 超时未发送成功，则是返回失败
		return -1;
	else
		return ret;
}

/* 如果调试需要参数检查，请打开此宏定义，具体参考本文头文件中说明 *****************************/
#ifdef USE_FULL_ASSERT

// 一般在main.c里面已经实现此函数
__WEAK void assert_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
    Em_ERROR("Wrong parameters value: file %s on line %d\r\n", file, line);

    // while (1)
    // {
    // }
}
#endif

/******************************************************************************************************/
/**
 * @brief  通过宏定义正确定位到出错那一行。
 * @param  传入函数名字符串等要显示的信息。
 * @retval None
 * @{
 */
/**  为了全局可见, 在EmLib.h定义
//#define Em_ERROR(expr)  __Em_ERROR((uint8_t *)__FILE__, __LINE__, (expr))
	*/

/** @}
 */

// 角度转弧度
double AngleToRadian(double _Angle)
{
	return (_Angle / 180.0) * Pi;
}

// 弧度转角度
double RadianToAngle(double _Radian)
{
	return (_Radian / Pi) * 180.0;
}

// 在字符串buf结尾添加格式化字符串
// 返回添加字节长度。注意dest数组长度一定要够长，不然后面的内存区域里面的数据就危险喽~
int Addstr(char *dest, const char *format, ...)
{
	va_list args;
	char buf[1024];
	int src_len = 0;

	memset(buf, 0, sizeof buf);

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	src_len = strlen(buf); // 为了下面两处节省时间提前计算

	strcpy(dest, buf);
	return src_len;
}

// 在源（参数1）中查找处字符串str1，返回源中首次出现str1字符串之后第一位
char *lookupstr(const char *source, char *str1)
{ // 搜寻字符串str1在source中第一次出现位置, 字符串str1之后的第一个位置
	uint16_t str1_len = strlen(str1);
	if (strstr(source, str1) != NULL)
		return (strstr(source, str1) + str1_len);
	else
		return NULL;
}

// 在源（参数1）中查找处字符串str1，返回源中首次出现str1字符串之前的字符串，存入目标数组
char *lookuplast(const char *source, char *str1, char *TargetArray)
{
	int str_len;

	memset(TargetArray, 0, strlen(TargetArray)); // 清空目标存储区域
	if (strstr(source, str1) == NULL)
	{ // 如果找不到str1，直接返回源数组
		//  	printf("Don't find!\r\n");
		memcpy(TargetArray, source, strlen(source));
		return (char *)source;
	}
	str_len = (strstr(source, str1) - source); // 字符串头位置指针差值
	memcpy(TargetArray, source, str_len);
	return TargetArray; // 通过返回值读取存储区的变量
}

// 在源字符串中提取字符串1和字符串2之间字符串，存入目标数组
char *lookupBetween(const char *source, char *str1, char *str2, char *TargetArray)
{
	char *p = lookupstr(source, str1);
	if (p == NULL)
		return NULL;

	lookuplast(p, str2, TargetArray);
	return TargetArray;
}

int Em_isdigit(int c)
{
	return ('0' <= c && c <= '9') ? 1 : 0;
}

long long int pow_int(int _X, int _Y)
{
	long long int result = 1;
	int i;
	for (i = 0, result = 1; i < _Y; i++)
		result = result * _X;
	return result;
}

double Em_atof(const char *str)
{
	const char *p = str;
	int sign = 1;								 // 存储符号位
	int hasDot = 0, hasE = 0;					 // 标记是否有小数点，是否有E或e
	double integerPart = 0.0, decimalPart = 0.0; // 存储整数部分，存储小数部分
	int decimalDigits = 1;
	int exponential = 0;
	double Result;

	while (*p == ' ')
		++p; // 忽略前置空格

	if (*p == '-') // 检查是否有符号位
	{
		sign = -1;
		++p;
	}
	else if (*p == '+')
		++p;

	// 遇到'e'或'.'字符则退出循环,设置hasE和hasDot。
	for (; *p; ++p)
	{
		if (Em_isdigit(*p)) // 若p指向的字符为数字则计算当前整数部分的值
			integerPart = 10 * integerPart + *p - '0';
		else if (*p == '.')
		{
			hasDot = 1;
			p++;
			break;
		}
		else if (*p == 'e' || *p == 'E')
		{
			hasE = 1;
			p++;
			break;
		}
		else // 如果遇到非法字符,则截取合法字符得到的数值,返回结果。
			return integerPart * sign;
	}

	// 上一部分循环中断有三种情况,一是遍历完成,这种情况下一部分的循环会自动跳过；其次便是是遇到'.'或'e',两种hasE和hasDot只可能一个为真,若hasDot为真则计算小数部分,若hasE为真则计算指数部分。
	for (; *p; p++)
	{
		if (hasDot && Em_isdigit(*p))
			decimalPart += ((*p - '0') * 1.0) / pow_int(10, decimalDigits++); // 修改了原作者这里小数无法计算问题
		else if (hasDot && (*p == 'e' || *p == 'E'))
		{
			integerPart += decimalPart;
			decimalPart = 0.0;
			hasE = 1;
			++p;
			break;
		}
		else if (hasE && Em_isdigit(*p))
			exponential = 10 * exponential + *p - '0';
		else
			break;
	}
	// 上一部分较难理解的就是else if (hasDot && (*p == 'e' || *p == 'E')) 这一特殊情况,对于合法的浮点数,出现'.'字符后,仍然有可能是科学计数法表示,但是出现'e'之后,指数部分不能为小数(这符合<string.h>对atof()的定义)。这种情况变量IntegerPart和decimalPart都是科学计数法的基数,因此有integerPart += decimalPart(这使得IntergerPart的命名可能欠妥,BasePart可能是一种好的选择)。
	// 上一部分循环结束一般情况下就能返回结果了,除非遇到前文所述的特殊情况，对于特殊情况需要继续计算指数。
	if (hasE && hasDot)
	{
		for (; *p; p++)
		{
			if (Em_isdigit(*p))
				exponential = 10 * exponential + *p - '0';
		}
	}
	Result = sign * (integerPart * pow_int(10, exponential) + decimalPart);
	return Result;
}

double Em_atof_Lite(const char *str)
{
	const char *p = str;
	int sign = 1;								 // 存储符号位
	int hasDot = 0;								 // 标记是否有小数点
	double integerPart = 0.0, decimalPart = 0.0; // 存储整数部分，存储小数部分
	double Result;

	int decimalDigits = 1;

	while (*p == ' ')
		++p; // 忽略前置空格

	if (*p == '-') // 检查是否有符号位
	{
		sign = -1;
		++p;
	}
	else if (*p == '+')
		++p;

	//
	for (; *p; p++)
	{
		if (Em_isdigit(*p)) // 若p指向的字符为数字则计算当前整数部分的值
			integerPart = (10 * integerPart + (*p - '0'));
		else if (*p == '.')
		{
			hasDot = 1;
			p++;
			break;
		}
		else // 如果遇到非法字符,则截取合法字符得到的数值,返回结果。
			return integerPart * sign;
	}

	// 计算小数部分
	for (; *p; p++)
	{
		if (hasDot && Em_isdigit(*p))
			decimalPart += ((*p - '0') * 1.0) / pow_int(10, decimalDigits++);
		else
			break;
	}

	Result = sign * (integerPart + decimalPart);
	return Result;
}

// f(被限制的数据的地址，限制量)
void *Limit(double *Limit_Data, double Limit_)
{
	if (*Limit_Data > Abs(Limit_))
		*Limit_Data = Abs(Limit_);
	else if (*Limit_Data < -Abs(Limit_))
		*Limit_Data = -Abs(Limit_);

	return Limit_Data;
}

// 来自正点原子USMART实验*****************************************************
// 读取指定地址的值
uint32_t read_addr(uint32_t addr) { return *(uint32_t *)addr; }
// 在指定地址写入指定的值
void write_addr(uint32_t addr, uint32_t val) { *(uint32_t *)addr = val; }

/**	@brief
 *	@param __i 输入想要转二进制的值
 * @param ptr 传入一个数组空间指针，长度必须大于32字节，用于显示32位的数据
 * @retval 返回值即生成的字符串的(头)指针，方便直接使用返回值打印，或者也可使用结构体中的成员
 *
 * @note
 */
char *int32_to_Binary(int32_to_Binary_TypeDef *Dataptr)
{
	int32_t tmp = Dataptr->Value_int32_t;
	uint8_t i = 0;								   // 原数据的位索引
	int8_t j = sizeof(Dataptr->Result_String) - 2; // 结果字符串下标索引， 从倒数第二位开始向前填充

	memset(Dataptr->Result_String, ' ', sizeof(Dataptr->Result_String));

	for (i = 0, j = sizeof(Dataptr->Result_String) - 2; i < 32; i++)
	{ // 32是原数据的位数
		if (j - 1 < 0)
		{ // 在结构体中数组长度设置合理(>= 39)时用不到这些
			Em_Warning("数组将在下面填充中溢出, 当前:%d, 下一步:%d\r\n原数据已解析至第%d位，正在停止", j, j - 1, i + 1);
			break; //
				   //
		}
		Dataptr->Result_String[j--] = (char)((tmp & (1 << i)) ? '1' : '0'); // 一位一位的找。倒序填充
																			// 此处用单引号，编译位ASCII码。使用双引号会返回字符串存储地址
		if ((i + 1) % 8 == 0 && j >= 2)
		{									   // 因为变量i要在这次循环完了之后才自加，所以这里要+1补充，而且这样可以躲开0
											   //		大于2会这样："0b11111110", 等于2: "0b 11111110", 。只要结构体定义中字符串足够，这个判断条件（≥2这个）可有可无
											   //		主要是0b后面，大于2时，紧挨着只有一个空格的时候不再输出空格，
			Dataptr->Result_String[j--] = ' '; // 插入一个空格
		}
		if ((i + 1) % 16 == 0 && j >= 2)
		{
			Dataptr->Result_String[j--] = ' '; // 有效位第16位后多插入一个空格（如下）
											   //|0b 11111111 11111111  11111111 11111111
											   //|--------------------------------------
		}
	}

	Dataptr->Result_String[sizeof(Dataptr->Result_String) - 1] = 0; // 将最后一位设0，用于停止打印字符
	Dataptr->Result_String[0] = '0';								// 二进制表示
	Dataptr->Result_String[1] = 'b';								// 二进制表示

	//	Em_INFOR("%s", Dataptr->Result_String);
	return Dataptr->Result_String;
}
