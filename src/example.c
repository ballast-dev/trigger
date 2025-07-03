#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "trigger.h"

// Callback function that gets called when file changes
void on_file_change(const char* filepath, int event_type) {
    printf("File '%s' was %s\n", filepath, trigger_get_event_string(event_type));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filepath>\n", argv[0]);
        printf("Example: %s /tmp/test.txt\n", argv[0]);
        return 1;
    }
    
    const char* filepath = argv[1];
    
    // Create a file watcher
    file_watcher_t* watcher = trigger_create_watcher(filepath, on_file_change);
    if (!watcher) {
        printf("Failed to create file watcher\n");
        return 1;
    }
    
    // Start watching
    if (trigger_start_watching(watcher) != 0) {
        printf("Failed to start watching file '%s'\n", filepath);
        trigger_destroy_watcher(watcher);
        return 1;
    }
    
    printf("Watching file: %s\n", filepath);
    printf("Modify the file to see events. Press Ctrl+C to stop.\n");
    
    // Main loop - wait for file changes
    while (1) {
        int result = trigger_wait_for_changes(watcher);
        if (result < 0) {
            printf("Error waiting for changes\n");
            break;
        }
        // Small delay to prevent busy waiting
        usleep(100000); // 100ms
    }
    
    // Clean up
    trigger_destroy_watcher(watcher);
    return 0;
} 