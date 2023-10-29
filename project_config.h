#ifndef __PROJECT_CONFIG
#define __PROJECT_CONFIG

#include <stdio.h>


#define IRQ_BUTTON_TASK_STACK_SIZE              ( 2048 )
#define IRQ_BUTTON_TASK_PRIORITY                ( 2 )

#define MQTT_PUBLISHER_TASK_STACK_SIZE          ( 8192 )
#define MQTT_PUBLISHER_TASK_PRIORITY            ( 6 )

#define MQTT_BUTTON_PUBLISH_TASK_STACK_SIZE     ( 8192 )
#define MQTT_BUTTON_PUBLISH_TASK_PRIORITY       ( 6 )

#endif