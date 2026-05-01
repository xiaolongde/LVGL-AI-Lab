/* startup_gd32f303rc.S — GCC 启动汇编 for GD32F303RCT6 (256K Flash, 48K SRAM)
 *
 * 翻译自 KIT/.../ARM/startup_gd32f30x_hd.s（Keil ARM 语法）。
 * 向量表对应 GD32F30x HD 系列（60 个外设 IRQ + 16 个 ARM exception）。
 *
 * Reset 流程：copy data 段 from Flash → SRAM；零 bss；调 SystemInit (PLL 配 120MHz)；
 *           调 __libc_init_array (newlib C++ ctors / 静态初始化)；调 main；死循环。
 */

    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

/* ------------------------------------------------------------------ */
/* Vector Table                                                       */
/* ------------------------------------------------------------------ */
    .section .isr_vector, "a", %progbits
    .type    g_pfnVectors, %object
    .size    g_pfnVectors, .-g_pfnVectors

    .global g_pfnVectors

g_pfnVectors:
    .word _estack                       /* Top of Stack */
    .word Reset_Handler                 /* Reset Handler */
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* External Interrupts (peripheral IRQs) */
    .word WWDGT_IRQHandler              /* 16: Window Watchdog Timer */
    .word LVD_IRQHandler                /* 17: LVD through EXTI Line */
    .word TAMPER_IRQHandler             /* 18: Tamper through EXTI Line */
    .word RTC_IRQHandler                /* 19: RTC through EXTI Line */
    .word FMC_IRQHandler                /* 20: FMC */
    .word RCU_CTC_IRQHandler            /* 21: RCU and CTC */
    .word EXTI0_IRQHandler              /* 22: EXTI Line 0 */
    .word EXTI1_IRQHandler              /* 23: EXTI Line 1 */
    .word EXTI2_IRQHandler              /* 24: EXTI Line 2 */
    .word EXTI3_IRQHandler              /* 25: EXTI Line 3 */
    .word EXTI4_IRQHandler              /* 26: EXTI Line 4 */
    .word DMA0_Channel0_IRQHandler      /* 27 */
    .word DMA0_Channel1_IRQHandler      /* 28 */
    .word DMA0_Channel2_IRQHandler      /* 29 */
    .word DMA0_Channel3_IRQHandler      /* 30 */
    .word DMA0_Channel4_IRQHandler      /* 31 */
    .word DMA0_Channel5_IRQHandler      /* 32 */
    .word DMA0_Channel6_IRQHandler      /* 33 */
    .word ADC0_1_IRQHandler             /* 34: ADC0 + ADC1 */
    .word USBD_HP_CAN0_TX_IRQHandler    /* 35 */
    .word USBD_LP_CAN0_RX0_IRQHandler   /* 36 */
    .word CAN0_RX1_IRQHandler           /* 37 */
    .word CAN0_EWMC_IRQHandler          /* 38 */
    .word EXTI5_9_IRQHandler            /* 39 */
    .word TIMER0_BRK_IRQHandler         /* 40 */
    .word TIMER0_UP_IRQHandler          /* 41 */
    .word TIMER0_TRG_CMT_IRQHandler     /* 42 */
    .word TIMER0_Channel_IRQHandler     /* 43 */
    .word TIMER1_IRQHandler             /* 44 */
    .word TIMER2_IRQHandler             /* 45 */
    .word TIMER3_IRQHandler             /* 46 */
    .word I2C0_EV_IRQHandler            /* 47 */
    .word I2C0_ER_IRQHandler            /* 48 */
    .word I2C1_EV_IRQHandler            /* 49 */
    .word I2C1_ER_IRQHandler            /* 50 */
    .word SPI0_IRQHandler               /* 51 */
    .word SPI1_IRQHandler               /* 52 */
    .word USART0_IRQHandler             /* 53 */
    .word USART1_IRQHandler             /* 54 */
    .word USART2_IRQHandler             /* 55 */
    .word EXTI10_15_IRQHandler          /* 56 */
    .word RTC_Alarm_IRQHandler          /* 57 */
    .word USBD_WKUP_IRQHandler          /* 58 */
    .word TIMER7_BRK_IRQHandler         /* 59 */
    .word TIMER7_UP_IRQHandler          /* 60 */
    .word TIMER7_TRG_CMT_IRQHandler     /* 61 */
    .word TIMER7_Channel_IRQHandler     /* 62 */
    .word ADC2_IRQHandler               /* 63 */
    .word EXMC_IRQHandler               /* 64 */
    .word SDIO_IRQHandler               /* 65 */
    .word TIMER4_IRQHandler             /* 66 */
    .word SPI2_IRQHandler               /* 67 */
    .word UART3_IRQHandler              /* 68 */
    .word UART4_IRQHandler              /* 69 */
    .word TIMER5_IRQHandler             /* 70 */
    .word TIMER6_IRQHandler             /* 71 */
    .word DMA1_Channel0_IRQHandler      /* 72 */
    .word DMA1_Channel1_IRQHandler      /* 73 */
    .word DMA1_Channel2_IRQHandler      /* 74 */
    .word DMA1_Channel3_4_IRQHandler    /* 75: DMA1 Channel3 + Channel4 */

/* ------------------------------------------------------------------ */
/* Reset_Handler                                                      */
/* ------------------------------------------------------------------ */
    .section .text.Reset_Handler
    .weak    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
    ldr   sp, =_estack         /* Set stack pointer */

    /* Copy .data from Flash (LMA) to SRAM (VMA) */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata
    movs  r3, #0
    b     .Lcopy_check
