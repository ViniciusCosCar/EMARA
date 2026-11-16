#pragma once
#include "trajectory.hpp"
#include <cstdint>

// ==============================================================================================================================
// Packet
// ==============================================================================================================================
namespace Packet
{
// ------------------------------------------------------------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------------------------------------------------------------
static constexpr uint8_t MAX_PACKET_SIZE = 32;
static constexpr uint8_t MAX_CHUNK_SIZE  = 29;
// ------------------------------------------------------------------------------------------------------------------------------
// communication ID
// ------------------------------------------------------------------------------------------------------------------------------
typedef uint8_t packet_id_t;
extern packet_id_t current_communication_id;
inline void update_current_communication_id() { current_communication_id++; } // yes, it's supposed to overflow
// ------------------------------------------------------------------------------------------------------------------------------
// packet types
// ------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
        TELEMETRY = 1,

        TRAJECTORY_INTENTION,
        TRAJECTORY_INTENTION_OK,

        TRAJECTORY_CHUNK,
        TRAJECTORY_CHUNK_OK,

        TRAJECTORY_FLASHED,
        TRAJECTORY_FLASHED_OK,

        COMMUNICATION_FAILURE,
        COMMUNICATION_FAILURE_OK,

} packet_type_t;
// ------------------------------------------------------------------------------------------------------------------------------
// packet abstract class
// ------------------------------------------------------------------------------------------------------------------------------
class packet_t
{
      protected:
        packet_type_t packet_type;

      public:
        packet_t(packet_type_t type) : packet_type(type) {}
        virtual ~packet_t() = default;

        packet_type_t get_type() const { return packet_type; }
        virtual char* stringify() = 0;
};
// ------------------------------------------------------------------------------------------------------------------------------
// telemetry
// ------------------------------------------------------------------------------------------------------------------------------
class telemetry_packet_t : public packet_t
{
      protected:
        Trajectory::angle_int32_t lat;
        Trajectory::angle_int32_t lng;

      public:
        telemetry_packet_t(Trajectory::angle_int32_t lat, Trajectory::angle_int32_t lng)
            : packet_t(TELEMETRY), lat(lat), lng(lng)
        {
        }
        Trajectory::angle_int32_t get_lat() const { return lat; }
        Trajectory::angle_int32_t get_lng() const { return lng; }
        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// trajectory intention
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_intention_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        Trajectory::trajectory_following_mode_t trajectory_following_mode;
        uint8_t nchunks;

      public:
        trajectory_intention_packet_t(packet_id_t communication_id,
                                      Trajectory::trajectory_following_mode_t trajectory_following_mode,
                                      uint8_t nchunks)
            : packet_t(TRAJECTORY_INTENTION), communication_id(communication_id),
              trajectory_following_mode(trajectory_following_mode), nchunks(nchunks)
        {
        }
        inline packet_id_t get_communication_id() { return communication_id; };
        inline Trajectory::trajectory_following_mode_t get_following_mode() const { return trajectory_following_mode; }
        uint8_t get_nchunks() const { return nchunks; }
        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_intention_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_intention_ok_packet_t(packet_id_t communication_id)
            : packet_t(TRAJECTORY_INTENTION_OK), communication_id(communication_id)
        {
        }
        inline packet_id_t get_communication_id() { return communication_id; };
        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// trajectory chunk
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_chunk_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        uint8_t chunk_n;
        uint8_t chunk_size;
        uint8_t chunk[MAX_CHUNK_SIZE];

      public:
        trajectory_chunk_packet_t(packet_id_t communication_id, uint8_t chunk_n, const uint8_t* data,
                                  uint8_t chunk_size)
            : packet_t(TRAJECTORY_CHUNK), communication_id(communication_id), chunk_n(chunk_n), chunk_size(chunk_size)
        {
                if (chunk_size > MAX_CHUNK_SIZE) this->chunk_size = MAX_CHUNK_SIZE;

                for (uint8_t i = 0; i < this->chunk_size; i++) chunk[i] = data[i];
        };

        packet_id_t get_communication_id() const { return communication_id; }
        uint8_t get_chunk_n() const { return chunk_n; }
        uint8_t get_chunk_size() const { return chunk_size; }
        const uint8_t* get_chunk() const { return chunk; }

        char* stringify() override;
};
class trajectory_chunk_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        uint8_t chunk_n;

      public:
        trajectory_chunk_ok_packet_t(packet_id_t communication_id, uint8_t chunk_n)
            : packet_t(TRAJECTORY_CHUNK_OK), communication_id(communication_id), chunk_n(chunk_n)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }
        uint8_t get_chunk_n() const { return chunk_n; }

        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// trajectory flashed
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_flashed_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_flashed_packet_t(packet_id_t communication_id)
            : packet_t(TRAJECTORY_FLASHED), communication_id(communication_id)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }

        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_flashed_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        explicit trajectory_flashed_ok_packet_t(packet_id_t communication_id)
            : packet_t(TRAJECTORY_FLASHED_OK), communication_id(communication_id)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }

        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// communication failure
// ------------------------------------------------------------------------------------------------------------------------------
class communication_failure_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        uint8_t error_code;

      public:
        communication_failure_packet_t(packet_id_t communication_id, uint8_t error_code)
            : packet_t(COMMUNICATION_FAILURE), communication_id(communication_id), error_code(error_code)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }
        uint8_t get_error_code() const { return error_code; }

        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class communication_failure_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        communication_failure_ok_packet_t(packet_id_t communication_id)
            : packet_t(COMMUNICATION_FAILURE_OK), communication_id(communication_id)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }

        char* stringify() override;
};
// ------------------------------------------------------------------------------------------------------------------------------
void send_packet(packet_t*);   // send packet
packet_t* listen_for_packet(); // blocking function that returns a ref. to the received packet
// ------------------------------------------------------------------------------------------------------------------------------
} // namespace Packet
