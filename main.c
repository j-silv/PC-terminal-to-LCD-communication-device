int main(void) {
  char *terminal_msg;

  //Peripheral reset
  HAL_Init();
  
  //System clock configuration
  SystemClock_Config();

  //Configure peripherals
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  //Set up serial terminal
  init_Terminal();

  //Set up LCD screen
  LCD_init();

  while (1) {
    //Read then spit back serial terminal data
    terminal_msg = readprint_Input();

    //Print message to LCD
    LCD_print(terminal_msg);
  }
}



//Set up initial viewing for serial terminal
void init_Terminal(void) {

  char prompt[100];

  //Place text cursor at top left of terminal and clear the terminal
  HAL_UART_Transmit(&huart2, (uint8_t*)"\033[0;0H", strlen("\033[0;0H"), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t*)"\033[2J", strlen("\033[2J"), HAL_MAX_DELAY);

  //Print prompt
  sprintf(prompt,"%s","*******************************************************************************\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

  sprintf(prompt,"%s","**    SERIAL TERMINAL TO LCD SCREEN - STM32 ARM CORTEX-M0 MICROCONTROLLER    **\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

  sprintf(prompt,"%s","*******************************************************************************\r\n\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

  sprintf(prompt,"%s","Type your message and then press ENTER:\r\n\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

}

//Read text from serial terminal
char * readprint_Input(void) {

  char msg_byte[1], prompt[100];
  static char msg[MAX_MSG_SIZE]; //Static variable so we can return the message to main()
  uint8_t i;

  //Have to null array to 0 every time readprint_Input is called, otherwise previous msg appears
  for (i = 0; i < MAX_MSG_SIZE; i++) {
    msg[i] = '\0';
  }
  i = 0;

  do {
    //Polling for user to send text through serial terminal
    HAL_UART_Receive(&huart2, (uint8_t*)msg_byte, 1, HAL_MAX_DELAY);

    //If ENTER key is sent, don't add it to the text message
    if ((int)msg_byte[0] != '\r') {

      //Byte by byte, construct the text message
      msg[i]=msg_byte[0];
      i++;
    }

  } while ((int)msg_byte[0] != '\r'); //Stop receiving once ENTER key is sent

  //Spit back out what was transmitted
  sprintf(prompt,"%s","You entered: ");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t*)(msg), strlen(msg), HAL_MAX_DELAY);
  sprintf(prompt,"%s","\r\n\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

  sprintf(prompt,"%s","Type your message and then press ENTER:\r\n\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);

  return msg;
}

void LCD_init(void) {
  //Initialization instructions on pg. 26 of the ks0066 datasheet
  //Most delays are much longer than necessary - this is because HAL_Delay only takes in milliseconds
  //To improve time response, internal free-running counters can be used to get microsecond delays

  HAL_Delay(50);

  //FUNCTION SET
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
  LCD_data_line = 0x30 | LCD_2LINE | LCD_5x8DOTS;
  write8bits(LCD_data_line);

  HAL_Delay(1);

  //DISPLAY ON/OFF CONTROL
  LCD_data_line = 0x8 | LCD_CURSOROFF | LCD_BLINKOFF | LCD_DISPLAYON;
  write8bits(LCD_data_line);

  HAL_Delay(1);

  //DISPLAY CLEAR
  LCD_clear();

  //ENTRY MODE SET
  LCD_data_line = 0x4 | LCD_ENTRYLEFT | ~LCD_ENTRYSHIFTINCREMENT;
  write8bits(LCD_data_line);

  HAL_Delay(1);
}

void write8bits(uint8_t data) {
  HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, data & (0x1 << 0));
  HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, data & (0x1 << 1));
  HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, data & (0x1 << 2));
  HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, data & (0x1 << 3));
  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, data & (0x1 << 4));
  HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, data & (0x1 << 5));
  HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, data & (0x1 << 6));
  HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, data & (0x1 << 7));

  pulseEnable();
}


void pulseEnable(void) {
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
}


void LCD_print(char* text) {

  //Clear LCD before printing new message
  LCD_clear();

  //Print out 1 character at a time from the passed string
  for (uint8_t i = 0; i < strlen(text); i++) {

    //Set cursor to next line if cursor reaches end of the 1st row
    if (i == LCD_NUM_CHAR) {
      LCD_nextline();
    }

    //Select data register
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);

    LCD_data_line = (uint8_t)text[i];
    write8bits(LCD_data_line);
  }
}


void LCD_clear(void) {
  //Select instruction register
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);

  LCD_data_line = LCD_CLEARDISPLAY;
  write8bits(LCD_data_line);
  HAL_Delay(2);
}


void LCD_nextline(void) {
  //Select instruction register
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);

  LCD_data_line = LCD_SETDDRAMADDR | LCD_2LINE_DDRAM_START;
  write8bits(LCD_data_line);
  HAL_Delay(1);
}