.Lcopy_loop:
    ldr   r4, [r2, r3]
    str   r4, [r0, r3]
    adds  r3, r3, #4
.Lcopy_check:
    adds  r4, r0, r3
    cmp   r4, r1
    bcc   .Lcopy_loop

    /* Zero .bss */
    ldr   r2, =_sbss
    ldr   r4, =_ebss
    movs  r3, #0
    b     .Lzero_check
.Lzero_loop:
    str   r3, [r2]
    adds  r2, r2, #4
.Lzero_check:
    cmp   r2, r4
    bcc   .Lzero_loop

    /* Call SystemInit (configures clock tree to 120 MHz from 8 MHz HXTAL) */
    bl    SystemInit

    /* Call C library init (newlib: __libc_init_array runs static ctors / init_array) */
    bl    __libc_init_array

    /* Call main */
    bl    main

    /* main returned — should not happen, but fall to infinite loop */
.Lmain_returned:
    b     .Lmain_returned

    .size Reset_Handler, .-Reset_Handler

/* ------------------------------------------------------------------ */
/* Default_Handler — infinite loop for any unhandled IRQ              */
/* ------------------------------------------------------------------ */
    .section .text.Default_Handler, "ax", %progbits
    .type    Default_Handler, %function
Default_Handler:
    b     .
    .size Default_Handler, .-Default_Handler

/* ------------------------------------------------------------------ */
/* Weak alias all handlers to Default_Handler                         */
/* ------------------------------------------------------------------ */

    .macro def_irq_handler handler_name
    .weak \handler_name
    .thumb_set \handler_name, Default_Handler
    .endm

    /* ARM exceptions */
    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler MemManage_Handler
    def_irq_handler BusFault_Handler
    def_irq_handler UsageFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler DebugMon_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler

    /* GD32F30x peripheral IRQs (60 entries, 16..75) */
    def_irq_handler WWDGT_IRQHandler
    def_irq_handler LVD_IRQHandler
    def_irq_handler TAMPER_IRQHandler
    def_irq_handler RTC_IRQHandler
    def_irq_handler FMC_IRQHandler
    def_irq_handler RCU_CTC_IRQHandler
    def_irq_handler EXTI0_IRQHandler
    def_irq_handler EXTI1_IRQHandler
    def_irq_handler EXTI2_IRQHandler
    def_irq_handler EXTI3_IRQHandler
    def_irq_handler EXTI4_IRQHandler
    def_irq_handler DMA0_Channel0_IRQHandler
    def_irq_handler DMA0_Channel1_IRQHandler
    def_irq_handler DMA0_Channel2_IRQHandler
    def_irq_handler DMA0_Channel3_IRQHandler
    def_irq_handler DMA0_Channel4_IRQHandler
    def_irq_handler DMA0_Channel5_IRQHandler
    def_irq_handler DMA0_Channel6_IRQHandler
    def_irq_handler ADC0_1_IRQHandler
    def_irq_handler USBD_HP_CAN0_TX_IRQHandler
    def_irq_handler USBD_LP_CAN0_RX0_IRQHandler
    def_irq_handler CAN0_RX1_IRQHandler
    def_irq_handler CAN0_EWMC_IRQHandler
    def_irq_handler EXTI5_9_IRQHandler
    def_irq_handler TIMER0_BRK_IRQHandler
    def_irq_handler TIMER0_UP_IRQHandler
    def_irq_handler TIMER0_TRG_CMT_IRQHandler
    def_irq_handler TIMER0_Channel_IRQHandler
    def_irq_handler TIMER1_IRQHandler
    def_irq_handler TIMER2_IRQHandler
    def_irq_handler TIMER3_IRQHandler
    def_irq_handler I2C0_EV_IRQHandler
    def_irq_handler I2C0_ER_IRQHandler
    def_irq_handler I2C1_EV_IRQHandler
    def_irq_handler I2C1_ER_IRQHandler
    def_irq_handler SPI0_IRQHandler
    def_irq_handler SPI1_IRQHandler
    def_irq_handler USART0_IRQHandler
    def_irq_handler USART1_IRQHandler
    def_irq_handler USART2_IRQHandler
    def_irq_handler EXTI10_15_IRQHandler
    def_irq_handler RTC_Alarm_IRQHandler
    def_irq_handler USBD_WKUP_IRQHandler
    def_irq_handler TIMER7_BRK_IRQHandler
    def_irq_handler TIMER7_UP_IRQHandler
    def_irq_handler TIMER7_TRG_CMT_IRQHandler
    def_irq_handler TIMER7_Channel_IRQHandler
    def_irq_handler ADC2_IRQHandler
    def_irq_handler EXMC_IRQHandler
    def_irq_handler SDIO_IRQHandler
    def_irq_handler TIMER4_IRQHandler
    def_irq_handler SPI2_IRQHandler
    def_irq_handler UART3_IRQHandler
    def_irq_handler UART4_IRQHandler
    def_irq_handler TIMER5_IRQHandler
    def_irq_handler TIMER6_IRQHandler
    def_irq_handler DMA1_Channel0_IRQHandler
    def_irq_handler DMA1_Channel1_IRQHandler
    def_irq_handler DMA1_Channel2_IRQHandler
    def_irq_handler DMA1_Channel3_4_IRQHandler
