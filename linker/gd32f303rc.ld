/* gd32f303rc.ld — GCC linker script for GD32F303RCT6
 * Memory layout:
 *   FLASH: 256 KB @ 0x08000000
 *   RAM:    48 KB @ 0x20000000
 *
 * Symbols exported (used by startup_gd32f303rc.S + newlib syscalls):
 *   _estack   = top of stack (highest RAM address)
 *   _sidata   = .data 段在 Flash 里的 LMA 起点
 *   _sdata    = .data 段在 RAM 里的 VMA 起点
 *   _edata    = .data 段 RAM 终点
 *   _sbss     = .bss 段起点
 *   _ebss     = .bss 段终点
 *   _end      = bss 之后（newlib _sbrk 从这里向上分配 heap）
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 48K
}

/* Top of stack = highest RAM address (栈向下增长) */
_estack    = ORIGIN(RAM) + LENGTH(RAM);

/* Reserve heap + stack sizes (检查用，实际地址由 _end / _estack 决定) */
_Min_Heap_Size  = 0x400;     /* 1 KB heap (newlib _sbrk) */
_Min_Stack_Size = 0x800;     /* 2 KB stack */

SECTIONS
{
    /* 中断向量表必须在 Flash 起点 */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* 程序代码 */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* 只读数据 */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* C++ 静态构造函数 (newlib __libc_init_array 用) */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    /* .data: VMA 在 RAM、LMA 在 Flash；启动汇编从 _sidata 复制到 _sdata */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* .bss: 启动汇编清零 */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    /* User heap + stack 占位（不实际分配，只验证还有足够 RAM） */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);          /* newlib _sbrk 从这里开始分 heap */
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    /* DWARF debug 段 (留 -g 调试信息) */
    .ARM.attributes 0 : { *(.ARM.attributes) }
}
