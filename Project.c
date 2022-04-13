#define KEY_BASE 0xFF200050
#define SW_BASE 0xFF200040
#define HEX3_HEX0_BASE 0xFF200020
#define HEX5_HEX4_BASE 0xFF200030
#define A9_TIMER_BASE 0xFFFEC600

#define CLEAR_TIMER 8
#define LAP_TIMER 4
#define STOP_TIMER 2
#define START_TIMER 1

//#include "address_map_arm.h"
#define AUDIO_BASE 0xFF203040
#define LED_BASE 0xFF200000
#define KEY_BASE 0xFF200050
/* globals */
#define BUF_SIZE 80000   // about 10 seconds of buffer (@ 8K samples/sec)
#define BUF_THRESHOLD 96 // 75% of 128 word buffer
/* function prototypes */
void check_KEYs(int *, int *, int *);

typedef struct
{
    int load;
    int count;
    int control;
    int status;
} a9_timer;

int minutes_pattern = 0;
int seconds_pattern = 0;

// variable for each digit on the display (MM:SS:DD)
int m_tens = 0, m_ones = 0, s_tens = 0, s_ones = 0, d_tens = 0, d_ones = 0;

// variable for displaying the lapped time
int split_m_tens = 0, split_m_ones = 0, split_s_tens = 0, split_s_ones = 0;
int split_d_tens = 0, split_d_ones = 0;

int milli_seconds = 0; // milli_seconds
int seconds = 0;       // seconds
int minutes = 0;       // minutes

int split_ms; // to store milli-seconds
int split_s;  // to store seconds
int split_m;  // to store minutes

int key_last_state = 0; // Last key press
int key_pressed;        // denotes what is the currently pressed key

volatile int *const sw_ptr = (int *)SW_BASE;
volatile int *const key_ptr = (int *)KEY_BASE;
// 3rd - 6th digits on seven segment display
volatile int *const hex1_ptr = (int *)HEX3_HEX0_BASE;
// 1st - 2nd digits
volatile int *const hex2_ptr = (int *)HEX5_HEX4_BASE;
volatile a9_timer *const a9_ptr = (a9_timer *)A9_TIMER_BASE;

void stop_timer(int timer_select);
void set_timer(int period, int timer_select);
void poll_timer(int period, int timer_select);

void write_time(int time);

int hex_code[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

void intialize_timer(signed int timer_load_value)
{
    a9_ptr->load = timer_load_value; // set the timer load value to timer load address
    a9_ptr->control = 3 + (1 << 8);
}

// reset the timer interrupt. Timer starts to count again.
void reset_interrupt()
{
    a9_ptr->status = 0x1;
}

// for only displaying milliseconds
void display_milliseconds()
{
    seconds_pattern = hex_code[d_tens] * 256 + hex_code[d_ones];
    *(hex1_ptr) = seconds_pattern;
}

void display_seconds() // for displaying seconds and milliseconds
{
    seconds_pattern = hex_code[s_tens] * 16777216 + hex_code[s_ones] * 65536 + hex_code[d_tens] * 256 + hex_code[d_ones];
    *(hex1_ptr) = seconds_pattern;
}

void display_minutes() // for displaying everything
{
    seconds_pattern = hex_code[s_tens] * 16777216 + hex_code[s_ones] * 65536 + hex_code[d_tens] * 256 + hex_code[d_ones];
    *(hex1_ptr) = seconds_pattern;
    minutes_pattern = hex_code[m_tens] * 256 + hex_code[m_ones];
    *(hex2_ptr) = minutes_pattern;
}

void start_timer()
{
    // when timer is counting
    while (a9_ptr->status & 0x1)
    {
        reset_interrupt(); // reset the interrupt to keep timer going

        milli_seconds = milli_seconds + 1; // increment milliseconds first
        d_tens = milli_seconds / 10;
        d_ones = milli_seconds % 10;
        s_tens = seconds / 10;
        s_ones = seconds % 10;
        m_tens = minutes / 10;
        m_ones = minutes % 10;

        *(hex1_ptr) = 0;
        *(hex2_ptr) = 0;

        if (minutes == 0) // when there are no minutes to display, don't light up those digits
        {
            if (seconds == 0)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds > 0 && seconds < 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds = 60) // when seconds roll over
            {
                seconds = 0;           // reset seconds
                minutes = minutes + 1; // increment minutes
                display_minutes();
            }
        }
        else if (minutes > 0 && minutes <= 59)
        {
            if (seconds == 0)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds > 0 && seconds < 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds = 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                seconds = 0;           // reset seconds
                minutes = minutes + 1; // increment minutes
                display_minutes();
            }
        }
        else if (minutes = 60)
        {
            minutes = 0;                // reset minutes
            seconds = 0;                // reset seconds
            milli_seconds = 0;          // reset milli seconds
            intialize_timer(225000000); // reset time load value to count for one second
        }
    }
}

