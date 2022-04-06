#define ADC_BASE 0xFF204000
#define Mask_12_bits 0x00000FFF
#define JP1_BASE 0xFF200060
#define SW_BASE 0xFF200040
/* This program
 * 1. updates the ADC in single conversion mode and
 * 2. reads the corresponding  values from  channels 0, 2, and 3
 */
int main(void)
{
    volatile int *const sw_ptr = (int *)SW_BASE;
    volatile int *GPIO_LED_ptr = (int *)JP1_BASE;
    volatile int *ADC_BASE_ptr = (int *)ADC_BASE;
    int mask = Mask_12_bits;
    int bit_mask = 1 << 15;
    volatile int channel0, channel1;
    int adc_data0, adc_data1;

    *(ADC_BASE_ptr + 1) = 1; // write 1 to channel 1 to auto update ADC

    *(GPIO_LED_ptr + 1) = 1023; // configure pins 0 - 9 as output


    while (1)
    {
        // read from ADC
        adc_data0 = *(ADC_BASE_ptr);
        // check if ADC is ready
        if (adc_data0 &= bit_mask)
        {
            // remove bit 15 and store in local variable
            channel0 = adc_data0 - bit_mask;
        }
        // read from ADC
        adc_data1 = (*(ADC_BASE_ptr) + 1);
        // check if ADC is ready
        if (adc_data1 &= bit_mask)
        {
            // remove bit 15 and store in array
            channel1 = adc_data0 - bit_mask;
        }

        if (*sw_ptr & 1)
        {
            // display channel 1 readings            
            if (channel1 = 1)
            {
                *(GPIO_LED_ptr) = 1;
            }
            else if (channel1 = 2)
            {
                *(GPIO_LED_ptr) = 3;
            }
            else if (channel1 = 3)
            {
                *(GPIO_LED_ptr) = 7;
            }
            else if (channel1 = 4)
            {
                *(GPIO_LED_ptr) = 15;
            }
            else if (channel1 = 5)
            {
                *(GPIO_LED_ptr) = 31;
            }
            else if (channel1 = 6)
            {
                *(GPIO_LED_ptr) = 63;
            }
            else if (channel1 = 7)
            {
                *(GPIO_LED_ptr) = 127;
            }
            else if (channel1 = 8)
            {
                *(GPIO_LED_ptr) = 255;
            }
            else if (channel1 = 9)
            {
                *(GPIO_LED_ptr) = 511;
            }
            else if (channel1 = 10)
            {
                *(GPIO_LED_ptr) = 1023;
            }
        }
        else
        {
            // display channel 0 readings
            // display channel 1 readings            
            if (channel0 = 1)
            {
                *(GPIO_LED_ptr) = 1;
            }
            else if (channel0 = 2)
            {
                *(GPIO_LED_ptr) = 3;
            }
            else if (channel0 = 3)
            {
                *(GPIO_LED_ptr) = 7;
            }
            else if (channel0 = 4)
            {
                *(GPIO_LED_ptr) = 15;
            }
            else if (channel0 = 5)
            {
                *(GPIO_LED_ptr) = 31;
            }
            else if (channel0 = 6)
            {
                *(GPIO_LED_ptr) = 63;
            }
            else if (channel0 = 7)
            {
                *(GPIO_LED_ptr) = 127;
            }
            else if (channel0 = 8)
            {
                *(GPIO_LED_ptr) = 255;
            }
            else if (channel0 = 9)
            {
                *(GPIO_LED_ptr) = 511;
            }
            else if (channel0 = 10)
            {
                *(GPIO_LED_ptr) = 1023;
            }
        }
    }
}
