
#include "packet.hpp"
#include "LoRa.hpp"
#include "trajectory.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

using namespace Packet;

packet_id_t Packet::current_communication_id = 0;

// --------------------------------------------------------
// helpers
// --------------------------------------------------------
// serialization helper
template <typename T>
bool write_field(uint8_t* buf, size_t buf_size, size_t& index, const T& value)
{
        if (index > buf_size || sizeof(T) > buf_size - index) return false;

        memcpy(&buf[index], &value, sizeof(T));
        index += sizeof(T);

        return true;
}
// deserialization helper
template <typename T>
bool read_field(const uint8_t* buf, size_t buf_size, size_t& index, T& value)
{
        if (index > buf_size || sizeof(T) > buf_size - index) return false;

        memcpy(&value, &buf[index], sizeof(T));
        index += sizeof(T);

        return true;
}
// general
bool valid_packet_type(const uint8_t* buf, size_t buf_size, packet_type_t expected)
{
        return buf != nullptr && buf_size > 0 && buf[0] == static_cast<uint8_t>(expected);
}
const char* packet_type_to_string(packet_type_t type)
{
        switch (type)
        {
        case TELEMETRY:
                return "TELEMETRY";
        case TRAJECTORY_INTENTION:
                return "TRAJECTORY_INTENTION";
        case TRAJECTORY_INTENTION_OK:
                return "TRAJECTORY_INTENTION_OK";
        case TRAJECTORY_CHUNK:
                return "TRAJECTORY_CHUNK";
        case TRAJECTORY_CHUNK_OK:
                return "TRAJECTORY_CHUNK_OK";
        case TRAJECTORY_FLASHED:
                return "TRAJECTORY_FLASHED";
        case TRAJECTORY_FLASHED_OK:
                return "TRAJECTORY_FLASHED_OK";
        case COMMUNICATION_FAILURE:
                return "COMMUNICATION_FAILURE";
        case COMMUNICATION_FAILURE_OK:
                return "COMMUNICATION_FAILURE_OK";
        default:
                return "UNKNOWN";
        }
}
const char* trajectory_mode_to_string(Trajectory::trajectory_following_mode_t mode)
{
        switch (mode)
        {
        case Trajectory::Patrol:
                return "Patrol";
        case Trajectory::Hold:
                return "Hold";
        case Trajectory::Move:
                return "Move";
        default:
                return "Unknown";
        }
}

// --------------------------------------------------------
// packet classes
// --------------------------------------------------------
packet_t* packet_t::from_buffer(const uint8_t* buf, size_t bufsize)
{
        if (!buf || bufsize == 0) return nullptr;

        packet_t* pk = nullptr;

        switch (static_cast<packet_type_t>(buf[0]))
        {
        case TELEMETRY:
                pk = new telemetry_packet_t();
                break;
        case TRAJECTORY_INTENTION:
                pk = new trajectory_intention_packet_t();
                break;
        case TRAJECTORY_INTENTION_OK:
                pk = new trajectory_intention_ok_packet_t();
                break;
        case TRAJECTORY_CHUNK:
                pk = new trajectory_chunk_packet_t();
                break;
        case TRAJECTORY_CHUNK_OK:
                pk = new trajectory_chunk_ok_packet_t();
                break;
        case TRAJECTORY_FLASHED:
                pk = new trajectory_flashed_packet_t();
                break;
        case TRAJECTORY_FLASHED_OK:
                pk = new trajectory_flashed_ok_packet_t();
                break;
        case COMMUNICATION_FAILURE:
                pk = new communication_failure_packet_t();
                break;
        case COMMUNICATION_FAILURE_OK:
                pk = new communication_failure_ok_packet_t();
                break;
        default:
                return nullptr;
        }

        if (!pk->deserialize(buf, bufsize))
        {
                delete pk;
                return nullptr;
        }

        return pk;
}
//
// telemetry
//
// serialize
size_t telemetry_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(lat) + sizeof(lng);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        const uint8_t type = static_cast<uint8_t>(packet_type);

        write_field(buf, buf_size, index, type);
        write_field(buf, buf_size, index, lat);
        write_field(buf, buf_size, index, lng);

        return index;
}
// deserialize
bool telemetry_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(lat) + sizeof(lng);
        if (!valid_packet_type(buf, buf_size, TELEMETRY) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);

        Trajectory::angle_int32_t new_lat;
        Trajectory::angle_int32_t new_lng;

        if (!read_field(buf, buf_size, index, new_lat) || !read_field(buf, buf_size, index, new_lng)) return false;

        lat = new_lat;
        lng = new_lng;

        return true;
}
// print
void telemetry_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Latitude:  %.7f\n", static_cast<double>(lat) / 1e7);
        printf("  Longitude: %.7f\n", static_cast<double>(lng) / 1e7);
}