void count_up()
{
    // when timer is counting
    while (a9_ptr->status & 0x1)
    {
        reset_interrupt(); // reset the interrupt to keep timer going

        milli_seconds = milli_seconds + 1; // increment milliseconds first
        d_tens = milli_seconds / 10;
        d_ones = milli_seconds % 10;
        s_tens = seconds / 10;
        s_ones = seconds % 10;
        m_tens = minutes / 10;
        m_ones = minutes % 10;

        *(hex1_ptr) = 0;
        *(hex2_ptr) = 0;

        if (minutes == 0) // when there are no minutes to display, don't light up those digits
        {
            if (seconds == 0)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds > 0 && seconds < 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds = 60) // when seconds roll over
            {
                seconds = 0;           // reset seconds
                minutes = minutes + 1; // increment minutes
                display_minutes();
            }
        }
        else if (minutes > 0 && minutes <= 59)
        {
            if (seconds == 0)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds > 0 && seconds < 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                display_minutes();
            }
            else if (seconds = 60)
            {
                if (milli_seconds == 100) // when milliseconds rolls over
                {
                    milli_seconds = 0;     // reset milliseconds
                    seconds = seconds + 1; // increment seconds
                }
                seconds = 0;           // reset seconds
                minutes = minutes + 1; // increment minutes
                display_minutes();
            }
        }
        else if (minutes = 60)
        {
            minutes = 0;                // reset minutes
            seconds = 0;                // reset seconds
            milli_seconds = 0;          // reset milli seconds
            intialize_timer(100000000); // reset time load value to count for one second
        }
    }
}

unsigned int getPressedKeys()
{                                              // To find which key is pressed
    unsigned int key_current_state = *key_ptr; // find what key is pressed

    if (key_current_state != key_last_state)
    { // if the pressed key is different than previously pressed key

        key_last_state = key_current_state;
        key_pressed = key_current_state; // set the key_pressed to currently pressed key
    }
    else
    {
        key_pressed = 0; // if nothing is pressed, set the value to zero.
    }
    return key_pressed;
}

void split_timer_capture()
{
    split_ms = milli_seconds; // capture milliseconds
    split_s = seconds;        // capture seconds
    split_m = minutes;        // capture minutes
}
// Function used to display the captured split times
void split_timer_display()
{
    split_d_tens = split_ms / 10;
    split_d_ones = split_ms % 10;
    split_s_tens = split_s / 10;
    split_s_ones = split_s % 10;
    split_m_tens = split_m / 10;
    split_m_ones = split_m % 10;
    seconds_pattern = hex_code[split_s_tens] * 16777216 + hex_code[split_s_ones] * 65536 + hex_code[split_d_tens] * 256 + hex_code[split_d_ones];
    *(hex1_ptr) = seconds_pattern;
    minutes_pattern = 32768 + hex_code[split_m_tens] * 256 + hex_code[split_m_ones];
    *(hex2_ptr) = minutes_pattern;
}

void initialize()
{
    // reset the digits
    *(hex1_ptr) = 0;
    *(hex2_ptr) = 0;
    intialize_timer(1000000); // set the load value in A9 private timer to count in milliseconds
    key_pressed = 0;          // set the pressed key state to zero
    milli_seconds = 0;        // reset milli seconds to zero
    seconds = 0;              // reset the seconds to zero
    minutes = 0;              // reset minutes to zero
}

