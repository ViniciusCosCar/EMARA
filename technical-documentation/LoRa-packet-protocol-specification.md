# LoRa Packet Protocol Specification

This document outlines the binary structure, data types, and byte allocations for the LoRa communication protocol defined in the `Trajectory` and `Packet` namespaces.

## 1. Global Constants & Base Types

* **Maximum Packet Size (`MAX_PACKET_SIZE`):** $32\text{ bytes}$
* **Maximum Chunk Payload Size (`MAX_CHUNK_SIZE`):** $28\text{ bytes}$ ($32 - 4\text{ bytes}$ reserved for the trajectory chunk header)

### Data Type Reference
| Type Alias / Enum | Underlying C++ Type | Size (Bytes) | Description |
| :--- | :--- | :---: | :--- |
| `packet_type_t` | `enum : uint8_t` | $1$ | Identifies the packet type (always Byte `0`). |
| `packet_id_t` | `uint8_t` | $1$ | Rolling communication session ID (`0`–`255`). |
| `angle_int32_t` | `int32_t` | $4$ | Fixed-point GNSS coordinate ($\text{angle} \times 10^7$). |
| `trajectory_following_mode_t` | `enum : uint8_t` | $1$ | `0`: Patrol, `1`: Hold, `2`: Move. |

---

## 2. Packet Type Identifiers (`packet_type_t`)

Every serialized packet begins with a $1\text{-byte}$ header indicating its type:

| Value | Enum Identifier | Description |
| :---: | :--- | :--- |
| `0` | `TELEMETRY` | Periodic GNSS location report. |
| `1` | `TRAJECTORY_INTENTION` | Initiates a new trajectory upload session. |
| `2` | `TRAJECTORY_INTENTION_OK` | Acknowledges trajectory upload initiation. |
| `3` | `TRAJECTORY_CHUNK` | Transmits a segment of waypoint data. |
| `4` | `TRAJECTORY_CHUNK_OK` | Acknowledges receipt of a specific data chunk. |
| `5` | `TRAJECTORY_FLASHED` | Confirms the complete trajectory has been saved/flashed. |
| `6` | `TRAJECTORY_FLASHED_OK` | Acknowledges the flash confirmation. |
| `7` | `COMMUNICATION_FAILURE` | Reports an error during transmission. |
| `8` | `COMMUNICATION_FAILURE_OK` | Acknowledges the failure report. |

---

## 3. Detailed Packet Structures

### 3.1. Telemetry Packet (`telemetry_packet_t`)
Used to transmit compressed GNSS coordinates.
* **Total Size:** $9\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `0` (`TELEMETRY`) |
| `1..4` | `lat` | `angle_int32_t` | $4$ | Latitude scaled by $10^7$ |
| `5..8` | `lng` | `angle_int32_t` | $4$ | Longitude scaled by $10^7$ |

---

### 3.2. Trajectory Intention Packet (`trajectory_intention_packet_t`)
Sent by the sender to announce an incoming trajectory and specify how many chunks to expect.
* **Total Size:** $4\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `1` (`TRAJECTORY_INTENTION`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Unique ID for this transfer session |
| `2` | `trajectory_following_mode` | `trajectory_following_mode_t` | $1$ | Movement mode (`0`: Patrol, `1`: Hold, `2`: Move) |
| `3` | `nchunks` | `uint8_t` | $1$ | Total number of chunks to be transmitted |

---

### 3.3. Trajectory Intention OK (`trajectory_intention_ok_packet_t`)
Sent by the receiver to confirm readiness to receive chunks.
* **Total Size:** $2\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `2` (`TRAJECTORY_INTENTION_OK`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |

---

### 3.4. Trajectory Chunk Packet (`trajectory_chunk_packet_t`)
Carries a slice of the serialized trajectory payload.
- **Total Size:** $4 + N\text{ bytes}$ (where $0 \le N \le 28$, up to `MAX_PACKET_SIZE` of $32\text{ bytes}$)

> **Note:** `MAX_CHUNK_SIZE` is defined as `MAX_PACKET_SIZE - 4` ($28\text{ bytes}$), accounting for the $4\text{-byte}$ header (`packet_type`, `communication_id`, `chunk_n`, and `chunk_size`) preceding the raw payload bytes.

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `3` (`TRAJECTORY_CHUNK`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |
| `2` | `chunk_n` | `uint8_t` | $1$ | Sequence index of the current chunk |
| `3` | `chunk_size` | `uint8_t` | $1$ | Number of valid payload bytes in this packet ($N$) |
| `4+(N-1)` | `chunk` | `uint8_t[]` | $0\text{ to }28$ | Raw trajectory payload data (omitted when $N = 0$) |


### 3.5. Trajectory Chunk OK (`trajectory_chunk_ok_packet_t`)
Acknowledges the successful receipt of a specific chunk index.
* **Total Size:** $3\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `4` (`TRAJECTORY_CHUNK_OK`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |
| `2` | `chunk_n` | `uint8_t` | $1$ | Index of the acknowledged chunk |

---

### 3.6. Trajectory Flashed (`trajectory_flashed_packet_t`)
Signals that all chunks have been received and written to non-volatile memory/storage.
* **Total Size:** $2\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `5` (`TRAJECTORY_FLASHED`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |

---

### 3.7. Trajectory Flashed OK (`trajectory_flashed_ok_packet_t`)
Final handshake packet confirming the trajectory update transaction is complete.
* **Total Size:** $2\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `6` (`TRAJECTORY_FLASHED_OK`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |

---

### 3.8. Communication Failure (`communication_failure_packet_t`)
Sent in case of any critical failure.
* **Total Size:** $3\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `7` (`COMMUNICATION_FAILURE`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |
| `2` | `error_code` | `uint8_t` | $1$ | Diagnostic code identifying the failure reason |

---

### 3.9. Communication Failure OK (`communication_failure_ok_packet_t`)
Acknowledges that the failure notification was received.
* **Total Size:** $2\text{ bytes}$

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :---: | :--- | :--- | :---: | :--- |
| `0` | `packet_type` | `packet_type_t` | $1$ | Fixed value: `8` (`COMMUNICATION_FAILURE_OK`) |
| `1` | `communication_id` | `packet_id_t` | $1$ | Matching session ID |

---

## 4. Summary Table

| Packet Class | Type ID | Total Size (Bytes) | Fields Breakdown |
| :--- | :---: | :---: | :--- |
| `telemetry_packet_t` | `0` | **$9$** | `type(1)` + `lat(4)` + `lng(4)` |
| `trajectory_intention_packet_t` | `1` | **$4$** | `type(1)` + `id(1)` + `mode(1)` + `nchunks(1)` |
| `trajectory_intention_ok_packet_t` | `2` | **$2$** | `type(1)` + `id(1)` |
| `trajectory_chunk_packet_t` | `3` | **$4\text{ to }32$** | `type(1)` + `id(1)` + `chunk_n(1)` + `chunk_size(1)` + `chunk(0..28)` |
| `trajectory_chunk_ok_packet_t` | `4` | **$3$** | `type(1)` + `id(1)` + `chunk_n(1)` |
| `trajectory_flashed_packet_t` | `5` | **$2$** | `type(1)` + `id(1)` |
| `trajectory_flashed_ok_packet_t` | `6` | **$2$** | `type(1)` + `id(1)` |
| `communication_failure_packet_t` | `7` | **$3$** | `type(1)` + `id(1)` + `error_code(1)` |
| `communication_failure_ok_packet_t` | `8` | **$2$** | `type(1)` + `id(1)` |his approach minimizes protocol overhead while maintaining the information required to reconstruct the original trajectory.
