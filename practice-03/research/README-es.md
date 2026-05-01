# Componente de investigación: protocolos de comunicación industrial

## Objetivo

Este componente de investigación compara los principales protocolos de
comunicación industrial utilizados en sistemas de automatización, identificando
sus características, aplicaciones típicas y las interfaces físicas o lógicas que
normalmente emplean.

La comunicación industrial debe seleccionarse de acuerdo con el nivel del
sistema:

* nivel de campo: sensores, actuadores, variadores, E/S remotas,
  instrumentación
* nivel de control: PLC, controladores, pasarelas, HMI
* nivel de supervisión: SCADA, historiadores, estaciones de ingeniería,
  computadores de borde
* nivel empresarial o nube: bases de datos, MES, analítica, tableros de control

No existe un único protocolo que sea el mejor para todos los niveles. Los buses
de bajo nivel priorizan temporización determinística, inmunidad al ruido y
cableado simple, mientras que los protocolos Ethernet de niveles superiores
priorizan ancho de banda, interoperabilidad, diagnóstico e integración con
sistemas de TI.

## Comparación general

| Protocolo | Interfaz o medio típico | Modelo de comunicación | Fortalezas principales | Aplicaciones comunes | Limitaciones principales |
| --- | --- | --- | --- | --- | --- |
| Modbus RTU | RS-485, a veces RS-232 | maestro/esclavo por solicitud-respuesta | simple, abierto, económico, ampliamente soportado | medidores, variadores, PLC, E/S remotas, instrumentación | modelo de datos limitado, diagnóstico limitado, sin seguridad nativa |
| Modbus TCP | Ethernet sobre TCP/IP | cliente/servidor por solicitud-respuesta | integración Ethernet sencilla, común en pasarelas y SCADA | PLC a SCADA, medidores de energía, pasarelas industriales | no es determinístico por sí mismo, sin seguridad nativa en Modbus TCP clásico |
| PROFIBUS DP | bus de campo RS-485 | intercambio cíclico controlador-dispositivo | bus de campo robusto, comportamiento determinístico, ecosistema maduro | automatización de fábrica, E/S remotas, variadores, módulos de proceso | menor ancho de banda que Ethernet industrial, cableado y configuración especializados |
| PROFIBUS PA | capa física MBP, usualmente mediante acopladores | bus de campo para instrumentos de proceso | soporte para instrumentación de proceso, dispositivos alimentados por bus, uso en áreas peligrosas | transmisores de presión, temperatura, flujo y nivel | menor velocidad, enfoque en proceso, requiere acopladores/segmentos compatibles |
| PROFINET | Ethernet industrial | Ethernet industrial en tiempo real | alto ancho de banda, diagnóstico, flexibilidad de topología, integración con TCP/IP | redes PLC, E/S distribuidas, variadores, automatización de máquinas | ingeniería más compleja que buses seriales simples |
| CAN / CANopen | bus diferencial CAN | orientado a mensajes con arbitraje por prioridad | robusto, arbitraje determinístico, ecosistema fuerte para embebidos | maquinaria, variadores, vehículos, equipos médicos, equipos móviles | cargas útiles cortas, ancho de banda limitado frente a Ethernet |
| EtherNet/IP | Ethernet con TCP/IP y UDP/IP, capa de aplicación CIP | productor-consumidor y mensajería explícita | infraestructura Ethernet estándar, ecosistema fuerte de PLC/dispositivos, servicios CIP | PLC, variadores, robots, seguridad, movimiento, redes de planta | el diseño de red es crítico para comportamiento en tiempo real y segmentación |
| HART | lazo de corriente 4-20 mA con superposición digital FSK; variante WirelessHART | datos digitales sobre lazo de instrumentación | conserva compatibilidad analógica y agrega diagnóstico/configuración | transmisores de proceso, posicionadores de válvulas, calibración y mantenimiento | baja tasa de datos, principalmente instrumentación de proceso |
| OPC UA | redes Ethernet/IP; TCP, HTTPS, transportes PubSub según el perfil | cliente/servidor y publicación/suscripción con modelo de información | independiente de plataforma, seguro, modelo semántico rico | SCADA, historiadores, MES, pasarelas de borde, integración con nube | más pesado que protocolos de campo, requiere gestión de seguridad y certificados |
| IO-Link | enlace punto a punto de tres hilos para sensores/actuadores | punto a punto maestro-dispositivo | integración sencilla de sensores inteligentes, diagnóstico, parametrización | sensores inteligentes, actuadores, módulos de E/S remotas | no es un bus de campo; normalmente se integra hacia arriba mediante un maestro IO-Link |
| MQTT | red IP por Ethernet/WiFi/celular | publicación/suscripción mediante broker | liviano, escalable, excelente para telemetría y tableros | pasarelas IIoT, monitoreo, recolección de datos borde/nube | no es un protocolo de control duro en tiempo real por sí mismo |

