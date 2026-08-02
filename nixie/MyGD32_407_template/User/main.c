#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include <stdlib.h>

// ================= 引脚宏定义 =================
#define BUZZER_PORT     GPIOB
#define BUZZER_PIN      GPIO_PIN_9

#define MOT_PORT        GPIOB
#define MOT_PIN         GPIO_PIN_8

#define LED_SW_PORT     GPIOC
#define LED_SW_PIN      GPIO_PIN_6

#define NIX_DI_PORT     GPIOD
#define NIX_DI_PIN      GPIO_PIN_0
#define NIX_RCK_PORT    GPIOD
#define NIX_RCK_PIN     GPIO_PIN_1
#define NIX_SCK_PORT    GPIOD
#define NIX_SCK_PIN     GPIO_PIN_4

#define BUZZER(x)   gpio_bit_write(BUZZER_PORT, BUZZER_PIN, (x) ? SET : RESET)
#define MOT(x)      gpio_bit_write(MOT_PORT, MOT_PIN, (x) ? SET : RESET)
#define LED_SW(x)   gpio_bit_write(LED_SW_PORT, LED_SW_PIN, (x) ? SET : RESET)

#define NIX_DI(x)   gpio_bit_write(NIX_DI_PORT, NIX_DI_PIN, (x) ? SET : RESET)
#define NIX_RCK(x)  gpio_bit_write(NIX_RCK_PORT, NIX_RCK_PIN, (x) ? SET : RESET)
#define NIX_SCK(x)  gpio_bit_write(NIX_SCK_PORT, NIX_SCK_PIN, (x) ? SET : RESET)

// ================= 全局音符频率定义 =================
#define NOTE_0   0
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_Bb4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_Eb5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_Fs5 740
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_Bb5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319

typedef struct {
    uint16_t freq;     // 频率(Hz)
    uint16_t duration; // 持续时间(ms)
    uint8_t  vibe_ms;  // 马达震动时间(ms)
} Note_t;

// 1. ?? 周杰伦《稻香》精选副歌曲谱
const Note_t DAO_XIANG_MELODY[] = {
    // 还记得你说家是唯一的城堡 (5 5 5 3 2 1 1 1)
    {NOTE_G5, 200, 50}, {NOTE_G5, 200, 20}, {NOTE_G5, 200, 20}, {NOTE_E5, 200, 20}, 
    {NOTE_D5, 200, 20}, {NOTE_C5, 200, 50}, {NOTE_C5, 200, 0},  {NOTE_C5, 300, 0}, {NOTE_0, 100, 0},

    // 随着稻风河流继续奔跑 (5 5 5 3 2 1 2 3 2)
    {NOTE_G5, 200, 50}, {NOTE_G5, 200, 20}, {NOTE_G5, 200, 20}, {NOTE_E5, 200, 20}, 
    {NOTE_D5, 200, 20}, {NOTE_C5, 200, 20}, {NOTE_D5, 200, 20}, {NOTE_E5, 200, 50}, {NOTE_D5, 300, 0}, {NOTE_0, 100, 0},

    // 微微笑 小时候的梦我知道 (5 5 5 3 2 1 1 1)
    {NOTE_G5, 200, 50}, {NOTE_G5, 200, 20}, {NOTE_G5, 200, 20}, {NOTE_E5, 200, 20}, 
    {NOTE_D5, 200, 20}, {NOTE_C5, 200, 50}, {NOTE_C5, 200, 0},  {NOTE_C5, 300, 0}, {NOTE_0, 100, 0},

    // 不要哭 让自己勇敢的站起来 (5 5 5 3 2 1 2 3 1)
    {NOTE_G5, 200, 50}, {NOTE_G5, 200, 20}, {NOTE_G5, 200, 20}, {NOTE_E5, 200, 20}, 
    {NOTE_D5, 200, 20}, {NOTE_C5, 200, 20}, {NOTE_D5, 200, 20}, {NOTE_E5, 200, 50}, {NOTE_C5, 300, 0}, {NOTE_0, 100, 0},

    // 追不到的梦想 换个梦不就好了 (1 1 2 3 3 3 2 1 2 1)
    {NOTE_C5, 180, 20}, {NOTE_C5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_E5, 200, 50}, 
    {NOTE_E5, 180, 20}, {NOTE_E5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_C5, 180, 20}, {NOTE_D5, 200, 50}, {NOTE_C5, 300, 0}, {NOTE_0, 100, 0},

    // 为自己的人生鲜艳上色 (1 1 2 3 3 3 2 1 2 3)
    {NOTE_C5, 180, 20}, {NOTE_C5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_E5, 200, 50}, 
    {NOTE_E5, 180, 20}, {NOTE_E5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_C5, 180, 20}, {NOTE_D5, 200, 20}, {NOTE_E5, 300, 50}, {NOTE_0, 200, 0}
};

