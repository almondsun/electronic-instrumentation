# Research Component: Industrial Communication Protocols

## Objective

This research component compares the main industrial communication protocols
used in automation systems, identifying their characteristics, typical
applications, and the physical or logical interfaces they normally employ.

Industrial communication must be selected according to the level of the system:

* field level: sensors, actuators, drives, remote I/O, instrumentation
* control level: PLCs, controllers, gateways, HMIs
* supervisory level: SCADA, historians, engineering stations, edge computers
* enterprise/cloud level: databases, MES, analytics, dashboards

No single protocol is best for every level. Low-level buses prioritize
deterministic timing, noise immunity, and simple wiring, while higher-level
Ethernet protocols prioritize bandwidth, interoperability, diagnostics, and
integration with IT systems.

## Summary Comparison

| Protocol | Typical interface or medium | Communication model | Main strengths | Common applications | Main limitations |
| --- | --- | --- | --- | --- | --- |
| Modbus RTU | RS-485, sometimes RS-232 | master/slave request-response | simple, open, low cost, widely supported | meters, drives, PLCs, remote I/O, instrumentation | limited data model, limited diagnostics, no native security |
| Modbus TCP | Ethernet over TCP/IP | client/server request-response | easy Ethernet integration, common in gateways and SCADA | PLC-to-SCADA, energy meters, industrial gateways | not deterministic by itself, no native security in classic Modbus TCP |
| PROFIBUS DP | RS-485 fieldbus | cyclic controller-device exchange | robust fieldbus, deterministic behavior, mature device ecosystem | factory automation, remote I/O, drives, process skids | lower bandwidth than Industrial Ethernet, specialized cabling/configuration |
| PROFIBUS PA | MBP physical layer, often via couplers | fieldbus for process instruments | process instrumentation support, bus-powered devices, hazardous-area use | pressure, temperature, flow, and level transmitters | lower speed, process-focused, requires compatible couplers/segments |
| PROFINET | Industrial Ethernet | real-time industrial Ethernet | high bandwidth, diagnostics, topology flexibility, integration with TCP/IP | PLC networks, distributed I/O, drives, machine automation | more complex engineering than simple serial buses |
| CAN / CANopen | CAN differential bus | message-oriented, priority arbitration | robust, deterministic arbitration, strong embedded profile ecosystem | machinery, drives, vehicles, medical equipment, mobile equipment | short payloads, limited bandwidth compared with Ethernet |
| EtherNet/IP | Ethernet with TCP/IP and UDP/IP, CIP application layer | producer-consumer and explicit messaging | standard Ethernet infrastructure, strong PLC/device ecosystem, CIP services | PLCs, drives, robots, safety, motion, plant networks | network design matters for real-time behavior and segmentation |
| HART | 4-20 mA current loop with FSK digital overlay; WirelessHART variant | digital data over instrumentation loop | preserves analog compatibility while adding diagnostics/configuration | process transmitters, valve positioners, calibration and maintenance | low data rate, mostly process instrumentation |
| OPC UA | Ethernet/IP networks; TCP, HTTPS, PubSub transports depending on profile | client/server and publish/subscribe information modeling | platform independent, secure, rich semantic data model | SCADA, historians, MES, edge gateways, cloud integration | heavier than fieldbus protocols, requires security and certificate management |
| IO-Link | point-to-point 3-wire sensor/actuator link | master-device point-to-point | simple smart-sensor integration, diagnostics, parameterization | smart sensors, actuators, remote I/O modules | not a fieldbus; usually connects upward through an IO-Link master |
| MQTT | Ethernet/WiFi/cellular IP network | publish/subscribe via broker | lightweight, scalable, excellent for telemetry and dashboards | IIoT gateways, monitoring, cloud/edge data collection | not a hard real-time control protocol by itself |

## Protocol Notes

### Modbus

Modbus is one of the most widely used industrial protocols because it is simple,
open, and easy to implement in small devices. It defines application-layer
messages for reading and writing coils, discrete inputs, input registers, and
holding registers.

Modbus RTU is commonly carried over RS-485. RS-485 is preferred over RS-232 in
industrial environments because it supports differential signaling, longer cable
runs, and multidrop wiring. RS-232 can be used for point-to-point links, but it
is less suitable for noisy or distributed industrial installations.

Modbus TCP carries Modbus messages over Ethernet and TCP/IP. It is common in
SCADA systems, gateways, PLC networks, energy meters, and data acquisition
systems where deterministic motion-level timing is not required.

Typical interfaces:

* RS-485 for Modbus RTU
* RS-232 for short point-to-point legacy links
* Ethernet/TCP/IP for Modbus TCP

Typical applications:

