#include "bsp_led.h"

uint32_t led_pin[8]= {
  GPIO_PIN_8,
  GPIO_PIN_9,
  GPIO_PIN_10,
  GPIO_PIN_11,
  GPIO_PIN_12,
  GPIO_PIN_13,
  GPIO_PIN_14,
  GPIO_PIN_15
};

void gpio_pp_init(rcu_periph_enum rcu,uint32_t port,uint32_t pin) {
  rcu_periph_clock_enable(rcu);
  gpio_mode_set(port,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,pin);
  gpio_output_options_set(port,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,pin);
}

void GPIO_config() {
  gpio_pp_init(RCU_GPIOC,GPIOC,GPIO_PIN_6);

  //LED1 (1~8)初始化
  for(uint8_t i=0; i<8; i++) {
    gpio_pp_init(RCU_GPIOD,GPIOD,led_pin[i]);
  }

}

//打开所有灯
void turn_on_all() {
  for(uint8_t i=0; i<8; i++) {
    gpio_bit_reset(GPIOD,led_pin[i]);
  }
}

//关闭所有灯
void turn_off_all() {
  for(uint8_t i=0; i<8; i++) {
    gpio_bit_set(GPIOD,led_pin[i]);
  }
}

//开单个灯
void led_turn_on(uint8_t i) {
  gpio_bit_reset(GPIOD,led_pin[i]);
}

//关单个灯
void led_turn_off(uint8_t i) {
  gpio_bit_set(GPIOD,led_pin[i]);
}