// 2. ?? 宫崎骏《Always with Me》(永远同在 / 千与千寻) 曲谱
const Note_t ALWAYS_WITH_YOU_MELODY[] = {
    // 3 4 | 5 - 3 1 | 7(low) - 1 3 | 5 - - | 4 - 3 2 | 1 - - | 7(low) - 1 2 | 3 - -
    {NOTE_E5, 250, 20}, {NOTE_F5, 250, 20},
    {NOTE_G5, 500, 50}, {NOTE_E5, 250, 20}, {NOTE_C5, 250, 20},
    {NOTE_B4, 500, 50}, {NOTE_C5, 250, 20}, {NOTE_E5, 250, 20},
    {NOTE_G5, 750, 50}, {NOTE_0, 250, 0},
    {NOTE_F5, 500, 50}, {NOTE_E5, 250, 20}, {NOTE_D5, 250, 20},
    {NOTE_C5, 750, 50}, {NOTE_0, 250, 0},
    {NOTE_B4, 500, 50}, {NOTE_C5, 250, 20}, {NOTE_D5, 250, 20},
    {NOTE_E5, 750, 50}, {NOTE_0, 250, 0},

    // 高潮句: 5 1' | 7 - 6 4 | 1' - 7 5 | 3 - - | 6 - 5 4 | 3 - 2 1 | 2 - -
    {NOTE_G5, 250, 20}, {NOTE_C6, 250, 50},
    {NOTE_B5, 500, 50}, {NOTE_A5, 250, 20}, {NOTE_F5, 250, 20},
    {NOTE_C6, 500, 50}, {NOTE_B5, 250, 20}, {NOTE_G5, 250, 20},
    {NOTE_E5, 750, 50}, {NOTE_0, 250, 0},
    {NOTE_A5, 500, 50}, {NOTE_G5, 250, 20}, {NOTE_F5, 250, 20},
    {NOTE_E5, 500, 50}, {NOTE_D5, 250, 20}, {NOTE_C5, 250, 20},
    {NOTE_D5, 750, 50}, {NOTE_0, 250, 0}
};

// 3. ??《超级玛丽》经典旋律
const Note_t MARIO_FULL_MELODY[] = {
    {NOTE_E5, 120, 50}, {NOTE_E5, 120, 0},  {NOTE_0, 120, 0},  {NOTE_E5, 120, 20}, 
    {NOTE_0, 120, 0},   {NOTE_C5, 120, 20}, {NOTE_E5, 120, 50}, {NOTE_0, 120, 0}, 
    {NOTE_G5, 240, 50}, {NOTE_0, 240, 0},   {NOTE_G4, 240, 50}, {NOTE_0, 240, 0},
    {NOTE_C5, 200, 50}, {NOTE_0, 100, 0},   {NOTE_G4, 200, 20}, {NOTE_0, 100, 0}, 
    {NOTE_E4, 200, 50}, {NOTE_0, 100, 0},   {NOTE_A4, 150, 20}, {NOTE_B4, 150, 20}, 
    {NOTE_Bb4, 150, 20},{NOTE_A4, 180, 50}, {NOTE_G4, 150, 20}, {NOTE_E5, 150, 20}
};

