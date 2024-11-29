# Instrucciones de Uso
> [! Requisitos Previos]
> Compilador GCC compatible con C11 o superior.
> Sistema operativo Unix/Linux con soporte para semáforos POSIX y memoria compartida.
> Herramientas de compilación estándar (make es opcional si se proporciona un Makefile).

# compilación
1. Compilar el servidor
Clonar el repositorio o copiar los archivos del proyecto en un directorio local.
Compilar el Servidor ("core"):
    * `gcc -o core core.c protocol.c queue.c -lpthread -lrt`

        Asegúrate de incluir todas las dependencias necesarias (protocol.c, queue.c).
        Las opciones -lpthread y -lrt son necesarias para los hilos y la memoria compartida.
    * `make server`
        Este es un atajo para la construccion del binario server(core) de forma sencilla.

2. Compilar el Cliente:
    * `gcc -o cclient client.c protocol.c -lpthread -lrt`

        Incluye protocol.c si es necesario.
        Las opciones -lpthread y -lrt también son necesarias.
    * `make client`
        Este es un atajo para la construccion del binario cclient de forma sencilla.
# Ejecución
**Iniciar el Servidor**

En una terminal, ejecuta:

`./server`

El servidor quedará en ejecución, esperando conexiones de clientes.

**Ejecutar Clientes**

Puedes ejecutar múltiples clientes para simular tráfico. Cada cliente puede ser de tipo pre-pago (-p 0) o pos-pago (-p 1), y se le puede asignar un nombre con -n.

Ejemplo:

`./cclient -n "Cliente1" -p 0`

Para simular múltiples clientes, puedes usar un script o abrir varias terminales.
Usando el Script de Automatización

Hay un script `run_clients.sh` que ejecuta múltiples clientes automáticamente.

### Contenido del script:

```bash
#!/bin/bash

# Número de clientes pre-pago y pos-pago
NUM_PREPAGO=5
NUM_POSPAGO=5

for i in $(seq 1 $NUM_PREPAGO)
do
    ./cclient -n "ClientePre$i" -p 0 &
done

for i in $(seq 1 $NUM_POSPAGO)
do
    ./cclient -n "ClientePos$i" -p 1 &
done

wait
```

### Ejecuta el script:
```bash
bash run_clients.sh
```

**Esto iniciará 5 clientes pre-pago y 5 pos-pago en segundo plano.**

## Uso del Cliente

Una vez que el cliente está en ejecución, puedes interactuar ingresando mensajes o comandos:

- **Enviar Mensajes:** Escribe el mensaje y presiona Enter para enviarlo al servidor.

- **Cambiar Tipo de Usuario:** Escribe cambiar y sigue las instrucciones para cambiar entre pre-pago y pos-pago.

## Finalización

- **Servidor:** Para detener el servidor, presiona Ctrl+C en la terminal donde está ejecutándose. El servidor manejará la señal y liberará los recursos correctamente.

- **Cliente:** Para cerrar un cliente, presiona Ctrl+C. El cliente liberará los recursos y terminará la ejecución.

---

## Notas Adicionales

* `Límites para Usuarios Pre-pago:` Los usuarios pre-pago tienen un límite de 10 mensajes. Una vez alcanzado, deben cambiar a pos-pago para continuar enviando mensajes.

* `Logs y Mensajes:` Tanto el servidor como los clientes muestran mensajes en la consola que permiten monitorear su estado y las acciones realizadas.

* `Manejo de Errores:` Si ocurre algún error, verifica que el servidor esté en ejecución y que no haya conflictos con los recursos compartidos.
