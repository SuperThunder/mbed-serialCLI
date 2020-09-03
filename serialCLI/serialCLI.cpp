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
    this->flushRXBuffer();

    this->serialInterface = serialInterface;

    //don't block, don't want to slow down rest of program if there's no uart input
    serialInterface->set_blocking(false);

    //callback arguments: (function arguments, function)
    //class members need a class instance as an implicit argument
    this->_thread_serialReceive.start( callback( this, &serialCLI::inputReceiveThread ) );
    this->_thread_serialProcess.start( callback( this, &serialCLI::inputProcessThread ) );
}

/*
void serialCLI::asyncRxCallbackHandler()
{

} */

//TODO: handle case of RXBUFFER overflow
void serialCLI::inputReceiveThread()
{
    
    ssize_t read_status;
    const char delim[] = "\n";
    char* cur_char = 0;
    uint32_t rxbuffer_index = 0, last_newline_index = -1, newline_count = 0;
    std::string* mail_str;
    
    while(true)
    {
        //add any newly received chars to the buffer, but don't read more than remaining space in RXBUFFER
		read_status = this->serialInterface->read(this->RXBUFFER+rxbuffer_index, CLI_RX_BUFFER_SIZE-rxbuffer_index);
		
        //if any bytes were read
		if(read_status > 0)
		{
            //Handle potentially large chunk of text
            //eg call handler every time special character is encountered
            //Alternatively, use strtok and split on the special character and then handle each group
            for(int i=0; i < read_status; i++)
            {
                cur_char = &( this->RXBUFFER[rxbuffer_index] );

                //echo back each received char
                //would be better if we could write in blocks, but need to take special action when \n encountered
                serialInterface->write( cur_char, 1 );

                //If newline encountered
                //Also check for CR
                if(*cur_char == '\n' || *cur_char == '\r')
                {
                    newline_count++;

                    //send each line to inputProcessThread by Mail
                    //In future alloc will block if no mem available, overall probably desirable behaviour for the receive thread
                    mail_str = this->line_mail.alloc();

                    if(mail_str != 0)
                    {
                        mail_str->assign(RXBUFFER+last_newline_index+1, i - last_newline_index);
                    }

                    printf("\r\nSending: ");
                    printf(mail_str->c_str());
                    this->line_mail.put(mail_str);
                    
                    //handle CR LF or LF CR cases by skipping the second char of the sequence
                    if( *(cur_char+1) == '\n' || *(cur_char+1) == '\r')
                    {
                        i++;
                    }

                    last_newline_index = rxbuffer_index;
                }

                //for each char received from this read, update the buffer index
                rxbuffer_index++;
            }

            //if we got any newlines, then they have all been sent
            //so reset the RXBUFFER
            if(newline_count > 0)
            {
                rxbuffer_index = 0;
                last_newline_index = -1;
                newline_count = 0;
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

    while(true)
    {
        //process all received lines
        while( !(this->line_mail.empty()) )
        {
            osevt = line_mail.get();

            //have to get pointer from osEvent and then cast back to string
            tmp_line = (std::string*) osevt.value.p;

            serialInterface->write( "Received line: ", strlen("Received line: ") );
            printf( tmp_line->c_str(), tmp_line->length() );

            //look for command and value
        }

        ThisThread::sleep_for(RX_PROCESS_INTERVAL_ms);
    }
}

/* 
//Called in receive thread to enqueue individual strings
uint32_t serialCLI::inputReceive_ProcessRxBuffer(ssize_t read_status)
{
    //echo chars back (note: on Windows will carriage return but not line feed)
    //serialInterface->write(this->RXBUFFER, read_status);

    return newline_count;
}*/

/* 
//process lines of input
void serialCLI::inputProcessThread()
{
    osEvent osevt;
    std::string* tmp_line;

    while(true)
    {
        //process all received lines
        while( !(this->line_mail.empty()) )
        {
            osevt = line_mail.get();

            //have to get pointer from osEvent and then cast back to string
            tmp_line = (std::string*) osevt.value.p;

            printf("Received line: ");
            printf(tmp_line->c_str());

            //look for command and value
        }
        

        ThisThread::sleep_for(RX_CHECK_INTERVAL_ms);
    }
} */


void serialCLI::flushRXBuffer()
{
    for(int i = 0; i < CLI_RX_BUFFER_SIZE; i++)   
    {
        RXBUFFER[i] = 0;
    }

    return;
}

int32_t serialCLI::parseLine(char* buffer, uint32_t bufsize)
{
    return 0;
}

serialCLI::~serialCLI()
{
    this->_thread_serialReceive.terminate();
    this->_thread_serialProcess.terminate();
}