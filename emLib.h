/**
 * @file emLib.h
 * @author Peng (Peng2333@vip.qq.com.com)
 * @authors Peng emmitt (emmitt2333@qq.com)
 *
 * @brief 用于新工程快速实现打印、Systick延时函数、字符串操作等简单函数，直接加入本工程即可
 * @version 0.3
 * @date 2020-09-01
 *
 * @encoding GB2312
 *
 * @copyright Copyright (c) 2020
 *
 */

/**
 * @modified
 * 	2023年9月5日：新增打印变量函数：Em_printf_var
 *
 */

#ifndef __EMLIB_H
#define __EMLIB_H

// 通过此头文件，引用硬件初始化头文件
#include "main.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

/* 设置区域 ************************************************************************************************************/
/**
 * @brief 设置本文件输出的串口
 *
 */
#define USART_DEBUG_PROT USART2
#define USART_DEBUG_PROT_InitFunc USART2_Init

/******************************************************************************/
/**
 * @brief 设置打印函数中文件名显示方式
 *
 * @param 1 使用C标准的__FILE__，返回完整文件路径加文件名
 * @param 0 使用C标准的__FILE__裁剪后的字符串，返回文件本名，删掉之前的路径
 *
 */
#if 0
#define __PRINTFFILE__ __FILE__
#else
#define __PRINTFFILE__ __FILENAME__
#endif

/******************************** 等级调试打印 *********************************/
/**
 * @brief 调试打印等级设置
 * @param
 * 	@arg -1:	全都不打印
 * 	@arg 0 :	仅打印ERROR
 * 	@arg 1 :	打印Warning及以上
 * 	@arg 2 :	打印输出状态消息及以上 当前值或以下值也决定printf_Em
 * 	@arg 3 :	打印Debug信息及以上
 * 	@arg 4 :
 *
 */
#define DEBUG_LEVEL 3

/******************************************************************************/
/**
 * @brief 选择需要使用的弱函数关键字的样式
 *
 */
#if (defined __WEAK)
#define KEYWORD_WEAK __WEAK
#elif (defined __weak)
#define KEYWORD_WEAK __weak
#elif 1 /* 这个作为兜底，应该可以的 */
#define KEYWORD_WEAK __attribute__((weak))
#else
#error "当前编译环境未找到相关情况弱函数定义"
#endif

/******************************************************************************/
/**
 * @brief 调试时钟的时候可以打开，可以打印调整前后的时钟频率
 *
 */
#ifndef DEBUG_SYSCLOCK
#define DEBUG_SYSCLOCK 0
#endif

/* 设置区域 end ********************************************************************************************************/

/**
 * @brief 将完整文件路径剪切为只剩下文件本名
 * @param 1 开启路径裁切，只留下文件名 （win系统的文件路径，使用反斜杠'\'作为分割）
 * @param 0 继续使用编译器输出路径"__FILE__"
 *
 * @attention 这个一般不改，上面有另一个宏定义了__PRINTFFILE__
 */
#if 1
/* 文件本名 */
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
/* 文件路径 */
#define __FILENAME__ __FILE__
#endif

#define Pi 3.1415926535897933f // 定义圆周率，下面那两个角度转换函数使用
// printf("π = %f\r\n", acos(-1) * 1000000000000000000);							//开始计算π用的

/**
 * @brief 简单实现一点最大最小值等函数等
 *
 */
#define min(__a, __b) ((__a) < (__b) ? (__a) : (__b)) /* 求最小值*/
#define max(__a, __b) ((__a) > (__b) ? (__a) : (__b)) /* 求最大值*/
#define Avg(__a, __b) (((__a) + (__b)) / 2.0)		  /* 求平均值*/
#define Abs(__x) ((__x) >= 0 ? (__x) : (-(__x)))	  /* 求绝对值 注意名字不能是abs*/

/**
 * @brief 消除未使用的变量在编译器中的警告
 *
 * @note 在变量前加(void)表示变量没有用
 */
#define UNUSED(x) ((void)(x)))

/**********************************************************************************************************************/
/*************************************************** 等级调试打印 ******************************************************/

