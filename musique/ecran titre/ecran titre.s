	.include "MPlayDef.s"

	.equ	pardrink_grp, voicegroup000
	.equ	pardrink_pri, 0
	.equ	pardrink_rev, 0
	.equ	pardrink_mvl, 127
	.equ	pardrink_key, 0
	.equ	pardrink_tbs, 1
	.equ	pardrink_exg, 0
	.equ	pardrink_cmp, 1

	.section .rodata
	.global	pardrink
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

pardrink_1:
	.byte		VOL   , 127*pardrink_mvl/mxv
	.byte	KEYSH , pardrink_key+0
@ 000   ----------------------------------------
	.byte	TEMPO , 4*pardrink_tbs/2
	.byte		VOICE , 0
	.byte		N96   , Cn3 , v064
	.byte	W96
@ 001   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

pardrink:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	pardrink_pri	@ Priority
	.byte	pardrink_rev	@ Reverb.

	.word	pardrink_grp

	.word	pardrink_1

	.end
