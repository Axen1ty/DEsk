/* Ending the session.
 *
 * Through logind, which is the only thing on a systemd machine that will
 * take this from an unprivileged process: it knows the asking session is
 * the active one on this seat, and that is the whole of the permission.
 * PolicyKit may still put a password in the way, which is its business and
 * not ours -- we get told no, and say so.
 *
 * Where there is no libsystemd to build against, the same request goes out
 * through systemctl, which does the same thing the long way round. */

#include "power.h"
#include "panel.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#ifdef DESK_HAVE_POST
#include <systemd/sd-bus.h>
#endif

#ifdef DESK_HAVE_POST
bool desk_power_request(enum desk_power what)
{
	sd_bus *bus = NULL;
	if (sd_bus_open_system(&bus) < 0) {
		return false;
	}
	sd_bus_error err = SD_BUS_ERROR_NULL;
	/* false: do not ask us to be polite about inhibitors. Whoever pushed
	 * the button had to turn a key and lift a lid to get at it. */
	int r = sd_bus_call_method(bus,
		"org.freedesktop.login1", "/org/freedesktop/login1",
		"org.freedesktop.login1.Manager",
		what == DESK_POWER_REBOOT ? "Reboot" : "PowerOff",
		&err, NULL, "b", 0);
	sd_bus_error_free(&err);
	sd_bus_unref(bus);
	return r >= 0;
}
#else
bool desk_power_request(enum desk_power what)
{
	pid_t pid = fork();
	if (pid < 0) {
		return false;
	}
	if (pid == 0) {
		setsid();
		execlp("systemctl", "systemctl",
			what == DESK_POWER_REBOOT ? "reboot" : "poweroff", (char *)NULL);
		_exit(127);
	}
	int st = 0;
	waitpid(pid, &st, 0);
	return WIFEXITED(st) && WEXITSTATUS(st) == 0;
}
#endif

/* ---- the screen's light ---- */

#ifdef DESK_HAVE_POST
static bool backlight_logind(const char *name, uint32_t raw)
{
	static sd_bus *bus;
	static bool tried;
	if (!bus && !tried) {
		tried = true;
		if (sd_bus_open_system(&bus) < 0) {
			bus = NULL;
		}
	}
	if (!bus) {
		return false;
	}
	sd_bus_error err = SD_BUS_ERROR_NULL;
	int r = sd_bus_call_method(bus, "org.freedesktop.login1",
		"/org/freedesktop/login1/session/auto", "org.freedesktop.login1.Session",
		"SetBrightness", &err, NULL, "ssu", "backlight", name, raw);
	sd_bus_error_free(&err);
	return r >= 0;
}
#endif

bool desk_backlight_set(double v)
{
	char name[256];
	double max, now;
	if (!desk_backlight_find(name, sizeof name, &max, &now)) {
		return false;
	}
	v = v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
	double low = fmax(1.0, ceil(max * 0.01));
	uint32_t raw = (uint32_t)fmax(low, lround(v * max));
	const char *test = getenv("DESK_SYSFS");
#ifdef DESK_HAVE_POST
	/* a tree of our own is ours to write; the real one is the seat's */
	if (!(test && *test) && backlight_logind(name, raw)) {
		return true;
	}
#else
	(void)test;
#endif
	char path[1024];
	snprintf(path, sizeof path, "%s/class/backlight/%s/brightness", desk_sys_root(), name);
	FILE *f = fopen(path, "we");
	if (!f) {
		return false;
	}
	bool ok = fprintf(f, "%u\n", (unsigned)raw) > 0;
	ok = fclose(f) == 0 && ok;
	return ok;
}
