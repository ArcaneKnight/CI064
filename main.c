//#include "cMIPS/include/uart_defs.h"
#include	"cMIPS/tests/uart_irx.c"

//As funções proberx() e probetx() retornam os valores de nrx e ntx, respectivamente. A função iostat() retorna o conteúdo do registrador de status da UART, e a função ioctl() permite escrever no seu registrador de controle.
# define Q_SZ (1 < <4) // 16 , MUST be a power of two

int proberx ( void ); // retorna nrx
int probetx ( void ); // retorna ntx
Tstatus iostat ( void ); // retorna inteiro com status no byte menos sign .
void ioctl ( Tcontrol ); // escreve byte menos sign no reg . de controle
char Getc ( void ); // retira octeto da fila , decrementa nrx
void Putc ( char octeto); // insere octeto na fila , decrementa ntx



char Getc ( void ) {
	char c ;
	disable_interr (); // exclui handler enquanto altera Ud

	Ud . nrx -= 1;
	c = Ud . rx_q [ Ud . rx_hd ];
	Ud . rx_hd = ( Ud . rx_hd + 1) & ( Q_SZ -1); // ( hd ++) mod Q_SZ

	enable_interr ();
	return c ;
}


void Putc(char octeto){
	disable_interr();

	Ud.ntx-=1;
	Ud.tx_q[ Ud.tx_tl ] = octeto;
	Ud . tx_tl = ( Ud . tx_tl + 1) & ( Q_SZ -1); // ( tl ++) mod Q_SZ

	
	if (Ud . tx_tl== 1 && (state==0)){ //*
		state=1;
	}

	enable_interr();
}


int proberx(){
	return(Ud.nrx);
}

int probetx(){
	return(Ud.ntx);
}

Tstatus iostat ( void ){
	return(uart->stat);
}


void ioctl ( Tcontrol novo ){
	ctrl=novo;
} 