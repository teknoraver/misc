#ifndef USBLCD_H
#define USBLCD_H

#include <usb.h>
#include <stdio.h>
#include <string.h>

#include "driver.h"
#include "debug.h"

typedef int hid_return;

struct hid_params {
    unsigned int endpoint;
    unsigned int packetlen;
    unsigned int timeout;
    char packet[_USBLCD_MAX_DATA_LEN];
};

struct usblcd_event {
    /* 0 keypad data, 1 infrared data */
    unsigned int type;
    int length;
    unsigned char data[_USBLCD_MAX_DATA_LEN];
};

struct usblcd_state {
    unsigned int usblcd_switch;
    unsigned int usblcd_cursor;
    unsigned int usblcd_cursor_blink;
};

struct hid_device {
    unsigned int id;
    struct usb_device *device;
    struct usb_dev_handle *handle;
};

struct usblcd {
/* lcd and cursor status */
    struct usblcd_state state;
/* leds state */
    unsigned int leds;
/* pointer to deeper level hid functions */
    struct hid_device hiddev;
};

static struct usb_device *hid_find_device(int vendor, int product) 
{
    struct usb_bus *bus;
    
    for (bus = usb_get_busses(); bus; bus = bus->next) {
	struct usb_device *dev;
	
	for (dev = bus->devices; dev; dev = dev->next) {
	    if (dev->descriptor.idVendor == vendor
		&& dev->descriptor.idProduct == product)
		return dev;
	}
    }
    return NULL;
}

#ifdef DEBUG
static char * hid_get_product(struct usb_device *dev, struct usb_dev_handle *devh)
{
    int len;
    int BUFLEN = 256;
    char *buffer;
    
    buffer = (char *) malloc (BUFLEN);
    
    if (buffer == NULL) 
	return NULL;
        
    if (dev->descriptor.iProduct) {
	len = usb_get_string_simple(devh, dev->descriptor.iProduct, buffer, BUFLEN);

	if (len > 0)
	    return buffer;
	else {
	    free(buffer);
	    return NULL;
	}      
    }
    return NULL;
}

static char * hid_get_manufacturer(struct usb_device *dev, struct usb_dev_handle *devh)
{
    int len;
    int BUFLEN = 256;
    char *buffer;
    
    buffer = (char *) malloc (BUFLEN);
    
    if (buffer == NULL) 
	return NULL;
        
    if (dev->descriptor.iManufacturer) {
	len = usb_get_string_simple(devh, dev->descriptor.iManufacturer, buffer, BUFLEN);

	if (len > 0)
	    return buffer;
	else {
	    free(buffer);
	    return NULL;
	}      
    }
    return NULL;
}

static char * hid_get_serial(struct usb_device *dev, struct usb_dev_handle *devh)
{
    int len;
    int BUFLEN = 256;
    char *buffer;
    
    buffer = (char *) malloc (BUFLEN);
    
    if (buffer == NULL) 
	return NULL;
        
    if (dev->descriptor.iSerialNumber) {
	len = usb_get_string_simple(devh, dev->descriptor.iSerialNumber, buffer, BUFLEN);

	if (len > 0)
	    return buffer;
	else {
	    free(buffer);
	    return NULL;
	}      
    }
    return NULL;
}
#endif

