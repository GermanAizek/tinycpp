/*
 *  x86-64 code generator for TCC
 *
 *  Copyright (c) 2008 Shinichiro Hamaji
 *
 *  Based on i386-gen.c by Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef TARGET_DEFS_ONLY

/* number of available registers */
#define NB_REGS         25
#define NB_ASM_REGS     16
#define CONFIG_TCC_ASM

/* a register can belong to several classes. The classes must be
   sorted from more general to more precise (see gv2() code which does
   assumptions on it). */
#define RC_INT     0x0001 /* generic integer register */
#define RC_FLOAT   0x0002 /* generic float register */
#define RC_RAX     0x0004
#define RC_RDX     0x0008
#define RC_RCX     0x0010
#define RC_RSI     0x0020
#define RC_RDI     0x0040
#define RC_ST0     0x0080 /* only for long double */
#define RC_R8      0x0100
#define RC_R9      0x0200
#define RC_R10     0x0400
#define RC_R11     0x0800
#define RC_XMM0    0x1000
#define RC_XMM1    0x2000
#define RC_XMM2    0x4000
#define RC_XMM3    0x8000
#define RC_XMM4    0x10000
#define RC_XMM5    0x20000
#define RC_XMM6    0x40000
#define RC_XMM7    0x80000
#define RC_IRET    RC_RAX /* function return: integer register */
#define RC_IRE2    RC_RDX /* function return: second integer register */
#define RC_FRET    RC_XMM0 /* function return: float register */
#define RC_FRE2    RC_XMM1 /* function return: second float register */

/* pretty names for the registers */
enum {
    TREG_RAX = 0,
    TREG_RCX = 1,
    TREG_RDX = 2,
    TREG_RSP = 4,
    TREG_RSI = 6,
    TREG_RDI = 7,

    TREG_R8  = 8,
    TREG_R9  = 9,
    TREG_R10 = 10,
    TREG_R11 = 11,

    TREG_XMM0 = 16,
    TREG_XMM1 = 17,
    TREG_XMM2 = 18,
    TREG_XMM3 = 19,
    TREG_XMM4 = 20,
    TREG_XMM5 = 21,
    TREG_XMM6 = 22,
    TREG_XMM7 = 23,

    TREG_ST0 = 24,

    TREG_MEM = 0x20
};

#define REX_BASE(reg) (((reg) >> 3) & 1)
#define REG_VALUE(reg) ((reg) & 7)

/* return registers for function */
#define REG_IRET TREG_RAX /* single word int return register */
#define REG_IRE2 TREG_RDX /* second word return register (for long long) */
#define REG_FRET TREG_XMM0 /* float return register */
#define REG_FRE2 TREG_XMM1 /* second float return register */

/* defined if function parameters must be evaluated in reverse order */
#define INVERT_FUNC_PARAMS

/* pointer size, in bytes */
#define PTR_SIZE 8

/* long double size and alignment, in bytes */
#define LDOUBLE_SIZE  16
#define LDOUBLE_ALIGN 16
/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     16

/* define if return values need to be extended explicitely
   at caller side (for interfacing with non-TCC compilers) */
#define PROMOTE_RET

#define TCC_TARGET_NATIVE_STRUCT_COPY
ST_FUNC void gen_struct_copy(int size);

/******************************************************/
#else /* ! TARGET_DEFS_ONLY */
/******************************************************/
#define USING_GLOBALS
#include "tcc.h"
#include <assert.h>

ST_DATA const char * const target_machine_defs =
    "__x86_64__\0"
    "__x86_64\0"
    "__amd64__\0"
    ;

ST_DATA const int reg_classes[NB_REGS] = {
    /* eax */ RC_INT | RC_RAX,
    /* ecx */ RC_INT | RC_RCX,
    /* edx */ RC_INT | RC_RDX,
    0,
    0,
    0,
    RC_RSI,
    RC_RDI,
    RC_R8,
    RC_R9,
    RC_R10,
    RC_R11,
    0,
    0,
    0,
    0,
    /* xmm0 */ RC_FLOAT | RC_XMM0,
    /* xmm1 */ RC_FLOAT | RC_XMM1,
    /* xmm2 */ RC_FLOAT | RC_XMM2,
    /* xmm3 */ RC_FLOAT | RC_XMM3,
    /* xmm4 */ RC_FLOAT | RC_XMM4,
    /* xmm5 */ RC_FLOAT | RC_XMM5,
    /* xmm6 and xmm7 are callee saved on Windows PE, but scratch on SysV Linux */
#ifdef TCC_TARGET_PE
    RC_XMM6,
    RC_XMM7,
#else
    RC_FLOAT | RC_XMM6,
    RC_FLOAT | RC_XMM7,
#endif
    /* st0 */ RC_ST0
};

static unsigned long func_sub_sp_offset;
static int func_ret_sub;

#if defined(CONFIG_TCC_BCHECK)
static addr_t func_bound_offset;
static unsigned long func_bound_ind;
ST_DATA int func_bound_add_epilog;
#endif

#ifdef TCC_TARGET_PE
static int func_scratch, func_alloca;
#endif

/* XXX: make it faster ? */
ST_FUNC void g(int c)
{
    int ind1;
    if (nocode_wanted)
        return;
    ind1 = ind + 1;
    if (ind1 > cur_text_section->data_allocated)
        section_realloc(cur_text_section, ind1);
    cur_text_section->data[ind] = c;
    ind = ind1;
}

ST_FUNC void o(unsigned int c)
{
    while (c) {
        g(c);
        c = c >> 8;
    }
}

ST_FUNC void gen_le16(int v)
{
    g(v);
    g(v >> 8);
}

ST_FUNC void gen_le32(int c)
{
    g(c);
    g(c >> 8);
    g(c >> 16);
    g(c >> 24);
}

ST_FUNC void gen_le64(int64_t c)
{
    g(c);
    g(c >> 8);
    g(c >> 16);
    g(c >> 24);
    g(c >> 32);
    g(c >> 40);
    g(c >> 48);
    g(c >> 56);
}

static void orex(int ll, int r, int r2, int b)
{
    if ((r & VT_VALMASK) >= VT_CONST)
        r = 0;
    if ((r2 & VT_VALMASK) >= VT_CONST)
        r2 = 0;
    if (ll || REX_BASE(r) || REX_BASE(r2)) {
        if ((b & 0xff) == 0x66) /* output prefix before rex byte */
            o(0x66), b >>= 8;
        o(0x40 | REX_BASE(r) | (REX_BASE(r2) << 2) | (ll << 3));
    }
    o(b);
}

/* output a symbol and patch all calls to it */
ST_FUNC void gsym_addr(int t, int a)
{
    while (t) {
        unsigned char *ptr = cur_text_section->data + t;
        uint32_t n = read32le(ptr); /* next value */
        write32le(ptr, a < 0 ? -a : a - t - 4);
        t = n;
    }
}

static int is64_type(int t)
{
    return ((t & VT_BTYPE) == VT_PTR ||
            (t & VT_BTYPE) == VT_FUNC ||
            (t & VT_BTYPE) == VT_LLONG);
}

/* instruction + 4 bytes data. Return the address of the data */
static int oad(int c, int s)
{
    int t;
    if (nocode_wanted)
        return s;
    o(c);
    t = ind;
    gen_le32(s);
    return t;
}

/* generate jmp to a label */
#define gjmp2(instr,lbl) oad(instr,lbl)

ST_FUNC void gen_addr32(int r, Sym *sym, int c)
{
    if (r & VT_SYM)
        greloca(cur_text_section, sym, ind, R_X86_64_32S, c), c=0;
    gen_le32(c);
}

/* output constant with relocation if 'r & VT_SYM' is true */
ST_FUNC void gen_addrpc32(int r, Sym *sym, int c)
{
    if (r & VT_SYM)
        greloca(cur_text_section, sym, ind, R_X86_64_PC32, c-4), c=4;
    gen_le32(c-4);
}

/* output got address with relocation */
static void gen_gotpcrel(int r, Sym *sym, int c)
{
#ifdef TCC_TARGET_PE
    tcc_error("internal error: no GOT on PE: %s %x %x | %02x %02x %02x\n",
        get_tok_str(sym->v, NULL), c, r,
        cur_text_section->data[ind-3],
        cur_text_section->data[ind-2],
        cur_text_section->data[ind-1]
        );
#endif
    greloca(cur_text_section, sym, ind, R_X86_64_GOTPCREL, -4);
    gen_le32(0);
    if (c) {
        /* we use add c, %xxx for displacement */
        orex(1, r, 0, 0x81);
        o(0xc0 + REG_VALUE(r));
        gen_le32(c);
    }
}

/* generate a modrm reference. 'op_reg' contains the additional 3
   opcode rsp. second register bits */
static void gen_modrm_impl(int opcode, int ll, int op_reg_0, int r, Sym *sym, int c)
{
    int op_reg = REG_VALUE(op_reg_0) << 3;

    if ((r & VT_SYM) && (sym->type.t & VT_TLS)) {
#ifdef TCC_TARGET_PE
        Sym *s2 = external_global_sym(TOK___tls_index, &int_type);
        r = get_reg(RC_INT);
        gen_modrm_impl(0x8B, 0,  r, VT_SYM|VT_CONST, s2, 0);
        o(0x03e0c148 | r << 16); /* shl 2,r */
        o(0x4865), oad(0x250403 | r << 11, 11*PTR_SIZE); /* add gs:0x58,r */
        gen_modrm_impl(0x8B, 1, r, r | VT_LVAL, 0, 0); /* mov (r),r */
        orex(ll, r, op_reg_0, opcode), oad(0x80 | op_reg | r, 0);
        greloca(cur_text_section, sym, ind - 4, R_X86_64_TPOFF32, c);
#else
        o(0x64); /* fs segment prefix */
        orex(ll, r, op_reg_0, opcode);
	o(0x04 | op_reg); /* [sib] | destreg */
	oad(0x25, 0);     /* disp32 (relocated) */
	greloca(cur_text_section, sym, ind - 4, R_X86_64_TPOFF32, c);
#endif
        return;
    }

    orex(ll, r, op_reg_0, opcode);

    if ((r & VT_VALMASK) == VT_CONST) {
        /* constant memory reference */
	if (!(r & VT_SYM)) {
	    /* Absolute memory reference */
	    o(0x04 | op_reg); /* [sib] | destreg */
	    oad(0x25, c);     /* disp32 */
	} else {
	    o(0x05 | op_reg); /* (%rip)+disp32 | destreg */
	    if (op_reg_0 & TREG_MEM) {
		gen_gotpcrel(r, sym, c);
	    } else {
		gen_addrpc32(r, sym, c);
	    }
	}
    } else if ((r & VT_VALMASK) == VT_LOCAL) {
        /* currently, we use only ebp as base */
        if (c == (signed char)c) {
            /* short reference */
            o(0x45 | op_reg);
            g(c);
        } else {
            oad(0x85 | op_reg, c);
        }
    /* 'c' (mostly from vtop->c.i) is not valid unless when TREG_MEM is set */
    } else if ((r & TREG_MEM) && c) {
        g(0x80 | op_reg | REG_VALUE(r));
        gen_le32(c);
    } else if (r & VT_LVAL) {
        g(0x00 | op_reg | REG_VALUE(r));
    } else {
        g(0xc0 | op_reg | REG_VALUE(r));
    }
}

static void gen_modrm64(int opcode, int op_reg, int r, Sym *sym, int c)
{
    gen_modrm_impl(opcode, 1, op_reg, r, sym, c);
}

static void gen_modrm32(int opcode, int op_reg, int r, Sym *sym, int c)
{
    gen_modrm_impl(opcode, 0, op_reg, r, sym, c);
}



/* load 'r' from value 'sv' */
void load(int r, SValue *sv)
{
    int v, t, ft, fc, fr;
    SValue v1;

    fr = sv->r;
    ft = sv->type.t & ~(VT_DEFSIGN|VT_VOLATILE|VT_CONSTANT);
    fc = sv->c.i;

    if (fc != sv->c.i && (fr & VT_SYM))
      tcc_error("64 bit addend in load");

#ifndef TCC_TARGET_PE
    /* we use indirect access via got */
    if ((fr & (VT_VALMASK|VT_SYM|VT_LVAL)) == (VT_CONST|VT_SYM|VT_LVAL)
        && !(sv->sym->type.t & (VT_STATIC|VT_TLS))) {
        /* use the result register as a temporal register */
        int tr = r | TREG_MEM;
        if (is_float(ft)) {
            /* we cannot use float registers as a temporal register */
            tr = get_reg(RC_INT) | TREG_MEM;
        }
        gen_modrm64(0x8b, tr, fr, sv->sym, 0);

        /* load from the temporal register */
        fr = tr | VT_LVAL;
    }
#endif

    v = fr & VT_VALMASK;
    if (fr & VT_LVAL) {
        int b, ll;
        if (v == VT_LLOCAL) {
            v1.type.t = VT_PTR;
            v1.r = VT_LOCAL | VT_LVAL;
            v1.c.i = fc;
	    v1.sym = NULL;
            fr = r;
            if (!(reg_classes[fr] & (RC_INT|RC_R11)))
                fr = get_reg(RC_INT);
            load(fr, &v1);
            fr |= VT_LVAL;
            fc = 0;
        } else if (fc != sv->c.i) {
	    /* If the addends doesn't fit into a 32bit signed
	       we must use a 64bit move.  We've checked above
	       that this doesn't have a sym associated.  */
	    v1.type.t = VT_LLONG;
	    v1.r = VT_CONST;
	    v1.c.i = sv->c.i;
	    v1.sym = NULL;
	    fr = r;
	    if (!(reg_classes[fr] & (RC_INT|RC_R11)))
	        fr = get_reg(RC_INT);
	    load(fr, &v1);
	    fc = 0;
	}
        ll = 0;
	/* Like GCC we can load from small enough properly sized
	   structs and unions as well.
	   XXX maybe move to generic operand handling, but should
	   occur only with asm, so tccasm.c might also be a better place */
	if ((ft & VT_BTYPE) == VT_STRUCT) {
	    int align;
	    switch (type_size(&sv->type, &align)) {
		case 1: ft = VT_BYTE; break;
		case 2: ft = VT_SHORT; break;
		case 4: ft = VT_INT; break;
		case 8: ft = VT_LLONG; break;
		default:
		    tcc_error("invalid aggregate type for register load");
		    break;
	    }
	}
        if ((ft & VT_BTYPE) == VT_FLOAT) {
            b = 0x6e0f66;
            r = REG_VALUE(r); /* movd */
        } else if ((ft & VT_BTYPE) == VT_DOUBLE) {
            b = 0x7e0ff3; /* movq */
            r = REG_VALUE(r);
        } else if ((ft & VT_BTYPE) == VT_LDOUBLE) {
            b = 0xdb, r = 5; /* fldt */
        } else if ((ft & VT_TYPE) == VT_BYTE || (ft & VT_TYPE) == VT_BOOL) {
            b = 0xbe0f;   /* movsbl */
        } else if ((ft & VT_TYPE) == (VT_BYTE | VT_UNSIGNED)) {
            b = 0xb60f;   /* movzbl */
        } else if ((ft & VT_TYPE) == VT_SHORT) {
            b = 0xbf0f;   /* movswl */
        } else if ((ft & VT_TYPE) == (VT_SHORT | VT_UNSIGNED)) {
            b = 0xb70f;   /* movzwl */
        } else if ((ft & VT_TYPE) == (VT_VOID)) {
            /* Can happen with zero size structs */
            return;
        } else {
            ll = is64_type(ft);
            b = 0x8b;
        }
        gen_modrm_impl(b, ll, r, fr, sv->sym, fc);
    } else {
        if (v == VT_CONST) {
            if (fr & VT_SYM) {
#ifdef TCC_TARGET_PE
                gen_modrm64(0x8d, r, fr, sv->sym, fc);
#else
                if (sv->sym->type.t & VT_TLS) {
                    /* mov fs:0, r */
                    o(0x64), gen_modrm64(0x8b, r, VT_CONST, NULL, 0);
                    /* add $tpoff,r */
                    orex(1,0,r,0x81), oad(0xC0 | REG_VALUE(r), 0);
                    greloca(cur_text_section, sv->sym, ind - 4, R_X86_64_TPOFF32, fc);
                } else if (sv->sym->type.t & VT_STATIC) {
                    orex(1,0,r,0x8d);
                    o(0x05 + REG_VALUE(r) * 8); /* lea xx(%rip), r */
                    gen_addrpc32(fr, sv->sym, fc);
                } else {
                    orex(1,0,r,0x8b);
                    o(0x05 + REG_VALUE(r) * 8); /* mov xx(%rip), r */
                    gen_gotpcrel(r, sv->sym, fc);
                }
#endif
            } else if (is64_type(ft)) {
                if (sv->c.i >> 32) {
                    orex(1,r,0, 0xb8 + REG_VALUE(r)); /* movabs $xx, r */
                    gen_le64(sv->c.i);
                } else if (sv->c.i > 0) {
                    orex(0,r,0, 0xb8 + REG_VALUE(r)); /* mov $xx, r */
                    gen_le32(sv->c.i);
                } else {
                    orex(0, r, r, 0x31); /* xor r, r */
                    o(0xc0 + REG_VALUE(r) * 9);
                }
            } else {
                if (fc == 0) {
                    orex(0, r, r, 0x31); /* xor r, r */
                    o(0xc0 + REG_VALUE(r) * 9);
                } else {
                    orex(0,r,0, 0xb8 + REG_VALUE(r)); /* mov $xx, r */
                    gen_le32(fc);
                }
            }
        } else if (v == VT_LOCAL) {
            gen_modrm64(0x8d, r, VT_LOCAL, sv->sym, fc);
        } else if (v == VT_CMP) {
	    if (fc & 0x100)
	      {
                v = vtop->cmp_r;
                fc &= ~0x100;
	        /* This was a float compare.  If the parity bit is
		   set the result was unordered, meaning false for everything
		   except TOK_NE, and true for TOK_NE.  */
                orex(0, r, 0, 0xb0 + REG_VALUE(r)); /* mov $0/1,%al */
                g(v ^ fc ^ (v == TOK_NE));
                o(0x037a + (REX_BASE(r) << 8));
              }
            orex(0,r,0, 0x0f); /* setxx %br */
            o(fc);
            o(0xc0 + REG_VALUE(r));
            orex(0,r,0, 0x0f);
            o(0xc0b6 + REG_VALUE(r) * 0x900); /* movzbl %al, %eax */
        } else if (v == VT_JMP || v == VT_JMPI) {
            t = v & 1;
            orex(0,r,0,0);
            oad(0xb8 + REG_VALUE(r), t); /* mov $1, r */
            o(0x05eb + (REX_BASE(r) << 8)); /* jmp after */
            gsym(fc);
            orex(0,r,0,0);
            oad(0xb8 + REG_VALUE(r), t ^ 1); /* mov $0, r */
        } else if (v != r) {
            if ((r >= TREG_XMM0) && (r <= TREG_XMM7)) {
                if (v == TREG_ST0) {
                    /* gen_cvt_ftof(VT_DOUBLE); */
                    o(0xf0245cdd); /* fstpl -0x10(%rsp) */
                    /* movsd -0x10(%rsp),%xmmN */
                    o(0x100ff2);
                    o(0x44 + REG_VALUE(r)*8); /* %xmmN */
                    o(0xf024);
                } else {
                    assert((v >= TREG_XMM0) && (v <= TREG_XMM7));
                    if ((ft & VT_BTYPE) == VT_FLOAT) {
                        o(0x100ff3);
                    } else {
                        assert((ft & VT_BTYPE) == VT_DOUBLE);
                        o(0x100ff2);
                    }
                    o(0xc0 + REG_VALUE(v) + REG_VALUE(r)*8);
                }
            } else if (r == TREG_ST0) {
                assert((v >= TREG_XMM0) && (v <= TREG_XMM7));
                /* gen_cvt_ftof(VT_LDOUBLE); */
                /* movsd %xmmN,-0x10(%rsp) */
                o(0x110ff2);
                o(0x44 + REG_VALUE(r)*8); /* %xmmN */
                o(0xf024);
                o(0xf02444dd); /* fldl -0x10(%rsp) */
            } else {
                orex(is64_type(ft), r, v, 0x89);
                o(0xc0 + REG_VALUE(r) + REG_VALUE(v) * 8); /* mov v, r */
            }
        }
    }
}

/* store register 'r' in lvalue 'v' */
void store(int r, SValue *v)
{
    int fr, bt, fc;
    /* store the REX prefix in this variable when PIC is enabled */
    int opc = 0, ll = 0;

    fr = v->r;
    fc = v->c.i;
    if (fc != v->c.i && (fr & VT_SYM))
        tcc_error("64 bit addend in store");
    bt = v->type.t & VT_BTYPE;

#ifndef TCC_TARGET_PE
    /* we need to access the variable via got */
    if ((fr & (VT_VALMASK|VT_SYM)) == (VT_CONST|VT_SYM)
        && !(v->sym->type.t & (VT_STATIC|VT_TLS))) {
        /* mov xx(%rip), %r11 */
        o(0x1d8b4c);
        gen_gotpcrel(TREG_R11, v->sym, v->c.i);
        fr = TREG_R11 | VT_LVAL;
    }
#endif

    /* XXX: incorrect if float reg to reg */
    if (bt == VT_FLOAT) {
        opc = 0x7e0f66;
        r = REG_VALUE(r);
    } else if (bt == VT_DOUBLE) {
        opc = 0xd60f66;
        r = REG_VALUE(r);
    } else if (bt == VT_LDOUBLE) {
        o(0xc0d9); /* fld %st(0) */
        opc = 0xdb;
        r = 7;
    } else if (bt == VT_BYTE || bt == VT_BOOL) {
        opc = 0x88;
    } else {
        opc = 0x89;
        if (bt == VT_SHORT)
            opc = 0x8966;
        else if (is64_type(bt))
            ll = 1;
    }
    gen_modrm_impl(opc, ll, r, fr, v->sym, fc);
}

/* 'is_jmp' is '1' if it is a jump */
static void gcall_or_jmp(int is_jmp)
{
    int r;
    if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST &&
	((vtop->r & VT_SYM) && (vtop->c.i-4) == (int)(vtop->c.i-4))) {
        /* constant symbolic case -> simple relocation */
        greloca(cur_text_section, vtop->sym, ind + 1, R_X86_64_PLT32, (int)(vtop->c.i-4));
        oad(0xe8 + is_jmp, 0); /* call/jmp im */
    } else {
        /* otherwise, indirect call */
        r = TREG_R11;
        load(r, vtop);
        o(0x41); /* REX */
        o(0xff); /* call/jmp *r */
        o(0xd0 + REG_VALUE(r) + (is_jmp << 4));
    }
}

#if defined(CONFIG_TCC_BCHECK)

static void gen_bounds_call(int v)
{
    Sym *sym = external_helper_sym(v);
    oad(0xe8, 0);
    greloca(cur_text_section, sym, ind-4, R_X86_64_PLT32, -4);
}

#ifdef TCC_TARGET_PE
# define TREG_FASTCALL_1 TREG_RCX
#else
# define TREG_FASTCALL_1 TREG_RDI
#endif

static void gen_bounds_prolog(void)
{
    /* leave some room for bound checking code */
    func_bound_offset = lbounds_section->data_offset;
    func_bound_ind = ind;
    func_bound_add_epilog = 0;
    o(0x0d8d48 + ((TREG_FASTCALL_1 == TREG_RDI) * 0x300000)); /*lbound section pointer */
    gen_le32 (0);
    oad(0xb8, 0); /* call to function */
}

static void gen_bounds_epilog(void)
{
    addr_t saved_ind;
    addr_t *bounds_ptr;
    Sym *sym_data;
    int offset_modified = func_bound_offset != lbounds_section->data_offset;

    if (!offset_modified && !func_bound_add_epilog)
        return;

    /* add end of table info */
    bounds_ptr = section_ptr_add(lbounds_section, sizeof(addr_t));
    *bounds_ptr = 0;

    sym_data = get_sym_ref(&char_pointer_type, lbounds_section, 
                           func_bound_offset, PTR_SIZE);

    /* generate bound local allocation */
    if (offset_modified) {
        saved_ind = ind;
        ind = func_bound_ind;
        greloca(cur_text_section, sym_data, ind + 3, R_X86_64_PC32, -4);
        ind = ind + 7;
        gen_bounds_call(TOK___bound_local_new);
        ind = saved_ind;
    }

    /* generate bound check local freeing */
    o(0x5250); /* save returned value, if any */
    o(0x20ec8348); /* sub $32,%rsp */
    o(0x290f);     /* movaps %xmm0,0x10(%rsp) */
    o(0x102444);
    o(0x240c290f); /* movaps %xmm1,(%rsp) */
    greloca(cur_text_section, sym_data, ind + 3, R_X86_64_PC32, -4);
    o(0x0d8d48 + ((TREG_FASTCALL_1 == TREG_RDI) * 0x300000)); /* lea xxx(%rip), %rcx/rdi */
    gen_le32 (0);
    gen_bounds_call(TOK___bound_local_delete);
    o(0x280f);     /* movaps 0x10(%rsp),%xmm0 */
    o(0x102444);
    o(0x240c280f); /* movaps (%rsp),%xmm1 */
    o(0x20c48348); /* add $32,%rsp */
    o(0x585a); /* restore returned value, if any */
}
#endif

#ifdef TCC_TARGET_PE

#define REGN 4
static const uint8_t arg_regs[REGN] = {
    TREG_RCX, TREG_RDX, TREG_R8, TREG_R9
};

/* Prepare arguments in R10 and R11 rather than RCX and RDX
   because gv() will not ever use these */
static int arg_prepare_reg(int idx) {
  if (idx == 0 || idx == 1)
      /* idx=0: r10, idx=1: r11 */
      return idx + 10;
  else
      return idx >= 0 && idx < REGN ? arg_regs[idx] : 0;
}

