// web system handler .cpp

#include "esp_event.h"
#include "freertos/FreeRTOS.h"

#include "LoRa.hpp"
#include "packet.hpp"
#include "pins_arduino.h"
#include "system_handler.hpp"
#include "trajectory.hpp"

using namespace Packet;
using namespace Trajectory;

// logging tag
static const char* TAG = "system handler";

TaskHandle_t system_handle = nullptr;

// initialize extern variables
volatile bool rx_mode            = true;
volatile bool tx_mode            = false;
uint8_t pk_data[MAX_PACKET_SIZE] = {0};

enum : uint8_t
{
        WAIT = 0, // update dashboard when LORA_PACKET_RECEIVED & TELEMETRY; goto next state when user defines
                  // trajectory on dashboard

        START_COMMUNICATION, // TX(intention) until RX(intention_ok) with the same communication_id

        TX_DATA_CHUNK,     // TX(c0)----------TX(p1)----------...TX(pn)
        RX_DATA_CHUNK_RES, // ------RX(p0_res)------RX(p1_res)...------RX(pn)->goto NEXT STATE

        FINISH_COMMUNICATION, // wait until receive boat's trajectory flashing status report, and determine
                              // the next state based on the trasaction' success or failure

} system_state;

void update_dashboard(angle_int32_t, angle_int32_t);
void system_handler(void*)
{
        BaseType_t result;
        uint32_t r_value;

        while (true)
        {
                // block task until notified, then returns pdPASS. Otherwise, in case of a timeout, returns pdFAIL.
                result = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_value, portMAX_DELAY);

                if (result == pdPASS)
                {
                        // TODO: What happens if the ISR runs in the middle of this process?
                        if (r_value & LORA_PACKET_RECEIVED)
                        {
                                packet_t* ppk = packet_t::from_buffer(pk_data, MAX_PACKET_SIZE);
                                // yes, since the callback on packet's reception only writes the datasize received, it
                                // does not make much sense to pass MAX_PACKET_SIZE in here. However, wewould need
                                // another global variable to store the datasize, and MAX_PACKET_SIZE also doesn't make
                                // any difference inside from_buffer(), since the number of bytes to read is mostly
                                // determined by the packet type, being the buffer size only an upperlimit

                                if (!ppk) break;

                                switch (system_state)
                                {
                                case WAIT:
                                {
                                        if (ppk->get_type() == TELEMETRY)
                                        {
                                                telemetry_packet_t* ptk = static_cast<telemetry_packet_t*>(ppk);
                                                update_dashboard(ptk->get_lat(), ptk->get_lng());
                                                break;
                                        } // else

                                        break;
                                }
                                case START_COMMUNICATION:
                                {
                                }
                                case TX_DATA_CHUNK:
                                case RX_DATA_CHUNK_RES:
                                case FINISH_COMMUNICATION:
                                }
                        }
                        if (r_value & LORA_PACKET_RECEIVED_BUT_ERROR)
                        {
                                if (r_value & (LORA_PACKET_RECEIVED_BUT_ERROR | FAILURE_RC_OVERFLOW))
                                {
                                        ...
                                }

                                if (r_value & (LORA_PACKET_RECEIVED_BUT_ERROR | FAILURE_RC_UNKNOWN))
                                {
                                        ...
                                }
                        }
                        if (r_value & TRAJECTORY_DEFINED)
                        {
                                ...
                        }
                }
        }
