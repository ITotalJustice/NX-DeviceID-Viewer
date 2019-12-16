#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <switch.h>


#define DEVICE_CERTIFICATE_SIZE 0x180   // size of entire cert.
#define DEVICE_ID_OFFSET        0xC6    // offset in cert.
#define DEVICE_ID_SIZE          0x10    // size of deviceID.
#define FILE_OUTPUT             "deviceID.txt"


typedef struct
{
    uint16_t nx;                    // NX.
    uint8_t id[DEVICE_ID_SIZE];     // device ID.
    uint8_t end[0x2];               // -0.
} device_t;

typedef struct
{
    uint8_t _0x0[0x40];             // unkown.
    uint8_t _0x30[0x40];            // empty.
    uint8_t n_nxca1_prod1[0x12];    // NintendoNXCA1Prod1.
    uint8_t _0x92[0x30];            // empty.
    uint16_t _0xC2;                 // always 2.
    device_t device;                // 
    uint8_t _0xD8[0x2C];            // empty.
    uint8_t _0x104[0x40];           // unkown.
    uint8_t _0x144[0x3C];
} cert_t;

cert_t cert;


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

bool get_device_certificate(cert_t *cert)
{
    print_message_display("getting device certificate...\n\n");

    if (cert == NULL)
    {
        print_message_loop_lock("cert is NULL\n\n");
        return false;
    }

    if (R_FAILED(setcalGetEciDeviceCertificate(cert, DEVICE_CERTIFICATE_SIZE)))
    {
        print_message_loop_lock("failed to get device cert\n\n");
        return false;
    }

    print_message_display("got certificate!\n\n");

    printf("\n\n\n\nyour device ID is:\t");
    for (uint8_t i = 0; i < DEVICE_ID_SIZE; i++)
    {
        printf("%c", cert->device.id[i]);
    }
    print_message_display("\n\n\n\n\n\n");

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
    print_message_display("NX Device ID: 1.0.0\n\n\n\n\n\n");

    bool rc = get_device_certificate(&cert);

    print_message_display("press a to dump to file\n\n");
    print_message_display("press + to exit...");

    while (appletMainLoop())
    {
        uint64_t input = poll_input();

        if (input & KEY_A)
            if (rc == true)
                write_to_file(FILE_OUTPUT, &cert.device.id, DEVICE_ID_SIZE, "w");
        
        if (input & KEY_PLUS)
            break;
    }

    app_exit();
    return 0;
}