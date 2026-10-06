/*
 *  i386 specific functions for TCC assembler
 *
 *  Copyright (c) 2001, 2002 Fabrice Bellard
 *  Copyright (c) 2009 Frédéric Feret (x86_64 support)
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

#define USING_GLOBALS
#include "tcc.h"

#define MAX_OPERANDS 4

#define TOK_ASM_first TOK_ASM_clc
#define TOK_ASM_last TOK_ASM_emms
#define TOK_ASM_alllast TOK_ASM_subps

#define OPC_B          0x01  /* only used with OPC_WL */
#define OPC_WL         0x02  /* accepts w, l or no suffix */
#define OPC_BWL        (OPC_B | OPC_WL) /* accepts b, w, l or no suffix */
#define OPC_REG        0x04 /* register is added to opcode */
#define OPC_MODRM      0x08 /* modrm encoding */

#define OPCT_MASK      0x70
#define OPC_FWAIT      0x10 /* add fwait opcode */
#define OPC_SHIFT      0x20 /* shift opcodes */
#define OPC_ARITH      0x30 /* arithmetic opcodes */
#define OPC_FARITH     0x40 /* FPU arithmetic opcodes */
#define OPC_TEST       0x50 /* test opcodes */
#define OPC_0F01       0x60 /* 0x0f01XX (group 7, XX is 2nd opcode,
                               no operands and unstructured mod/rm) */
#define OPCT_IS(v,i) (((v) & OPCT_MASK) == (i))

#define OPC_0F        0x100 /* Is secondary map (0x0f prefix) */
#define OPC_48        0x200 /* Always has REX prefix */
#ifdef TCC_TARGET_X86_64
# define OPC_WLQ     0x1000  /* accepts w, l, q or no suffix */
# define OPC_BWLQ    (OPC_B | OPC_WLQ) /* accepts b, w, l, q or no suffix */
# define OPC_WLX     OPC_WLQ
# define OPC_BWLX    OPC_BWLQ
#else
# define OPC_WLX     OPC_WL
# define OPC_BWLX    OPC_BWL
#endif

#define OPC_GROUP_SHIFT 13

/* in order to compress the operand type, we use specific operands and
   we or only with EA  */
enum {
    OPT_REG8=0, /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_REG16,  /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_REG32,  /* warning: value is hardcoded from TOK_ASM_xxx */
#ifdef TCC_TARGET_X86_64
    OPT_REG64,  /* warning: value is hardcoded from TOK_ASM_xxx */
#endif
    OPT_MMX,    /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_SSE,    /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_CR,     /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_TR,     /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_DB,     /* warning: value is hardcoded from TOK_ASM_xxx */
    OPT_SEG,
    OPT_ST,
#ifdef TCC_TARGET_X86_64
    OPT_REG8_LOW, /* %spl,%bpl,%sil,%dil, encoded like ah,ch,dh,bh, but
		     with REX prefix, not used in insn templates */
#endif
    OPT_IM8,
    OPT_IM8S,
    OPT_IM16,
    OPT_IM32,
#ifdef TCC_TARGET_X86_64
    OPT_IM64,
#endif
    OPT_EAX,    /* %al, %ax, %eax or %rax register */
    OPT_ST0,    /* %st(0) register */
    OPT_CL,     /* %cl register */
    OPT_DX,     /* %dx register */
    OPT_ADDR,   /* OP_EA with only offset */
    OPT_INDIR,  /* *(expr) */
    /* composite types */
    OPT_COMPOSITE_FIRST,
    OPT_IM,     /* IM8 | IM16 | IM32 */
    OPT_REG,    /* REG8 | REG16 | REG32 | REG64 */
    OPT_REGW,   /* REG16 | REG32 | REG64 */
    OPT_IMW,    /* IM16 | IM32 */
    OPT_MMXSSE, /* MMX | SSE */
    OPT_DISP,   /* Like OPT_ADDR, but emitted as displacement (for jumps) */
    OPT_DISP8,  /* Like OPT_ADDR, but only 8bit (short jumps) */
    /* can be ored with any OPT_xxx */
    OPT_EA = 0x80
};

/* VEX/EVEX dedicated operand types (used in VEXInstr.op_type) */
#define OPT_YMM    33
#define OPT_ZMM    34
#define OPT_MASK   35
#define OPT_XYZMM  36

#define OP_REG8   (1 << OPT_REG8)
#define OP_REG16  (1 << OPT_REG16)
#define OP_REG32  (1 << OPT_REG32)
#define OP_MMX    (1 << OPT_MMX)
#define OP_SSE    (1 << OPT_SSE)
#define OP_CR     (1 << OPT_CR)
#define OP_TR     (1 << OPT_TR)
#define OP_DB     (1 << OPT_DB)
#define OP_SEG    (1 << OPT_SEG)
#define OP_ST     (1 << OPT_ST)
#define OP_IM8    (1 << OPT_IM8)
#define OP_IM8S   (1 << OPT_IM8S)
#define OP_IM16   (1 << OPT_IM16)
#define OP_IM32   (1 << OPT_IM32)
#define OP_EAX    (1 << OPT_EAX)
#define OP_ST0    (1 << OPT_ST0)
#define OP_CL     (1 << OPT_CL)
#define OP_DX     (1 << OPT_DX)
#define OP_ADDR   (1 << OPT_ADDR)
#define OP_INDIR  (1 << OPT_INDIR)
#ifdef TCC_TARGET_X86_64
# define OP_REG64 (1 << OPT_REG64)
# define OP_REG8_LOW (1 << OPT_REG8_LOW)
# define OP_IM64  (1 << OPT_IM64)
# define OP_EA32  (OP_EA << 1)
#else
# define OP_REG64 0
# define OP_REG8_LOW 0
# define OP_IM64  0
# define OP_EA32  0
#endif

#define OP_EA     0x40000000u
#define OP_REG    (OP_REG8 | OP_REG16 | OP_REG32 | OP_REG64)
#define OP_YMM    (1u << 23)
#define OP_ZMM    (1u << 24)
#define OP_MASK   (1u << 25)

#ifdef TCC_TARGET_X86_64
# define TREG_XAX   TREG_RAX
# define TREG_XCX   TREG_RCX
# define TREG_XDX   TREG_RDX
# define TOK_ASM_xax TOK_ASM_rax
#else
# define TREG_XAX   TREG_EAX
# define TREG_XCX   TREG_ECX
# define TREG_XDX   TREG_EDX
# define TOK_ASM_xax TOK_ASM_eax
#endif

typedef struct ASMInstr {
    uint16_t sym;
    uint16_t opcode;
    uint16_t instr_type;
    uint8_t nb_ops;
    uint8_t op_type[MAX_OPERANDS]; /* see OP_xxx */
} ASMInstr;

typedef struct Operand {
    uint32_t type;
    int8_t  reg; /* register, -1 if none */
    int8_t  reg2; /* second register, -1 if none */
    uint8_t shift;
    ExprValue e;
} Operand;

static const uint8_t reg_to_size[9] = {
/*
    [OP_REG8] = 0,
    [OP_REG16] = 1,
    [OP_REG32] = 2,
#ifdef TCC_TARGET_X86_64
    [OP_REG64] = 3,
#endif
*/
    0, 0, 1, 0, 2, 0, 0, 0, 3
};

#define NB_TEST_OPCODES 30

static const uint8_t test_bits[NB_TEST_OPCODES] = {
 0x00, /* o */
 0x01, /* no */
 0x02, /* b */
 0x02, /* c */
 0x02, /* nae */
 0x03, /* nb */
 0x03, /* nc */
 0x03, /* ae */
 0x04, /* e */
 0x04, /* z */
 0x05, /* ne */
 0x05, /* nz */
 0x06, /* be */
 0x06, /* na */
 0x07, /* nbe */
 0x07, /* a */
 0x08, /* s */
 0x09, /* ns */
 0x0a, /* p */
 0x0a, /* pe */
 0x0b, /* np */
 0x0b, /* po */
 0x0c, /* l */
 0x0c, /* nge */
 0x0d, /* nl */
 0x0d, /* ge */
 0x0e, /* le */
 0x0e, /* ng */
 0x0f, /* nle */
 0x0f, /* g */
};

static const uint8_t segment_prefixes[] = {
 0x26, /* es */
 0x2e, /* cs */
 0x36, /* ss */
 0x3e, /* ds */
 0x64, /* fs */
 0x65  /* gs */
};

static const ASMInstr asm_instrs[] = {
#define ALT(x) x
/* This removes a 0x0f in the second byte */
#define O(o) ((uint64_t) ((((o) & 0xff00) == 0x0f00) ? ((((o) >> 8) & ~0xff) | ((o) & 0xff)) : (o)))
/* This constructs instr_type from opcode, type and group.  */
#define T(o,i,g) ((i) | ((g) << OPC_GROUP_SHIFT) | ((((o) & 0xff00) == 0x0f00) ? OPC_0F : 0))
#define DEF_ASM_OP0(name, opcode)
#define DEF_ASM_OP0L(name, opcode, group, instr_type) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 0, { 0 } },
#define DEF_ASM_OP1(name, opcode, group, instr_type, op0) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 1, { op0 }},
#define DEF_ASM_OP2(name, opcode, group, instr_type, op0, op1) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 2, { op0, op1 }},
#define DEF_ASM_OP3(name, opcode, group, instr_type, op0, op1, op2) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 3, { op0, op1, op2 }},
#ifdef TCC_TARGET_X86_64
# include "x86_64-asm.h"
#else
# include "i386-asm.h"
#endif
    /* last operation */
    { 0, },
};