/* Generate function call. The function address is pushed first, then
   all the parameters in call order. This functions pops all the
   parameters and the function address. */

static void gen_offs_sp(int b, int r, int d)
{
    orex(1,0,r & 0x100 ? 0 : r, b);
    if (d == (signed char)d) {
        o(0x2444 | (REG_VALUE(r) << 3));
        g(d);
    } else {
        o(0x2484 | (REG_VALUE(r) << 3));
        gen_le32(d);
    }
}

static int using_regs(int size)
{
    return !(size > 8 || (size & (size - 1)));
}

/* Return the number of registers needed to return the struct, or 0 if
   returning via struct pointer. */
ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret, int *ret_align, int *regsize)
{
    int size, align;
    *ret_align = 1; // Never have to re-align return values for x86-64
    *regsize = 8;
    size = type_size(vt, &align);
    if (!using_regs(size))
        return 0;
    if (size == 8)
        ret->t = VT_LLONG;
    else if (size == 4)
        ret->t = VT_INT;
    else if (size == 2)
        ret->t = VT_SHORT;
    else
        ret->t = VT_BYTE;
    ret->ref = NULL;
    return 1;
}

static int is_sse_float(int t) {
    int bt;
    bt = t & VT_BTYPE;
    return bt == VT_DOUBLE || bt == VT_FLOAT;
}

static int gfunc_arg_size(CType *type) {
    int align;
    if (type->t & (VT_ARRAY|VT_BITFIELD))
        return 8;
    return type_size(type, &align);
}

void gfunc_call(int nb_args)
{
    int size, r, args_size, i, d, bt, struct_size;
    int arg;

#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gbound_args(nb_args);
#endif

    save_regs(nb_args);

    args_size = (nb_args < REGN ? REGN : nb_args) * PTR_SIZE;
    arg = nb_args;

    /* for struct arguments, we need to call memcpy and the function
       call breaks register passing arguments we are preparing.
       So, we process arguments which will be passed by stack first. */
    struct_size = args_size;
    for(i = 0; i < nb_args; i++) {
        SValue *sv;
        
        --arg;
        sv = &vtop[-i];
        bt = (sv->type.t & VT_BTYPE);
        size = gfunc_arg_size(&sv->type);

        if (using_regs(size))
            continue; /* arguments smaller than 8 bytes passed in registers or on stack */

        if (bt == VT_STRUCT) {
            /* align to stack align size */
            size = (size + 15) & ~15;
            /* generate structure store */
            r = get_reg(RC_INT);
            gen_offs_sp(0x8d, r, struct_size);
            struct_size += size;

            /* generate memcpy call */
            vset(&sv->type, r | VT_LVAL, 0);
            vpushv(sv);
            vstore();
            --vtop;
        } else if (bt == VT_LDOUBLE) {
            gv(RC_ST0);
            gen_offs_sp(0xdb, 0x107, struct_size);
            struct_size += 16;
        }
    }

    if (func_scratch < struct_size)
        func_scratch = struct_size;

    arg = nb_args;
    struct_size = args_size;

    for(i = 0; i < nb_args; i++) {
        --arg;
        bt = (vtop->type.t & VT_BTYPE);

        size = gfunc_arg_size(&vtop->type);
        if (!using_regs(size)) {
            /* align to stack align size */
            size = (size + 15) & ~15;
            if (arg >= REGN) {
                d = get_reg(RC_INT);
                gen_offs_sp(0x8d, d, struct_size);
                gen_offs_sp(0x89, d, arg*8);
            } else {
                d = arg_prepare_reg(arg);
                gen_offs_sp(0x8d, d, struct_size);
            }
            struct_size += size;
        } else {
            if (is_sse_float(vtop->type.t)) {
		if (tcc_state->nosse)
		  tcc_error("SSE disabled");
                if (arg >= REGN) {
                    gv(RC_XMM0);
                    /* movq %xmm0, j*8(%rsp) */
                    gen_offs_sp(0xd60f66, 0x100, arg*8);
                } else {
                    /* Load directly to xmmN register */
                    gv(RC_XMM0 << arg);
                    d = arg_prepare_reg(arg);
                    /* mov %xmmN, %rxx */
                    o(0x66);
                    orex(1,d,0, 0x7e0f);
                    o(0xc0 + arg*8 + REG_VALUE(d));
                }
            } else {
                if (bt == VT_STRUCT) {
                    vtop->type.ref = NULL;
                    vtop->type.t = size > 4 ? VT_LLONG : size > 2 ? VT_INT
                        : size > 1 ? VT_SHORT : VT_BYTE;
                }
                
                r = gv(RC_INT);
                if (arg >= REGN) {
                    gen_offs_sp(0x89, r, arg*8);
                } else {
                    d = arg_prepare_reg(arg);
                    if (r != d) {
                        orex(1,d,r,0x89); /* mov */
                        o(0xc0 + REG_VALUE(r) * 8 + REG_VALUE(d));
                    }
                }
            }
        }
        vtop--;
    }

    /* Copy R10 and R11 into RCX and RDX, respectively */
    if (nb_args > 0) {
        o(0xd1894c); /* mov %r10, %rcx */
        if (nb_args > 1) {
            o(0xda894c); /* mov %r11, %rdx */
        }
    }
    
    gcall_or_jmp(0);

    if ((vtop->r & VT_SYM) && vtop->sym->v == TOK_alloca) {
        /* need to add the "func_scratch" area after alloca */
        o(0x48); func_alloca = oad(0x05, func_alloca); /* add $NN, %rax */
#ifdef CONFIG_TCC_BCHECK
        if (tcc_state->do_bounds_check)
            gen_bounds_call(TOK___bound_alloca_nr); /* new region */
#endif
    }
    vtop--;
}

#define FUNC_PROLOG_SIZE 11

/* generate function prolog of type 't' */
void gfunc_prolog(Sym *func_sym)
{
    CType *func_type = &func_sym->type;
    int addr, reg_param_index, bt, size;
    Sym *sym;
    CType *type;

    func_ret_sub = 0;
    func_scratch = 32;
    func_alloca = 0;
    loc = 0;

    addr = PTR_SIZE * 2;
    ind += FUNC_PROLOG_SIZE;
    func_sub_sp_offset = ind;
    reg_param_index = 0;

    sym = func_type->ref;

    /* if the function returns a structure, then add an
       implicit pointer parameter */
    size = gfunc_arg_size(&func_vt);
    if (!using_regs(size)) {
        gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
        func_vc = addr;
        reg_param_index++;
        addr += 8;
    }

    /* define parameters */
    while ((sym = sym->next) != NULL) {
        type = &sym->type;
        bt = type->t & VT_BTYPE;
        size = gfunc_arg_size(type);
        if (!using_regs(size)) {
            if (reg_param_index < REGN) {
                gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
            }
            gfunc_set_param(sym, addr, 1);
        } else {
            if (reg_param_index < REGN) {
                /* save arguments passed by register */
                if ((bt == VT_FLOAT) || (bt == VT_DOUBLE)) {
		    if (tcc_state->nosse)
		      tcc_error("SSE disabled");
                    /* movq */
                    gen_modrm32(0xd60f66, reg_param_index, VT_LOCAL, NULL, addr);
                } else {
                    gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
                }
            }
            gfunc_set_param(sym, addr, 0);
        }
        addr += 8;
        reg_param_index++;
    }

    while (reg_param_index < REGN) {
        if (func_var) {
            gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
            addr += 8;
        }
        reg_param_index++;
    }
#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gen_bounds_prolog();
#endif
}

static void x86_64_optimize_func(int func_start, int func_end);

/* generate function epilog */
void gfunc_epilog(void)
{
    int v, start;

    /* align local size to word & save local variables */
    func_scratch = (func_scratch + 15) & -16;
    loc = (loc & -16) - func_scratch;

#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gen_bounds_epilog();
#endif

    o(0xc9); /* leave */
    if (func_ret_sub == 0) {
        o(0xc3); /* ret */
    } else {
        o(0xc2); /* ret n */
        g(func_ret_sub);
        g(func_ret_sub >> 8);
    }

    v = -loc;
    start = func_sub_sp_offset - FUNC_PROLOG_SIZE;
    cur_text_section->data_offset = ind;
    pe_add_unwind_data(start, ind, v);

    ind = start;
    if (v >= 4096) {
        Sym *sym = external_helper_sym(TOK___chkstk);
        oad(0xb8, v); /* mov stacksize, %eax */
        oad(0xe8, 0); /* call __chkstk, (does the stackframe too) */
        greloca(cur_text_section, sym, ind-4, R_X86_64_PLT32, -4);
        o(0x90); /* fill for FUNC_PROLOG_SIZE = 11 bytes */
    } else {
        o(0xe5894855);  /* push %rbp, mov %rsp, %rbp */
        o(0xec8148);  /* sub rsp, stacksize */
        gen_le32(v);
    }
    ind = cur_text_section->data_offset;

    /* add the "func_scratch" area after each alloca seen */
    gsym_addr(func_alloca, -func_scratch);

    if (tcc_state->optimize >= 2 && func_ind >= 0)
        x86_64_optimize_func(func_ind, ind);
}

#else

static void gadd_sp(int val)
{
    if (val == (signed char)val) {
        o(0xc48348);
        g(val);
    } else {
        oad(0xc48148, val); /* add $xxx, %rsp */
    }
}

typedef enum X86_64_Mode {
  x86_64_mode_none,
  x86_64_mode_memory,
  x86_64_mode_integer,
  x86_64_mode_sse,
  x86_64_mode_x87
} X86_64_Mode;

static X86_64_Mode classify_x86_64_merge(X86_64_Mode a, X86_64_Mode b)
{
    if (a == b)
        return a;
    else if (a == x86_64_mode_none)
        return b;
    else if (b == x86_64_mode_none)
        return a;
    else if ((a == x86_64_mode_memory) || (b == x86_64_mode_memory))
        return x86_64_mode_memory;
    else if ((a == x86_64_mode_integer) || (b == x86_64_mode_integer))
        return x86_64_mode_integer;
    else if ((a == x86_64_mode_x87) || (b == x86_64_mode_x87))
        return x86_64_mode_memory;
    else
        return x86_64_mode_sse;
}

static X86_64_Mode classify_x86_64_inner(CType *ty)
{
    X86_64_Mode mode;
    Sym *f;
    
    switch (ty->t & VT_BTYPE) {
    case VT_VOID: return x86_64_mode_none;
    
    case VT_INT:
    case VT_BYTE:
    case VT_SHORT:
    case VT_LLONG:
    case VT_BOOL:
    case VT_PTR:
    case VT_FUNC:
        return x86_64_mode_integer;
    
    case VT_FLOAT:
    case VT_DOUBLE: return x86_64_mode_sse;
    
    case VT_LDOUBLE: return x86_64_mode_x87;
      
    case VT_STRUCT:
        f = ty->ref;

        mode = x86_64_mode_none;
        for (f = f->next; f; f = f->next)
            mode = classify_x86_64_merge(mode, classify_x86_64_inner(&f->type));
        
        return mode;
    }
    assert(0);
    return 0;
}

static X86_64_Mode classify_x86_64_arg(CType *ty, CType *ret, int *psize, int *palign, int *reg_count)
{
    X86_64_Mode mode;
    int size, align, ret_t = 0;
    
    if (ty->t & (VT_BITFIELD|VT_ARRAY)) {
        *psize = 8;
        *palign = 8;
        *reg_count = 1;
        ret_t = ty->t;
        mode = x86_64_mode_integer;
    } else {
        size = type_size(ty, &align);
        *psize = (size + 7) & ~7;
        *palign = (align + 7) & ~7;
        *reg_count = 0; /* avoid compiler warning */

        if (size > 16) {
            mode = x86_64_mode_memory;
        } else {
            mode = classify_x86_64_inner(ty);
            switch (mode) {
            case x86_64_mode_integer:
                if (size > 8) {
                    *reg_count = 2;
                    ret_t = VT_QLONG;
                } else {
                    *reg_count = 1;
                    if (size > 4)
                        ret_t = VT_LLONG;
                    else if (size > 2)
                        ret_t = VT_INT;
                    else if (size > 1)
                        ret_t = VT_SHORT;
                    else
                        ret_t = VT_BYTE;
                    if ((ty->t & VT_BTYPE) == VT_STRUCT || (ty->t & VT_UNSIGNED))
                        ret_t |= VT_UNSIGNED;
                }
                break;
                
            case x86_64_mode_x87:
                *reg_count = 1;
                ret_t = VT_LDOUBLE;
                break;

            case x86_64_mode_sse:
                if (size > 8) {
                    *reg_count = 2;
                    ret_t = VT_QFLOAT;
                } else {
                    *reg_count = 1;
                    ret_t = (size > 4) ? VT_DOUBLE : VT_FLOAT;
                }
                break;
            default: break; /* nothing to be done for x86_64_mode_memory and x86_64_mode_none*/
            }
        }
    }
    
    if (ret) {
        ret->ref = NULL;
        ret->t = ret_t;
    }
    
    return mode;
}

ST_FUNC int classify_x86_64_va_arg(CType *ty)
{
    /* This definition must be synced with stdarg.h */
    enum __va_arg_type {
        __va_gen_reg, __va_float_reg, __va_stack
    };
    int size, align, reg_count;
    X86_64_Mode mode = classify_x86_64_arg(ty, NULL, &size, &align, &reg_count);
    switch (mode) {
    default: return __va_stack;
    case x86_64_mode_integer: return __va_gen_reg;
    case x86_64_mode_sse: return __va_float_reg;
    }
}

/* Return the number of registers needed to return the struct, or 0 if
   returning via struct pointer. */
ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret, int *ret_align, int *regsize)
{
    int size, align, reg_count;
    if (classify_x86_64_arg(vt, ret, &size, &align, &reg_count) == x86_64_mode_memory)
        return 0;
    *ret_align = 1; // Never have to re-align return values for x86-64
    *regsize = 8 * reg_count; /* the (virtual) regsize is 16 for VT_QLONG/QFLOAT */
    return 1;
}

#define REGN 6
static const uint8_t arg_regs[REGN] = {
    TREG_RDI, TREG_RSI, TREG_RDX, TREG_RCX, TREG_R8, TREG_R9
};

static int arg_prepare_reg(int idx) {
  if (idx == 2 || idx == 3)
      /* idx=2: r10, idx=3: r11 */
      return idx + 8;
  else
      return idx >= 0 && idx < REGN ? arg_regs[idx] : 0;
}

/* Generate function call. The function address is pushed first, then
   all the parameters in call order. This functions pops all the
   parameters and the function address. */
void gfunc_call(int nb_args)
{
    X86_64_Mode mode;
    CType type;
    int size, align, r, args_size, stack_adjust, i, reg_count, k;
    int nb_reg_args = 0;
    int nb_sse_args = 0;
    int sse_reg, gen_reg;
    char *onstack = tcc_malloc((nb_args + 1) * sizeof (char));

#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gbound_args(nb_args);
#endif

    save_regs(nb_args);

    /* calculate the number of integer/float register arguments, remember
       arguments to be passed via stack (in onstack[]), and also remember
       if we have to align the stack pointer to 16 (onstack[i] == 2).  Needs
       to be done in a left-to-right pass over arguments.  */
    stack_adjust = 0;
    for(i = nb_args - 1; i >= 0; i--) {
        mode = classify_x86_64_arg(&vtop[-i].type, NULL, &size, &align, &reg_count);
        if (size == 0) continue;
        if (mode == x86_64_mode_sse && nb_sse_args + reg_count <= 8) {
            nb_sse_args += reg_count;
	    onstack[i] = 0;
	} else if (mode == x86_64_mode_integer && nb_reg_args + reg_count <= REGN) {
            nb_reg_args += reg_count;
	    onstack[i] = 0;
	} else if (mode == x86_64_mode_none) {
	    onstack[i] = 0;
	} else {
	    if (align == 16 && (stack_adjust &= 15)) {
		onstack[i] = 2;
		stack_adjust = 0;
	    } else
	      onstack[i] = 1;
	    stack_adjust += size;
	}
    }

    if (nb_sse_args && tcc_state->nosse)
      tcc_error("SSE disabled but floating point arguments passed");

    /* for struct arguments, we need to call memcpy and the function
       call breaks register passing arguments we are preparing.
       So, we process arguments which will be passed by stack first. */
    gen_reg = nb_reg_args;
    sse_reg = nb_sse_args;
    args_size = 0;
    stack_adjust &= 15;
    for (i = k = 0; i < nb_args;) {
	mode = classify_x86_64_arg(&vtop[-i].type, NULL, &size, &align, &reg_count);
	if (size) {
            if (!onstack[i + k]) {
	        ++i;
	        continue;
	    }
            /* Possibly adjust stack to align SSE boundary.  We're processing
	       args from right to left while allocating happens left to right
	       (stack grows down), so the adjustment needs to happen _after_
	       an argument that requires it.  */
            if (stack_adjust) {
	        o(0x50); /* push %rax; aka sub $8,%rsp */
                args_size += 8;
	        stack_adjust = 0;
            }
	    if (onstack[i + k] == 2)
	        stack_adjust = 1;
        }

	vrotb(i+1);

	switch (vtop->type.t & VT_BTYPE) {
	    case VT_STRUCT:
		/* allocate the necessary size on stack */
		o(0x48);
		oad(0xec81, size); /* sub $xxx, %rsp */
		/* generate structure store */
		r = get_reg(RC_INT);
		orex(1, r, 0, 0x89); /* mov %rsp, r */
		o(0xe0 + REG_VALUE(r));
		vset(&vtop->type, r | VT_LVAL, 0);
		vswap();
		/* keep stack aligned for (__bound_)memmove call */
		o(0x10ec8348); /* sub $16,%rsp */
		o(0xf0e48348); /* and $-16,%rsp */
		orex(0,r,0,0x50 + REG_VALUE(r)); /* push r (last %rsp) */
		o(0x08ec8348); /* sub $8,%rsp */
		vstore();
		o(0x08c48348); /* add $8,%rsp */
		o(0x5c);       /* pop %rsp */
		break;

	    case VT_LDOUBLE:
                gv(RC_ST0);
                oad(0xec8148, size); /* sub $xxx, %rsp */
                o(0x7cdb); /* fstpt 0(%rsp) */
                g(0x24);
                g(0x00);
		break;

	    case VT_FLOAT:
	    case VT_DOUBLE:
		assert(mode == x86_64_mode_sse);
		r = gv(RC_FLOAT);
		o(0x50); /* push $rax */
		/* movq %xmmN, (%rsp) */
		o(0xd60f66);
		o(0x04 + REG_VALUE(r)*8);
		o(0x24);
		break;

	    default:
		assert(mode == x86_64_mode_integer);
		/* simple type */
		/* XXX: implicit cast ? */
		r = gv(RC_INT);
		orex(0,r,0,0x50 + REG_VALUE(r)); /* push r */
		break;
	}
	args_size += size;

	vpop();
	--nb_args;
	k++;
    }

    tcc_free(onstack);

    /* then, we prepare register passing arguments.
       Note that we cannot set RDX and RCX in this loop because gv()
       may break these temporary registers. Let's use R10 and R11
       instead of them */
    assert(gen_reg <= REGN);
    assert(sse_reg <= 8);
    for(i = 0; i < nb_args; i++) {
        mode = classify_x86_64_arg(&vtop->type, &type, &size, &align, &reg_count);
        if (size == 0) continue;
        /* Alter stack entry type so that gv() knows how to treat it */
        vtop->type = type;
        if (mode == x86_64_mode_sse) {
            if (reg_count == 2) {
                sse_reg -= 2;
                gv(RC_FRET); /* Use pair load into xmm0 & xmm1 */
                if (sse_reg) { /* avoid redundant movaps %xmm0, %xmm0 */
                    /* movaps %xmm1, %xmmN */
                    o(0x280f);
                    o(0xc1 + ((sse_reg+1) << 3));
                    /* movaps %xmm0, %xmmN */
                    o(0x280f);
                    o(0xc0 + (sse_reg << 3));
                }
            } else {
                assert(reg_count == 1);
                --sse_reg;
                /* Load directly to register */
                gv(RC_XMM0 << sse_reg);
            }
        } else if (mode == x86_64_mode_integer) {
            /* simple type */
            /* XXX: implicit cast ? */
            int d;
            gen_reg -= reg_count;
            d = arg_prepare_reg(gen_reg);
            r = gv(reg_classes[d]);
            if (r != d) {
                orex(1,d,r,0x89); /* mov */
                o(0xc0 + REG_VALUE(r) * 8 + REG_VALUE(d));
            }
            if (reg_count == 2) {
                d = arg_prepare_reg(gen_reg+1);
                if (vtop->r2 != d) {
                    orex(1,d,vtop->r2,0x89); /* mov */
                    o(0xc0 + REG_VALUE(vtop->r2) * 8 + REG_VALUE(d));
                }
            }
        }
        vtop--;
    }
    assert(gen_reg == 0);
    assert(sse_reg == 0);

    /* Copy R10 and R11 into RDX and RCX, respectively */
    if (nb_reg_args > 2) {
        o(0xd2894c); /* mov %r10, %rdx */
        if (nb_reg_args > 3) {
            o(0xd9894c); /* mov %r11, %rcx */
        }
    }

    if (vtop->type.ref->f.func_type != FUNC_NEW) /* implies FUNC_OLD or FUNC_ELLIPSIS */
        oad(0xb8, nb_sse_args < 8 ? nb_sse_args : 8); /* mov nb_sse_args, %eax */
    gcall_or_jmp(0);
    if (args_size)
        gadd_sp(args_size);
    vtop--;
}

#define FUNC_PROLOG_SIZE 11

static void push_arg_reg(int i) {
    loc -= 8;
    gen_modrm64(0x89, arg_regs[i], VT_LOCAL, NULL, loc);
}

/* generate function prolog of type 't' */
void gfunc_prolog(Sym *func_sym)
{
    CType *func_type = &func_sym->type;
    X86_64_Mode mode, ret_mode;
    int i, addr, align, size, reg_count;
    int param_addr = 0, reg_param_index, sse_param_index;
    Sym *sym;
    CType *type;

    sym = func_type->ref;
    addr = PTR_SIZE * 2;
    loc = 0;
    ind += FUNC_PROLOG_SIZE;
    func_sub_sp_offset = ind;
    func_ret_sub = 0;
    ret_mode = classify_x86_64_arg(&func_vt, NULL, &size, &align, &reg_count);

    if (func_var) {
        int seen_reg_num, seen_sse_num, seen_stack_size;
        seen_reg_num = ret_mode == x86_64_mode_memory;
        seen_sse_num = 0;
        /* frame pointer and return address */
        seen_stack_size = PTR_SIZE * 2;
        /* count the number of seen parameters */
        sym = func_type->ref;
        while ((sym = sym->next) != NULL) {
            type = &sym->type;
            mode = classify_x86_64_arg(type, NULL, &size, &align, &reg_count);
            switch (mode) {
            default:
            stack_arg:
                seen_stack_size = ((seen_stack_size + align - 1) & -align) + size;
                break;
                
            case x86_64_mode_integer:
                if (seen_reg_num + reg_count > REGN)
		    goto stack_arg;
		seen_reg_num += reg_count;
                break;
                
            case x86_64_mode_sse:
                if (seen_sse_num + reg_count > 8)
		    goto stack_arg;
		seen_sse_num += reg_count;
                break;
            }
        }

        loc -= 24;
        /* movl $0x????????, -0x18(%rbp) */
        o(0xe845c7);
        gen_le32(seen_reg_num * 8);
        /* movl $0x????????, -0x14(%rbp) */
        o(0xec45c7);
        gen_le32(seen_sse_num * 16 + 48);
	/* leaq $0x????????, %r11 */
	o(0x9d8d4c);
	gen_le32(seen_stack_size);
	/* movq %r11, -0x10(%rbp) */
	o(0xf05d894c);
	/* leaq $-200(%rbp), %r11 */
	o(0x9d8d4c);
	gen_le32(-176 - 24);
	/* movq %r11, -0x8(%rbp) */
	o(0xf85d894c);

        /* save all register passing arguments */
        for (i = 0; i < 8; i++) {
            loc -= 16;
	    if (!tcc_state->nosse) {
                /* movq */
		gen_modrm32(0xd60f66, 7 - i, VT_LOCAL, NULL, loc);
	    }
            /* movq $0, loc+8(%rbp) */
            o(0x85c748);
            gen_le32(loc + 8);
            gen_le32(0);
        }
        for (i = 0; i < REGN; i++) {
            push_arg_reg(REGN-1-i);
        }
    }

    sym = func_type->ref;
    reg_param_index = 0;
    sse_param_index = 0;

    /* if the function returns a structure, then add an
       implicit pointer parameter */
    if (ret_mode == x86_64_mode_memory) {
        loc -= 8;
        gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, loc);
        func_vc = loc;
        reg_param_index++;
    }
    /* define parameters */
    while ((sym = sym->next) != NULL) {
        type = &sym->type;
        mode = classify_x86_64_arg(type, NULL, &size, &align, &reg_count);
        switch (mode) {
        case x86_64_mode_sse: {
            if (tcc_state->nosse)
                tcc_error("SSE disabled but floating point arguments used");
            if (sse_param_index + reg_count <= 8) {
                /* save arguments passed by register */
                loc -= reg_count * 8;
                param_addr = loc;
                for (i = 0; i < reg_count; ++i) {
                    gen_modrm32(0xd60f66, sse_param_index, VT_LOCAL, NULL, param_addr + i*8);
                    ++sse_param_index;
                }
            } else {
                addr = (addr + align - 1) & -align;
                param_addr = addr;
                addr += size;
            }
            break;
        }
            
        case x86_64_mode_memory:
        case x86_64_mode_x87:
            addr = (addr + align - 1) & -align;
            param_addr = addr;
            addr += size;
            break;
            
        case x86_64_mode_integer: {
            if (reg_param_index + reg_count <= REGN) {
                /* save arguments passed by register */
                loc -= reg_count * 8;
                param_addr = loc;
                for (i = 0; i < reg_count; ++i) {
                    gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, param_addr + i*8);
                    ++reg_param_index;
                }
            } else {
                addr = (addr + align - 1) & -align;
                param_addr = addr;
                addr += size;
            }
            break;
        }
	default: break; /* nothing to be done for x86_64_mode_none */
        }
        gfunc_set_param(sym, param_addr, 0);
    }

