#include "uart_metal_isr.h"

// doubleton, 
UART_METAL_ISR* UART_METAL_ISR::instances[2] = {nullptr, nullptr};

UART_METAL_ISR::UART_METAL_ISR(uart_inst_t* _uart_instance, uint32_t _rx_pin, uint32_t _tx_pin){
  //
  uart = _uart_instance;
  rx_pin = _rx_pin; 
  tx_pin = _tx_pin; 
  // for that routing step, 
  if(uart == uart0){
    instances[0] = this; 
    uart_irq_num = UART0_IRQ;
  } else {
    instances[1] = this;
    uart_irq_num = UART1_IRQ; 
  }
}

void UART_METAL_ISR::begin(uint32_t baudrate){
  // ... lol ?
  uart_init(uart, baudrate);

  // 
  gpio_set_function(rx_pin, GPIO_FUNC_UART);
  gpio_set_function(tx_pin, GPIO_FUNC_UART);

  // use cts, rts ? 
  uart_set_hw_flow(uart, false, false);

  // 8N1
  uart_set_format(uart, 8, 1, UART_PARITY_NONE);

  // yes fifo 
  uart_set_fifo_enabled(uart, true); 

  // make one-such
  critical_section_init(&isr_crit_sec);

  // use IRQs... route thru static funcs 
  irq_set_exclusive_handler(uart_irq_num, (uart == uart0) ? uart_metal_irq_router_0 : uart_metal_irq_router_1);

  // enable interrupts, (rx, tx=false, init when needed) 
  uart_set_irq_enables(uart, true, false);
  irq_set_enabled(uart_irq_num, true); 
} 

// not tested, but should do:
void UART_METAL_ISR::end(void){
  irq_set_enabled(uart_irq_num, false);
  uart_set_irq_enables(uart, false, false);
  uart_deinit(uart);
}

size_t UART_METAL_ISR::available(void){
  // how many bytes to read ? use snapshots, 
  size_t head = rx_head;
  size_t tail = rx_tail; 
  return (head - tail) & (UART_METAL_ISR_RB_SIZE - 1);
}

uint8_t UART_METAL_ISR::read(void){
  // if no data 
  if(rx_tail == rx_head) return 0; 
  // isr block not req. because ISR only modifies head, 
  uint8_t data = rx_buffer[rx_tail & (UART_METAL_ISR_RB_SIZE - 1)];
  rx_tail ++; // no need to wrap, use uint32 overflow + wrap with mask  
  // sendy 
  return data; 
}

uint8_t UART_METAL_ISR::read_unsafe(void){
  // same in this impl, 
  return read();
}

size_t UART_METAL_ISR::availableForWrite(void){
  // snapshot, 
  size_t head = tx_head;
  size_t tail = tx_tail; 
  // 
  size_t used = (head - tail) & (UART_METAL_ISR_RB_SIZE - 1);
  // return UART_METAL_ISR_RB_SIZE - 1 - used;
  return UART_METAL_ISR_RB_SIZE - 1 - used;
}

void UART_METAL_ISR::write(uint8_t* data, size_t len){
  // no stuff when underfull 
  if(availableForWrite() < len) return; 

  // do safety (bad pattern: make and init crit_sec only once, bad robot) 
  // critical_section_t crit_sec; 
  // critical_section_init(&crit_sec);
  // critical_section_enter_blocking(&crit_sec);

  // stuff tx buff w/ as many bytes, 
  for(size_t i = 0; i < len; i ++){
    tx_buffer[tx_head & (UART_METAL_ISR_RB_SIZE - 1)] = data[i]; 
    tx_head ++; 
  }

  // and ship as many as possible to kick ISR, 
  // ISR will only fire if we have stuffed more than fit in the FIFO (half?)
  // this is fine: everything will make its way out, *or* if they pile up, we get the ISR to continue 
  // and we maintain order because we are stuffing from the tail, 

  // but, when we write tail, we can't also have the isr writing tail, so: 
  critical_section_enter_blocking(&isr_crit_sec);
  uart_hw_t* hw = uart_get_hw(uart);
  while(uart_is_writable(uart) && tx_tail != tx_head){
    uint8_t data = tx_buffer[tx_tail & (UART_METAL_ISR_RB_SIZE - 1)];
    tx_tail ++;
    uart_putc_raw(uart, data);
  }
  critical_section_exit(&isr_crit_sec);

  // reset tx isr, 
  uart_set_irq_enables(uart, true, true);

  // unlock 
  // critical_section_exit(&crit_sec);
}

// handler 
void __time_critical_func(UART_METAL_ISR::uart_metal_isr_irq_handler)(void){
  // get flags, 
  uart_hw_t *hw = uart_get_hw(uart);

  // check rx 
  uint32_t mis = hw->mis; // masked interrupt status register 
  if(mis & (UART_UARTMIS_RXMIS_BITS | UART_UARTMIS_RTMIS_BITS)){
    while(uart_is_readable(uart)){
      size_t next_head = rx_head + 1; 
      if((next_head & (UART_METAL_ISR_RB_SIZE - 1)) == (rx_tail & (UART_METAL_ISR_RB_SIZE - 1))){
        // rx buffer full, read & break, 
        uart_getc(uart);
        break; 
      }

      uint8_t data = uart_getc(uart);
      rx_buffer[rx_head & (UART_METAL_ISR_RB_SIZE - 1)] = data; 
      rx_head ++; 
    }

    // clear rx timeout if that fired, 
    if(mis & UART_UARTMIS_RTMIS_BITS){
      hw->icr = UART_UARTICR_RTIC_BITS;
    }

  }

  // tx fifo, 
  mis = hw->mis; // read again ? 
  if(mis & UART_UARTMIS_TXMIS_BITS){
    while(uart_is_writable(uart)){
      if(tx_tail == tx_head){
        // stop tx isr, 
        uart_set_irq_enables(uart, true, false); 
        break; 
      }

      uart_putc_raw(uart, tx_buffer[tx_tail & (UART_METAL_ISR_RB_SIZE - 1)]);
      tx_tail ++; 
    }
  }

  // clear various flags... 
  if(mis & UART_UARTMIS_BEMIS_BITS){
    hw->icr = UART_UARTICR_BEIC_BITS;
  }

  if(mis & UART_UARTMIS_PEMIS_BITS){
    hw->icr = UART_UARTICR_PEIC_BITS; 
  }

  if(mis & UART_UARTMIS_FEMIS_BITS){
    hw->icr = UART_UARTICR_FEIC_BITS;
  }

}

// routers 
void __time_critical_func(UART_METAL_ISR::uart_metal_irq_router_0)(void){
  if(instances[0]) instances[0]->uart_metal_isr_irq_handler();
}

void __time_critical_func(UART_METAL_ISR::uart_metal_irq_router_1)(void){
  if(instances[1]) instances[1]->uart_metal_isr_irq_handler();  
}
