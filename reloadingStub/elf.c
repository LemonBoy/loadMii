// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "loader.h"

// Returns the entry point address.

typedef struct
{
	u32 textoff [7];
	u32 dataoff [11];
	u32 textmem [7];
	u32 datamem [11];
	u32 textsize[7];
	u32 datasize[11];
	u32 bssmem;
	u32 bsssize;
	u32 entry;
} dol;

void *load_dol_image(void *addr)
{
	dol *header = (dol *)addr;
	int sect;
	
	memset((void*)header->bssmem, 0, header->bsssize);
	
	for (sect=0;sect<7;sect++)
	{
		memcpy((void*)header->textmem[sect], addr + header->textoff[sect], header->textsize[sect]);
		sync_before_exec((void*)header->textmem[sect], header->textsize[sect]);
	}
	
	for (sect=0;sect<11;sect++)
	{
		memcpy((void*)header->datamem[sect], addr + header->dataoff[sect], header->datasize[sect]);
		sync_before_exec((void*)header->datamem[sect], header->datasize[sect]);		
	}
	
	return (void *)header->entry;
}
