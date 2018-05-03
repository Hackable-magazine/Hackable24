#include <stdint.h>
#include <stdio.h>

uint8_t toto = 0x42;

void main(void)
{
	while(1) {
		putchar( ((toto-(toto%100))/100)+48 );
		putchar( (((toto%100)-(toto%10))/10)+48 );
		putchar( (toto%10)+48 );
		putchar(' ');
		puts("Coucou Hackable !\r");
		toto++;
	};
}
