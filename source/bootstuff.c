#include <stdio.h>
#include <string.h>
#include <gccore.h>

#include "bootstuff.h"
#include "tools.h"

int validateHeader(u8 *buffer)
{
    if (buffer[0] == 0x0 &&
        buffer[1] == 0x0 &&
        buffer[2] == 0x1)
    {
        return 0;
    }
    
    if (buffer[0] == 0x7F &&
        buffer[1] == 'E' &&
        buffer[2] == 'L' &&
        buffer[3] == 'F')
    {
        return 1;
    }

    return 0xdeadbeef;
}

u32 relocateDol (u8 *buffer, struct __argv *argv)
{
        int loop;
        dolheader *hdr = (dolheader *)buffer;
        memset((void *)hdr->bssmem, 0, hdr->bsssize);
		printf("BSS @ %08x (%08x)\n", hdr->bssmem, hdr->bsssize);
        for (loop = 0; loop < maxTextSections; loop++)
        {
			if (hdr->textsize[loop])
			{	
				printf("Text @ %08x (%08x)\n", hdr->textmem[loop], hdr->textsize[loop]);
                memcpy((void *)hdr->textmem[loop], buffer + hdr->textoff[loop], hdr->textsize[loop]);
				DCFlushRange((void *)hdr->textmem[loop], hdr->textsize[loop]);
				ICInvalidateRange((void *)hdr->textmem[loop], hdr->textsize[loop]);
			}
        }
        for (loop = 0; loop < maxDataSections; loop++)
        {
			if (hdr->datasize[loop])
			{
				printf("Data @ %08x (%08x)\n", hdr->datamem[loop], hdr->datasize[loop]);
                memcpy((void *)hdr->datamem[loop], buffer + hdr->dataoff[loop], hdr->datasize[loop]);
				DCFlushRange((void *)hdr->datamem[loop], hdr->datasize[loop]);
			}
        }
		if (argv && argv->argvMagic == ARGV_MAGIC)
		{
			memmove((void *)(hdr->entry + 8), argv, sizeof(*argv));
			DCFlushRange((void *)(hdr->entry + 8), sizeof(*argv));
		}
		printf("entry %08x\n", hdr->entry);
        return hdr->entry;
}        

u32 relocateElf (u8 *addr)
{
        Elf32_Ehdr *ehdr;
        Elf32_Shdr *shdr;
        u8 *strtab = 0;
        u8 *image;
        int i;

        ehdr = (Elf32_Ehdr *) addr;
        /* Check if the elf is a Powerpc one. */
        if (ehdr->e_machine != MACHINE_PPC)
        {
                setError(3);
				return 0;
        }
        /* Find the section header string table for output info */
        shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
                               (ehdr->e_shstrndx * sizeof (Elf32_Shdr)));

        if (shdr->sh_type == SHT_STRTAB)
                strtab = (u8 *) (addr + shdr->sh_offset);

        /* Load each appropriate section */
        for (i = 0; i < ehdr->e_shnum; ++i) {
                shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
                                       (i * sizeof (Elf32_Shdr)));

                if (!(shdr->sh_flags & SHF_ALLOC)
                   || shdr->sh_addr == 0 || shdr->sh_size == 0) {
                        continue;
                }
                
                shdr->sh_addr &= 0x3FFFFFFF;
                shdr->sh_addr |= 0x80000000;

                if (shdr->sh_type == SHT_NOBITS) {
                        memset ((void *) shdr->sh_addr, 0, shdr->sh_size);
                } else {
                        image = (u8 *) addr + shdr->sh_offset;
                        memcpy ((void *) shdr->sh_addr,
                                (const void *) image,
                                shdr->sh_size);
                }
                DCFlushRangeNoSync ((void *) shdr->sh_addr, shdr->sh_size);
        }

        return (ehdr->e_entry & 0x3FFFFFFF) | 0x80000000;
}
