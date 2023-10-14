/**
 * @file emLib.c
 * @author Peng (Peng2333@vip.qq.com.com)
 * @authors Peng emmitt (emmitt2333@qq.com)
 *
 * @brief 一些常用的C函数
 * @version 0.3
 * @date 2020-09-01
 *
 * @encoding GB2312
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "emLib.h"

// C库
#include "stdarg.h"

// #include "usart.h"

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

KEYWORD_WEAK PUTCHAR_PROTOTYPE
{
	unsigned int TimeOut = 1000000;
	USART_TypeDef *USARTx = USART_DEBUG_PROT;

	/* 实现串口发送一个字节数据的函数 */
	// serial_write(&serial1, (uint8_t)ch); //发送一个自己的数据到串口
	//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

// 目前这些型号只适配了遇见过的芯片，对于新芯片只需查看"USART_TypeDef"的定义即可
#if (defined STM32H743xx) || (defined STM32F030)
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
	if (TimeOut == 0 || (TimeOut + 1 == 0)) // 如果这个超时，多数情况就是初始化串口之前使用了打印函数
	{
		while (1)
		{
			if (0)
				break;
		}
	}

	return ch;
}

// char Printf_buff[2048];

/**
 * @brief 功能上和sprintf没有区别
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
/**
 * @brief
 *
 * @param USARTx
 * @param format
 * @param ...
 * @return int
 *
 * @note 函数中设定最大只能发送1023个字符, 增大空间需要注意stack大小
 */
int USART_printf(USART_TypeDef *USARTx, char *format, ...)
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
		pStr++;

// 目前这些型号只适配了遇见过的芯片，对于新芯片只需查看"USART_TypeDef"的定义即可
#if (defined STM32H743xx) || (defined STM32F030)
		while (((USARTx->ISR & 0X40) == 0) && --TimeOut)
			;
		USARTx->TDR = (uint8_t)*pStr;
#elif (defined STM32F10X_HD) || (defined STM32F10X_MD) || (defined STM32F103xE) || (defined STM32F411xE) || (defined STM32F407xx) || (defined STM32F40_41xxx)
		while (((USARTx->SR & 0X40) == 0) && --TimeOut)
			; // 等待发送完成
		USARTx->DR = (uint8_t)*pStr;
#else
#error "检查上面的设置，可能没有定义目前相关的芯片"
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

	while (1)
	{
	}
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
		//  	printf_Em("Don't find!\r\n");
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

/**
 * @brief 读取指定地址的值
 *
 * @param addr
 * @return uint32_t
 */
uint32_t read_addr(uint32_t addr) { return *(uint32_t *)addr; }

/**
 * @brief 在指定地址写入指定的值
 *
 * @param addr
 * @param val
 */
void write_addr(uint32_t addr, uint32_t val) { *(uint32_t *)addr = val; }

/**	
 * @brief
 * @param __i 输入想要转二进制的值
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

/* Systick ********************************************************************/

/* 计时变量 */
__IO uint64_t __Tick_ms = 0; // 计数毫秒值
__IO uint64_t __Tick_us = 0; // 计数微秒值

__IO uint64_t __Tick_us_TIM = 0; // 定时器计数使用的计数微秒值

/* 配置变量 */					  // 下面这个有空解决一下
const uint32_t __TickFreq = 1000; // 定时器溢出频率 (Hz)	// 1000 us一次
uint32_t __TickInc_ms = 0;		  // 每次达到中断，ms计数变量的增量值，每次中断不足1 us，也将会被设置为1，但是不是每次中断都递增__Tick_ms
uint32_t __TickInc_us = 0;		  // 每次达到中断，ms计数变量的增量值

/**
 * @brief 初始化Systick用于延时函数
 *
 * @note 加个前缀可以在键入"delay..."的时候编辑器不会第一个提示自动补全这个
 *
 */
