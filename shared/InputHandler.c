#include "InputHandler.h"
#include "main.h"

// Global input state
InputState current_input = {0};

// Track button presses in interrupt
static volatile uint8_t btn2_raw_press = 0;
static volatile uint8_t btn3_raw_press = 0;
static volatile uint8_t btn4_raw_press = 0;
static volatile uint8_t btn5_raw_press = 0;

void Input_Init(void) {
    // GPIO and EXTI already initialized by MX_GPIO_Init() in main.c
    // Just reset the state
    current_input.btn2_pressed = 0;
    current_input.btn3_pressed = 0;
    current_input.btn4_pressed = 0;
    current_input.btn5_pressed = 0;
    current_input.btn5_held = 0;
    btn2_raw_press = 0;
    btn3_raw_press = 0;
    btn4_raw_press = 0;
    btn5_raw_press = 0;
}

void Input_Read(void) {
    // Copy the button press flags from interrupt to current input state
    // This is read once per frame by the main loop
    current_input.btn2_pressed = btn2_raw_press;
    current_input.btn3_pressed = btn3_raw_press;
    current_input.btn4_pressed = btn4_raw_press;
    current_input.btn5_pressed = btn5_raw_press;
    current_input.btn5_held = (HAL_GPIO_ReadPin(BTN5_GPIO_Port, BTN5_Pin) == GPIO_PIN_RESET);
    
    // Reset the flags after reading so they only trigger once
    btn2_raw_press = 0;
    btn3_raw_press = 0;
    btn4_raw_press = 0;
    btn5_raw_press = 0;
}

// ===== INTERRUPT CALLBACK FOR BUTTONS =====
// Called by hardware when button is pressed
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t last_btn2_interrupt = 0;
    static uint32_t last_btn3_interrupt = 0;
    static uint32_t last_btn4_interrupt = 0;
    static uint32_t last_btn5_interrupt = 0;
    uint32_t current_time = HAL_GetTick();
    
    // Handle BT2
    if (GPIO_Pin == BTN2_Pin) {
        // Software debouncing (200ms) Jackp: dropped this to 50ms as button is unresponsive
        if ((current_time - last_btn2_interrupt) > 50) {
            last_btn2_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn2_raw_press = 1;
        }
    }
    
    // Handle BT3 (joystick button)
    if (GPIO_Pin == BTN3_Pin) {
        // Software debouncing (200ms)
        if ((current_time - last_btn3_interrupt) > 200) {
            last_btn3_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn3_raw_press = 1;
        }
    }

    // Handle BT4
    if (GPIO_Pin == BTN4_Pin) {
        // Software debouncing (50ms)
        if ((current_time - last_btn4_interrupt) > 50) {
            last_btn4_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn4_raw_press = 1;
        }
    }

    // Handle BT5
    if (GPIO_Pin == BTN5_Pin) {
        // Software debouncing (50ms)
        if ((current_time - last_btn5_interrupt) > 50) {
            last_btn5_interrupt = current_time;
            
            // Toggle LED to indicate button press
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            
            // Set flag indicating button was pressed
            btn5_raw_press = 1;
        }
    }
}
