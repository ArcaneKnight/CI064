#include "cMIPS.h"
#include "uart_defs.h"
#include "fib_vet.h"
//#include "../tests/uart_irx.c"
//#include "../include/handlersORIGINAL.s"



#define D_SZ 100
#define U_DATA 0
#define U_FLAG 1

// operate at 1/4 of the highest data rate
#define SPEED 4

extern UARTdriver volatile Ud;
Tserial volatile *uart;


char Getc ( void ) {

	char c ;
	disableInterr (); // exclui handler enquanto altera Ud

	Ud.nrx -= 1;
	c = Ud.rx_q [Ud.rx_hd ];
	Ud.rx_hd = (Ud.rx_hd + 1) & ( Q_SZ -1); // ( hd ++) mod Q_SZ

	enableInterr ();
	return c ;
}

int state=0;

void Putc(char octeto){

	uart = (Tserial *)IO_UART_ADDR;

	disableInterr();

	Ud.ntx-=1;
	Ud.tx_q[ Ud.tx_tl ] = octeto;
	Ud . tx_tl = ( Ud . tx_tl + 1) & ( Q_SZ -1); // ( tl ++) mod Q_SZ

	
	if (Ud.ntx == 15 && (state==0)){ //*
	    uart->interr.setTX = 1;
		state=1;
	}

	enableInterr();
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
	Tcontrol ctrl;

	ctrl = novo;
} 

//--DEBUG DEFINEs--------------------------
#define ETAPA 0
#define LOOP_TESTE 1
#define FIB_TESTE 0
//-----------------------------------------




int main(void) {
	Tserial volatile *uart;
	uart = (Tserial *)IO_UART_ADDR;

	Tcontrol ctrl;
	Tstatus stat;
	char c;
	int num, aux, i;

	char filaT[1024];
	static int t_h=0,
				  t_t=0;

	char filaR[256];
	static int r_h=0,
				  r_t=0;

	state = 0;
	//r_h =	r_t = t_h = t_t = 0;

	uart = (void *)IO_UART_ADDR; // bottom of UART address range

	ctrl.ign   = 0;
	ctrl.rts   = 0;  // make RTS=0 to hold remote unit inactive
	ctrl.intTX = 0;
	ctrl.intRX = 0;
	ctrl.speed = SPEED;
	uart->ctl = ctrl; // initizlize UART

	Ud.nrx = 0;
	Ud.rx_hd = 0;
	Ud.rx_tl = 0; 

	ctrl.ign   = 0;
	ctrl.rts   = 1;  // make RTS=1 to activate remote unit
	ctrl.intTX = 0;
	ctrl.intRX = 1;  // do generate interrupts on RXbuffer full
	ctrl.speed = SPEED;  // operate at 1/4 of the highest data rate
	uart->ctl = ctrl;
	
//preenche a fila;
	enableInterr();
	do{
																					if (ETAPA) print(1);
		while(proberx()==0){
		}
		do{
			c=Getc();
			if((c!=EOT)&& (c!='\n') && (c!=0)){
																					if (LOOP_TESTE) to_stdout('\n');
				to_stdout(c);
				//print(c);
				filaR[r_h]=c;
				r_h = (r_h+1)%256;

			}
		 }while ((c!= EOT)&& (c!='\n') && (c!=0));
		 																			if (ETAPA) print(2);
		//transformaçao dos octetos e soma
		num=0;
		i=16;
		while (r_h!=r_t){
			//print (filaR[r_t]);
			num += i*c2i(filaR[r_t]);
			r_t = (r_t+1)%256;
			i*=16;    // 16
		}
																					if (ETAPA) print(3);
		//interpretaçao
		 																			if (FIB_TESTE) print( num);
		num= dat[num];
		 																			if (FIB_TESTE) print( num);
		//envia para fila de transmiçao
		aux=0; 
		i=16;
		while(((int)(num/i))>15){
			i*=16;  // 16
		}
																					if (ETAPA) print(4);
		while (i > 1){
			filaT[t_h]= i2c(num%i);
			t_h = (t_h+1)%1024;
			i=(int)(i/16); // 16
			aux++;
		}
																					if (ETAPA) print(5);
		while (aux<=8){
		  filaT[t_h]='0';
			t_h= (t_h+1)%1024;
			aux++;
		}

		if(probetx()!=0){
			if (t_h!=t_t){
				Putc(filaT[t_t]);
				t_t= (t_t+1)%1024;
			}
		}
																					if (ETAPA) print(6);
		
		}while(c!=EOT);

																					if (ETAPA) print(7);
		return 0;
}