void em_delay_init(void)
{
	RCC_ClocksTypeDef RCC_ClocksTypeDefStructure = {0};
	uint32_t SYSCLK_Frequency = 0;
#if DEBUG_SYSCLOCK || 0
	Em_INFOR("\r\n正在配置Systick:");
#endif /* DEBUG_SYSCLOCK */

	if (__TickFreq > 1000000)
	{
		Em_ERROR("__TickFreq = %d设置过大", __TickFreq);
	}
	else if (__TickFreq == 0)
	{
		Em_ERROR("__TickFreq = %d 值设置错误", __TickFreq);
	}

	/* 配置时钟 **********************/
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	RCC_GetClocksFreq(&RCC_ClocksTypeDefStructure);
	SYSCLK_Frequency = RCC_ClocksTypeDefStructure.SYSCLK_Frequency; // 取得时钟频率
#if DEBUG_SYSCLOCK
	Em_printf_var("%d", SYSCLK_Frequency, "");
#endif /* DEBUG_SYSCLOCK */

	/* 设置装载值 ********************/
	SysTick->LOAD = ((SYSCLK_Frequency / (__TickFreq)) & SysTick_LOAD_RELOAD_Msk) - 1; /* set reload register */
#if DEBUG_SYSCLOCK
	Em_printf_var("%d", SysTick->LOAD, "");
#endif /* DEBUG_SYSCLOCK */

	/* 设置优先级 ********************/
	NVIC_SetPriority(SysTick_IRQn, 0); // 最高优先级 		/* set Priority for Cortex-M0 System Interrupts */

	SysTick->VAL = 0; /* 计数器值清0*/ /* Load the SysTick Counter Value */

	__TickInc_us = (uint32_t)1000000 / __TickFreq; // 计算计数周期us的递增值
#if DEBUG_SYSCLOCK
	Em_printf_var("%d", __TickInc_us, "");
#endif /* DEBUG_SYSCLOCK */

	// 也就是每次递增值大于等于1 ms
	if (__TickInc_us < 1000)
		__TickInc_ms = 1;
	else
		__TickInc_ms = (uint32_t)1000 / __TickFreq;
#if DEBUG_SYSCLOCK
	Em_printf_var("%d", __TickInc_ms, "");
#endif /* DEBUG_SYSCLOCK */

	// __Tick_us = 0;
	__Tick_ms = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk | /* 开启Tick中断 */
					SysTick_CTRL_ENABLE_Msk;   /* Enable SysTick IRQ and SysTick Timer */
}

/**
 * @brief
 *
 * @note SysTick_Handler调用
 */