//
// trajectory intention
//
// serialize
size_t trajectory_intention_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes =
            sizeof(packet_type) + sizeof(communication_id) + sizeof(trajectory_following_mode) + sizeof(nchunks);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);
        write_field(buf, buf_size, index, trajectory_following_mode);
        write_field(buf, buf_size, index, nchunks);

        return index;
}
// deserialize
bool trajectory_intention_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes =
            sizeof(packet_type) + sizeof(communication_id) + sizeof(trajectory_following_mode) + sizeof(nchunks);

        if (!valid_packet_type(buf, buf_size, TRAJECTORY_INTENTION) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);

        packet_id_t new_communication_id;
        Trajectory::trajectory_following_mode_t new_trajectory_following_mode;
        uint8_t new_nchunks;

        if (!read_field(buf, buf_size, index, new_communication_id) ||
            !read_field(buf, buf_size, index, new_trajectory_following_mode) ||
            !read_field(buf, buf_size, index, new_nchunks))
                return false;

        communication_id          = new_communication_id;
        trajectory_following_mode = new_trajectory_following_mode;
        nchunks                   = new_nchunks;

        return true;
}
// print
void trajectory_intention_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", (communication_id));
        printf("  Following mode:   %s\n", trajectory_mode_to_string(trajectory_following_mode));
        printf("  Number of chunks: %u\n", nchunks);
}
//
// trajectory intention ok
//
// serialize
size_t trajectory_intention_ok_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);

        return index;
}
// deserialize
bool trajectory_intention_ok_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);
        if (!valid_packet_type(buf, buf_size, TRAJECTORY_INTENTION_OK) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);
        packet_id_t new_communication_id;

        if (!read_field(buf, buf_size, index, new_communication_id)) return false;

        communication_id = new_communication_id;

        return true;
}
// print
void trajectory_intention_ok_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
}
//
// trajectory chunk
//
trajectory_chunk_packet_t::trajectory_chunk_packet_t(packet_id_t communication_id, uint8_t chunk_n, const uint8_t* data,
                                                     const size_t data_size)
    : packet_t(TRAJECTORY_CHUNK), communication_id(communication_id), chunk_n(chunk_n)
{
        if (data != nullptr)
        {
                chunk_size = (data_size > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : data_size;
                memcpy(chunk, data, chunk_size);
        }
        else
        {
                chunk_size = 0;
        }
}

// serialize
size_t trajectory_chunk_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes =
            sizeof(packet_type) + sizeof(communication_id) + sizeof(chunk_n) + sizeof(chunk_size) + chunk_size;

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);
        write_field(buf, buf_size, index, chunk_n);
        write_field(buf, buf_size, index, chunk_size);

        memcpy(&buf[index], chunk, chunk_size);
        index += chunk_size;

        return index;
}
// deserialize
bool trajectory_chunk_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t min_n_bytes =
            sizeof(packet_type) + sizeof(communication_id) + sizeof(chunk_n) + sizeof(chunk_size);
        if (!valid_packet_type(buf, buf_size, TRAJECTORY_CHUNK) || buf_size < min_n_bytes) return false;

        size_t index = sizeof(packet_type);

        packet_id_t new_communication_id;
        uint8_t new_chunk_n;
        uint8_t new_chunk_size;

        if (!read_field(buf, buf_size, index, new_communication_id) || !read_field(buf, buf_size, index, new_chunk_n) ||
            !read_field(buf, buf_size, index, new_chunk_size))
                return false;

        if (new_chunk_size > MAX_CHUNK_SIZE || buf_size != index + new_chunk_size) return false;

        communication_id = new_communication_id;
        chunk_n          = new_chunk_n;
        chunk_size       = new_chunk_size;

        memcpy(chunk, &buf[index], new_chunk_size);

        return true;
}
// print
void trajectory_chunk_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
        printf("  Chunk number:     %u\n", chunk_n);
        printf("  Chunk size:       %u bytes\n", chunk_size);
        printf("  Chunk data:       ");

        for (size_t i = 0; i < chunk_size; i++)
        {
                printf("%02X ", static_cast<unsigned>(chunk[i]));
        }

        printf("\n");
}

