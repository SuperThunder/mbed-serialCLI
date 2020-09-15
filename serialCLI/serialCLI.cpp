/**
 * @brief       serialCLI.cpp
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

#include "serialCLI.h"


serialCLI::serialCLI(UARTSerial* serialInterface)
{
    //init Rx buffer to zeros
    this->wipeRXBuffer();

    this->serialInterface = serialInterface;

    //don't block, don't want to slow down rest of program if there's no uart input
    serialInterface->set_blocking(false);

    //callback arguments: (function arguments, function)
    //class members need a class instance as an implicit argument
    this->_thread_serialReceive.start( callback( this, &serialCLI::inputReceiveThread ) );
    this->_thread_serialProcess.start( callback( this, &serialCLI::inputProcessThread ) );
}

int32_t serialCLI::attachVariableHandler(std::string command, std::function<void(std::string*, lineCommandType, serialCLI*)> command_handler)
{
    _attachedCommands[command] = command_handler;

    //todo: figure out success/failure conditions and return values
    return 0;
}

void serialCLI::printf(const char fmt[], ...)
{
    //sized to RX buffer size in case entire line needs to be written
    char buffer[CLI_RX_BUFFER_SIZE] = {0};
    uint32_t vsn_retval = 0;

    va_list args;
    va_start(args, fmt);
    
    //non negative and less than specified buffer size: entire string written to buffer
    vsn_retval = vsnprintf(buffer,CLI_RX_BUFFER_SIZE,fmt,args);
    
    va_end(args);
    
    if(vsn_retval > 0 && vsn_retval < CLI_RX_BUFFER_SIZE)
    {
        this->write(buffer, vsn_retval);
    }
    else if(vsn_retval >= CLI_RX_BUFFER_SIZE)
    {
        error("E: vsnprintf return value exceeded buffer size! (%d)\r\n", vsn_retval);
    }
    else
    {
        error("E: vsnprintf negative value (%d)\r\n", vsn_retval);
    }
}


//TODO: handle case of RXBUFFER overflow
void serialCLI::inputReceiveThread()
{
    
    ssize_t read_status;
    char* cur_char = 0;
    //keep track of: index of last unused char in receive buffer, index of the last newline found in the receive buffer, and the number of newlines encountered in one pass over the newline buffer
    uint32_t rxbuffer_index = 0, last_newline_index = -1;
    std::string* mail_str;
    
    while(true)
    {
        //add any newly received chars to the buffer, but don't read more than remaining space in RXBUFFER
        read_status = this->serialInterface->read(this->RXBUFFER+rxbuffer_index, CLI_RX_BUFFER_SIZE-rxbuffer_index);
		
        //if any bytes were read from serial
        if(read_status > 0)
        {
            for(int i=0; i < read_status; i++)
            {
                cur_char = &( this->RXBUFFER[rxbuffer_index] );

                //If newline encountered
                //Also check for CR
                if(*cur_char == '\n' || *cur_char == '\r')
                {

                    //handle CR LF or LF CR cases by skipping the second char of the sequence
                    if( *(cur_char+1) == '\n' || *(cur_char+1) == '\r')
                    {
                        i++;
                    }
                    
                    //but send CR LF in either case
                    {
                        //if newline received, 
                        this->printf("\r\n");
                    }
                    

                    //send each line to inputProcessThread by Mail
                    //In future alloc will block if no mem available, overall probably desirable behaviour for the receive thread
                    mail_str = this->line_mail.alloc();

                    if(mail_str != 0)
                    {
                        mail_str->assign(RXBUFFER+last_newline_index+1, rxbuffer_index - last_newline_index);
                    }

                    //printf("\Sending: ");
                    //printf(mail_str->c_str());
                    //printf("\r\n");
                    this->line_mail.put(mail_str);
                    
                    last_newline_index = rxbuffer_index;
                }
                else
                //directly echo back non-newline characters
                {
                    //echo back each received characters
                    //would be better if we could write in blocks, but need to take special action when \n encountered
                    serialInterface->write( cur_char, 1 );
                }

                //for each char received from this read, update the buffer index
                rxbuffer_index++;
            }

            //if newline was last char received, then we can reset buffer
            //this is the main mechanism to avoid overflowing the receive buffer
            if(last_newline_index == rxbuffer_index-1)
            {
                //debug("Resetting rxbuffer index %d->0\r\n", rxbuffer_index);

                rxbuffer_index = 0;
                last_newline_index = -1;
            }
		}
         
        ThisThread::sleep_for(RX_CHECK_INTERVAL_ms);   
    }
}

//Check Mail queue that contains lines of text received from serial
//Looks for commands and calls the relevant handler
void serialCLI::inputProcessThread()
{
    osEvent osevt;
    std::string* tmp_line;

    std::string command_type, var_name, var_content;
    std::stringstream ss;

    while(true)
    {
        //process all received lines
        while( !(this->line_mail.empty()) )
        {
            //reset string variables
            command_type = var_name = var_content = "";

            //get a mail item
            osevt = line_mail.get();

            //have to get pointer from osEvent and then cast back to string
            tmp_line = (std::string*) osevt.value.p;

            //debug("\r\nserialCLI inputProcessThread received line: ");
            //debug( tmp_line->c_str(), tmp_line->length() );
            //debug("\r\n");

            ss = std::stringstream(*tmp_line);

            ss >> command_type >> var_name >> var_content;

            //debug("Parsed from line: ||%s||%s||%s||\r\n", command_type.c_str(), var_name.c_str(), var_content.c_str());

            //TODO: Have an EventQueue for the handler function calls?
            //TODO: should get/set have their own handlers, which then deal with the variable name?
            if(command_type == "get" || command_type == "GET")
            {
                if(_attachedCommands.count(var_name))
                {
                    //call the handler function for the variable
                    std::function<void(std::string*, lineCommandType, serialCLI*)> handlerFunction = _attachedCommands[var_name];
                    handlerFunction( &var_content, lineCommandType::GET, this );
                }
                else
                {
                    //show error if no matching variable name
                    this->printf("E: Unknown attached variable for GET \'%s\'\r\n", var_name.c_str() );
                }
            }
            else if(command_type == "set" || command_type == "SET")
            {
                if(_attachedCommands.count(var_name))
                {
                    //call the handler function for the variable
                    std::function<void(std::string*, lineCommandType, serialCLI*)> handlerFunction = _attachedCommands[var_name];
                    handlerFunction( &var_content, lineCommandType::SET, this );
                }
                else
                {
                    this->printf("E: Unknown attached variable for SET \'%s\'\r\n", var_name.c_str() );
                }
            }
            else
            {
                this->printf("E: Unknown command type \'%s\'\r\n", command_type.c_str());
            }

            //free the memory used by mail for the message
            this->line_mail.free(tmp_line);

        }

        ThisThread::sleep_for(RX_PROCESS_INTERVAL_ms);
    }
}

void serialCLI::printLineMessage(serialCLI::lineMessageType outputType, std::string* s)
{
    std::string tmp = "";
    switch(outputType)
    {
        case serialCLI::lineMessageType::INFO:
            tmp += "I:" + *s + "\r\n";
            break;

        case serialCLI::lineMessageType::DATA:
            tmp += "D:" + *s + "\r\n";
            break;

        case serialCLI::lineMessageType::ERROR:
            tmp += "E:" + *s + "\r\n";
            break;
    }

    this->printf(&tmp);
}

void serialCLI::wipeRXBuffer()
{
    for(int i = 0; i < CLI_RX_BUFFER_SIZE; i++)   
    {
        RXBUFFER[i] = 0;
    }

    return;
}

serialCLI::~serialCLI()
{
    this->_thread_serialReceive.terminate();
    this->_thread_serialProcess.terminate();
}

