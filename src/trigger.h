#ifndef TRIGGER_H
#define TRIGGER_H
// Get triggered
// ▗   ▘        
// ▜▘▛▘▌▛▌▛▌█▌▛▘
// ▐▖▌ ▌▙▌▙▌▙▖▌ 
//      ▄▌▄▌    
#ifdef __cplusplus
extern "C" {
#endif

// Callback function type for file changes
typedef void (*file_change_callback_t)(const char* filepath, int event_type);

// Event types
#define FILE_EVENT_MODIFIED 1
#define FILE_EVENT_CREATED  2
#define FILE_EVENT_DELETED  3

// Forward declaration of the file watcher structure
typedef struct file_watcher file_watcher_t;

// Create a new file watcher
file_watcher_t* trigger_create_watcher(const char* filepath, file_change_callback_t callback);

// Start watching the file
int trigger_start_watching(file_watcher_t* watcher);

// Stop watching the file
void trigger_stop_watching(file_watcher_t* watcher);

// Check for file changes (non-blocking)
int trigger_check_changes(file_watcher_t* watcher);

// Wait for file changes (blocking)
int trigger_wait_for_changes(file_watcher_t* watcher);

// Destroy the file watcher
void trigger_destroy_watcher(file_watcher_t* watcher);

// Get event type string for debugging
const char* trigger_get_event_string(int event_type);

#ifdef __cplusplus
}
#endif

#endif // TRIGGER_H 