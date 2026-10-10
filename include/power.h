#ifndef DESK_POWER_H
#define DESK_POWER_H

#include <stdbool.h>

/* What the three buttons under the glass do.
 *
 * Only the first two are here. Logging out is the compositor stopping,
 * which it can do by itself and does not need to ask anybody about. */
enum desk_power {
	DESK_POWER_OFF,
	DESK_POWER_REBOOT,
};

/* Ask logind. Returns false if it would not, or could not be reached --
 * the caller is then holding a dark screen and had better say so. */
bool desk_power_request(enum desk_power what);

/* Turn the screen's backlight (panel.c's choice of device) to v, 0..1 of
 * its scale -- never quite to nothing, which is a black screen with no way
 * to see to turn it back up. Through logind, which lets the user at the
 * seat set it; or else straight into sysfs, where that file is writable
 * (a udev rule, or DESK_SYSFS's tree when trying it). False if neither
 * would have it. */
bool desk_backlight_set(double v);

#endif
