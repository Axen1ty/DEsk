#pragma once

#include <stdbool.h>

struct wl_event_loop;

/* Whether a camera or a microphone is in use (camera.c): the two little
 * lamps on the clock.
 *
 * A camera is a V4L2 device, /dev/video*, and a microphone a sound card's
 * capture end, /dev/snd/pcmC*D*c (under PipeWire or PulseAudio it is the
 * sound server that has it open, and only while something records -- and
 * for the few seconds it takes to let go after). Either is in use while
 * some program has one open -- found the way fuser finds it, from /proc/<pid>/fd,
 * and only among the programs this user may look into, which are the ones
 * on this desk. Looked for on a thread of its own, and only when one of
 * the devices has just been opened or closed (inotify) or one comes or
 * goes, so the desk never waits on it. DESK_DEV stands in for /dev, for
 * the tests. On a system without inotify there is no lamp. */

enum desk_camera {
	DESK_CAMERA_NONE = -1,   /* none of the kind on this machine */
	DESK_CAMERA_IDLE = 0,
	DESK_CAMERA_ON = 1,
};

/* Start looking; changed(data) is called on the compositor's loop each
 * time the answer changes. False if it could not be started. */
bool desk_camera_watch(struct wl_event_loop *loop, void (*changed)(void *data), void *data);
/* The answers as they stand. */
enum desk_camera desk_camera_state(void);
enum desk_camera desk_mic_state(void);
