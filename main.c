//#include "cMIPS/include/uart_defs.h"

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
	ctrl = novo;
} 
#include "cMIPS.h"

#include "uart_defs.h"

#define U_DATA 0
#define U_FLAG 1

// operate at 1/4 of the highest data rate
#define SPEED 2   


int main(void) {
	Tserial volatile *uart;
	Tcontrol ctrl;
	Tstatus stat;
	extern   int Ud[2];  // declared in include/handlers.s
	volatile int *bfr;
	char c;
	int num, aux, i;

	char filaT[1024];
	int t_h,  t_t;

	char filaR[256]
	int r_h,  r_t;

	bfr = (int *)Ud;
	uart = (void *)IO_UART_ADDR; // bottom of UART address range

	ctrl.ign   = 0;
	ctrl.rts   = 0;  // make RTS=0 to hold remote unit inactive
	ctrl.intTX = 0;
	ctrl.intRX = 0;
	ctrl.speed = SPEED;
	uart->ctl = ctrl; // initizlize UART

	// handler sets flag=[U_FLAg] to 1 after new character is received;
	// this program resets the flag on fetching a new character from buffer
	bfr[U_FLAG] = 0;      //   reset flag  

	ctrl.ign   = 0;
	ctrl.rts   = 1;  // make RTS=1 to activate remote unit
	ctrl.intTX = 0;
	ctrl.intRX = 1;  // do generate interrupts on RXbuffer full
	ctrl.speed = SPEED;  // operate at 1/4 of the highest data rate
	uart->ctl = ctrl;
	
//preenche a fila;
	do{
		while(proberx()>0);
		do{
			c=Getc();
			if((c!=EOT)&& (c!='\n')){
				
				filaR[r_h]=c;
				r_h= (r_h+1)%256;
			}
		
		 }while ((c!= EOT)&& (c!='\n'));

		//transformaçao dos octetos e soma
		num=0;
		i=1;
		while (r_h!=r_t){
			num += i*c2i(fila[r_t]);
			r_t = (r_t+1)%256;
			i*=10;
		}
		//interpretaçao
		num=fibo(num);


		//envia para fila de transmiçao
		aux=0 
		i=10;
		while(num%i>0){
			i*=10;
		}

		while (i != 0){
			filaT[t_h]= i2c(num%i);
			t_h = (t_h+1)%1024;
			i=(int)(i/10);
			aux++;
		}
		while (aux<=8){
			filaT[t_h]='0'
			t_h= (t_h+1)%1024;
			aux++;
		}

		if(probetx()>0){
			while (t_h!=t_t){
				Putc(filaT[t_t]);
				t_t= (t_t+1)%1024;
			}
		}
		}while(c!=EOT);

}