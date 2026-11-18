#pragma once
#include "trajectory.hpp"
#include <cstdint>

namespace Packet
{
// ==============================================================================================================================
// constants
// ==============================================================================================================================
static constexpr uint8_t MAX_PACKET_SIZE = 32;
static constexpr uint8_t MAX_CHUNK_SIZE  = MAX_PACKET_SIZE - 4; // - trajectory chunk headeer

// ==============================================================================================================================
// communication ID
// ==============================================================================================================================
typedef uint8_t packet_id_t;
extern packet_id_t current_communication_id;
inline void update_current_communication_id() { current_communication_id++; } // yes, it's supposed to overflow

// ==============================================================================================================================
// packet types
// ==============================================================================================================================
typedef enum : uint8_t

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

// ==============================================================================================================================
// packet class
// ==============================================================================================================================
class packet_t
{
      protected:
        packet_type_t packet_type;

      public:
        packet_t(packet_type_t);
        virtual ~packet_t() = default;

        packet_type_t get_type() const { return packet_type; }
        virtual size_t serialize(uint8_t* buf, size_t buf_size) const = 0;
        virtual bool deserialize(const uint8_t* buf, size_t buf_size) = 0;
        virtual void print() const                                    = 0;
};
// ------------------------------------------------------------------------------------------------------------------------------
// telemetry:
// ------------------------------------------------------------------------------------------------------------------------------
class telemetry_packet_t : public packet_t
{
      protected:
        Trajectory::angle_int32_t lat;
        Trajectory::angle_int32_t lng;

      public:
        telemetry_packet_t(Trajectory::angle_int32_t, Trajectory::angle_int32_t);
        Trajectory::angle_int32_t get_lat() const;
        Trajectory::angle_int32_t get_lng() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
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
                                      uint8_t nchunks);

        packet_id_t get_communication_id();
        Trajectory::trajectory_following_mode_t get_following_mode() const;
        uint8_t get_nchunks() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_intention_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_intention_ok_packet_t(packet_id_t communication_id);
        packet_id_t get_communication_id() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// trajectory chunk
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_chunk_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        uint8_t chunk_n;
        uint8_t chunk[MAX_CHUNK_SIZE];
        uint8_t chunk_size;

      public:
        trajectory_chunk_packet_t(packet_id_t communication_id, uint8_t chunk_n, const uint8_t* data, size_t data_size);
        packet_id_t get_communication_id() const;
        uint8_t get_chunk_n() const;
        size_t get_chunk_size() const;
        const uint8_t* get_chunk() const;

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_chunk_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;
        uint8_t chunk_n;

      public:
        trajectory_chunk_ok_packet_t(packet_id_t communication_id, uint8_t chunk_n);
        packet_id_t get_communication_id() const;
        uint8_t get_chunk_n() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// trajectory flashed
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_flashed_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_flashed_packet_t(packet_id_t communication_id);
        packet_id_t get_communication_id() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class trajectory_flashed_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_flashed_ok_packet_t(packet_id_t communication_id);
        packet_id_t get_communication_id() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
// communication failure
// ------------------------------------------------------------------------------------------------------------------------------
class communication_failure_packet_t : public packet_t
{
      private:
      protected:
        packet_id_t communication_id;
        uint8_t error_code;

      public:
        communication_failure_packet_t(packet_id_t communication_id, uint8_t error_code);
        packet_id_t get_communication_id() const;
        uint8_t get_error_code() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
// ------------------------------------------------------------------------------------------------------------------------------
class communication_failure_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        communication_failure_ok_packet_t(packet_id_t communication_id);
        packet_id_t get_communication_id() const;
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};

// ==============================================================================================================================
// general functions
// ==============================================================================================================================
void send_packet(packet_t*);   // send packet
packet_t* listen_for_packet(); // blocking function that returns a ref. to the received packet

} // namespace Packet
