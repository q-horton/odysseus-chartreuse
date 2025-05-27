#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include <stdint.h>

static const struct pwm_dt_spec pan_servo = PWM_DT_SPEC_GET(DT_NODELABEL(pan_servo));
static const struct pwm_dt_spec tilt_servo = PWM_DT_SPEC_GET(DT_NODELABEL(tilt_servo));

#define SERVO_PW_US_MIN_ANGLE 400
#define SERVO_PW_US_MAX_ANGLE 2500

#define ANGLE_MIN 0
#define ANGLE_MAX 180

#define ANGLE_TO_PW(angle) (((angle) - ANGLE_MIN) * (SERVO_PW_US_MAX_ANGLE - SERVO_PW_US_MIN_ANGLE) / (ANGLE_MAX - ANGLE_MIN) + SERVO_PW_US_MIN_ANGLE)

void set_pan_angle(uint32_t angle) {
    if (angle < ANGLE_MIN || angle > ANGLE_MAX) {
        return;
    }

    pwm_set_pulse_dt(&pan_servo, PWM_USEC(ANGLE_TO_PW(angle)));
}

void set_tilt_angle(uint32_t angle) {
    if (angle < ANGLE_MIN || angle > ANGLE_MAX) {
        return;
    }

    pwm_set_pulse_dt(&tilt_servo, PWM_USEC(ANGLE_TO_PW(angle)));
}
