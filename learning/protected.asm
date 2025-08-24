[org 0x7c00]

mov [BOOT_DISK], dl

; equ is used to set constants
CODE_SEG equ code_descriptor - GDT_Start
DATA_SEG equ data_descriptor - GDT_Start

cli
lgdt [GDT_Descriptor]

; change last bit of cr0 to 1
mov eax, cr0 ; seems we also can't access this one directly
or eax, 1
mov cr0, eax

; far jump (jmp to another segment, our newly setup code segment in this case)
jmp CODE_SEG:start_protected_mode

jmp $ ; hopefully this isn't needed

GDT_Start: ; must be at the end of real mode code
  null_descriptor:
    dd 0 ; double word = 4 bytes
    dd 0
  ; defines a code segment with base 0 and limit 0xfffff
  code_descriptor:
    ; first 16 bits of limit
    dw 0xffff
    ; 24 bits of base
    dw 0 ; word = 2 bytes = 16 bits
    db 0 ; 8 bits
    ; present, privilege,type + type flags
    ; present: is this segment is use (1)
    ; priv (2 bits): ring level (00)
    ; type: 1 if code OR data (1)
    ; Type flags:
    ; Code: will this segment contain code? (1)
    ; conforming: can this code be executed from lower privileges? (0)
    ; readable: lets you read constants (1)
    ; accessed: managed by the CPU, will be set to one if it's currently using it
    db 0b10011010
    ; Other flags + last four bits of limit
    ; Other flags:
    ; Granularity: if 1, limit is multiplied by 0x1000 (1)
    ; 32bit? (1)
    ; last two flags unused here (set to 0)
    db 0b11001111
    ; last 8 bits of base
    db 0
  ; mostly the same as code
  data_descriptor:
    dw 0xffff
    dw 0
    db 0
    ; Type flags:
    ; Code? (0)
    ; direction flag: segment will grow negative if 1 (0)
    ; writable (1)
    ; accessed same as code
    db 0b10010010
    db 0b11001111
    db 0
GDT_End:

GDT_Descriptor:
  dw GDT_End - GDT_Start - 1 ; size
  dd GDT_Start               ; start



[bits 32]
start_protected_mode:
  ; So since we are now in protected mode (hopefully), all the BIOS functions are GONE
  ; How do we do the Hello World I hear you ask? Well, we just write the characters directly into memory (:neofox_googly_shocked:) 
  ; In text mode (which seems to be the default):
  ; video memory starts at 0xb8000
  ; first byte: character
  ; second byte: colour
  mov al, 'D' ; lower bits first because little endian (i hate little endian)
  mov ah, 0x0b
  mov [0xb8000], ax
  
  mov al, 'A'
  mov ah, 0x0d
  mov [0xb8002], ax
  
  mov al, 'G'
  mov ah, 0x0f
  mov [0xb8004], ax
  
  mov al, 'O'
  mov ah, 0x0d
  mov [0xb8006], ax
  
  mov al, 'N'
  mov ah, 0x0b
  mov [0xb8008], ax
  jmp $

; edx: address of string
; bl: color
strprt:
  mov ecx, 0xb8000
strprt_loop:
  mov al, [edx]
  
  cmp al, 0
  je strprt_loopend
  
  mov ah, bl
  mov [ecx], ax

  inc edx
  add ecx, 2
  
  jmp strprt_loop
strprt_loopend:
  ret

BOOT_DISK: db 0
hello:
  db "Hello World!", 0

times 510-($-$$) db 0
db 0x55, 0xaa