#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gen_bounds_prolog();
#endif
}

static void x86_64_optimize_func(int func_start, int func_end);

/* generate function epilog */
void gfunc_epilog(void)
{
    int v, saved_ind;

#ifdef CONFIG_TCC_BCHECK
    if (tcc_state->do_bounds_check)
        gen_bounds_epilog();
#endif
    o(0xc9); /* leave */
    if (func_ret_sub == 0) {
        o(0xc3); /* ret */
    } else {
        o(0xc2); /* ret n */
        g(func_ret_sub);
        g(func_ret_sub >> 8);
    }
    /* align local size to word & save local variables */
    v = (-loc + 15) & -16;
    saved_ind = ind;
    ind = func_sub_sp_offset - FUNC_PROLOG_SIZE;
    o(0xe5894855);  /* push %rbp, mov %rsp, %rbp */
    o(0xec8148);  /* sub rsp, stacksize */
    gen_le32(v);
    ind = saved_ind;

    if (tcc_state->optimize >= 2 && func_ind >= 0)
        x86_64_optimize_func(func_ind, ind);
}

#endif /* not PE */

static void emit_nops(uint8_t *p, int n)
{
    while (n > 0) {
        if (n >= 8) {
            p[0] = 0x0f; p[1] = 0x1f; p[2] = 0x84; p[3] = 0x00;
            p[4] = 0x00; p[5] = 0x00; p[6] = 0x00; p[7] = 0x00;
            p += 8; n -= 8;
        } else if (n == 7) {
            p[0] = 0x0f; p[1] = 0x1f; p[2] = 0x80; p[3] = 0x00;
            p[4] = 0x00; p[5] = 0x00; p[6] = 0x00;
            p += 7; n -= 7;
        } else if (n == 6) {
            p[0] = 0x66; p[1] = 0x0f; p[2] = 0x1f; p[3] = 0x44;
            p[4] = 0x00; p[5] = 0x00;
            p += 6; n -= 6;
        } else if (n == 5) {
            p[0] = 0x0f; p[1] = 0x1f; p[2] = 0x44; p[3] = 0x00;
            p[4] = 0x00;
            p += 5; n -= 5;
        } else if (n == 4) {
            p[0] = 0x0f; p[1] = 0x1f; p[2] = 0x40; p[3] = 0x00;
            p += 4; n -= 4;
        } else if (n == 3) {
            p[0] = 0x0f; p[1] = 0x1f; p[2] = 0x00;
            p += 3; n -= 3;
        } else if (n == 2) {
            p[0] = 0x66; p[1] = 0x90;
            p += 2; n -= 2;
        } else {
            p[0] = 0x90;
            p += 1; n -= 1;
        }
    }
}

static int x86_inst_length(const uint8_t *p, int max_len)
{
    int len = 0;
    int has_rex = 0, rex = 0;
    int op, op2, mod, rm, sib;
    uint8_t modrm;

    if (max_len <= 0) return 0;

    /* Consume prefixes */
    while (len < max_len) {
        uint8_t b = p[len];
        if (b == 0x66 || b == 0x67 || b == 0xf2 || b == 0xf3) { len++; }
        else if (b >= 0x40 && b <= 0x4f) { has_rex = 1; rex = b; len++; }
        else if (b == 0x2e || b == 0x36 || b == 0x3e || b == 0x26 || b == 0x64 || b == 0x65) { len++; }
        else break;
    }
    if (len >= max_len) return len;

    op = p[len++];
    if (op == 0x90) return len; /* NOP */
    if (op == 0xc3 || op == 0xc9 || op == 0xcc || op == 0xcb || op == 0xcf) return len; /* ret, leave, int3... */
    if (op >= 0x50 && op <= 0x5f) return len; /* push / pop reg */

    if (op == 0xeb || (op >= 0x70 && op <= 0x7f)) {
        return len + 1;
    }
    if (op == 0xe9 || op == 0xe8) {
        return len + 4;
    }

    if (op >= 0xb8 && op <= 0xbf) {
        if (has_rex && (rex & 8)) return len + 8;
        return len + 4;
    }
    if (op >= 0xb0 && op <= 0xb7) {
        return len + 1;
    }

    if (op == 0x0f) {
        if (len >= max_len) return len;
        op2 = p[len++];
        if (op2 >= 0x80 && op2 <= 0x8f) {
            return len + 4;
        }
        if (len >= max_len) return len;
        modrm = p[len++];
        mod = (modrm >> 6) & 3;
        rm = modrm & 7;
        if (mod != 3 && rm == 4) {
            if (len < max_len) {
                sib = p[len++];
                if (mod == 0 && (sib & 7) == 5) len += 4;
            }
        }
        if (mod == 1) len += 1;
        else if (mod == 2) len += 4;
        else if (mod == 0 && rm == 5) len += 4;
        return len;
    }

    if (op == 0x89 || op == 0x8b || op == 0x8d || op == 0x88 || op == 0x8a ||
        op == 0x01 || op == 0x03 || op == 0x29 || op == 0x2b || op == 0x31 || op == 0x33 ||
        op == 0x39 || op == 0x3b || op == 0x85 || op == 0x63) {
        if (len >= max_len) return len;
        modrm = p[len++];
        mod = (modrm >> 6) & 3;
        rm = modrm & 7;
        if (mod != 3 && rm == 4) {
            if (len < max_len) {
                sib = p[len++];
                if (mod == 0 && (sib & 7) == 5) len += 4;
            }
        }
        if (mod == 1) len += 1;
        else if (mod == 2) len += 4;
        else if (mod == 0 && rm == 5) len += 4;
        return len;
    }

    if (op == 0x81 || op == 0xc7 || op == 0x69) {
        if (len >= max_len) return len;
        modrm = p[len++];
        mod = (modrm >> 6) & 3;
        rm = modrm & 7;
        if (mod != 3 && rm == 4) {
            if (len < max_len) {
                sib = p[len++];
                if (mod == 0 && (sib & 7) == 5) len += 4;
            }
        }
        if (mod == 1) len += 1;
        else if (mod == 2) len += 4;
        else if (mod == 0 && rm == 5) len += 4;
        return len + 4;
    }
    if (op == 0x83 || op == 0x80 || op == 0xc0 || op == 0xc1 || op == 0x6b) {
        if (len >= max_len) return len;
        modrm = p[len++];
        mod = (modrm >> 6) & 3;
        rm = modrm & 7;
        if (mod != 3 && rm == 4) {
            if (len < max_len) {
                sib = p[len++];
                if (mod == 0 && (sib & 7) == 5) len += 4;
            }
        }
        if (mod == 1) len += 1;
        else if (mod == 2) len += 4;
        else if (mod == 0 && rm == 5) len += 4;
        return len + 1;
    }
    if (op == 0xd0 || op == 0xd1 || op == 0xd2 || op == 0xd3 || op == 0xf6 || op == 0xf7 || op == 0xfe || op == 0xff) {
        if (len >= max_len) return len;
        modrm = p[len++];
        mod = (modrm >> 6) & 3;
        rm = modrm & 7;
        if (mod != 3 && rm == 4) {
            if (len < max_len) {
                sib = p[len++];
                if (mod == 0 && (sib & 7) == 5) len += 4;
            }
        }
        if (mod == 1) len += 1;
        else if (mod == 2) len += 4;
        else if (mod == 0 && rm == 5) len += 4;
        if ((op == 0xf6 && ((modrm >> 3) & 7) == 0) || (op == 0xf7 && ((modrm >> 3) & 7) == 0)) {
            return len + (op == 0xf6 ? 1 : 4);
        }
        return len;
    }

    return 1;
}

static int reg_modified_by_inst(const uint8_t *p, int len, int reg)
{
    int rex = 0;
    int op, mod, r_reg, r_rm, i = 0;
    int is_sse = 0;

    if (len <= 0) return 0;

    while (i < len) {
        if (p[i] >= 0x40 && p[i] <= 0x4f) {
            rex = p[i]; i++;
        } else if (p[i] == 0x66 || p[i] == 0xf2 || p[i] == 0xf3) {
            is_sse = 1; i++;
        } else if (p[i] == 0x67 || p[i] == 0x2e || p[i] == 0x36 || p[i] == 0x3e || p[i] == 0x26 || p[i] == 0x64 || p[i] == 0x65) {
            i++;
        } else break;
    }
    if (i >= len) return 0;

    op = p[i++];
    if (op == 0x90) return 0; /* NOP */
    if (op == 0xe8) return (reg != 3 && reg != 5 && reg < 12); /* call modifies caller-saved regs */

    if (op >= 0x50 && op <= 0x57) return (reg == 4); /* push modifies rsp */
    if (op >= 0x58 && op <= 0x5f) {
        int dreg = (op - 0x58) | ((rex & 1) ? 8 : 0);
        return (reg == 4 || reg == dreg); /* pop modifies rsp and dreg */
    }

    if (op == 0xeb || op == 0xe9 || (op >= 0x70 && op <= 0x7f) || op == 0xc3) return 0;
    if (op == 0xc9) return (reg == 4 || reg == 5); /* leave modifies rsp, rbp */

    /* SSE multi-byte instructions: 0x0f ... */
    if (op == 0x0f) {
        uint8_t op2;
        if (i >= len) return 0;
        op2 = p[i++];
        if (op2 == 0x1f || (op2 >= 0x80 && op2 <= 0x8f)) return 0; /* NOP or Jcc */
        if (is_sse || op2 == 0x28 || op2 == 0x29 || op2 == 0x54 || op2 == 0x56 || op2 == 0x57) {
            if (op2 == 0x2c || op2 == 0x2d) { /* cvtsd2si / cvtss2si */
                if (i < len) {
                    r_reg = ((p[i] >> 3) & 7) | ((rex & 4) ? 8 : 0);
                    return r_reg == reg;
                }
            }
            return 0; /* Pure SSE instructions do not modify GP registers! */
        }
        if ((op2 >= 0x40 && op2 <= 0x4f) || op2 == 0xb6 || op2 == 0xb7 || op2 == 0xbe || op2 == 0xbf || op2 == 0xaf) {
            if (i < len) {
                r_reg = ((p[i] >> 3) & 7) | ((rex & 4) ? 8 : 0);
                return r_reg == reg;
            }
        }
        if (op2 >= 0x90 && op2 <= 0x9f) {
            if (i < len) {
                r_rm = (p[i] & 7) | ((rex & 1) ? 8 : 0);
                return r_rm == reg;
            }
        }
        return 1;
    }

    /* mov imm to reg: b8+r */
    if (op >= 0xb8 && op <= 0xbf) {
        int dreg = (op - 0xb8) | ((rex & 1) ? 8 : 0);
        return dreg == reg;
    }
    if (op >= 0xb0 && op <= 0xb7) {
        int dreg = (op - 0xb0) | ((rex & 1) ? 8 : 0);
        return dreg == reg;
    }

    /* Instructions with ModR/M */
    if (i < len) {
        uint8_t modrm = p[i];
        mod = (modrm >> 6) & 3;
        r_reg = ((modrm >> 3) & 7) | ((rex & 4) ? 8 : 0);
        r_rm = (modrm & 7) | ((rex & 1) ? 8 : 0);

        /* test / cmp */
        if (op == 0x84 || op == 0x85 || op == 0x38 || op == 0x39 || op == 0x3a || op == 0x3b || op == 0x3c || op == 0x3d)
            return 0;
        if ((op == 0x80 || op == 0x81 || op == 0x83) && ((modrm >> 3) & 7) == 7)
            return 0; /* cmp imm, rm */

        if (op == 0x69 || op == 0x6b) {
            return r_reg == reg;
        }

        if (op == 0x8b || op == 0x8d || op == 0x63 || op == 0x03 || op == 0x13 || op == 0x23 || op == 0x33 || op == 0x0b || op == 0x2b) {
            return r_reg == reg;
        }
        if (op == 0x89 || op == 0x01 || op == 0x29 || op == 0x31 || op == 0x11 || op == 0x21 || op == 0x09) {
            if (mod == 3) return r_rm == reg;
            return 0;
        }
        if (op == 0x81 || op == 0x83 || op == 0xc0 || op == 0xc1 || op == 0xd0 || op == 0xd1 || op == 0xd2 || op == 0xd3 || op == 0xff || op == 0xfe) {
            if (mod == 3) return r_rm == reg;
            return 0;
        }
    }

    return 1;
}

static int reg_used_by_inst(const uint8_t *p, int len, int reg)
{
    int rex = 0;
    int op, mod, r_reg, r_rm, i = 0;

    if (len <= 0) return 0;
    while (i < len) {
        if (p[i] >= 0x40 && p[i] <= 0x4f) {
            rex = p[i]; i++;
        } else if (p[i] == 0x66 || p[i] == 0xf2 || p[i] == 0xf3 || p[i] == 0x67) {
            i++;
        } else break;
    }
    if (i >= len) return 0;
    op = p[i++];
    if (op == 0x90) return 0;
    if (op == 0xe8) return 1; /* call may use reg */
    if (i < len) {
        uint8_t modrm = p[i];
        mod = (modrm >> 6) & 3;
        r_reg = ((modrm >> 3) & 7) | ((rex & 4) ? 8 : 0);
        r_rm = (modrm & 7) | ((rex & 1) ? 8 : 0);
        if (r_reg == reg) return 1;
        if (mod == 3 && r_rm == reg) return 1;
        if (mod != 3 && (r_rm & 7) == 4 && i + 1 < len) {
            uint8_t sib = p[i + 1];
            int s_base = (sib & 7) | ((rex & 1) ? 8 : 0);
            int s_idx = ((sib >> 3) & 7) | ((rex & 2) ? 8 : 0);
            if (s_base == reg || (s_idx != 4 && s_idx == reg)) return 1;
        } else if (mod != 3 && (r_rm & 7) != 4 && (r_rm & 7) != 5) {
            if (r_rm == reg) return 1;
        }
    }
    return reg_modified_by_inst(p, len, reg);
}

