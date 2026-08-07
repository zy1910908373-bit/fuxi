#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t StartTask_Handler;
TaskHandle_t CyberpunkTask_Handler;
TaskHandle_t ScreenDanceTask_Handler;

/* ---------------- 硬件引脚定义 ---------------- */
#define BUZZER_PIN          GPIO_PIN_9  /* PB9 蜂鸣器 */
#define MOTOR_PIN           GPIO_PIN_8  /* PB8 震动电机 */
#define BUZZER_PORT         GPIOB

#define NIX_DI_PIN          GPIO_PIN_0  /* PD0 595 数据 */
#define NIX_RCK_PIN         GPIO_PIN_1  /* PD1 595 锁存 */
#define NIX_SCK_PIN         GPIO_PIN_4  /* PD4 595 时钟 */
#define NIX_PORT            GPIOD

/* SPI OLED 屏幕引脚定义 (天空星扩展板) */
#define SPI_OLED_SCL_PIN    GPIO_PIN_5  /* PA5 CLK */
#define SPI_OLED_SDA_PIN    GPIO_PIN_7  /* PA7 MOSI (修复：之前漏掉) */
#define SPI_OLED_DC_PIN     GPIO_PIN_4  /* PA4 DC (修复：之前错设为输入) */
#define SPI_OLED_CS1_PIN    GPIO_PIN_3  /* PA3 CS1 */
#define SPI_OLED_CS2_PIN    GPIO_PIN_5  /* PC5 CS2 (修复：第二块屏片选) */
#define SPI_ROM_CS_PIN      GPIO_PIN_2  /* PA2 字库芯片片选 */
#define SPI_ROM_FS0_PIN     GPIO_PIN_6  /* PA6 字库数据输入 (MISO) */

/* I2C OLED 屏幕引脚定义 */
#define I2C_OLED_SCL_PIN    GPIO_PIN_6  /* PB6 SCL */
#define I2C_OLED_SDA_PIN    GPIO_PIN_7  /* PB7 SDA */
#define I2C_OLED_PORT       GPIOB
#define I2C_OLED_ADDR1      0x78
#define I2C_OLED_ADDR2      0x7A

/* 8路 LED 引脚 */
static const uint32_t LED_PINS[8] = {
    GPIO_PIN_6,  GPIO_PIN_8,  GPIO_PIN_10, GPIO_PIN_11,
    GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_9
};

/* 595 涡轮帧 */
static const uint16_t TURBO_16BIT_FRAMES[6] = {
    (0x01 << 8) | 0x20, (0x02 << 8) | 0x10, (0x04 << 8) | 0x08,
    (0x08 << 8) | 0x04, (0x10 << 8) | 0x02, (0x20 << 8) | 0x01
};

/* 音符定义 */
#define REST 0
#define A3   220
#define B3   247
#define C4   262
#define D4   294
#define F4   349
#define G4   392
#define G4S  415
#define A4   440
#define C5   523
#define D5   587
#define F5   698
#define G5   784
#define A5   880

typedef struct {
    uint16_t freq;
    uint16_t duration;
    uint8_t  vibrate;
} CyberNote;

static const CyberNote Cyber_Melody[] = {
    {D4, 120, 1}, {D4, 120, 0}, {D5, 240, 1}, {A4, 240, 0}, {REST, 60, 0},
    {G4S,200, 1}, {G4, 200, 0}, {F4, 200, 1}, {D4, 120, 0}, {F4, 120, 0}, {G4, 120, 1},
    {C4, 120, 1}, {C4, 120, 0}, {D5, 240, 1}, {A4, 240, 0}, {REST, 60, 0},
    {G4S,200, 1}, {G4, 200, 0}, {F4, 200, 1}, {D4, 120, 0}, {F4, 120, 0}, {G4, 120, 1},
    {B3, 120, 1}, {B3, 120, 0}, {D5, 240, 1}, {A4, 240, 0}, {REST, 60, 0},
    {G4S,200, 1}, {G4, 200, 0}, {F4, 200, 1}, {D4, 120, 0}, {F4, 120, 0}, {G4, 120, 1},
    {A3, 120, 1}, {A3, 120, 0}, {D5, 240, 1}, {A4, 240, 0}, {REST, 60, 0},
    {G4S,200, 1}, {G4, 200, 0}, {F4, 200, 1}, {D4, 120, 0}, {F4, 120, 0}, {G4, 120, 1},
    {REST, 300, 0}
};

