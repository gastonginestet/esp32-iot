# Stack NodeRed + MQTT + InfluxDB 

Este repositorio contiene lo minimo requerido para correr un stack con 

* [NodeRed](https://nodered.org/) Sistema de programacion orientada a flujos (open-source)
* [Mosquitto](https://mosquitto.org/) Servidor MQTT (open-source)
* [InfluxDB](https://www.influxdata.com/products/influxdb-overview/) Base de datos orientada a series temporales (open-source)

## Run

Clonar repositorio

> git clone git@gitlab.com:dgraselli/stack-nodered-mqtt

Ejecutar el stack

> docker-compose -d up

Si todo esta OK, ya se puede ver:

* [NodeRed IDE](http://localhost:1880)
* [NodeRed UI](http://localhost:1880/ui)

## Stop

> docker-compose stop

## Remove

> docker-compose down

