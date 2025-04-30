#ifndef NATIVE_CRASH_HANDLER_H
#define NATIVE_CRASH_HANDLER_H

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <sys/stat.h>
#include <errno.h>

#define CRASH_LOG_TAG "NativeCrash"
// Use a more reliable path similar to the Java CrashHandler
#define CRASH_LOG_PATH "/storage/emulated/0/Documents/mixmod_native_crash_%s.txt"
#define CRASH_LOG_DIR "/storage/emulated/0/Documents/"

// Device information
static const char* get_android_property(const char* name) {
    static char property_value[PROP_VALUE_MAX + 1];
    __system_property_get(name, property_value);
    return property_value;
}

// Structure to store the previous signal handlers
struct SignalHandlers {
    struct sigaction prev_sigsegv;
    struct sigaction prev_sigbus;
    struct sigaction prev_sigabrt;
    struct sigaction prev_sigill;
    struct sigaction prev_sigfpe;
};

static SignalHandlers g_prev_signal_handlers;

// Function to get the current time as a string
static void get_current_time(char* buffer, size_t buffer_size) {
    time_t raw_time;
    struct tm* time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", time_info);
}

// Function to ensure directory exists
static int ensure_directory_exists(const char* dir_path) {
    struct stat st;

    // Check if directory already exists
    if (stat(dir_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0; // Directory exists
        }
        return -1; // Path exists but is not a directory
    }

    // Create directory with permissions 0755 (rwxr-xr-x)
    if (mkdir(dir_path, 0755) == 0) {
        __android_log_print(ANDROID_LOG_INFO, CRASH_LOG_TAG, "Created directory: %s", dir_path);
        return 0;
    }

    // Failed to create directory
    int err = errno;
    char err_msg[256];
    strerror_r(err, err_msg, sizeof(err_msg));
    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG, "Failed to create directory: %s (errno: %d, error: %s)", 
                       dir_path, err, err_msg);
    return -1;
}

// Function to get the backtrace
static void get_backtrace(int fd) {
    // Note: backtrace() and backtrace_symbols() are not available in Android NDK
    // This is a simplified version that just logs the unavailability
    dprintf(fd, "Backtrace functionality not available on this platform\n");
    dprintf(fd, "For native crashes, check system logcat for more details\n");
}