hid_return hid_init(struct hid_device *hiddev)
{
    hid_return ret;
    char buf[65535];
    
    usb_init();
    
    usb_find_busses();
    
    usb_find_devices();
    
    hiddev->device = hid_find_device(_USBLCD_IDVENDOR, _USBLCD_IDPRODUCT);
    
    if (hiddev->device == NULL) {
	printf("Device not detected !\n");
	exit (1);
    }
    
    hiddev->handle = usb_open(hiddev->device);

//	FIXME
//    signal(SIGTERM, release_usb_device);

    ret = usb_get_driver_np(hiddev->handle, 0, buf, sizeof(buf));

    if (ret == 0) {
	printf("interface 0 already claimed by driver \"%s\", attempting to detach it\n", buf);
	ret = usb_detach_kernel_driver_np(hiddev->handle, 0);
	printf("usb_detach_kernel_driver_np returned %d\n", ret);
    }
    ret = usb_claim_interface(hiddev->handle, 0);
    if (ret != 0) {
	printf("claim failed with error %d\n", ret);
		exit(1);
    }
    
    ret = usb_set_altinterface(hiddev->handle, 0);
    
    MESSAGE("Product: %s, Manufacturer: %s, Firmware Version: %s\n", hid_get_product(hiddev->device, hiddev->handle),
	    hid_get_manufacturer(hiddev->device, hiddev->handle), hid_get_serial(hiddev->device, hiddev->handle));
        
    #ifdef USB_DEV_TEST
    ret = usb_set_configuration(handle, 0x0000001);
    ret = usb_control_msg(handle, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 0x000000a, 0x0000000, 0x0000000, usb_reply, 0x0000000, 1000);
    #endif

    MESSAGE("Found and initialized hid device: %x", hiddev->handle);
    return 0;
}

void hid_interrupt_write(void *handle, struct hid_params *params)
{
    hid_return ret;
    
    MESSAGE("Device: %x", handle);
    ret = usb_interrupt_write((usb_dev_handle *) handle, (unsigned int const) params->endpoint , \
				 (char*) params->packet, (unsigned int const) params->packetlen, \
				 (unsigned int const) params->timeout);
    
    if (ret < 0) 
	fprintf(stderr, "hid_interrupt_write failed with return code %d\n", ret);
}

void usblcd_getversion(struct usblcd *usblcd)
{

    struct hid_params params;
    
    params.endpoint = USB_ENDPOINT_OUT + 1;
    params.packetlen = 1;
    params.timeout = HID_TIMEOUT;
    
    snprintf (params.packet, params.packetlen + 1 , "%c", HID_REPORT_GET_VERSION);
    MESSAGE("Wrinting %s to lcd device %x", params.packet, usblcd->hiddev.handle);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}


void usblcd_init(struct usblcd *usblcd) 
{
	usblcd->state.usblcd_switch = _USBLCD_SWITCH_ON;
	usblcd->state.usblcd_cursor = 0;
	usblcd->state.usblcd_cursor_blink = 0;
	
	usblcd->leds = 0;

	hid_init(&usblcd->hiddev);
	usblcd_getversion(usblcd);
}

void usblcd_control(struct usblcd *usblcd)
{
    struct hid_params params;
    params.endpoint = USB_ENDPOINT_OUT + 1;
    params.packetlen = 2;
    params.timeout = HID_TIMEOUT;
    snprintf (params.packet, params.packetlen + 1 , "%c%c", OUT_REPORT_LCD_CONTROL, usblcd->state.usblcd_switch | usblcd->state.usblcd_cursor | usblcd->state.usblcd_cursor_blink);
    MESSAGE("Wrinting %s to lcd device %x", params.packet, usblcd->hiddev.handle);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}

void usblcd_set_cursor(struct usblcd *usblcd, unsigned int status)
{
    if (status) usblcd->state.usblcd_cursor = _USBLCD_CURSOR_ON;
    else usblcd->state.usblcd_cursor = 0;
    usblcd_control(usblcd);    
}

void usblcd_set_cursor_blink(struct usblcd *usblcd, unsigned int status)
{
    if (status) usblcd->state.usblcd_cursor_blink = _USBLCD_CURSOR_BLINK_ON;
    else usblcd->state.usblcd_cursor_blink = 0;
    
    usblcd_control(usblcd);    
}

void usblcd_setled(struct usblcd *usblcd, unsigned int led, unsigned int status) 
{
    struct hid_params params;
    
    if (led > _USBLCD_MAX_LEDS) led = _USBLCD_MAX_LEDS;
    if (status > 1 || status < 0) status = 0;
    
    /* set led bit to 1 or 0 */
    if (status)	usblcd->leds |= 1 << led;
    else usblcd->leds &= ~ (1 << led);
    
    params.endpoint = USB_ENDPOINT_OUT + 1;
    params.packetlen = 2;
    params.timeout = HID_TIMEOUT;
    
    snprintf (params.packet, params.packetlen + 1 , "%c%c", OUT_REPORT_LED_STATE, usblcd->leds);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}