## Notas por protocolo

### Modbus

Modbus es uno de los protocolos industriales más usados porque es simple,
abierto y fácil de implementar en dispositivos pequeños. Define mensajes de
capa de aplicación para leer y escribir bobinas, entradas discretas, registros
de entrada y registros de retención.

Modbus RTU se transporta comúnmente sobre RS-485. RS-485 se prefiere frente a
RS-232 en ambientes industriales porque soporta señalización diferencial,
distancias mayores y cableado multidrop. RS-232 puede usarse en enlaces punto a
punto, pero es menos adecuado para instalaciones industriales ruidosas o
distribuidas.

Modbus TCP transporta mensajes Modbus sobre Ethernet y TCP/IP. Es común en
sistemas SCADA, pasarelas, redes PLC, medidores de energía y sistemas de
adquisición de datos donde no se requiere temporización determinística de nivel
movimiento.

Interfaces típicas:

* RS-485 para Modbus RTU
* RS-232 para enlaces heredados cortos punto a punto
* Ethernet/TCP/IP para Modbus TCP

Aplicaciones típicas:

* medidores de energía y potencia
* variadores de frecuencia
* comunicación PLC-SCADA
* módulos de E/S remotas
* pasarelas industriales
* instrumentación de temperatura, presión y flujo

### PROFIBUS

PROFIBUS es una familia de buses de campo mantenida por PROFIBUS & PROFINET
International para automatización industrial. PROFIBUS DP es común en
automatización de fábrica y conecta controladores con E/S distribuidas,
variadores y dispositivos de campo. PROFIBUS PA está orientado a automatización
de procesos e instrumentación.

PROFIBUS DP normalmente usa RS-485. Está diseñado para intercambio cíclico de
datos de proceso entre un controlador y dispositivos de campo, lo que lo hace
adecuado para actualizaciones predecibles de E/S de PLC. PROFIBUS PA utiliza
una capa física adecuada para instrumentos de proceso, incluyendo aplicaciones
donde importan la alimentación por bus y los requisitos de instalación en áreas
peligrosas.

Interfaces típicas:

* RS-485 para PROFIBUS DP
* capa física MBP para PROFIBUS PA

Aplicaciones típicas:

* E/S distribuidas
* variadores de motor
* celdas de automatización de fábrica
* transmisores de proceso mediante segmentos PA
* redes de campo para PLC

### PROFINET

PROFINET es el estándar Ethernet industrial de PI. Extiende el ecosistema
PROFIBUS hacia Ethernet y soporta comunicación industrial en tiempo real,
perfiles de dispositivo, diagnóstico, gestión de topología y coexistencia con
comunicación TCP/IP regular.

PROFINET es apropiado cuando la instalación necesita mayor ancho de banda,
integración más sencilla con infraestructura Ethernet y diagnóstico más avanzado
que el ofrecido por buses de campo seriales tradicionales.

Interfaces típicas:

* Ethernet industrial
* cableado Ethernet de cobre
* fibra en instalaciones que requieren mayor distancia o aislamiento electrico

Aplicaciones típicas:

* comunicación PLC a E/S remotas
* automatización de máquinas
* variadores y automatización relacionada con movimiento
* automatización de procesos con integración Ethernet
* seguridad y diagnóstico cuando el ecosistema de dispositivos lo soporta

### CANbus y CANopen

CAN es un bus diferencial robusto asociado originalmente con vehículos, pero
también ampliamente usado en sistemas industriales embebidos. Utiliza
identificadores de mensaje y arbitraje de bus, permitiendo que mensajes de alta
prioridad accedan al bus sin corromper tramás de menor prioridad.

CANopen es un protocolo de capa superior y un sistema de perfiles de dispositivo
construido sobre CAN. Agrega objetos de comunicación estándarizados, gestión de
red, configuración y diccionarios de objetos, lo que lo hace más adecuado que
CAN puro para dispositivos industriales interoperables.

Interfaces típicas:

* bus diferencial CAN
* CANopen sobre Classical CAN
* CANopen FD sobre CAN FD en sistemas más recientes