/* 小人点阵 32x32 (2帧) */
static const uint8_t Dance_Frame_0[128] = {
    0x00,0x00,0x00,0x00,0xF0,0x08,0x08,0xF0,0x00,0xE0,0x10,0x08,0x04,0x02,0x00,0x00,
    0x00,0x02,0x04,0x08,0x10,0xE0,0x00,0xF0,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x0F,0x10,0x10,0x0F,0x01,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
    0x00,0xFF,0x00,0x00,0x00,0x00,0x01,0x0F,0x10,0x10,0x0F,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x07,0x00,
    0x07,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00,
    0x20,0x10,0x08,0x04,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t Dance_Frame_1[128] = {
    0x00,0x00,0x00,0x00,0xF0,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x02,0x04,0x08,0x10,0xE0,0x00,0xF0,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x0F,0x10,0x10,0x0F,0x00,0x80,0x40,0x20,0x10,0x08,0xFF,0x00,
    0x00,0xFF,0x00,0x00,0x00,0x00,0x01,0x0F,0x10,0x10,0x0F,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x0F,0x00,
    0x07,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x00,
    0x20,0x10,0x08,0x04,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t* Dance_Frames[2] = { Dance_Frame_0, Dance_Frame_1 };

/* ---------------- 免 SysTick 微秒延时 ---------------- */
static void CPU_DelayUs(uint32_t us) {
    volatile uint32_t count = us * (SystemCoreClock / 1000000U) / 5U;
    while (count--) { __NOP(); }
}

/* ---------------- 1. SPI OLED 驱动底层 ---------------- */
static void SPI_OLED_WR_Byte(uint8_t dat, uint8_t is_data, uint8_t cs_mask) {
    /* DC 选择 */
    if (is_data)
        gpio_bit_set(GPIOA, SPI_OLED_DC_PIN);
    else
        gpio_bit_reset(GPIOA, SPI_OLED_DC_PIN);

    /* 片选控制: bit0-CS1(PA3), bit1-CS2(PC5) */
    if (cs_mask & 0x01) gpio_bit_reset(GPIOA, SPI_OLED_CS1_PIN);
    if (cs_mask & 0x02) gpio_bit_reset(GPIOC, SPI_OLED_CS2_PIN);

    for (uint8_t i = 0; i < 8; i++) {
        gpio_bit_reset(GPIOA, SPI_OLED_SCL_PIN);
        if (dat & 0x80)
            gpio_bit_set(GPIOA, SPI_OLED_SDA_PIN);
        else
            gpio_bit_reset(GPIOA, SPI_OLED_SDA_PIN);
        gpio_bit_set(GPIOA, SPI_OLED_SCL_PIN);
        dat <<= 1;
    }

    if (cs_mask & 0x01) gpio_bit_set(GPIOA, SPI_OLED_CS1_PIN);
    if (cs_mask & 0x02) gpio_bit_set(GPIOC, SPI_OLED_CS2_PIN);
    gpio_bit_set(GPIOA, SPI_OLED_DC_PIN);
}

static void SPI_OLED_InitDevice(uint8_t cs_mask) {
    SPI_OLED_WR_Byte(0xAE, 0, cs_mask); /* turn off oled panel */
    SPI_OLED_WR_Byte(0x00, 0, cs_mask); /* low column */
    SPI_OLED_WR_Byte(0x10, 0, cs_mask); /* high column */
    SPI_OLED_WR_Byte(0x40, 0, cs_mask); /* start line */
    SPI_OLED_WR_Byte(0x81, 0, cs_mask); /* contrast */
    SPI_OLED_WR_Byte(0xCF, 0, cs_mask);
    SPI_OLED_WR_Byte(0xA1, 0, cs_mask); /* SEG Mapping */
    SPI_OLED_WR_Byte(0xC8, 0, cs_mask); /* COM Scan */
    SPI_OLED_WR_Byte(0xA6, 0, cs_mask); /* normal display */
    SPI_OLED_WR_Byte(0xA8, 0, cs_mask); /* multiplex ratio */
    SPI_OLED_WR_Byte(0x3F, 0, cs_mask); /* 1/64 duty */
    SPI_OLED_WR_Byte(0xD3, 0, cs_mask); /* display offset */
    SPI_OLED_WR_Byte(0x00, 0, cs_mask);
    SPI_OLED_WR_Byte(0xD5, 0, cs_mask); /* clock divide */
    SPI_OLED_WR_Byte(0x80, 0, cs_mask);
    SPI_OLED_WR_Byte(0xD9, 0, cs_mask); /* pre-charge */
    SPI_OLED_WR_Byte(0xF1, 0, cs_mask);
    SPI_OLED_WR_Byte(0xDA, 0, cs_mask); /* com pins */
    SPI_OLED_WR_Byte(0x12, 0, cs_mask);
    SPI_OLED_WR_Byte(0xDB, 0, cs_mask); /* vcomh */
    SPI_OLED_WR_Byte(0x40, 0, cs_mask);
    SPI_OLED_WR_Byte(0x20, 0, cs_mask); /* Page Addressing */
    SPI_OLED_WR_Byte(0x02, 0, cs_mask);
    SPI_OLED_WR_Byte(0x8D, 0, cs_mask); /* Charge Pump */
    SPI_OLED_WR_Byte(0x14, 0, cs_mask);
    SPI_OLED_WR_Byte(0xA4, 0, cs_mask);
    SPI_OLED_WR_Byte(0xAF, 0, cs_mask); /* display ON */
}

static void SPI_OLED_Clear(uint8_t cs_mask) {
    for (uint8_t page = 0; page < 8; page++) {
        SPI_OLED_WR_Byte(0xB0 + page, 0, cs_mask);
        SPI_OLED_WR_Byte(0x00, 0, cs_mask);
        SPI_OLED_WR_Byte(0x10, 0, cs_mask);
        for (uint8_t col = 0; col < 128; col++) {
            SPI_OLED_WR_Byte(0x00, 1, cs_mask);
        }
    }
}

static void SPI_OLED_DrawDanceMan(uint8_t cs_mask, uint8_t x, uint8_t page_start, const uint8_t *frame) {
    for (uint8_t page = 0; page < 4; page++) {
        SPI_OLED_WR_Byte(0xB0 + page_start + page, 0, cs_mask);
        SPI_OLED_WR_Byte(x & 0x0F, 0, cs_mask);
        SPI_OLED_WR_Byte(0x10 | ((x >> 4) & 0x0F), 0, cs_mask);
        for (uint8_t col = 0; col < 32; col++) {
            SPI_OLED_WR_Byte(frame[page * 32 + col], 1, cs_mask);
        }
    }
}

/* ---------------- 2. I2C OLED 驱动底层 ---------------- */
static void I2C_Start(void) {
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
    CPU_DelayUs(2);
    gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
    CPU_DelayUs(2);
    gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
}

static void I2C_Stop(void) {
    gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
    CPU_DelayUs(2);
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
    CPU_DelayUs(2);
}

static void I2C_WriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & (0x80 >> i))
            gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
        else
            gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SDA_PIN);

        gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
        CPU_DelayUs(2);
        gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
        CPU_DelayUs(2);
    }
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SDA_PIN);
    gpio_bit_set(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
    CPU_DelayUs(2);
    gpio_bit_reset(I2C_OLED_PORT, I2C_OLED_SCL_PIN);
}

