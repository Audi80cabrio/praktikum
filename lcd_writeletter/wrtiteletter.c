void LCD_WriteLetter(char letter) {
    int offset = letter * 40;  // Position im Array berechnen

    for (int row = 0; row < 20; row++) {
        unsigned char byte1 = console_font_10x20[offset + row * 2];     // erstes Byte der Zeile
        unsigned char byte2 = console_font_10x20[offset + row * 2 + 1]; // zweites Byte der Zeile

        for (int bit = 0; bit < 8; bit++) {
            if (byte1 & (1 << (7 - bit))) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        for (int bit = 0; bit < 2; bit++) {
            if (byte2 & (1 << (7 - bit))) {
                printf("*");
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }
}