Aplicaciones típicas:

* control de movimiento
* módulos de maquina
* maquinaria movil
* ascensores y elevadores
* equipos médicos
* sistemas ferroviarios, maritimos y vehículos especiales

### EtherNet/IP y CIP

EtherNet/IP es un protocolo Ethernet industrial mantenido por ODVA. Adapta el
Common Industrial Protocol, o CIP, a redes Ethernet estándar con TCP/IP y
UDP/IP. CIP proporciona servicios orientados a objetos para control,
configuración, diagnóstico, seguridad, sincronización, movimiento y gestión de
red, dependiendo del perfil y soporte del dispositivo.

EtherNet/IP es común en ecosistemas PLC donde se desea infraestructura Ethernet
estándar, pero se siguen necesitando perfiles de dispositivo y semantica de
control industrial.

Interfaces típicas:

* Ethernet estándar basado en IEEE 802.3
* TCP/IP para mensajería explícita
* UDP/IP para mensajería implicita de E/S

Aplicaciones típicas:

* redes PLC
* E/S remotas
* variadores
* robots
* dispositivos de seguridad
* control de movimiento y control sincronizado cuando el ecosistema completo lo
  soporta

### HART

HART está fuertemente asociado con instrumentación de procesos. Su forma clasica
superpone comunicación digital sobre un lazo analógico de corriente 4-20 mA,
permitiendo que sistemas analógicos heredados sigan recibiendo una variable de
proceso mientras herramientas de mantenimiento o sistemas de gestión de activos
acceden a datos digitales de diagnóstico y configuración.

Interfaces típicas:

* lazo de corriente 4-20 mA con superposición digital FSK
* WirelessHART para redes inalámbricas de instrumentación de proceso

Aplicaciones típicas:

* transmisores de presión
* transmisores de flujo
* transmisores de temperatura
* posicionadores de válvulas
* configuración y diagnóstico de instrumentos
* flujos de mantenimiento de proceso

### OPC UA

OPC UA es un marco de interoperabilidad, no simplemente un bus de campo. Se usa
para exponer datos industriales con un modelo de información independiente de
plataforma y conceptos de seguridad integrados. Puede representar datos de
dispositivo, alarmas, datos históricos, metadatos y relaciones semánticas, por
lo que es importante en niveles SCADA, borde, MES e integración con nube.

OPC UA normalmente no se elige como reemplazo de más bajo nivel para un bus
simple de sensores. En cambio, suele usarse por encima de PLC, pasarelas o
dispositivos de borde para exponer datos industriales estructurados a sistemas
de nivel superior.

Interfaces típicas:

* redes Ethernet/IP
* OPC UA TCP
* HTTPS
* perfiles OPC UA PubSub según la arquitectura

Aplicaciones típicas:

* integración SCADA
* acceso a datos de historiadores
* pasarelas de borde
* integración MES
* intercambio de datos industriales independiente del proveedor
* modelos semánticos para equipos y datos de producción

### IO-Link

IO-Link es una tecnologia de comunicación punto a punto para sensores y
actuadores, estándarizada como IEC 61131-9. No es un bus de campo. En cambio,
conecta sensores y actuadores inteligentes a maestros IO-Link mediante cableado
simple de tres hilos. Luego, el maestro se conecta hacia arriba a una red
industrial de nivel superior, como PROFINET, EtherNet/IP, EtherCAT, Modbus TCP u
otra red PLC.

Interfaces típicas:

* cable punto a punto de tres hilos para sensores/actuadores
* conectores M12, M8 o M5 según el dispositivo
* bus de campo o Ethernet industrial de nivel superior mediante un maestro
  IO-Link

Aplicaciones típicas:

* sensores inteligentes
* actuadores
* diagnóstico de dispositivos
* descarga automatica de parametros despues del reemplazo de un dispositivo
* máquinas compactas con muchos puntos de sensado

### MQTT

MQTT es un protocolo liviano de publicación/suscripción usado comúnmente para
telemetría. No es tradicionalmente un protocolo determinístico de bus de campo,
pero es muy util en escenarios de Internet Industrial de las Cosas donde
dispositivos de borde publican datos a un broker y tableros, bases de datos o
sistemas en la nube se suscriben a esos datos.

En está práctica, MQTT se usa en la Actividad 04 Fase C para publicar una
medición de potenciometro y recibir comandos de LED a través de un broker.

Interfaces típicas:

