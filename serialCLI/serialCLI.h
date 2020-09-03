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
        int32_t attachVariableHandler(std::string command, std::function<int32_t(std::string*, lineCommandType)> command_handler);

        ~serialCLI();

    private:
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

        //parses a received serial line and calls the appropriate callback
        //called only by input process thread
        int32_t parseLine(char* buffer, uint32_t bufsize);

        std::map< std::string, std::function<int32_t(std::string*, lineCommandType)> > _attachedCommands;

        //use enum class so that we can use specific namespace line lineParseStates::get_Type
        typedef enum class lineParseStates
        {
            get_Type,
            get_VarName,
            get_Value,
            Done
        };

        void wipeRXBuffer(); //set RXBUFFER to all zeros

        //receives bytes from serialInterface in a loop
        //call parseLine when a newline is received
        void inputReceiveThread();
        void inputProcessThread();

};

#endif