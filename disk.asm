; big and beautiful code
; even has error checking
[org 0x7c00]
mov [BOOT_DISK], dl ; BIOS puts the current drive number in dl for us

; setting up the stack

xor ax, ax
mov es, ax ; nasm doesn't like assigning immediate (is this the right term?) values to es
mov ds, ax
mov bp, 0x8000 ; mmmmm magick numbrssss
mov sp, bp

mov ah, 2 ; Tells BIOS we want to read data
mov al, 1 ; Number of Sectors to read
mov ch, 0 ; Cylinder Number
mov cl, 2 ; Sector number (indexes start at 1)
mov dh, 0 ; Head Number
mov dl, [BOOT_DISK]

mov bx, 0x7e00 ; load the data at this address
int 0x13

mov ah, 0x0e

jc bad ; jump if carry flag is set

cmp al, 1 ; read the correct number of sectors?
jne bad

mov al, 'S'
jmp prt
bad:
  mov al, 'F'
prt:
int 0x10
jmp $
BOOT_DISK: db 0

; prints a NUL-terminated string
; bx: pointer to str
strprt:
  mov ah, 0x0e
loop:
  mov al, [bx]
  cmp al, 0
  je endloop
  int 0x10
  add bx,2
  jmp loop
endloop:
  ret

times 510-($-$$) db 0
db 0x55, 0xaa

times 512 db 'A'
