#include "consts.h"
.include "fq.inc"

.text
reduce128_avx:
#load
vmovdqu		(%rdi),%ymm2
vmovdqu		32(%rdi),%ymm3
vmovdqu		64(%rdi),%ymm4
vmovdqu		96(%rdi),%ymm5
vmovdqu		128(%rdi),%ymm6
vmovdqu		160(%rdi),%ymm7
vmovdqu		192(%rdi),%ymm8
vmovdqu		224(%rdi),%ymm9

red16		2
red16		3
red16		4
red16		5
red16		6
red16		7
red16		8
red16		9

#store
vmovdqu		%ymm2,(%rdi)
vmovdqu		%ymm3,32(%rdi)
vmovdqu		%ymm4,64(%rdi)
vmovdqu		%ymm5,96(%rdi)
vmovdqu		%ymm6,128(%rdi)
vmovdqu		%ymm7,160(%rdi)
vmovdqu		%ymm8,192(%rdi)
vmovdqu		%ymm9,224(%rdi)

ret

.global cdecl(reduce_avx)
cdecl(reduce_avx):
#consts
vmovdqu		_16XQ*2(%rsi),%ymm0
vmovdqu		_16XV*2(%rsi),%ymm1
call		reduce128_avx
add		$256,%rdi
call		reduce128_avx
ret

tomont128_avx:
#load
vmovdqu		(%rdi),%ymm3
vmovdqu		32(%rdi),%ymm4
vmovdqu		64(%rdi),%ymm5
vmovdqu		96(%rdi),%ymm6
vmovdqu		128(%rdi),%ymm7
vmovdqu		160(%rdi),%ymm8
vmovdqu		192(%rdi),%ymm9
vmovdqu		224(%rdi),%ymm10

fqmulprecomp	1,2,3,11
fqmulprecomp	1,2,4,12
fqmulprecomp	1,2,5,13
fqmulprecomp	1,2,6,14
fqmulprecomp	1,2,7,15
fqmulprecomp	1,2,8,11
fqmulprecomp	1,2,9,12
fqmulprecomp	1,2,10,13

#store
vmovdqu		%ymm3,(%rdi)
vmovdqu		%ymm4,32(%rdi)
vmovdqu		%ymm5,64(%rdi)
vmovdqu		%ymm6,96(%rdi)
vmovdqu		%ymm7,128(%rdi)
vmovdqu		%ymm8,160(%rdi)
vmovdqu		%ymm9,192(%rdi)
vmovdqu		%ymm10,224(%rdi)

ret

.global cdecl(tomont_avx)
cdecl(tomont_avx):
#consts
vmovdqu		_16XQ*2(%rsi),%ymm0
vmovdqu		_16XMONTSQLO*2(%rsi),%ymm1
vmovdqu		_16XMONTSQHI*2(%rsi),%ymm2
call		tomont128_avx
add		$256,%rdi
call		tomont128_avx
ret
