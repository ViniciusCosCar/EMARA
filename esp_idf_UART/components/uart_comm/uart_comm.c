#include "uart_comm.h"
#include "driver/uart.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

const uart_port_t uart_num = UART_NUM_1; //?

const int uart_buffer_size = (1024 * 2); //?

QueueHandle_t uart_queue; //?

uart_config_t uart_config = 
{
  .baud_rate = 115200, //Number of bits transmitted per second
  .data_bits = UART_DATA_8_BITS, //How many bits are transmitted
  .parity = UART_PARITY_DISABLE, //?
  .stop_bits = UART_STOP_BITS_1, //?
  .flow_ctrl =  UART_HW_FLOWCTRL_DISABLE, //?
  .rx_flow_ctrl_thresh = 122, //?
};

esp_err_t uart_comm_init(void)  //verify if a error happens
{
  ESP_ERROR_CHECK(uart_param_config(uart_num,&uart_config));
  ESP_ERROR_CHECK(uart_driver_install(uart_num,uart_buffer_size,uart_buffer_size,10,&uart_queue,0));
  return ESP_OK; 
}