static const uint16_t op0_codes[] = {
#define ALT(x)
#define DEF_ASM_OP0(x, opcode) opcode,
#define DEF_ASM_OP0L(name, opcode, group, instr_type)
#define DEF_ASM_OP1(name, opcode, group, instr_type, op0)
#define DEF_ASM_OP2(name, opcode, group, instr_type, op0, op1)
#define DEF_ASM_OP3(name, opcode, group, instr_type, op0, op1, op2)
#ifdef TCC_TARGET_X86_64
# include "x86_64-asm.h"
#else
# include "i386-asm.h"
#endif
};

static inline int get_reg_shift(TCCState *s1)
{
    int shift, v;
    v = asm_int_expr(s1);
    switch(v) {
    case 1:
        shift = 0;
        break;
    case 2:
        shift = 1;
        break;
    case 4:
        shift = 2;
        break;
    case 8:
        shift = 3;
        break;
    default:
        expect("1, 2, 4 or 8 constant");
        shift = 0;
        break;
    }
    return shift;
}

#ifdef TCC_TARGET_X86_64
static int asm_parse_numeric_reg(int t, unsigned int *type)
{
    int reg = -1;
    if (t >= TOK_IDENT && t < tok_ident) {
	const char *s = table_ident[t - TOK_IDENT]->str;
	char c;
        /* Check for ymm/zmm/k register names (for extended registers not in
           the fixed token table, e.g. ymm16-ymm31, zmm16-zmm31) */
        if (s[0] == 'y' && s[1] == 'm' && s[2] == 'm') {
            s += 3;
            *type = OP_YMM;
            goto parse_num;
        } else if (s[0] == 'z' && s[1] == 'm' && s[2] == 'm') {
            s += 3;
            *type = OP_ZMM;
            goto parse_num;
        } else if (s[0] == 'k' && s[1] >= '0' && s[1] <= '7' && !s[2]) {
            *type = OP_MASK;
            return s[1] - '0';
        }
	*type = OP_REG64;
	if (*s == 'c') {
	    s++;
	    *type = OP_CR;
	}
	if (*s++ != 'r')
	  return -1;
    parse_num:
	/* Don't allow leading '0'.  */
	if ((c = *s++) >= '1' && c <= '9')
	  reg = c - '0';
	else
	  return -1;
	if ((c = *s) >= '0' && c <= '5')
	  s++, reg = reg * 10 + c - '0';
	if (reg > 31)
	  return -1;
        /* ymm/zmm: only 0-31 valid, checked above */
        if (*type == OP_YMM || *type == OP_ZMM) {
            if (reg > 31 || *s != 0)
                return -1;
            return reg;
        }
	if (reg > 15)
	  return -1;
	if ((c = *s) == 0)
	  ;
	else if (*type != OP_REG64)
	  return -1;
	else if (c == 'b' && !s[1])
	  *type = OP_REG8;
	else if (c == 'w' && !s[1])
	  *type = OP_REG16;
	else if (c == 'd' && !s[1])
	  *type = OP_REG32;
	else
	  return -1;
    }
    return reg;
}
#endif

static int asm_parse_reg(unsigned int *type)
{
    int reg = 0;
    *type = 0;
    if (tok != '%')
        goto error_32;
    next();
    if (tok >= TOK_ASM_eax && tok <= TOK_ASM_edi) {
        reg = tok - TOK_ASM_eax;
	*type = OP_REG32;
#ifdef TCC_TARGET_X86_64
    } else if (tok >= TOK_ASM_rax && tok <= TOK_ASM_rdi) {
        reg = tok - TOK_ASM_rax;
	*type = OP_REG64;
    } else if (tok == TOK_ASM_rip) {
        reg = -2; /* Probably should use different escape code. */
	*type = OP_REG64;
    } else if ((reg = asm_parse_numeric_reg(tok, type)) >= 0
	       && (*type == OP_REG32 || *type == OP_REG64)) {
	;
#endif
    } else {
    error_32:
        expect("register");
    }
    next();
    return reg;
}

static void parse_operand(TCCState *s1, Operand *op)
{
    ExprValue e;
    int reg, indir;
    const char *p;

    indir = 0;
    if (tok == '*') {
        next();
        indir = OP_INDIR;
    }

    if (tok == '%') {
        next();
        if (tok >= TOK_ASM_al && tok <= TOK_ASM_db7) {
            reg = tok - TOK_ASM_al;
            op->type = 1 << (reg >> 3); /* WARNING: do not change constant order */
            op->reg = reg & 7;
            if ((op->type & OP_REG) && op->reg == TREG_XAX)
                op->type |= OP_EAX;
            else if (op->type == OP_REG8 && op->reg == TREG_XCX)
                op->type |= OP_CL;
            else if (op->type == OP_REG16 && op->reg == TREG_XDX)
                op->type |= OP_DX;
        } else if (tok >= TOK_ASM_dr0 && tok <= TOK_ASM_dr7) {
            op->type = OP_DB;
            op->reg = tok - TOK_ASM_dr0;
        } else if (tok >= TOK_ASM_es && tok <= TOK_ASM_gs) {
            op->type = OP_SEG;
            op->reg = tok - TOK_ASM_es;
        } else if (tok == TOK_ASM_st) {
            op->type = OP_ST;
            op->reg = 0;
            next();
            if (tok == '(') {
                next();
                if (tok != TOK_PPNUM)
                    goto reg_error;
                p = tokc.str.data;
                reg = p[0] - '0';
                if ((unsigned)reg >= 8 || p[1] != '\0')
                    goto reg_error;
                op->reg = reg;
                next();
                skip(')');
            }
            if (op->reg == 0)
                op->type |= OP_ST0;
            goto no_skip;
#ifdef TCC_TARGET_X86_64
	} else if (tok >= TOK_ASM_spl && tok <= TOK_ASM_dil) {
	    op->type = OP_REG8 | OP_REG8_LOW;
	    op->reg = 4 + tok - TOK_ASM_spl;
        /* YMM registers ymm0-ymm7 (and x86-64: ymm8-ymm15) */
        } else if (tok >= TOK_ASM_ymm0 && tok <= TOK_ASM_ymm7) {
            op->type = OP_YMM;
            op->reg = tok - TOK_ASM_ymm0;
        } else if (tok >= TOK_ASM_ymm8 && tok <= TOK_ASM_ymm15) {
            op->type = OP_YMM;
            op->reg = 8 + tok - TOK_ASM_ymm8;
        /* ZMM registers zmm0-zmm7 (and x86-64: zmm8-zmm31) */
        } else if (tok >= TOK_ASM_zmm0 && tok <= TOK_ASM_zmm7) {
            op->type = OP_ZMM;
            op->reg = tok - TOK_ASM_zmm0;
        } else if (tok >= TOK_ASM_zmm8 && tok <= TOK_ASM_zmm31) {
            op->type = OP_ZMM;
            op->reg = 8 + tok - TOK_ASM_zmm8;
        /* Opmask registers k0-k7 */
        } else if (tok >= TOK_ASM_k0 && tok <= TOK_ASM_k7) {
            op->type = OP_MASK;
            op->reg = tok - TOK_ASM_k0;
        /* XMM registers xmm8-xmm15 (x86-64 extended) */
        } else if (tok >= TOK_ASM_xmm8 && tok <= TOK_ASM_xmm15) {
            op->type = OP_SSE;
            op->reg = 8 + tok - TOK_ASM_xmm8;
        } else if ((op->reg = asm_parse_numeric_reg(tok, &op->type)) >= 0) {
	    ;
#endif
        } else {
        reg_error:
            tcc_error("unknown register %%%s", get_tok_str(tok, &tokc));
        }
        next();
    no_skip: ;
    } else if (tok == '$') {
        /* constant value */
        next();
        asm_expr(s1, &e);
        op->type = OP_IM32;
        op->e = e;
        if (!op->e.sym) {
            if (op->e.v == (uint8_t)op->e.v)
                op->type |= OP_IM8;
            if (op->e.v == (int8_t)op->e.v)
                op->type |= OP_IM8S;
            if (op->e.v == (uint16_t)op->e.v)
                op->type |= OP_IM16;
#ifdef TCC_TARGET_X86_64
            if (op->e.v != (int32_t)op->e.v && op->e.v != (uint32_t)op->e.v)
                op->type = OP_IM64;
#endif
        }
    } else {
        /* address(reg,reg2,shift) with all variants */
        op->type = OP_EA;
        op->reg = -1;
        op->reg2 = -1;
        op->shift = 0;
        if (tok != '(') {
            asm_expr(s1, &e);
            op->e = e;
        } else {
            next();
            if (tok == '%') {
                unget_tok('(');
                op->e.v = 0;
                op->e.sym = NULL;
            } else {
                /* bracketed offset expression */
                asm_expr(s1, &e);
                if (tok != ')')
                    expect(")");
                next();
                op->e.v = e.v;
                op->e.sym = e.sym;
            }
	    op->e.pcrel = 0;
        }
        if (tok == '(') {
	    unsigned int type = 0;
            next();
            if (tok != ',') {
                op->reg = asm_parse_reg(&type);
            }
            if (tok == ',') {
                next();
                if (tok != ',') {
                    op->reg2 = asm_parse_reg(&type);
                }
                if (tok == ',') {
                    next();
                    op->shift = get_reg_shift(s1);
                }
            }
	    if (type & OP_REG32)
	        op->type |= OP_EA32;
            skip(')');
        }
        if (op->reg == -1 && op->reg2 == -1)
            op->type |= OP_ADDR;
    }
    op->type |= indir;
}

