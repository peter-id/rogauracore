/**
 * rogaura
 * Copyright (c) 2019 Will Roberts
 *
 * Author:        Will Roberts <wildwilhelm@gmail.com> (WKR)
 * Creation Date: 13 March 2019
 *
 * Description:
 *    RGB keyboard control for Asus ROG laptops
 *
 * Revision Information:
 *
 *    (WKR) 13 March 2019
 *          - Boilerplate header added.
 *
 * \file rogaura.c
 */

// sudo apt install libusb-1.0-0 libusb-1.0-0-dev

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define MESSAGE_LENGTH 17
#define MAX_NUM_MESSAGES 6
#define MAX_NUM_COLORS 4
#define MAX_FUNCNAME_LEN 32

// https://stackoverflow.com/a/14251257/1062499
#define DEBUG
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


// ------------------------------------------------------------
//  Data structures
// ------------------------------------------------------------

typedef struct {
    uint8_t nRed;
    uint8_t nGreen;
    uint8_t nBlue;
} Color;

typedef enum {Slow = 1, Medium, Fast} Speed;

typedef struct {
    Color colors[MAX_NUM_COLORS];
    Speed speed;
} Arguments;

typedef struct {
    int nMessages;
    uint8_t messages[MAX_NUM_MESSAGES][MESSAGE_LENGTH];
} Messages;

typedef struct {
    const char *szName;
    void (*function)(Arguments *args, Messages *outputs);
    int nColors;
    int nSpeed;
} FunctionRecord;


// ------------------------------------------------------------
//  USB protocol for RGB keyboard
// ------------------------------------------------------------

const uint8_t SPEED_BYTE_VALUES[] = {0xe1, 0xeb, 0xf5};

uint8_t speedByteValue(Speed speed) {
    return SPEED_BYTE_VALUES[speed - 1];
}