/**
 * @brief  通过宏定义正确定位到出错那一行。
 * @param  ...
 * @retval None
 * @{
 */

/**
 * @brief
 * @param 1
 * @param 0
 */
#if DEBUG_LEVEL >= 2 /* 暂时定义在INFOR级别 */
#if 1
/**
 * @brief 使用这个函数，同样不打印多余的东西，功能相似于printf，但是可以用打印级别控制，统一开关
 * @param ... 和printf一样
 *
 * @note 把"_Em"放在后面，是为了打函数自动提示的时候方便找到
 */
#define printf_Em(...) printf(__VA_ARGS__)
#else
// 相比函数USART_printf只是函数的第一个参数固定下来了
#define printf_Em(...) USART_printf(USART_DEBUG_PROT, __VA_ARGS__)
#endif
#else
#define printf_Em(...) (UNUSED(__VA_ARGS__))
#endif /* DEBUG_LEVEL >= x */

/*__FUNCTION__和__func__均能使用*/
#if DEBUG_LEVEL >= 0
// 错误输出

// 是否需要ERROR进行卡住程序
#if 0
#define Em_ERROR(...)                                                                                         \
	do                                                                                                        \
	{                                                                                                         \
		printf("\r\nERROR -> %s:(%d):%s():\t", (uint8_t *)__PRINTFFILE__, __LINE__, (uint8_t *)__FUNCTION__); \
		printf(__VA_ARGS__);                                                                                  \
		printf("\r\n\r\n\r\n\r\nERROR->%s(%d):已卡死\r\n", (uint8_t *)__PRINTFFILE__, __LINE__);              \
		/* 此处通过通信向其他地方汇报 */                                                         \
		while (1)                                                                                             \
			/*if (0) */                                                                                       \
			/*break*/;                                                                                        \
	} while (0)
#else
#define Em_ERROR(...)                                                                                        \
	do                                                                                                       \
	{                                                                                                        \
		printf("\r\nERROR -> %s(%d):%s():\t", (uint8_t *)__PRINTFFILE__, __LINE__, (uint8_t *)__FUNCTION__); \
		printf(__VA_ARGS__);                                                                                 \
		printf("\r\nERROR -> %s(%d):!!!!!!!!!!!!!!!!!!!!!!\r\n", (uint8_t *)__PRINTFFILE__, __LINE__);       \
		/* 此处通过通信向其他地方汇报 */                                                        \
	} while (0)
#endif
#else
#define Em_ERROR(...) ((void)0)
#endif

