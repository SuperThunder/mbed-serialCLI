#include "mbed.h"
#include "platform/mbed_thread.h"


#include "serialCLI.h"

static BufferedSerial pc(USBTX,USBRX,115200);


InterruptIn chargeButton(USER_BUTTON);
volatile bool buttonPressed = false; //set to true by button ISR, set to false when acknowledged


void blinkled()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    
    while (true) {
        led = !led;
        ThisThread::sleep_for(1s);
    }
}


// Button to initiate / stop charging
// Called on button rise
// Set to true to signal to EN control thread that button was pressed
void buttonISR()
{
    static uint64_t lastButtonPushTime = 0;
    
    uint64_t now = Kernel::get_ms_count();
    
    const uint64_t buttonMinimumWaitTime = 500; //minimum 500 ms wait between button pushes
    
    if(now - lastButtonPushTime > buttonMinimumWaitTime)
    {
        buttonPressed = true;
        lastButtonPushTime = now;
    }
       
}

int main()
{
    printf("Started serialCLI test program\r\n");
    
    //BUILT IN LED THREAD
    Thread led1;
    //osStatus err =  led1.start(&blinkled);
    led1.start(blinkled);

    // START/STOP CHARGE BUTTON
    chargeButton.rise(&buttonISR);

    Thread cli_thread;

   serialCLI cli(&pc, &cli_thread);

    
    while(true)
    {
        ThisThread::sleep_for(1s);
    }
    
}