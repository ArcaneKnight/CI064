_u_rx:
	lui   $k0, %hi(_uart_buff)  # get buffer's address
	ori   $k0, $k0, %lo(_uart_buff)
	
	sw    $a0, 5*4($k0)	    # save registers $a0,$a1, others?
	sw    $a1, 6*4($k0)
	sw    $a2, 7*4($k0)
	sw    $a3, 8*4($k0)

	lui   $a0, %hi(HW_uart_addr)     # get device's address
	ori   $a0, $a0, %lo(HW_uart_addr)

	lw    $k1, USTAT($a0) 	    # Read status

	li    $a1, U_rx_irq	    

	and   $a1, $k1, $a1         # Is this reception?
	beq   $a1, $zero, UARTtx
	nop

	# -------- handle reception -------------
	li    $a1, U_rx_irq	    
	sw    $a1, UINTER($a0)

	lw    $a1, UDATA($a0) 	    # Read data from device

	lui   $a2, %hi(Ud)
	ori   $a2, $a2, %lo(Ud)
	
	lw $a3, NRX($a2)    # a3 <= nrx
	nop
	addi $a3, $a3, 1    # nrx++
	sw $a3, NRX($a2)    # save nrx

	# A fila eh circular por isso tem que ser rx_tail modulo 16

	lw  $a3, RXTL($a2) # a3 <= rx_tail
	add $a2, $a2, $a3 # a2 <=  Ud + rx_tail

	sb $a1, RX_Q($a2)     # rx_q[rx_tail] <= UDATA
	
	lui   $a2, %hi(Ud)
    ori   $a2, $a2, %lo(Ud)

	addi $a3, $a3, 1     # rx_tail++
    andi $a3, $a3, 0xf   # modulo 16
    sw $a3, RXTL($a2)  # save rx_tail

	li    $a1, U_tx_irq

    and   $a1, $k1, $a1         # Eh transmissao?
    bne   $a1, $zero, UARTtx
    nop


UARTret:
	lw    $a3, 8*4($k0)
	lw    $a2, 7*4($k0)
	lw    $a1, 6*4($k0)         # restore registers $a0,$a1, others?
	lw    $a0, 5*4($k0)

	eret			    # Return from interrupt

UARTtx:

	lui   $a0, %hi(tx_has_started) # get device's address
	ori   $a0, $a0, %lo(tx_has_started) 

	lw $a1, 0($a0)

	bne $zero, $a1, AAA # if (tx_has_started == 0) {tx_has_started++}
	nop
	addi $a1, $a1, 1
	sw $a1, 0($a0)

AAA:

	lui   $a0, %hi(HW_uart_addr) # get device's address
	ori   $a0, $a0, %lo(HW_uart_addr)

	li    $a1, U_tx_irq         # remove interrupcao de transmissao
    sw    $a1, UINTER($a0)

	lui   $a2, %hi(Ud)
    ori   $a2, $a2, %lo(Ud)

	lw $a3, NTX($a2)    # a3 <= ntx
    
    addi $a1, $zero, 16
    beq $a3, $a1, UARTret # if (ntx == 16) { j UARTret}
    nop

    addi $a3, $a3, 1    # ntx++
    sw $a3, NTX($a2)    # save ntx

	lw  $a3, TXHD($a2) # a3 <= tx_head
    add $a2, $a2, $a3 # a2 <=  Ud + tx_head

    lbu $a1, TX_Q($a2)     # a1 <= tx_q[tx_head]
	sb $a1, UDATA($a0)   # UDATA <= tx_q[tx_head]

    lui   $a2, %hi(Ud)          
    ori   $a2, $a2, %lo(Ud)

    addi $a3, $a3, 1     # tx_head++
    andi $a3, $a3, 0xf   # modulo 16
    sw $a3, TXHD($a2)  # save tx_head
    
    addi $a2, $zero, 0x04 # a2 <= EOT
    bne $a1, $a2, UARTret # if (UDATA == EOT) {
    nop
    lw $a3, UCTRL($a0)
    andi $a3, $a3, 0xef
    sw $a3, UCTRL($a0) # }

	j UARTret
	nop

