/* Whether a camera or a microphone is in use (camera.h). */

/* pipe2 and readlinkat's company are glibc's and the BSDs', past POSIX */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include "camera.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wayland-server-core.h>

#ifdef __linux__
#include <sys/inotify.h>
#endif

static atomic_int g_state[2] = { DESK_CAMERA_NONE, DESK_CAMERA_NONE };

enum desk_camera desk_camera_state(void)
{
	return (enum desk_camera)atomic_load(&g_state[0]);
}

enum desk_camera desk_mic_state(void)
{
	return (enum desk_camera)atomic_load(&g_state[1]);
}

#ifdef __linux__

/* where each kind is, as /proc spells it: the cameras in /dev, and the
 * sound cards' capture ends in /dev/snd */
static char g_dir[2][PATH_MAX];
static int g_in = -1;            /* inotify */
static int g_wake[2] = { -1, -1 };
static void (*g_changed)(void *data);
static void *g_data;

/* video0, video1, ...; and pcmC0D0c, a card's capture -- not pcmC0D0p,
 * which only plays */
static bool is_kind(int k, const char *name)
{
	if (k == 0) {
		return strncmp(name, "video", 5) == 0 && name[5] >= '0' && name[5] <= '9';
	}
	size_t n = strlen(name);
	return strncmp(name, "pcmC", 4) == 0 && n > 4 && name[n - 1] == 'c';
}

/* Watch every device of the kind there is now for being opened and
 * closed, and say whether there is one. Watching one already watched only
 * says so again; one that has gone took its watch with it. */
static bool watch_devices(int k)
{
	DIR *d = opendir(g_dir[k]);
	if (!d) {
		return false;
	}
	bool any = false;
	struct dirent *e;
	while ((e = readdir(d))) {
		if (!is_kind(k, e->d_name)) {
			continue;
		}
		char path[PATH_MAX + 288];
		snprintf(path, sizeof path, "%s/%s", g_dir[k], e->d_name);
		inotify_add_watch(g_in, path, IN_OPEN | IN_CLOSE);
		any = true;
	}
	closedir(d);
	return any;
}

/* Which kinds any program this user may look into has one of open. */
static void any_open(bool open_now[2])
{
	size_t nd[2] = { strlen(g_dir[0]), strlen(g_dir[1]) };
	pid_t self = getpid();
	open_now[0] = open_now[1] = false;
	DIR *p = opendir("/proc");
	if (!p) {
		return;
	}
	struct dirent *e;
	while (!(open_now[0] && open_now[1]) && (e = readdir(p))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9' || atoi(e->d_name) == self) {
			continue;
		}
		char dn[288];
		snprintf(dn, sizeof dn, "/proc/%s/fd", e->d_name);
		/* someone else's: not ours to read, and it says so at once */
		int dfd = open(dn, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
		if (dfd < 0) {
			continue;
		}
		DIR *f = fdopendir(dfd);
		if (!f) {
			close(dfd);
			continue;
		}
		struct dirent *g;
		while ((g = readdir(f))) {
			if (g->d_name[0] == '.') {
				continue;
			}
			char l[PATH_MAX];
			ssize_t n = readlinkat(dfd, g->d_name, l, sizeof l - 1);
			if (n <= 0) {
				continue;
			}
			l[n] = '\0';
			for (int k = 0; k < 2; k++) {
				if ((size_t)n > nd[k] && strncmp(l, g_dir[k], nd[k]) == 0
					&& l[nd[k]] == '/' && is_kind(k, l + nd[k] + 1)) {
					open_now[k] = true;
				}
			}
		}
		closedir(f);
	}
	closedir(p);
}

static void drain(void)
{
	char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
	while (read(g_in, buf, sizeof buf) > 0) {
	}
}

