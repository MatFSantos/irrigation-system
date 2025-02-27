#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"

#include "modules/pwm.h"
#include "modules/led.h"
#include "modules/push_button.h"
#include "modules/ssd1306.h"
#include "modules/ws2812b.h"

/** definições de pinos */
#define BUZZER 21
#define HUMIDITY_SENSOR 27 /* eixo X */
#define LM35 26 /* eixo Y */
#define PWM_DIVISER 20
#define PWM_WRAP 2000 /* aprox 3.5kHz freq */

/** variaveis globais */
_ws2812b * ws; /* 5x5 matrix */
ssd1306_t ssd; /* oled display ssd1306 */
static volatile uint32_t last_time = 0;
static volatile bool manual_mode = false;
static volatile bool bomb_state = false;
const double resoluion_humidity_sensor = 100.0f/4095.0f; /* % por valor digital*/
const double resolution =  3300.0f/4095.0f; /* resolução do ADC em mV*/

static volatile double temperature = 0.0;
static volatile double humidity = 0.0;

/**
 * @brief Function to read the humidity sensor
 * 
 * @param channel analog channel to read
 * 
 */
void read_humidity(uint8_t channel){
    adc_select_input(channel);
    double value = (double) adc_read();
    humidity = 100 - (value * (100.0f/4095.0f)); /* 100% indica nenhuma umidade, 0% indica umidade total*/
}

/**
 * @brief Function to read the temperature sensor
 * 
 * @param channel analog channel to read
 * 
 */
void read_temperature(uint8_t channel){
    adc_select_input(channel);
    double value = (double) adc_read();
    /*é adicionado um shift positivo de 1.4V, para que os valores em simulação sejam reais usando o potenciometro dos joysticks */
    value = value - (1400*4095/3300);
    if (value < 0) value = 0;
    temperature = (value * (3300.0f/4095.0f))/10; /* converte a leitura para a temperatura em °C*/
}

/**
 * @brief Initialize the SSD1306 display
 *
 */
void init_display(){
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
}

/**
 * @brief Set irrigation mode
 * 
 */
void set_irrigation(bool on){
    if(on) pwm_set_gpio_level(PIN_BLUE_LED, PWM_WRAP/2);
    else pwm_set_gpio_level(PIN_BLUE_LED, 0);
}
/**
 * @brief Initialize the all GPIOs that will be used in project
 *      - I2C;
 *      - RGB LED;
 *      - Push buttons A and B 
 *      - Buzzer;
 */
void init_gpio(){
    /** initialize i2c communication */
    int baudrate = 400*1000; // 400kHz baud rate for i2c communication
    i2c_init(I2C_PORT, baudrate);

    // set GPIO pin function to I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL);

    /** initialize RGB LED */
    pwm_init_(PIN_BLUE_LED);
    pwm_setup(PIN_BLUE_LED, PWM_DIVISER, PWM_WRAP);
    pwm_start(PIN_BLUE_LED, 0);

    pwm_init_(PIN_RED_LED);
    pwm_setup(PIN_RED_LED, PWM_DIVISER, PWM_WRAP);
    pwm_start(PIN_RED_LED, 0);

    pwm_init_(PIN_GREEN_LED);
    pwm_setup(PIN_GREEN_LED, PWM_DIVISER, PWM_WRAP);
    pwm_start(PIN_GREEN_LED, 0);

    /** initialize buttons */
    init_push_button(PIN_BUTTON_A);
    init_push_button(PIN_BUTTON_B);

    /** initialize buzzer */
    pwm_init_(BUZZER);
    pwm_setup(BUZZER, PWM_DIVISER, PWM_WRAP);
    pwm_start(BUZZER, 0);
}


/**
 * @brief Update the display informations
 */
void update_display() {
    char temp[20], hmd[20];
    snprintf(temp,20,"%.0f", temperature);
    snprintf(hmd,20,"%.0f", humidity);
    printf("temperatura: %s\n", temp);
    printf("umidade: %s\n", hmd);
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
    ssd1306_draw_string(&ssd,"temp",3,3);
    ssd1306_draw_string(&ssd,temp,40,3);
    ssd1306_line(&ssd, 3, 12,125,12,true);

    ssd1306_draw_string(&ssd,"umid",3,14);
    ssd1306_draw_string(&ssd,hmd,40,14);
    ssd1306_line(&ssd, 3, 23,125,23,true);

    ssd1306_draw_string(&ssd,bomb_state ? "bomba on": "bomba off",3,25);
    ssd1306_line(&ssd, 3, 34,125,34,true);

    ssd1306_draw_string(&ssd,manual_mode ? "manual on": "manual off",3,36);
    ssd1306_line(&ssd, 3, 45,125,45,true);

    if (humidity < 30.0){
        ssd1306_draw_string(&ssd,"umidade baixa",16,50);
    }
    ssd1306_send_data(&ssd); // update display
}

int64_t turn_off_callback(alarm_id_t id, void *user_data) {
    pwm_set_gpio_level(BUZZER, 0);
    return 0;
}


/**
 * @brief Handler function to interruption
 * 
 * @param gpio GPIO that triggered the interruption
 * @param event The event which caused the interruption
 */
void gpio_irq_handler(uint gpio, uint32_t event) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if(current_time - last_time >= 200) { // debounce
        if (gpio == PIN_BUTTON_A){
            manual_mode = !manual_mode; // ativa/desativa modo manual da bomba
            if (manual_mode){
                pwm_set_gpio_level(BUZZER, PWM_WRAP/2);
                add_alarm_in_ms(500, turn_off_callback, NULL, false);
            }
        }
        else if (gpio == PIN_BUTTON_B && manual_mode){
            bomb_state = !bomb_state;
            set_irrigation(bomb_state);
        }
        last_time = current_time;
    }
}

int main (){
    PIO pio = pio0;
    bool ok;
    uint16_t x = 0, y = 0, x_led_level = 0, y_led_level = 0;

    // set clock freq to 128MHz
    ok = set_sys_clock_khz(128000, false);
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    // init gpios and stdio functions
    stdio_init_all();
    init_gpio();

    //init 5x5 matrix
    ws = init_ws2812b(pio,PIN_WS2812B);
    ws2812b_turn_off(ws);

    // get ws and ssd struct
    init_display();

    // Clear display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // init adc channels
    adc_init();
    adc_gpio_init(HUMIDITY_SENSOR);
    adc_gpio_init(LM35);

    // configure interruptions handlers
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_A, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_B, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
    sleep_ms(50);

    while (1) {
        read_temperature(0);
        read_humidity(1);
        printf("temperatura: %f\n", temperature);
        printf("umidade: %f\n", humidity);
        if (humidity < 30.0){ // plantas ornamentais deve está com a humidade entre 30% e 50%
            pwm_set_gpio_level(PIN_RED_LED, PWM_WRAP/2);
            ws2812b_plot(ws,&SAD);
            if(!manual_mode && !bomb_state) {
                bomb_state = true;
                set_irrigation(bomb_state);
            }
        }
        else if (humidity < 40.0) {
            pwm_set_gpio_level(PIN_RED_LED, 0);
            ws2812b_plot(ws,&NEUTRAL);
        }
        else if (humidity >= 40.0){
            pwm_set_gpio_level(PIN_RED_LED, 0);
            ws2812b_plot(ws,&SMILE);
            if(!manual_mode && bomb_state) {
                bomb_state = false;
                set_irrigation(bomb_state);
            }
        }
        update_display();
        sleep_ms(100);
    }

    return 0;
}