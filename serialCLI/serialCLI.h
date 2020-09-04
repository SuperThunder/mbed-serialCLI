/**
 * @brief       serialCLI.h
 * @details     command-value interactive serial input
 *
 *
 * @return      N/A
 *
 * @author      
 * @date        
 * @version     
 * @pre         N/A.
 * @warning     N/A
 * @pre        
 */


#ifndef serialCLI_H
#define serialCLI_H

//todo: is there any point to the CLI_RXBUFFER being larger than the RX circular buffer of the (default: 256 bytes) BufferedSerial object?
//      - possibly yes, if very long lines need to be received
#define CLI_RX_BUFFER_SIZE 256

//TODO: testing needed on how often these need to run, especially to use higher (115200+) serial speeds
//      - on the whole the bandwidth heavy part will probably be TX
#define RX_CHECK_INTERVAL_ms 10
#define RX_PROCESS_INTERVAL_ms 10

#include "mbed.h"

#include <string>
#include <sstream>
#include <map>
#include "UARTSerial.h"


class serialCLI
{
    public:
        typedef enum class lineCommandType
        {
            GET,
            SET
        };

        //take the serial interface to read bytes from, and Thread variable from 
        serialCLI(UARTSerial* serialInterface);

        //attach a handler for a given command
        //handlers are passed a string of everything after the command and whitespace
        int32_t attachVariableHandler(std::string command, std::function<void(std::string*, lineCommandType, serialCLI*)> command_handler);

        //Handles serial output by wrapping around write()
        //public so that main program threads can use the same interface
        //TODO: make extra wrappers printfErrorLine, printfInfoLine, printfDataLine that prepend E:/I:/D: and append CRLF
        void printfCLI(const char fmt[], ...);
        //make call to write() of serial interface
        //TODO: enqueue calls in an EventQueue that always dispatches instead? may be useful if the blocking of this call becomes a problem
        void write(const char* str, uint32_t len){ this->serialInterface->write(str, len); };
        
        void printfCLI(std::string* str){ write(str->c_str(), str->length()); }


        ~serialCLI();


    private://TODO: Replace printfs with call to queue handled by thread
        //TODO: separate printf callback for debug messages?
        //Or as a way of thread-safe sharing of a serial port?

        //Pointer to serial object used for CLI
        UARTSerial* serialInterface;

        //Thread which is running the serial receive code
        //Destructor will stop all threads
        Thread _thread_serialReceive;
        Thread _thread_serialProcess;

        //each line sent as a separate string as they are received
        Mail<std::string, 16> line_mail;

        //Buffer for receiving lines from Serial
        char RXBUFFER[CLI_RX_BUFFER_SIZE];

        //todo: use static allocation here instead?
        std::map< std::string, std::function<void(std::string*, lineCommandType, serialCLI*)> > _attachedCommands;


        void wipeRXBuffer(); //set RXBUFFER to all zeros

        //receives bytes from serialInterface in a loop
        //call parseLine when a newline is received
        void inputReceiveThread();
        void inputProcessThread();

};

#endif