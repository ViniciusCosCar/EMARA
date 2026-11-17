#include <cstdint>
#include <functional>

// ------------------------------------------------------
// --- on ESP32-web ---
STATE_SEND_TRAJECTORY:
{
        packet_t rx_pk = listen_for_packet();
        if(rx_pk.type == TELEMETRY) {
                packet_t trajectory_intention = trajectory_intention_t();
                current_communication_id = trajectory_intention.communication_id;
                send_packet(trajectory_intention.stringify());
        }
        else if (rx_pk.type == TRAJECTORY_INTENTION_OK){
                if(rx_pk.communication_id is valid and == current_communication_id) {

                        system_state = STATE_SEND_TRAJECTORY_CHUNK;
                }
        }
}
// --- on ESP32-boat ---
STATE_TELEMETRY:
{
    send_packet(telemetry_pk.stringify());
    packet_t rx_pk = listen_for_packet();

    // if receive trajectory_intention
    if (rx_pk.type == TRAJECTORY_INTENTION) {
            if (rx_pk.communication_id is valid) {

                    // register and create packet with the same communication_id
                    current_communication_id = rx_pk.communication_id;
                    packet_t trajectory_intention_ok = trajectory_intention_ok(packet);

                    send_packet(trajectory_intention_ok.stringify());
                    system_state = STATE_TRAJECTORY_RECEPTION;
            }
    }
}
// ------------------------------------------------------
// ================================================= //
// BAD DAY SCENARIO: lost packets
// ================================================= //
bool TX_wait(packet tx_pk,system_state &current_state, std::function<bool(packet)> compare,
        std::function<void(system_state)> handle, uint8_t max_tries, uint8_t max_wrong_packets)
{
        // tries max_tries times
        for(uint8_t retries = 0; retries < max_tries; retries++)
        {
                TX(tx_pk);
                
                // receives max_wrong_packets
                for(uint8_t wrong_packets = 0; wrong_packets < max_wrong_packets; wrong_packets++)
                {
                        if (compare(RX())) return true;
                        // this function should answer the following packets:
                        // is this the desired packet? Yes or No?
                        // what happens if packet A was received? What if it was packet B?
                        // what if the received packet has a different communication id?
                }
        }
        return false;
}