/* XXX: unify with C code output ? */
ST_FUNC void gen_expr32(ExprValue *pe)
{
    if (pe->pcrel)
        /* If PC-relative, always set VT_SYM, even without symbol,
	   so as to force a relocation to be emitted.  */
	gen_addrpc32(VT_SYM, pe->sym, pe->v + (ind + 4));
    else
	gen_addr32(pe->sym ? VT_SYM : 0, pe->sym, pe->v);
}

/* XXX: unify with C code output ? */
static void gen_disp32(ExprValue *pe)
{
    Sym *sym = pe->sym;
    ElfSym *esym = elfsym(sym);
    if (esym && esym->st_shndx == cur_text_section->sh_num) {
        /* same section: we can output an absolute value. Note
           that the TCC compiler behaves differently here because
           it always outputs a relocation to ease (future) code
           elimination in the linker */
        gen_le32(pe->v + esym->st_value - ind - 4);
    } else {
        if (sym && sym->type.t == VT_VOID) {
            sym->type.t = VT_FUNC;
            sym->type.ref = NULL;
        }
#ifdef TCC_TARGET_X86_64
        greloca(cur_text_section, sym, ind, R_X86_64_PLT32, pe->v - 4);
        gen_le32(0);
#else
        gen_addrpc32(VT_SYM, sym, pe->v);
#endif

    }
}

/* generate the modrm operand */
static inline int asm_modrm(int reg, Operand *op)
{
    int mod, reg1, reg2, sib_reg1;

    if (op->type & (OP_REG | OP_MMX | OP_SSE | OP_YMM | OP_ZMM | OP_MASK)) {
        g(0xc0 + ((reg & 7) << 3) + (op->reg & 7));
    } else if (op->reg == -1 && op->reg2 == -1) {
        /* displacement only */
#ifdef TCC_TARGET_X86_64
	g(0x04 + (reg << 3));
	g(0x25);
#else
	g(0x05 + (reg << 3));
#endif
	gen_expr32(&op->e);
#ifdef TCC_TARGET_X86_64
    } else if (op->reg == -2) {
        ExprValue *pe = &op->e;
        g(0x05 + (reg << 3));
        gen_addrpc32(pe->sym ? VT_SYM : 0, pe->sym, pe->v);
        return ind;
#endif
    } else {
        sib_reg1 = op->reg;
        /* fist compute displacement encoding */
        if (sib_reg1 == -1) {
            sib_reg1 = 5;
            mod = 0x00;
        } else if (op->e.v == 0 && !op->e.sym && op->reg != 5) {
            mod = 0x00;
        } else if (op->e.v == (int8_t)op->e.v && !op->e.sym) {
            mod = 0x40;
        } else {
            mod = 0x80;
        }
        /* compute if sib byte needed */
        reg1 = op->reg;
        if (op->reg2 != -1)
            reg1 = 4;
        g(mod + (reg << 3) + reg1);
        if (reg1 == 4) {
            /* add sib byte */
            reg2 = op->reg2;
            if (reg2 == -1)
                reg2 = 4; /* indicate no index */
            g((op->shift << 6) + (reg2 << 3) + sib_reg1);
        }
        /* add offset */
        if (mod == 0x40) {
            g(op->e.v);
        } else if (mod == 0x80 || op->reg == -1) {
	    gen_expr32(&op->e);
        }
    }
    return 0;
}

#ifdef TCC_TARGET_X86_64
#define REX_W 0x48
#define REX_R 0x44
#define REX_X 0x42
#define REX_B 0x41

static void asm_rex(int width64, Operand *ops, int nb_ops, int *op_type,
		    int regi, int rmi)
{
  unsigned char rex = width64 ? 0x48 : 0;
  int saw_high_8bit = 0;
  int i;
  if (rmi == -1) {
      /* No mod/rm byte, but we might have a register op nevertheless
         (we will add it to the opcode later).  */
      for(i = 0; i < nb_ops; i++) {
	  if (op_type[i] & (OP_REG | OP_ST)) {
	      if (ops[i].reg >= 8) {
		  rex |= REX_B;
		  ops[i].reg -= 8;
	      } else if (ops[i].type & OP_REG8_LOW)
		  rex |= 0x40;
	      else if (ops[i].type & OP_REG8 && ops[i].reg >= 4)
		  /* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */
		  saw_high_8bit = ops[i].reg;
	      break;
	  }
      }
  } else {
      if (regi != -1) {
	  if (ops[regi].reg >= 8) {
	      rex |= REX_R;
	      ops[regi].reg -= 8;
	  } else if (ops[regi].type & OP_REG8_LOW)
	      rex |= 0x40;
	  else if (ops[regi].type & OP_REG8 && ops[regi].reg >= 4)
	      /* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */
	      saw_high_8bit = ops[regi].reg;
      }
      if (ops[rmi].type & (OP_REG | OP_MMX | OP_SSE | OP_CR | OP_EA)) {
	  if (ops[rmi].reg >= 8) {
	      rex |= REX_B;
	      ops[rmi].reg -= 8;
	  } else if (ops[rmi].type & OP_REG8_LOW)
	      rex |= 0x40;
	  else if (ops[rmi].type & OP_REG8 && ops[rmi].reg >= 4)
	      /* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */
	      saw_high_8bit = ops[rmi].reg;
      }
      if (ops[rmi].type & OP_EA && ops[rmi].reg2 >= 8) {
	  rex |= REX_X;
	  ops[rmi].reg2 -= 8;
      }
  }
  if (rex) {
      if (saw_high_8bit)
	  tcc_error("can't encode register %%%ch when REX prefix is required",
		    "acdb"[saw_high_8bit-4]);
      g(rex);
  }
}
#endif

/* ======================================================================
 * VEX / EVEX prefix encoder infrastructure
 * Supports AVX, AVX2, FMA3, AES-NI, SHA-NI, and AVX-512 instructions.
 * ====================================================================== */

#ifdef TCC_TARGET_X86_64

/* VEX instruction descriptor — separate from ASMInstr to avoid widening
   the hot legacy table. Uses a compact 8-byte record. */
typedef struct VEXInstr {
    uint32_t tok;        /* TOK_ASM_xxx */
    uint8_t  mm;         /* map_select: 1=0F, 2=0F38, 3=0F3A */
    uint8_t  pp;         /* pp field: 0=none, 1=66, 2=F3, 3=F2 */
    uint8_t  opcode;     /* 1-byte opcode after escape map */
    uint8_t  vex_flags;  /* VEXF_xxx flags (see below) */
    uint8_t  nb_ops;
    uint8_t  op_type[4]; /* operand type flags (OPT_SSE|OPT_YMM|OPT_ZMM|
                            OPT_MASK|OPT_EA|OPT_REG32|OPT_REG64|OPT_IM8) */
} VEXInstr;

/* vex_flags bits */
#define VEXF_W1      0x01  /* VEX.W=1 (64-bit or double-precision) */
#define VEXF_L256    0x02  /* VEX.L=1 (256-bit YMM operation) */
#define VEXF_L512    0x04  /* EVEX L'L=10 (512-bit ZMM operation) */
#define VEXF_MODRM   0x08  /* instruction has ModR/M byte */
#define VEXF_VEX3    0x10  /* force 3-byte VEX even when 2-byte would work */
#define VEXF_EVEX    0x20  /* use EVEX (0x62 prefix) rather than VEX */
#define VEXF_IMM8    0x40  /* 8-bit immediate follows ModR/M */
#define VEXF_MASKZ   0x80  /* zeroing masking supported (EVEX.z) */

/* pp encoding values */
#define VEX_PP_NONE  0
#define VEX_PP_66    1
#define VEX_PP_F3    2
#define VEX_PP_F2    3

/* map_select values */
#define VEX_MM_0F    1
#define VEX_MM_0F38  2
#define VEX_MM_0F3A  3

/* -----------------------------------------------------------------------
 * 2-byte VEX prefix: C5 [R | ~vvvv | L | pp]
 *   Valid only when: X=1, B=1, W=0, map_select=1
 * ----------------------------------------------------------------------- */
static void emit_vex2(int R, int vvvv, int L, int pp)
{
    g(0xC5);
    g(((R & 1) << 7) | ((~vvvv & 0xF) << 3) | ((L & 1) << 2) | (pp & 3));
}

/* -----------------------------------------------------------------------
 * 3-byte VEX prefix: C4 [R X B map_select] [W ~vvvv L pp]
 * ----------------------------------------------------------------------- */
static void emit_vex3(int R, int X, int B, int mm,
                      int W, int vvvv, int L, int pp)
{
    g(0xC4);
    g(((R & 1) << 7) | ((X & 1) << 6) | ((B & 1) << 5) | (mm & 0x1F));
    g(((W & 1) << 7) | ((~vvvv & 0xF) << 3) | ((L & 1) << 2) | (pp & 3));
}

/* -----------------------------------------------------------------------
 * 4-byte EVEX prefix: 62 [R X B R' 00 mm] [W ~vvvv 1 pp]
 *                        [z L' L b V' aaa]
 * ----------------------------------------------------------------------- */
static void emit_evex(int R, int X, int B, int Rp, int mm, int pp,
                      int W, int vvvv, int z, int Lp, int L,
                      int b, int Vp, int aaa)
{
    g(0x62);
    g(((R & 1) << 7) | ((X & 1) << 6) | ((B & 1) << 5) |
      ((Rp & 1) << 4) | (mm & 3));
    g(((W & 1) << 7) | ((~vvvv & 0xF) << 3) | 0x04 | (pp & 3));
    g(((z & 1) << 7) | ((Lp & 1) << 6) | ((L & 1) << 5) |
      ((b & 1) << 4) | ((Vp & 1) << 3) | (aaa & 7));
}

