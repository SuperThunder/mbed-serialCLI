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

#define RX_CHECK_INTERVAL_ms 5
#define RX_PROCESS_INTERVAL_ms 100

#include "mbed.h"

#include <string>
#include "UARTSerial.h"



class serialCLI
{
    public:
        //take the serial interface to read bytes from, and Thread variable from 
        serialCLI(UARTSerial* serialInterface);

        //attach a handler for a given command
        //handlers are passed a string of everything after the command and whitespace
        int32_t attachCommand(std::string command, std::function<int32_t(std::string content)> );

        

        uint32_t inputReceive_ProcessRxBuffer(ssize_t read_status);


        //TODO: hook directly to serial Rx rather than using parseLine
        //function to be attached in constructor to a serial interface Receive
        //int32_t serialRxAttachment()

        ~serialCLI();

    private:
        //TODO: separate printf callback for debug messages?
        //Or as a way of thread-safe sharing of a serial port?
        //std::function<uint32_t(std::string)> printf_callback;

        //Or just directly point to Serial object instead?
        UARTSerial* serialInterface;

        //Thread which is running the serial receive code
        //Destructor will stop all threads
        Thread _thread_serialReceive;
        Thread _thread_serialProcess;

        //each line sent as a separate string as they are received
        Mail<std::string, 16> line_mail;

        //Buffer for receiving lines from Serial
        //TODO: Replace with CircularBuffer so that processing and writing can happen interleaved
        char RXBUFFER[CLI_RX_BUFFER_SIZE];

        uint32_t RXBUFFER_INDEX;

        //parses a received serial line and calls the appropriate callback
        //called only by input receive thread
        int32_t parseLine(char* buffer, uint32_t bufsize);

        void flushRXBuffer(); //set RXBUFFER to all zeros

        //receives bytes from serialInterface in a loop
        //call parseLine when a newline is received
        void inputReceiveThread();
        void inputProcessThread();

};

#endif