#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <Adafruit_dvhstx.h>
#include <Adafruit_TinyUSB.h>
#include <pio_usb.h>
#include "commonvars.h"
#include "sound.h"
#include "rubido.h"
#include "glcdfont.h"
#include "framebuffer.h"
#include "usbh_processor.h"
#include "i2stones.h"

static uint32_t core1_stack[CORE1_STACK_SIZE / sizeof(uint32_t)];
Adafruit_USBH_Host USBHost;

const uint16_t timePerFrame =  1000000 / FPS; 
static float frameRate = 0;
static uint32_t currentTime = 0, lastTime = 0, frameTime = 0;
static bool endFrame = true;

uint32_t getFreeRam() { 
  return rp2040.getFreeHeap();
}

uint8_t readButtons()
{
  uint8_t ret = 0;
  if (!digitalRead(BUTTON_1_PIN))
    ret |= BUTTON_1_MASK;
  if (!digitalRead(BUTTON_2_PIN))
    ret |= BUTTON_2_MASK;
  if (!digitalRead(BUTTON_3_PIN))
    ret |= BUTTON_3_MASK;
  return ret;
}

void printDebugCpuRamLoad()
{
    if(debugMode)
    {
        int currentFPS = (int)frameRate;
        char debuginfo[80];
        
        int fps_int = (int)frameRate;
        int fps_frac = (int)((frameRate - fps_int) * 100);
        float cpuTemp = analogReadTemp();
        int cpuTemp_int = (int)cpuTemp;
        int cpuTemp_frac = (int)((cpuTemp - cpuTemp_int) * 100);
        sprintf(debuginfo, "F:%3d.%2d R:%3d A:%2d B:%d%% O:%d U:%d C:%2d.%2d", 
            fps_int, fps_frac, getFreeRam(), 
            getActiveChannelCount(), 
            (getBufferAvailable()*100)/getActualBufferSize(),
            getBufferSkipCount(),
            getBufferUnderrunCount(),
            cpuTemp_int,
            cpuTemp_frac
        );
        //Serial.println(debuginfo); 
        bufferPrint(&fb, 0, 0, debuginfo, tft.color565(255,255,255), tft.color565(0,0,0), 1, font);
    }
}


void setupButtons()
{
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
}

void core1_setup()
{
#ifdef PIN_5V_EN
    pinMode(PIN_5V_EN, OUTPUT);
    digitalWrite(PIN_5V_EN, PIN_5V_EN_STATE);
#endif

#ifdef PIN_USB_HOST_VBUS
    //not sure if required, doom port did this as well
    Serial.printf("Enabling USB host VBUS power on GP%d\r\n", PIN_USB_HOST_VBUS);
    gpio_init(PIN_USB_HOST_VBUS);
    gpio_set_dir(PIN_USB_HOST_VBUS, GPIO_OUT);
    gpio_put(PIN_USB_HOST_VBUS, 1);
#endif

    //same doom port did this as well i guess this fixes usb host stability
    irq_set_priority(USBCTRL_IRQ, 0xc0);
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = PIN_USB_HOST_DP;
    pio_cfg.tx_ch = 8; //use a free dma channel 0-4 seemed to be in use and default was 0
    USBHost.configure_pio_usb(1, &pio_cfg); 
    if(!USBHost.begin(1))
    {
        pinMode(LED_BUILTIN, OUTPUT);
        for (;;)
        digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
    }
    delay(4000); //needs to be high enough or also does not work
}

void core1_loop()
{
    USBHost.task();
    delayMicroseconds(100);
}

void core1_entry() {
    core1_setup();
    
    while(true) {
        core1_loop();
    }
}

void setup()
{   
    Serial.begin(9600);
    //while(!Serial) delay(10);
    Serial.println("Rubido");

    multicore_reset_core1();
    multicore_launch_core1_with_stack(core1_entry, core1_stack, CORE1_STACK_SIZE);

    if (!tft.begin()) { // Blink LED if insufficient RAM
        Serial.printf("failed to setup display\n");
        pinMode(LED_BUILTIN, OUTPUT);
        for (;;)
        digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
    }
    fb.buffer = tft.getBuffer();
    fb.width = tft.width();
    fb.height = tft.height();
    fb.littleEndian = 1;
    fb.bgr = 0;

    setupButtons();
    setupGame();
    currentTime = micros();
    lastTime = 0;
}

void loop()
{
    updateI2SAudio();
    
    currentTime = micros();
    frameTime  = currentTime - lastTime;  
    if((frameTime < timePerFrame) || !endFrame)
       return;     
    endFrame = false;
    frameRate = 1000000.0 / frameTime;
    lastTime = currentTime;
    prevButtons = currButtons;
    currButtons = readButtons();
    updateUSBHButtons();
    
    if(gamepadButtonJustPressed(GAMEPAD_LEFT_SHOULDER) || keyJustPressed(F1KEY) ||
		((currButtons & BUTTON_2_MASK) && (currButtons & BUTTON_1_MASK) && ! (prevButtons & BUTTON_1_MASK)))
		decVolumeSound();

	if(gamepadButtonJustPressed(GAMEPAD_RIGHT_SHOULDER) || keyJustPressed(F2KEY) ||
		((currButtons & BUTTON_2_MASK) && (currButtons & BUTTON_3_MASK) && ! (prevButtons & BUTTON_3_MASK)))
		incVolumeSound();

    if(gamepadButtonJustPressed(GAMEPAD_SELECT) || keyJustPressed(DKEY))
        debugMode = !debugMode;
    
    mainLoop();

    printDebugCpuRamLoad();
    tft.swap();
    fb.buffer = tft.getBuffer();
    endFrame = true;
}