#if DEBUG_LEVEL >= 1
// 警告输出
#define Em_Warning(format, ...)                                                                \
	do                                                                                         \
	{                                                                                          \
		printf("W %s:(%d):%s():\t", (uint8_t *)__PRINTFFILE__, __LINE__, (uint8_t *)__func__); \
		printf(format "\r\n", ##__VA_ARGS__);                                                  \
	} while (0)
#else
#define Em_Warning(format, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= 2
// 消息输出(information)
#define Em_INFOR(format, ...)                                         \
	do                                                                \
	{                                                                 \
		printf("I %s()-(%d行): ", (uint8_t *)__FUNCTION__, __LINE__); \
		printf(format "\r\n", ##__VA_ARGS__);                         \
	} while (0)
#else
#define Em_INFOR(format, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= 3
// 调试信息输出
#define Em_Debug(format, ...)                                                                                    \
	do                                                                                                           \
	{                                                                                                            \
		printf(/*"Debug-> */ "D %s:(%d):%s():\t", (uint8_t *)__PRINTFFILE__, __LINE__, (uint8_t *)__FUNCTION__); \
		printf(format "\r\n", ##__VA_ARGS__);                                                                    \
	} while (0)
#else
#define Em_Debug(format, ...) ((void)0)
#endif

/**************************************************** end *************************************************************/

/**
 * @brief 一个打印变量的函数，同时会打印变量名字变量值。 目前这里使用Peng_INFOR函数，会受到打印等级的控制
 *
 * @param type 输入打印变量的类型，如%d, %f等。需要写:"%d"; 当然这里也可以写一大串字符（按照C字符串里面的写法就行，）然后其中包含%d or %f ...就行
 * @param x 输入一个变量，函数将会取变量的值和写的这个字符串
 * @param ... 打印前缀, 支持格式化字符串；本来想把这个写在前面的，但是"..."只能写在函数最后面
 *
 * @note 对于type参数：如果是使用"#"连接对"type"进行引用，直接写:%d;(或者其他)就可，唯一问题是使用编辑器代码格式化的时候会将调用
 * 	位置的"%"与"d"(或者"f"...)分开, 变成"% d"，还需要手动调整回去; 当然这里如果使用拼接引用，写法也和上面一样，随便写，只是少了引号。
 */
#define Em_printf_var(type, x, format, ...)               \
	do                                                    \
	{                                                     \
		Em_INFOR(format #x " = " type, ##__VA_ARGS__, x); \
		/* (format #x " = " #type, ##__VA_ARGS__, x) */   \
	} while (0)

// extern char Printf_buff[2048];

/**
 *************************** Assert Selection **********************************
 * @brief 控制是否使能 Full Assert
 *        Uncomment the line below to expanse the "assert_param" macro in the HAL drivers code
 *
 * @attention
 *		使能HAL/Std库里面的参数校验，一般不在这里打开
 *		需要在stm32xxxx(_hal)_conf.h里面打开
 *		如果是使用CubeMX生成的工程，请在"Project Manager" -> "code Generator" -> "HAL Setting" 中勾选 "Enable Full Assert" 即可。
 * @{
 */

/*	#define USE_FULL_ASSERT    1U */

/** @}
 */

#ifdef USE_FULL_ASSERT
void __assert_failed(uint8_t *file, uint32_t line);
#endif

int Em_sprintf(char *buf, const char *format, ...);
int USART_printf(USART_TypeDef *USARTx, char *format, ...);

double AngleToRadian(double _Angle);
double RadianToAngle(double _Radian);

int Addstr(char *dest, const char *format, ...);
char *lookupstr(const char *source, char *str1);
char *lookuplast(const char *source, char *str1, char *TargetArray);
char *lookupBetween(const char *source, char *str1, char *str2, char *TargetArray);

int Em_isdigit(int c);
long long int pow_int(int _X, int _Y);
double Em_atof(const char *str);
double Em_atof_Lite(const char *str);
void *Limit(double *Limit_Data, double Limit_);

uint32_t read_addr(uint32_t addr);
void write_addr(uint32_t addr, uint32_t val);

/** @brief 整形转二进制字符串的函数
 *	@{
 */

typedef struct
{
	int32_t Value_int32_t;
	char Result_String[40]; // 存储生成的32位字符，四个中间空格，两位头"0b"，一位结束位，共计39位。设置40，可以让0b后面填充有个空格(第三位位置)
} int32_to_Binary_TypeDef;

char *int32_to_Binary(int32_to_Binary_TypeDef *Dataptr);

/**
 * @brief 使用SysTick的延时函数
 *
 */
extern __IO uint64_t __Tick_us_TIM; // 定时器计数使用的计数微秒值

void em_delay_init(void);
void SysTick_IncTick(void);
// 为了保证速度，1 us递增值暂时放在宏定义里
// #define TIM_IncTick()                      \
// 	{                                      \
// 		__Tick_us_TIM = __Tick_us_TIM + 1; \
// 	}

uint64_t GetTick(void);
#define GetTick_us_TIM() ((uint64_t)__Tick_us_TIM)
uint64_t GetTick_us(void);
uint64_t GetTick_ms(void);
void delay_ms(uint64_t ms);
void delay_us(uint64_t us);
void delay_us_TIM(uint64_t us);
void delay_us_Soft(uint64_t us);
void Test_delay_us_Soft(void);

void USART1_Init(uint32_t bound);
void USART2_Init(uint32_t bound);
void USART3_Init(uint32_t bound);

/** @}
 */

#endif /* #ifndef __EMLIB_H */