// Signal handler for crashes
static void crash_signal_handler(int sig, siginfo_t* info, void* context) {
    // Get the current time for the filename
    char time_buffer[64];
    get_current_time(time_buffer, sizeof(time_buffer));

    // Replace spaces and colons with underscores for the filename
    for (char* p = time_buffer; *p; p++) {
        if (*p == ' ' || *p == ':') *p = '_';
    }

    // Create the log file path with the timestamp
    char log_path[256];
    snprintf(log_path, sizeof(log_path), CRASH_LOG_PATH, time_buffer);

    // Ensure the directory exists
    if (ensure_directory_exists(CRASH_LOG_DIR) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG, "Failed to ensure directory exists, will try to create file anyway");
    }

    // Open the crash log file
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        // Get the error code and message
        int err = errno;
        char err_msg[256];
        strerror_r(err, err_msg, sizeof(err_msg));

        // Log detailed error information to console
        __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG, "Failed to open crash log file: %s (errno: %d, error: %s)", 
                           log_path, err, err_msg);

        // Call the previous signal handler
        switch (sig) {
            case SIGSEGV:
                if (g_prev_signal_handlers.prev_sigsegv.sa_flags & SA_SIGINFO) {
                    g_prev_signal_handlers.prev_sigsegv.sa_sigaction(sig, info, context);
                } else if (g_prev_signal_handlers.prev_sigsegv.sa_handler != SIG_DFL && 
                           g_prev_signal_handlers.prev_sigsegv.sa_handler != SIG_IGN) {
                    g_prev_signal_handlers.prev_sigsegv.sa_handler(sig);
                }
                break;
            case SIGBUS:
                if (g_prev_signal_handlers.prev_sigbus.sa_flags & SA_SIGINFO) {
                    g_prev_signal_handlers.prev_sigbus.sa_sigaction(sig, info, context);
                } else if (g_prev_signal_handlers.prev_sigbus.sa_handler != SIG_DFL && 
                           g_prev_signal_handlers.prev_sigbus.sa_handler != SIG_IGN) {
                    g_prev_signal_handlers.prev_sigbus.sa_handler(sig);
                }
                break;
            case SIGABRT:
                if (g_prev_signal_handlers.prev_sigabrt.sa_flags & SA_SIGINFO) {
                    g_prev_signal_handlers.prev_sigabrt.sa_sigaction(sig, info, context);
                } else if (g_prev_signal_handlers.prev_sigabrt.sa_handler != SIG_DFL && 
                           g_prev_signal_handlers.prev_sigabrt.sa_handler != SIG_IGN) {
                    g_prev_signal_handlers.prev_sigabrt.sa_handler(sig);
                }
                break;
            case SIGILL:
                if (g_prev_signal_handlers.prev_sigill.sa_flags & SA_SIGINFO) {
                    g_prev_signal_handlers.prev_sigill.sa_sigaction(sig, info, context);
                } else if (g_prev_signal_handlers.prev_sigill.sa_handler != SIG_DFL && 
                           g_prev_signal_handlers.prev_sigill.sa_handler != SIG_IGN) {
                    g_prev_signal_handlers.prev_sigill.sa_handler(sig);
                }
                break;
            case SIGFPE:
                if (g_prev_signal_handlers.prev_sigfpe.sa_flags & SA_SIGINFO) {
                    g_prev_signal_handlers.prev_sigfpe.sa_sigaction(sig, info, context);
                } else if (g_prev_signal_handlers.prev_sigfpe.sa_handler != SIG_DFL && 
                           g_prev_signal_handlers.prev_sigfpe.sa_handler != SIG_IGN) {
                    g_prev_signal_handlers.prev_sigfpe.sa_handler(sig);
                }
                break;
        }
        return;
    }

    // Get the current time
    char current_time_buffer[64];
    get_current_time(current_time_buffer, sizeof(current_time_buffer));

    // Write crash information to the file
    dprintf(fd, "=== Native Crash Report ===\n");
    dprintf(fd, "Time: %s\n", current_time_buffer);
    dprintf(fd, "Signal: %d (%s)\n", sig, strsignal(sig));
    dprintf(fd, "Fault address: %p\n", info->si_addr);

    // Write device information
    dprintf(fd, "\n=== Device Information ===\n");
    dprintf(fd, "Device Manufacturer: %s\n", get_android_property("ro.product.manufacturer"));
    dprintf(fd, "Device Model: %s\n", get_android_property("ro.product.model"));
    dprintf(fd, "Android Version: %s\n", get_android_property("ro.build.version.release"));
    dprintf(fd, "Android SDK: %s\n", get_android_property("ro.build.version.sdk"));
    dprintf(fd, "Build Fingerprint: %s\n", get_android_property("ro.build.fingerprint"));

    // Get and write the backtrace
    dprintf(fd, "\n=== Backtrace ===\n");
    get_backtrace(fd);

    // Close the file
    close(fd);

    // Log the crash
    __android_log_print(ANDROID_LOG_ERROR, CRASH_LOG_TAG, "Native crash detected. Signal: %d (%s). Log saved to %s", 
                        sig, strsignal(sig), log_path);

    // Call the previous signal handler
    switch (sig) {
        case SIGSEGV:
            if (g_prev_signal_handlers.prev_sigsegv.sa_flags & SA_SIGINFO) {
                g_prev_signal_handlers.prev_sigsegv.sa_sigaction(sig, info, context);
            } else if (g_prev_signal_handlers.prev_sigsegv.sa_handler != SIG_DFL && 
                       g_prev_signal_handlers.prev_sigsegv.sa_handler != SIG_IGN) {
                g_prev_signal_handlers.prev_sigsegv.sa_handler(sig);
            }
            break;
        case SIGBUS:
            if (g_prev_signal_handlers.prev_sigbus.sa_flags & SA_SIGINFO) {
                g_prev_signal_handlers.prev_sigbus.sa_sigaction(sig, info, context);
            } else if (g_prev_signal_handlers.prev_sigbus.sa_handler != SIG_DFL && 
                       g_prev_signal_handlers.prev_sigbus.sa_handler != SIG_IGN) {
                g_prev_signal_handlers.prev_sigbus.sa_handler(sig);
            }
            break;
        case SIGABRT:
            if (g_prev_signal_handlers.prev_sigabrt.sa_flags & SA_SIGINFO) {
                g_prev_signal_handlers.prev_sigabrt.sa_sigaction(sig, info, context);
            } else if (g_prev_signal_handlers.prev_sigabrt.sa_handler != SIG_DFL && 
                       g_prev_signal_handlers.prev_sigabrt.sa_handler != SIG_IGN) {
                g_prev_signal_handlers.prev_sigabrt.sa_handler(sig);
            }
            break;
        case SIGILL:
            if (g_prev_signal_handlers.prev_sigill.sa_flags & SA_SIGINFO) {
                g_prev_signal_handlers.prev_sigill.sa_sigaction(sig, info, context);
            } else if (g_prev_signal_handlers.prev_sigill.sa_handler != SIG_DFL && 
                       g_prev_signal_handlers.prev_sigill.sa_handler != SIG_IGN) {
                g_prev_signal_handlers.prev_sigill.sa_handler(sig);
            }
            break;
        case SIGFPE:
            if (g_prev_signal_handlers.prev_sigfpe.sa_flags & SA_SIGINFO) {
                g_prev_signal_handlers.prev_sigfpe.sa_sigaction(sig, info, context);
            } else if (g_prev_signal_handlers.prev_sigfpe.sa_handler != SIG_DFL && 
                       g_prev_signal_handlers.prev_sigfpe.sa_handler != SIG_IGN) {
                g_prev_signal_handlers.prev_sigfpe.sa_handler(sig);
            }
            break;
    }
}

// Function to initialize the native crash handler
static void init_native_crash_handler() {
    struct sigaction handler;
    memset(&handler, 0, sizeof(handler));
    handler.sa_sigaction = crash_signal_handler;
    handler.sa_flags = SA_SIGINFO;

    // Register signal handlers for common crash signals
    sigaction(SIGSEGV, &handler, &g_prev_signal_handlers.prev_sigsegv);
    sigaction(SIGBUS, &handler, &g_prev_signal_handlers.prev_sigbus);
    sigaction(SIGABRT, &handler, &g_prev_signal_handlers.prev_sigabrt);
    sigaction(SIGILL, &handler, &g_prev_signal_handlers.prev_sigill);
    sigaction(SIGFPE, &handler, &g_prev_signal_handlers.prev_sigfpe);

    __android_log_print(ANDROID_LOG_INFO, CRASH_LOG_TAG, "Native crash handler initialized");
}

#endif // NATIVE_CRASH_HANDLER_H