uint8_t MESSAGE_SET[] = {0x5d, 0xb5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t MESSAGE_APPLY[] = {0x5d, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void initMessage(uint8_t *msg) {
    memset(msg, 0, MESSAGE_LENGTH);
    msg[0] = 0x5d;
    msg[1] = 0xb3;
}

void single_static(Arguments *args, Messages *outputs) {
    D(printf("single_static\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[4] = args->colors[0].nRed;
    m[5] = args->colors[0].nGreen;
    m[6] = args->colors[0].nBlue;
}

void single_breathing(Arguments *args, Messages *outputs) {
    D(printf("single_breathing\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 1;
    m[4] = args->colors[0].nRed;
    m[5] = args->colors[0].nGreen;
    m[6] = args->colors[0].nBlue;
    m[7] = speedByteValue(args->speed);
    m[9] = 1;
    m[10] = args->colors[1].nRed;
    m[11] = args->colors[1].nGreen;
    m[12] = args->colors[1].nBlue;
}

void single_colorcycle(Arguments *args, Messages *outputs) {
    D(printf("single_colorcycle\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 2;
    m[4] = 0xff;
    m[7] = speedByteValue(args->speed);
}

void multi_static(Arguments *args, Messages *outputs) {
    D(printf("multi_static\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[4] = args->colors[i].nRed;
        m[5] = args->colors[i].nGreen;
        m[6] = args->colors[i].nBlue;
        m[7] = 0xeb;
    }
}

void multi_breathing(Arguments *args, Messages *outputs) {
    D(printf("multi_breathing\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[3] = 1;
        m[4] = args->colors[i].nRed;
        m[5] = args->colors[i].nGreen;
        m[6] = args->colors[i].nBlue;
        m[7] = speedByteValue(args->speed);
    }
}

const uint8_t RED[] = { 0xff, 0x00, 0x00 };
const uint8_t GREEN[] = { 0x00, 0xff, 0x00 };
const uint8_t BLUE[] = { 0x00, 0x00, 0xff };
const uint8_t YELLOW[] = { 0xff, 0xff, 0x00 };
const uint8_t CYAN[] = { 0x00, 0xff, 0xff };
const uint8_t MAGENTA[] = { 0xff, 0x00, 0xff };
const uint8_t WHITE[] = { 0xff, 0xff, 0xff };
const uint8_t BLACK[] = { 0x00, 0x00, 0x00 };

void red(Arguments *args, Messages *messages) {
    memcpy(args->colors, RED, 3);
    single_static(args, messages);
}

void green(Arguments *args, Messages *messages) {
    memcpy(args->colors, GREEN, 3);
    single_static(args, messages);
}

void blue(Arguments *args, Messages *messages) {
    memcpy(args->colors, BLUE, 3);
    single_static(args, messages);
}

void yellow(Arguments *args, Messages *messages) {
    memcpy(args->colors, YELLOW, 3);
    single_static(args, messages);
}

void cyan(Arguments *args, Messages *messages) {
    memcpy(args->colors, CYAN, 3);
    single_static(args, messages);
}

void magenta(Arguments *args, Messages *messages) {
    memcpy(args->colors, MAGENTA, 3);
    single_static(args, messages);
}

void white(Arguments *args, Messages *messages) {
    memcpy(args->colors, WHITE, 3);
    single_static(args, messages);
}

void black(Arguments *args, Messages *messages) {
    memcpy(args->colors, BLACK, 3);
    single_static(args, messages);
}

void rainbow(Arguments *args, Messages *messages) {
    memcpy(&(args->colors[0]), RED, 3);
    memcpy(&(args->colors[1]), YELLOW, 3);
    memcpy(&(args->colors[2]), CYAN, 3);
    memcpy(&(args->colors[3]), MAGENTA, 3);
    multi_static(args, messages);
}


// ------------------------------------------------------------
//  Command line argument parsing
// ------------------------------------------------------------

const FunctionRecord FUNCTION_RECORDS[] = {
    {"single_static", &single_static, 1, 0},
    {"single_breathing", &single_breathing, 2, 1},
    {"single_colorcycle", &single_colorcycle, 0, 1},
    {"multi_static", &multi_static, 4, 0},
    {"multi_breathing", &multi_breathing, 4, 1},
    {"red", &red, 0, 0},
    {"green", &green, 0, 0},
    {"blue", &blue, 0, 0},
    {"yellow", &yellow, 0, 0},
    {"cyan", &cyan, 0, 0},
    {"magenta", &magenta, 0, 0},
    {"white", &white, 0, 0},
    {"black", &black, 0, 0},
    {"rainbow", &rainbow, 0, 0},
};

const int NUM_FUNCTION_RECORDS = (int)(sizeof(FUNCTION_RECORDS) / sizeof(FUNCTION_RECORDS[0]));

void usage() {
    printf("rogaura - RGB keyboard control for Asus ROG laptops\n");
    printf("(c) 2019 Will Roberts\n\n");
    printf("Usage:\n");
    printf("   rogaura COMMAND ARGUMENTS\n\n");
    printf("COMMAND should be one of:\n");
    for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
        printf("   %s\n", FUNCTION_RECORDS[i].szName);
    }
}

int parseColor(char *arg, Color *pResult) {
    D(printf("parse color %s\n", arg));
    uint32_t v = 0;
    if (strlen(arg) != 6) goto fail;
    for (int i = 0; i < 6; ++i) {
        if (!isxdigit(arg[i])) goto fail;
    }
    v = (uint32_t)strtol(arg, 0, 16);
    if (errno == ERANGE) goto fail;
    pResult->nRed = (v >> 16) & 0xFF;
    pResult->nGreen = (v >> 8) & 0xFF;
    pResult->nBlue = v & 0xFF;
    D(printf("interpreted color %d %d %d\n", pResult->nRed, pResult->nGreen, pResult->nBlue));
    return 0;
fail:
    printf("Could not interpret color parameter value %s\n", arg);
    printf("Please give this value as a six-character hex string like ff0000.\n");
    return -1;
}

int parseSpeed(char *arg, Speed *pResult) {
    D(printf("parse speed %s\n", arg));
    long nSpeed = strtol(arg, 0, 0);
    if (errno == ERANGE || nSpeed < 1 || nSpeed > 3) {
        printf("Could not interpret speed parameter value %s\n", arg);
        printf("Please give this value as an integer: 1 (slow), 2 (medium), or 3 (fast).\n");
        return -1;
    }
    *pResult = (Speed)nSpeed;
    return 0;
}

int parseArguments(int argc, char **argv, Messages *messages) {
    int                   nRetval;
    const FunctionRecord *pDesiredFunc  = 0;
    Arguments             args;
    int                   nColors       = 0;
    // identify the function the user has asked for
    if (argc > 1) {
        for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
            if (!strncmp(argv[1], FUNCTION_RECORDS[i].szName, MAX_FUNCNAME_LEN)) {
                pDesiredFunc = &(FUNCTION_RECORDS[i]);
                break;
            }
        }
    }
    if (!pDesiredFunc) {
        usage();
        return -1;
    }
    // check that the function signature is satisfied
    if (argc != (1 + 1 + pDesiredFunc->nColors + pDesiredFunc->nSpeed)) {
        usage();
        printf("\nFunction %s takes ", pDesiredFunc->szName);
        if (pDesiredFunc->nColors > 0) {
            if (pDesiredFunc->nSpeed) {
                printf("%d color(s) and a speed", pDesiredFunc->nColors);
            } else {
                printf("%d color(s)", pDesiredFunc->nColors);
            }
        } else {
            if (pDesiredFunc->nSpeed) {
                printf("a speed");
            } else {
                printf("no arguments");
            }
        }
        printf(":\n   rogaura %s ", pDesiredFunc->szName);
        for (int i = 0; i < pDesiredFunc->nColors; i++) {
            printf("color%d ", i+1);
        }
        if (pDesiredFunc->nSpeed) {
            printf("speed");
        }
        printf("\n\ncolor arguments should be given as hex values like ff0000\n");
        printf("speed argument should be given as an integer: 1, 2, or 3\n");
        return -1;
    }
    // parse the argument values
    for (int i = 2; i < argc; ++i) {
        if (nColors < pDesiredFunc->nColors) {
            nRetval = parseColor(argv[i], &(args.colors[nColors]));
            if (nRetval < 0) return -1;
            nColors++;
        } else {
            nRetval = parseSpeed(argv[i], &args.speed);
            if (nRetval < 0) return -1;
        }
    }
    D(printf("args:\n"));
    for (int i = 0; i < MAX_NUM_COLORS; ++i) {
        D(printf("color%d %d %d %d\n", i + 1, args.colors[i].nRed, args.colors[i].nGreen, args.colors[i].nBlue));
    }
    D(printf("speed %d\n", args.speed));
    // call the function the user wants
    pDesiredFunc->function(&args, messages);
    D(printf("constructed %d messages:\n", messages->nMessages));
    for (int i = 0; i < messages->nMessages; ++i) {
        D(printf("message %d: ", i));
        for (int j = 0; j < MESSAGE_LENGTH; j++)
        {
            D(printf("%02x ", messages->messages[i][j] & 0xff));
        }
        D(printf("\n"));
    }
    return 0;
}


// ------------------------------------------------------------
//  Libusb interface
// ------------------------------------------------------------

const uint16_t ASUS_VENDOR_ID = 0x0b05;
const uint16_t ASUS_PRODUCT_IDS[] = { 0x1854, 0x1869 };
const int NUM_ASUS_PRODUCTS = (int)(sizeof(ASUS_PRODUCT_IDS) / sizeof(ASUS_PRODUCT_IDS[0]));

int checkDevice(libusb_device *pDevice) {
    struct libusb_device_descriptor devDesc;
    libusb_get_device_descriptor(pDevice, &devDesc);
    printf("   Checking device %04x:%04x, address %d\n",
           devDesc.idVendor, devDesc.idProduct,
           libusb_get_device_address(pDevice));
    if (devDesc.idVendor == ASUS_VENDOR_ID) {
        for (int i = 0; i < NUM_ASUS_PRODUCTS; ++i)
        {
            if (devDesc.idProduct == ASUS_PRODUCT_IDS[i]) return 1;
        }
    }
    return 0;
}

int controlTransfer(libusb_device_handle *pHandle, unsigned char *sData, uint16_t wLength) {
    int nRetval = libusb_control_transfer(
        pHandle,
        0x21 /* bmRequestType */,
        9 /* bRequest */,
        0x035d /* wValue */,
        0 /* wIndex */,
        sData,
        wLength,
        0 /* standard device timeout */
        );
    if (nRetval < 0) {
        printf("Control transfer error: %s\n", libusb_error_name(nRetval));
    }
    return nRetval;
}

int handleUsb(Messages *pMessages) {
    int                              nRetval;
    libusb_device                  **deviceList       = 0;
    int                              nDevices         = 0;
    libusb_device                   *device           = 0;
    libusb_device                   *auraCoreDevice   = 0;
    libusb_device_handle            *pHandle          = 0;
    uint8_t                          bInterfaceNumber = 0;
    struct libusb_config_descriptor *pConfig          = 0;
    // Try to initialise the libusb library
    D(printf("Initialising libusb\n"));
    if (libusb_init(0) < 0) {
        printf("Could not initialise libusb.\n");
        nRetval = -1; goto exit;
    }
    D(printf("Initialised libusb.\n"));

    // Lets try to find our HID device that controls backlight LEDs.
    nDevices = libusb_get_device_list(0, &deviceList);
    if (nDevices < 0) {
        printf("Could not fetch usb device list.\n");
        nRetval = -1; goto deinit;
    }
    D(printf("Found %d USB devices.\n", nDevices));
    for (int i = 0; i < nDevices; i++) {
        device = deviceList[i];
        if (checkDevice(device)) {
            D(printf("Found ROG Aura Core keyboard.\n"));
            auraCoreDevice = device;
            break;
        }
    }
    if (!auraCoreDevice) {
        printf("Could not find ROG Aura Core keyboard.\n");
        nRetval = -1; goto freelist;
    }
    nRetval = libusb_open(auraCoreDevice, &pHandle);
    if (nRetval < 0) {
        printf("Could not open ROG Aura Core keyboard: %s\n", libusb_error_name(nRetval));
        goto freelist;
    }
    D(printf("Opened USB device.\n"));

    // Detach kernel drivers before USB communication
    nRetval = libusb_set_auto_detach_kernel_driver(pHandle, 1);
    if (nRetval < 0) {
        printf("Could not set auto detach kernel mode: %s\n",
               libusb_error_name(nRetval));
    } else {
        D(printf("Auto detach kernel mode set.\n"));
    }

    // Get configuration descriptor
    nRetval = libusb_get_active_config_descriptor(auraCoreDevice, &pConfig);
    if (nRetval < 0) {
        printf("Could not get configuration descriptor: %s.\n", libusb_error_name(nRetval));
        goto close;
    }

    // We want to claim the first interface on the device
    if (pConfig->bNumInterfaces == 0) {
        printf("No interfaces defined on the USB device.");
        nRetval = -1; goto freedesc;
    }
    if (pConfig->interface[0].num_altsetting == 0) {
        printf("No interface descriptors for the first interface of the USB device.");
        nRetval = -1; goto freedesc;
    }
    bInterfaceNumber = pConfig->interface[0].altsetting[0].bInterfaceNumber;

    // Claim the interface
    nRetval = libusb_claim_interface(pHandle, bInterfaceNumber);
    if(nRetval < 0) {
        printf("Could not claim interface: %s.\n", libusb_error_name(nRetval));
        goto freedesc;
    }
    D(printf("Claimed interface %d.\n", bInterfaceNumber));

    // Send the control messages
    for (int i = 0; i < pMessages->nMessages; ++i) {
        nRetval = controlTransfer(pHandle, pMessages->messages[i], MESSAGE_LENGTH);
        if (nRetval < 0) goto release;
    }
    nRetval = controlTransfer(pHandle, MESSAGE_SET, MESSAGE_LENGTH);
    if (nRetval < 0) goto release;
    nRetval = controlTransfer(pHandle, MESSAGE_APPLY, MESSAGE_LENGTH);
    if (nRetval < 0) goto release;

release:
    libusb_release_interface(pHandle, bInterfaceNumber);
freedesc:
    libusb_free_config_descriptor(pConfig);
close:
    libusb_close(pHandle);
freelist:
    libusb_free_device_list(deviceList, 1);
deinit:
    libusb_exit(0);
exit:
    return nRetval;
}

// ------------------------------------------------------------
//  Main function
// ------------------------------------------------------------

int main(int argc, char **argv) {
    Messages messages;
    if (parseArguments(argc, argv, &messages) == 0) {
        handleUsb(&messages);
    }
}
