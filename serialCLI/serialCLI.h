/**
 * @brief       serialCLI.h
 * @details     command-value interactive serial input
 *
 *
 * @return      N/A
 *
 * @author      kuutei
 * @date        
 * @version     
 * @pre         N/A.
 * @warning     N/A
 * @pre        
 */


#ifndef serialCLI_H
#define serialCLI_H

//todo: is there any point to the CLI_RXBUFFER being larger than the RX circular buffer of the (default: 256 bytes) BufferedSerial object? 
#define CLI_RX_BUFFER_SIZE 1024

#define RX_CHECK_INTERVAL_ms 5

#include "mbed.h"

#include <string>
#include <cwchar>


class serialCLI
{
    public:
        //take the serial interface to read bytes from, and Thread variable from 
        serialCLI(BufferedSerial* serialInterface, Thread* serialCLIThread);

        //attach a handler for a given command
        //handlers are passed a string of everything after the command and whitespace
        int32_t attachCommand(std::string command, std::function<int32_t(std::string content)> );

        //receives bytes from serialInterface in a loop
        //call parseLine when a newline is received
        void inputReceiveThread();

        //void inputProcess();

        //void asyncRxCallbackHandler();

        //called for each received byte, prints back. fills up buffer, signals to inputProcess when newline encountered
        void inputReceiveISR();

        //TODO: hook directly to serial Rx rather than using parseLine
        //function to be attached in constructor to a serial interface Receive
        //int32_t serialRxAttachment()

        ~serialCLI();

    private:
        //TODO: separate printf callback for debug messages?
        //Or as a way of thread-safe sharing of a serial port?
        //std::function<uint32_t(std::string)> printf_callback;

        //Or just directly point to Serial object instead?
        BufferedSerial* serialInterface;

        //Thread which is running the serial receive code
        //Keep track of it so destructor can stop the serial receive thread
        Thread* serialRxThread;

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

};

#endif