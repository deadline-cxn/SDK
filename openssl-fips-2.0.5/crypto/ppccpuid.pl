#!/usr/bin/env perl

$flavour = shift;

$0 =~ m/(.*[\/\\])[^\/\\]+$/; $dir=$1;
( $xlate="${dir}ppc-xlate.pl" and -f $xlate ) or
( $xlate="${dir}perlasm/ppc-xlate.pl" and -f $xlate) or
die "can't locate ppc-xlate.pl";

open STDOUT,"| $^X $xlate $flavour ".shift || die "can't call $xlate: $!";

if ($flavour=~/64/) {
    $CMPLI="cmpldi";
    $SHRLI="srdi";
    $SIGNX="extsw";
} else {
    $CMPLI="cmplwi";
    $SHRLI="srwi";
    $SIGNX="mr";
}

$code=<<___;
.machine	"any"
.text

.globl	.OPENSSL_ppc64_probe
.align	4
.OPENSSL_ppc64_probe:
	fcfid	f1,f1
	extrdi	r0,r0,32,0
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0

.globl	.OPENSSL_altivec_probe
.align	4
.OPENSSL_altivec_probe:
	.long	0x10000484	# vor	v0,v0,v0
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0

.globl	.OPENSSL_wipe_cpu
.align	4
.OPENSSL_wipe_cpu:
	xor	r0,r0,r0
	fmr	f0,f31
	fmr	f1,f31
	fmr	f2,f31
	mr	r3,r1
	fmr	f3,f31
	xor	r4,r4,r4
	fmr	f4,f31
	xor	r5,r5,r5
	fmr	f5,f31
	xor	r6,r6,r6
	fmr	f6,f31
	xor	r7,r7,r7
	fmr	f7,f31
	xor	r8,r8,r8
	fmr	f8,f31
	xor	r9,r9,r9
	fmr	f9,f31
	xor	r10,r10,r10
	fmr	f10,f31
	xor	r11,r11,r11
	fmr	f11,f31
	xor	r12,r12,r12
	fmr	f12,f31
	fmr	f13,f31
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0

.globl	.OPENSSL_atomic_add
.align	4
.OPENSSL_atomic_add:
Ladd:	lwarx	r5,0,r3
	add	r0,r4,r5
	stwcx.	r0,0,r3
	bne-	Ladd
	$SIGNX	r3,r0
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,2,0
	.long	0

.globl	.OPENSSL_rdtsc
.align	4
.OPENSSL_rdtsc:
	mftb	r3
	mftbu	r4
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0

.globl	.OPENSSL_cleanse
.align	4
.OPENSSL_cleanse:
	$CMPLI	r4,7
	li	r0,0
	bge	Lot
	$CMPLI	r4,0
	beqlr-
Little:	mtctr	r4
	stb	r0,0(r3)
	addi	r3,r3,1
	bdnz-	\$-8
	blr
Lot:	andi.	r5,r3,3
	beq	Laligned
	stb	r0,0(r3)
	subi	r4,r4,1
	addi	r3,r3,1
	b	Lot
Laligned:
	$SHRLI	r5,r4,2
	mtctr	r5
	stw	r0,0(r3)
	addi	r3,r3,4
	bdnz-	\$-8
	andi.	r4,r4,3
	bne	Little
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,2,0
	.long	0
___
{
my ($out,$cnt,$max)=("r3","r4","r5");
my ($tick,$lasttick)=("r6","r7");
my ($diff,$lastdiff)=("r8","r9");

$code.=<<___;
.globl	.OPENSSL_instrument_bus
.align	4
.OPENSSL_instrument_bus:
	mtctr	$cnt

	mftb	$lasttick		# collect 1st tick
	li	$diff,0

	dcbf	0,$out			# flush cache line
	lwarx	$tick,0,$out		# load and lock
	add	$tick,$tick,$diff
	stwcx.	$tick,0,$out
	stwx	$tick,0,$out

Loop:	mftb	$tick
	sub	$diff,$tick,$lasttick
	mr	$lasttick,$tick
	dcbf	0,$out			# flush cache line
	lwarx	$tick,0,$out		# load and lock
	add	$tick,$tick,$diff
	stwcx.	$tick,0,$out
	stwx	$tick,0,$out
	addi	$out,$out,4		# ++$out
	bdnz	Loop

	mr	r3,$cnt
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,2,0
	.long	0

.globl	.OPENSSL_instrument_bus2
.align	4
.OPENSSL_instrument_bus2:
	mr	r0,$cnt
	slwi	$cnt,$cnt,2

	mftb	$lasttick		# collect 1st tick
	li	$diff,0

	dcbf	0,$out			# flush cache line
	lwarx	$tick,0,$out		# load and lock
	add	$tick,$tick,$diff
	stwcx.	$tick,0,$out
	stwx	$tick,0,$out

	mftb	$tick			# collect 1st diff
	sub	$diff,$tick,$lasttick
	mr	$lasttick,$tick
	mr	$lastdiff,$diff
Loop2:
	dcbf	0,$out			# flush cache line
	lwarx	$tick,0,$out		# load and lock
	add	$tick,$tick,$diff
	stwcx.	$tick,0,$out
	stwx	$tick,0,$out

	addic.	$max,$max,-1
	beq	Ldone2

	mftb	$tick
	sub	$diff,$tick,$lasttick
	mr	$lasttick,$tick
	cmplw	7,$diff,$lastdiff
	mr	$lastdiff,$diff

	mfcr	$tick			# pull cr
	not	$tick,$tick		# flip bits
	rlwinm	$tick,$tick,1,29,29	# isolate flipped eq bit and scale

	sub.	$cnt,$cnt,$tick		# conditional --$cnt
	add	$out,$out,$tick		# conditional ++$out
	bne	Loop2

Ldone2:
	srwi	$cnt,$cnt,2
	sub	r3,r0,$cnt
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,3,0
	.long	0
___
}

$code =~ s/\`([^\`]*)\`/eval $1/gem;
print $code;
close STDOUT;
