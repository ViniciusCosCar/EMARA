#pragma once

#include "trajectory.hpp"
#include <cstdint>

namespace Packet
{
// --------------------------------------------------------
// constants
// --------------------------------------------------------
static constexpr uint8_t MAX_PACKET_SIZE = 32;
static constexpr uint8_t MAX_CHUNK_SIZE  = MAX_PACKET_SIZE - 4; // - trajectory chunk headeer
// --------------------------------------------------------
// communication ID
// --------------------------------------------------------
typedef uint8_t packet_id_t;
extern packet_id_t current_communication_id;
inline void update_current_communication_id() { current_communication_id++; } // yes, it's supposed to overflow
// --------------------------------------------------------
// packet types
// --------------------------------------------------------
typedef enum : uint8_t

{
        TELEMETRY = 0,

        TRAJECTORY_INTENTION,
        TRAJECTORY_INTENTION_OK,

        TRAJECTORY_CHUNK,
        TRAJECTORY_CHUNK_OK,

        TRAJECTORY_FLASHED,
        TRAJECTORY_FLASHED_OK,

        COMMUNICATION_FAILURE,
        COMMUNICATION_FAILURE_OK,

        N_PACKET_TYPES

} packet_type_t;
// --------------------------------------------------------
// packet classes
// --------------------------------------------------------
//
// abstract packet class
//
class packet_t
{
      protected:
        packet_type_t packet_type;

      public:
        packet_t(packet_type_t packet_type) : packet_type(packet_type) {};
        virtual ~packet_t() = default;

        // return a pointer to the correct packet type
        static packet_t* from_buffer(const uint8_t* buf, size_t buf_size);

        packet_type_t get_type() const { return packet_type; }
        virtual size_t serialize(uint8_t* buf, size_t buf_size) const = 0;
        virtual bool deserialize(const uint8_t* buf, size_t buf_size) = 0;
        virtual void print() const                                    = 0;
};
//
// telemetry:
//
class telemetry_packet_t : public packet_t
{
      protected:
        Trajectory::angle_int32_t lat = 0;
        Trajectory::angle_int32_t lng = 0;

      public:
        telemetry_packet_t() : packet_t(TELEMETRY) {};
        telemetry_packet_t(Trajectory::angle_int32_t lat, Trajectory::angle_int32_t lng)
            : packet_t(TELEMETRY), lat(lat), lng(lng)
        {
        }
        Trajectory::angle_int32_t get_lat() const { return lat; }
        Trajectory::angle_int32_t get_lng() const { return lng; }
        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory intention
//
class trajectory_intention_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id                                      = 0;
        Trajectory::trajectory_following_mode_t trajectory_following_mode = Trajectory::Patrol;
        uint8_t nchunks                                                   = 0;

      public:
        trajectory_intention_packet_t() : packet_t(TRAJECTORY_INTENTION) {};
        trajectory_intention_packet_t(packet_id_t cid, Trajectory::trajectory_following_mode_t tfm, uint8_t nc)
            : packet_t(TRAJECTORY_INTENTION), communication_id(cid), trajectory_following_mode(tfm), nchunks(nc)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; };
        Trajectory::trajectory_following_mode_t get_following_mode() const { return trajectory_following_mode; };
        uint8_t get_nchunks() const { return nchunks; };

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory intention ok
//
class trajectory_intention_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id = 0;

      public:
        trajectory_intention_ok_packet_t() : packet_t(TRAJECTORY_INTENTION_OK) {};
        trajectory_intention_ok_packet_t(packet_id_t cid) : packet_t(TRAJECTORY_INTENTION_OK), communication_id(cid) {};

        packet_id_t get_communication_id() const { return communication_id; };

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory chunk
//
class trajectory_chunk_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id  = 0;
        uint8_t chunk_n               = 0;
        uint8_t chunk_size            = 0;
        uint8_t chunk[MAX_CHUNK_SIZE] = {0};

      public:
        trajectory_chunk_packet_t() : packet_t(TRAJECTORY_CHUNK) {};
        trajectory_chunk_packet_t(packet_id_t, uint8_t, const uint8_t*, size_t);

        packet_id_t get_communication_id() const { return communication_id; };
        uint8_t get_chunk_n() const { return chunk_n; };
        size_t get_chunk_size() const { return chunk_size; };
        const uint8_t* get_chunk() const { return chunk; }

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory chunk ok
//
class trajectory_chunk_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id = 0;
        uint8_t chunk_n              = 0;

      public:
        trajectory_chunk_ok_packet_t() : packet_t(TRAJECTORY_CHUNK_OK) {};
        trajectory_chunk_ok_packet_t(packet_id_t cid, uint8_t cn)
            : packet_t(TRAJECTORY_CHUNK_OK), communication_id(cid), chunk_n(cn)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }
        uint8_t packet_get_chunk_n() const { return chunk_n; }

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory flashed
//
class trajectory_flashed_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id = 0;

      public:
        trajectory_flashed_packet_t() : packet_t(TRAJECTORY_FLASHED) {};
        trajectory_flashed_packet_t(packet_id_t cid) : packet_t(TRAJECTORY_FLASHED), communication_id(cid) {}

        packet_id_t get_communication_id() const { return communication_id; };

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// trajectory flashed ok
//
class trajectory_flashed_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        trajectory_flashed_ok_packet_t() : packet_t(TRAJECTORY_FLASHED_OK) {}
        trajectory_flashed_ok_packet_t(packet_id_t communication_id)
            : packet_t(TRAJECTORY_FLASHED_OK), communication_id(communication_id) {};

        packet_id_t get_communication_id() const { return communication_id; }

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// communication failure
//
class communication_failure_packet_t : public packet_t
{
      private:
      protected:
        packet_id_t communication_id;
        uint8_t error_code;

      public:
        communication_failure_packet_t() : packet_t(COMMUNICATION_FAILURE) {}
        communication_failure_packet_t(packet_id_t cid, uint8_t rc)
            : packet_t(COMMUNICATION_FAILURE), communication_id(cid), error_code(rc)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }
        uint8_t get_error_code() const { return error_code; }

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
//
// communication failure ok
//
class communication_failure_ok_packet_t : public packet_t
{
      protected:
        packet_id_t communication_id;

      public:
        communication_failure_ok_packet_t() : packet_t(COMMUNICATION_FAILURE_OK) {}
        communication_failure_ok_packet_t(packet_id_t communication_id)
            : packet_t(COMMUNICATION_FAILURE_OK), communication_id(communication_id)
        {
        }

        packet_id_t get_communication_id() const { return communication_id; }

        size_t serialize(uint8_t* buf, size_t buf_size) const override;
        bool deserialize(const uint8_t* buf, size_t buf_size) override;
        void print() const override;
};
} // namespace Packet
