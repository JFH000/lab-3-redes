# **Laboratorio #3: Análisis de Capa de Transporte y Sockets**

Este repositorio contiene la implementación de un sistema de comunicación en tiempo real bajo un modelo de Publicación-Suscripción. El objetivo principal es analizar y comparar el comportamiento de los protocolos de transporte TCP y UDP en un entorno práctico.

El sistema consta de tres componentes principales:
*   **Broker:** Un servidor central que gestiona los temas (partidos) y redirige los mensajes de los publicadores a los suscriptores interesados.
*   **Publisher (Publicador):** Un cliente que simula a un periodista enviando actualizaciones en vivo (goles, tarjetas, etc.) sobre un partido específico.
*   **Subscriber (Suscriptor):** Un cliente que simula a un aficionado que se suscribe a uno o varios partidos para recibir las actualizaciones en tiempo real.

El proyecto está implementado en su totalidad en lenguaje C y se ha desarrollado en dos versiones, una utilizando sockets TCP y otra con sockets UDP.

## **Estructura de Archivos**

El repositorio está organizado de la siguiente manera:

```
.
├── broker_tcp.c          # Código fuente del Broker que opera sobre TCP
├── publisher_tcp.c       # Código fuente del Publicador que opera sobre TCP
├── subscriber_tcp.c      # Código fuente del Suscriptor que opera sobre TCP
|
├── broker_udp.c          # Código fuente del Broker que opera sobre UDP
├── publisher_udp.c       # Código fuente del Publicador que opera sobre UDP
├── subscriber_udp.c      # Código fuente del Suscriptor que opera sobre UDP
|
├── tcp_pubsub.pcap       # Captura de tráfico Wireshark para la prueba con TCP
├── udp_pubsub.pcap       # Captura de tráfico Wireshark para la prueba con UDP
└── README.md             # Este archivo```
```

## **Prerrequisitos**

Para compilar y ejecutar este proyecto, necesitarás:
*   Un compilador de C, preferiblemente **GCC**.
*   Un sistema operativo basado en Linux (recomendado para la compatibilidad de sockets) o wsl en windows.

## **Compilación**

Abre una terminal y ejecuta los siguientes comandos para compilar los ejecutables para ambas versiones del sistema.

**1. Compilar la versión TCP:**
```bash
gcc broker_tcp.c -o broker_tcp
gcc publisher_tcp.c -o publisher_tcp
gcc subscriber_tcp.c -o subscriber_tcp
```

**2. Compilar la versión UDP:**
```bash
gcc broker_udp.c -o broker_udp
gcc publisher_udp.c -o publisher_udp
gcc subscriber_udp.c -o subscriber_udp
```
Al finalizar, tendrás seis archivos ejecutables en tu directorio.

## **Ejecución y Pruebas**

Para probar el sistema, necesitarás abrir **múltiples terminales**. El orden de ejecución es importante: primero el broker, luego los suscriptores y finalmente los publicadores.

*(Nota: En los siguientes ejemplos, se asume que todos los componentes se ejecutan en la misma máquina, por lo que la IP del broker es `127.0.0.1`. Si ejecutas los componentes en máquinas diferentes, reemplaza esta IP por la correspondiente.)*

### **Prueba del Sistema TCP**

**Paso 1: Iniciar el Broker (Terminal 1)**
El broker se inicia especificando el puerto en el que escuchará las conexiones.
```bash
./broker_tcp 8080
```
> El broker ahora está esperando conexiones en el puerto 8080.

**Paso 2: Iniciar Suscriptores (Terminal 2 y 3)**
Inicia uno o más suscriptores. Cada uno se conecta al broker y se suscribe a un "tema" (por ejemplo, un partido de fútbol).

*   Suscriptor para el partido "BARCA_VS_MADRID":
    ```bash
    # Terminal 2
    ./subscriber_tcp 127.0.0.1 8080 BARCA_VS_MADRID
    ```
*   Suscriptor para el partido "MILLOS_VS_NAL":
    ```bash
    # Terminal 3
    ./subscriber_tcp 127.0.0.1 8080 MILLOS_VS_NAL
    ```
> Los suscriptores están ahora esperando mensajes de los partidos a los que se suscribieron.

**Paso 3: Iniciar Publicadores (Terminal 4 y 5)**
Inicia los publicadores para enviar mensajes a los temas.

*   Publicador para el partido "BARCA_VS_MADRID":
    ```bash
    # Terminal 4
    ./publisher_tcp 127.0.0.1 8080 BARCA_VS_MADRID "Gol de Barcelona al minuto 30!"
    ```
    > El mensaje aparecerá en la Terminal 2.

*   Publicador para el partido "MILLOS_VS_NAL":
    ```bash
    # Terminal 5
    ./publisher_tcp 127.0.0.1 8080 MILLOS_VS_NAL "Tarjeta amarilla para Millonarios"
    ```
    > El mensaje aparecerá en la Terminal 3.

### **Prueba del Sistema UDP**

El procedimiento es idéntico al de TCP, pero utilizando los ejecutables de UDP.

**Paso 1: Iniciar el Broker (Terminal 1)**
```bash
./broker_udp 8081
```

**Paso 2: Iniciar Suscriptores (Terminal 2 y 3)**
```bash
# Terminal 2
./subscriber_udp 127.0.0.1 8081 BARCA_VS_MADRID

# Terminal 3
./subscriber_udp 127.0.0.1 8081 MILLOS_VS_NAL
```

**Paso 3: Iniciar Publicadores (Terminal 4 y 5)**
```bash
# Terminal 4
./publisher_udp 127.0.0.1 8081 BARCA_VS_MADRID "Gol de Madrid al minuto 75!"

# Terminal 5
./publisher_udp 127.0.0.1 8081 MILLOS_VS_NAL "Cambio en Nacional"
```

## **Análisis de Tráfico**

Este repositorio incluye los archivos `tcp_pubsub.pcap` y `udp_pubsub.pcap`. Estos archivos son capturas de tráfico de red realizadas con Wireshark durante la ejecución de las pruebas.

Puedes abrir estos archivos con Wireshark para observar directamente las diferencias entre ambos protocolos:
*   El handshake de tres vías en TCP.
*   Los acuses de recibo (ACKs) en TCP.
*   La simplicidad y falta de conexión en los datagramas UDP.
*   Las diferencias en el tamaño de las cabeceras.
