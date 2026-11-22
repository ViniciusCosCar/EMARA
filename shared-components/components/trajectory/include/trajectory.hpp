#pragma once

#include <cstdint>
// ==============================================================================================================================
// Trajectory
// ==============================================================================================================================
namespace Trajectory
{
// ------------------------------------------------------------------------------------------------------------------------------
typedef double angle_t;
typedef int32_t angle_int32_t;

// https://github.com/mytechnotalent/meshcore-bare/blob/main/textbook/17_Chapter_17_GPS_and_Location.md
inline angle_int32_t get_angle_int32_t(angle_t angle) { return static_cast<angle_int32_t>(angle * 1e7); } // 481173000
// ------------------------------------------------------------------------------------------------------------------------------
typedef enum : uint8_t
{
        Patrol = 0,
        Hold   = 1,
        Move   = 2,
} trajectory_following_mode_t;
typedef struct
{
        angle_t lat;
        angle_t lng;
} waypoint_t;
typedef struct
{
        waypoint_t* waypoints;
        uint8_t n_waypoints;
        trajectory_following_mode_t trajectory_mode;
} trajectory_t;

} // namespace Trajectory