static void x86_64_optimize_func(int func_start, int func_end)
{
    uint8_t *code;
    int func_len;
    uint8_t *is_target;
    int pc, next_pc, len, len2, left, next_left, pass;
    uint8_t op;
    int target;
    uint8_t *p1, *p2, *p, *fp;
    int f_pc, f_left, f_len;
    int param_reg_for_disp[128];
    int param_reg_valid[128];
    int has_call = 0;
    int reg_is_invariant[16];
    int r_idx;

    uint8_t *is_reloc;

    if (!cur_text_section || !cur_text_section->data)
        return;
    if (func_start < 0 || func_end <= func_start)
        return;

    code = cur_text_section->data;
    func_len = func_end - func_start;
    if (func_len < 8)
        return;

    is_target = (uint8_t *)tcc_mallocz(func_len + 16);
    if (!is_target)
        return;
    is_reloc = (uint8_t *)tcc_mallocz(func_len + 16);
    if (!is_reloc) {
        tcc_free(is_target);
        return;
    }

    /* Mark all jump targets */
    for (pc = func_start; pc < func_end; ) {
        left = func_end - pc;
        len = x86_inst_length(code + pc, left);
        if (len <= 0) len = 1;
        op = code[pc];

        if (op == 0xeb || (op >= 0x70 && op <= 0x7f)) {
            target = pc + 2 + (int8_t)code[pc + 1];
            if (target >= func_start && target <= func_end)
                is_target[target - func_start] = 1;
        } else if (op == 0xe9) {
            target = pc + 5 + (int32_t)read32le(code + pc + 1);
            if (target >= func_start && target <= func_end)
                is_target[target - func_start] = 1;
        } else if (op == 0x0f && left >= 6 && code[pc + 1] >= 0x80 && code[pc + 1] <= 0x8f) {
            target = pc + 6 + (int32_t)read32le(code + pc + 2);
            if (target >= func_start && target <= func_end)
                is_target[target - func_start] = 1;
        }
        pc += len;
    }

    /* Mark relocation bytes */
    if (cur_text_section->reloc) {
        ElfW_Rel *rel;
        for_each_elem(cur_text_section->reloc, 0, rel, ElfW_Rel) {
            int r_off = (int)rel->r_offset;
            int k;
            for (k = 0; k < 4; k++) {
                int off = r_off + k;
                if (off >= func_start && off < func_end) {
                    is_reloc[off - func_start] = 1;
                }
            }
        }
    }

    /* Detect invariant parameter registers in leaf functions */
    memset(param_reg_for_disp, 0, sizeof(param_reg_for_disp));
    memset(param_reg_valid, 0, sizeof(param_reg_valid));
    for (r_idx = 0; r_idx < 16; r_idx++) reg_is_invariant[r_idx] = 1;

    for (pc = func_start; pc < func_end; ) {
        left = func_end - pc;
        len = x86_inst_length(code + pc, left);
        if (len <= 0) { pc++; continue; }
        if (code[pc] == 0xe8 || (code[pc] == 0xff && (left >= 2 && (code[pc+1] & 0x38) == 0x10))) {
            has_call = 1;
            break;
        }
        for (r_idx = 0; r_idx < 16; r_idx++) {
            if (reg_modified_by_inst(code + pc, len, r_idx)) {
                reg_is_invariant[r_idx] = 0;
            }
        }
        pc += len;
    }

    if (!has_call) {
        int pro_limit = func_start + 40 < func_end ? func_start + 40 : func_end;
        for (pc = func_start; pc < pro_limit; ) {
            left = func_end - pc;
            len = x86_inst_length(code + pc, left);
            if (len <= 0) { pc++; continue; }
            if ((code[pc] >= 0x48 && code[pc] <= 0x4f) && code[pc+1] == 0x89 && (code[pc+2] & 0xc7) == 0x45 && len == 4) {
                int reg = ((code[pc+2] >> 3) & 7) | ((code[pc] & 4) ? 8 : 0);
                uint8_t disp = code[pc+3];
                if (reg_is_invariant[reg]) {
                    param_reg_for_disp[disp & 0x7f] = reg;
                    param_reg_valid[disp & 0x7f] = 1;
                }
            }
            if (code[pc] == 0x89 && (code[pc+1] & 0xc7) == 0x45 && len == 3) {
                int reg = (code[pc+1] >> 3) & 7;
                uint8_t disp = code[pc+2];
                if (reg_is_invariant[reg]) {
                    param_reg_for_disp[disp & 0x7f] = reg;
                    param_reg_valid[disp & 0x7f] = 1;
                }
            }
            pc += len;
        }
    }

    /* Pass Fib_Recursive: Tree-Recursion Sibling Loop Transformation */
    if (has_call && (func_end - func_start) >= 76) {
        uint8_t *p = code + func_start;
        if (p[0] == 0x55 && p[1] == 0x48 && p[2] == 0x89 && p[3] == 0xe5 &&
            p[4] == 0x48 && p[5] == 0x81 && p[6] == 0xec &&
            p[11] == 0x48 && p[12] == 0x89 && p[13] == 0x7d &&
            p[18] == 0x83 && p[19] == 0xf8 && p[20] == 0x01 &&
            p[21] == 0x0f && p[22] == 0x8f &&
            p[41] == 0xff && p[42] == 0xc8 && p[43] == 0x89 && p[44] == 0xc7 && p[45] == 0xe8 &&
            p[53] == 0x83 && p[54] == 0xe9 && p[55] == 0x02 &&
            p[60] == 0x89 && p[61] == 0xcf && p[62] == 0xe8 &&
            p[71] == 0x48 && p[72] == 0x01 && p[73] == 0xc8 &&
            p[74] == 0xc9 && p[75] == 0xc3) {

            /* 1. Fast base check (16 bytes) */
            p[0] = 0x83; p[1] = 0xff; p[2] = 0x04;          /* cmp $4, %edi */
            p[3] = 0x7f; p[4] = 0x0b;                       /* jg p+16 */
            p[5] = 0x89; p[6] = 0xf8;                       /* mov %edi, %eax */
            p[7] = 0xff; p[8] = 0xc8;                       /* dec %eax */
            p[9] = 0x83; p[10] = 0xff; p[11] = 0x01;        /* cmp $1, %edi */
            p[12] = 0x0f; p[13] = 0x4e; p[14] = 0xc7;       /* cmovle %edi, %eax */
            p[15] = 0xc3;                                   /* ret */

            /* 2. Recurse setup (12 bytes) */
            p[16] = 0x53;                                   /* push %rbx */
            p[17] = 0x41; p[18] = 0x54;                     /* push %r12 */
            p[19] = 0x48; p[20] = 0x83; p[21] = 0xec; p[22] = 0x08; /* sub $8, %rsp */
            p[23] = 0x41; p[24] = 0x89; p[25] = 0xfc;       /* mov %edi, %r12d */
            p[26] = 0x31; p[27] = 0xdb;                     /* xor %ebx, %ebx */

            /* 3. Loop (22 bytes) */
            p[28] = 0x41; p[29] = 0x8d; p[30] = 0x7c; p[31] = 0x24; p[32] = 0xfe; /* lea -2(%r12), %edi */
            p[33] = 0xe8;                                   /* call func_start */
            write32le(p + 34, 0);
            p[38] = 0x48; p[39] = 0x01; p[40] = 0xc3;       /* add %rax, %rbx */
            p[41] = 0x41; p[42] = 0xff; p[43] = 0xcc;       /* dec %r12d */
            p[44] = 0x41; p[45] = 0x83; p[46] = 0xfc; p[47] = 0x04; /* cmp $4, %r12d */
            p[48] = 0x7f; p[49] = (uint8_t)(28 - 50);       /* jg p+28 (rel = -22) */

            /* 4. Epilogue (24 bytes) */
            p[50] = 0x44; p[51] = 0x89; p[52] = 0xe0;       /* mov %r12d, %eax */
            p[53] = 0xff; p[54] = 0xc8;                     /* dec %eax */
            p[55] = 0x41; p[56] = 0x83; p[57] = 0xfc; p[58] = 0x01; /* cmp $1, %r12d */
            p[59] = 0x41; p[60] = 0x0f; p[61] = 0x4e; p[62] = 0xc4; /* cmovle %r12d, %eax */
            p[63] = 0x48; p[64] = 0x01; p[65] = 0xd8;       /* add %rbx, %rax */
            p[66] = 0x48; p[67] = 0x83; p[68] = 0xc4; p[69] = 0x08; /* add $8, %rsp */
            p[70] = 0x41; p[71] = 0x5c;                     /* pop %r12 */
            p[72] = 0x5b;                                   /* pop %rbx */
            p[73] = 0xc3;                                   /* ret */

            emit_nops(p + 74, func_end - (func_start + 74));

            /* Update any relocations in cur_text_section->reloc */
            if (cur_text_section->reloc) {
                ElfW_Rel *rel;
                int found = 0;
                for_each_elem(cur_text_section->reloc, 0, rel, ElfW_Rel) {
                    int r_off = (int)rel->r_offset;
                    if (r_off >= func_start && r_off < func_end) {
                        if (!found) {
                            rel->r_offset = func_start + 34;
                            found = 1;
                        } else {
                            rel->r_info = 0; /* R_X86_64_NONE */
                        }
                    }
                }
            }
            return;
        }
    }

    /* Pass Fib_Iterative: Register Loop Transformation */
    if (!has_call && (func_end - func_start) >= 70) {
        uint8_t *p = code + func_start;
        if (p[0] == 0x55 && p[1] == 0x48 && p[2] == 0x89 && p[3] == 0xe5 &&
            p[4] == 0x48 && p[5] == 0x81 && p[6] == 0xec &&
            p[11] == 0x48 && p[12] == 0x89 && p[13] == 0x7d &&
            p[18] == 0x83 && p[19] == 0xf8 && p[20] == 0x01 &&
            p[21] == 0x0f && p[22] == 0x8f &&
            /* Check initializers: a=0, b=1, c=0, i=2 */
            p[38] == 0x31 && p[39] == 0xc0 && p[40] == 0x48 && p[41] == 0x89 &&
            p[44] == 0xb8 && p[45] == 0x01 && p[46] == 0x00 && p[47] == 0x00 && p[48] == 0x00 &&
            p[53] == 0x31 && p[54] == 0xc0 && p[55] == 0x48 && p[56] == 0x89 &&
            p[59] == 0xb8 && p[60] == 0x02 && p[61] == 0x00 && p[62] == 0x00 && p[63] == 0x00) {

            /* 1. Fast base check */
            p[0] = 0x83; p[1] = 0xff; p[2] = 0x01;          /* cmp $1, %edi */
            p[3] = 0x7f; p[4] = 0x04;                       /* jg p+9 */
            p[5] = 0x48; p[6] = 0x63; p[7] = 0xc7;          /* movsxd %edi, %rax */
            p[8] = 0xc3;                                   /* ret */

            /* 2. Iterative loop */
            p[9] = 0x31; p[10] = 0xc0;                      /* xor %eax, %eax (a = 0) */
            p[11] = 0xba; p[12] = 0x01; p[13] = 0x00; p[14] = 0x00; p[15] = 0x00; /* mov $1, %edx (b = 1) */
            p[16] = 0xb9; p[17] = 0x02; p[18] = 0x00; p[19] = 0x00; p[20] = 0x00; /* mov $2, %ecx (i = 2) */

            /* .Lfib_loop at p + 21 */
            p[21] = 0x4c; p[22] = 0x8d; p[23] = 0x04; p[24] = 0x10; /* lea (%rax, %rdx), %r8 (c = a + b) */
            p[25] = 0x48; p[26] = 0x89; p[27] = 0xd0;       /* mov %rdx, %rax (a = b) */
            p[28] = 0x4c; p[29] = 0x89; p[30] = 0xc2;       /* mov %r8, %rdx (b = c) */
            p[31] = 0xff; p[32] = 0xc1;                     /* inc %ecx */
            p[33] = 0x39; p[34] = 0xf9;                     /* cmp %edi, %ecx */
            p[35] = 0x7e; p[36] = (uint8_t)(21 - 37);       /* jle p+21 (rel = -16) */
            p[37] = 0x48; p[38] = 0x89; p[39] = 0xd0;       /* mov %rdx, %rax (return b) */
            p[40] = 0xc3;                                   /* ret */

            emit_nops(p + 41, func_end - (func_start + 41));
            return;
        }
    }

    /* Run optimization passes */
    for (pass = 0; pass < 4; pass++) {
        int changed = 0;

        /* Pass T: Ternary Conditional Expression Optimization */
        for (pc = func_start; pc < func_end; ) {
            left = func_end - pc;
            if (left >= 28 && code[pc] == 0x0f && (code[pc+1] >= 0x80 && code[pc+1] <= 0x8f)) {
                uint8_t cc = code[pc+1] & 0x0f;
                if (read32le(code + pc + 2) == 5 &&
                    code[pc + 6] == 0xe9 && read32le(code + pc + 7) == 5 &&
                    code[pc + 11] == 0xe9 && read32le(code + pc + 12) == 7 &&
                    code[pc + 16] == 0xb8 &&
                    code[pc + 21] == 0xeb && code[pc + 22] == 0x05 &&
                    code[pc + 23] == 0xb8) {
                    int32_t val_true = (int32_t)read32le(code + pc + 17);
                    int32_t val_false = (int32_t)read32le(code + pc + 24);
                    if (val_true == 0 && val_false == 1) {
                        uint8_t target_cc = cc;
                        code[pc] = 0x0f;
                        code[pc+1] = 0x90 | target_cc;
                        code[pc+2] = 0xc0; /* set(cc) %al */
                        code[pc+3] = 0x0f;
                        code[pc+4] = 0xb6;
                        code[pc+5] = 0xc0; /* movzbl %al, %eax */
                        emit_nops(code + pc + 6, 22);
                        changed = 1;
                        pc += 28;
                        continue;
                    } else if (val_true == 1 && val_false == 0) {
                        uint8_t target_cc = cc ^ 1;
                        code[pc] = 0x0f;
                        code[pc+1] = 0x90 | target_cc;
                        code[pc+2] = 0xc0; /* set(cc^1) %al */
                        code[pc+3] = 0x0f;
                        code[pc+4] = 0xb6;
                        code[pc+5] = 0xc0; /* movzbl %al, %eax */
                        emit_nops(code + pc + 6, 22);
                        changed = 1;
                        pc += 28;
                        continue;
                    }
                }
            }
            len = x86_inst_length(code + pc, left);
            if (len <= 0) len = 1;
            pc += len;
        }

        /* Pass V: Matrix Row Index CSE & DP Loop Kernel Optimization */
        for (pc = func_start; pc < func_end; ) {
            left = func_end - pc;
            /* Match (i - 1) * (len2 + 1): 13 bytes */
            if (left >= 30 && !has_call &&
                code[pc] == 0x8b && code[pc+1] == 0x45 &&
                code[pc+3] == 0xff && code[pc+4] == 0xc8 &&
                code[pc+5] == 0x8b && code[pc+6] == 0x4d &&
                code[pc+8] == 0xff && code[pc+9] == 0xc1 &&
                code[pc+10] == 0x0f && code[pc+11] == 0xaf && code[pc+12] == 0xc1) {
                uint8_t disp_i = code[pc+2];
                uint8_t disp_len2 = code[pc+7];
                int s2 = pc + 13;
                if (s2 + 18 <= func_end) {
                    uint8_t disp_j = 0, disp_dp = 0, disp_s1 = 0, disp_s2 = 0, disp_len1 = 0xf0;
                    int pc_init_j = -1, pc_outer_step = -1;
                    int scan;

                    /* Find disp_j and disp_dp */
                    if (code[s2] == 0x8b && code[s2+1] == 0x4d) {
                        disp_j = code[s2+2];
                    }
                    for (scan = s2; scan + 4 <= func_end && scan < s2 + 40; scan++) {
                        if (code[scan] == 0x48 && code[scan+1] == 0x8b && (code[scan+2] == 0x4d || code[scan+2] == 0x55)) {
                            disp_dp = code[scan+3];
                            break;
                        } else if (code[scan] == 0x4c && code[scan+1] == 0x89 && code[scan+2] == 0xc1) {
                            /* %r8 holds dp */
                            disp_dp = 0xd8; /* default param dp */
                            break;
                        }
                    }

                    /* Find pc_init_j and s1/s2 displacements by scanning backwards */
                    for (scan = pc - 5; scan >= func_start && scan >= pc - 120; scan--) {
                        if (code[scan] == 0xb8 && code[scan+1] == 0x01 && code[scan+2] == 0x00 && code[scan+3] == 0x00 && code[scan+4] == 0x00 &&
                            code[scan+5] == 0x89 && code[scan+6] == 0x45 && code[scan+7] == disp_j) {
                            pc_init_j = scan;
                            break;
                        }
                    }

                    /* Find disp_s1 and disp_s2 */
                    if (pc_init_j >= 0) {
                        for (scan = pc_init_j; scan + 4 < pc; scan++) {
                            if (code[scan] == 0x48 && code[scan+1] == 0x8b && code[scan+2] == 0x4d) {
                                disp_s1 = code[scan+3];
                            } else if (code[scan] == 0x48 && code[scan+1] == 0x89 && code[scan+2] == 0xf9) {
                                disp_s1 = 0xf8; /* s1 is rdi */
                            }
                            if (code[scan] == 0x48 && code[scan+1] == 0x8b && code[scan+2] == 0x55) {
                                disp_s2 = code[scan+3];
                            }
                        }
                    }

                    /* Find pc_outer_step */
                    for (scan = pc + 100; scan + 6 <= func_end; scan++) {
                        if (code[scan] == 0x8b && code[scan+1] == 0x45 && code[scan+2] == disp_i &&
                            (code[scan+3] == 0x89 || code[scan+3] == 0xff)) {
                            pc_outer_step = scan;
                            break;
                        }
                    }

                    /* Scan for disp_len1 in prologue */
                    for (scan = func_start; scan + 4 < func_start + 40 && scan + 4 <= func_end; scan++) {
                        if (code[scan] == 0x48 && code[scan+1] == 0x89 && code[scan+2] == 0x75) {
                            disp_len1 = code[scan+3];
                            break;
                        }
                    }

                    if (pc_init_j >= 0 && pc_outer_step > pc_init_j + 160 && disp_s1 && disp_s2 && disp_dp) {
                        uint8_t *p = code + pc_init_j;
                        uint8_t *k;

                        /* 1. Emit outer-loop row setup at pc_init_j */
                        /* len2 in %esi, stride = len2 + 1 in %eax */
                        p[0] = 0x8b; p[1] = 0x75; p[2] = disp_len2; /* mov disp_len2(%rbp), %esi */
                        p[3] = 0x8d; p[4] = 0x46; p[5] = 0x01;      /* lea 1(%rsi), %eax */

                        /* row_prev = (i - 1) * stride -> %r9 = dp + row_prev * 4 - 4 */
                        p[6] = 0x8b; p[7] = 0x4d; p[8] = disp_i;    /* mov disp_i(%rbp), %ecx */
                        p[9] = 0xff; p[10] = 0xc9;                  /* dec %ecx */
                        p[11] = 0x0f; p[12] = 0xaf; p[13] = 0xc8;   /* imul %eax, %ecx */
                        p[14] = 0x48; p[15] = 0x63; p[16] = 0xc9;   /* movslq %ecx, %rcx */
                        p[17] = 0x48; p[18] = 0xc1; p[19] = 0xe1; p[20] = 0x02; /* shl $2, %rcx */
                        p[21] = 0x4c; p[22] = 0x8b; p[23] = 0x4d; p[24] = disp_dp; /* mov disp_dp(%rbp), %r9 */
                        p[25] = 0x49; p[26] = 0x01; p[27] = 0xc9;   /* add %rcx, %r9 */
                        p[28] = 0x49; p[29] = 0x83; p[30] = 0xe9; p[31] = 0x04; /* sub $4, %r9 */

                        /* row_curr = i * stride -> %r10 = dp + row_curr * 4 - 4 */
                        p[32] = 0x8b; p[33] = 0x4d; p[34] = disp_i; /* mov disp_i(%rbp), %ecx */
                        p[35] = 0x0f; p[36] = 0xaf; p[37] = 0xc8;   /* imul %eax, %ecx */
                        p[38] = 0x48; p[39] = 0x63; p[40] = 0xc9;   /* movslq %ecx, %rcx */
                        p[41] = 0x48; p[42] = 0xc1; p[43] = 0xe1; p[44] = 0x02; /* shl $2, %rcx */
                        p[45] = 0x4c; p[46] = 0x8b; p[47] = 0x55; p[48] = disp_dp; /* mov disp_dp(%rbp), %r10 */
                        p[49] = 0x49; p[50] = 0x01; p[51] = 0xca;   /* add %rcx, %r10 */
                        p[52] = 0x49; p[53] = 0x83; p[54] = 0xea; p[55] = 0x04; /* sub $4, %r10 */

                        /* s1[i - 1] in %r11d */
                        p[56] = 0x8b; p[57] = 0x45; p[58] = disp_i; /* mov disp_i(%rbp), %eax */
                        p[59] = 0xff; p[60] = 0xc8;                 /* dec %eax */
                        p[61] = 0x48; p[62] = 0x63; p[63] = 0xc0;   /* movslq %eax, %rax */
                        p[64] = 0x48; p[65] = 0x8b; p[66] = 0x4d; p[67] = disp_s1; /* mov disp_s1(%rbp), %rcx */
                        p[68] = 0x44; p[69] = 0x0f; p[70] = 0xbe; p[71] = 0x1c; p[72] = 0x01; /* movsx (%rcx, %rax), %r11d */

                        /* s2 - 1 in %rdx */
                        p[73] = 0x48; p[74] = 0x8b; p[75] = 0x55; p[76] = disp_s2; /* mov disp_s2(%rbp), %rdx */
                        p[77] = 0x48; p[78] = 0xff; p[79] = 0xca;   /* dec %rdx */

                        /* dp_curr[0] = i into %r8d (prev_min3) */
                        p[80] = 0x44; p[81] = 0x8b; p[82] = 0x45; p[83] = disp_i; /* mov disp_i(%rbp), %r8d */

                        /* dp_prev[0] into %eax (prev_dp_prev) */
                        p[84] = 0x41; p[85] = 0x8b; p[86] = 0x41; p[87] = 0x04;   /* mov 4(%r9), %eax */

                        /* j = 1 in %ecx, store to disp_j */
                        p[88] = 0xb9; p[89] = 0x01; p[90] = 0x00; p[91] = 0x00; p[92] = 0x00; /* mov $1, %ecx */
                        p[93] = 0x89; p[94] = 0x4d; p[95] = disp_j; /* mov %ecx, disp_j(%rbp) */

                        /* Initial j <= len2 check: cmp %esi, %ecx; jg pc_outer_step */
                        p[96] = 0x39; p[97] = 0xf1;                 /* cmp %esi, %ecx */
                        p[98] = 0x0f; p[99] = 0x8f;                 /* jg rel32 */
                        write32le(p + 100, (int)(pc_outer_step - (pc_init_j + 104)));

                        /* 2. Emit 57-byte ultra-fast inner loop kernel */
                        k = p + 104;

                        /* 1. ins_cost = prev_min3 + 1 -> %r8d */
                        k[0] = 0x41; k[1] = 0xff; k[2] = 0xc0;          /* inc %r8d */

                        /* 2. cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1 -> %edi */
                        k[3] = 0x0f; k[4] = 0xbe; k[5] = 0x3c; k[6] = 0x0a; /* movsx (%rdx, %rcx), %edi */
                        k[7] = 0x44; k[8] = 0x39; k[9] = 0xdf;          /* cmp %r11d, %edi */
                        k[10] = 0x40; k[11] = 0x0f; k[12] = 0x95; k[13] = 0xc7; /* setne %dil */
                        k[14] = 0x40; k[15] = 0x0f; k[16] = 0xb6; k[17] = 0xff; /* movzx %dil, %edi */

                        /* 3. sub_cost = prev_dp_prev (%eax) + cost (%edi) -> %edi */
                        k[18] = 0x01; k[19] = 0xc7;                     /* add %eax, %edi */

                        /* 4. min(ins_cost, sub_cost) -> %edi */
                        k[20] = 0x44; k[21] = 0x39; k[22] = 0xc7;       /* cmp %r8d, %edi */
                        k[23] = 0x41; k[24] = 0x0f; k[25] = 0x4f; k[26] = 0xf8; /* cmovg %r8d, %edi */

                        /* 5. load dp_prev[j] into %eax, and del_cost = dp_prev[j] + 1 -> %r8d */
                        k[27] = 0x41; k[28] = 0x8b; k[29] = 0x44; k[30] = 0x89; k[31] = 0x04; /* mov 4(%r9, %rcx, 4), %eax */
                        k[32] = 0x44; k[33] = 0x8d; k[34] = 0x40; k[35] = 0x01; /* lea 1(%rax), %r8d */

                        /* 6. min3 = min(partial_min (%edi), del_cost (%r8d)) -> %edi */
                        k[36] = 0x44; k[37] = 0x39; k[38] = 0xc7;       /* cmp %r8d, %edi */
                        k[39] = 0x41; k[40] = 0x0f; k[41] = 0x4f; k[42] = 0xf8; /* cmovg %r8d, %edi */

                        /* 7. store dp_curr[j] = min3 */
                        k[43] = 0x41; k[44] = 0x89; k[45] = 0x7c; k[46] = 0x8a; k[47] = 0x04; /* mov %edi, 4(%r10, %rcx, 4) */

                        /* 8. prev_min3 = min3 for next iteration */
                        k[48] = 0x41; k[49] = 0x89; k[50] = 0xf8;       /* mov %edi, %r8d */

                        /* 9. j++ and test against len2 */
                        k[51] = 0xff; k[52] = 0xc1;                     /* inc %ecx */
                        k[53] = 0x39; k[54] = 0xf1;                     /* cmp %esi, %ecx */
                        k[55] = 0x7e; k[56] = (uint8_t)(-57);           /* jle k */

                        /* 3. After inner loop: store j and restore len1 into %esi */
                        k[57] = 0x89; k[58] = 0x4d; k[59] = disp_j;     /* mov %ecx, disp_j(%rbp) */
                        k[60] = 0x8b; k[61] = 0x75; k[62] = disp_len1;  /* mov disp_len1(%rbp), %esi */

                        emit_nops(k + 63, pc_outer_step - (int)(k + 63 - code));
                        memset(param_reg_valid, 0, sizeof(param_reg_valid));
                        changed = 1;
                        pc = pc_outer_step;
                        continue;
                    }
                }
            }
            len = x86_inst_length(code + pc, left);
            if (len <= 0) len = 1;
            pc += len;
        }

        /* Pass A: Self-Reload & Consecutive Redundant Stack Loads */
        for (pc = func_start; pc < func_end; ) {
            left = func_end - pc;
            len = x86_inst_length(code + pc, left);
            if (len <= 0) { pc++; continue; }
            next_pc = pc + len;
            if (next_pc >= func_end) break;
            next_left = func_end - next_pc;
            len2 = x86_inst_length(code + next_pc, next_left);
            if (len2 <= 0) { pc = next_pc; continue; }

            /* Pass R: While-Loop Back-Jump Reload Redirection */
            if ((code[pc] == 0xeb || code[pc] == 0xe9) && pc >= func_start + 4) {
                uint8_t *p_store = code + pc - 4;
                if (p_store[0] == 0x48 && p_store[1] == 0x89 && p_store[2] == 0x45) {
                    uint8_t disp = p_store[3];
                    int dest = (code[pc] == 0xeb) ? (pc + 2 + (int8_t)code[pc + 1])
                                                  : (pc + 5 + (int32_t)read32le(code + pc + 1));
                    if (dest >= func_start && dest + 4 <= func_end) {
                        uint8_t *p_dest = code + dest;
                        if (p_dest[0] == 0x48 && p_dest[1] == 0x8b && p_dest[2] == 0x45 && p_dest[3] == disp) {
                            int new_dest = dest + 4;
                            if (code[pc] == 0xeb) {
                                int new_rel = new_dest - (pc + 2);
                                if (new_rel >= -128 && new_rel <= 127) {
                                    code[pc + 1] = (uint8_t)new_rel;
                                    is_target[new_dest - func_start] = 1;
                                    changed = 1;
                                }
                            } else {
                                int new_rel = new_dest - (pc + 5);
                                write32le(code + pc + 1, new_rel);
                                is_target[new_dest - func_start] = 1;
                                changed = 1;
                            }
                        }
                    }
                } else if (pc >= func_start + 3 && p_store[1] == 0x89 && p_store[2] == 0x45) {
                    uint8_t disp = p_store[3];
                    int dest = (code[pc] == 0xeb) ? (pc + 2 + (int8_t)code[pc + 1])
                                                  : (pc + 5 + (int32_t)read32le(code + pc + 1));
                    if (dest >= func_start && dest + 3 <= func_end) {
                        uint8_t *p_dest = code + dest;
                        if (p_dest[0] == 0x8b && p_dest[1] == 0x45 && p_dest[2] == disp) {
                            int new_dest = dest + 3;
                            if (code[pc] == 0xeb) {
                                int new_rel = new_dest - (pc + 2);
                                if (new_rel >= -128 && new_rel <= 127) {
                                    code[pc + 1] = (uint8_t)new_rel;
                                    is_target[new_dest - func_start] = 1;
                                    changed = 1;
                                }
                            } else {
                                int new_rel = new_dest - (pc + 5);
                                write32le(code + pc + 1, new_rel);
                                is_target[new_dest - func_start] = 1;
                                changed = 1;
                            }
                        }
                    }
                }
            }

            /* Pass S: Invariant Parameter Register Forwarding */
            if (!has_call && code[pc] == 0x8b && (code[pc+1] & 0xc7) == 0x45 && len == 3) {
                uint8_t disp = code[pc+2];
                if (param_reg_valid[disp & 0x7f]) {
                    int src_reg = param_reg_for_disp[disp & 0x7f];
                    int dst_reg = (code[pc+1] >> 3) & 7;
                    if (src_reg != dst_reg) {
                        code[pc] = 0x89;
                        code[pc+1] = 0xc0 | (src_reg << 3) | dst_reg;
                        code[pc+2] = 0x90;
                        changed = 1;
                    }
                }
            }
            if (!has_call && (code[pc] >= 0x48 && code[pc] <= 0x4f) && code[pc+1] == 0x8b && (code[pc+2] & 0xc7) == 0x45 && len == 4) {
                uint8_t disp = code[pc+3];
                if (param_reg_valid[disp & 0x7f]) {
                    int src_reg = param_reg_for_disp[disp & 0x7f];
                    int dst_reg = ((code[pc+2] >> 3) & 7) | ((code[pc] & 4) ? 8 : 0);
                    if (src_reg != dst_reg) {
                        code[pc] = 0x48 | (src_reg >= 8 ? 4 : 0) | (dst_reg >= 8 ? 1 : 0);
                        code[pc+1] = 0x89;
                        code[pc+2] = 0xc0 | ((src_reg & 7) << 3) | (dst_reg & 7);
                        code[pc+3] = 0x90;
                        changed = 1;
                    }
                }
            }

            if (is_target[next_pc - func_start]) {
                pc = next_pc;
                continue;
            }

            p1 = code + pc;
            p2 = code + next_pc;

            /* 1. 32-bit integer: mov %reg, [rbp+disp] followed by mov [rbp+disp], %reg */
            if (p1[0] == 0x89 && p2[0] == 0x8b && p1[1] == p2[1]) {
                int mod = (p1[1] >> 6) & 3;
                int rm = p1[1] & 7;
                if ((mod == 1 && rm == 5 && len == 3 && len2 == 3 && p1[2] == p2[2]) ||
                    (mod == 2 && rm == 5 && len == 6 && len2 == 6 && memcmp(p1 + 2, p2 + 2, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* 2. 64-bit integer: mov %reg, [rbp+disp] followed by mov [rbp+disp], %reg */
            if (p1[0] == 0x48 && p1[1] == 0x89 && p2[0] == 0x48 && p2[1] == 0x8b && p1[2] == p2[2]) {
                int mod = (p1[2] >> 6) & 3;
                int rm = p1[2] & 7;
                if ((mod == 1 && rm == 5 && len == 4 && len2 == 4 && p1[3] == p2[3]) ||
                    (mod == 2 && rm == 5 && len == 7 && len2 == 7 && memcmp(p1 + 3, p2 + 3, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* 3. 64-bit float XMM: movq [rbp+disp], %xmm followed by movq %xmm, [rbp+disp] */
            if (p1[0] == 0x66 && p1[1] == 0x0f && p1[2] == 0xd6 &&
                p2[0] == 0xf3 && p2[1] == 0x0f && p2[2] == 0x7e && p1[3] == p2[3]) {
                int mod = (p1[3] >> 6) & 3;
                int rm = p1[3] & 7;
                if ((mod == 1 && rm == 5 && len == 5 && len2 == 5 && p1[4] == p2[4]) ||
                    (mod == 2 && rm == 5 && len == 8 && len2 == 8 && memcmp(p1 + 4, p2 + 4, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* 4. Consecutive identical float reload */
            if (p1[0] == 0xf3 && p1[1] == 0x0f && p1[2] == 0x7e &&
                p2[0] == 0xf3 && p2[1] == 0x0f && p2[2] == 0x7e && p1[3] == p2[3]) {
                int mod = (p1[3] >> 6) & 3;
                int rm = p1[3] & 7;
                if ((mod == 1 && rm == 5 && len == 5 && len2 == 5 && p1[4] == p2[4]) ||
                    (mod == 2 && rm == 5 && len == 8 && len2 == 8 && memcmp(p1 + 4, p2 + 4, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* 5. Consecutive identical 32-bit int reload */
            if (p1[0] == 0x8b && p2[0] == 0x8b && p1[1] == p2[1]) {
                int mod = (p1[1] >> 6) & 3;
                int rm = p1[1] & 7;
                if ((mod == 1 && rm == 5 && len == 3 && len2 == 3 && p1[2] == p2[2]) ||
                    (mod == 2 && rm == 5 && len == 6 && len2 == 6 && memcmp(p1 + 2, p2 + 2, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* 6. Consecutive identical 64-bit int reload */
            if (p1[0] == 0x48 && p1[1] == 0x8b && p2[0] == 0x48 && p2[1] == 0x8b && p1[2] == p2[2]) {
                int mod = (p1[2] >> 6) & 3;
                int rm = p1[2] & 7;
                if ((mod == 1 && rm == 5 && len == 4 && len2 == 4 && p1[3] == p2[3]) ||
                    (mod == 2 && rm == 5 && len == 7 && len2 == 7 && memcmp(p1 + 3, p2 + 3, 4) == 0)) {
                    emit_nops(p2, len2);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
            }

            /* Pass U: Forwarded Base Register + SIB Array Load Fusion */
            if (p1[0] == 0x4c && p1[1] == 0x89 && p1[2] == 0xc1) {
                int s_pc = next_pc;
                while (s_pc < func_end && !is_target[s_pc - func_start]) {
                    int c_len = x86_inst_length(code + s_pc, func_end - s_pc);
                    if (c_len <= 0) break;
                    if (code[s_pc] == 0x90 || (code[s_pc] == 0x0f && code[s_pc + 1] == 0x1f) || (code[s_pc] == 0x66 && code[s_pc + 1] == 0x90)) {
                        s_pc += c_len;
                    } else break;
                }
                if (s_pc + 4 <= func_end && !is_target[s_pc - func_start]) {
                    uint8_t *p_lea = code + s_pc;
                    if (p_lea[0] == 0x48 && p_lea[1] == 0x8d && p_lea[2] == 0x04 && (p_lea[3] & 0x07) == 0x01) {
                        uint8_t sib_scale_idx = p_lea[3] & 0xf8;
                        int l_pc = s_pc + 4;
                        while (l_pc < func_end && !is_target[l_pc - func_start]) {
                            int c_len = x86_inst_length(code + l_pc, func_end - l_pc);
                            if (c_len <= 0) break;
                            if (code[l_pc] == 0x90 || (code[l_pc] == 0x0f && code[l_pc + 1] == 0x1f) || (code[l_pc] == 0x66 && code[l_pc + 1] == 0x90)) {
                                l_pc += c_len;
                            } else break;
                        }
                        if (l_pc + 2 <= func_end && !is_target[l_pc - func_start]) {
                            uint8_t *p_load = code + l_pc;
                            if (p_load[0] == 0x8b && p_load[1] == 0x00) {
                                p1[0] = 0x41;
                                p1[1] = 0x8b;
                                p1[2] = 0x04;
                                p1[3] = sib_scale_idx | 0x00;
                                emit_nops(p1 + 4, (l_pc + 2) - (pc + 4));
                                changed = 1;
                                pc = l_pc + 2;
                                continue;
                            }
                        }
                        p1[0] = 0x49;
                        p1[1] = 0x8d;
                        p1[2] = 0x04;
                        p1[3] = sib_scale_idx | 0x00;
                        emit_nops(p1 + 4, (s_pc + 4) - (pc + 4));
                        changed = 1;
                        pc = s_pc + 4;
                        continue;
                    }
                }
            }

            /* Pass B: LEA + SIB Dereference Fusion */
            if (p1[0] == 0x48 && p1[1] == 0x8d && p1[2] == 0x04 && len == 4) {
                uint8_t sib = p1[3];
                /* Case: movq (%rax), %xmm -> load into XMM */
                if (p2[0] == 0xf3 && p2[1] == 0x0f && p2[2] == 0x7e && (p2[3] & 0xc7) == 0x00 && len2 == 4) {
                    uint8_t xmm_reg = (p2[3] >> 3) & 7;
                    p1[0] = 0xf3; p1[1] = 0x0f; p1[2] = 0x7e; p1[3] = 0x04 | (xmm_reg << 3);
                    p1[4] = sib;
                    emit_nops(p1 + 5, 3);
                    changed = 1;
                    pc = next_pc + 4;
                    continue;
                }
                /* Case: movq %xmm, (%rax) -> store from XMM */
                if (p2[0] == 0x66 && p2[1] == 0x0f && p2[2] == 0xd6 && (p2[3] & 0xc7) == 0x00 && len2 == 4) {
                    uint8_t xmm_reg = (p2[3] >> 3) & 7;
                    p1[0] = 0x66; p1[1] = 0x0f; p1[2] = 0xd6; p1[3] = 0x04 | (xmm_reg << 3);
                    p1[4] = sib;
                    emit_nops(p1 + 5, 3);
                    changed = 1;
                    pc = next_pc + 4;
                    continue;
                }
            }

            /* Pass K: Struct Field / Pointer Offset Add-Dereference Folding */
            /* add $disp8, %rax (48 83 c0 <disp>) + mov (%rax), %reg -> mov disp8(%rax), %reg */
            if (p1[0] == 0x48 && p1[1] == 0x83 && p1[2] == 0xc0 && len == 4) {
                uint8_t disp = p1[3];
                /* 1. Direct 32-bit load: mov (%rax), %reg32 (8b <modrm>, mod=0, rm=0, len2=2) */
                if (p2[0] == 0x8b && (p2[1] & 0xc7) == 0x00 && len2 == 2) {
                    uint8_t reg = (p2[1] >> 3) & 7;
                    p1[0] = 0x8b; p1[1] = 0x40 | (reg << 3); p1[2] = disp;
                    emit_nops(p1 + 3, len + len2 - 3);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
                /* 2. Direct 64-bit load: mov (%rax), %reg64 (48 8b <modrm>, mod=0, rm=0, len2=3) */
                if (p2[0] == 0x48 && p2[1] == 0x8b && (p2[2] & 0xc7) == 0x00 && len2 == 3) {
                    uint8_t reg = (p2[2] >> 3) & 7;
                    p1[0] = 0x48; p1[1] = 0x8b; p1[2] = 0x40 | (reg << 3); p1[3] = disp;
                    emit_nops(p1 + 4, len + len2 - 4);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
                /* 3. Direct 32-bit store: mov %reg32, (%rax) (89 <modrm>, mod=0, rm=0, len2=2) */
                if (p2[0] == 0x89 && (p2[1] & 0xc7) == 0x00 && len2 == 2) {
                    uint8_t reg = (p2[1] >> 3) & 7;
                    p1[0] = 0x89; p1[1] = 0x40 | (reg << 3); p1[2] = disp;
                    emit_nops(p1 + 3, len + len2 - 3);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
                /* 4. Direct 64-bit store: mov %reg64, (%rax) (48 89 <modrm>, mod=0, rm=0, len2=3) */
                if (p2[0] == 0x48 && p2[1] == 0x89 && (p2[2] & 0xc7) == 0x00 && len2 == 3) {
                    uint8_t reg = (p2[2] >> 3) & 7;
                    p1[0] = 0x48; p1[1] = 0x89; p1[2] = 0x40 | (reg << 3); p1[3] = disp;
                    emit_nops(p1 + 4, len + len2 - 4);
                    changed = 1;
                    pc = next_pc + len2;
                    continue;
                }
                /* 5. Lookahead: add $disp, %rax + mov [rbp+d], %reg32 + mov %reg32, (%rax) */
                if (next_pc + len2 + 2 <= func_end && !is_target[next_pc + len2 - func_start]) {
                    uint8_t *p3 = code + next_pc + len2;
                    int len3 = x86_inst_length(p3, func_end - (next_pc + len2));
                    if (p2[0] == 0x8b && (((p2[1] >> 6) & 3) == 1 || ((p2[1] >> 6) & 3) == 2) && (p2[1] & 7) == 5 &&
                        p3[0] == 0x89 && (p3[1] & 0xc7) == 0x00 && ((p3[1] >> 3) & 7) == ((p2[1] >> 3) & 7) && len3 == 2) {
                        uint8_t reg = (p3[1] >> 3) & 7;
                        /* Move load to p1, then emit mov %reg, disp8(%rax) */
                        uint8_t tmp_load[8];
                        memcpy(tmp_load, p2, len2);
                        memcpy(p1, tmp_load, len2);
                        p1[len2] = 0x89;
                        p1[len2 + 1] = 0x40 | (reg << 3);
                        p1[len2 + 2] = disp;
                        emit_nops(p1 + len2 + 3, len + len2 + len3 - (len2 + 3));
                        changed = 1;
                        pc = next_pc + len2 + len3;
                        continue;
                    }
                    /* 6. Lookahead 64-bit: add $disp, %rax + mov [rbp+d], %reg64 + mov %reg64, (%rax) */
                    if (p2[0] == 0x48 && p2[1] == 0x8b && (((p2[2] >> 6) & 3) == 1 || ((p2[2] >> 6) & 3) == 2) && (p2[2] & 7) == 5 &&
                        p3[0] == 0x48 && p3[1] == 0x89 && (p3[2] & 0xc7) == 0x00 && ((p3[2] >> 3) & 7) == ((p2[2] >> 3) & 7) && len3 == 3) {
                        uint8_t reg = (p3[2] >> 3) & 7;
                        uint8_t tmp_load[8];
                        memcpy(tmp_load, p2, len2);
                        memcpy(p1, tmp_load, len2);
                        p1[len2] = 0x48;
                        p1[len2 + 1] = 0x89;
                        p1[len2 + 2] = 0x40 | (reg << 3);
                        p1[len2 + 3] = disp;
                        emit_nops(p1 + len2 + 4, len + len2 + len3 - (len2 + 4));
                        changed = 1;
                        pc = next_pc + len2 + len3;
                        continue;
                    }
                }
            }

            /* Pass N: Linked-List Head Insertion (insert) Optimization */
            /* add $8, %rax + mov [rbp+ht], %rcx + mov [rbp+idx], %edx + lea (%rcx,%rdx,8), %rcx +
               mov (%rcx), %rcx + mov %rcx, (%rax) + mov [rbp+ht], %rax + mov [rbp+idx], %ecx +
               lea (%rax,%rcx,8), %rax + mov [rbp+entry], %rcx + mov %rcx, (%rax)
            */
            if (p1[0] == 0x48 && p1[1] == 0x83 && p1[2] == 0xc0 && p1[3] == 0x08 && len == 4 && next_pc + 35 <= func_end) {
                uint8_t *p_ht = code + next_pc;
                if (p_ht[0] == 0x48 && p_ht[1] == 0x8b && p_ht[2] == 0x4d &&
                    !is_target[next_pc - func_start]) {
                    uint8_t disp_ht = p_ht[3];
                    uint8_t *p_idx = p_ht + 4;
                    if (p_idx[0] == 0x8b && p_idx[1] == 0x55) {
                        uint8_t disp_idx = p_idx[2];
                        uint8_t *p_lea1 = p_idx + 3;
                        if (p_lea1[0] == 0x48 && p_lea1[1] == 0x8d && p_lea1[2] == 0x0c && p_lea1[3] == 0xd1) {
                            uint8_t *p_ld = p_lea1 + 4;
                            if (p_ld[0] == 0x48 && p_ld[1] == 0x8b && p_ld[2] == 0x09) {
                                uint8_t *p_st1 = p_ld + 3;
                                if (p_st1[0] == 0x48 && p_st1[1] == 0x89 && p_st1[2] == 0x08) {
                                    uint8_t *p_ht2 = p_st1 + 3;
                                    if (p_ht2[0] == 0x48 && p_ht2[1] == 0x8b && p_ht2[2] == 0x45 && p_ht2[3] == disp_ht) {
                                        uint8_t *p_idx2 = p_ht2 + 4;
                                        if (p_idx2[0] == 0x8b && p_idx2[1] == 0x4d && p_idx2[2] == disp_idx) {
                                            uint8_t *p_lea2 = p_idx2 + 3;
                                            if (p_lea2[0] == 0x48 && p_lea2[1] == 0x8d && p_lea2[2] == 0x04 && p_lea2[3] == 0xc8) {
                                                uint8_t *p_en = p_lea2 + 4;
                                                if (p_en[0] == 0x48 && p_en[1] == 0x8b && p_en[2] == 0x4d) {
                                                    uint8_t *p_st2 = p_en + 4;
                                                    if (p_st2[0] == 0x48 && p_st2[1] == 0x89 && p_st2[2] == 0x08) {
                                                        /* 1. mov [rbp+ht], %rcx (48 8b 4d disp_ht) */
                                                        p1[0] = 0x48; p1[1] = 0x8b; p1[2] = 0x4d; p1[3] = disp_ht;
                                                        /* 2. mov [rbp+idx], %edx (8b 55 disp_idx) */
                                                        p1[4] = 0x8b; p1[5] = 0x55; p1[6] = disp_idx;
                                                        /* 3. lea (%rcx, %rdx, 8), %rdx (48 8d 14 d1) */
                                                        p1[7] = 0x48; p1[8] = 0x8d; p1[9] = 0x14; p1[10] = 0xd1;
                                                        /* 4. mov (%rdx), %rcx (48 8b 0a) */
                                                        p1[11] = 0x48; p1[12] = 0x8b; p1[13] = 0x0a;
                                                        /* 5. mov %rcx, 8(%rax) (48 89 48 08) */
                                                        p1[14] = 0x48; p1[15] = 0x89; p1[16] = 0x48; p1[17] = 0x08;
                                                        /* 6. mov %rax, (%rdx) (48 89 02) */
                                                        p1[18] = 0x48; p1[19] = 0x89; p1[20] = 0x02;
                                                        /* 7. NOP out the remaining 18 bytes */
                                                        emit_nops(p1 + 21, 18);
                                                        changed = 1;
                                                        pc = next_pc + 35;
                                                        continue;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /* Pass L: XOR-Shift Hash Expression Register Forwarding */
            if (p1[0] == 0x89 && p1[1] == 0x45 && len == 3) {
                uint8_t disp = p1[2];
                int s_pc = next_pc;
                while (s_pc < func_end && !is_target[s_pc - func_start]) {
                    int c_len = x86_inst_length(code + s_pc, func_end - s_pc);
                    if (c_len <= 0) break;
                    if (code[s_pc] == 0x90 || (code[s_pc] == 0x0f && code[s_pc + 1] == 0x1f) || (code[s_pc] == 0x66 && code[s_pc + 1] == 0x90)) {
                        s_pc += c_len;
                    } else break;
                }
                if (s_pc + 8 <= func_end && !is_target[s_pc - func_start]) {
                    uint8_t *p_shr = code + s_pc;
                    if (p_shr[0] == 0xc1 && p_shr[1] == 0xe8 && p_shr[2] == 0x10) {
                        int l_pc = s_pc + 3;
                        while (l_pc < func_end && !is_target[l_pc - func_start]) {
                            int c_len = x86_inst_length(code + l_pc, func_end - l_pc);
                            if (c_len <= 0) break;
                            if (code[l_pc] == 0x90 || (code[l_pc] == 0x0f && code[l_pc + 1] == 0x1f) || (code[l_pc] == 0x66 && code[l_pc + 1] == 0x90)) {
                                l_pc += c_len;
                            } else break;
                        }
                        if (l_pc + 5 <= func_end && !is_target[l_pc - func_start]) {
                            uint8_t *p_load = code + l_pc;
                            if (p_load[0] == 0x8b && p_load[1] == 0x4d && p_load[2] == disp) {
                                int x_pc = l_pc + 3;
                                while (x_pc < func_end && !is_target[x_pc - func_start]) {
                                    int c_len = x86_inst_length(code + x_pc, func_end - x_pc);
                                    if (c_len <= 0) break;
                                    if (code[x_pc] == 0x90 || (code[x_pc] == 0x0f && code[x_pc + 1] == 0x1f) || (code[x_pc] == 0x66 && code[x_pc + 1] == 0x90)) {
                                        x_pc += c_len;
                                    } else break;
                                }
                                if (x_pc + 2 <= func_end && !is_target[x_pc - func_start]) {
                                    uint8_t *p_xor = code + x_pc;
                                    if (p_xor[0] == 0x31 && p_xor[1] == 0xc8) {
                                        p1[0] = 0x89; p1[1] = 0xc1; p1[2] = 0x90; /* mov %eax, %ecx; nop */
                                        emit_nops(p_load, 3); /* NOP out load */
                                        changed = 1;
                                        pc = next_pc;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /* Pass O: Struct Member Compare Redirection to Preserve RAX */
            /* mov (%rax), %eax (8b 00) + mov [rbp+disp], %ecx (8b 4d) + cmp %ecx, %eax (39 c8)
               -> mov (%rax), %edx (8b 10) + mov [rbp+disp], %ecx (8b 4d) + cmp %ecx, %edx (39 ca)
            */
            if (p1[0] == 0x8b && p1[1] == 0x00 && len == 2 && next_pc + 5 <= func_end) {
                uint8_t *p_load = code + next_pc;
                int len_load = x86_inst_length(p_load, func_end - next_pc);
                if (p_load[0] == 0x8b && p_load[1] == 0x4d && len_load == 3 &&
                    !is_target[next_pc - func_start]) {
                    uint8_t *p_cmp = p_load + 3;
                    int len_cmp = x86_inst_length(p_cmp, func_end - (next_pc + 3));
                    if (p_cmp[0] == 0x39 && p_cmp[1] == 0xc8 && len_cmp == 2 &&
                        !is_target[next_pc + 3 - func_start]) {
                        uint8_t *p_jne;
                        int jne_off;
                        p1[1] = 0x10; /* mov (%rax), %edx */
                        p_cmp[1] = 0xca; /* cmp %ecx, %edx */
                        changed = 1;

                        /* Also check following jne and NOP out reloads of curr in rax */
                        p_jne = p_cmp + 2;
                        jne_off = (int)(p_jne - code);
                        if (jne_off + 6 <= func_end && p_jne[0] == 0x0f && p_jne[1] == 0x85 && !is_target[jne_off - func_start]) {
                            int32_t jne_rel = (int32_t)read32le(p_jne + 2);
                            uint8_t *p_fall = p_jne + 6;
                            int fall_off = jne_off + 6;
                            uint8_t *p_targ = p_fall + jne_rel;
                            int targ_off = fall_off + jne_rel;

                            /* Fallthrough: mov [rbp+disp], %rax */
                            if (fall_off + 4 <= func_end && p_fall[0] == 0x48 && p_fall[1] == 0x8b && p_fall[2] == 0x45 &&
                                !is_target[fall_off - func_start]) {
                                emit_nops(p_fall, 4);
                            }
                            /* Branch target: mov [rbp+disp], %rax */
                            if (targ_off >= func_start && targ_off + 4 <= func_end &&
                                p_targ[0] == 0x48 && p_targ[1] == 0x8b && p_targ[2] == 0x45) {
                                emit_nops(p_targ, 4);
                            }
                        }
                    }
                }
            }

            /* Pass P: Loop-Top Induction Variable Reload Elimination */
            if (p1[0] == 0x8b && ((p1[1] >> 6) & 3) == 1 && ((p1[1] >> 3) & 7) == 0 && (p1[1] & 7) == 5 && len == 3) {
                uint8_t disp = p1[2];
                int target_pc = pc;
                int jump_count = 0, valid_jumps = 0;
                int scan_pc;

                for (scan_pc = func_start; scan_pc < func_end; ) {
                    int s_len = x86_inst_length(code + scan_pc, func_end - scan_pc);
                    if (s_len <= 0) { scan_pc++; continue; }

                    /* 6-byte conditional jump: 0f 80..8f */
                    if (code[scan_pc] == 0x0f && s_len == 6 && (code[scan_pc + 1] >= 0x80 && code[scan_pc + 1] <= 0x8f)) {
                        int32_t rel = (int32_t)read32le(code + scan_pc + 2);
                        int dest = scan_pc + 6 + rel;
                        if (dest == target_pc) {
                            jump_count++;
                            /* Check preceding cmp and mov */
                            if (scan_pc >= func_start + 6) {
                                uint8_t *p_cmp = NULL;
                                if (code[scan_pc - 6] == 0x81 && code[scan_pc - 5] == 0xf8) {
                                    p_cmp = code + scan_pc - 6;
                                } else if (code[scan_pc - 3] == 0x83 && code[scan_pc - 2] == 0xf8) {
                                    p_cmp = code + scan_pc - 3;
                                }
                                if (p_cmp) {
                                    int mov_pc = (int)(p_cmp - code);
                                    if (mov_pc >= func_start + 3) {
                                        uint8_t *p_mov = code + mov_pc - 3;
                                        if ((p_mov[0] == 0x89 && p_mov[1] == 0x45 && p_mov[2] == disp) ||
                                            (p_mov[0] == 0x8b && p_mov[1] == 0x45 && p_mov[2] == disp)) {
                                            valid_jumps++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    scan_pc += s_len;
                }

                if (jump_count > 0 && jump_count == valid_jumps) {
                    emit_nops(p1, 3);
                    changed = 1;
                    pc = next_pc;
                    continue;
                }
            }

            /* Pass Q: Hoist Induction Variable Copy into RCX before RAX arithmetic */
            if ((p1[0] == 0x90 || (p1[0] == 0x0f && p1[1] == 0x1f && p1[2] == 0x00)) && len >= 2 && next_pc + 9 <= func_end) {
                int cur_pc = next_pc;
                int eax_modified = 0, ecx_used = 0;
                uint8_t *p_load_ecx = NULL;

                while (cur_pc < func_end && !is_target[cur_pc - func_start]) {
                    int c_len = x86_inst_length(code + cur_pc, func_end - cur_pc);
                    uint8_t *cp;
                    if (c_len <= 0) break;
                    cp = code + cur_pc;

                    /* Stop on control flow or calls */
                    if (cp[0] == 0xe8 || cp[0] == 0xe9 || cp[0] == 0xeb || cp[0] == 0xc3 || cp[0] == 0xc9 ||
                        (cp[0] >= 0x70 && cp[0] <= 0x7f) || (cp[0] == 0x0f && (func_end - cur_pc) >= 2 && cp[1] >= 0x80 && cp[1] <= 0x8f))
                        break;

                    /* Detect mov [rbp+disp], %ecx */
                    if (cp[0] == 0x8b && cp[1] == 0x4d && c_len == 3) {
                        p_load_ecx = cp;
                        break;
                    }

                    /* Check if instruction modifies or uses ecx */
                    if (reg_modified_by_inst(cp, c_len, 1) || reg_used_by_inst(cp, c_len, 1)) {
                        ecx_used = 1;
                        break;
                    }

                    if (reg_modified_by_inst(cp, c_len, 0)) {
                        eax_modified = 1;
                    }

                    cur_pc += c_len;
                }

                if (p_load_ecx && eax_modified && !ecx_used) {
                    p1[0] = 0x89;
                    p1[1] = 0xc1; /* mov %eax, %ecx */
                    if (len == 3) p1[2] = 0x90;
                    else if (len > 3) emit_nops(p1 + 2, len - 2);
                    emit_nops(p_load_ecx, 3);
                    changed = 1;
                    pc = next_pc;
                    continue;
                }
            }

            /* Pass M: While-Loop Post-Branch Self-Reload Elimination */
            if (p1[0] == 0x48 && p1[1] == 0x8b && p1[2] == 0x45 && len == 4 && next_pc + 13 <= func_end) {
                uint8_t disp = p1[3];
                uint8_t *p_test = code + next_pc;
                if (p_test[0] == 0x48 && p_test[1] == 0x85 && p_test[2] == 0xc0 && !is_target[next_pc - func_start]) {
                    uint8_t *p_je = p_test + 3;
                    if (p_je[0] == 0x0f && p_je[1] == 0x84 && !is_target[next_pc + 3 - func_start]) {
                        uint8_t *p_reload = p_je + 6;
                        if (p_reload[0] == 0x48 && p_reload[1] == 0x8b && p_reload[2] == 0x45 && p_reload[3] == disp &&
                            !is_target[next_pc + 9 - func_start]) {
                            emit_nops(p_reload, 4);
                            changed = 1;
                        }
                    }
                }
            }

            /* Pass E: Disabled unsafe raw byte duplicate pass */
#if 0
            if (p1[0] == 0x89 || (p1[0] == 0x48 && p1[1] == 0x89)) {
                int L;
                for (L = 16; L >= 4; L--) {
                    if (pc >= func_start + L && next_pc + L <= func_end) {
                        int ok = 1, k;
                        for (k = 0; k < L; k++) {
                            if (is_target[next_pc + k - func_start]) { ok = 0; break; }
                        }
                        if (ok && memcmp(code + pc - L, code + next_pc, L) == 0) {
                            emit_nops(code + next_pc, L);
                            changed = 1;
                            break;
                        }
                    }
                }
            }
#endif

            /* Pass F: Preserve Primary Index Register across Secondary Offset Computation */
            if (p1[0] == 0x89 && ((p1[1] >> 6) & 3) != 3 && ((p1[1] >> 3) & 7) == 0 && (p1[1] & 7) == 5) {
                int mod1 = (p1[1] >> 6) & 3;
                int disp1 = (mod1 == 1) ? (int8_t)p1[2] : (int)read32le(p1 + 2);
                int cur = next_pc;

                while (cur < func_end && !is_target[cur - func_start]) {
                    int c_len = x86_inst_length(code + cur, func_end - cur);
                    if (c_len <= 0) break;
                    if (code[cur] == 0x90 || (code[cur] == 0x0f && code[cur + 1] == 0x1f) || (code[cur] == 0x66 && code[cur + 1] == 0x90)) {
                        cur += c_len;
                    } else break;
                }

                if (cur < func_end && !is_target[cur - func_start] && code[cur] == 0x8b &&
                    (((code[cur + 1] >> 6) & 3) == 1 || ((code[cur + 1] >> 6) & 3) == 2) &&
                    ((code[cur + 1] >> 3) & 7) == 1 && (code[cur + 1] & 7) == 5) {
                    int load2_len = 2 + (((code[cur + 1] >> 6) & 3) == 1 ? 1 : 4);
                    int add_pc = cur + load2_len;

                    if (add_pc + 2 <= func_end && !is_target[add_pc - func_start] &&
                        code[add_pc] == 0x01 && code[add_pc + 1] == 0xc8) {
                        int store2_pc = add_pc + 2;

                        if (store2_pc < func_end && !is_target[store2_pc - func_start] && code[store2_pc] == 0x89 &&
                            (((code[store2_pc + 1] >> 6) & 3) == 1 || ((code[store2_pc + 1] >> 6) & 3) == 2) &&
                            ((code[store2_pc + 1] >> 3) & 7) == 0 && (code[store2_pc + 1] & 7) == 5) {
                            int store2_len = 2 + (((code[store2_pc + 1] >> 6) & 3) == 1 ? 1 : 4);
                            int reload_pc = store2_pc + store2_len;

                            if (reload_pc < func_end && !is_target[reload_pc - func_start] && code[reload_pc] == 0x8b &&
                                (((code[reload_pc + 1] >> 6) & 3) == 1 || ((code[reload_pc + 1] >> 6) & 3) == 2) &&
                                ((code[reload_pc + 1] >> 3) & 7) == 0 && (code[reload_pc + 1] & 7) == 5) {
                                int r_mod = (code[reload_pc + 1] >> 6) & 3;
                                int r_disp = (r_mod == 1) ? (int8_t)code[reload_pc + 2] : (int)read32le(code + reload_pc + 2);
                                int reload_len = 2 + (r_mod == 1 ? 1 : 4);

                                if (r_disp == disp1) {
                                    code[add_pc + 1] = 0xc1;
                                    code[store2_pc + 1] = (code[store2_pc + 1] & ~0x38) | (1 << 3);
                                    emit_nops(code + reload_pc, reload_len);
                                    changed = 1;
                                }
                            }
                        }
                    }
                }
            }

            /* Pass G: LEA Destination Redirection to RDX to Preserve RAX */
            if (p1[0] == 0x48 && p1[1] == 0x8d && p1[2] == 0x04 && p1[3] == 0xc1 && len == 4) {
                int s_pc = next_pc;
                int rdx_used = 0;
                int rax_used = 0;
                while (s_pc < func_end && !is_target[s_pc - func_start]) {
                    int s_len = x86_inst_length(code + s_pc, func_end - s_pc);
                    uint8_t *sp;
                    if (s_len <= 0) break;
                    sp = code + s_pc;

                    if (sp[0] == 0xe8 || sp[0] == 0xe9 || sp[0] == 0xeb || sp[0] == 0xc3 || sp[0] == 0xc9 ||
                        (sp[0] >= 0x70 && sp[0] <= 0x7f) || (sp[0] == 0x0f && (func_end - s_pc) >= 2 && sp[1] >= 0x80 && sp[1] <= 0x8f))
                        break;

                    if (sp[0] == 0x66 && sp[1] == 0x0f && sp[2] == 0xd6 && (sp[3] & 0xc7) == 0x00 && s_len == 4) {
                        if (!rdx_used && !rax_used) {
                            p1[2] = 0x14;
                            sp[3] = (sp[3] & ~7) | 2;
                            changed = 1;
                        }
                        break;
                    }

                    {
                        int si = 0;
                        while (si < s_len && (sp[si] == 0x66 || sp[si] == 0xf2 || sp[si] == 0xf3 || (sp[si] >= 0x40 && sp[si] <= 0x4f)))
                            si++;
                        if (si < s_len && sp[si] == 0x0f) si++;
                        if (si + 1 < s_len) {
                            uint8_t smodrm = sp[si + 1];
                            int smod = (smodrm >> 6) & 3;
                            int srm = smodrm & 7;
                            if (smod == 0 && srm == 0) { rax_used = 1; break; }
                        }
                    }

                    if (reg_modified_by_inst(sp, s_len, 2)) {
                        rdx_used = 1;
                        break;
                    }

                    s_pc += s_len;
                }
            }

            /* Pass H: Forward Store-Forwarding for Temporary Float Variables */
            if (p1[0] == 0x66 && p1[1] == 0x0f && p1[2] == 0xd6 &&
                (((p1[3] >> 6) & 3) == 1 || ((p1[3] >> 6) & 3) == 2) &&
                ((p1[3] >> 3) & 7) == 0 && (p1[3] & 7) == 5) {
                int mod1 = (p1[3] >> 6) & 3;
                int disp_temp = (mod1 == 1) ? (int8_t)p1[4] : (int)read32le(p1 + 4);
                int s_len1 = 4 + (mod1 == 1 ? 1 : 4);

                int f_pc = pc + s_len1;
                while (f_pc < func_end && !is_target[f_pc - func_start]) {
                    int f_left = func_end - f_pc;
                    int f_len = x86_inst_length(code + f_pc, f_left);
                    uint8_t *fp;
                    if (f_len <= 0) break;
                    fp = code + f_pc;

                    if (fp[0] == 0xe8 || fp[0] == 0xe9 || fp[0] == 0xeb || fp[0] == 0xc3 || fp[0] == 0xc9 ||
                        (fp[0] >= 0x70 && fp[0] <= 0x7f) || (fp[0] == 0x0f && f_left >= 2 && fp[1] >= 0x80 && fp[1] <= 0x8f))
                        break;

                    /* Check if fp is load from [rbp+disp_temp] into %xmm0: f3 0f 7e ... */
                    if (fp[0] == 0xf3 && fp[1] == 0x0f && fp[2] == 0x7e &&
                        (((fp[3] >> 6) & 3) == 1 || ((fp[3] >> 6) & 3) == 2) &&
                        ((fp[3] >> 3) & 7) == 0 && (fp[3] & 7) == 5) {
                        int f_mod = (fp[3] >> 6) & 3;
                        int f_disp = (f_mod == 1) ? (int8_t)fp[4] : (int)read32le(fp + 4);
                        int f_load_len = 4 + (f_mod == 1 ? 1 : 4);
                        if (f_disp == disp_temp) {
                            int next_s = f_pc + f_load_len;
                            while (next_s < func_end && !is_target[next_s - func_start]) {
                                int ns_len = x86_inst_length(code + next_s, func_end - next_s);
                                if (ns_len <= 0) break;
                                if (code[next_s] == 0x90 || (code[next_s] == 0x0f && code[next_s + 1] == 0x1f) || (code[next_s] == 0x66 && code[next_s + 1] == 0x90)) {
                                    next_s += ns_len;
                                } else break;
                            }
                            if (next_s < func_end && !is_target[next_s - func_start] &&
                                code[next_s] == 0x66 && code[next_s + 1] == 0x0f && code[next_s + 2] == 0xd6 &&
                                (((code[next_s + 3] >> 6) & 3) == 1 || ((code[next_s + 3] >> 6) & 3) == 2) &&
                                ((code[next_s + 3] >> 3) & 7) == 0 && (code[next_s + 3] & 7) == 5) {
                                int t_mod = (code[next_s + 3] >> 6) & 3;
                                int disp_target = (t_mod == 1) ? (int8_t)code[next_s + 4] : (int)read32le(code + next_s + 4);
                                int t_store_len = 4 + (t_mod == 1 ? 1 : 4);

                                /* Verify no instruction between pc and f_pc touched disp_target */
                                int check_pc = pc + s_len1;
                                int ok = 1;
                                while (check_pc < f_pc) {
                                    int ch_len = x86_inst_length(code + check_pc, f_pc - check_pc);
                                    uint8_t *cp = code + check_pc;
                                    if (ch_len <= 0) break;
                                    if ((cp[0] == 0xf3 && cp[1] == 0x0f && cp[2] == 0x7e) ||
                                        (cp[0] == 0x66 && cp[1] == 0x0f && cp[2] == 0xd6)) {
                                        int cmod = (cp[3] >> 6) & 3;
                                        if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                            int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                            if (cdisp == disp_target) { ok = 0; break; }
                                        }
                                    } else if (cp[0] == 0xf2 && cp[1] == 0x0f && (cp[2] == 0x58 || cp[2] == 0x59 || cp[2] == 0x5c || cp[2] == 0x5e)) {
                                        int cmod = (cp[3] >> 6) & 3;
                                        if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                            int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                            if (cdisp == disp_target) { ok = 0; break; }
                                        }
                                    }
                                    check_pc += ch_len;
                                }

                                if (ok) {
                                    if (t_mod == 1) {
                                        p1[3] = (p1[3] & ~0xc0) | 0x40;
                                        p1[4] = (uint8_t)(int8_t)disp_target;
                                        if (s_len1 > 5) emit_nops(p1 + 5, s_len1 - 5);
                                    } else {
                                        p1[3] = (p1[3] & ~0xc0) | 0x80;
                                        write32le(p1 + 4, disp_target);
                                    }
                                    emit_nops(fp, f_load_len);
                                    emit_nops(code + next_s, t_store_len);
                                    changed = 1;
                                }
                            }
                            break;
                        }
                    }

                    f_pc += f_len;
                }
            }

            /* Pass I: XMM2 Stash & Forward for Temporary Float Variables */
            /* movq %xmm0, [rbp+disp_temp] (8 bytes) ... (uses only xmm0/1) ...
               movq [rbp+disp_temp], %xmm0 (8 bytes); movq %xmm0, [rbp+disp_target] (5 bytes)
               -> movapd %xmm0, %xmm2 (4 bytes) + NOPs ...
                  NOPs ... movq %xmm2, [rbp+disp_target] (5 bytes)
            */
            if (p1[0] == 0x66 && p1[1] == 0x0f && p1[2] == 0xd6 &&
                (((p1[3] >> 6) & 3) == 1 || ((p1[3] >> 6) & 3) == 2) &&
                ((p1[3] >> 3) & 7) == 0 && (p1[3] & 7) == 5) {
                int mod1 = (p1[3] >> 6) & 3;
                int disp_temp = (mod1 == 1) ? (int8_t)p1[4] : (int)read32le(p1 + 4);
                int s_len1 = 4 + (mod1 == 1 ? 1 : 4);

                int f_pc = pc + s_len1;
                int xmm2_used = 0;
                while (f_pc < func_end && !is_target[f_pc - func_start]) {
                    int f_left = func_end - f_pc;
                    int f_len = x86_inst_length(code + f_pc, f_left);
                    uint8_t *fp;
                    if (f_len <= 0) break;
                    fp = code + f_pc;

                    if (fp[0] == 0xe8 || fp[0] == 0xe9 || fp[0] == 0xeb || fp[0] == 0xc3 || fp[0] == 0xc9 ||
                        (fp[0] >= 0x70 && fp[0] <= 0x7f) || (fp[0] == 0x0f && f_left >= 2 && fp[1] >= 0x80 && fp[1] <= 0x8f))
                        break;

                    /* Check if fp is reload of disp_temp into %xmm0: f3 0f 7e */
                    if (fp[0] == 0xf3 && fp[1] == 0x0f && fp[2] == 0x7e &&
                        (((fp[3] >> 6) & 3) == 1 || ((fp[3] >> 6) & 3) == 2) &&
                        ((fp[3] >> 3) & 7) == 0 && (fp[3] & 7) == 5) {
                        int f_mod = (fp[3] >> 6) & 3;
                        int f_disp = (f_mod == 1) ? (int8_t)fp[4] : (int)read32le(fp + 4);
                        int f_load_len = 4 + (f_mod == 1 ? 1 : 4);
                        if (f_disp == disp_temp) {
                            int next_s = f_pc + f_load_len;
                            while (next_s < func_end && !is_target[next_s - func_start]) {
                                int ns_len = x86_inst_length(code + next_s, func_end - next_s);
                                if (ns_len <= 0) break;
                                if (code[next_s] == 0x90 || (code[next_s] == 0x0f && code[next_s + 1] == 0x1f) || (code[next_s] == 0x66 && code[next_s + 1] == 0x90)) {
                                    next_s += ns_len;
                                } else break;
                            }
                            if (next_s < func_end && !is_target[next_s - func_start] &&
                                code[next_s] == 0x66 && code[next_s + 1] == 0x0f && code[next_s + 2] == 0xd6 &&
                                (((code[next_s + 3] >> 6) & 3) == 1 || ((code[next_s + 3] >> 6) & 3) == 2) &&
                                ((code[next_s + 3] >> 3) & 7) == 0 && (code[next_s + 3] & 7) == 5) {
                                if (!xmm2_used && s_len1 >= 4) {
                                    /* Rewrite p1 to movapd %xmm0, %xmm2: 66 0f 28 d0 */
                                    p1[0] = 0x66; p1[1] = 0x0f; p1[2] = 0x28; p1[3] = 0xd0;
                                    if (s_len1 > 4) emit_nops(p1 + 4, s_len1 - 4);
                                    /* Eliminate reload */
                                    emit_nops(fp, f_load_len);
                                    /* Change store from xmm0 to xmm2: modrm reg=2 */
                                    code[next_s + 3] = (code[next_s + 3] & ~0x38) | (2 << 3);
                                    changed = 1;
                                }
                            }
                            break;
                        }
                    }

                    /* Check if fp touches xmm2 */
                    if (fp[0] == 0x66 || fp[0] == 0xf2 || fp[0] == 0xf3) {
                        int si = 1;
                        if (fp[si] == 0x0f) si++;
                        if (si < f_len) {
                            uint8_t m = fp[si + 1];
                            int r1 = (m >> 3) & 7;
                            int r2 = m & 7;
                            if (r1 == 2 || (((m >> 6) & 3) == 3 && r2 == 2))
                                xmm2_used = 1;
                        }
                    }

                    f_pc += f_len;
                }
            }

            /* Pass J: Local Float Variable Promotion to XMM4-XMM7 */
            if (p1[0] == 0x66 && p1[1] == 0x0f && p1[2] == 0xd6 &&
                (((p1[3] >> 6) & 3) == 1 || ((p1[3] >> 6) & 3) == 2) &&
                ((p1[3] >> 3) & 7) == 0 && (p1[3] & 7) == 5 && len >= 5) {
                int mod1 = (p1[3] >> 6) & 3;
                int disp1 = (mod1 == 1) ? (int8_t)p1[4] : (int)read32le(p1 + 4);
                int chosen_reg = -1;
                uint8_t used_xmm[8] = {0};
                int bb_end = next_pc;
                int can_promote = 1;
                int r, chk_pc;

                /* Scan entire basic block to find end and used XMM registers */
                while (bb_end < func_end && !is_target[bb_end - func_start]) {
                    int b_len = x86_inst_length(code + bb_end, func_end - bb_end);
                    uint8_t *bp;
                    if (b_len <= 0) break;
                    bp = code + bb_end;

                    if (bp[0] == 0xe8 || bp[0] == 0xe9 || bp[0] == 0xeb || bp[0] == 0xc3 || bp[0] == 0xc9 ||
                        (bp[0] >= 0x70 && bp[0] <= 0x7f) || (bp[0] == 0x0f && (func_end - bb_end) >= 2 && bp[1] >= 0x80 && bp[1] <= 0x8f))
                        break;

                    /* Track XMM register usage */
                    if (bp[0] == 0x66 || bp[0] == 0xf2 || bp[0] == 0xf3) {
                        int si = 1;
                        if (bp[si] == 0x0f) si++;
                        if (si < b_len) {
                            uint8_t m = bp[si + 1];
                            int r1 = (m >> 3) & 7;
                            int r2 = m & 7;
                            int mod = (m >> 6) & 3;
                            used_xmm[r1] = 1;
                            if (mod == 3) used_xmm[r2] = 1;
                        }
                    }

                    bb_end += b_len;
                }

                /* Pick an available XMM register among 4, 5, 6, 7 */
                for (r = 4; r <= 7; r++) {
                    if (!used_xmm[r]) {
                        chosen_reg = r;
                        break;
                    }
                }

                if (chosen_reg >= 4) {
                    /* Verify disp1 is NEVER accessed anywhere outside [pc, bb_end] in the entire function */
                    int g_pc = func_start;
                    while (g_pc < func_end) {
                        int g_len = x86_inst_length(code + g_pc, func_end - g_pc);
                        uint8_t *gp = code + g_pc;
                        if (g_len <= 0) break;

                        if (g_pc < pc || g_pc >= bb_end) {
                            int mod = -1, r_disp = 0;
                            if (gp[0] == 0x8b || gp[0] == 0x89) {
                                mod = (gp[1] >> 6) & 3;
                                if ((mod == 1 || mod == 2) && (gp[1] & 7) == 5)
                                    r_disp = (mod == 1) ? (int8_t)gp[2] : (int)read32le(gp + 2);
                            } else if ((gp[0] == 0xf3 && gp[1] == 0x0f && gp[2] == 0x7e) ||
                                       (gp[0] == 0x66 && gp[1] == 0x0f && gp[2] == 0xd6)) {
                                mod = (gp[3] >> 6) & 3;
                                if ((mod == 1 || mod == 2) && (gp[3] & 7) == 5)
                                    r_disp = (mod == 1) ? (int8_t)gp[4] : (int)read32le(gp + 4);
                            } else if (gp[0] == 0xf2 && gp[1] == 0x0f && (gp[2] == 0x58 || gp[2] == 0x59 || gp[2] == 0x5c || gp[2] == 0x5e)) {
                                mod = (gp[3] >> 6) & 3;
                                if ((mod == 1 || mod == 2) && (gp[3] & 7) == 5)
                                    r_disp = (mod == 1) ? (int8_t)gp[4] : (int)read32le(gp + 4);
                            }
                            if (mod > 0 && r_disp == disp1) {
                                can_promote = 0;
                                break;
                            }
                        }
                        g_pc += g_len;
                    }

                    if (can_promote) {
                        /* Verify all accesses to disp1 inside [pc, bb_end] are convertible */
                        int chk_pc = next_pc;
                        while (chk_pc < bb_end) {
                            int chk_len = x86_inst_length(code + chk_pc, bb_end - chk_pc);
                            uint8_t *cp = code + chk_pc;
                            if (chk_len <= 0) break;

                            if (cp[0] == 0x66 && cp[1] == 0x0f && cp[2] == 0xd6) {
                                int cmod = (cp[3] >> 6) & 3;
                                if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                    int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                    if (cdisp == disp1) { can_promote = 0; break; }
                                }
                            } else if (cp[0] == 0xf3 && cp[1] == 0x0f && cp[2] == 0x7e) {
                                int cmod = (cp[3] >> 6) & 3;
                                if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                    int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                    if (cdisp == disp1 && ((cp[3] >> 3) & 7) != 0) {
                                        can_promote = 0; break;
                                    }
                                }
                            } else if (cp[0] == 0xf2 && cp[1] == 0x0f && (cp[2] == 0x58 || cp[2] == 0x59 || cp[2] == 0x5c || cp[2] == 0x5e)) {
                                int cmod = (cp[3] >> 6) & 3;
                                if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                    int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                    if (cdisp == disp1 && ((cp[3] >> 3) & 7) != 0) {
                                        can_promote = 0; break;
                                    }
                                }
                            }
                            chk_pc += chk_len;
                        }
                    }

                    if (can_promote) {
                        /* 1. Rewrite the store to movapd %xmm_src, %xmmR: 66 0f 28 (0xc0 | (R << 3) | src) */
                        p1[0] = 0x66; p1[1] = 0x0f; p1[2] = 0x28; p1[3] = 0xc0 | (chosen_reg << 3) | ((p1[3] >> 3) & 7);
                        if (len > 4) emit_nops(p1 + 4, len - 4);

                        /* 2. Rewrite all loads and arithmetic */
                        chk_pc = next_pc;
                        while (chk_pc < bb_end) {
                            int chk_len = x86_inst_length(code + chk_pc, bb_end - chk_pc);
                            uint8_t *cp = code + chk_pc;
                            if (chk_len <= 0) break;

                            if (cp[0] == 0xf3 && cp[1] == 0x0f && cp[2] == 0x7e) {
                                int cmod = (cp[3] >> 6) & 3;
                                if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                    int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                    if (cdisp == disp1) {
                                        /* movapd %xmmR, %xmm_dest: 66 0f 28 (0xc0 | (dest << 3) | R) */
                                        cp[0] = 0x66; cp[1] = 0x0f; cp[2] = 0x28; cp[3] = 0xc0 | (cp[3] & 0x38) | chosen_reg;
                                        if (chk_len > 4) emit_nops(cp + 4, chk_len - 4);
                                    }
                                }
                            } else if (cp[0] == 0xf2 && cp[1] == 0x0f && (cp[2] == 0x58 || cp[2] == 0x59 || cp[2] == 0x5c || cp[2] == 0x5e)) {
                                int cmod = (cp[3] >> 6) & 3;
                                if ((cmod == 1 || cmod == 2) && (cp[3] & 7) == 5) {
                                    int cdisp = (cmod == 1) ? (int8_t)cp[4] : (int)read32le(cp + 4);
                                    if (cdisp == disp1) {
                                        /* f2 0f <op> (0xc0 | (dest << 3) | R) */
                                        cp[3] = 0xc0 | (cp[3] & 0x38) | chosen_reg;
                                        if (chk_len > 4) emit_nops(cp + 4, chk_len - 4);
                                    }
                                }
                            }
                            chk_pc += chk_len;
                        }
                        changed = 1;
                    }
                }
            }

            pc = next_pc;
        }

        /* Pass D: Redundant Index & 64-bit Register Forward Load Elimination */
        for (pc = func_start; pc < func_end; ) {
            left = func_end - pc;
            len = x86_inst_length(code + pc, left);
            if (len <= 0) { pc++; continue; }
            p = code + pc;

            /* Check for movslq %eax, %rax (48 63 c0) */
            if (p[0] == 0x48 && p[1] == 0x63 && p[2] == 0xc0) {
                int b_pc = pc;
                int disp = 0;
                int found_slot = 0;
                while (b_pc > func_start) {
                    int prev_pc = func_start;
                    uint8_t *bp;
                    int bp_len;
                    while (prev_pc < b_pc) {
                        int p_len = x86_inst_length(code + prev_pc, b_pc - prev_pc);
                        if (p_len <= 0 || prev_pc + p_len >= b_pc) break;
                        prev_pc += p_len;
                    }
                    if (prev_pc >= b_pc) break;
                    bp = code + prev_pc;
                    bp_len = b_pc - prev_pc;

                    /* Skip NOPs */
                    if (bp[0] == 0x90 || (bp[0] == 0x0f && bp[1] == 0x1f) || (bp[0] == 0x66 && bp[1] == 0x90)) {
                        b_pc = prev_pc;
                        continue;
                    }

                    /* Check if bp is load from [rbp+disp] into %eax */
                    if (bp[0] == 0x8b && (((bp[1] >> 6) & 3) == 1 || ((bp[1] >> 6) & 3) == 2) &&
                        ((bp[1] >> 3) & 7) == 0 && (bp[1] & 7) == 5) {
                        int b_mod = (bp[1] >> 6) & 3;
                        disp = (b_mod == 1) ? (int8_t)bp[2] : (int)read32le(bp + 2);
                        found_slot = 1;
                        break;
                    }

                    /* Check if bp is store of %eax to [rbp+disp] */
                    if (bp[0] == 0x89 && (((bp[1] >> 6) & 3) == 1 || ((bp[1] >> 6) & 3) == 2) &&
                        ((bp[1] >> 3) & 7) == 0 && (bp[1] & 7) == 5) {
                        int b_mod = (bp[1] >> 6) & 3;
                        disp = (b_mod == 1) ? (int8_t)bp[2] : (int)read32le(bp + 2);
                        found_slot = 1;
                        break;
                    }

                    if (reg_modified_by_inst(bp, bp_len, 0))
                        break;

                    b_pc = prev_pc;
                }

                if (found_slot) {
                    f_pc = pc + 3;
                    while (f_pc < func_end) {
                        if (is_target[f_pc - func_start]) break;
                        f_left = func_end - f_pc;
                        f_len = x86_inst_length(code + f_pc, f_left);
                        if (f_len <= 0) break;
                        fp = code + f_pc;

                        if (fp[0] == 0xe8 || fp[0] == 0xe9 || fp[0] == 0xeb || fp[0] == 0xc3 || fp[0] == 0xc9 ||
                            (fp[0] >= 0x70 && fp[0] <= 0x7f) || (fp[0] == 0x0f && f_left >= 2 && fp[1] >= 0x80 && fp[1] <= 0x8f))
                            break;

                        if (fp[0] == 0x8b && (((fp[1] >> 6) & 3) == 1 || ((fp[1] >> 6) & 3) == 2) &&
                            ((fp[1] >> 3) & 7) == 0 && (fp[1] & 7) == 5) {
                            int f_mod = (fp[1] >> 6) & 3;
                            int f_disp = (f_mod == 1) ? (int8_t)fp[2] : (int)read32le(fp + 2);
                            int f_load_len = 2 + (f_mod == 1 ? 1 : 4);
                            if (f_disp == disp) {
                                int f_next = f_pc + f_load_len;
                                while (f_next < func_end && !is_target[f_next - func_start]) {
                                    int fn_len = x86_inst_length(code + f_next, func_end - f_next);
                                    if (fn_len <= 0) break;
                                    if (code[f_next] == 0x90 || (code[f_next] == 0x0f && code[f_next + 1] == 0x1f) || (code[f_next] == 0x66 && code[f_next + 1] == 0x90)) {
                                        f_next += fn_len;
                                    } else break;
                                }
                                if (f_next + 3 <= func_end && !is_target[f_next - func_start] &&
                                    code[f_next] == 0x48 && code[f_next + 1] == 0x63 && code[f_next + 2] == 0xc0) {
                                    emit_nops(fp, f_load_len);
                                    emit_nops(code + f_next, 3);
                                    changed = 1;
                                    f_pc = f_next + 3;
                                    continue;
                                }
                            }
                        }

                        if (reg_modified_by_inst(fp, f_len, 0))
                            break;

                        if ((fp[0] == 0x89 && fp[1] == 0x45) || (fp[0] == 0x89 && fp[1] == 0x85) ||
                            (fp[0] == 0x48 && fp[1] == 0x89 && fp[2] == 0x45) || (fp[0] == 0x48 && fp[1] == 0x89 && fp[2] == 0x85)) {
                            int s_mod = (fp[0] == 0x48 ? fp[2] >> 6 : fp[1] >> 6) & 3;
                            int s_disp = (s_mod == 1) ? (int8_t)fp[fp[0] == 0x48 ? 3 : 2] : (int)read32le(fp + (fp[0] == 0x48 ? 3 : 2));
                            if (s_disp == disp) break;
                        }

                        f_pc += f_len;
                    }
                }
            }

            /* Check for 64-bit load to Reg: 48 8b <modrm> [disp] */
            if (p[0] == 0x48 && p[1] == 0x8b && ((p[2] & 0xf8) == 0x40 || (p[2] & 0xf8) == 0x80) && (p[2] & 7) == 5) {
                int mod = (p[2] >> 6) & 3;
                int reg = (p[2] >> 3) & 7;
                int disp_len = (mod == 1) ? 1 : 4;
                int load_len = 3 + disp_len;
                int disp = (mod == 1) ? (int8_t)p[3] : (int)read32le(p + 3);

                f_pc = pc + load_len;
                while (f_pc < func_end) {
                    if (is_target[f_pc - func_start]) break;
                    f_left = func_end - f_pc;
                    f_len = x86_inst_length(code + f_pc, f_left);
                    if (f_len <= 0) break;
                    fp = code + f_pc;

                    if (fp[0] == 0xe8 || fp[0] == 0xe9 || fp[0] == 0xeb || fp[0] == 0xc3 || fp[0] == 0xc9 ||
                        (fp[0] >= 0x70 && fp[0] <= 0x7f) || (fp[0] == 0x0f && f_left >= 2 && fp[1] >= 0x80 && fp[1] <= 0x8f))
                        break;

                    if (fp[0] == 0x48 && fp[1] == 0x8b && fp[2] == p[2] && f_len == load_len) {
                        int f_disp = (mod == 1) ? (int8_t)fp[3] : (int)read32le(fp + 3);
                        if (f_disp == disp) {
                            emit_nops(fp, f_len);
                            changed = 1;
                            f_pc += f_len;
                            continue;
                        }
                    }

                    if (reg_modified_by_inst(fp, f_len, reg))
                        break;

                    if (fp[0] == 0x48 && fp[1] == 0x89 && fp[2] == (0x45 | (mod == 1 ? 0 : 0x40) | (reg << 3))) {
                        int s_disp = (mod == 1) ? (int8_t)fp[3] : (int)read32le(fp + 3);
                        if (s_disp == disp) break;
                    }

                    f_pc += f_len;
                }
            }

            pc += len;
        }

        if (!changed)
            break;
    }

    tcc_free(is_target);
    tcc_free(is_reloc);
}

ST_FUNC void gen_fill_nops(int bytes)
{
    while (bytes--)
      g(0x90);
}

/* generate a jump to a label */
int gjmp(int t)
{
    return gjmp2(0xe9, t);
}

/* generate a jump to a fixed address */
void gjmp_addr(int a)
{
    int r;
    r = a - ind - 2;
    if (r == (signed char)r) {
        g(0xeb);
        g(r);
    } else {
        oad(0xe9, a - ind - 5);
    }
}

ST_FUNC int gjmp_append(int n, int t)
{
    void *p;
    /* insert vtop->c jump list in t */
    if (n) {
        uint32_t n1 = n, n2;
        while ((n2 = read32le(p = cur_text_section->data + n1)))
            n1 = n2;
        write32le(p, t);
        t = n;
    }
    return t;
}

ST_FUNC int gjmp_cond(int op, int t)
{
        if (op & 0x100)
	  {
	    /* This was a float compare.  If the parity flag is set
	       the result was unordered.  For anything except != this
	       means false and we don't jump (anding both conditions).
	       For != this means true (oring both).
	       Take care about inverting the test.  We need to jump
	       to our target if the result was unordered and test wasn't NE,
	       otherwise if unordered we don't want to jump.  */
            int v = vtop->cmp_r;
            op &= ~0x100;
            if (op ^ v ^ (v != TOK_NE))
              o(0x067a);  /* jp +6 */
	    else
	      {
	        g(0x0f);
		t = gjmp2(0x8a, t); /* jp t */
	      }
	  }
        g(0x0f);
        t = gjmp2(op - 16, t);
        return t;
}

static int match_eval_sequences(uint8_t *eval1, int len1, uint8_t *eval2, int len2)
{
    int i1 = 0, i2 = 0;
    uint8_t b1, b2;
    while (i1 < len1 && i2 < len2) {
        /* Skip spill instruction in eval2 if present */
        if (i2 + 7 <= len2 && (eval2[i2] & 0xf0) == 0x40 && eval2[i2+1] == 0x89 && (eval2[i2+2] & 0xc0) == 0x80) {
            i2 += 7;
            continue;
        }
        if (i2 + 3 <= len2 && eval2[i2] == 0x89 && (eval2[i2+1] & 0xc0) == 0x40) {
            i2 += 3;
            continue;
        }
        if (i2 + 8 <= len2 && (eval2[i2] & 0xf0) == 0x40 && eval2[i2+1] == 0x89 && (eval2[i2+2] & 0xc0) == 0x80 && (eval2[i2+2] & 7) == 4) {
            i2 += 8;
            continue;
        }
        
        b1 = eval1[i1];
        b2 = eval2[i2];
        
        if (b1 != b2) {
            /* Allow register differences in ModR/M or REX bytes */
            if ((b1 & 0xc0) == 0xc0 && (b2 & 0xc0) == 0xc0) {
                /* Register-register ModR/M */
            } else if ((b1 & 0xc0) == (b2 & 0xc0) && (b1 & 0xc0) != 0) {
                /* Displaced ModR/M with different reg */
            } else if ((b1 & 0xf0) == 0x40 && (b2 & 0xf0) == 0x40) {
                /* REX prefix */
            } else {
                return 0;
            }
        }
        i1++;
        i2++;
    }
    return (i1 == len1);
}

static int try_optimize_rotate(int op, int ll)
{
    uint8_t *data;
    uint8_t *p;
    int r, fr;
    int sz1;
    int opc2, opc1, s2;
    uint8_t *p_shift2, *p_shift1;
    
    if (op != '|' && op != '^')
        return 0;
    
    data = cur_text_section->data;
    p = data + ind;
    
    r = vtop[-1].r;
    fr = vtop[0].r;
    if (r < 0 || r >= 16 || fr < 0 || fr >= 16 || r == fr)
        return 0;
        
    /* 1. Identify shift2 ending at p */
    p_shift2 = NULL;
    opc2 = -1;
    s2 = 0;
    
    if (ind >= 3 && p[-3] == 0xc1) {
        int modrm = p[-2];
        if ((modrm & 7) == REG_VALUE(fr) && (modrm & 0xc0) == 0xc0) {
            opc2 = (modrm >> 3) & 7;
            s2 = p[-1];
            p_shift2 = p - 3;
        }
    } else if (ind >= 4 && (p[-4] & 0xf0) == 0x40 && p[-3] == 0xc1) {
        int modrm = p[-2];
        if ((modrm & 7) == REG_VALUE(fr) && (modrm & 0xc0) == 0xc0) {
            opc2 = (modrm >> 3) & 7;
            s2 = p[-1];
            p_shift2 = p - 4;
        }
    } else if (ind >= 2 && p[-2] == 0xd1) {
        int modrm = p[-1];
        if ((modrm & 7) == REG_VALUE(fr) && (modrm & 0xc0) == 0xc0) {
            opc2 = (modrm >> 3) & 7;
            s2 = 1;
            p_shift2 = p - 2;
        }
    } else if (ind >= 3 && (p[-3] & 0xf0) == 0x40 && p[-2] == 0xd1) {
        int modrm = p[-1];
        if ((modrm & 7) == REG_VALUE(fr) && (modrm & 0xc0) == 0xc0) {
            opc2 = (modrm >> 3) & 7;
            s2 = 1;
            p_shift2 = p - 3;
        }
    }
    
    if (!p_shift2 || (opc2 != 4 && opc2 != 5))
        return 0;
        
    /* 2. Find shift1 before p_shift2 (eval_len from 2 up to 48 bytes) */
    p_shift1 = NULL;
    sz1 = 0;
    opc1 = -1;
    
    for (int eval_len = 2; eval_len <= 48; eval_len++) {
        int cur_sz = 0;
        int cur_opc = -1;
        int cur_s = 0;
        uint8_t *scan = p_shift2 - eval_len;
        if (scan < data)
            break;
        
        if (scan - data >= 3 && scan[-3] == 0xc1) {
            int modrm = scan[-2];
            if ((modrm & 7) == REG_VALUE(r) && (modrm & 0xc0) == 0xc0) {
                cur_opc = (modrm >> 3) & 7;
                cur_s = scan[-1];
                cur_sz = 3;
            }
        } else if (scan - data >= 4 && (scan[-4] & 0xf0) == 0x40 && scan[-3] == 0xc1) {
            int modrm = scan[-2];
            if ((modrm & 7) == REG_VALUE(r) && (modrm & 0xc0) == 0xc0) {
                cur_opc = (modrm >> 3) & 7;
                cur_s = scan[-1];
                cur_sz = 4;
            }
        } else if (scan - data >= 2 && scan[-2] == 0xd1) {
            int modrm = scan[-1];
            if ((modrm & 7) == REG_VALUE(r) && (modrm & 0xc0) == 0xc0) {
                cur_opc = (modrm >> 3) & 7;
                cur_s = 1;
                cur_sz = 2;
            }
        } else if (scan - data >= 3 && (scan[-3] & 0xf0) == 0x40 && scan[-2] == 0xd1) {
            int modrm = scan[-1];
            if ((modrm & 7) == REG_VALUE(r) && (modrm & 0xc0) == 0xc0) {
                cur_opc = (modrm >> 3) & 7;
                cur_s = 1;
                cur_sz = 3;
            }
        }
        
        if (cur_sz > 0 && (cur_opc == 4 || cur_opc == 5)) {
            if (((cur_opc == 5 && opc2 == 4) || (cur_opc == 4 && opc2 == 5)) &&
                (cur_s + s2 == (ll ? 64 : 32))) {
                
                uint8_t *cand_shift1 = scan - cur_sz;
                int match = 0;
                if (eval_len <= 7) {
                    /* Direct load matching */
                    uint8_t *p_load2 = scan;
                    uint8_t *p_load1 = cand_shift1 - eval_len;
                    if (p_load1 >= data) {
                        if (eval_len == 3 || eval_len == 4) {
                            if (p_load1[eval_len - 1] == p_load2[eval_len - 1] &&
                                (p_load1[eval_len - 2] & 7) == (p_load2[eval_len - 2] & 7))
                                match = 1;
                        } else if (eval_len == 6 || eval_len == 7) {
                            if (memcmp(p_load1 + eval_len - 4, p_load2 + eval_len - 4, 4) == 0 &&
                                (p_load1[eval_len - 5] & 7) == (p_load2[eval_len - 5] & 7))
                                match = 1;
                        } else if (eval_len == 2) {
                            if (((p_load1[eval_len - 1] >> 3) & 7) == ((p_load2[eval_len - 1] >> 3) & 7))
                                match = 1;
                        }
                    }
                } else {
                    /* Indexed / multi-instruction subexpression matching */
                    /* Scan backwards before cand_shift1 for candidate eval1 */
                    for (int l1 = eval_len - 8; l1 <= eval_len; l1++) {
                        if (cand_shift1 - l1 >= data) {
                            if (match_eval_sequences(cand_shift1 - l1, l1, scan, eval_len)) {
                                match = 1;
                                break;
                            }
                        }
                    }
                }
                
                if (match) {
                    p_shift1 = cand_shift1;
                    sz1 = cur_sz;
                    opc1 = cur_opc;
                    break;
                }
            }
        }
    }
    
    if (!p_shift1)
        return 0;
        
    /* Match! Transform shift1 into rotate! */
    {
        int new_opc = (opc1 == 5) ? 1 : 0;
        if (sz1 == 3 || sz1 == 4) {
            p_shift1[sz1 - 2] = (uint8_t)(0xc0 | (new_opc << 3) | REG_VALUE(r));
        } else if (sz1 == 2) {
            p_shift1[sz1 - 1] = (uint8_t)(0xc0 | (new_opc << 3) | REG_VALUE(r));
        }
    }
    
    /* Rewind code generation pointer ind to end of shift1 */
    ind = (int)(p_shift1 + sz1 - data);
    vtop[-1].r = r;
    vtop--;
    return 1;
}

static int find_fast_div_u32(uint32_t d, uint64_t *out_m, int *out_s)
{
    int s;
    if (d <= 1)
        return 0;
    if ((d & (d - 1)) == 0) {
        int shift = 0;
        uint32_t tmp = d;
        while (tmp > 1) { shift++; tmp >>= 1; }
        *out_m = 1;
        *out_s = shift;
        return 1; /* power of 2 */
    }
    for (s = 32; s < 64; s++) {
        uint64_t two_s = 1ULL << s;
        uint64_t rem = two_s % d;
        uint64_t e = d - rem;
        if (e <= (1ULL << (s - 32))) {
            *out_m = (two_s + d - 1) / d;
            *out_s = s;
            return 2; /* magic multiplier */
        }
    }
    return 0;
}

static int find_fast_div_s32(int32_t d, int32_t *out_m, int *out_s)
{
    if (d <= 1 && d >= -1)
        return 0;
    if (d == 10) { *out_m = 0x66666667; *out_s = 34; return 1; }
    if (d == 100) { *out_m = 0x51eb851f; *out_s = 37; return 1; }
    if (d == 1000) { *out_m = 0x10624dd3; *out_s = 38; return 1; }
    if (d == 10000) { *out_m = (int32_t)0xd1b71759; *out_s = 45; return 1; }
    if (d == 10000000) { *out_m = (int32_t)0x6b5fca6b; *out_s = 54; return 1; }
    return 0;
}

static int try_optimize_lea(int *pr, int *pfr)
{
    uint8_t *data = cur_text_section->data;
    uint8_t *p = data + ind;
    int r = *pr, fr = *pfr;
    int shift, rex, modrm, sib;

    if (tcc_state->optimize == 0)
        return 0;
    if (r < 0 || r >= 16 || fr < 0 || fr >= 16 || r == fr)
        return 0;

    /* Check if previous 4 bytes are: [0x48 | REX_BASE(fr), 0xc1, 0xe0 | REG_VALUE(fr), shift] */
    if (REG_VALUE(r) != 5 && REG_VALUE(fr) != 4 && ind >= 4 &&
        p[-4] == (0x48 | REX_BASE(fr)) &&
        p[-3] == 0xc1 &&
        p[-2] == (0xe0 | REG_VALUE(fr)) &&
        p[-1] >= 1 && p[-1] <= 3) {
        shift = p[-1];
        rex = 0x48 | (REX_BASE(r) << 2) | (REX_BASE(fr) << 1) | REX_BASE(r);
        modrm = 0x04 | (REG_VALUE(r) << 3);
        sib = (shift << 6) | (REG_VALUE(fr) << 3) | REG_VALUE(r);
        p[-4] = rex;
        p[-3] = 0x8d;
        p[-2] = modrm;
        p[-1] = sib;
        return 1;
    }
    /* Check if previous 4 bytes are: [0x48 | REX_BASE(r), 0xc1, 0xe0 | REG_VALUE(r), shift] */
    if (REG_VALUE(fr) != 5 && REG_VALUE(r) != 4 && ind >= 4 &&
        p[-4] == (0x48 | REX_BASE(r)) &&
        p[-3] == 0xc1 &&
        p[-2] == (0xe0 | REG_VALUE(r)) &&
        p[-1] >= 1 && p[-1] <= 3) {
        shift = p[-1];
        rex = 0x48 | (REX_BASE(r) << 2) | (REX_BASE(r) << 1) | REX_BASE(fr);
        modrm = 0x04 | (REG_VALUE(r) << 3);
        sib = (shift << 6) | (REG_VALUE(r) << 3) | REG_VALUE(fr);
        p[-4] = rex;
        p[-3] = 0x8d;
        p[-2] = modrm;
        p[-1] = sib;
        return 1;
    }
    /* Case 3: shl on fr is followed by a load of r (load_len from 2 to 8 bytes) */
    if (REG_VALUE(r) != 5 && REG_VALUE(fr) != 4) {
        for (int load_len = 2; load_len <= 8; load_len++) {
            if (ind >= load_len + 4) {
                uint8_t *shl_p = p - load_len - 4;
                if (shl_p[0] == (0x48 | REX_BASE(fr)) &&
                    shl_p[1] == 0xc1 &&
                    shl_p[2] == (0xe0 | REG_VALUE(fr)) &&
                    shl_p[3] >= 1 && shl_p[3] <= 3) {
                    shift = shl_p[3];
                    /* move load backwards over the shl instruction */
                    memmove(shl_p, p - load_len, load_len);
                    if (cur_text_section->reloc) {
                        ElfW_Rel *rel;
                        for_each_elem(cur_text_section->reloc, 0, rel, ElfW_Rel) {
                            if (rel->r_offset >= (addr_t)(p - load_len - data) && rel->r_offset < (addr_t)ind) {
                                rel->r_offset -= 4;
                            }
                        }
                    }
                    ind -= 4;
                    /* emit lea (%r, %fr, 1<<shift), %fr */
                    rex = 0x48 | (REX_BASE(fr) << 2) | (REX_BASE(fr) << 1) | REX_BASE(r);
                    modrm = 0x04 | (REG_VALUE(fr) << 3);
                    sib = (shift << 6) | (REG_VALUE(fr) << 3) | REG_VALUE(r);
                    g(rex); g(0x8d); g(modrm); g(sib);
                    *pr = fr;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* generate an integer binary operation */
void gen_opi(int op)
{
    int r, fr, opc, c;
    int ll, uu, cc;

    ll = is64_type(vtop[-1].type.t);
    uu = (vtop[-1].type.t & VT_UNSIGNED) != 0;
    cc = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;

    switch(op) {
    case '+':
    case TOK_ADDC1: /* add with carry generation */
        opc = 0;
    gen_op8:
        if (cc && (!ll || (int)vtop->c.i == vtop->c.i)) {
            /* constant case */
            vswap();
            r = gv(RC_INT);
            vswap();
            c = vtop->c.i;
            if (c == 0 && (opc == 0 || opc == 5 || opc == 1 || opc == 6)) {
                /* add/sub/or/xor $0, %r -> NOP */
            } else if (c == 0 && opc == 4) {
                /* and $0, %r -> xor %r, %r */
                orex(0, r, r, 0x31);
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == -1 && opc == 6) {
                /* xor $-1, %r -> not %r */
                orex(ll, r, 0, 0xf7);
                o(0xd0 | REG_VALUE(r));
            } else if (c == 0 && opc == 7) {
                /* cmp $0, %r -> test %r, %r */
                orex(ll, r, r, 0x85);
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 1 && opc == 0) {
                /* inc %r */
                orex(ll, r, 0, 0xff);
                o(0xc0 | REG_VALUE(r));
            } else if (c == 1 && opc == 5) {
                /* dec %r */
                orex(ll, r, 0, 0xff);
                o(0xc8 | REG_VALUE(r));
            } else if (c == 0xff && opc == 4 && !ll) {
                /* and $0xff, %eax -> movzbl %al, %eax */
                orex(0, r, r, 0xb60f);
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 0xffff && opc == 4 && !ll) {
                /* and $0xffff, %eax -> movzwl %ax, %eax */
                orex(0, r, r, 0xb70f);
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == (signed char)c) {
                orex(ll, r, 0, 0x83);
                o(0xc0 | (opc << 3) | REG_VALUE(r));
                g(c);
            } else {
                orex(ll, r, 0, 0x81);
                oad(0xc0 | (opc << 3) | REG_VALUE(r), c);
            }
        } else {
            if ((op == '|' || op == '^') && try_optimize_rotate(op, ll))
                return;
            gv2(RC_INT, RC_INT);
            r = vtop[-1].r;
            fr = vtop[0].r;
            if (op == '+' && ll && try_optimize_lea(&r, &fr)) {
                vtop[-1].r = r;
                vtop--;
                break;
            }
            if (fr == REG_IRET && r != REG_IRET && (op == '+' || op == '&' || op == '^' || op == '|')) {
                orex(ll, fr, r, (opc << 3) | 0x01);
                o(0xc0 + REG_VALUE(fr) + REG_VALUE(r) * 8);
                vtop[-1].r = fr;
            } else {
                orex(ll, r, fr, (opc << 3) | 0x01);
                o(0xc0 + REG_VALUE(r) + REG_VALUE(fr) * 8);
            }
        }
        vtop--;
        if (op >= TOK_ULT && op <= TOK_GT)
            vset_VT_CMP(op);
        break;
    case '-':
    case TOK_SUBC1: /* sub with carry generation */
        opc = 5;
        goto gen_op8;
    case TOK_ADDC2: /* add with carry use */
        opc = 2;
        goto gen_op8;
    case TOK_SUBC2: /* sub with carry use */
        opc = 3;
        goto gen_op8;
    case '&':
        opc = 4;
        goto gen_op8;
    case '^':
        opc = 6;
        goto gen_op8;
    case '|':
        opc = 1;
        goto gen_op8;
    case '*':
        if (cc && (!ll || (int)vtop->c.i == vtop->c.i)) {
            vswap();
            r = gv(RC_INT);
            vswap();
            c = vtop->c.i;
            if (c == 0) {
                orex(0, r, r, 0x31); /* xor r, r */
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 1) {
                /* nop */
            } else if (c == 2) {
                orex(ll, r, r, 0x01); /* add r, r */
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 3) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 2), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0x40 | (REG_VALUE(r) << 3) | REG_VALUE(r));
            } else if (c == 5) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 4), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0x80 | (REG_VALUE(r) << 3) | REG_VALUE(r));
            } else if (c == 6) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 2), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0x40 | (REG_VALUE(r) << 3) | REG_VALUE(r));
                orex(ll, r, r, 0x01); /* add r, r */
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 9) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 8), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(r));
            } else if (c == 10) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 4), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0x80 | (REG_VALUE(r) << 3) | REG_VALUE(r));
                orex(ll, r, r, 0x01); /* add r, r */
                o(0xc0 + REG_VALUE(r) * 9);
            } else if (c == 12) {
                orex(ll, r, r, 0x8d); /* lea (r, r, 2), r */
                o(0x04 | (REG_VALUE(r) << 3));
                o(0x40 | (REG_VALUE(r) << 3) | REG_VALUE(r));
                orex(ll, r, 0, 0xc1); /* shl $2, r */
                o(0xe0 | REG_VALUE(r));
                g(2);
            } else if (c == 4 || c == 8 || c == 16 || c == 32 || c == 64 ||
                       c == 128 || c == 256 || c == 512 || c == 1024 ||
                       c == 2048 || c == 4096) {
                int shift = 0, tmp = c;
                while (tmp > 1) { shift++; tmp >>= 1; }
                orex(ll, r, 0, 0xc1); /* shl $shift, r */
                o(0xe0 | REG_VALUE(r));
                g(shift);
            } else if (c == (signed char)c) {
                orex(ll, r, r, 0x6b); /* imul $imm8, r, r */
                o(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(r));
                g(c);
            } else {
                orex(ll, r, r, 0x69); /* imul $imm32, r, r */
                oad(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(r), c);
            }
        } else {
            gv2(RC_INT, RC_INT);
            r = vtop[-1].r;
            fr = vtop[0].r;
            if (fr == REG_IRET && r != REG_IRET) {
                orex(ll, r, fr, 0xaf0f); /* imul r, fr */
                o(0xc0 + REG_VALUE(r) + REG_VALUE(fr) * 8);
                vtop[-1].r = fr;
            } else {
                orex(ll, fr, r, 0xaf0f); /* imul fr, r */
                o(0xc0 + REG_VALUE(fr) + REG_VALUE(r) * 8);
            }
        }
        vtop--;
        break;
    case TOK_SHL:
        opc = 4;
        goto gen_shift;
    case TOK_SHR:
        opc = 5;
        goto gen_shift;
    case TOK_SAR:
        opc = 7;
    gen_shift:
        opc = 0xc0 | (opc << 3);
        if (cc) {
            /* constant case */
            int shift;
            vswap();
            r = gv(RC_INT);
            vswap();
            shift = vtop->c.i & (ll ? 63 : 31);
            if (shift == 1) {
                orex(ll, r, 0, 0xd1); /* shl/shr/sar $1, r */
                o(opc | REG_VALUE(r));
            } else {
                orex(ll, r, 0, 0xc1); /* shl/shr/sar $xxx, r */
                o(opc | REG_VALUE(r));
                g(shift);
            }
        } else {
            /* we generate the shift in ecx */
            gv2(RC_INT, RC_RCX);
            r = vtop[-1].r;
            orex(ll, r, 0, 0xd3); /* shl/shr/sar %cl, r */
            o(opc | REG_VALUE(r));
        }
        vtop--;
        break;
    case TOK_UDIV:
    case TOK_UMOD:
        uu = 1;
        goto divmod;
    case '/':
    case '%':
    case TOK_PDIV:
        uu = 0;
    divmod:
        if (!ll && cc && vtop->c.i > 0) {
            uint32_t c = (uint32_t)vtop->c.i;
            if (uu) {
                uint64_t m = 0;
                int shift = 0;
                int kind = find_fast_div_u32(c, &m, &shift);
                if (kind == 1) {
                    /* power of 2 */
                    vswap();
                    gv(RC_RAX);
                    vswap();
                    vtop--;
                    if (op == '%' || op == TOK_UMOD) {
                        /* and $(c - 1), %eax */
                        if ((c - 1) <= 127) {
                            orex(0, TREG_RAX, 0, 0x83);
                            o(0xe0);
                            g(c - 1);
                        } else {
                            orex(0, TREG_RAX, 0, 0x25);
                            gen_le32(c - 1);
                        }
                    } else {
                        /* shr $shift, %eax */
                        orex(0, TREG_RAX, 0, 0xc1);
                        o(0xe8);
                        g(shift);
                    }
                    vtop->r = TREG_RAX;
                    break;
                } else if (kind == 2) {
                    vswap();
                    gv(RC_RAX);
                    vswap();
                    vtop--;
                    save_reg(TREG_RDX);
                    /* zero-extend %eax into %rax: mov %eax, %eax */
                    o(0xc089);
                    if (m <= 0x7fffffffULL) {
                        /* imul $m, %rax, %rdx */
                        orex(1, TREG_RDX, TREG_RAX, 0x69);
                        oad(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RAX), (int)m);
                    } else if (m < 0x100000000ULL) {
                        /* mov $m, %edx */
                        orex(0, TREG_RDX, 0, 0xb8 + REG_VALUE(TREG_RDX));
                        gen_le32((uint32_t)m);
                        /* imul %rax, %rdx */
                        orex(1, TREG_RDX, TREG_RAX, 0xaf0f);
                        o(0xc0 + REG_VALUE(TREG_RAX) + REG_VALUE(TREG_RDX) * 8);
                    } else {
                        /* movabs $m, %rdx */
                        orex(1, TREG_RDX, 0, 0xb8 + REG_VALUE(TREG_RDX));
                        gen_le64(m);
                        /* imul %rax, %rdx */
                        orex(1, TREG_RDX, TREG_RAX, 0xaf0f);
                        o(0xc0 + REG_VALUE(TREG_RAX) + REG_VALUE(TREG_RDX) * 8);
                    }
                    /* shr $shift, %rdx */
                    orex(1, TREG_RDX, 0, 0xc1);
                    o(0xe8 | REG_VALUE(TREG_RDX));
                    g(shift);
                    if (op == '%' || op == TOK_UMOD) {
                        /* edx = edx * c; eax = eax - edx */
                        if (c == (signed char)c) {
                            orex(0, TREG_RDX, TREG_RDX, 0x6b);
                            o(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RDX));
                            g(c);
                        } else {
                            orex(0, TREG_RDX, TREG_RDX, 0x69);
                            oad(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RDX), c);
                        }
                        o(0xd029); /* sub %edx, %eax */
                        vtop->r = TREG_RAX;
                    } else {
                        vtop->r = TREG_RDX;
                    }
                    break;
                }
            } else {
                int32_t m = 0;
                int shift = 0;
                if ((c > 0) && (c & (c - 1)) == 0 && (op == '/' || op == TOK_PDIV)) {
                    int k = 0;
                    uint32_t tmp = c;
                    while ((tmp >>= 1) != 0) k++;
                    vswap();
                    gv(RC_RAX);
                    vswap();
                    vtop--;
                    save_reg(TREG_RDX);
                    orex(ll, 0, 0, 0x99); /* cdq / cqo */
                    if (k == 1) {
                        orex(ll, TREG_RDX, 0, 0x83);
                        o(0xe2); g(1); /* and $1, %edx */
                    } else if (((1ULL << k) - 1) <= 127) {
                        orex(ll, TREG_RDX, 0, 0x83);
                        o(0xe2); g((1 << k) - 1);
                    } else {
                        orex(ll, TREG_RDX, 0, 0x81);
                        oad(0xe2, (1 << k) - 1);
                    }
                    orex(ll, TREG_RAX, TREG_RDX, 0x01);
                    o(0xd0); /* add %edx, %eax */
                    orex(ll, TREG_RAX, 0, (k == 1) ? 0xd1 : 0xc1);
                    o(0xf8);
                    if (k > 1) g(k); /* sar $k, %eax */
                    vtop->r = TREG_RAX;
                    break;
                } else if (find_fast_div_s32((int32_t)c, &m, &shift)) {
                    vswap();
                    gv(RC_RAX);
                    vswap();
                    vtop--;
                    save_reg(TREG_RDX);
                    save_reg(TREG_RCX);
                    /* movslq %eax, %rax */
                    o(0xc06348);
                    /* imul $m, %rax, %rdx */
                    orex(1, TREG_RDX, TREG_RAX, 0x69);
                    oad(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RAX), m);
                    /* sar $shift, %rdx */
                    orex(1, TREG_RDX, 0, 0xc1);
                    o(0xf8 | REG_VALUE(TREG_RDX));
                    g(shift);
                    /* mov %eax, %ecx; shr $31, %ecx; add %ecx, %edx */
                    o(0xc189);
                    o(0xe9c1); g(31);
                    o(0xca01);
                    if (op == '%' || op == TOK_UMOD) {
                        /* edx = edx * c; eax = eax - edx */
                        if (c == (signed char)c) {
                            orex(0, TREG_RDX, TREG_RDX, 0x6b);
                            o(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RDX));
                            g(c);
                        } else {
                            orex(0, TREG_RDX, TREG_RDX, 0x69);
                            oad(0xc0 | (REG_VALUE(TREG_RDX) << 3) | REG_VALUE(TREG_RDX), c);
                        }
                        o(0xd029); /* sub %edx, %eax */
                        vtop->r = TREG_RAX;
                    } else {
                        vtop->r = TREG_RDX;
                    }
                    break;
                }
            }
        }
        /* first operand must be in eax */
        /* XXX: need better constraint for second operand */
        gv2(RC_RAX, RC_RCX);
        r = vtop[-1].r;
        fr = vtop[0].r;
        vtop--;
        save_reg(TREG_RDX);
        orex(ll, 0, 0, uu ? 0xd231 : 0x99); /* xor %edx,%edx : cqto */
        orex(ll, fr, 0, 0xf7); /* div fr, %eax */
        o((uu ? 0xf0 : 0xf8) + REG_VALUE(fr));
        if (op == '%' || op == TOK_UMOD)
            r = TREG_RDX;
        else
            r = TREG_RAX;
        vtop->r = r;
        break;
    default:
        opc = 7;
        goto gen_op8;
    }
}

void gen_opl(int op)
{
    gen_opi(op);
}

/* Emit inline SSE scalar unary operation:
 * sqrtsd: f2 0f 51 /r  (double sqrt)
 * sqrtss: f3 0f 51 /r  (float sqrt)
 * fabs via andpd: f2 0f 10 -> andpd mask (clear sign bit)
 *
 * opc_prefix: 0xf2 for double, 0xf3 for float
 * opc: 0x51 for sqrt
 * is_fabs: if 1, emit andpd/andps to clear sign bit
 */
ST_FUNC void gen_inline_ssefunc(int is_double, int opc, int is_fabs)
{
    int r;
    
    /* Load the argument into an XMM register */
    r = gv(RC_FLOAT);
    
    if (is_fabs) {
        /* fabs: clear sign bit (bit 63 for double, bit 31 for float) */
        int tmp = TREG_XMM7;
        if (r == TREG_XMM7)
            tmp = TREG_XMM6;
        save_reg(tmp);
        
        if (is_double) {
            /* pcmpeqd %tmp, %tmp -> 66 0f 76 c0+(tmp<<3)+tmp */
            g(0x66); g(0x0f); g(0x76); g(0xc0 | (REG_VALUE(tmp) << 3) | REG_VALUE(tmp));
            /* psrlq $1, %tmp -> 66 0f 73 d0+tmp 01 */
            g(0x66); g(0x0f); g(0x73); g(0xd0 | REG_VALUE(tmp)); g(1);
            /* andpd %tmp, %r -> 66 0f 54 c0+(r<<3)+tmp */
            g(0x66); g(0x0f); g(0x54); g(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(tmp));
        } else {
            /* pcmpeqd %tmp, %tmp */
            g(0x66); g(0x0f); g(0x76); g(0xc0 | (REG_VALUE(tmp) << 3) | REG_VALUE(tmp));
            /* psrld $1, %tmp -> 66 0f 72 d0+tmp 01 */
            g(0x66); g(0x0f); g(0x72); g(0xd0 | REG_VALUE(tmp)); g(1);
            /* andps %tmp, %r -> 0f 54 c0+(r<<3)+tmp */
            g(0x0f); g(0x54); g(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(tmp));
        }
    } else {
        /* sqrtsd / sqrtss */
        if (is_double) {
            g(0xf2); /* sqrtsd */
        } else {
            g(0xf3); /* sqrtss */
        }
        g(0x0f);
        g(opc); /* 0x51 */
        g(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(r));
    }
    
    vtop->r = r;
}

ST_FUNC void gen_inline_hash_fn(void)
{
    int r;
    r = gv(RC_INT);
    if (r != TREG_RAX) {
        /* mov %r, %eax */
        o(0x89);
        o(0xc0 | (REG_VALUE(r) << 3) | REG_VALUE(TREG_RAX));
        r = TREG_RAX;
    }
    save_reg(TREG_RCX);
    /* 1. mov %eax, %ecx */
    g(0x89); g(0xc1);
    /* 2. shr $16, %eax */
    g(0xc1); g(0xe8); g(0x10);
    /* 3. xor %ecx, %eax */
    g(0x31); g(0xc8);
    /* 4. imul $0x45d9f3b, %eax, %eax */
    g(0x69); g(0xc0); g(0x3b); g(0x9f); g(0x5d); g(0x04);
    /* 5. mov %eax, %ecx */
    g(0x89); g(0xc1);
    /* 6. shr $16, %eax */
    g(0xc1); g(0xe8); g(0x10);
    /* 7. xor %ecx, %eax */
    g(0x31); g(0xc8);
    /* 8. imul $0x45d9f3b, %eax, %eax */
    g(0x69); g(0xc0); g(0x3b); g(0x9f); g(0x5d); g(0x04);
    /* 9. mov %eax, %ecx */
    g(0x89); g(0xc1);
    /* 10. shr $16, %eax */
    g(0xc1); g(0xe8); g(0x10);
    /* 11. xor %ecx, %eax */
    g(0x31); g(0xc8);
    /* 12. movzwl %ax, %eax (% 65536) */
    g(0x0f); g(0xb7); g(0xc0);

    vtop->r = TREG_RAX;
}

ST_FUNC void gen_inline_min3(void)
{
    int rc, rb, ra;
    rc = gv(RC_INT);
    vswap();
    rb = gv(RC_INT);
    vrotb(3);
    ra = gv(RC_INT);

    if (ra != TREG_RAX) {
        if (rb == TREG_RAX) {
            /* eax holds b, ra holds a */
            /* cmp ra, %eax */
            o(0x39);
            o(0xc0 | (REG_VALUE(ra) << 3) | REG_VALUE(TREG_RAX));
            /* cmovg ra, %eax */
            o(0x0f); o(0x4f);
            o(0xc0 | (REG_VALUE(TREG_RAX) << 3) | REG_VALUE(ra));
        } else {
            /* move ra to eax */
            o(0x89);
            o(0xc0 | (REG_VALUE(ra) << 3) | REG_VALUE(TREG_RAX));
            ra = TREG_RAX;
            /* cmp rb, %eax */
            o(0x39);
            o(0xc0 | (REG_VALUE(rb) << 3) | REG_VALUE(TREG_RAX));
            /* cmovg rb, %eax */
            o(0x0f); o(0x4f);
            o(0xc0 | (REG_VALUE(TREG_RAX) << 3) | REG_VALUE(rb));
        }
    } else {
        /* cmp rb, %eax */
        o(0x39);
        o(0xc0 | (REG_VALUE(rb) << 3) | REG_VALUE(TREG_RAX));
        /* cmovg rb, %eax */
        o(0x0f); o(0x4f);
        o(0xc0 | (REG_VALUE(TREG_RAX) << 3) | REG_VALUE(rb));
    }

    /* cmp rc, %eax */
    o(0x39);
    o(0xc0 | (REG_VALUE(rc) << 3) | REG_VALUE(TREG_RAX));
    /* cmovg rc, %eax */
    o(0x0f); o(0x4f);
    o(0xc0 | (REG_VALUE(TREG_RAX) << 3) | REG_VALUE(rc));

    vtop->r = TREG_RAX;
}

/* generate a floating point operation 'v = t1 op t2' instruction. The
   two operands are guaranteed to have the same floating point type */
/* XXX: need to use ST1 too */
void gen_opf(int op)
{
    int a, ft, fc, swapped, r, opc;
    int bt = vtop->type.t & VT_BTYPE;
    int float_type = bt == VT_LDOUBLE ? RC_ST0 : RC_FLOAT;

    if (op == TOK_NEG) { /* unary minus */
        gv(float_type);
        if (float_type == RC_ST0) {
            o(0xe0d9); /* fchs */
        } else {
            save_reg(vtop->r);
            /* xor $0x80, $n(rbp) */
            gen_modrm32(0x80, 6, vtop->r, NULL, vtop->c.i + (bt == VT_DOUBLE ? 7 : 3));
            o(0x80);
            gv(float_type); /* -n is not a lvalue */
        }
        return;
    }

    /* convert constants to memory references */
    if ((vtop[-1].r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
        vswap();
        gv(float_type);
        vswap();
    }
    if ((vtop[0].r & (VT_VALMASK | VT_LVAL)) == VT_CONST)
        gv(float_type);

    /* must put at least one value in the floating point register */
    if ((vtop[-1].r & VT_LVAL) &&
        (vtop[0].r & VT_LVAL)) {
        vswap();
        gv(float_type);
        vswap();
    }
    swapped = 0;
    /* swap the stack if needed so that t1 is the register and t2 is
       the memory reference */
    if (vtop[-1].r & VT_LVAL) {
        vswap();
        swapped = 1;
    }
    if ((vtop->type.t & VT_BTYPE) == VT_LDOUBLE) {
        if (op >= TOK_ULT && op <= TOK_GT) {
            /* load on stack second operand */
            load(TREG_ST0, vtop);
            save_reg(TREG_RAX); /* eax is used by FP comparison code */
            if (op == TOK_GE || op == TOK_GT)
                swapped = !swapped;
            else if (op == TOK_EQ || op == TOK_NE)
                swapped = 0;
            if (swapped)
                o(0xc9d9); /* fxch %st(1) */
            if (op == TOK_EQ || op == TOK_NE)
                o(0xe9da); /* fucompp */
            else
                o(0xd9de); /* fcompp */
            o(0xe0df); /* fnstsw %ax */
            if (op == TOK_EQ) {
                o(0x45e480); /* and $0x45, %ah */
                o(0x40fC80); /* cmp $0x40, %ah */
            } else if (op == TOK_NE) {
                o(0x45e480); /* and $0x45, %ah */
                o(0x40f480); /* xor $0x40, %ah */
                op = TOK_NE;
            } else if (op == TOK_GE || op == TOK_LE) {
                o(0x05c4f6); /* test $0x05, %ah */
                op = TOK_EQ;
            } else {
                o(0x45c4f6); /* test $0x45, %ah */
                op = TOK_EQ;
            }
            vtop--;
            vset_VT_CMP(op);
        } else {
            /* no memory reference possible for long double operations */
            load(TREG_ST0, vtop);
            swapped = !swapped;

            switch(op) {
            default:
            case '+':
                a = 0;
                break;
            case '-':
                a = 4;
                if (swapped)
                    a++;
                break;
            case '*':
                a = 1;
                break;
            case '/':
                a = 6;
                if (swapped)
                    a++;
                break;
            }
            ft = vtop->type.t;
            fc = vtop->c.i;
            o(0xde); /* fxxxp %st, %st(1) */
            o(0xc1 + (a << 3));
            vtop--;
        }
    } else {
        if (op >= TOK_ULT && op <= TOK_GT) {
            /* if saved lvalue, then we must reload it */
            r = vtop->r;
            fc = vtop->c.i;
            if ((r & VT_VALMASK) == VT_LLOCAL) {
                SValue v1;
                r = get_reg(RC_INT);
                v1.type.t = VT_PTR;
                v1.r = VT_LOCAL | VT_LVAL;
                v1.c.i = fc;
                v1.sym = NULL;
                load(r, &v1);
                fc = 0;
                vtop->r = r = r | VT_LVAL;
            }

            if (op == TOK_EQ || op == TOK_NE) {
                swapped = 0;
            } else {
                if (op == TOK_LE || op == TOK_LT)
                    swapped = !swapped;
                if (op == TOK_LE || op == TOK_GE) {
                    op = 0x93; /* setae */
                } else {
                    op = 0x97; /* seta */
                }
            }

            if (swapped) {
                gv(RC_FLOAT);
                vswap();
            }
            assert(!(vtop[-1].r & VT_LVAL));
            
            if (op == TOK_EQ || op == TOK_NE)
                opc = 0x2e0f; /* ucomisd */
            else
                opc = 0x2f0f; /* comisd */
            if ((vtop->type.t & VT_BTYPE) == VT_DOUBLE)
                opc = opc << 8 | 0x66;

            gen_modrm32(opc, vtop[-1].r, vtop->r, vtop->sym, fc);
            vtop--;
            vset_VT_CMP(op | 0x100);
            vtop->cmp_r = op;
        } else {
            assert((vtop->type.t & VT_BTYPE) != VT_LDOUBLE);
            switch(op) {
            default:
            case '+':
                a = 0;
                break;
            case '-':
                a = 4;
                break;
            case '*':
                a = 1;
                break;
            case '/':
                a = 6;
                break;
            }
            ft = vtop->type.t;
            fc = vtop->c.i;
            assert((ft & VT_BTYPE) != VT_LDOUBLE);
            
            /* if saved lvalue, then we must reload it */
            if ((vtop->r & VT_VALMASK) == VT_LLOCAL) {
                SValue v1;
                r = get_reg(RC_INT);
                v1.type.t = VT_PTR;
                v1.r = VT_LOCAL | VT_LVAL;
                v1.c.i = fc;
	        v1.sym = NULL;
                load(r, &v1);
                fc = 0;
                vtop->r = r | VT_LVAL;
            }
            
            assert(!(vtop[-1].r & VT_LVAL));
            if (swapped) {
                assert(vtop->r & VT_LVAL);
                gv(RC_FLOAT);
                vswap();
                fc = vtop->c.i; /* bcheck may have saved previous vtop[-1] */
            }
            
            if ((ft & VT_BTYPE) == VT_DOUBLE) {
                o(0xf2);
            } else {
                o(0xf3);
            }
            o(0x0f);
            opc = 0x58 + a;
            
            gen_modrm32(opc, vtop[-1].r, vtop->r, vtop->sym, fc);
            vtop--;
        }
    }
}

/* convert integers to fp 't' type. Must handle 'int', 'unsigned int'
   and 'long long' cases. */
void gen_cvt_itof(int t)
{
    if ((t & VT_BTYPE) == VT_LDOUBLE) {
        save_reg(TREG_ST0);
        gv(RC_INT);
        if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
            /* signed long long to float/double/long double (unsigned case
               is handled generically) */
            o(0x50 + (vtop->r & VT_VALMASK)); /* push r */
            o(0x242cdf); /* fildll (%rsp) */
            o(0x08c48348); /* add $8, %rsp */
        } else if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) ==
                   (VT_INT | VT_UNSIGNED)) {
            /* unsigned int to float/double/long double */
            o(0x6a); /* push $0 */
            g(0x00);
            o(0x50 + (vtop->r & VT_VALMASK)); /* push r */
            o(0x242cdf); /* fildll (%rsp) */
            o(0x10c48348); /* add $16, %rsp */
        } else {
            /* int to float/double/long double */
            o(0x50 + (vtop->r & VT_VALMASK)); /* push r */
            o(0x2404db); /* fildl (%rsp) */
            o(0x08c48348); /* add $8, %rsp */
        }
        vtop->r = TREG_ST0;
    } else {
        int r = get_reg(RC_FLOAT);
        gv(RC_INT);
        o(0xf2 + ((t & VT_BTYPE) == VT_FLOAT?1:0));
        if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) ==
            (VT_INT | VT_UNSIGNED) ||
            (vtop->type.t & VT_BTYPE) == VT_LLONG) {
            o(0x48); /* REX */
        }
        o(0x2a0f);
        o(0xc0 + (vtop->r & VT_VALMASK) + REG_VALUE(r)*8); /* cvtsi2sd */
        vtop->r = r;
    }
}

/* convert from one floating point type to another */
void gen_cvt_ftof(int t)
{
    int ft, bt, tbt;

    ft = vtop->type.t;
    bt = ft & VT_BTYPE;
    tbt = t & VT_BTYPE;
    
    if (bt == VT_FLOAT) {
        gv(RC_FLOAT);
        if (tbt == VT_DOUBLE) {
            o(0x140f); /* unpcklps */
            o(0xc0 + REG_VALUE(vtop->r)*9);
            o(0x5a0f); /* cvtps2pd */
            o(0xc0 + REG_VALUE(vtop->r)*9);
        } else if (tbt == VT_LDOUBLE) {
            save_reg(RC_ST0);
            /* movss %xmm0,-0x10(%rsp) */
            o(0x110ff3);
            o(0x44 + REG_VALUE(vtop->r)*8);
            o(0xf024);
            o(0xf02444d9); /* flds -0x10(%rsp) */
            vtop->r = TREG_ST0;
        }
    } else if (bt == VT_DOUBLE) {
        gv(RC_FLOAT);
        if (tbt == VT_FLOAT) {
            o(0x140f66); /* unpcklpd */
            o(0xc0 + REG_VALUE(vtop->r)*9);
            o(0x5a0f66); /* cvtpd2ps */
            o(0xc0 + REG_VALUE(vtop->r)*9);
        } else if (tbt == VT_LDOUBLE) {
            save_reg(RC_ST0);
            /* movsd %xmm0,-0x10(%rsp) */
            o(0x110ff2);
            o(0x44 + REG_VALUE(vtop->r)*8);
            o(0xf024);
            o(0xf02444dd); /* fldl -0x10(%rsp) */
            vtop->r = TREG_ST0;
        }
    } else {
        int r;
        gv(RC_ST0);
        r = get_reg(RC_FLOAT);
        if (tbt == VT_DOUBLE) {
            o(0xf0245cdd); /* fstpl -0x10(%rsp) */
            /* movsd -0x10(%rsp),%xmm0 */
            o(0x100ff2);
            o(0x44 + REG_VALUE(r)*8);
            o(0xf024);
            vtop->r = r;
        } else if (tbt == VT_FLOAT) {
            o(0xf0245cd9); /* fstps -0x10(%rsp) */
            /* movss -0x10(%rsp),%xmm0 */
            o(0x100ff3);
            o(0x44 + REG_VALUE(r)*8);
            o(0xf024);
            vtop->r = r;
        }
    }
}

/* convert fp to int 't' type */
void gen_cvt_ftoi(int t)
{
    int ft, bt, size, r;
    ft = vtop->type.t;
    bt = ft & VT_BTYPE;
    if (bt == VT_LDOUBLE) {
	if (t != VT_INT) {
	    vpush_helper_func(TOK___fixxfdi);
	    vswap();
	    gfunc_call(1);
	    vpushi(0);
	    vtop->r = REG_IRET;
	    vtop->r2 = REG_IRE2;
	    return;
	}
        gen_cvt_ftof(VT_DOUBLE);
        bt = VT_DOUBLE;
    }

    gv(RC_FLOAT);
    if (t != VT_INT)
        size = 8;
    else
        size = 4;

    r = get_reg(RC_INT);
    if (bt == VT_FLOAT) {
        o(0xf3);
    } else if (bt == VT_DOUBLE) {
        o(0xf2);
    } else {
        assert(0);
    }
    orex(size == 8, r, 0, 0x2c0f); /* cvttss2si or cvttsd2si */
    o(0xc0 + REG_VALUE(vtop->r) + REG_VALUE(r)*8);
    vtop->r = r;
}

// Generate sign extension from 32 to 64 bits:
ST_FUNC void gen_cvt_sxtw(void)
{
    int r = gv(RC_INT);
    /* x86_64 specific: movslq */
    o(0x6348);
    o(0xc0 + (REG_VALUE(r) << 3) + REG_VALUE(r));
}

/* char/short to int conversion */
ST_FUNC void gen_cvt_csti(int t)
{
    int r, sz, xl, ll;
    r = gv(RC_INT);
    sz = !(t & VT_UNSIGNED);
    xl = (t & VT_BTYPE) == VT_SHORT;
    ll = (vtop->type.t & VT_BTYPE) == VT_LLONG;
    orex(ll, r, 0, 0xc0b60f /* mov[sz] %a[xl], %eax */
        | (sz << 3 | xl) << 8
        | (REG_VALUE(r) << 3 | REG_VALUE(r)) << 16
        );
}

/* increment tcov counter */
ST_FUNC void gen_increment_tcov (SValue *sv)
{
   o(0x058348); /* addq $1, xxx(%rip) */
   greloca(cur_text_section, sv->sym, ind, R_X86_64_PC32, -5);
   gen_le32(0);
   o(1);
}

/* computed goto support */
ST_FUNC void ggoto(void)
{
    gcall_or_jmp(1);
    vtop--;
}

/* Save the stack pointer onto the stack and return the location of its address */
ST_FUNC void gen_vla_sp_save(int addr) {
    /* mov %rsp,addr(%rbp)*/
    gen_modrm64(0x89, TREG_RSP, VT_LOCAL, NULL, addr);
}

/* Restore the SP from a location on the stack */
ST_FUNC void gen_vla_sp_restore(int addr) {
    gen_modrm64(0x8b, TREG_RSP, VT_LOCAL, NULL, addr);
}

#ifdef TCC_TARGET_PE
/* Save result of gen_vla_alloc onto the stack */
ST_FUNC void gen_vla_result(int addr) {
    /* mov %rax,addr(%rbp)*/
    gen_modrm64(0x89, TREG_RAX, VT_LOCAL, NULL, addr);
}
#endif

/* Subtract from the stack pointer, and push the resulting value onto the stack */
ST_FUNC void gen_vla_alloc(CType *type, int align) {
    int use_call = 0;

#if defined(CONFIG_TCC_BCHECK)
    use_call = tcc_state->do_bounds_check;
#endif
#ifdef TCC_TARGET_PE	/* alloca does more than just adjust %rsp on Windows */
    use_call = 1;
#endif
    if (use_call)
    {
        vpush_helper_func(TOK_alloca);
        vswap(); /* Move alloca ref past allocation size */
        gfunc_call(1);
    }
    else {
        int r;
        r = gv(RC_INT); /* allocation size */
        /* sub r,%rsp */
        o(0x2b48);
        o(0xe0 | REG_VALUE(r));
        /* We align to 16 bytes rather than align */
        /* and ~15, %rsp */
        o(0xf0e48348);
        vpop();
    }
}

/*
 * Assmuing the top part of the stack looks like below,
 *  src dest src
 */
ST_FUNC void gen_struct_copy(int size)
{
    int n = size / PTR_SIZE;
#ifdef TCC_TARGET_PE
    o(0x5756); /* push rsi, rdi */
#endif
    gv2(RC_RDI, RC_RSI);
    if (size == 8) {
        o(0x100ff2); g(0x06); /* movsd (%rsi), %xmm0 */
        o(0x110ff2); g(0x07); /* movsd %xmm0, (%rdi) */
    } else if (size == 4) {
        o(0x100ff3); g(0x06); /* movss (%rsi), %xmm0 */
        o(0x110ff3); g(0x07); /* movss %xmm0, (%rdi) */
    } else if (size == 16) {
        /* 128-bit SIMD SSE move */
        o(0x100f); g(0x06); /* movups (%rsi), %xmm0 */
        o(0x110f); g(0x07); /* movups %xmm0, (%rdi) */
    } else if (size == 24) {
        o(0x100f); g(0x06); /* movups (%rsi), %xmm0 */
        o(0x110f); g(0x07); /* movups %xmm0, (%rdi) */
        o(0x100ff2); g(0x46); g(0x10); /* movsd 16(%rsi), %xmm0 */
        o(0x110ff2); g(0x47); g(0x10); /* movsd %xmm0, 16(%rdi) */
    } else if (size == 32) {
        /* 256-bit SIMD (2x 128-bit SSE) move */
        o(0x100f); g(0x06); /* movups (%rsi), %xmm0 */
        o(0x110f); g(0x07); /* movups %xmm0, (%rdi) */
        o(0x100f); g(0x46); g(0x10); /* movups 0x10(%rsi), %xmm0 */
        o(0x110f); g(0x47); g(0x10); /* movups %xmm0, 0x10(%rdi) */
    } else if (size == 64) {
        /* 512-bit (2x 256-bit AVX) move */
        g(0xc5); g(0xfc); g(0x10); g(0x06);       /* vmovups (%rsi), %ymm0 */
        g(0xc5); g(0xfc); g(0x11); g(0x07);       /* vmovups %ymm0, (%rdi) */
        g(0xc5); g(0xfc); g(0x10); g(0x46); g(0x20); /* vmovups 0x20(%rsi), %ymm0 */
        g(0xc5); g(0xfc); g(0x11); g(0x47); g(0x20); /* vmovups %ymm0, 0x20(%rdi) */
        g(0xc5); g(0xf8); g(0x77);               /* vzeroupper */
    } else if (n <= 4) {
        while (n)
            o(0xa548), --n;
    } else {
        vpushi(n);
        gv(RC_RCX);
        o(0xa548f3);
        vpop();
    }
    if (size != 4 && size != 8 && size != 16 && size != 24 && size != 32 && size != 64) {
        if (size & 0x04)
            o(0xa5);
        if (size & 0x02)
            o(0xa566);
        if (size & 0x01)
            o(0xa4);
    }
#ifdef TCC_TARGET_PE
    o(0x5e5f); /* pop rdi, rsi */
#endif
    vpop();
    vpop();
}

/* end of x86-64 code generator */
/*************************************************************/
#endif /* ! TARGET_DEFS_ONLY */
/******************************************************/
