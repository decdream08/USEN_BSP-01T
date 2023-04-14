;/*****************************************************************************
; * @file:    startup_**_CM0.s
; * @purpose: CMSIS Cortex-M0 Core Device Startup File 
; *           for the ARM 'Microcontroller Prototyping System' 
; * @version: V1.01
; * @date:    19. Aug. 2009
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * Copyright (C) 2008-2009 ARM Limited. All rights reserved.
; * ARM Limited (ARM) is supplying this software for use with Cortex-M3 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; *****************************************************************************/


; <h> Stack Configuration
; <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size


        DATA
__vector_table
                DCD     sfe(CSTACK)     						; Top of Stack
                DCD     Reset_Handler             	; Reset Handler
                DCD     NMI_Handler              		; NMI Handler
                DCD     HardFault_Handler         	; Hard Fault Handler
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     SVC_Handler               	; SVCall Handler
                DCD     0                         	; Reserved
                DCD     0                         	; Reserved
                DCD     PendSV_Handler           	; PendSV Handler
                DCD     SysTick_Handler           	; SysTick Handler

                ; Device Interrupts
                DCD     LVI_IRQHandler                        	; IRQ0
                DCD     SYSCLKFAIL_IRQHandler                   ; IRQ1
                DCD     WDT_IRQHandler                     	; IRQ2
                DCD     GPIOAB_IRQHandler                	; IRQ3
                DCD     GPIOCD_IRQHandler	                ; IRQ4
                DCD     GPIOE_IRQHandler                  	; IRQ5
                DCD     GPIOF_IRQHandler                        ; IRQ6
                DCD     TIMER10_IRQHandler                      ; IRQ7
                DCD     TIMER11_IRQHandler                      ; IRQ8
                DCD     TIMER12_IRQHandler                      ; IRQ9
                DCD     I2C0_IRQHandler                         ; IRQ10
                DCD     USART10_IRQHandler                      ; IRQ11
                DCD     WT_IRQHandler                           ; IRQ12
                DCD     TIMER30_IRQHandler                      ; IRQ13
                DCD     I2C1_IRQHandler                          ; IRQ14
                DCD     TIMER20_IRQHandler                      ; IRQ15
                DCD     TIMER21_IRQHandler                      ; IRQ16
                DCD     USART11_IRQHandler                      ; IRQ17
                DCD     ADC_IRQHandler                          ; IRQ18
                DCD     UART0_IRQHandler                        ; IRQ19
                DCD     UART1_IRQHandler                         ; IRQ20
                DCD     TIMER13_IRQHandler                      ; IRQ21
		DCD     NULL_IRQHandler                         ; IRQ22
                DCD     NULL_IRQHandler                         ; IRQ23
                DCD     NULL_IRQHandler                         ; IRQ24
                DCD     SPI20_IRQHandler                        ; IRQ25
                DCD     SPI21_IRQHandler        		; IRQ26
                DCD     NULL_IRQHandler                         ; IRQ27 DAC
                DCD     LED_IRQHandler                          ; IRQ28
                DCD     TOUCH_IRQHandler                        ; IRQ29
		DCD     NULL_IRQHandler                         ; IRQ30
                DCD     CRC_IRQHandler                          ; IRQ31
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size 	EQU 	__Vectors_End - __Vectors     
           
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        PUBWEAK LVI_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
LVI_IRQHandler
        B LVI_IRQHandler
        

        PUBWEAK SYSCLKFAIL_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SYSCLKFAIL_IRQHandler
        B SYSCLKFAIL_IRQHandler

        PUBWEAK WDT_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
WDT_IRQHandler
        B WDT_IRQHandler

        PUBWEAK GPIOAB_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPIOAB_IRQHandler
        B GPIOAB_IRQHandler

        PUBWEAK GPIOCD_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPIOCD_IRQHandler
        B GPIOCD_IRQHandler

        PUBWEAK GPIOE_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPIOE_IRQHandler
        B GPIOE_IRQHandler
        
        PUBWEAK GPIOF_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPIOF_IRQHandler
        B GPIOF_IRQHandler        

        PUBWEAK TIMER10_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER10_IRQHandler
        B TIMER10_IRQHandler

        PUBWEAK TIMER11_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER11_IRQHandler
        B TIMER11_IRQHandler

        PUBWEAK TIMER12_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER12_IRQHandler
        B TIMER12_IRQHandler

        PUBWEAK I2C0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
I2C0_IRQHandler
        B I2C0_IRQHandler

        PUBWEAK USART10_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USART10_IRQHandler
        B USART10_IRQHandler

        PUBWEAK WT_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
WT_IRQHandler
        B WT_IRQHandler

        PUBWEAK TIMER30_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER30_IRQHandler
        B TIMER30_IRQHandler

        PUBWEAK I2C1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
I2C1_IRQHandler
        B I2C1_IRQHandler

        PUBWEAK TIMER20_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER20_IRQHandler
        B TIMER20_IRQHandler

        PUBWEAK TIMER21_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER21_IRQHandler
        B TIMER21_IRQHandler

        PUBWEAK USART11_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USART11_IRQHandler
        B USART11_IRQHandler

        PUBWEAK ADC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ADC_IRQHandler
        B ADC_IRQHandler

        PUBWEAK UART0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
UART0_IRQHandler
        B UART0_IRQHandler

        PUBWEAK UART1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
UART1_IRQHandler
        B UART1_IRQHandler
        
        
        PUBWEAK TIMER13_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER13_IRQHandler
        B TIMER13_IRQHandler
        

        PUBWEAK SPI20_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SPI20_IRQHandler
        B SPI20_IRQHandler
        
        PUBWEAK SPI21_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SPI21_IRQHandler
        B SPI21_IRQHandler
        
        PUBWEAK LED_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
LED_IRQHandler
        B LED_IRQHandler
        
        PUBWEAK TOUCH_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TOUCH_IRQHandler
        B TOUCH_IRQHandler
        
        PUBWEAK CRC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CRC_IRQHandler
        B CRC_IRQHandler    
        
        PUBWEAK NULL_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
NULL_IRQHandler  
        B NULL_IRQHandler
        
        END