static void I2C_OLED_WriteCmd(uint8_t addr, uint8_t cmd) {
    I2C_Start();
    I2C_WriteByte(addr);
    I2C_WriteByte(0x00);
    I2C_WriteByte(cmd);
    I2C_Stop();
}

static void I2C_OLED_WriteData(uint8_t addr, uint8_t data) {
    I2C_Start();
    I2C_WriteByte(addr);
    I2C_WriteByte(0x40);
    I2C_WriteByte(data);
    I2C_Stop();
}

static void I2C_OLED_InitDevice(uint8_t addr) {
    I2C_OLED_WriteCmd(addr, 0xAE);
    I2C_OLED_WriteCmd(addr, 0x20); I2C_OLED_WriteCmd(addr, 0x02);
    I2C_OLED_WriteCmd(addr, 0x8D); I2C_OLED_WriteCmd(addr, 0x14);
    I2C_OLED_WriteCmd(addr, 0xA1); I2C_OLED_WriteCmd(addr, 0xC8);
    I2C_OLED_WriteCmd(addr, 0xAF);
}

static void I2C_OLED_Clear(uint8_t addr) {
    for (uint8_t page = 0; page < 8; page++) {
        I2C_OLED_WriteCmd(addr, 0xB0 + page);
        I2C_OLED_WriteCmd(addr, 0x00);
        I2C_OLED_WriteCmd(addr, 0x10);
        for (uint8_t col = 0; col < 128; col++) {
            I2C_OLED_WriteData(addr, 0x00);
        }
    }
}

