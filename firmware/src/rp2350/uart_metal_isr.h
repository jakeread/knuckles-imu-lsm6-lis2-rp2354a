#ifndef UART_METAL_ISR_H_
#define UART_METAL_ISR_H_ 

#include <Arduino.h>
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/regs/uart.h"

// got 2 be power of 2, 
#define UART_METAL_ISR_RB_SIZE 1024 

class UART_METAL_ISR {
  public:
    UART_METAL_ISR(uart_inst_t* _uart_instance, uint32_t _rx_pin, uint32_t _tx_pin);
    // start stop 
    void begin(uint32_t baudrate);
    void end(void);

    // read api 
    size_t available(void);
    uint8_t read(void);
    uint8_t read_unsafe(void);

    // write api 
    size_t availableForWrite(void);
    void write(uint8_t* data, size_t len);

    // handler
    void uart_metal_isr_irq_handler(void);

  static void uart_metal_irq_router_0(void);
  static void uart_metal_irq_router_1(void);

  private:
    // hardware 
    uart_inst_t* uart; 
    uint32_t uart_irq_num;
    uint32_t rx_pin;
    uint32_t tx_pin; 
    // blocker, 
    critical_section_t isr_crit_sec;
    // le buf
    uint8_t rx_buffer[UART_METAL_ISR_RB_SIZE]; 
    volatile size_t rx_head = 0; // receive into 
    volatile size_t rx_tail = 0; // read from 
    uint8_t tx_buffer[UART_METAL_ISR_RB_SIZE];
    volatile size_t tx_head = 0; // write into 
    volatile size_t tx_tail = 0; // transmit from 

    // use routing to static member funcs for ISRs
    static UART_METAL_ISR* instances[2];
};

#endif 