* energy and power meters
* variable-frequency drives
* PLC-to-SCADA communication
* remote I/O modules
* industrial gateways
* temperature, pressure, and flow instrumentation

### PROFIBUS

PROFIBUS is a fieldbus family maintained by PROFIBUS & PROFINET International
for industrial automation. PROFIBUS DP is common in factory automation and links
controllers with distributed I/O, drives, and field devices. PROFIBUS PA targets
process automation and instrumentation.

PROFIBUS DP normally uses RS-485. It is designed for cyclic process-data
exchange between a controller and field devices, which makes it suitable for
predictable PLC I/O updates. PROFIBUS PA uses a physical layer suited to process
instruments, including applications where bus power and hazardous-area
installation requirements matter.

Typical interfaces:

* RS-485 for PROFIBUS DP
* MBP fieldbus physical layer for PROFIBUS PA

Typical applications:

* distributed I/O
* motor drives
* factory automation cells
* process transmitters through PA segments
* PLC field networks

### PROFINET

PROFINET is PI's Industrial Ethernet standard. It extends the PROFIBUS ecosystem
toward Ethernet while supporting industrial real-time communication, device
profiles, diagnostics, topology management, and coexistence with regular TCP/IP
communication.

PROFINET is appropriate when the installation needs higher bandwidth, easier
integration with Ethernet infrastructure, and more advanced diagnostics than
traditional serial fieldbuses.

Typical interfaces:

* Industrial Ethernet
* copper Ethernet cabling
* fiber in installations that require longer distance or electrical isolation

Typical applications:

* PLC-to-remote-I/O communication
* machine automation
* drives and motion-related automation
* process automation with Ethernet integration
* safety and diagnostics when supported by the device ecosystem

### CANbus and CANopen

CAN is a robust differential bus originally associated with vehicles but also
widely used in embedded industrial systems. It uses message identifiers and bus
arbitration, allowing high-priority messages to win access without corrupting
lower-priority frames.

CANopen is a higher-layer protocol and device-profile system built on CAN. It
adds standardized communication objects, network management, configuration, and
object dictionaries, which makes it more suitable for interoperable industrial
devices than raw CAN alone.

Typical interfaces:

* CAN differential bus
* CANopen over Classical CAN
* CANopen FD over CAN FD in newer systems

Typical applications:

* motion control
* machine modules
* mobile machinery
* lifts and elevators
* medical equipment
* rail, maritime, and specialty vehicle systems

### EtherNet/IP and CIP

EtherNet/IP is an Industrial Ethernet protocol maintained by ODVA. It adapts the
Common Industrial Protocol, or CIP, to standard Ethernet and TCP/IP/UDP/IP
networks. CIP provides object-oriented services for control, configuration,
diagnostics, safety, synchronization, motion, and network management depending
on the profile and device support.

EtherNet/IP is common in PLC ecosystems where standard Ethernet infrastructure
is desired but industrial device profiles and control semantics are still
needed.

Typical interfaces:

* standard Ethernet based on IEEE 802.3
* TCP/IP for explicit messaging
* UDP/IP for implicit I/O messaging

Typical applications:

* PLC networks
* remote I/O
* drives
* robots
* safety devices
* motion control and synchronized control when the full ecosystem supports it

### HART

HART is strongly associated with process instrumentation. Its classic form
superimposes digital communication on a 4-20 mA analog current loop, allowing
legacy analog control systems to continue receiving a process variable while
maintenance tools or asset-management systems access digital diagnostics and
configuration data.

Typical interfaces:

* 4-20 mA current loop with digital FSK overlay
* WirelessHART for wireless process instrumentation networks

Typical applications:

* pressure transmitters
* flow transmitters
* temperature transmitters
* valve positioners
* instrument configuration and diagnostics
* process maintenance workflows

### OPC UA

OPC UA is an interoperability framework rather than a simple fieldbus. It is
used to expose industrial data with a platform-independent information model and
built-in security concepts. It can represent device data, alarms, historical
data, metadata, and semantic relationships, which makes it important at the
SCADA, edge, MES, and cloud-integration levels.

OPC UA is usually not chosen as the lowest-level replacement for a simple sensor
bus. Instead, it is often used above PLCs, gateways, or edge devices to expose
structured industrial data to higher-level systems.

Typical interfaces:

* Ethernet/IP networks
* OPC UA TCP
* HTTPS
* OPC UA PubSub profiles depending on architecture

Typical applications:

* SCADA integration
* historian data access
* edge gateways
* MES integration
* vendor-independent industrial data exchange
* semantic models for equipment and production data

### IO-Link

IO-Link is a point-to-point sensor and actuator communication technology
standardized as IEC 61131-9. It is not a fieldbus. Instead, it connects smart
sensors and actuators to IO-Link masters using simple three-wire cabling. The
master then connects upward to a higher-level industrial network such as
PROFINET, EtherNet/IP, EtherCAT, Modbus TCP, or another PLC network.

