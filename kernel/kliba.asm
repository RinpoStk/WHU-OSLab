
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;			       klib.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;							Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; 导入全局变量
extern	disp_pos
extern  PageDirBase
extern  PageTblBase
extern  dwPDENum
extern  szPageAllocErr
extern  szPageFreeErr


[SECTION .text]

; 导出函数
global	disp_str
global	disp_color_str
global	out_byte
global	in_byte
global	enable_irq
global	disable_irq
global	enable_int
global	disable_int
global	port_read
global	port_write
global	glitter

global  alloc_pages
global  free_pages


; ========================================================================
;		   void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp
    push ebx
    push esi
    push edi

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

    pop edi
    pop esi
    pop ebx
	pop	ebp
	ret

; ========================================================================
;		   void disp_color_str(char * info, int color);
; ========================================================================
disp_color_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12]	; color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

; ========================================================================
;		   void out_byte(u16 port, u8 value);
; ========================================================================
out_byte:
	mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;		   u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;                  void port_read(u16 port, void* buf, int n);
; ========================================================================
port_read:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	insw
	ret

; ========================================================================
;                  void port_write(u16 port, void* buf, int n);
; ========================================================================
port_write:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	outsw
	ret

; ========================================================================
;		   void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;	}
disable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl			; ah = (1 << (irq % 8))
	cmp	cl, 8
	jae	disable_8		; disable irq >= 8 at the slave 8259
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
dis_already:
	popf
	xor	eax, eax		; already disabled
	ret

; ========================================================================
;		   void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;	}
;
enable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl			; ah = ~(1 << (irq % 8))
	cmp	cl, 8
	jae	enable_8		; enable irq >= 8 at the slave 8259
enable_0:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al	; clear bit at master 8259
	popf
	ret
enable_8:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al	; clear bit at slave 8259
	popf
	ret

; ========================================================================
;		   void disable_int();
; ========================================================================
disable_int:
	cli
	ret

; ========================================================================
;		   void enable_int();
; ========================================================================
enable_int:
	sti
	ret

; ========================================================================
;                  void glitter(int row, int col);
; ========================================================================
glitter:
	push	eax
	push	ebx
	push	edx

	mov	eax, [.current_char]
	inc	eax
	cmp	eax, .strlen
	je	.1
	jmp	.2
.1:
	xor	eax, eax
.2:
	mov	[.current_char], eax
	mov	dl, byte [eax + .glitter_str]

	xor	eax, eax
	mov	al, [esp + 16]		; row
	mov	bl, .line_width
	mul	bl			; ax <- row * 80
	mov	bx, [esp + 20]		; col
	add	ax, bx
	shl	ax, 1
	movzx	eax, ax

	mov	[gs:eax], dl

	inc	eax
	mov	byte [gs:eax], 4

	jmp	.end

.current_char:	dd	0
.glitter_str:	db	'-\|/'
		db	'1234567890'
		db	'abcdefghijklmnopqrstuvwxyz'
		db	'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
.strlen		equ	$ - .glitter_str
.line_width	equ	80

.end:
	pop	edx
	pop	ebx
	pop	eax
	ret


find_free_pages:
    ;[ebp+8] 需要分配的页数目
    ;[ebp-16] 返回分配的页的首地址
    ;[ebp-20] 已分配的页数目
    push ebp
    mov ebp,esp
    sub esp,8
    push ebx
    push edi
    push esi
    mov dword [ebp-16],0  ; push 0  
    mov dword [ebp-20],0  ; push 0
    xor ebx,ebx
    mov edi,0xffffffff
    mov ecx,1023

.find_addr:
    inc ecx
    cmp ecx,1024
    jnz .pass
    xor ecx,ecx

    inc edi
    mov edx,[dwPDENum]
    ; mov edx,[edx]

    dec edx
    cmp edi,edx
    
    je .pageAllocErr

.pass:
    mov eax, [PageDirBase]
    
    and eax,0xfffff000
    mov ebx,[eax+edi*4]
    and ebx,0xfffff000

    mov eax,edi
    shl eax,22
    mov edx,ecx
    shl edx,12
    add eax,edx
    mov [ebp-16],eax
    mov dword[ebp-20],0
.1:
.2:
    mov eax,[ebx+ecx*4]
    test eax,1
    jnz .find_addr
    mov eax,[ebp-20]
    add eax,1
    cmp eax,[ebp+8]
    jz .back                    ;累计已分配的页数，若分配完毕则返回
    mov [ebp-20],eax
    inc ecx
    cmp ecx,1024
    jle .2

    mov ecx,0                   ;一个PDE对应的页已经分配完毕，开始分配下一个PDE的页
    inc edi
    mov edx,[dwPDENum]
    ; mov edx,[edx]
    dec edx
    cmp edi,edx
    je .pageAllocErr

    jmp .1