void SysTick_IncTick(void)
{
	static uint32_t IsIncMs = 0; // 虽然叫这个名字，但是这不一定是1ms，是设置递增的那个值，也许是10us，也许是500 us...
	// static uint32_t Isprintf = 0;

	IsIncMs++;							  // 递增，用于判断是否需要增加
	if (IsIncMs >= (1000 / __TickInc_us)) // 注意，这个应该看看对不对
	{
		IsIncMs = 0;
		__Tick_ms = __Tick_ms + __TickInc_ms;
	}
	__Tick_us = __Tick_us + __TickInc_us;

	// // 秒测试，注意这是在中断里面
	// if(++Isprintf >= ((1000 / __TickInc_us) * 1000))
	// {
	//     Isprintf = 0;
	//     Em_printf_var("%lld", __Tick_us, "");
	//     Em_printf_var("%lld", __Tick_ms, "");
	//     printf_Em("\r\n");
	// }
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 *
 * @attention 一般情况下，请在stm32f0xx_it.c中实现此函数
 * 	stm32f0xx_it.c中定义了该函数但是没有填写相关内容，此处弱函数关键字会让此函数失效，导致延时函数无法运行。
 * 	将下面函数在stm32f0xx_it.c中实现即可
 */
KEYWORD_WEAK void SysTick_Handler(void)
{
	SysTick_IncTick();
}

/** 时间查询函数
 * @{
 */

/**
 * @brief Get the Tick object
 *
 * @return uint64_t
 * @note 此函数默认返回ms时刻
 */
uint64_t GetTick(void)
{
	return __Tick_ms;
}

/**
 * @brief us查询
 *
 * @return uint64_t
 */
uint64_t GetTick_us(void) { return __Tick_us; }

/**
 * @brief ms查询，功能等同于默认情况下的 GetTick()
 *
 * @return uint64_t
 */
uint64_t GetTick_ms(void) { return __Tick_ms; }
/** 时间查询函数 end
 * @}
 */

/**
 * @brief 毫秒延时函数
 *
 * @param ms
 */
void delay_ms(uint64_t ms)
{
	uint64_t tickstart = GetTick_ms();
	__IO uint64_t ticknow = 0;

	do
	{
		ticknow = GetTick_ms();
	} while (((ticknow - tickstart) < ms) && (ticknow >= tickstart)); // 时间没达到而且现在时间比之前大。小的话可能变量溢出或者被清零了
																	  //   后面的必须加上等号，CPU速度过快，有可能时间还没来得及改变
}

/**
 * @brief 微秒延时函数
 *
 * @param us
 *
 * @attention 根据您设置的溢出频率，可能不能做到1 us这类型的超小延时，us级延时可以尝试使用软件延时，或者定时器的阻塞式延时
 */
void delay_us(uint64_t us)
{
	uint64_t tickstart = GetTick_us();
	__IO uint64_t ticknow = 0;

	do
	{
		ticknow = GetTick_us();
	} while (((ticknow - tickstart) < us) && (ticknow >= tickstart)); // 时间没达到而且现在时间比之前大。小的话可能变量溢出或者被清零了
																	  //   后面的必须加上等号，CPU速度过快，有可能时间还没来得及改变
}

/**
 * @brief 硬件定时器进行us延时
 *
 * @param us
 */
// /*inline*/ void delay_us_TIM(uint64_t us)
// {
// 	uint64_t tickstart = GetTick_us_TIM();
// 	__IO uint64_t ticknow = 0;

// 	do
// 	{
// 		ticknow = GetTick_us_TIM();
// 	} while (((ticknow - tickstart) < us) && (ticknow >= tickstart)); // 时间没达到而且现在时间比之前大。小的话可能变量溢出或者被清零了
// 																	  //   后面的必须加上等号，有可能时间还没来得及改变
// }

/**
 * @brief 用于软件微秒延时的常数
 * 不是比例函数，以为函数调用，可能是一次函数啥的函数
 */
// double Constant_delay_us_Soft = 5.15;	//
uint64_t Constant_delay_us_Soft = 3; //

/**
 * @brief
 *
 * @param us
 *
 * @note 当前在10 us延时在10.51us左右（包含函数调用时间）
 *        1us 在 1.648 us左右（有函数调用时间，不准确，严重不准确）
 * @attention 此函数只在F0 48MHz下测试了
 */
void delay_us_Soft(uint64_t us)
{

	//   static int64_t i = 0;
	//   i = (int64_t)(Constant_delay_us_Soft * us);

	if (us == 0) // 这个并不影响太多时间
		return;

	//   while(--us)
	for (; us; --us)
	{
		++us;
		--us;
		++us;
		--us;
		++us;
		--us;
		++us;
		--us;
		++us;
		--us;
		++us;
		--us;
	}
}

void Test_delay_us_Soft(void)
{
	Em_INFOR("准备测试 , 当前tick = %lld ms", GetTick());
	uint64_t StartTick = GetTick();

	for (uint32_t i = 0; i < 5000 * 100; i++)
	{
		// ONEWIRE_DELAY(10);
	}

	uint64_t EndTick = GetTick();
	Em_INFOR("运行完成, 共花费%lld ms [%lld ms -> %lld ms]",
			 EndTick - StartTick, StartTick, EndTick);
}

/* Systick end ****************************************************************/

/* 串口初始化函数 *******************************************************************************************************/
/**
 * @brief
 *
 * @param bound 波特率;
 * 	@arg 0, 表示使用上一次的值; 如果第一次就使用0, 则使用默认的115200
 */
KEYWORD_WEAK void USART1_Init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitTypeStrue;
	USART_InitTypeDef USART_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;

	static uint32_t USART1_bound = 115200;
	if (bound != 0)
	{
		USART1_bound = bound;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// PA9-TX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_AF_PP; // 推推挽复用输出
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitTypeStrue);

	// PA10-RX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitTypeStrue);

	// USART_Init()
	USART_InitStrue.USART_BaudRate = USART1_bound;								// 波特率
	USART_InitStrue.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流控制->不使用
	USART_InitStrue.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 发送和接受都使能
	USART_InitStrue.USART_Parity = USART_Parity_No;								// 不用奇偶校验
	USART_InitStrue.USART_StopBits = USART_StopBits_1;							// 1位停止位
	USART_InitStrue.USART_WordLength = USART_WordLength_8b;						// 8位字长
	USART_Init(USART1, &USART_InitStrue);										// 串口初始化

	// 使用中断的话还得配置中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 接受缓冲区非空进入中断

	// NVIC_Init()函数的参数
	NVIC_InitStrue.NVIC_IRQChannel = USART1_IRQn;		  // 选择中断通道----通道在顶层头文件stm32f10x.h中，找IRQ的 317行
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 3; // 设置抢占优先级
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 0;		  // 设置响应优先级
	NVIC_InitStrue.NVIC_IRQChannelCmd = DISABLE;		  // IRQ通道使能
	NVIC_Init(&NVIC_InitStrue);

	USART_Cmd(USART1, ENABLE); // 串口函数使能
}

