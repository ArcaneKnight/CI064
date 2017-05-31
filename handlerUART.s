lui   $k0, %hi(_uart_buff)  # get buffer's address
ori   $k0, $k0, %lo(_uart_buff)

sw    $a0, 5*4($k0)	    # save registers $a0,$a1,a2,a3
sw    $a1, 6*4($k0)
sw    $a2, 7*4($k0)
sw    $a3, 8*4($k0)

lui   $a0, %hi(HW_uart_addr) # get device's address
ori   $a0, $a0, %lo(HW_uart_addr)

lw    $k1, USTAT($a0) 	    # Read status
sw    $k1, 0*4($k0)         #  and save UART status to memory

li    $a1, U_rx_irq	    # remove interrupt request
sw    $a1, UINTER($t0)


and   $a1, $k1, $a1         # Is this reception?
beq   $a1, $zero, transmit   #   no, ignore it and return
nop
j u_rx
nop

transmit:
	li    $a1, U_tx_irq	    # remove interrupt request
	sw    $a1, UINTER($t0)

	and   $a1, $k1, $a1         # Is this transmission?
	beq   $a1, $zero, UARTret   #   no, ignore it and return
	nop
	j _u_tx
	nop

	# handle reception
u_rx:
	lw    $a1, UDATA($a0) 	    # Read data from device

	lui   $a2, %hi(Ud)          # get address for data & flag
	ori   $a2, $a2, %lo(Ud)

	# Reception space verification

	lw $a3, NRX($a2)
	slti $a3, $a3, Q_SZ
	beq	$a3, $zero, overrun #Reception FIFO is full
	nop


	lw $a3, RXTL($a2)  #a3 <= rxtail
	nop

	add $a2, $a2, $a3 	#ud +rx_tail
	sb $a1, RX_Q($a2)  # rx_q[rx_tail] <=data

	addi $a3, $a3, 1 # rxtail ++
	andi $a3, $a3 , (Q_SZ -1) # mod
	sw $a3, RXTL($a2) # rxtail <= rxtail+1


	sw    $a1, 0*4($a2)         #   and return from interrupt
	addiu $a1, $zero, 1
	sw    $a1, 1*4($a2)         # set flag to signal new arrival

	j transmit
	nop

# Reception FIFO is full
overrun:
	lw $a2, USTAT($a0)
	ori $a2, $a2, 0x20
	sw $a2, USTAT($a0) # bit RXCheio =1
	j UARTret
	nop

	# handle transmission
_u_tx: 
	lui   $a2, %hi(Ud)          # get address for data & flag
	ori   $a2, $a2, %lo(Ud)

	#Transmission FIFO verification

	lw $a3, NTX($a2)
	slti $a3, $a3, Q_SZ
	beq	$a3, $zero, underrun #FIFO is empty
	nop


	lw $a3, TXHD($a2)  #a3 <= txhead
	nop

	add $a2, $a2, $a3 	#ud +tx_head
	lb $a1, TX_Q($a2)  # $a1 <= tx_q[tx_head]

	addi $a3, $a3, 1 # txhead ++
	andi $a3, $a3, (Q_SZ -1) # mod
	sw $a3, TXHD($a2) # txhead <= txhead+1

	sw    $a1, UDATA($t0) 	    # save data to device


	sw    $a1, 0*4($a2)         #   and return from interrupt
	addiu $a1, $zero, 1
	sw    $a1, 1*4($a2)         # set flag to signal new arrival

	j UARTret
nop

#Transmission FIFO is empty

underrun:
	lw $a2, USTAT($t0)
	ori $a2, $a2, 0x40
	sw $a2, USTAT($t0) # bit TVazio =1
	j UARTret
	nop

UARTret:
	lw    $a3, 8*4($k0)
	lw    $a2, 7*4($k0)
	lw    $a1, 6*4($k0)         # restore registers $a0,$a1,a2,a3
	lw    $a0, 5*4($k0)

	eret			    # Return from interrupt


	
