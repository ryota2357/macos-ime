#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        fprintf(stderr, "Error: failed to get current keyboard input source\n");
        return EXIT_FAILURE;
    }

    auto source_id = (CFStringRef)TISGetInputSourceProperty(current, kTISPropertyInputSourceID);
    if (!source_id) {
        fprintf(stderr, "Error: current input source has no ID\n");
        CFRelease(current);
        return EXIT_FAILURE;
    }

    char buffer[256] = {0};
    if (!CFStringGetCString(source_id, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
        fprintf(stderr, "Error: failed to convert input source ID to UTF-8\n");
        CFRelease(current);
        return EXIT_FAILURE;
    }
    puts(buffer);

    CFRelease(current);
    return EXIT_SUCCESS;
}

int cmd_list(void) {
    auto source_list = copy_input_sources_matching(kTISPropertyInputSourceCategory, kTISCategoryKeyboardInputSource);
    if (!source_list) {
        fprintf(stderr, "Error: failed to enumerate input sources\n");
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

int cmd_set(const char* target_id_cstr) {
    auto target_id = CFStringCreateWithCString(kCFAllocatorDefault, target_id_cstr, kCFStringEncodingUTF8);
    if (!target_id) {
        fprintf(stderr, "Error: failed to encode input source ID\n");
        return EXIT_FAILURE;
    }

    auto source_list = copy_input_sources_matching(kTISPropertyInputSourceID, target_id);
    CFRelease(target_id);
    if (!source_list || CFArrayGetCount(source_list) == 0) {
        fprintf(stderr, "Error: input source '%s' not found\n", target_id_cstr);
        if (source_list) CFRelease(source_list);
        return EXIT_FAILURE;
    }

    auto target = (TISInputSourceRef)CFArrayGetValueAtIndex(source_list, 0);
    auto status = TISSelectInputSource(target);
    CFRelease(source_list);

    if (status != noErr) {
        fprintf(stderr, "Error: failed to select input source '%s' (OSStatus %d)\n", target_id_cstr, (int)status);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: missing command\n");
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    const char* cmd = argv[1];

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0) {
        print_usage(stdout, argv[0]);
        return EXIT_SUCCESS;
    }

    if (strcmp(cmd, "get") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Error: 'get' takes no arguments\n");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_get();
    }

    if (strcmp(cmd, "list") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Error: 'list' takes no arguments\n");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_list();
    }

    if (strcmp(cmd, "set") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Error: 'set' requires exactly one input source ID\n");
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
        return cmd_set(argv[2]);
    }

    fprintf(stderr, "Error: unknown command '%s'\n", cmd);
    print_usage(stderr, argv[0]);
    return EXIT_FAILURE;
}
