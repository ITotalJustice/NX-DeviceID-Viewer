#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <switch.h>


#define DEVICE_CERTIFICATE_SIZE 0x180
#define DEVICE_ID_OFFSET        0xC6
#define DEVICE_ID_SIZE          0x10
#define FILE_OUTPUT             "deviceID.txt"

uint8_t cert[DEVICE_CERTIFICATE_SIZE];
char device_id[DEVICE_ID_SIZE+1];   // add 1 for NULL  


uint64_t poll_input(void)
{
    hidScanInput();
    return hidKeysDown(CONTROLLER_P1_AUTO);
}

void print_message_display(const char *message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);
}

void print_message_loop_lock(const char *message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s\n\npress b to exit...", new_message);
    consoleUpdate(NULL);

    while (appletMainLoop())
    {
        if (poll_input() & KEY_B)
            break;
    }
}

void app_init(void)
{
    consoleInit(NULL);
    setcalInitialize();
}

void app_exit(void)
{
    consoleExit(NULL);
    setcalExit();
}

bool get_device_certificate(uint8_t *cert)
{
    if (cert == NULL)
    {
        print_message_loop_lock("cert is NULL\n\n");
        return false;
    }

    print_message_display("getting device certificate...\n\n");

    if (R_FAILED(setcalGetEciDeviceCertificate(cert, DEVICE_CERTIFICATE_SIZE)))
    {
        print_message_loop_lock("failed to get device cert\n\n");
        return false;
    }

    print_message_display("got certificate!\n\n");
    return true;
}

bool get_device_id(const uint8_t *cert, char *device_id)
{
    if (cert == NULL || device_id == NULL)
    {
        print_message_loop_lock("cert or device_id is NULL\n\n");
        return false;
    }

    print_message_display("getting device ID...\n\n");

    memcpy(device_id, &cert[DEVICE_ID_OFFSET], DEVICE_ID_SIZE);
    print_message_display("\n\n\n\nyour device ID is:\t%s\n\n\n\n\n\n", device_id);

    return true;
}

bool write_to_file(const char *f, const void *in, size_t size, const char *mode)
{
    if (f == NULL || in == NULL || mode == NULL)
    {
        print_message_loop_lock("f, in or mode are NULL\n\n");
        return false;
    }

    FILE *file = fopen(f, mode);
    if (!file)
    {
        print_message_loop_lock("failed to open file %s\n\n", f);
        return false;
    }

    fwrite(in, size, 1, file);
    fclose(file);
    print_message_display("\nwrote to file!\n");
    return true;
}

int main(int argc, char *argv[])
{
    app_init();
    print_message_display("NX Device ID: 0.0.1\n\n\n\n\n\n");

    bool rc = false;

    if ((rc = get_device_certificate(cert)))
        rc = get_device_id(cert, device_id);

    print_message_display("press a to dump to file\n\n");
    print_message_display("press + to exit...");

    while (appletMainLoop())
    {
        uint64_t input = poll_input();

        if (input & KEY_A)
            if (rc == true)
                write_to_file(FILE_OUTPUT, device_id, DEVICE_ID_SIZE, "w");
        
        if (input & KEY_PLUS)
            break;
    }

    app_exit();
    return 0;
}