/* Forward declaration for the VEX opcode table */
static void asm_vex_opcode(TCCState *s1, int opcode,
                           Operand *ops, int nb_ops);

/* -----------------------------------------------------------------------
 * VEX instruction table
 * Included once to build the table; included again in i386-tok.h (with
 * DEF_VEX_OPx → DEF_ASM) to register token names.
 * ----------------------------------------------------------------------- */
static const VEXInstr vex_instrs[] = {
#define ALT(x) x
#define DEF_VEX_OP0(name, mm, pp, opc, fl) \
    { TOK_ASM_##name, mm, pp, opc, fl, 0, { 0, 0, 0, 0 } },
#define DEF_VEX_OP1(name, mm, pp, opc, fl, o0) \
    { TOK_ASM_##name, mm, pp, opc, fl, 1, { o0, 0, 0, 0 } },
#define DEF_VEX_OP2(name, mm, pp, opc, fl, o0, o1) \
    { TOK_ASM_##name, mm, pp, opc, fl, 2, { o0, o1, 0, 0 } },
#define DEF_VEX_OP3(name, mm, pp, opc, fl, o0, o1, o2) \
    { TOK_ASM_##name, mm, pp, opc, fl, 3, { o0, o1, o2, 0 } },
#define DEF_VEX_OP4(name, mm, pp, opc, fl, o0, o1, o2, o3) \
    { TOK_ASM_##name, mm, pp, opc, fl, 4, { o0, o1, o2, o3 } },
# include "x86_64-vex.h"
#undef DEF_VEX_OP0
#undef DEF_VEX_OP1
#undef DEF_VEX_OP2
#undef DEF_VEX_OP3
#undef DEF_VEX_OP4
#undef ALT
    { 0 }
};

/* -----------------------------------------------------------------------
 * asm_vex_opcode — dispatch VEX/EVEX instructions
 *
 * Called from asm_opcode() when the legacy instruction table fails.
 * Operands are in AT&T / GAS order:
 *   0-op:  vzeroupper, vzeroall
 *   1-op:  src/dst in ops[0]
 *   2-op:  load: ops[0]=src(rm), ops[1]=dst(reg)
 *          store: ops[0]=src(reg), ops[1]=dst(rm)
 *   3-op:  ops[0]=src2(rm), ops[1]=src1(vvvv), ops[2]=dst(reg)
 *          imm3: ops[0]=imm, ops[1]=src(rm), ops[2]=dst(reg)
 *   4-op:  imm4: ops[0]=imm, ops[1]=src2(rm), ops[2]=src1(vvvv), ops[3]=dst(reg)
 * ----------------------------------------------------------------------- */
static void asm_vex_opcode(TCCState *s1, int opcode,
                           Operand *ops, int nb_ops)
{
    const VEXInstr *pv;
    int i, reg, rm, vvvv_reg;
    int R, X, B, Rp, Vp;
    int L, Lp, W, z, b_bc, aaa;
    int modrm_index, vvvv_index, reg_index;
    int match;
    int pc;
    uint8_t imm_val = 0;

    /* Search the VEX instruction table */
    for (pv = vex_instrs; pv->tok != 0; pv++) {
        if (pv->tok != (uint32_t)opcode)
            continue;
        if (pv->nb_ops != nb_ops)
            continue;
        /* Check operand types */
        match = 1;
        for (i = 0; i < nb_ops; i++) {
            uint32_t need = pv->op_type[i];
            uint32_t got  = ops[i].type;
            uint32_t vmask;
            uint32_t base_type = need & ~OPT_EA;
            if (base_type == OPT_XYZMM)
                vmask = OP_SSE | OP_YMM | OP_ZMM;
            else if (base_type == OPT_YMM)
                vmask = OP_YMM;
            else if (base_type == OPT_ZMM)
                vmask = OP_ZMM;
            else if (base_type == OPT_MASK)
                vmask = OP_MASK;
            else if (base_type == OPT_SSE)
                vmask = OP_SSE;
            else if (base_type == OPT_MMX)
                vmask = OP_MMX;
            else if (base_type == OPT_MMXSSE)
                vmask = OP_MMX | OP_SSE;
            else if (base_type == OPT_REG32)
                vmask = OP_REG32;
            else if (base_type == OPT_REG64)
                vmask = OP_REG64;
            else if (base_type == OPT_IM8)
                vmask = OP_IM8;
            else
                vmask = (1u << (base_type & 0x1F));
            if (need & OPT_EA)
                vmask |= OP_EA;
            if (!(got & vmask)) {
                match = 0;
                break;
            }
        }
        if (match)
            break;
    }

    if (pv->tok == 0) {
        tcc_error("unknown VEX opcode '%s'", get_tok_str(opcode, NULL));
        return;
    }

    /* Determine vector length from operand types:
       ZMM → EVEX L'L=10 (512-bit)
       YMM → L=1 (256-bit)
       XMM → L=0 (128-bit) */
    L = 0; Lp = 0;
    for (i = 0; i < nb_ops; i++) {
        if (ops[i].type & OP_ZMM) { L = 0; Lp = 1; break; }
        if (ops[i].type & OP_YMM) { L = 1; }
    }
    /* Flags override auto-detect */
    if (pv->vex_flags & VEXF_L512) { L = 0; Lp = 1; }
    else if (pv->vex_flags & VEXF_L256) { L = 1; Lp = 0; }

    W = (pv->vex_flags & VEXF_W1) ? 1 : 0;

    /* Determine operand roles according to GAS convention:
       Immediate is always ops[0] if instruction has VEXF_IMM8. */
    modrm_index = -1;  /* ModR/M rm field */
    vvvv_index  = -1;  /* VEX.vvvv field */
    reg_index   = -1;  /* ModR/M reg field */

    if (nb_ops == 0) {
        emit_vex2(1, 0, L, pv->pp);
        g(pv->opcode);
        return;
    }

    if (pv->vex_flags & VEXF_IMM8) {
        imm_val = (uint8_t)ops[0].e.v;
        if (nb_ops == 4) {
            /* e.g. vshufps $imm, %xmm3, %xmm2, %xmm1 */
            modrm_index = 1;
            vvvv_index  = 2;
            reg_index   = 3;
        } else if (nb_ops == 3) {
            /* e.g. vroundps $imm, %xmm2, %xmm1 */
            modrm_index = 1;
            reg_index   = 2;
        }
    } else {
        if (nb_ops == 3) {
            /* e.g. vaddps %xmm2, %xmm1, %xmm0: rm=ops[0], vvvv=ops[1], reg=ops[2] */
            modrm_index = 0;
            vvvv_index  = 1;
            reg_index   = 2;
        } else if (nb_ops == 2) {
            if (ops[1].type & OP_EA) {
                /* store: vmovaps %xmm1, (%rax) -> reg=ops[0], rm=ops[1] */
                reg_index   = 0;
                modrm_index = 1;
            } else {
                /* load / reg-reg: vmovaps (%rax), %xmm1 -> rm=ops[0], reg=ops[1] */
                modrm_index = 0;
                reg_index   = 1;
            }
        } else if (nb_ops == 1) {
            modrm_index = 0;
            reg_index   = 0;
        }
    }

    /* Extract register numbers */
    reg = (reg_index >= 0) ? ops[reg_index].reg : 0;
    rm  = (modrm_index >= 0) ? ops[modrm_index].reg : 0;
    vvvv_reg = (vvvv_index >= 0) ? ops[vvvv_index].reg : 0; /* 0 inverts to 1111=unused */

    /* Extension bits (1 = not extended, 0 = extended in VEX):
       reg >= 8  -> R = 0
       rm >= 8   -> B = 0
       index >= 8-> X = 0 */
    R  = (reg >= 8) ? 0 : 1;
    Rp = (reg >= 16) ? 0 : 1;
    B  = (rm >= 8) ? 0 : 1;
    Vp = (vvvv_reg >= 16) ? 0 : 1;
    X  = 1;
    if (modrm_index >= 0) {
        if (!(ops[modrm_index].type & OP_EA)) {
            if (rm >= 16)
                X = 0;
        } else if (ops[modrm_index].reg2 >= 8) {
            X = 0;
        }
    }

    /* Normalize register indices for asm_modrm (must be 0..7) */
    if (reg_index >= 0 && ops[reg_index].reg >= 8)
        ops[reg_index].reg &= 7;
    if (modrm_index >= 0 && ops[modrm_index].reg >= 8)
        ops[modrm_index].reg &= 7;
    if (modrm_index >= 0 && (ops[modrm_index].type & OP_EA) && ops[modrm_index].reg2 >= 8)
        ops[modrm_index].reg2 &= 7;

    reg      = reg & 0xF;
    rm       = rm & 0xF;
    vvvv_reg = vvvv_reg & 0xF;

    z = 0; aaa = 0; b_bc = 0;

    /* Decide VEX vs EVEX */
    if ((pv->vex_flags & VEXF_EVEX) || Lp ||
        (vvvv_reg > 15) || (reg > 15) || (rm > 15)) {
        emit_evex(R, X, B, Rp, pv->mm, pv->pp, W, vvvv_reg,
                  z, Lp, L, b_bc, Vp, aaa);
        g(pv->opcode);
    } else if ((pv->vex_flags & VEXF_VEX3) ||
               pv->mm != VEX_MM_0F || W != 0 || !X || !B) {
        emit_vex3(R, X, B, pv->mm, W, vvvv_reg, L, pv->pp);
        g(pv->opcode);
    } else {
        emit_vex2(R, vvvv_reg, L, pv->pp);
        g(pv->opcode);
    }

    /* ModR/M */
    pc = 0;
    if ((pv->vex_flags & VEXF_MODRM) && modrm_index >= 0) {
        pc = asm_modrm(reg & 7, &ops[modrm_index]);
    }

    /* 8-bit immediate */
    if (pv->vex_flags & VEXF_IMM8) {
        g(imm_val);
    }

    /* Fixup PC-relative displacement */
    if (pc)
        add32le(cur_text_section->data + pc - 4, pc - ind);
}
#endif /* TCC_TARGET_X86_64 */


static void maybe_print_stats (void)
{
    static int already;

    if (0 && !already)
    /* print stats about opcodes */
    {
        const struct ASMInstr *pa;
        int freq[4];
        int op_vals[500];
        int nb_op_vals, i, j;

	already = 1;
        nb_op_vals = 0;
        memset(freq, 0, sizeof(freq));
        for(pa = asm_instrs; pa->sym != 0; pa++) {
            freq[pa->nb_ops]++;
            //for(i=0;i<pa->nb_ops;i++) {
                for(j=0;j<nb_op_vals;j++) {
                    //if (pa->op_type[i] == op_vals[j])
                    if (pa->instr_type == op_vals[j])
                        goto found;
                }
                //op_vals[nb_op_vals++] = pa->op_type[i];
                op_vals[nb_op_vals++] = pa->instr_type;
            found: ;
            //}
        }
        for(i=0;i<nb_op_vals;i++) {
            int v = op_vals[i];
            //if ((v & (v - 1)) != 0)
                printf("%3d: %08x\n", i, v);
        }
        printf("size=%d nb=%d f0=%d f1=%d f2=%d f3=%d\n",
               (int)sizeof(asm_instrs),
	       (int)sizeof(asm_instrs) / (int)sizeof(ASMInstr),
               freq[0], freq[1], freq[2], freq[3]);
    }
}

ST_FUNC void asm_opcode(TCCState *s1, int opcode)
{
    const ASMInstr *pa;
    int i, modrm_index, modreg_index, reg, v, op1, seg_prefix, pc, p;
    int nb_ops, s;
    Operand ops[MAX_OPERANDS], *pop;
    int op_type[3]; /* decoded op type */
    int alltypes;   /* OR of all operand types */
    int autosize;
    int p66;
#ifdef TCC_TARGET_X86_64
    int rex64;
#endif

    maybe_print_stats();
    /* force synthetic ';' after prefix instruction, so we can handle */
    /* one-line things like "rep stosb" instead of only "rep\nstosb" */
    if (opcode >= TOK_ASM_wait && opcode <= TOK_ASM_repnz)
        unget_tok(';');

    /* get operands */
    pop = ops;
    nb_ops = 0;
    seg_prefix = 0;
    alltypes = 0;
    for(;;) {
        if (tok == ';' || tok == TOK_LINEFEED)
            break;
        if (nb_ops >= MAX_OPERANDS) {
            tcc_error("incorrect number of operands");
        }
        parse_operand(s1, pop);
        if (tok == ':') {
           if (!(pop->type & OP_SEG) || seg_prefix)
               tcc_error("incorrect prefix");
           seg_prefix = segment_prefixes[pop->reg];
           next();
           parse_operand(s1, pop);
           if (!(pop->type & OP_EA)) {
               tcc_error("segment prefix must be followed by memory reference");
           }
        }
        pop++;
        nb_ops++;
        if (tok != ',')
            break;
        next();
    }

    s = 0; /* avoid warning */
again:
    /* optimize matching by using a lookup table (no hashing is needed
       !) */
    for(pa = asm_instrs; pa->sym != 0; pa++) {
	int it = pa->instr_type & OPCT_MASK;
        s = 0;
        if (it == OPC_FARITH) {
            v = opcode - pa->sym;
            if (!((unsigned)v < 8 * 6 && (v % 6) == 0))
                continue;
        } else if (it == OPC_ARITH) {
            if (!(opcode >= pa->sym && opcode < pa->sym + 8*NBWLX))
                continue;
            s = (opcode - pa->sym) % NBWLX;
	    if ((pa->instr_type & OPC_BWLX) == OPC_WLX)
	      {
		/* We need to reject the xxxb opcodes that we accepted above.
		   Note that pa->sym for WLX opcodes is the 'w' token,
		   to get the 'b' token subtract one.  */
		if (((opcode - pa->sym + 1) % NBWLX) == 0)
		    continue;
	        s++;
	      }
        } else if (it == OPC_SHIFT) {
            if (!(opcode >= pa->sym && opcode < pa->sym + 7*NBWLX))
                continue;
            s = (opcode - pa->sym) % NBWLX;
        } else if (it == OPC_TEST) {
            if (!(opcode >= pa->sym && opcode < pa->sym + NB_TEST_OPCODES))
                continue;
	    /* cmovxx is a test opcode but accepts multiple sizes.
	       The suffixes aren't encoded in the table, instead we
	       simply force size autodetection always and deal with suffixed
	       variants below when we don't find e.g. "cmovzl".  */
	    if (pa->instr_type & OPC_WLX)
	        s = NBWLX - 1;
        } else if (pa->instr_type & OPC_B) {
#ifdef TCC_TARGET_X86_64
	    /* Some instructions don't have the full size but only
	       bwl form.  insb e.g. */
	    if ((pa->instr_type & OPC_WLQ) != OPC_WLQ
		&& !(opcode >= pa->sym && opcode < pa->sym + NBWLX-1))
	        continue;
#endif
            if (!(opcode >= pa->sym && opcode < pa->sym + NBWLX))
                continue;
            s = opcode - pa->sym;
        } else if (pa->instr_type & OPC_WLX) {
            if (!(opcode >= pa->sym && opcode < pa->sym + NBWLX-1))
                continue;
            s = opcode - pa->sym + 1;
        } else {
            if (pa->sym != opcode)
                continue;
        }
        if (pa->nb_ops != nb_ops)
            continue;
#ifdef TCC_TARGET_X86_64
	/* Special case for moves.  Selecting the IM64->REG64 form
	   should only be done if we really have an >32bit imm64, and that
	   is hardcoded.  Ignore it here.  */
	if (pa->opcode == 0xb0 && ops[0].type != OP_IM64
	    && (ops[1].type & OP_REG) == OP_REG64
	    && !(pa->instr_type & OPC_0F))
	    continue;
#endif
        /* now decode and check each operand */
	alltypes = 0;
        for(i = 0; i < nb_ops; i++) {
            int op1, op2;
            op1 = pa->op_type[i];
            op2 = op1 & 0x1f;
            switch(op2) {
            case OPT_IM:
                v = OP_IM8 | OP_IM16 | OP_IM32;
                break;
            case OPT_REG:
                v = OP_REG8 | OP_REG16 | OP_REG32 | OP_REG64;
                break;
            case OPT_REGW:
                v = OP_REG16 | OP_REG32 | OP_REG64;
                break;
            case OPT_IMW:
                v = OP_IM16 | OP_IM32;
                break;
	    case OPT_MMXSSE:
		v = OP_MMX | OP_SSE;
		break;
	    case OPT_DISP:
	    case OPT_DISP8:
		v = OP_ADDR;
		break;
            default:
                v = 1 << op2;
                break;
            }
            if (op1 & OPT_EA)
                v |= OP_EA;
	    op_type[i] = v;
            if ((ops[i].type & v) == 0)
                goto next;
	    alltypes |= ops[i].type;
        }
        (void)alltypes; /* maybe unused */
        /* all is matching ! */
        break;
    next: ;
    }
    if (pa->sym == 0) {
        if (opcode >= TOK_ASM_first && opcode <= TOK_ASM_last) {
            int b;
            b = op0_codes[opcode - TOK_ASM_first];
            if (b & 0xff00)
                g(b >> 8);
            g(b);
            return;
        } else if (opcode <= TOK_ASM_alllast) {
            tcc_error("bad operand with opcode '%s'",
                  get_tok_str(opcode, NULL));
        } else {
	    /* Special case for cmovcc, we accept size suffixes but ignore
	       them, but we don't want them to blow up our tables.  */
	    TokenSym *ts = table_ident[opcode - TOK_IDENT];
	    if (ts->len >= 6
		&& strchr("wlq", ts->str[ts->len-1])
		&& !memcmp(ts->str, "cmov", 4)) {
		opcode = tok_alloc(ts->str, ts->len-1)->tok;
		goto again;
	    }
#ifdef TCC_TARGET_X86_64
            /* Try VEX/EVEX instruction table before giving up */
            asm_vex_opcode(s1, opcode, ops, nb_ops);
            return;
#else
            tcc_error("unknown opcode '%s'", ts->str);
#endif
        }
    }
    /* if the size is unknown, then evaluate it (OPC_B or OPC_WL case) */
    autosize = NBWLX-1;
#ifdef TCC_TARGET_X86_64
    /* XXX the autosize should rather be zero, to not have to adjust this
       all the time.  */
    if ((pa->instr_type & OPC_BWLQ) == OPC_B)
        autosize = NBWLX-2;
#endif
    if (s == autosize) {
	/* Check for register operands providing hints about the size.
	   Start from the end, i.e. destination operands.  This matters
	   only for opcodes accepting different sized registers, lar and lsl
	   are such opcodes.  */
        for(i = nb_ops - 1; s == autosize && i >= 0; i--) {
            if ((ops[i].type & OP_REG) && !(op_type[i] & (OP_CL | OP_DX)))
                s = reg_to_size[ops[i].type & OP_REG];
        }
        if (s == autosize) {
            if ((opcode == TOK_ASM_push || opcode == TOK_ASM_pop) &&
                (ops[0].type & (OP_SEG | OP_IM8S | OP_IM32)))
                s = 2;
	    else if ((opcode == TOK_ASM_push || opcode == TOK_ASM_pop) &&
		     (ops[0].type & OP_EA))
	        s = NBWLX - 2;
            else
                tcc_error("cannot infer opcode suffix");
        }
    }

#ifdef TCC_TARGET_X86_64
    rex64 = 0;
    if (pa->instr_type & OPC_48)
        rex64 = 1;
    else if (s == 3 || (alltypes & OP_REG64)) {
        /* generate REX prefix */
	int default64 = 0;
	for(i = 0; i < nb_ops; i++) {
	    if (op_type[i] == OP_REG64 && pa->opcode != 0xb8) {
		/* If only 64bit regs are accepted in one operand
		   this is a default64 instruction without need for
		   REX prefixes, except for movabs(0xb8).  */
		default64 = 1;
		break;
	    }
	}
	/* XXX find better encoding for the default64 instructions.  */
        if (((opcode != TOK_ASM_push && opcode != TOK_ASM_pop
	      && opcode != TOK_ASM_pushw && opcode != TOK_ASM_pushl
	      && opcode != TOK_ASM_pushq && opcode != TOK_ASM_popw
	      && opcode != TOK_ASM_popl && opcode != TOK_ASM_popq
	      && opcode != TOK_ASM_call && opcode != TOK_ASM_jmp))
	    && !default64)
            rex64 = 1;
    }
#endif

    /* now generates the operation */
    if (OPCT_IS(pa->instr_type, OPC_FWAIT))
        g(0x9b);
    if (seg_prefix)
        g(seg_prefix);
#ifdef TCC_TARGET_X86_64
    /* Generate addr32 prefix if needed */
    for(i = 0; i < nb_ops; i++) {
        if (ops[i].type & OP_EA32) {
	    g(0x67);
	    break;
        }
    }
#endif
    /* generate data16 prefix if needed */
    p66 = 0;
    if (s == 1)
        p66 = 1;
    else {
	/* accepting mmx+sse in all operands --> needs 0x66 to
	   switch to sse mode.  Accepting only sse in an operand --> is
	   already SSE insn and needs 0x66/f2/f3 handling.  */
        for (i = 0; i < nb_ops; i++)
            if ((op_type[i] & (OP_MMX | OP_SSE)) == (OP_MMX | OP_SSE)
	        && ops[i].type & OP_SSE)
	        p66 = 1;
    }
    if (p66)
        g(0x66);

    v = pa->opcode;
    p = v >> 8;  /* possibly prefix byte(s) */
    switch (p) {
        case 0: break;  /* no prefix */
        case 0x48: break; /* REX, handled elsewhere */
        case 0x66:
        case 0x67:
        case 0xf2:
        case 0xf3: v = v & 0xff; g(p); break;
        case 0xd4: case 0xd5: break; /* aam and aad, not prefix, but hardcoded immediate argument "10" */
        case 0xd8: case 0xd9: case 0xda: case 0xdb: /* x87, no normal prefix */
        case 0xdc: case 0xdd: case 0xde: case 0xdf: break;
        default: tcc_error("bad prefix 0x%2x in opcode table", p); break;
    }
    if (pa->instr_type & OPC_0F)
        v = ((v & ~0xff) << 8) | 0x0f00 | (v & 0xff);
    if ((v == 0x69 || v == 0x6b) && nb_ops == 2) {
        /* kludge for imul $im, %reg */
        nb_ops = 3;
        ops[2] = ops[1];
        op_type[2] = op_type[1];
    } else if (v == 0xcd && ops[0].e.v == 3 && !ops[0].e.sym) {
        v--; /* int $3 case */
        nb_ops = 0;
    } else if ((v == 0x06 || v == 0x07)) {
        if (ops[0].reg >= 4) {
            /* push/pop %fs or %gs */
            v = 0x0fa0 + (v - 0x06) + ((ops[0].reg - 4) << 3);
        } else {
            v += ops[0].reg << 3;
        }
        nb_ops = 0;
    } else if (v <= 0x05) {
        /* arith case */
        v += ((opcode - TOK_ASM_addb) / NBWLX) << 3;
    } else if ((pa->instr_type & (OPCT_MASK | OPC_MODRM)) == OPC_FARITH) {
        /* fpu arith case */
        v += ((opcode - pa->sym) / 6) << 3;
    }

    /* search which operand will be used for modrm */
    modrm_index = -1;
    modreg_index = -1;
    if (pa->instr_type & OPC_MODRM) {
	if (!nb_ops) {
	    /* A modrm opcode without operands is a special case (e.g. mfence).
	       It has a group and acts as if there's an register operand 0 */
	    i = 0;
	    ops[i].type = OP_REG;
#ifdef TCC_TARGET_X86_64
	    if (pa->sym == TOK_ASM_endbr64)
	      ops[i].reg = 2; // dx
	    else if (pa->sym >= TOK_ASM_lfence && pa->sym <= TOK_ASM_sfence)
  	      ops[i].reg = 0; // ax
#else
	    if (pa->sym == TOK_ASM_endbr32)
	      ops[i].reg = 3; // bx
#endif
	    else
	      tcc_error("bad MODR/M opcode without operands");
	    goto modrm_found;
	}
        /* first look for an ea operand */
        for(i = 0;i < nb_ops; i++) {
            if (op_type[i] & OP_EA)
                goto modrm_found;
        }
        /* then if not found, a register or indirection (shift instructions) */
        for(i = 0;i < nb_ops; i++) {
            if (op_type[i] & (OP_REG | OP_MMX | OP_SSE | OP_INDIR))
                goto modrm_found;
        }
#ifdef ASM_DEBUG
        tcc_error("bad op table");
#endif
    modrm_found:
        modrm_index = i;
        /* if a register is used in another operand then it is
           used instead of group */
        for(i = 0;i < nb_ops; i++) {
            int t = op_type[i];
            if (i != modrm_index &&
                (t & (OP_REG | OP_MMX | OP_SSE | OP_CR | OP_TR | OP_DB | OP_SEG))) {
                modreg_index = i;
                break;
            }
        }
    }
#ifdef TCC_TARGET_X86_64
    asm_rex (rex64, ops, nb_ops, op_type, modreg_index, modrm_index);
#endif

    if (pa->instr_type & OPC_REG) {
        /* mov $im, %reg case */
        if (v == 0xb0 && s >= 1)
            v += 7;
        for(i = 0; i < nb_ops; i++) {
            if (op_type[i] & (OP_REG | OP_ST)) {
                v += ops[i].reg;
                break;
            }
        }
    }
    if (pa->instr_type & OPC_B)
        v += s >= 1;
    if (nb_ops == 1 && pa->op_type[0] == OPT_DISP8) {
	ElfSym *esym;
        int jmp_disp;

        /* see if we can really generate the jump with a byte offset */
	esym = elfsym(ops[0].e.sym);
        if (!esym || esym->st_shndx != cur_text_section->sh_num)
            goto no_short_jump;
        jmp_disp = ops[0].e.v + esym->st_value - ind - 2 - (v >= 0xff);
        if (jmp_disp == (int8_t)jmp_disp) {
            /* OK to generate jump */
	    ops[0].e.sym = 0;
            ops[0].e.v = jmp_disp;
	    op_type[0] = OP_IM8S;
        } else {
        no_short_jump:
	    /* long jump will be allowed. need to modify the
	       opcode slightly */
	    if (v == 0xeb) /* jmp */
	        v = 0xe9;
	    else if (v == 0x70) /* jcc */
	        v += 0x0f10;
	    else
	        tcc_error("invalid displacement");
        }
    }
    if (OPCT_IS(pa->instr_type, OPC_TEST))
        v += test_bits[opcode - pa->sym];
    else if (OPCT_IS(pa->instr_type, OPC_0F01))
        v |= 0x0f0100;
    op1 = v >> 16;
    if (op1)
        g(op1);
    op1 = (v >> 8) & 0xff;
    if (op1)
        g(op1);
    g(v);

    if (OPCT_IS(pa->instr_type, OPC_SHIFT)) {
        reg = (opcode - pa->sym) / NBWLX;
        if (reg == 6)
            reg = 7;
    } else if (OPCT_IS(pa->instr_type, OPC_ARITH)) {
        reg = (opcode - pa->sym) / NBWLX;
    } else if (OPCT_IS(pa->instr_type, OPC_FARITH)) {
        reg = (opcode - pa->sym) / 6;
    } else {
        reg = (pa->instr_type >> OPC_GROUP_SHIFT) & 7;
    }

    pc = 0;
    if (pa->instr_type & OPC_MODRM) {
        /* if a register is used in another operand then it is
           used instead of group */
	if (modreg_index >= 0)
	    reg = ops[modreg_index].reg;
        pc = asm_modrm(reg, &ops[modrm_index]);
    }

    /* emit constants */
#ifndef TCC_TARGET_X86_64
    if (!(pa->instr_type & OPC_0F)
	&& (pa->opcode == 0x9a || pa->opcode == 0xea)) {
        /* ljmp or lcall kludge */
	gen_expr32(&ops[1].e);
        if (ops[0].e.sym)
            tcc_error("cannot relocate");
        gen_le16(ops[0].e.v);
        return;
    }
#endif
    for(i = 0;i < nb_ops; i++) {
        v = op_type[i];
        if (v & (OP_IM8 | OP_IM16 | OP_IM32 | OP_IM64 | OP_IM8S | OP_ADDR)) {
            /* if multiple sizes are given it means we must look
               at the op size */
            if ((v | OP_IM8 | OP_IM64) == (OP_IM8 | OP_IM16 | OP_IM32 | OP_IM64)) {
                if (s == 0)
                    v = OP_IM8;
                else if (s == 1)
                    v = OP_IM16;
                else if (s == 2 || (v & OP_IM64) == 0)
                    v = OP_IM32;
                else
                    v = OP_IM64;
            }

            if ((v & (OP_IM8 | OP_IM8S | OP_IM16)) && ops[i].e.sym)
                tcc_error("cannot relocate");

            if (v & (OP_IM8 | OP_IM8S)) {
                g(ops[i].e.v);
            } else if (v & OP_IM16) {
                gen_le16(ops[i].e.v);
#ifdef TCC_TARGET_X86_64
            } else if (v & OP_IM64) {
                gen_expr64(&ops[i].e);
#endif
	    } else if (pa->op_type[i] == OPT_DISP || pa->op_type[i] == OPT_DISP8) {
                gen_disp32(&ops[i].e);
            } else {
                gen_expr32(&ops[i].e);
            }
        }
    }

    /* after immediate operands, adjust pc-relative address */
    if (pc)
        add32le(cur_text_section->data + pc - 4, pc - ind);
}

/* return the constraint priority (we allocate first the lowest
   numbered constraints) */
static inline int constraint_priority(const char *str)
{
    int priority, c, pr;

    /* we take the lowest priority */
    priority = 0;
    for(;;) {
        c = *str;
        if (c == '\0')
            break;
        str++;
        switch(c) {
        case 'A':
            pr = 0;
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'S':
        case 'D':
            pr = 1;
            break;
        case 'q':
            pr = 2;
            break;
        case 'r':
	case 'R':
	case 'p':
            pr = 3;
            break;
        case 'N':
        case 'M':
        case 'I':
	case 'e':
        case 'i':
        case 'm':
        case 'g':
            pr = 4;
            break;
        default:
            tcc_error("unknown constraint '%c'", c);
            pr = 0;
        }
        if (pr > priority)
            priority = pr;
    }
    return priority;
}

static const char *skip_constraint_modifiers(const char *p)
{
    while (*p == '=' || *p == '&' || *p == '+' || *p == '%')
        p++;
    return p;
}

/* If t (a token) is of the form "%reg" or "reg" return the register number and
   type, otherwise return -1. With GCC the % is optional, too. */
ST_FUNC int asm_parse_regvar (int t)
{
    const char *s;
    Operand op;
    if (t < TOK_IDENT || (t & SYM_FIELD))
        return -1;
    s = table_ident[t - TOK_IDENT]->str;
    if (s[0] == '%')
        ++s;
    t = tok_alloc_const(s);
    unget_tok(t);
    /* Internally the % prefix is required. */
    unget_tok('%');
    parse_operand(tcc_state, &op);
    /* Accept only integer regs for now. */
    if (op.type & OP_REG)
        return op.reg;
    else
        return -1;
}

#define REG_OUT_MASK 0x01
#define REG_IN_MASK  0x02

#define is_reg_allocated(reg) (regs_allocated[reg] & reg_mask)

ST_FUNC void asm_compute_constraints(ASMOperand *operands,
                                    int nb_operands, int nb_outputs,
                                    const uint8_t *clobber_regs,
                                    int *pout_reg)
{
    ASMOperand *op;
    int sorted_op[MAX_ASM_OPERANDS];
    int i, j, k, p1, p2, tmp, reg, c, reg_mask;
    const char *str;
    uint8_t regs_allocated[NB_ASM_REGS];

    /* init fields */
    for(i=0;i<nb_operands;i++) {
        op = &operands[i];
        op->input_index = -1;
        op->ref_index = -1;
        op->reg = -1;
        op->is_memory = 0;
        op->is_rw = 0;
    }
    /* compute constraint priority and evaluate references to output
       constraints if input constraints */
    for(i=0;i<nb_operands;i++) {
        op = &operands[i];
        str = op->constraint;
        str = skip_constraint_modifiers(str);
        if (isnum(*str) || *str == '[') {
            /* this is a reference to another constraint */
            k = find_constraint(operands, nb_operands, str, NULL);
            if ((unsigned)k >= i || i < nb_outputs)
                tcc_error("invalid reference in constraint %d ('%s')",
                      i, str);
            op->ref_index = k;
            if (operands[k].input_index >= 0)
                tcc_error("cannot reference twice the same operand");
            operands[k].input_index = i;
            op->priority = 5;
	} else if ((op->vt->r & VT_VALMASK) == VT_LOCAL
		   && op->vt->sym
		   && (reg = op->vt->sym->r & VT_VALMASK) < VT_CONST) {
	    op->priority = 1;
	    op->reg = reg;
        } else {
            op->priority = constraint_priority(str);
        }
    }

    /* sort operands according to their priority */
    for(i=0;i<nb_operands;i++)
        sorted_op[i] = i;
    for(i=0;i<nb_operands - 1;i++) {
        for(j=i+1;j<nb_operands;j++) {
            p1 = operands[sorted_op[i]].priority;
            p2 = operands[sorted_op[j]].priority;
            if (p2 < p1) {
                tmp = sorted_op[i];
                sorted_op[i] = sorted_op[j];
                sorted_op[j] = tmp;
            }
        }
    }

    for(i = 0;i < NB_ASM_REGS; i++) {
        if (clobber_regs[i])
            regs_allocated[i] = REG_IN_MASK | REG_OUT_MASK;
        else
            regs_allocated[i] = 0;
    }
    /* esp cannot be used */
    regs_allocated[4] = REG_IN_MASK | REG_OUT_MASK;
    /* ebp cannot be used yet */
    regs_allocated[5] = REG_IN_MASK | REG_OUT_MASK;

    /* allocate registers and generate corresponding asm moves */
    for(i=0;i<nb_operands;i++) {
        j = sorted_op[i];
        op = &operands[j];
        str = op->constraint;
        /* no need to allocate references */
        if (op->ref_index >= 0)
            continue;
        /* select if register is used for output, input or both */
        if (op->input_index >= 0) {
            reg_mask = REG_IN_MASK | REG_OUT_MASK;
        } else if (j < nb_outputs) {
            reg_mask = REG_OUT_MASK;
        } else {
            reg_mask = REG_IN_MASK;
        }
	if (op->reg >= 0) {
	    if (is_reg_allocated(op->reg))
	        tcc_error("asm regvar requests register that's taken already");
	    reg = op->reg;
	}
    try_next:
        c = *str++;
        switch(c) {
        case '=':
            goto try_next;
        case '+':
            op->is_rw = 1;
            /* FALL THRU */
        case '&':
            if (j >= nb_outputs)
                tcc_error("'%c' modifier can only be applied to outputs", c);
            reg_mask = REG_IN_MASK | REG_OUT_MASK;
            goto try_next;
        case 'A':
            /* allocate both eax and edx */
            if (is_reg_allocated(TREG_XAX) ||
                is_reg_allocated(TREG_XDX))
                goto try_next;
            op->is_llong = 1;
            op->reg = TREG_XAX;
            regs_allocated[TREG_XAX] |= reg_mask;
            regs_allocated[TREG_XDX] |= reg_mask;
            break;
        case 'a':
            reg = TREG_XAX;
            goto alloc_reg;
        case 'b':
            reg = 3;
            goto alloc_reg;
        case 'c':
            reg = TREG_XCX;
            goto alloc_reg;
        case 'd':
            reg = TREG_XDX;
            goto alloc_reg;
        case 'S':
            reg = 6;
            goto alloc_reg;
        case 'D':
            reg = 7;
        alloc_reg:
            if (op->reg >= 0 && reg != op->reg)
                goto try_next;
            if (is_reg_allocated(reg))
                goto try_next;
            goto reg_found;
        case 'q':
            /* eax, ebx, ecx or edx */
            if (op->reg >= 0) {
                if ((reg = op->reg) < 4)
                    goto reg_found;
            } else for(reg = 0; reg < 4; reg++) {
                if (!is_reg_allocated(reg))
                    goto reg_found;
            }
            goto try_next;
        case 'r':
	case 'R':
	case 'p': /* A general address, for x86(64) any register is acceptable*/
            /* any general register */
            if ((reg = op->reg) >= 0)
                goto reg_found;
            else for(reg = 0; reg < NB_ASM_REGS; reg++) {
                if (!is_reg_allocated(reg))
                    goto reg_found;
            }
            goto try_next;
        reg_found:
            /* now we can reload in the register */
            op->is_llong = 0;
            op->reg = reg;
            regs_allocated[reg] |= reg_mask;
            break;
	case 'e':
        case 'i':
            if (!((op->vt->r & (VT_VALMASK | VT_LVAL)) == VT_CONST))
                goto try_next;
            break;
        case 'I':
        case 'N':
        case 'M':
            if (!((op->vt->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST))
                goto try_next;
            break;
        case 'm':
        case 'g':
            /* nothing special to do because the operand is already in
               memory, except if the pointer itself is stored in a
               memory variable (VT_LLOCAL case) */
            /* XXX: fix constant case */
            /* if it is a reference to a memory zone, it must lie
               in a register, so we reserve the register in the
               input registers and a load will be generated
               later */
            if (j < nb_outputs || c == 'm') {
                if ((op->vt->r & VT_VALMASK) == VT_LLOCAL) {
                    /* any general register */
                    for(reg = 0; reg < NB_ASM_REGS; reg++) {
                        if (!(regs_allocated[reg] & REG_IN_MASK))
                            goto reg_found1;
                    }
                    goto try_next;
                reg_found1:
                    /* now we can reload in the register */
                    regs_allocated[reg] |= REG_IN_MASK;
                    op->reg = reg;
                    op->is_memory = 1;
                }
            }
            break;
        default:
            tcc_error("asm constraint %d ('%s') could not be satisfied",
                  j, op->constraint);
            break;
        }
        /* if a reference is present for that operand, we assign it too */
        if (op->input_index >= 0) {
            operands[op->input_index].reg = op->reg;
            operands[op->input_index].is_llong = op->is_llong;
        }
    }

    /* compute out_reg. It is used to store outputs registers to memory
       locations references by pointers (VT_LLOCAL case) */
    *pout_reg = -1;
    for(i=0;i<nb_operands;i++) {
        op = &operands[i];
        if (op->reg >= 0 &&
            (op->vt->r & VT_VALMASK) == VT_LLOCAL  &&
            !op->is_memory) {
            for(reg = 0; reg < NB_ASM_REGS; reg++) {
                if (!(regs_allocated[reg] & REG_OUT_MASK))
                    goto reg_found2;
            }
            tcc_error("could not find free output register for reloading");
        reg_found2:
            *pout_reg = reg;
            break;
        }
    }

    /* print sorted constraints */
#ifdef ASM_DEBUG
    for(i=0;i<nb_operands;i++) {
        j = sorted_op[i];
        op = &operands[j];
        printf("%%%d [%s]: \"%s\" r=0x%04x reg=%d\n",
               j,
               op->id ? get_tok_str(op->id, NULL) : "",
               op->constraint,
               op->vt->r,
               op->reg);
    }
    if (*pout_reg >= 0)
        printf("out_reg=%d\n", *pout_reg);
#endif
}

ST_FUNC void subst_asm_operand(CString *add_str,
                              SValue *sv, int modifier)
{
    int r, reg, size, val;

    r = sv->r;
    if ((r & VT_VALMASK) == VT_CONST) {
        if (!(r & VT_LVAL) && modifier != 'c' && modifier != 'n' &&
	    modifier != 'P')
            cstr_ccat(add_str, '$');
        if (r & VT_SYM) {
	    const char *name = get_tok_str(sv->sym->v, NULL);
	    if (sv->sym->v >= SYM_FIRST_ANOM) {
		/* In case of anonymous symbols ("L.42", used
		   for static data labels) we can't find them
		   in the C symbol table when later looking up
		   this name.  So enter them now into the asm label
		   list when we still know the symbol.  */
		get_asm_sym(tok_alloc_const(name), sv->sym);
	    }
            if (tcc_state->leading_underscore)
              cstr_ccat(add_str, '_');
            cstr_cat(add_str, name, -1);
            if ((uint32_t)sv->c.i == 0)
                goto no_offset;
	    cstr_ccat(add_str, '+');
        }
        val = sv->c.i;
        if (modifier == 'n')
            val = -val;
        cstr_printf(add_str, "%d", val);
    no_offset:;
#ifdef TCC_TARGET_X86_64
        if (r & VT_LVAL)
            cstr_cat(add_str, "(%rip)", -1);
#endif
    } else if ((r & VT_VALMASK) == VT_LOCAL) {
        cstr_printf(add_str, "%d(%%%s)", (int)sv->c.i, get_tok_str(TOK_ASM_xax + 5, NULL));
    } else if (r & VT_LVAL) {
        reg = r & VT_VALMASK;
        if (reg >= VT_CONST)
            tcc_internal_error("");
        cstr_printf(add_str, "(%%%s)", get_tok_str(TOK_ASM_xax + reg, NULL));
    } else {
        /* register case */
        reg = r & VT_VALMASK;
        if (reg >= VT_CONST)
            tcc_internal_error("");

        /* choose register operand size */
        if ((sv->type.t & VT_BTYPE) == VT_BYTE ||
	    (sv->type.t & VT_BTYPE) == VT_BOOL)
            size = 1;
        else if ((sv->type.t & VT_BTYPE) == VT_SHORT)
            size = 2;
#ifdef TCC_TARGET_X86_64
        else if ((sv->type.t & VT_BTYPE) == VT_LLONG ||
		 (sv->type.t & VT_BTYPE) == VT_PTR)
            size = 8;
#endif
        else
            size = 4;
        if (size == 1 && reg >= 4)
            size = 4;

        if (modifier == 'b') {
            if (reg >= 4)
                tcc_error("cannot use byte register");
            size = 1;
        } else if (modifier == 'h') {
            if (reg >= 4)
                tcc_error("cannot use byte register");
            size = -1;
        } else if (modifier == 'w') {
            size = 2;
        } else if (modifier == 'k') {
            size = 4;
#ifdef TCC_TARGET_X86_64
        } else if (modifier == 'q') {
            size = 8;
#endif
        }

        if (reg >= 8) {
            cstr_printf(add_str, "%%r%d%c", reg, (size == 1) ? 'b' : ((size == 2) ? 'w' : ((size == 4) ? 'd' : ' ')));
            return;
        }
        switch(size) {
        case -1:
            reg = TOK_ASM_ah + reg;
            break;
        case 1:
            reg = TOK_ASM_al + reg;
            break;
        case 2:
            reg = TOK_ASM_ax + reg;
            break;
        default:
            reg = TOK_ASM_eax + reg;
            break;
#ifdef TCC_TARGET_X86_64
        case 8:
            reg = TOK_ASM_rax + reg;
            break;
#endif
        }
        cstr_printf(add_str, "%%%s", get_tok_str(reg, NULL));
    }
}

/* generate prolog and epilog code for asm statement */
ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands,
                         int nb_outputs, int is_output,
                         uint8_t *clobber_regs,
                         int out_reg)
{
    uint8_t regs_allocated[NB_ASM_REGS];
    ASMOperand *op;
    int i, reg;

    /* Strictly speaking %Xbp and %Xsp should be included in the
       call-preserved registers, but currently it doesn't matter.  */
#ifdef TCC_TARGET_X86_64
#ifdef TCC_TARGET_PE
    static const uint8_t reg_saved[] = { 3, 6, 7, 12, 13, 14, 15 };
#else
    static const uint8_t reg_saved[] = { 3, 12, 13, 14, 15 };
#endif
#else
    static const uint8_t reg_saved[] = { 3, 6, 7 };
#endif

    /* mark all used registers */
    memcpy(regs_allocated, clobber_regs, sizeof(regs_allocated));
    for(i = 0; i < nb_operands;i++) {
        op = &operands[i];
        if (op->reg >= 0)
            regs_allocated[op->reg] = 1;
    }
    if (!is_output) {
        /* generate reg save code */
        for(i = 0; i < sizeof(reg_saved)/sizeof(reg_saved[0]); i++) {
            reg = reg_saved[i];
            if (regs_allocated[reg]) {
		if (reg >= 8)
		  g(0x41), reg-=8;
                g(0x50 + reg);
            }
        }

        /* generate load code */
        for(i = 0; i < nb_operands; i++) {
            op = &operands[i];
            if (op->reg >= 0) {
                if ((op->vt->r & VT_VALMASK) == VT_LLOCAL &&
                    op->is_memory) {
                    /* memory reference case (for both input and
                       output cases) */
                    SValue sv;
                    sv = *op->vt;
                    sv.r = (sv.r & ~VT_VALMASK) | VT_LOCAL | VT_LVAL;
                    sv.type.t = VT_PTR;
                    load(op->reg, &sv);
                } else if (i >= nb_outputs || op->is_rw) {
                    /* load value in register */
                    load(op->reg, op->vt);
                    if (op->is_llong) {
                        SValue sv;
                        sv = *op->vt;
                        sv.c.i += 4;
                        load(TREG_XDX, &sv);
                    }
                }
            }
        }
    } else {
        /* generate save code */
        for(i = 0 ; i < nb_outputs; i++) {
            op = &operands[i];
            if (op->reg >= 0) {
                if ((op->vt->r & VT_VALMASK) == VT_LLOCAL) {
                    if (!op->is_memory) {
                        SValue sv;
                        sv = *op->vt;
                        sv.r = (sv.r & ~VT_VALMASK) | VT_LOCAL;
			sv.type.t = VT_PTR;
                        load(out_reg, &sv);

			sv = *op->vt;
                        sv.r = (sv.r & ~VT_VALMASK) | out_reg;
                        store(op->reg, &sv);
                    }
                } else {
                    store(op->reg, op->vt);
                    if (op->is_llong) {
                        SValue sv;
                        sv = *op->vt;
                        sv.c.i += 4;
                        store(TREG_XDX, &sv);
                    }
                }
            }
        }
        /* generate reg restore code */
        for(i = sizeof(reg_saved)/sizeof(reg_saved[0]) - 1; i >= 0; i--) {
            reg = reg_saved[i];
            if (regs_allocated[reg]) {
		if (reg >= 8)
		  g(0x41), reg-=8;
                g(0x58 + reg);
            }
        }
    }
}

ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str)
{
    int reg;
#ifdef TCC_TARGET_X86_64
    unsigned int type;
#endif

    if (!strcmp(str, "memory") ||
        !strcmp(str, "cc") ||
	!strcmp(str, "flags"))
        return;
    reg = tok_alloc_const(str);
    if (reg >= TOK_ASM_eax && reg <= TOK_ASM_edi) {
        reg -= TOK_ASM_eax;
    } else if (reg >= TOK_ASM_ax && reg <= TOK_ASM_di) {
        reg -= TOK_ASM_ax;
#ifdef TCC_TARGET_X86_64
    } else if (reg >= TOK_ASM_rax && reg <= TOK_ASM_rdi) {
        reg -= TOK_ASM_rax;
    } else if ((reg = asm_parse_numeric_reg(reg, &type)) >= 0) {
	;
#endif
    } else {
        tcc_error("invalid clobber register '%s'", str);
    }
    clobber_regs[reg] = 1;
}