// 歌单枚举
typedef enum {
    SONG_DAO_XIANG = 0,       // 1. 周杰伦《稻香》
    SONG_ALWAYS_WITH_YOU,    // 2. 宫崎骏《Always with Me》
    SONG_SUPER_MARIO,         // 3. 《超级玛丽》
    SONG_TOTAL_NUM
} Song_ID_t;

// 改变此变量可指定播放哪首歌！
Song_ID_t g_current_song = SONG_SUPER_MARIO; 

// 火柴人姿势表
const uint8_t DANCE_POSES[6] = {
    ~(0x01 | 0x20 | 0x02 | 0x40 | 0x10 | 0x04), // 0: 立正
    ~(0x20 | 0x01 | 0x40 | 0x04 | 0x08),        // 1: 左迪斯科
    ~(0x02 | 0x01 | 0x40 | 0x10 | 0x08),        // 2: 右迪斯科
    ~(0x01 | 0x20 | 0x02 | 0x40),               // 3: 空中跳跃
    ~(0x40 | 0x10 | 0x04 | 0x08),               // 4: 街舞下蹲
    ~(0x20 | 0x10 | 0x40 | 0x08)                // 5: 侧身旋转
};

volatile uint32_t g_system_ticks = 0;
volatile uint32_t g_buzzer_half_period = 0;

uint8_t g_current_note_level = 0;
uint16_t g_motor_vibe_timer = 0;
uint8_t g_dance_frame = 0;

const uint8_t DIG_MASK[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t nix_display_buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct {
    uint32_t period;
    uint32_t last_run;
    void (*task_func)(void);
} Task_t;

void delay_1us(uint32_t count) {
    uint32_t i;
    while (count--) {
        for (i = 0; i < 30; i++) __NOP();
    }
}

void HC595_SendByte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        NIX_SCK(0);
        NIX_DI((byte >> i) & 0x01);
        NIX_SCK(1);
    }
}

void HC595_WriteData(uint8_t com_mask, uint8_t seg_code) {
    NIX_RCK(0);
    HC595_SendByte(seg_code);
    HC595_SendByte(com_mask);
    NIX_RCK(1);
}

// ================= 任务 1：数码管 1ms 无阻塞扫描 =================
void Task_NixDisplay_Scan(void) {
    static uint8_t current_dig = 0;
    HC595_WriteData(DIG_MASK[current_dig], nix_display_buf[current_dig]);
    current_dig = (current_dig + 1) % 8;
}

// ================= 任务 2：多曲谱播放器 & 视触联动 =================
void Task_Music_Player(void) {
    static uint16_t note_idx = 0;
    static uint32_t note_start_time = 0;

    const Note_t* cur_melody = NULL;
    uint16_t melody_len = 0;

    // 根据全局变量 g_current_song 动态切换歌单
    switch (g_current_song) {
        case SONG_DAO_XIANG:
            cur_melody = DAO_XIANG_MELODY;
            melody_len = sizeof(DAO_XIANG_MELODY) / sizeof(Note_t);
            break;
        case SONG_ALWAYS_WITH_YOU:
            cur_melody = ALWAYS_WITH_YOU_MELODY;
            melody_len = sizeof(ALWAYS_WITH_YOU_MELODY) / sizeof(Note_t);
            break;
        case SONG_SUPER_MARIO:
        default:
            cur_melody = MARIO_FULL_MELODY;
            melody_len = sizeof(MARIO_FULL_MELODY) / sizeof(Note_t);
            break;
    }

    Note_t cur = cur_melody[note_idx];

    if (g_system_ticks - note_start_time >= cur.duration) {
        note_start_time = g_system_ticks;
        note_idx++;
        
        // 自动切歌控制：一首唱完自动播放下一首！
        if (note_idx >= melody_len) {
            note_idx = 0;
//            g_current_song = (Song_ID_t)((g_current_song + 1) % SONG_TOTAL_NUM);
        }
        
        cur = cur_melody[note_idx];

        // 1. 发声控制
        if (cur.freq > 0) {
            g_buzzer_half_period = 500000 / cur.freq;
            g_current_note_level = (cur.freq - 200) / 80;
            if (g_current_note_level > 8) g_current_note_level = 8;
            if (g_current_note_level < 1) g_current_note_level = 1;
        } else {
            g_buzzer_half_period = 0;
            g_current_note_level = 0;
            BUZZER(0);
        }

        // 2. 马达节拍震动
        if (cur.vibe_ms > 0) {
            g_motor_vibe_timer = cur.vibe_ms; 
            MOT(1);
        }

        // 3. 火柴人动作帧自增
        g_dance_frame = (g_dance_frame + 1) % 6;
    }
}

