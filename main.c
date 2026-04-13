#include <Carbon/Carbon.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((format(printf, 1, 2)))
void print_error(const char* fmt, ...) {
    static int use_color = -1;
    if (use_color < 0) {
        const char* no_color = getenv("NO_COLOR");
        use_color = (no_color && no_color[0] != '\0') ? 0 : 1;
    }
    fputs(use_color ? "\033[1;31merror:\033[0m " : "error: ", stderr);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void print_usage(FILE* out, const char* prog_name) {
    fprintf(out, "Usage: %s <command> [args]\n", prog_name);
    fputs("\n", out);
    fputs("Commands:\n", out);
    fputs("  get         Print the current keyboard input source ID\n", out);
    fputs("  set <ID>    Switch to the specified input source ID\n", out);
    fputs("  list        List enabled keyboard input source IDs\n", out);
    fputs("  help        Print this help message\n", out);
}

CFArrayRef copy_input_sources_matching(CFStringRef property_key, CFTypeRef property_value) {
    const void* keys[] = {property_key};
    const void* values[] = {property_value};
    auto filter = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!filter) return NULL;
    auto list = TISCreateInputSourceList(filter, false);
    CFRelease(filter);
    return list;
}

int cmd_get(void) {
    auto current = TISCopyCurrentKeyboardInputSource();
    if (!current) {
        print_error("failed to get current keyboard input source");
        return EXIT_FAILURE;
    }

    auto source_id = (CFStringRef)TISGetInputSourceProperty(current, kTISPropertyInputSourceID);
    if (!source_id) {
        print_error("current input source has no ID");
        CFRelease(current);
        return EXIT_FAILURE;
    }

    char buffer[256] = {0};
    if (!CFStringGetCString(source_id, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
        print_error("failed to convert input source ID to UTF-8");
        CFRelease(current);
        return EXIT_FAILURE;
    }
    puts(buffer);

    CFRelease(current);
    return EXIT_SUCCESS;
}

int cmd_set(const char* target_id_cstr) {
    auto target_id = CFStringCreateWithCString(kCFAllocatorDefault, target_id_cstr, kCFStringEncodingUTF8);
    if (!target_id) {
        print_error("failed to encode input source ID");
        return EXIT_FAILURE;
    }

    auto source_list = copy_input_sources_matching(kTISPropertyInputSourceID, target_id);
    CFRelease(target_id);
    if (!source_list || CFArrayGetCount(source_list) == 0) {
        print_error("input source '%s' not found", target_id_cstr);
        if (source_list) CFRelease(source_list);
        return EXIT_FAILURE;
    }

    auto target = (TISInputSourceRef)CFArrayGetValueAtIndex(source_list, 0);
    auto status = TISSelectInputSource(target);
    CFRelease(source_list);

    if (status != noErr) {
        print_error("failed to select input source '%s' (OSStatus %d)", target_id_cstr, (int)status);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int cmd_list(void) {
    auto source_list = copy_input_sources_matching(kTISPropertyInputSourceCategory, kTISCategoryKeyboardInputSource);
    if (!source_list) {
        print_error("failed to enumerate input sources");
        return EXIT_FAILURE;
    }

    auto count = CFArrayGetCount(source_list);
    for (CFIndex i = 0; i < count; i++) {
        auto source = (TISInputSourceRef)CFArrayGetValueAtIndex(source_list, i);
        auto selectable = (CFBooleanRef)TISGetInputSourceProperty(source, kTISPropertyInputSourceIsSelectCapable);
        auto source_id = (CFStringRef)TISGetInputSourceProperty(source, kTISPropertyInputSourceID);
        if (selectable == kCFBooleanFalse || !source_id) continue;
        char buffer[256] = {0};
        if (CFStringGetCString(source_id, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
            puts(buffer);
        }
    }

    CFRelease(source_list);
    return EXIT_SUCCESS;
}


int main(int argc, const char* argv[]) {
    if (argc < 2) {
        print_error("missing command");
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    const char* cmd = argv[1];

    if (strcmp(cmd, "get") == 0) {
        if (argc != 2) {
            print_error("'get' takes no arguments");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_get();
    }

    if (strcmp(cmd, "set") == 0) {
        if (argc != 3) {
            print_error("'set' requires exactly one input source ID");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_set(argv[2]);
    }

    if (strcmp(cmd, "list") == 0) {
        if (argc != 2) {
            print_error("'list' takes no arguments");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_list();
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0) {
        print_usage(stdout, argv[0]);
        return EXIT_SUCCESS;
    }

    print_error("unknown command '%s'", cmd);
    print_usage(stderr, argv[0]);
    return EXIT_FAILURE;
}
