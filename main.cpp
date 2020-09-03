#include "mbed.h"
#include "platform/mbed_thread.h"


#include "serialCLI.h"

//mbed 6
//static BufferedSerial pc(USBTX,USBRX,115200);

//mbed 5
UARTSerial pc(USBTX,USBRX,115200);

uint32_t testint1 = 0;


void blinkled()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    
    while (true) {
        led = !led;
        ThisThread::sleep_for(1000);
    }
}

//from https://stackoverflow.com/questions/194465/how-to-parse-a-string-to-an-int-in-c
bool str2int (int32_t &i, const char* s)
{
    char              c;
    std::stringstream ss(s);
    ss >> i;
    if (ss.fail() || ss.get(c)) {
        // not an integer
        return false;
    }
    return true;
}

void helpHandler(std::string* args, serialCLI::lineCommandType command_type, serialCLI* cli)
{
    cli->vprintfCLI("I: Help handler called with args: \'%s\'\r\n", args->c_str() );
}

//test out setting of integer and error conditions
//TODO: takes callback to printf wrapper around serialInterface->write instead
//      eventqueue, or mail handled by output thread
void setIntTest(std::string* args, serialCLI::lineCommandType command_type, serialCLI* cli)
{
    cli->vprintfCLI("I: Testint1 handler called with command type %d args: \'%s\'\r\n", command_type, args->c_str() );

    int32_t tmpint;
    //if get, return value
    if(command_type == serialCLI::lineCommandType::GET)
    {
        cli->vprintfCLI("D: test1, %d \r\n", testint1);
    }
    //if set, set value
    else if(command_type == serialCLI::lineCommandType::SET)
    {
        //parse argument content to integer
        if( str2int(tmpint, args->c_str() ) )
        {
            testint1 = tmpint;
            cli->vprintfCLI("I: testint1 set to %d \r\n", testint1);
        }
        else
        {
            cli->vprintfCLI("E: Invalid set value for testint1 %s\r\n", args->c_str());
        }
    }

}


int main()
{
    printf("Started serialCLI test program\r\n");
    
    //BUILT IN LED THREAD
    Thread led1;
    //osStatus err =  led1.start(&blinkled);
    led1.start(blinkled);

    serialCLI cli(&pc);

    cli.attachVariableHandler("help", helpHandler );
    cli.attachVariableHandler("test1", setIntTest );

    
    while(true)
    {
        ThisThread::sleep_for(1000);
    }
    
}