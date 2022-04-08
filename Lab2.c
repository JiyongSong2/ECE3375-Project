#define KEY_BASE 0xFF200050
#define SW_BASE 0xFF200040
#define HEX3_HEX0_BASE 0xFF200020
#define HEX5_HEX4_BASE 0xFF200030
#define A9_TIMER_BASE 0xFFFEC600

#define CLEAR_TIMER 8
#define LAP_TIMER 4
#define STOP_TIMER 2
#define START_TIMER 1

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
int seconds = 0;	   // seconds
int minutes = 0;	   // minutes

int split_ms; // to store milli-seconds
int split_s;  // to store seconds
int split_m;  // to store minutes

int key_last_state = 0; // Last key press
int key_pressed;		// denotes what is the currently pressed key

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
					milli_seconds = 0;	   // reset milliseconds
					seconds = seconds + 1; // increment seconds
				}
				display_minutes();
			}
			else if (seconds > 0 && seconds < 60)
			{
				if (milli_seconds == 100) // when milliseconds rolls over
				{
					milli_seconds = 0;	   // reset milliseconds
					seconds = seconds + 1; // increment seconds
				}
				display_minutes();
			}
			else if (seconds = 60) // when seconds roll over
			{
				seconds = 0;		   // reset seconds
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
					milli_seconds = 0;	   // reset milliseconds
					seconds = seconds + 1; // increment seconds
				}
				display_minutes();
			}
			else if (seconds > 0 && seconds < 60)
			{
				if (milli_seconds == 100) // when milliseconds rolls over
				{
					milli_seconds = 0;	   // reset milliseconds
					seconds = seconds + 1; // increment seconds
				}
				display_minutes();
			}
			else if (seconds = 60)
			{
				if (milli_seconds == 100) // when milliseconds rolls over
				{
					milli_seconds = 0;	   // reset milliseconds
					seconds = seconds + 1; // increment seconds
				}
				seconds = 0;		   // reset seconds
				minutes = minutes + 1; // increment minutes
				display_minutes();
			}
		}
		else if (minutes = 60)
		{
			minutes = 0;				// reset minutes
			seconds = 0;				// reset seconds
			milli_seconds = 0;			// reset milli seconds
			intialize_timer(225000000); // reset time load value to count for one second
		}
	}
}

unsigned int getPressedKeys()
{											   // To find which key is pressed
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
	split_s = seconds;		  // capture seconds
	split_m = minutes;		  // capture minutes
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
	minutes_pattern = 32768 + hex_code[split_m_tens]* 256 + hex_code[split_m_ones];
	*(hex2_ptr) = minutes_pattern;
}

void initialize()
{
	// reset the digits
	*(hex1_ptr) = 0;
	*(hex2_ptr) = 0;
	intialize_timer(2250000); // set the load value in A9 private timer to count in milliseconds
	key_pressed = 0;		  // set the pressed key state to zero
	milli_seconds = 0;		  // reset milli seconds to zero
	seconds = 0;			  // reset the seconds to zero
	minutes = 0;			  // reset minutes to zero
}

int main(void)
{
	while (1)
	{ // Main Application loop

		initialize(); // reset the system first
		display_minutes();
		key_pressed = getPressedKeys(); // check which key is pressed

		RESET: if (key_pressed & 0x1)
		{ // start the counter when button 1 is pressed
			initialize();
			while (1)
			{
				key_pressed = getPressedKeys(); // check if other keys are pressed

				if (!(key_pressed & 2) && !(key_pressed & 4) && !(key_pressed & 8))
				{				   // if no other key is pressed, continue to count
					start_timer(); // start timer
					if (*sw_ptr & 1)
					{						   // if button 3 is pressed, run in loop to display the captured splits
					split_timer_display(); // display the split time
					}
				}
				else if (key_pressed & 2) // button 2 is pressed
				{
					while (1)
					{
						display_minutes();
						key_pressed = getPressedKeys(); // check which key is pressed

						if (key_pressed & 1) // button 1 is pressed again
						{
							break;
						}
					}	
				}
				else if (key_pressed & 4)
				{						   // if button 3 is pressed then,
					split_timer_capture(); // goto function that captures the current time
				}
				// button 4 is pressed	
				else if (key_pressed & 8)
				{	
					// reset the digits and pressed buttons
					*(hex1_ptr) = 0;
					*(hex2_ptr) = 0;
					key_pressed = 0;
					// reset time		
					milli_seconds = 0;		
					seconds = 0;			 
					minutes = 0; 
					split_ms = 0;
					split_s = 0;
					split_m = 0;
					display_minutes();

					while (1)
					{
						
						key_pressed = getPressedKeys();
						if (key_pressed & 1) // button 1 is pressed again
						{
							goto RESET;							
						}					
					}
					
				}		
				
			}
				
		}
	}
}