//
// trajectory chunk ok
//
// serialize
size_t trajectory_chunk_ok_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id) + sizeof(chunk_n);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);
        write_field(buf, buf_size, index, chunk_n);

        return index;
}
// deserialize
bool trajectory_chunk_ok_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id) + sizeof(chunk_n);

        if (!valid_packet_type(buf, buf_size, TRAJECTORY_CHUNK_OK) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);

        packet_id_t new_communication_id;
        uint8_t new_chunk_n;

        if (!read_field(buf, buf_size, index, new_communication_id) || !read_field(buf, buf_size, index, new_chunk_n))
                return false;

        communication_id = new_communication_id;
        chunk_n          = new_chunk_n;

        return true;
}
// print
void trajectory_chunk_ok_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
        printf("  Chunk number:     %u\n", chunk_n);
}

//
// trajectory flashed
//
// serialize
size_t trajectory_flashed_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);

        return index;
}
// deserialize
bool trajectory_flashed_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);
        if (!valid_packet_type(buf, buf_size, TRAJECTORY_FLASHED) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);
        packet_id_t new_communication_id;

        if (!read_field(buf, buf_size, index, new_communication_id)) return false;

        communication_id = new_communication_id;

        return true;
}
// print
void trajectory_flashed_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
}
//
// trajectory flashed ok
//
// serialize
size_t trajectory_flashed_ok_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        const uint8_t type = static_cast<uint8_t>(packet_type);

        write_field(buf, buf_size, index, type);
        write_field(buf, buf_size, index, communication_id);

        return index;
}
// deserialize
bool trajectory_flashed_ok_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);
        if (!valid_packet_type(buf, buf_size, TRAJECTORY_FLASHED_OK) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);
        packet_id_t new_communication_id;

        if (!read_field(buf, buf_size, index, new_communication_id)) return false;

        communication_id = new_communication_id;

        return true;
}
// print
void trajectory_flashed_ok_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
}
//
// communication failure
//
// serialize
size_t communication_failure_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id) + sizeof(error_code);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        write_field(buf, buf_size, index, packet_type);
        write_field(buf, buf_size, index, communication_id);
        write_field(buf, buf_size, index, error_code);

        return index;
}
// deserialize
bool communication_failure_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id) + sizeof(error_code);
        if (!valid_packet_type(buf, buf_size, COMMUNICATION_FAILURE) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);

        packet_id_t new_communication_id;
        uint8_t new_error_code;

        if (!read_field(buf, buf_size, index, new_communication_id) ||
            !read_field(buf, buf_size, index, new_error_code))
                return false;

        communication_id = new_communication_id;
        error_code       = new_error_code;

        return true;
}
// print
void communication_failure_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
        printf("  Error code:        %u\n", error_code);
}
//
// communication failure ok
//
// serialize
size_t communication_failure_ok_packet_t::serialize(uint8_t* buf, size_t buf_size) const
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);

        if (buf == nullptr || buf_size < n_bytes) return 0;

        size_t index = 0;

        const uint8_t type = static_cast<uint8_t>(packet_type);

        write_field(buf, buf_size, index, type);
        write_field(buf, buf_size, index, communication_id);

        return index;
}
// deserialize
bool communication_failure_ok_packet_t::deserialize(const uint8_t* buf, size_t buf_size)
{
        const size_t n_bytes = sizeof(packet_type) + sizeof(communication_id);
        if (!valid_packet_type(buf, buf_size, COMMUNICATION_FAILURE_OK) || buf_size != n_bytes) return false;

        size_t index = sizeof(packet_type);
        packet_id_t new_communication_id;

        if (!read_field(buf, buf_size, index, new_communication_id)) return false;

        communication_id = new_communication_id;

        return true;
}
// print
void communication_failure_ok_packet_t::print() const
{
        printf("[PACKET] %s\n", packet_type_to_string(packet_type));
        printf("  Communication ID: %u\n", communication_id);
}