void usblcd_backlight(struct usblcd *usblcd, unsigned int status) 
{
    struct hid_params params;
    params.endpoint = USB_ENDPOINT_OUT + 1;
    params.packetlen = 2;
    params.timeout = HID_TIMEOUT;
    snprintf (params.packet, params.packetlen + 1 , "%c%c", OUT_REPORT_LCD_BACKLIGHT, status);
    MESSAGE("Wrinting %s to lcd device %x", params.packet, usblcd->hiddev.handle);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}

void usblcd_clear(struct usblcd *usblcd)
{
    struct hid_params params;
    params.endpoint = USB_ENDPOINT_OUT + 1;
    params.packetlen = 1;
    params.timeout = HID_TIMEOUT;
    snprintf (params.packet, params.packetlen + 1 , "%c", OUT_REPORT_LCD_CLEAR);
    MESSAGE("Wrinting %s to lcd device %x", params.packet, usblcd->hiddev.handle);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}

void usblcd_settext(struct usblcd *usblcd, unsigned int row, unsigned int column, char *text)
{
    struct hid_params params;
    unsigned int len;
    
    len = strlen(text);
    
    if (len > 20) len = 20;
    if (row > _USBLCD_MAX_ROWS) row = _USBLCD_MAX_ROWS;
    if (column > _USBLCD_MAX_COLS) column = _USBLCD_MAX_COLS;
    
    params.endpoint = USB_ENDPOINT_OUT + 1;
    /* 3 control bytes row, column, text length*/
    params.packetlen = len + 1 + 3;
    params.timeout = HID_TIMEOUT;
    snprintf (params.packet, params.packetlen + 1 , "%c%c%c%c%s", OUT_REPORT_LCD_TEXT,  row, column, len, text);
    MESSAGE("Wrinting %s to lcd device %x", params.packet, usblcd->hiddev.handle);
    hid_interrupt_write(usblcd->hiddev.handle, &params);
}

hid_return hid_close(void *handle)
{
    hid_return ret;
    ret = usb_close(handle);
    
    if (ret < 0) {
	fprintf(stderr, "hid_close failed with return code %d\n", ret);
	return 1;
    }
    return ret;
}

void usblcd_close(struct usblcd *usblcd)
{
    if (usblcd->hiddev.handle) 
	hid_close(usblcd->hiddev.handle);
}

int usblcd_read_events(struct usblcd *usblcd, struct usblcd_event *event)
{
    int ret = -1;
    char read_packet[_USBLCD_MAX_DATA_LEN];

    //hid_set_idle(usblcd->hiddev.handle, 0, 0);
         
    ret = usb_interrupt_read(usblcd->hiddev.handle, USB_ENDPOINT_IN + 1, read_packet, _USBLCD_MAX_DATA_LEN, 10000);
    
    if (ret > 0) {
        switch ((unsigned char)read_packet[0]) {
	    case IN_REPORT_KEY_STATE:
	    {	
		event->type = 0;
		event->length = 2;
		
		memcpy(event->data, &read_packet[1], event->length);
#ifdef DEBUG
		print_buffer(event->data, event->length);
#endif	
		return 1;
		
	    } break;

    	    case IN_REPORT_IR_DATA:
	    {

		event->type = 1;
		event->length = read_packet[1];
		
		/* IR data packet also has a IR data length field */
		memcpy(event->data, &read_packet[2], event->length);
#ifdef DEBUG
		print_buffer(event->data, event->length);
#endif	
		return 1;

	    } break;
	
	    case IN_REPORT_INT_EE_DATA:
	    {	
#ifdef DEBUG
		fprintf(stderr,"IN_REPORT_INT_EE_DATA: ");
		print_buffer(read_packet, _USBLCD_MAX_DATA_LEN);
#endif
	    } break;

	    default:
		break;
	}
    }
    
    return 0;
}

#endif