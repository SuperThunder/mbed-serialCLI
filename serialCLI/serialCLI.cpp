/**
 * @brief       serialCLI.cpp
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

#include "serialCLI.h"

serialCLI::serialCLI(BufferedSerial* serialInterface, Thread* serialCLIThread)
{
    //init Rx buffer to zeros
    this->flushRXBuffer();

    this->serialInterface = serialInterface;
    this->serialRxThread = serialCLIThread;

    //call inputReceiveISR when bytes received
    //serialInterface->attach(callback(this, &serialCLI::inputReceiveISR), SerialBase::RxIrq);
    
    
    //Mysteriously removed in mbed6
    //serialInterface->read(this->RXBUFFER, CLI_RX_BUFFER_SIZE, callback(this, &serialCLI::asyncRxCallbackHandler), SERIAL_EVENT_RX_ALL, '\n')

    //don't block, don't want to slow down rest of program if there's no uart input
    serialInterface->set_blocking(false);

    //callback arguments: (function arguments, function)
    //class members need a class instance as an implicit argument
    serialCLIThread->start( callback( this, &serialCLI::inputReceiveThread ) );
}

/*
void serialCLI::asyncRxCallbackHandler()
{

} */

void serialCLI::inputReceiveThread()
{
    const char delim[] = "\n";
    ssize_t read_status;
    uint32_t last_newline_index;

    std::string* mail_str;

    while(true)
    {
		read_status = this->serialInterface->read(this->RXBUFFER, CLI_RX_BUFFER_SIZE);
		
        //if any bytes were read
		if(read_status > 0)
		{
			//Handle potentially large chunk of text
			//eg call handler every time special character is encountered
			//Alternatively, use strtok and split on the special character and then handle each
			for(int i=0; i < read_status; i++)
			{
                last_newline_index = -1;
                //If newline encountered
				if(this->RXBUFFER[i] == '\n')
                {
                    //send each line to inputProcessThread by Mail
                    //In future alloc will block if no mem available, overall probably desirable behaviour for this thread
                    mail_str = this->line_mail.try_alloc();

                    if(mail_str != 0)
                    {
                        //mail_str->assign(this->RXBUFFER, i);
                        *mail_str = "test";
                    }

                    //*mail_str = "test";
                    //std::string tmpstr(this->RXBUFFER+last_newline_index+1, i - last_newline_index);
                    //create a string that consists from the character after the previous newline to the currently found newline
                    
                    //weird linker errors
                    //assign(const char* s, size_t n)
                    //tmpstr->assign(RXBUFFER+last_newline_index+1, i - last_newline_index);

                    //this->line_mail.put(tmpstr);

                    last_newline_index = i;
                }
			}

            //echo chars back (note: on Windows will carriage return but not line feed)
            serialInterface->write(this->RXBUFFER, read_status);
		}   
         
        ThisThread::sleep_for(10ms);   
    }
}

/*
void serialCLI::inputProcessThread()
{
    while(true)
    {
        //echo input back


        //then process

        ThisThread::sleep_for(RX_CHECK_INTERVAL_ms);
    }
}
 */

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
    this->serialRxThread->terminate();
}