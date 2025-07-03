#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#ifdef __APPLE__
#include <sys/event.h>
#include <sys/time.h>
#else
#include <sys/inotify.h>
#endif

// Callback function type for file changes
typedef void (*file_change_callback_t)(const char* filepath, int event_type);

// Event types
#define FILE_EVENT_MODIFIED 1
#define FILE_EVENT_CREATED  2
#define FILE_EVENT_DELETED  3

// File watcher structure
typedef struct {
    char* filepath;
    file_change_callback_t callback;
    int is_watching;
    int fd;
#ifdef __APPLE__
    int kq;
    int file_fd;
#else
    int watch_fd;
#endif
} file_watcher_t;

// Create a new file watcher
file_watcher_t* trigger_create_watcher(const char* filepath, file_change_callback_t callback) {
    if (!filepath || !callback) {
        return NULL;
    }
    
    file_watcher_t* watcher = malloc(sizeof(file_watcher_t));
    if (!watcher) {
        return NULL;
    }
    
    watcher->filepath = strdup(filepath);
    watcher->callback = callback;
    watcher->is_watching = 0;
    watcher->fd = -1;
    
#ifdef __APPLE__
    watcher->kq = -1;
    watcher->file_fd = -1;
#else
    watcher->watch_fd = -1;
#endif
    
    return watcher;
}

// Start watching the file
int trigger_start_watching(file_watcher_t* watcher) {
    if (!watcher || watcher->is_watching) {
        return -1;
    }
    
#ifdef __APPLE__
    // Use kqueue on macOS
    watcher->kq = kqueue();
    if (watcher->kq == -1) {
        return -1;
    }
    
    watcher->file_fd = open(watcher->filepath, O_RDONLY);
    if (watcher->file_fd == -1) {
        close(watcher->kq);
        return -1;
    }
    
    struct kevent kev;
    EV_SET(&kev, watcher->file_fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
           NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE,
           0, NULL);
    
    if (kevent(watcher->kq, &kev, 1, NULL, 0, NULL) == -1) {
        close(watcher->file_fd);
        close(watcher->kq);
        return -1;
    }
    
    watcher->fd = watcher->kq;
#else
    // Use inotify on Linux
    watcher->watch_fd = inotify_init();
    if (watcher->watch_fd == -1) {
        return -1;
    }
    
    int wd = inotify_add_watch(watcher->watch_fd, watcher->filepath, 
                              IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE);
    if (wd == -1) {
        close(watcher->watch_fd);
        return -1;
    }
    
    watcher->fd = watcher->watch_fd;
#endif
    
    watcher->is_watching = 1;
    return 0;
}

// Stop watching the file
void trigger_stop_watching(file_watcher_t* watcher) {
    if (!watcher || !watcher->is_watching) {
        return;
    }
    
    if (watcher->fd != -1) {
        close(watcher->fd);
        watcher->fd = -1;
    }
    
#ifdef __APPLE__
    if (watcher->file_fd != -1) {
        close(watcher->file_fd);
        watcher->file_fd = -1;
    }
    watcher->kq = -1;
#else
    watcher->watch_fd = -1;
#endif
    
    watcher->is_watching = 0;
}

// Check for file changes (non-blocking)
int trigger_check_changes(file_watcher_t* watcher) {
    if (!watcher || !watcher->is_watching) {
        return -1;
    }
    
#ifdef __APPLE__
    struct kevent kev;
    struct timespec timeout = {0, 0}; // Non-blocking
    
    int nevents = kevent(watcher->kq, NULL, 0, &kev, 1, &timeout);
    if (nevents > 0) {
        int event_type = FILE_EVENT_MODIFIED;
        if (kev.fflags & NOTE_DELETE) {
            event_type = FILE_EVENT_DELETED;
        } else if (kev.fflags & NOTE_WRITE || kev.fflags & NOTE_EXTEND) {
            event_type = FILE_EVENT_MODIFIED;
        }
        
        watcher->callback(watcher->filepath, event_type);
        return 1;
    }
#else
    char buffer[1024];
    int length = read(watcher->watch_fd, buffer, sizeof(buffer));
    
    if (length > 0) {
        int i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            
            int event_type = FILE_EVENT_MODIFIED;
            if (event->mask & IN_DELETE || event->mask & IN_DELETE_SELF) {
                event_type = FILE_EVENT_DELETED;
            } else if (event->mask & IN_CREATE) {
                event_type = FILE_EVENT_CREATED;
            } else if (event->mask & IN_MODIFY) {
                event_type = FILE_EVENT_MODIFIED;
            }
            
            watcher->callback(watcher->filepath, event_type);
            i += sizeof(struct inotify_event) + event->len;
        }
        return 1;
    }
#endif
    
    return 0;
}

// Wait for file changes (blocking)
int trigger_wait_for_changes(file_watcher_t* watcher) {
    if (!watcher || !watcher->is_watching) {
        return -1;
    }
    
#ifdef __APPLE__
    struct kevent kev;
    struct timespec timeout = {1, 0}; // 1 second timeout
    
    int nevents = kevent(watcher->kq, NULL, 0, &kev, 1, &timeout);
    if (nevents > 0) {
        int event_type = FILE_EVENT_MODIFIED;
        if (kev.fflags & NOTE_DELETE) {
            event_type = FILE_EVENT_DELETED;
        } else if (kev.fflags & NOTE_WRITE || kev.fflags & NOTE_EXTEND) {
            event_type = FILE_EVENT_MODIFIED;
        }
        
        watcher->callback(watcher->filepath, event_type);
        return 1;
    }
#else
    char buffer[1024];
    int length = read(watcher->watch_fd, buffer, sizeof(buffer));
    
    if (length > 0) {
        int i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            
            int event_type = FILE_EVENT_MODIFIED;
            if (event->mask & IN_DELETE || event->mask & IN_DELETE_SELF) {
                event_type = FILE_EVENT_DELETED;
            } else if (event->mask & IN_CREATE) {
                event_type = FILE_EVENT_CREATED;
            } else if (event->mask & IN_MODIFY) {
                event_type = FILE_EVENT_MODIFIED;
            }
            
            watcher->callback(watcher->filepath, event_type);
            i += sizeof(struct inotify_event) + event->len;
        }
        return 1;
    }
#endif
    
    return 0;
}

// Destroy the file watcher
void trigger_destroy_watcher(file_watcher_t* watcher) {
    if (!watcher) {
        return;
    }
    
    trigger_stop_watching(watcher);
    
    if (watcher->filepath) {
        free(watcher->filepath);
    }
    
    free(watcher);
}

// Get event type string for debugging
const char* trigger_get_event_string(int event_type) {
    switch (event_type) {
        case FILE_EVENT_MODIFIED:
            return "MODIFIED";
        case FILE_EVENT_CREATED:
            return "CREATED";
        case FILE_EVENT_DELETED:
            return "DELETED";
        default:
            return "UNKNOWN";
    }
}
