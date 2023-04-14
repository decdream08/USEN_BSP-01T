
#ifndef __COMMON_H__
#define __COMMON_H__

/* TYPE DEFINE */
#if (0)
typedef unsigned long	uint32_t;
typedef unsigned int	uint16_t;
typedef unsigned char	uint8_t;
typedef signed long 	s32;
typedef signed int		int16_t;
typedef signed char 	s8;
#endif

#define BIT(n)	((uint32_t)1 << (n))
#define ABS(x)	(x<0)?-x:x

#define DI()	do{IE &= ~0x80;}while(0)
#define EI()	do{IE |=  0x80;}while(0)

typedef struct{
		uint8_t reinit	:1;
		uint8_t cal_ok	:1; 
		uint8_t led_end	:1;
		uint8_t ts_end	:1;
		uint8_t led_en	:1;
		uint8_t dbg_en	:1;
		uint8_t led_io	:1; 
		uint8_t reserv	:1;
}str_flag; 



extern str_flag t_sysbit;


#endif // __COMMON_H__