.back:
    mov eax,[ebp-16]
    jmp .return

.pageAllocErr:
    xchg bx,bx
    push szPageAllocErr
    call disp_str
    add esp,4
    hlt

.return:
    pop esi
    pop edi
    pop ebx
    add esp,8
    mov esp,ebp
    pop ebp
    ret

alloc_pages:
    ;[ebp+8] 需要分配的页数目
    ;[ebp+12] 返回分配的页的首地址
    push ebp
    mov ebp,esp
    sub esp,8
    push ebx
    push edi
    push esi
    xchg bx,bx
    push dword [ebp+8]
    call find_free_pages
    add esp,4
    


    ; mov [ebp-4],eax         ;push eax
    ; mov dword [ebp-8],0     ;push 0
    mov [ebp-16],eax
    mov dword [ebp-20],0

    ; push eax
    ; call disp_int
    ; add esp,4
    ; ret
    
    mov ebx,[ebp-16]
    shr ebx,22
    mov eax, [PageDirBase]
    
    and eax,0xfffff000
    mov edi,ebx
    mov ebx,[eax+edi*4]
    or ebx,1
    mov [es:edi*4],ebx

    and ebx,0xfffff000

    mov ecx,[ebp-16]
    shr ecx, 12
    and ecx, 0x3ff
.1:
.2:
    mov eax,[ebx+ecx*4]
    test eax,1
    jnz .pageAllocErr
    or eax,1
    mov [ebx+ecx*4],eax      ;获取PTE并修改P位为1

    mov eax,[ebp-20]
    add eax,1
    cmp eax,[ebp+8]
    jz .back                    ;累计已分配的页数，若分配完毕则返回
    mov [ebp-20],eax
    inc ecx
    cmp ecx,1024
    jle .2

    mov ecx,0                   ;一个PDE对应的页已经分配完毕，开始分配下一个PDE的页
    mov eax, [PageDirBase]
    
    and eax,0xfffff000
    inc edi
    mov ebx,[eax+edi*4]
    or ebx,1
    mov [eax+edi*4],ebx          
    and ebx,0xfffff000
    jmp .1

.back:
    mov eax,[ebp-16]
    jmp .return

.pageAllocErr:
    push szPageAllocErr
    call disp_str
    add esp,4

.return:
    pop esi
    pop edi
    pop ebx
    add esp,8
    mov esp,ebp
    pop ebp
    ret

free_pages:
    ;[ebp+8] 需要释放的页数目
    ;[ebp+12] 需要释放的页的首地址
    ;[ebp-16] 已释放的页数目

    push ebp
    mov ebp,esp
    sub esp,4
    push ebx
    push edi
    push esi

    xchg bx,bx
    mov dword [ebp-16],0    ; push 0  

    mov ebx,[ebp+12]
    shr ebx,22
    mov eax, [PageDirBase]
    
    and eax,0xfffff000
    mov edi,ebx
    mov ebx,[eax+edi*4]
    test ebx,1
    jz .pageFreeErr         ;若P位为0则报错
    and ebx,0xfffff000

    mov ecx,[ebp+12]
    shr ecx, 12
    and ecx, 0x3ff
.1:
.2:
    mov edx,ecx
    mov eax,[ebx+ecx*4]
    test eax,1
    jz .pageFreeErr
    and eax,0xfffffffe
    mov [ebx+ecx*4],eax
    mov eax,[ebp-16]
    add eax,1
    cmp eax,[ebp+8]
    jz .back                ;释放完毕则返回
    mov [ebp-16],eax
    inc ecx
    cmp ecx,1024
    jle .2

    mov ecx,0
    mov eax, [PageDirBase]
    
    and eax,0xfffff000

    cmp edx,0                 ;若一个PDE对应的所有PTE均被释放则释放PDE
    jne .pass
    mov ebx,[eax+edi*4]
    and ebx,0xfffffffe
    mov [eax+edi*4],ebx
.pass:
    inc edi
    mov ebx,[eax+edi*4]
    and ebx,0xfffff000
    jmp .1

.back:
    mov eax,[esp+4]
    ; push eax
    ; call DispInt
    ; add esp,4
    jmp .return

.pageFreeErr:
    push szPageFreeErr
    call disp_str
    add esp,4

.return:
    pop esi
    pop edi
    pop ebx
    add esp,4
    mov esp,ebp
    pop ebp
    ret