Typical interfaces:

* point-to-point three-wire sensor/actuator cable
* M12, M8, or M5 connectors depending on device
* higher-level fieldbus or Industrial Ethernet through an IO-Link master

Typical applications:

* smart sensors
* actuators
* device diagnostics
* automatic parameter download after device replacement
* compact machines with many sensor points

### MQTT

MQTT is a lightweight publish/subscribe protocol commonly used for telemetry.
It is not traditionally a deterministic fieldbus protocol, but it is very useful
for Industrial Internet of Things scenarios where edge devices publish data to a
broker and dashboards, databases, or cloud systems subscribe to that data.

In this practice, MQTT is used in Activity 04 Phase C to publish a potentiometer
measurement and receive LED commands through a broker.

Typical interfaces:

* Ethernet
* WiFi
* cellular IP networks
* TCP/IP with optional TLS
* WebSockets for browser dashboards

Typical applications:

* IIoT telemetry
* remote monitoring
* cloud integration
* dashboards
* non-critical command and status exchange

## Interface Comparison

### RS-232

RS-232 is a point-to-point serial interface. It is simple and common in older
equipment, configuration ports, and laboratory instruments, but it is not ideal
for multidrop industrial networks or long cable runs in noisy environments.

### RS-485

RS-485 uses differential signaling and supports multidrop networks. It is common
for Modbus RTU and PROFIBUS DP. It is preferred for many industrial serial links
because it is more noise-tolerant and supports longer distances than RS-232.

### Ethernet

Ethernet is the dominant physical and data-link foundation for modern industrial
networks. Protocols such as Modbus TCP, PROFINET, EtherNet/IP, OPC UA, and MQTT
can use Ethernet/IP infrastructure, although their real-time behavior and
application models differ significantly.

### CAN

CAN is a differential bus designed for robust embedded communication. It is
well-suited to machine modules, vehicles, and distributed embedded devices that
need predictable arbitration and compact messages.

### 4-20 mA Current Loop

The 4-20 mA current loop remains important in process instrumentation because it
is simple, robust, and can represent an analog process variable over long
industrial cable runs. HART extends it with digital communication while
preserving analog compatibility.

### Three-Wire Sensor/Actuator Links

IO-Link uses simple three-wire point-to-point wiring for smart sensors and
actuators. This keeps field wiring simple while enabling digital diagnostics and
parameterization.

## Selection Criteria

Use the following questions when selecting a protocol:

1. Is the system field-level, control-level, supervisory, or enterprise-level?
2. Does the application require deterministic timing or only monitoring?
3. Is the installation point-to-point, multidrop, star Ethernet, ring, or mixed?
4. Are the devices constrained sensors, PLCs, drives, instruments, or gateways?
5. Is vendor interoperability required?
6. Is built-in security required, or must it be provided by network design?
7. Are diagnostics and remote parameterization important?
8. Is the protocol expected to integrate with cloud or IT systems?

## Practical Recommendation

For small embedded laboratory systems using ESP32 boards:

* UART, I2C, and SPI are appropriate for local board-level communication.
* WiFi and MQTT are useful for simple distributed monitoring and dashboards.
* Modbus RTU over RS-485 is a practical next step for industrial serial
  interoperability.
* Modbus TCP, MQTT, and OPC UA are useful for Ethernet and higher-level
  integration.

For real industrial installations:

* use Modbus RTU or PROFIBUS when a robust serial fieldbus is required
* use PROFINET or EtherNet/IP when PLC-centered Industrial Ethernet is required
* use CANopen where embedded distributed control and device profiles are needed
* use HART for process instrumentation compatibility
* use IO-Link for smart sensor and actuator integration
* use OPC UA for structured interoperability between vendors and higher-level
  systems
* use MQTT for telemetry, dashboards, and edge/cloud data distribution, not for
  hard real-time control loops

## References

* Modbus Organization. "An Introduction to Modbus." https://www.modbus.org/introduction-to-modbus
* PI North America. "PROFIBUS." https://us.profinet.com/technology/profibus/
* PROFIBUS & PROFINET International. "PROFINET." https://www.profinet.com/
* CAN in Automation. "CANopen." https://www.can-cia.org/can-knowledge/canopen
* ODVA. "EtherNet/IP." https://www.odva.org/technology-standards/key-technologies/ethernet-ip/
* ODVA. "Common Industrial Protocol." https://www.odva.org/technology-standards/key-technologies/
* OPC Foundation. "What is OPC?" https://opcfoundation.org/about/what-is-opc/
* IO-Link Community. "IO-Link Technology." https://io-link.com/technology