/**
 * @brief
 *
 * @param bound 波特率;
 * 	@arg 0, 表示使用上一次的值; 如果第一次就使用0, 则使用默认的115200
 */
KEYWORD_WEAK void USART2_Init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitTypeStrue;
	USART_InitTypeDef USART_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;

	static uint32_t USART2_bound = 115200;
	if (bound != 0)
	{
		USART2_bound = bound;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// PA2-TX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_AF_PP; // 推推挽复用输出
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitTypeStrue);

	// PA3-RX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitTypeStrue);

	// USART_Init()
	USART_InitStrue.USART_BaudRate = USART2_bound;								// 波特率
	USART_InitStrue.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流控制->不使用
	USART_InitStrue.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 发送和接受都使能
	USART_InitStrue.USART_Parity = USART_Parity_No;								// 不用奇偶校验
	USART_InitStrue.USART_StopBits = USART_StopBits_1;							// 1位停止位
	USART_InitStrue.USART_WordLength = USART_WordLength_8b;						// 8位字长
	USART_Init(USART2, &USART_InitStrue);										// 串口初始化

	// 使用中断的话还得配置中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 接受缓冲区非空进入中断

	// NVIC_Init()函数的参数
	NVIC_InitStrue.NVIC_IRQChannel = USART2_IRQn;		  // 选择中断通道----通道在顶层头文件stm32f10x.h中，找IRQ的 317行
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 3; // 设置抢占优先级
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 0;		  // 设置响应优先级
	NVIC_InitStrue.NVIC_IRQChannelCmd = DISABLE;		  // IRQ通道使能
	NVIC_Init(&NVIC_InitStrue);

	USART_Cmd(USART2, ENABLE); // 串口函数使能
}

/**
 * @brief
 *
 * @param bound 波特率;
 * 	@arg 0, 表示使用上一次的值; 如果第一次就使用0, 则使用默认的115200
 */
KEYWORD_WEAK void USART3_Init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitTypeStrue;
	USART_InitTypeDef USART_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;

	static uint32_t USART3_bound = 115200;
	if (bound != 0)
	{
		USART3_bound = bound;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	// PB10-TX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_AF_PP; // 推推挽复用输出
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitTypeStrue);

	// PB11-RX
	GPIO_InitTypeStrue.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_InitTypeStrue.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitTypeStrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitTypeStrue);

	// USART_Init()
	USART_InitStrue.USART_BaudRate = USART3_bound;								// 波特率
	USART_InitStrue.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流控制->不使用
	USART_InitStrue.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 发送和接受都使能
	USART_InitStrue.USART_Parity = USART_Parity_No;								// 不用奇偶校验
	USART_InitStrue.USART_StopBits = USART_StopBits_1;							// 1位停止位
	USART_InitStrue.USART_WordLength = USART_WordLength_8b;						// 8位字长
	USART_Init(USART3, &USART_InitStrue);										// 串口初始化

	// 使用中断的话还得配置中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 接受缓冲区非空进入中断

	// NVIC_Init()函数的参数
	NVIC_InitStrue.NVIC_IRQChannel = USART3_IRQn;		  // 选择中断通道----通道在顶层头文件stm32f10x.h中，找IRQ的 317行
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 3; // 设置抢占优先级
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 0;		  // 设置响应优先级
	NVIC_InitStrue.NVIC_IRQChannelCmd = DISABLE;		  // IRQ通道使能
	NVIC_Init(&NVIC_InitStrue);

	USART_Cmd(USART3, ENABLE); // 串口函数使能
}

/* 串口初始化函数 end ***************************************************************************************************/
