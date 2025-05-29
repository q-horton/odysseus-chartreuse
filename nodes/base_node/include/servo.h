#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#define PAN_HOME 25
#define TILT_DAM_UP 65
#define TILT_DAM_DOWN 155

void set_pan_angle(uint32_t angle);
void set_tilt_angle(uint32_t angle);

#endif // SERVO_H