static void I2C_OLED_DrawDanceMan(uint8_t addr, uint8_t x, uint8_t page_start, const uint8_t *frame) {
    for (uint8_t page = 0; page < 4; page++) {
        I2C_OLED_WriteCmd(addr, 0xB0 + page_start + page);
        I2C_OLED_WriteCmd(addr, (x & 0x0F));
        I2C_OLED_WriteCmd(addr, 0x10 | ((x >> 4) & 0x0F));
        for (uint8_t col = 0; col < 32; col++) {
            I2C_OLED_WriteData(addr, frame[page * 32 + col]);
        }
    }
}

/* ---------------- 74HC595 16-Bit ---------------- */
static void HC595_Send16Bit(uint16_t data) {
    for (int i = 0; i < 16; i++) {
        if (data & (0x8000 >> i))
            gpio_bit_set(NIX_PORT, NIX_DI_PIN);
        else
            gpio_bit_reset(NIX_PORT, NIX_DI_PIN);

        gpio_bit_reset(NIX_PORT, NIX_SCK_PIN);
        gpio_bit_set(NIX_PORT, NIX_SCK_PIN);
    }
    gpio_bit_reset(NIX_PORT, NIX_RCK_PIN);
    gpio_bit_set(NIX_PORT, NIX_RCK_PIN);
}

/* ---------------- 硬件初始化 ---------------- */
void Hardware_Init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    /* 1. PB9 蜂鸣器 + PB8 震动电机 */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BUZZER_PIN | MOTOR_PIN);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BUZZER_PIN | MOTOR_PIN);

    /* 2. 数码管 595 */
    gpio_mode_set(NIX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, NIX_DI_PIN | NIX_RCK_PIN | NIX_SCK_PIN);
    gpio_output_options_set(NIX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, NIX_DI_PIN | NIX_RCK_PIN | NIX_SCK_PIN);

    /* 3. 8路 LED */
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
                  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
                  GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, 
                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
                            GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    /* 4. SPI OLED 完美硬件初始化配置 (彻底解决黑屏) */
    /* PA2(ROM_CS), PA3(CS1), PA4(DC-推挽输出!), PA5(SCL), PA7(SDA) */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
                  SPI_ROM_CS_PIN | SPI_OLED_CS1_PIN | SPI_OLED_DC_PIN | SPI_OLED_SCL_PIN | SPI_OLED_SDA_PIN);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, 
                            SPI_ROM_CS_PIN | SPI_OLED_CS1_PIN | SPI_OLED_DC_PIN | SPI_OLED_SCL_PIN | SPI_OLED_SDA_PIN);
    
    /* PC5(CS2-推挽输出!) */
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_OLED_CS2_PIN);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_OLED_CS2_PIN);

    /* PA6 (字库 MISO 上拉输入) */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, SPI_ROM_FS0_PIN);

    /* 拉高所有 CS，准备就绪 */
    gpio_bit_set(GPIOA, SPI_ROM_CS_PIN | SPI_OLED_CS1_PIN);
    gpio_bit_set(GPIOC, SPI_OLED_CS2_PIN);

    /* 5. I2C OLED 引脚初始化 PB6, PB7 */
    gpio_mode_set(I2C_OLED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C_OLED_SCL_PIN | I2C_OLED_SDA_PIN);
    gpio_output_options_set(I2C_OLED_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, I2C_OLED_SCL_PIN | I2C_OLED_SDA_PIN);

    /* 6. 点亮所有屏幕（SPI 屏1 + SPI 屏2 + I2C 屏1 + I2C 屏2） */
    SPI_OLED_InitDevice(0x03); /* 同时初始化 CS1 和 CS2 */
    SPI_OLED_Clear(0x03);

    I2C_OLED_InitDevice(I2C_OLED_ADDR1);
    I2C_OLED_InitDevice(I2C_OLED_ADDR2);
    I2C_OLED_Clear(I2C_OLED_ADDR1);
    I2C_OLED_Clear(I2C_OLED_ADDR2);
}

