
# RobeRT Communication Protocol Specification (v1.0)

## 1. Introduction

The **RobeRT** protocol is designed to provide a lightweight data communication for the project

---

## 2. Layer 1: Client $\rightarrow$ Middleware

**Transport:** ZeroMQ (Request-Reply Pattern)

**Port:** `5555` (Default)

**Format:** ASCII String, Pipe-Delimited (`|`)

**Data Format:** CSV for coordinate values.

### 2.1 Motion Commands

| Command | Format | Description |
| --- | --- | --- |
| **MoveL** | `MOVEL|x,y,z,q1,q2,q3,q4,cf1,cf4,cf6,cfx` | Linear movement to Cartesian target. |
| **MoveJ** | `MOVEJ|x,y,z,q1,q2,q3,q4,cf1,cf4,cf6,cfx` | Non-linear axis movement to Cartesian target. |
| **MoveC** | `MOVEC|target_cir|target_to` | Circular move via `target_cir` to `target_to`. |
| **MoveAbsJ** | `MOVEABSJ|j1,j2,j3,j4,j5,j6` | Absolute joint move in degrees. |

### 2.2 System Commands

| Command | Format | Description |
| --- | --- | --- |
| **SetSpeed** | `SETSPEED\|value` | Sets the TCP speed in mm/s. |
| **SetPrecision** | `SETPRECISION\|value` | Sets the movement precision (fine, z10, etc) |
| **Stop** | `STOP\|NONE` | Executes a category 1 stop on the controller. |
| **Exit** | `EXIT\|NONE` | Gracefully closes the Middleware session. |

---

## 3. Layer 2: Middleware $\rightarrow$ Robot (C++ to RAPID)

**Transport:** TCP/IP Native Sockets (PC Interface Option 616-1)

**Port:** `1025` (Default)

**Format:** ASCII String, RAPID-Record Compatible.

**Instruction Delimiter:** Semicolon (`;`)

### 3.1 Packet Structure

The Middleware translates simplified CSV data into strict RAPID data structures (`robtarget`, `jointtarget`) to allow the use of the `StrToVal` instruction on the IRC5.

| Prefix | RAPID Instruction | Formatted Payload Example |
| --- | --- | --- |
| **ML** | `MoveL` | `ML\|[[x,y,z],[q1,q2,q3,q4],[cf1,cf4,cf6,cfx],[9e9,9e9,9e9,9e9,9e9,9e9]];` |
| **MJ** | `MoveJ` | `MJ\|[[x,y,z],[q1,q2,q3,q4],[cf1,cf4,cf6,cfx],[9e9,9e9,9e9,9e9,9e9,9e9]];` |
| **MC** | `MoveC` | `MC\|[[cir_pos...],[...]]\|[[to_pos...],[...]];` |
| **MA** | `MoveAbsJ` | `MA\|[[j1,j2,j3,j4,j5,j6],[9e9,9e9,9e9,9e9,9e9,9e9]];` |
| **SS** | `VelSet` | `SS\|100;` |
| **SP** | `SetPrecision` | `SP\|fine;` |
| **ST** | `Stop` | `ST\|STOP;` |
| **HOME** | `Home` | `HOME\|NONE;` |
| **ORG** | `Origin` | `ORG\|NONE;` |
| **EX** | `Exit` | `EX\|EXIT;` |

---

## 4. Error Handling

The Robot (RAPID) will respond with a status string after every received command:

* `ACK`: Instruction received and parsed successfully.
* `ERR_PARSE`: Data structure mismatch in `StrToVal`.
* `ERR_LIMIT`: Target outside of the robot's reachable workspace.
* `ERR_SPEED`: Speed value out of bounds.
* `ERR_UNKNOWN`: Unrecognized command prefix.