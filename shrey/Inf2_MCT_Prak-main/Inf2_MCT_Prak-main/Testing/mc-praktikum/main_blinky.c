/*
while ( 1 ) {

				// Display Backlighting with GPIOD PD13
				GPIOD -> ODR |= 1 << 13;

        uint16_t i = 0;
        uint16_t s = 1;

        for (i = 0; i <= 16; i++) {
            LEDs_Write(s);
            s = s << 1;
            u_delay(50000);
        }

        s = 1;

        LEDs_Write(0xFFFF);

        for (i = 0; i <= 16; i++) {
            LEDs_Write(~s);
            s = s << 1;
            u_delay(50000);
        }
    }
*/
