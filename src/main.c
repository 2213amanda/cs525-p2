
#define _POSIX_C_SOURCE 200809L

#include "lab.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST
#define main main_exclude
#endif

int main(int argc, char *argv[])
{
    const char *mode;
    const char *session = NULL;
    const char *relay;
    const char *filename;
    const char *port = "4250";

    char port_buffer[6];

    uint32_t window = 8U;
    uint32_t timeout_ms = 250U;

    double loss = 0.0;
    double corrupt = 0.0;
    double dup = 0.0;

    int index = 2;
    int result;

    /* No arguments: print usage and exit successfully. */
    if (argc == 1) {
        printUsage();
        return 0;
    }

    /* Validate the mode. */
    if (argc < 2 ||
        (strcmp(argv[1], "send") != 0 &&
         strcmp(argv[1], "recv") != 0)) {
        fprintf(stderr, "Error: mode must be 'send' or 'recv'.\n");
        printUsage();
        return 1;
    }

    mode = argv[1];

    /* Parse command-line options. */
    while (index < argc && argv[index][0] == '-') {
        const char *option = argv[index];

        if (strcmp(option, "-s") == 0) {
            if (index + 1 >= argc || session != NULL) {
                fprintf(stderr, "Error: invalid -s option.\n");
                printUsage();
                return 1;
            }

            session = argv[index + 1];
            index += 2;
        } else if (strcmp(option, "-p") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc || argv[index + 1][0] == '\0') {
                fprintf(stderr, "Error: -p requires a port.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 1UL || value > 65535UL) {
                fprintf(stderr, "Error: invalid port.\n");
                printUsage();
                return 1;
            }

            (void)snprintf(port_buffer, sizeof(port_buffer),
                           "%lu", value);
            port = port_buffer;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   strcmp(option, "-w") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc) {
                fprintf(stderr, "Error: -w requires a window size.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 1UL ||
                value > (unsigned long)MAX_WINDOW) {
                fprintf(stderr, "Error: window must be between 1 and %u.\n",
                        (unsigned int)MAX_WINDOW);
                printUsage();
                return 1;
            }

            window = (uint32_t)value;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   strcmp(option, "-T") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc) {
                fprintf(stderr, "Error: -T requires a timeout.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value == 0UL ||
                value > UINT32_MAX) {
                fprintf(stderr, "Error: timeout must be positive.\n");
                printUsage();
                return 1;
            }

            timeout_ms = (uint32_t)value;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   (strcmp(option, "-l") == 0 ||
                    strcmp(option, "-c") == 0 ||
                    strcmp(option, "-d") == 0)) {
            char *end = NULL;
            double value;

            if (index + 1 >= argc || argv[index + 1][0] == '\0') {
                fprintf(stderr, "Error: %s requires a probability.\n",
                        option);
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtod(argv[index + 1], &end);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 0.0 || value > 0.5) {
                fprintf(stderr,
                        "Error: %s probability must be between 0 and 0.5.\n",
                        option);
                printUsage();
                return 1;
            }

            if (strcmp(option, "-l") == 0) {
                loss = value;
            } else if (strcmp(option, "-c") == 0) {
                corrupt = value;
            } else {
                dup = value;
            }

            index += 2;
        } else {
            fprintf(stderr, "Error: unknown option '%s'.\n", option);
            printUsage();
            return 1;
        }
    }

    /* Validate the required session name. */
    if (session == NULL) {
        fprintf(stderr, "Error: -s is required.\n");
        printUsage();
        return 1;
    }

    {
        size_t length = strlen(session);

        if (length < 1U || length > 32U) {
            fprintf(stderr, "Error: invalid session length.\n");
            return 1;
        }

        for (size_t i = 0; i < length; i++) {
            char c = session[i];

            if (!((c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') || c == '-')) {
                fprintf(stderr, "Error: invalid session characters.\n");
                return 1;
            }
        }
    }

    /* Exactly two positional arguments must remain. */
    if (argc - index != 2) {
        fprintf(stderr, "Error: expected relay and filename.\n");
        printUsage();
        return 1;
    }

    relay = argv[index];
    filename = argv[index + 1];

    if (relay[0] == '\0' || filename[0] == '\0') {
        fprintf(stderr, "Error: relay and filename cannot be empty.\n");
        return 1;
    }

    /*
     * Start the requested transfer.
     * The run functions return zero on success and nonzero on failure.
     */
    if (strcmp(mode, "send") == 0) {
        result = runSender(relay, port, session, filename,
                           window, timeout_ms, loss, corrupt, dup);
    } else {
        result = runReceiver(relay, port, session, filename);
    }

    if (result != 0) {
        fprintf(stderr, "%s transfer failed.\n",
                strcmp(mode, "send") == 0 ? "Sender" : "Receiver");
        return 2;
    }

    return 0;
}
