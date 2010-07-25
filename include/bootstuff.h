#ifndef loadMiiBootstuff
#define loadMiiBootstuff

#include <gccore.h>

#define maxTextSections 7
#define maxDataSections 11

typedef struct _dolheader 
{
	u32 textoff [maxTextSections];
	u32 dataoff [maxDataSections];
	u32 textmem [maxTextSections];
	u32 datamem [maxDataSections];
	u32 textsize[maxTextSections];
	u32 datasize[maxDataSections];
	u32 bssmem;
	u32 bsssize;
	u32 entry;
} dolheader;

#define MACHINE_PPC 	20
#define EI_NIDENT 	16
#define SHT_STRTAB	3
#define SHT_NOBITS	8
#define SHF_ALLOC	2


typedef struct elfhdr{
	u8	e_ident[EI_NIDENT]; /* ELF Identification */
	u16	e_type;		/* object file type */
	u16	e_machine;	/* machine */
	u32	e_version;	/* object file version */
	u32	e_entry;	/* virtual entry point */
	u32	e_phoff;	/* program header table offset */
	u32	e_shoff;	/* section header table offset */
	u32	e_flags;	/* processor-specific flags */
	u16	e_ehsize;	/* ELF header size */
	u16	e_phentsize;	/* program header entry size */
	u16	e_phnum;	/* number of program header entries */
	u16	e_shentsize;	/* section header entry size */
	u16	e_shnum;	/* number of section header entries */
	u16	e_shstrndx;	/* section header table's "section
				   header string table" entry offset */
} Elf32_Ehdr;

typedef struct {
	u32	sh_name;	/* name - index into section header
					   string table section */
	u32	sh_type;	/* type */
	u32	sh_flags;	/* flags */
	u32	sh_addr;	/* address */
	u32	sh_offset;	/* file offset */
	u32	sh_size;	/* section size */
	u32	sh_link;	/* section header table index link */
	u32	sh_info;	/* extra information */
	u32	sh_addralign;	/* address alignment */
	u32	sh_entsize;	/* section entry size */
} Elf32_Shdr;

int validateHeader(u8 *buffer);
u32 relocateDol (u8 *buffer, struct __argv *argv);
u32 relocateElf (u8 *addr);

#endif
