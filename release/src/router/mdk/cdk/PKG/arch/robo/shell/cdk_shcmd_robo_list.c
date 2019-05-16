/*
 * $Id: cdk_shcmd_robo_list.c,v 1.8 Broadcom SDK $
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * ROBO shell command LIST
 *
 */

#include <cdk/arch/robo_shell.h>

#include <cdk/arch/shcmd_robo_list.h>

#if CDK_CONFIG_SHELL_INCLUDE_LIST == 1

#if CDK_CONFIG_INCLUDE_CHIP_SYMBOLS == 1

typedef struct sym_info_s {    
    int unit; 
    uint32_t flags;
} sym_info_t; 

/*******************************************************************************
 *
 * Prints the information for a single symbol
 *
 *
 ******************************************************************************/

static int
_print_sym(const cdk_symbol_t *s, void *vptr)
{
    sym_info_t *si = (sym_info_t *)vptr;
    uint32_t mask; 
    uint32_t size, min, max; 
    const char *flagstr;
    
    if (si->flags & CDK_SHELL_IDF_RAW) {
        CDK_PRINTF("%s\n", s->name); 
        return 0;
    }

    size = CDK_SYMBOL_INDEX_SIZE_GET(s->index); 
    min = CDK_SYMBOL_INDEX_MIN_GET(s->index); 
    max = CDK_SYMBOL_INDEX_MAX_GET(s->index); 

    CDK_PRINTF("Name:     %s\n", s->name); 
    CDK_PRINTF("Address:  0x%08"PRIx32"\n", s->addr); 
    if (s->flags & CDK_SYMBOL_FLAG_MEMORY) {
        CDK_PRINTF("Size:     %"PRIu32" bytes (%"PRIu32" words)\n", 
                   size, CDK_BYTES2WORDS(size)); 
    } else {
        CDK_PRINTF("Size:     %d-bit\n", (int)size * 8); 
    }

    CDK_PRINTF("Flags:    "); 
    
    for (mask = 1; mask; mask <<= 1) {
        if (s->flags & mask) {
            flagstr = cdk_shell_symflag_type2name(mask);
            if (flagstr != NULL) {
                CDK_PRINTF("%s,", flagstr);
            } 
        }
    }

    CDK_PRINTF("(0x%"PRIx32")\n", s->flags); 

    if ((s->flags & CDK_SYMBOL_FLAG_MEMORY) || (max - min) > 0) {
        CDK_PRINTF("Index:    %"PRIu32":%"PRIx32"\n", min, max); 
    }

#if CDK_CONFIG_INCLUDE_FIELD_INFO == 1
    if (s->fields) {
        CDK_PRINTF("Fields:   %"PRIu32"\n", cdk_field_info_count(s->fields)); 
        cdk_shell_show_fields(s, CDK_ROBO_SYMBOLS(si->unit)->field_names, NULL);
    }
#endif

    CDK_PRINTF("\n"); 
    return 0; 
}
#endif /* CDK_CONFIG_INCLUDE_CHIP_SYMBOLS */

/*******************************************************************************
 *
 * Prints out all symbols containing the input string
 *
 *
 ******************************************************************************/

int
cdk_shcmd_robo_list(int argc, char *argv[])
{
#if CDK_CONFIG_INCLUDE_CHIP_SYMBOLS == 0
    return CDK_SHELL_CMD_NO_SYM; 
#else
    int i; 
    int unit;
    const char *name = ""; 
    const cdk_symbols_t *symbols;
    cdk_shell_tokens_t csts[2]; 
    cdk_symbols_iter_t iter; 
    sym_info_t sym_info;
    uint32_t flags = 0;

    unit = cdk_shell_unit_arg_extract(&argc, argv, 1);
    if(!CDK_DEV_EXISTS(unit)) {
        return CDK_SHELL_CMD_BAD_ARG;
    }
    symbols = CDK_ROBO_SYMBOLS(unit);

    CDK_SHELL_CMD_REQUIRE_SYMBOLS(symbols); 
    CDK_SHELL_CMD_ARGCHECK(0, 2); 

    /* Parse all of our input arguments for options */
    i = 0; 
    if ((argc == 0) || 
        ((i = cdk_robo_shell_parse_args(argc, argv, csts, 2)) >= 0)) {
        /* Error in argument i */
        return cdk_shell_parse_error("symbol", argv[i]); 
    }

    CDK_MEMSET(&iter, 0, sizeof(iter)); 

    /* Look through our arguments */
    for (i = 0; i < argc; i++) {
        /* Flags Specified? */
        if (CDK_STRCMP("flags", csts[i].argv[0]) == 0) {
            cdk_robo_shell_symflag_cst2flags(&csts[i], &iter.pflags, &iter.aflags);
        }
        else if (CDK_STRCMP("raw", csts[i].argv[0]) == 0) {
            flags |= CDK_SHELL_IDF_RAW;
        }
        else {
            /* Symbol expression */
            name = csts[i].argv[0]; 
        }
    }
        
    /* 
     * By default we list all symbols with the input name as a substring
     */
    iter.matching_mode = CDK_SYMBOLS_ITER_MODE_STRSTR; 

    /*
     * The user can specify explicitly the type of matching with the 
     * first character.
     */
    if (name && CDK_STRCMP(name, "*")) {
        switch (name[0]) {
        case '^': 
            iter.matching_mode = CDK_SYMBOLS_ITER_MODE_START; 
            name++; 
            break;
        case '*':
            iter.matching_mode = CDK_SYMBOLS_ITER_MODE_STRSTR;
            name++;
            break; 
        case '@':
            iter.matching_mode = CDK_SYMBOLS_ITER_MODE_EXACT;
            name++;
            break;
        default: 
            break;
        }
    }

    sym_info.unit = unit; 
    sym_info.flags = flags; 

    /* Interate over all matching symbols */
    iter.name = name; 
    iter.symbols = symbols; 
    iter.function = _print_sym; 
    iter.vptr = &sym_info; 
    
    if (cdk_symbols_iter(&iter) <= 0) {
        CDK_PRINTF("no matching symbols\n"); 
    }
    return 0; 
#endif /* CDK_CONFIG_INCLUDE_CHIP_SYMBOLS */
}

#endif