int main(void)
{

    int KEY_value;

    volatile int *red_LED_ptr = (int *)LED_BASE;
    volatile int *audio_ptr = (int *)AUDIO_BASE;
    /* used for audio record/playback */
    int fifospace;
    int record = 0, play = 0, buffer_index = 0;
    int left_buffer[BUF_SIZE];
    int right_buffer[BUF_SIZE];
    /* read and echo audio data */
    record = 0;
    play = 0;

    while (1)
    { // Main Application loop

        initialize(); // reset the system first

        key_pressed = getPressedKeys(); // check which key is pressed
        KEY_value = *(key_ptr);         // read the pushbutton KEY values

    RESET:
        if (KEY_value == 0x1)
        { // start the counter when button 1 is pressed
          // reset counter to start recording
            buffer_index = 0;
            // clear audio-in FIFO
            *(audio_ptr) = 0x4;
            *(audio_ptr) = 0x0;
            record = 1;
            initialize();
            display_minutes();
            while (record)
            {
                key_pressed = getPressedKeys(); // check if other keys are pressed

                // if no other key is pressed, continue to count
                start_timer(); // start timer

                *(red_LED_ptr) = 0x1; // turn on LEDR[0]
                fifospace =
                    *(audio_ptr + 1);                         // read the audio port fifospace register
                if ((fifospace & 0x000000FF) > BUF_THRESHOLD) // check RARC
                {
                    KEY_value = *(key_ptr); // read the pushbutton KEY values
                    // store data until the the audio-in FIFO is empty or the buffer
                    // is full
                    while ((fifospace & 0x000000FF) && (buffer_index < BUF_SIZE))
                    {
                        left_buffer[buffer_index] = *(audio_ptr + 2);
                        right_buffer[buffer_index] = *(audio_ptr + 3);
                        ++buffer_index;
                        if (buffer_index == BUF_SIZE || KEY_value != 0x1)
                        { // done recording
                            record = 0;
                            *(red_LED_ptr) = 0x0; // turn off LEDR
                            split_timer_capture();
                        }
                        fifospace = *(audio_ptr +
                                      1); // read the audio port fifospace register
                    }
                }
            }
            split_timer_display();
        }
        else if (KEY_value == 0x2) // button 2 is pressed
        {
            // reset counter to start recording
            buffer_index = 0;
            // clear audio-in FIFO
            *(audio_ptr) = 0x8;
            *(audio_ptr) = 0x0;
            play = 1;
            milli_seconds = 0; // reset milli seconds to zero
            seconds = 0;       // reset the seconds to zero
            minutes = 0;       // reset minutes to zero

            display_minutes();
            while (play)
            {
                start_timer();

                *(red_LED_ptr) = 0x2; // turn on LEDR_1
                fifospace =
                    *(audio_ptr + 1);                         // read the audio port fifospace register
                if ((fifospace & 0x00FF0000) > BUF_THRESHOLD) // check WSRC
                {
                    KEY_value = *(key_ptr);
                    // output data until the buffer is empty or the audio-out FIFO
                    // is full
                    while ((fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE))
                    {
                        *(audio_ptr + 2) = left_buffer[buffer_index];
                        *(audio_ptr + 3) = right_buffer[buffer_index];
                        ++buffer_index;
                        if (buffer_index == BUF_SIZE || (minutes == split_m && seconds == split_s && milli_seconds == split_ms)|| KEY_value != 0x2)
                        {
                            // done playback
                            play = 0;
                            *(red_LED_ptr) = 0x0; // turn off LEDR

                            KEY_value = 0;
                            // reset time
                            milli_seconds = 0;
                            seconds = 0;
                            minutes = 0;
                            split_timer_display();
                        }
                        fifospace = *(audio_ptr +
                                      1); // read the audio port fifospace register
                    }
                }
            }
            
        }
    }
}

/****************************************************************************************
 * Subroutine to read KEYs
 ****************************************************************************************/
void check_KEYs(int *KEY0, int *KEY1, int *counter)
{
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *audio_ptr = (int *)AUDIO_BASE;
    int KEY_value;
    KEY_value = *(KEY_ptr); // read the pushbutton KEY values
    while (*KEY_ptr)
        ;                 // wait for pushbutton KEY release
    if (KEY_value == 0x1) // check KEY0
    {
        // reset counter to start recording
        *counter = 0;
        // clear audio-in FIFO
        *(audio_ptr) = 0x4;
        *(audio_ptr) = 0x0;
        *KEY0 = 1;
    }
    else if (KEY_value == 0x2) // check KEY1
    {
        // reset counter to start playback
        *counter = 0;
        // clear audio-out FIFO
        *(audio_ptr) = 0x8;
        *(audio_ptr) = 0x0;
        *KEY1 = 1;
    }
}
