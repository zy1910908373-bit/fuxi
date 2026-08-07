#include "gd32f4xx.h"

// 蜂鸣器 PWM 初始化 (PB9 -> TIMER3_CH3)
void buzzer_pwm_init(void)
{
    // 1. 使能 GPIOB 和 TIMER3 时钟
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_TIMER3);

    // 2. 配置 PB9 为复用推挽输出 (AF2 -> TIMER3_CH3)
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_9); // AF2 代表 TIMER3

    // 3. 配置 TIMER3 参数，产生 2.7kHz 的 PWM 方波
    // TIMER3 在 APB1 总线上，时钟为 84MHz
    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocinitpara;

    timer_deinit(TIMER3);

    // 预分频器 83 -> 定时器计数频率 = 84MHz / (83 + 1) = 1MHz (周期 1us)
    timer_initpara.prescaler         = 83;      
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    // 自动重装载值 370 -> PWM 频率 = 1MHz / 370 ≈ 2702Hz (完美的 2.7kHz 蜂鸣器谐振频率)
    timer_initpara.period            = 370;     
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER3, &timer_initpara);

    // 4. 配置通道 3 输出 50% 占空比的 PWM 方波
    timer_ocinitpara.outputstate  = TIMER_CCX_DISABLE; // 默认关闭输出（响声停止）
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_channel_output_config(TIMER3, TIMER_CH_3, &timer_ocinitpara);

    // 占空比 50% (370 / 2 = 185)
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, 185);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_PWM0);

    // 5. 启动 TIMER3 计数器
    timer_enable(TIMER3);
}

void buzzer_init()
{
		buzzer_pwm_init();
}

// 开启蜂鸣器（只需开启 PWM 输出通道）
void buzzer_on(void)
{
    timer_channel_output_state_config(TIMER3, TIMER_CH_3, TIMER_CCX_ENABLE);
}

// 关闭蜂鸣器（只需关闭 PWM 输出通道）
void buzzer_off(void)
{
    timer_channel_output_state_config(TIMER3, TIMER_CH_3, TIMER_CCX_DISABLE);
}