/* ---------------- 音浪灯效果 ---------------- */
static void Cyber_LED_Effect(uint16_t freq, uint8_t step) {
    uint8_t mask = 0x00;
    if (freq == REST) mask = 0x00;
    else if (freq > 600) mask = (step % 2 == 0) ? 0xFF : 0x00;
    else if (freq > 400) mask = (step % 2 == 0) ? 0x18 : 0x3C;
    else mask = (step % 2 == 0) ? 0x81 : 0xC3;

    for (int k = 0; k < 8; k++) {
        bit_status state = (mask & (1 << k)) ? SET : RESET;
        if (k == 0)
            gpio_bit_write(GPIOC, LED_PINS[k], state);
        else
            gpio_bit_write(GPIOD, LED_PINS[k], state);
    }
}

/* ---------------- 4D 电音引擎 ---------------- */
void Play_4D_CyberTone(uint16_t freq, uint16_t duration_ms, uint8_t vibrate, uint8_t *frame_idx) {
    Cyber_LED_Effect(freq, *frame_idx);

    if (vibrate) gpio_bit_set(GPIOB, MOTOR_PIN);
    else gpio_bit_reset(GPIOB, MOTOR_PIN);

    if (freq == REST) {
        gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
        gpio_bit_reset(GPIOB, MOTOR_PIN);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }

    uint32_t play_ms = (duration_ms * 80U) / 100U;
    uint32_t pause_ms = duration_ms - play_ms;
    uint32_t half_period_us = 1000000U / (freq * 2U);
    uint32_t cycles = (play_ms * 1000U) / (half_period_us * 2U);

    for (uint32_t i = 0; i < cycles; i++) {
        gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
        CPU_DelayUs(half_period_us);
        gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
        CPU_DelayUs(half_period_us);

        if (i % 12 == 0) {
            HC595_Send16Bit(TURBO_16BIT_FRAMES[*frame_idx]);
            *frame_idx = (*frame_idx + 1) % 6;
        }
    }

    gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
    gpio_bit_reset(GPIOB, MOTOR_PIN);
    vTaskDelay(pdMS_TO_TICKS(pause_ms));
}

/* ---------------- 任务 1：电音狂欢任务 ---------------- */
void cyberpunk_task(void *pvParameters) {
    (void)pvParameters;
    uint8_t note_count = sizeof(Cyber_Melody) / sizeof(CyberNote);
    uint8_t frame_idx = 0;

    while (1) {
        for (uint8_t i = 0; i < note_count; i++) {
            Play_4D_CyberTone(Cyber_Melody[i].freq, 
                              Cyber_Melody[i].duration, 
                              Cyber_Melody[i].vibrate, 
                              &frame_idx);
        }
    }
}

/* ---------------- 任务 2：四屏幕像素小人齐舞任务 ---------------- */
void screen_dance_task(void *pvParameters) {
    (void)pvParameters;
    uint8_t dance_step = 0;

    while (1) {
        /* SPI 屏 1 (PA3) 小人在左 */
        SPI_OLED_DrawDanceMan(0x01, 16, 2, Dance_Frames[dance_step]);
        /* SPI 屏 2 (PC5) 小人在右 (镜面对舞) */
        SPI_OLED_DrawDanceMan(0x02, 80, 2, Dance_Frames[1 - dance_step]);

        /* I2C 屏 1 (0x78) 小人在左 */
        I2C_OLED_DrawDanceMan(I2C_OLED_ADDR1, 16, 2, Dance_Frames[dance_step]);
        /* I2C 屏 2 (0x7A) 小人在右 */
        I2C_OLED_DrawDanceMan(I2C_OLED_ADDR2, 80, 2, Dance_Frames[1 - dance_step]);

        dance_step = (dance_step + 1) % 2;
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

/* ---------------- 启动任务 ---------------- */
void start_task(void *pvParameters) {
    (void)pvParameters;
    Hardware_Init();

    xTaskCreate((TaskFunction_t)cyberpunk_task,
                (const char *)"cyber_task",
                256,
                NULL,
                2,
                (TaskHandle_t *)&CyberpunkTask_Handler);

    xTaskCreate((TaskFunction_t)screen_dance_task,
                (const char *)"screen_dance",
                512,
                NULL,
                2,
                (TaskHandle_t *)&ScreenDanceTask_Handler);

    vTaskDelete(StartTask_Handler);
}

int main(void) {
    xTaskCreate((TaskFunction_t)start_task,
                (const char *)"start_task",
                128,
                NULL,
                1,
                (TaskHandle_t *)&StartTask_Handler);

    vTaskStartScheduler();

    while (1) {}
}