static void *watcher(void *arg)
{
	(void)arg;
	for (;;) {
		if (strcmp(g_dir[1], "/nonexistent") == 0) {
			/* the first sound card, come since */
			char snd[PATH_MAX + 8];
			snprintf(snd, sizeof snd, "%s/snd", g_dir[0]);
			if (realpath(snd, g_dir[1])) {
				inotify_add_watch(g_in, g_dir[1], IN_CREATE | IN_DELETE | IN_MOVED_TO
					| IN_MOVED_FROM | IN_ONLYDIR);
			} else {
				snprintf(g_dir[1], sizeof g_dir[1], "%s", "/nonexistent");
			}
		}
		bool have[2] = { watch_devices(0), watch_devices(1) };
		bool on[2] = { false, false };
		if (have[0] || have[1]) {
			any_open(on);
		}
		bool told = false;
		for (int k = 0; k < 2; k++) {
			int now = !have[k] ? DESK_CAMERA_NONE : on[k] ? DESK_CAMERA_ON
				: DESK_CAMERA_IDLE;
			if (atomic_exchange(&g_state[k], now) != now && !told) {
				char c = 1;
				if (write(g_wake[1], &c, 1) < 0) {
					/* full already: it has been told */
				}
				told = true;
			}
		}
		struct pollfd pf = { g_in, POLLIN, 0 };
		if (poll(&pf, 1, -1) < 0 && errno != EINTR) {
			return NULL;
		}
		/* Then wait for it to be quiet a moment: a program looking at
		 * what devices there are opens and closes each in a blink, and
		 * that is not the camera being on. */
		do {
			drain();
			pf.revents = 0;
		} while (poll(&pf, 1, 250) > 0);
	}
	return NULL;
}

static int woken(int fd, uint32_t mask, void *data)
{
	(void)mask;
	(void)data;
	char buf[64];
	while (read(fd, buf, sizeof buf) > 0) {
	}
	if (g_changed) {
		g_changed(g_data);
	}
	return 0;
}

bool desk_camera_watch(struct wl_event_loop *loop, void (*changed)(void *data), void *data)
{
	if (g_in >= 0) {
		return true;
	}
	const char *dev = getenv("DESK_DEV");
	if (!realpath(dev && *dev ? dev : "/dev", g_dir[0])) {
		return false;
	}
	/* no sound cards: no microphone, and a directory nothing is in */
	char snd[PATH_MAX + 8];
	snprintf(snd, sizeof snd, "%s/snd", g_dir[0]);
	if (!realpath(snd, g_dir[1])) {
		snprintf(g_dir[1], sizeof g_dir[1], "%s", "/nonexistent");
	}
	g_in = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if (g_in < 0) {
		return false;
	}
	const uint32_t dir_mask = IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM | IN_ONLYDIR;
	if (inotify_add_watch(g_in, g_dir[0], dir_mask) < 0
		|| pipe2(g_wake, O_NONBLOCK | O_CLOEXEC) < 0) {
		close(g_in);
		g_in = -1;
		return false;
	}
	/* a card plugged in after: /dev/snd comes and goes with them */
	inotify_add_watch(g_in, g_dir[1], dir_mask);
	g_changed = changed;
	g_data = data;
	wl_event_loop_add_fd(loop, g_wake[0], WL_EVENT_READABLE, woken, NULL);
	/* the thread hears no signals: they are the compositor's */
	sigset_t all, old;
	sigfillset(&all);
	pthread_sigmask(SIG_SETMASK, &all, &old);
	pthread_t t;
	pthread_attr_t at;
	pthread_attr_init(&at);
	pthread_attr_setdetachstate(&at, PTHREAD_CREATE_DETACHED);
	int err = pthread_create(&t, &at, watcher, NULL);
	pthread_attr_destroy(&at);
	pthread_sigmask(SIG_SETMASK, &old, NULL);
	return err == 0;
}

#else

bool desk_camera_watch(struct wl_event_loop *loop, void (*changed)(void *data), void *data)
{
	(void)loop;
	(void)changed;
	(void)data;
	return false;
}

#endif
