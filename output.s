#Prolog:
.text 
.globl  main 
main: 
move  $fp,  $sp 
la  $a0,  ProgStart 
li  $v0, 4 
syscall 
#End of Prolog
li $t0,20
sw $t0,-12($sp)
lw $a0,-12($sp)
li $t0,30
sw $t0,-16($sp)
lw $a1,-16($sp)
li $t0,40
sw $t0,-20($sp)
lw $a2,-20($sp)
jal avg
lw $t0,-24($sp)
sw $t0,-8($sp)
la $a0,write1
li $v0,4
syscall
lw $a0,-8($sp)
li $v0,1
syscall
j Postlog
avg:
addi $sp,$sp,-28
sw $fp,0($sp)
move $fp,$sp
sw $ra,-4($sp)
sw $a0,-8($sp)
sw $a1,-12($sp)
sw $a2,-16($sp)
lw $a0,-8($sp)
lw $a1,-12($sp)
lw $a2,-16($sp)
jal sum
lw $t0,-24($sp)
sw $t0,-20($sp)
li $t0,3
sw $t0,-28($sp)
lw $t0,-20($sp)
lw $t1,-28($sp)
div $t0,$t1
mflo $t0
sw $t0-32($sp)
lw $t0,-32($sp)
lw $t1,0($sp)
lw $t2,-4($sp)
move $sp,$t1
move $fp,$t1
sw $t0,-24($sp)
jr $t2
j Postlog
sum:
addi $sp,$sp,-36
sw $fp,0($sp)
move $fp,$sp
sw $ra,-4($sp)
sw $a0,-8($sp)
sw $a1,-12($sp)
sw $a2,-16($sp)
lw $t0,-8($sp)
lw $t1,-12($sp)
add $t0,$t0,$t1
sw $t0-20($sp)
lw $t0,-20($sp)
lw $t1,-16($sp)
add $t0,$t0,$t1
sw $t0-24($sp)
lw $t0,-24($sp)
lw $t1,0($sp)
lw $t2,-4($sp)
move $sp,$t1
move $fp,$t1
sw $t0,-24($sp)
jr $t2
Postlog:
la $a0, ProgEnd
li $v0, 4
syscall
li $v0, 10
syscall
.data
write1: .asciiz "\nThe average of 20, 30 & 40 is\n"
ProgStart:  .asciiz	"Program Start\n"
ProgEnd:  .asciiz	"\nProgram End\n"
