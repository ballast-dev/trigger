# Get Triggered

A `C` library for watching files and directories for changes and calling callbacks when events occur.

## Features

- **Cross-platform**: Works on macOS (using kqueue) and Linux (using inotify)
- **Simple API**: Easy-to-use callback-based interface
- **Multiple event types**: Detects file modifications, creations, and deletions
- **Non-blocking and blocking modes**: Choose between polling and waiting for events
- **Memory safe**: Proper cleanup and error handling

## Building

`just build`

## Usage

### Basic Example

```c
#include "trigger.h"
#include <stdio.h>

void on_file_change(const char* filepath, int event_type) {
    printf("File '%s' was %s\n", filepath, trigger_get_event_string(event_type));
}

int main() {
    // Create a file watcher
    file_watcher_t* watcher = trigger_create_watcher("/path/to/file.txt", on_file_change);
    
    // Start watching
    trigger_start_watching(watcher);
    
    // Wait for changes (blocking)
    while (1) {
        trigger_wait_for_changes(watcher);
    }
    
    // Clean up
    trigger_destroy_watcher(watcher);
    return 0;
}
```

### API Reference

#### Functions

- `trigger_create_watcher(filepath, callback)` - Create a new file watcher
- `trigger_start_watching(watcher)` - Start watching the file
- `trigger_stop_watching(watcher)` - Stop watching the file
- `trigger_check_changes(watcher)` - Check for changes (non-blocking)
- `trigger_wait_for_changes(watcher)` - Wait for changes (blocking)
- `trigger_destroy_watcher(watcher)` - Destroy the watcher and free memory
- `trigger_get_event_string(event_type)` - Get human-readable event type

#### Event Types

- `FILE_EVENT_MODIFIED` - File was modified
- `FILE_EVENT_CREATED` - File was created
- `FILE_EVENT_DELETED` - File was deleted

#### Callback Function

```c
typedef void (*file_change_callback_t)(const char* filepath, int event_type);
```

The callback receives:
- `filepath`: The path of the file that changed
- `event_type`: The type of event that occurred

### Running the Example

```bash
# Build the example
cd build
make

# Run the example (replace with your file path)
./trigger_example /tmp/test.txt

# In another terminal, modify the file to see events
echo "test" > /tmp/test.txt
```

## Platform Support

- **macOS**: Uses kqueue for efficient file system monitoring
- **Linux**: Uses inotify for file system monitoring
- **Other Unix-like systems**: May work with inotify if available

## Error Handling

All functions return appropriate error codes:
- `0` on success
- `-1` on error
- `1` when changes are detected (for check/wait functions)

Check return values and handle errors appropriately in your application.

## Thread Safety

The library is not thread-safe. If you need to use it from multiple threads, you must implement your own synchronization.