* Ethernet
* WiFi
* redes IP celulares
* TCP/IP con TLS opcional
* WebSockets para tableros en navegador

Aplicaciones típicas:

* telemetría IIoT
* monitoreo remoto
* integración con nube
* tableros de control
* intercambio no crítico de comandos y estado

## Comparación de interfaces

### RS-232

RS-232 es una interfaz serial punto a punto. Es simple y común en equipos
antiguos, puertos de configuración e instrumentos de laboratorio, pero no es
ideal para redes industriales multidrop ni para tendidos largos en ambientes
ruidosos.

### RS-485

RS-485 utiliza señalización diferencial y soporta redes multidrop. Es común en
Modbus RTU y PROFIBUS DP. Se prefiere en muchos enlaces seriales industriales
porque tolera mejor el ruido y soporta mayores distancias que RS-232.

### Ethernet

Ethernet es la base física y de enlace dominante en redes industriales modernas.
Protocolos como Modbus TCP, PROFINET, EtherNet/IP, OPC UA y MQTT pueden usar
infraestructura Ethernet/IP, aunque su comportamiento en tiempo real y sus
modelos de aplicación difieren significativamente.

### CAN

CAN es un bus diferencial diseñado para comunicación embebida robusta. Es
adecuado para módulos de maquina, vehículos y dispositivos embebidos
distribuidos que necesitan arbitraje predecible y mensajes compactos.

### Lazo de corriente 4-20 mA

El lazo de corriente 4-20 mA sigue siendo importante en instrumentación de
procesos porque es simple, robusto y puede representar una variable analógica de
proceso sobre tendidos industriales largos. HART lo extiende con comunicación
digital mientras conserva compatibilidad analógica.

### Enlaces de tres hilos para sensores/actuadores

IO-Link usa cableado punto a punto simple de tres hilos para sensores y
actuadores inteligentes. Esto mantiene el cableado de campo sencillo y permite
diagnóstico digital y parametrización.

## Criterios de selección

Use las siguientes preguntas al seleccionar un protocolo:

1. El sistema está en nivel de campo, control, supervisión o empresa?
2. La aplicación requiere temporización determinística o solo monitoreo?
3. La instalación es punto a punto, multidrop, estrella Ethernet, anillo o
   mixta?
4. Los dispositivos son sensores limitados, PLC, variadores, instrumentos o
   pasarelas?
5. Se requiere interoperabilidad entre proveedores?
6. Se requiere seguridad integrada o debe proporcionarse mediante el diseño de
   red?
7. Son importantes el diagnóstico y la parametrización remota?
8. Se espera que el protocolo se integre con nube o sistemas de TI?

## Recomendación práctica

Para pequeños sistemas embebidos de laboratorio con placas ESP32:

* UART, I2C y SPI son adecuados para comunicación local a nivel de placa.
* WiFi y MQTT son útiles para monitoreo distribuido simple y tableros.
* Modbus RTU sobre RS-485 es un siguiente paso práctico para interoperabilidad
  serial industrial.
* Modbus TCP, MQTT y OPC UA son útiles para Ethernet e integración de nivel
  superior.

Para instalaciones industriales reales:

* use Modbus RTU o PROFIBUS cuando se requiera un bus de campo serial robusto
* use PROFINET o EtherNet/IP cuando se requiera Ethernet industrial centrado en
  PLC
* use CANopen cuando se necesite control embebido distribuido y perfiles de
  dispositivo
* use HART para compatibilidad con instrumentación de proceso
* use IO-Link para integración de sensores y actuadores inteligentes
* use OPC UA para interoperabilidad estructurada entre proveedores y sistemas de
  nivel superior
* use MQTT para telemetría, tableros y distribución de datos borde/nube, no para
  lazos de control duro en tiempo real

## Referencias

* Modbus Organization. "An Introduction to Modbus." https://www.modbus.org/introduction-to-modbus
* PI North America. "PROFIBUS." https://us.profinet.com/technology/profibus/
* PROFIBUS & PROFINET International. "PROFINET." https://www.profinet.com/
* CAN in Automation. "CANopen." https://www.can-cia.org/can-knowledge/canopen
* ODVA. "EtherNet/IP." https://www.odva.org/technology-standards/key-technologies/ethernet-ip/
* ODVA. "Common Industrial Protocol." https://www.odva.org/technology-standards/key-technologies/
* OPC Foundation. "What is OPC?" https://opcfoundation.org/about/what-is-opc/
* IO-Link Community. "IO-Link Technology." https://io-link.com/technology