// ================= 任务 3：马达触觉控制 =================
void Task_Motor_Control(void) {
    if (g_motor_vibe_timer > 0) {
        g_motor_vibe_timer--;
        if (g_motor_vibe_timer == 0) {
            MOT(0);
        }
    }
}

// ================= 任务 4：8路 LED VU 音乐频谱柱 =================
void Task_LED_VU_Meter(void) {
    LED_SW(0);

    uint8_t led_mask = 0xFF;
    if (g_current_note_level > 0) {
        led_mask = ~(0xFF >> (8 - g_current_note_level));
    }
    gpio_port_write(GPIOD, (gpio_output_port_get(GPIOD) & 0x00FF) | ((uint16_t)led_mask << 8));
}

// ================= 任务 5：数码管火柴人舞蹈动画 =================
void Task_Dancer_Animation(void) {
    // 舞台音浪灯
    const uint8_t stage_light[4] = {0xFF, ~0x08, ~(0x08|0x40), ~(0x08|0x40|0x01)};
    uint8_t h = g_current_note_level / 3;
    if (h > 3) h = 3;
    nix_display_buf[0] = stage_light[h];
    nix_display_buf[7] = stage_light[h];

    // 伴舞小人
    nix_display_buf[1] = DANCE_POSES[(g_dance_frame) % 6];
    nix_display_buf[6] = DANCE_POSES[(g_dance_frame + 3) % 6];

    // C 位双人领舞 (对舞特效)
    if (g_current_note_level > 5) {
        nix_display_buf[3] = DANCE_POSES[3]; // 高音双双跳跃
        nix_display_buf[4] = DANCE_POSES[3];
    } else {
        nix_display_buf[3] = DANCE_POSES[1 + (g_dance_frame % 2)];
        nix_display_buf[4] = DANCE_POSES[2 - (g_dance_frame % 2)];
    }

    nix_display_buf[2] = 0xFF;
    nix_display_buf[5] = 0xFF;
}

// ================= 硬件初始化 =================
void Hardware_Init(void) {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);

    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
                  GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | 0xFF00);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, 
                            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | 0xFF00);

    BUZZER(0);
    MOT(0);
    LED_SW(1);
    gpio_bit_write(GPIOD, 0xFF00, SET);
}

// ================= 蜂鸣器发声软定时器 =================
void Buzzer_Service(void) {
    if (g_buzzer_half_period > 0) {
        delay_1us(g_buzzer_half_period);
        gpio_bit_write(GPIOB, GPIO_PIN_9, (gpio_output_bit_get(GPIOB, GPIO_PIN_9) == RESET) ? SET : RESET);
    }
}

// ================= 主函数与时间片调度 =================
int main(void) {
    systick_config();
    Hardware_Init();

    Task_t tasks[] = {
        { 1,   0, Task_NixDisplay_Scan }, // 1ms  数码管扫描
        { 1,   0, Task_Motor_Control    }, // 1ms  马达触觉控制
        { 10,  0, Task_Music_Player     }, // 10ms 播放器与曲谱切歌
        { 20,  0, Task_LED_VU_Meter     }, // 20ms LED 舞台灯
        { 30,  0, Task_Dancer_Animation }  // 30ms 火柴人舞蹈
    };
    uint8_t task_num = sizeof(tasks) / sizeof(Task_t);

    while (1) {
        uint32_t now = g_system_ticks;

        for (int i = 0; i < task_num; i++) {
            if (now - tasks[i].last_run >= tasks[i].period) {
                tasks[i].last_run = now;
                tasks[i].task_func();
            }
        }

        Buzzer_Service();
    }
}