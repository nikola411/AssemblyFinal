import sys
sys.path.insert(0, '/home/nikola/.local/lib/python3.12/site-packages')
from fpdf import FPDF
from fpdf.enums import XPos, YPos

class PDF(FPDF):
    def header(self):
        self.set_font('Helvetica', 'B', 10)
        self.set_text_color(100, 100, 100)
        self.cell(0, 8, 'ELF - Executable and Linkable Format', align='R', new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(2)
        self.set_draw_color(180, 180, 180)
        self.line(10, self.get_y(), 200, self.get_y())
        self.ln(3)

    def footer(self):
        self.set_y(-15)
        self.set_font('Helvetica', 'I', 8)
        self.set_text_color(150, 150, 150)
        self.cell(0, 10, str(self.page_no()), align='C')

    def h1(self, txt):
        self.ln(4)
        self.set_font('Helvetica', 'B', 13)
        self.set_text_color(20, 60, 120)
        self.set_fill_color(230, 238, 255)
        self.cell(0, 8, txt, fill=True, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(2)

    def h2(self, txt):
        self.ln(3)
        self.set_font('Helvetica', 'B', 10)
        self.set_text_color(50, 50, 50)
        self.cell(0, 6, txt, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(1)

    def body(self, txt):
        self.set_font('Helvetica', '', 10)
        self.set_text_color(40, 40, 40)
        self.multi_cell(0, 6, txt)
        self.ln(1)

    def bullet(self, txt):
        self.set_font('Helvetica', '', 10)
        self.set_text_color(40, 40, 40)
        self.set_x(12)
        self.multi_cell(0, 6, '  * ' + txt)

    def code_block(self, txt):
        self.set_font('Courier', '', 8)
        self.set_fill_color(245, 245, 245)
        self.set_text_color(30, 30, 30)
        lines = txt.strip().split('\n')
        self.ln(1)
        y0 = self.get_y()
        for line in lines:
            self.set_x(14)
            self.cell(0, 5, line, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        y1 = self.get_y()
        self.set_fill_color(245, 245, 245)
        self.rect(12, y0 - 1, 186, y1 - y0 + 2, 'F')
        self.set_y(y0)
        for line in lines:
            self.set_x(14)
            self.cell(0, 5, line, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(2)

    def legend(self, items):
        self.set_font('Helvetica', 'I', 8)
        self.set_text_color(110, 110, 110)
        text = '   |   '.join(f'{k} = {v}' for k, v in items)
        self.set_x(12)
        self.multi_cell(0, 5, text)
        self.ln(1)

    def table(self, headers, rows, col_widths=None):
        if col_widths is None:
            w = 186 // len(headers)
            col_widths = [w] * len(headers)
        self.set_font('Helvetica', 'B', 9)
        self.set_fill_color(50, 90, 150)
        self.set_text_color(255, 255, 255)
        self.set_x(12)
        for i, h in enumerate(headers):
            self.cell(col_widths[i], 7, h, border=1, fill=True)
        self.ln()
        self.set_font('Courier', '', 8)
        self.set_text_color(30, 30, 30)
        for ri, row in enumerate(rows):
            self.set_fill_color(248, 248, 248) if ri % 2 == 0 else self.set_fill_color(255, 255, 255)
            self.set_x(12)
            for i, cell in enumerate(row):
                self.cell(col_widths[i], 6, str(cell), border=1, fill=True)
            self.ln()
        self.ln(2)


pdf = PDF()
pdf.add_page()

# Title
pdf.set_font('Helvetica', 'B', 26)
pdf.set_text_color(20, 60, 120)
pdf.ln(8)
pdf.cell(0, 14, 'ELF', align='C', new_x=XPos.LMARGIN, new_y=YPos.NEXT)
pdf.set_font('Helvetica', 'I', 13)
pdf.set_text_color(80, 80, 80)
pdf.cell(0, 8, 'Executable and Linkable Format', align='C', new_x=XPos.LMARGIN, new_y=YPos.NEXT)
pdf.ln(4)
pdf.set_draw_color(60, 100, 160)
pdf.set_line_width(0.8)
pdf.line(10, pdf.get_y(), 200, pdf.get_y())
pdf.set_line_width(0.2)
pdf.ln(8)

# 1. Pregled
pdf.h1('1. Pregled')
pdf.body(
    'ELF (Executable and Linkable Format) je standardni binarni format na UNIX-like sistemima. '
    'Koristi se za objektne fajlove (.o), izvrsne programe i deljene biblioteke (.so). '
    'Isti format pokriva sve tri faze: kompajliranje, povezivanje (linking) i izvrsavanje.'
)
pdf.table(
    ['Tip fajla', 'Ekstenzija', 'Opis'],
    [
        ['relocatable', '.o', 'Izlaz kompajtlera; sadrzi masinski kod ali nedovrsene adrese. Namenjen linker-u.'],
        ['executable', '(bez)', 'Gotov program koji OS moze pokrenuti. Sve adrese su razresene.'],
        ['shared object', '.so', 'Deljeni kod koji se moze ulinkovati staticno ili ucitati dinamicki u toku izvrsavanja.'],
    ],
    [30, 20, 136]
)

# 2. Struktura i dva pogleda
pdf.h1('2. Struktura ELF datoteke')
pdf.body(
    'Svaka ELF datoteka pocinje obaveznim ELF headerom koji opisuje celu datoteku. '
    'Iza headera mogu slediti Program Header Table (PHT), sekcije i Section Header Table (SHT). '
    'Redosled nije fiksiran; sve lokacije su opisane ofsetima u ELF headeru.'
)
pdf.code_block(
    '+-----------------------+\n'
    '|      ELF Header       |  uvek prisutan\n'
    '+-----------------------+\n'
    '| Program Header Table  |  obavezan pri izvrsavanju, opcioni pri linkovanju\n'
    '+-----------------------+\n'
    '|    Sekcije (.text,    |\n'
    '|    .data, .symtab...) |\n'
    '+-----------------------+\n'
    '| Section Header Table  |  obavezna pri linkovanju, opciona pri izvrsavanju\n'
    '+-----------------------+'
)
pdf.legend([
    ('ELF', 'Executable and Linkable Format'),
    ('PHT', 'Program Header Table'),
    ('SHT', 'Section Header Table'),
])
pdf.body(
    'ELF datoteka se moze posmatrati na dva nacina:\n\n'
    'Linking view (pogled pri linkovanju) - koristi Section Header Table. '
    'Opisuje sadrzaj datoteke kao kolekciju sekcija (.text, .data, .symtab, .rela.text, ...). '
    'Linker cita sekcije iz vise .o fajlova, resava simbole i spaja ih u jedan izvrsni program.\n\n'
    'Execution view (pogled pri izvrsavanju) - koristi Program Header Table. '
    'Opisuje sadrzaj kao segmente koje OS loader preslikava u virtuelni adresni prostor procesa. '
    'Vise sekcija moze biti spojeno u jedan segment radi efikasnosti (npr. .text i .rodata -> RX segment).'
)

# 3. Primitivni tipovi
pdf.h1('3. Primitivni tipovi (ELF64)')
pdf.body(
    'ELF specifikacija definise sopstvene tipove podataka kako bi bila nezavisna od konkretnog kompajtlera. '
    'Poravnanje je uvek jednako velicini tipa, sto garantuje da strukturna polja ne zahtevaju eksplicitni padding.'
)
pdf.table(
    ['Tip', 'Vel. (B)', 'Znacenje'],
    [
        ['Elf64_Addr', '8', 'Virtuelna ili fizicka adresa'],
        ['Elf64_Off',  '8', 'Offset u okviru ELF fajla'],
        ['Elf64_Xword','8', 'Unsigned 64-bit ceo broj'],
        ['Elf64_Sxword','8','Signed 64-bit ceo broj'],
        ['Elf64_Word', '4', 'Unsigned 32-bit ceo broj'],
        ['Elf64_Sword','4', 'Signed 32-bit ceo broj'],
        ['Elf64_Half', '2', 'Unsigned 16-bit ceo broj'],
        ['unsigned char','1','Unsigned 8-bit ceo broj'],
    ],
    [45, 20, 121]
)

# 4. ELF Header
pdf.h1('4. ELF Header - Elf64_Ehdr')
pdf.body(
    'ELF header se uvek nalazi na offsetu 0 i ima fiksnu velicinu (64 bajta za ELF64). '
    'Sadrzi sve informacije potrebne da se lociraju ostale tabele i strukture unutar fajla.'
)
pdf.code_block(
    'typedef struct {\n'
    '    unsigned char  e_ident[16]; // magic, class, endianness, ABI...\n'
    '    Elf64_Half     e_type;      // tip fajla: ET_REL, ET_EXEC, ET_DYN\n'
    '    Elf64_Half     e_machine;   // ciljna arhitektura (npr. EM_X86_64 = 62)\n'
    '    Elf64_Word     e_version;   // uvek EV_CURRENT = 1\n'
    '    Elf64_Addr     e_entry;     // virtuelna adresa _start(); 0 za .o fajlove\n'
    '    Elf64_Off      e_phoff;     // pozicija PHT u fajlu; 0 ako PHT ne postoji\n'
    '    Elf64_Off      e_shoff;     // pozicija SHT u fajlu; 0 ako SHT ne postoji\n'
    '    Elf64_Word     e_flags;     // zastavice specificne za arhitekturu\n'
    '    Elf64_Half     e_ehsize;    // velicina ovog headera (= 64 za ELF64)\n'
    '    Elf64_Half     e_phentsize; // velicina jednog unosa u PHT\n'
    '    Elf64_Half     e_phnum;     // broj unosa u PHT\n'
    '    Elf64_Half     e_shentsize; // velicina jednog unosa u SHT\n'
    '    Elf64_Half     e_shnum;     // broj unosa u SHT\n'
    '    Elf64_Half     e_shstrndx;  // indeks SHT unosa za .shstrtab sekciju\n'
    '} Elf64_Ehdr;'
)
pdf.legend([
    ('ET_*', 'ELF Type - konstante za e_type'),
    ('EM_*', 'ELF Machine - konstante za e_machine'),
    ('EI_*', 'ELF Ident - indeksi e_ident niza'),
    ('PHT',  'Program Header Table'),
    ('SHT',  'Section Header Table'),
])

pdf.h2('e_ident[] - identifikacioni niz')
pdf.body(
    'Prvih 16 bajtova svake ELF datoteke. Sadrze informacije koje su nezavisne od arhitekture '
    'i omogucavaju parsiranje ostatka fajla (da li je 32/64-bit, little/big-endian, koji OS ABI).'
)
pdf.table(
    ['Bajt(i)', 'Konstanta', 'Vrednost / znacenje'],
    [
        ['0-3', 'EI_MAG0..MAG3', '0x7f, "E", "L", "F"  - magic bytes, identifikuju ELF fajl'],
        ['4', 'EI_CLASS', 'ELFCLASS32=1 (32-bit ELF),  ELFCLASS64=2 (64-bit ELF)'],
        ['5', 'EI_DATA', 'ELFDATA2LSB=1 (little-endian),  ELFDATA2MSB=2 (big-endian)'],
        ['6', 'EI_VERSION', 'EV_CURRENT=1  (uvek 1, jedina definisana verzija)'],
        ['7', 'EI_OSABI', 'ELFOSABI_NONE=0 (System V),  ELFOSABI_LINUX=3'],
        ['8', 'EI_ABIVERSION', 'Verzija ABI-ja; za OSABI_NONE uglavnom 0'],
        ['9-15', 'EI_PAD', 'Nekorisceni bajtovi, moraju biti 0'],
    ],
    [18, 42, 126]
)
pdf.legend([
    ('ABI',  'Application Binary Interface - konvencija za pozive funkcija, tip podataka, sistemske pozive'),
    ('OS',   'Operating System'),
])

pdf.h2('e_type - tip fajla')
pdf.table(
    ['Konstanta', 'Vrednost', 'Znacenje'],
    [
        ['ET_REL',  '1', 'Relocatable - izlaz kompajtlera, ulaz linkera'],
        ['ET_EXEC', '2', 'Executable - gotov program sa fiksnim adresama'],
        ['ET_DYN',  '3', 'Shared object - .so biblioteka ili PIE izvrsni program'],
        ['ET_CORE', '4', 'Core dump - snimak stanja procesa pri padu'],
    ],
    [50, 20, 116]
)
pdf.legend([
    ('PIE', 'Position Independent Executable - izvrsni program bez fiksnih adresa, ucitava se na proizvoljnu adresu'),
])

pdf.h2('e_machine - ciljna arhitektura (izbor)')
pdf.table(
    ['Konstanta', 'Vrednost', 'Arhitektura'],
    [
        ['EM_386',   '3',   'Intel 32-bit x86'],
        ['EM_ARM',   '40',  'ARM 32-bit'],
        ['EM_X86_64','62',  'AMD/Intel 64-bit (x86-64)'],
        ['EM_AARCH64','183','ARM 64-bit (AArch64)'],
        ['EM_RISCV', '243', 'RISC-V'],
    ],
    [50, 20, 116]
)

# 5. Section Header Table
pdf.h1('5. Section Header Table - Elf64_Shdr')
pdf.body(
    'Section Header Table (SHT) je niz Elf64_Shdr struktura; svaki unos opisuje jednu sekciju. '
    'Linker koristi SHT da pronadje i interpretira sadrzaj sekcija (.text, .symtab, .rela.text, ...). '
    'Lokacija SHT u fajlu: e_shoff. Broj unosa: e_shnum. Velicina jednog unosa: e_shentsize (= 64B za ELF64). '
    'Unos sa indeksom 0 (SHN_UNDEF) je rezervisan i ima sve nule.'
)
pdf.code_block(
    'typedef struct {\n'
    '    Elf64_Word  sh_name;      // offset u .shstrtab - string sa imenom sekcije\n'
    '    Elf64_Word  sh_type;      // tip sekcije (SHT_PROGBITS, SHT_SYMTAB, ...)\n'
    '    Elf64_Xword sh_flags;     // bitmaska atributa (SHF_ALLOC, SHF_WRITE, ...)\n'
    '    Elf64_Addr  sh_addr;      // virt. adresa u memoriji; 0 ako se ne ucitava\n'
    '    Elf64_Off   sh_offset;    // offset sekcije od pocetka fajla\n'
    '    Elf64_Xword sh_size;      // velicina sekcije u bajtovima\n'
    '    Elf64_Word  sh_link;      // indeks "partnera" - zavisi od tipa, videti tabelu\n'
    '    Elf64_Word  sh_info;      // dodatni kontekst    - zavisi od tipa, videti tabelu\n'
    '    Elf64_Xword sh_addralign; // zahtev za poravnanje; 0 ili 1 znaci bez zahteva\n'
    '    Elf64_Xword sh_entsize;   // velicina jednog zapisa ako je sekcija tabela; inace 0\n'
    '} Elf64_Shdr;'
)
pdf.legend([
    ('SHT_*', 'Section Header Type - prefiks konstanti za sh_type'),
    ('SHF_*', 'Section Header Flag - prefiks konstanti za sh_flags'),
    ('SHN_*', 'Section Number - prefiks specijalnih indeksa sekcija'),
])

pdf.h2('sh_type')
pdf.table(
    ['Konstanta', 'Vred.', 'Opis'],
    [
        ['SHT_NULL',    '0', 'Neaktivan unos (rezervisan indeks 0)'],
        ['SHT_PROGBITS','1', 'Podaci definisani programom (.text, .data, .rodata)'],
        ['SHT_SYMTAB',  '2', 'Tabela simbola (puna, za debugovanje i linkovanje)'],
        ['SHT_STRTAB',  '3', 'Tabela stringova (null-terminated stringovi jedan za drugim)'],
        ['SHT_RELA',    '4', 'Relokacioni zapisi sa eksplicitnim addend poljem (Elf64_Rela)'],
        ['SHT_NOBITS',  '8', 'Sekcija bez podataka u fajlu (.bss); zauzima prostor u memoriji'],
        ['SHT_REL',     '9', 'Relokacioni zapisi bez addend polja; addend je u masinskom kodu'],
        ['SHT_DYNSYM', '11', 'Redukovana tabela simbola za dinamicko povezivanje'],
    ],
    [52, 14, 120]
)

pdf.h2('sh_flags - bitmaska atributa sekcije')
pdf.table(
    ['Konstanta', 'Znacenje'],
    [
        ['SHF_WRITE',    'Sekcija je upisiva tokom izvrsavanja (npr. .data, .bss)'],
        ['SHF_ALLOC',    'OS loader preslikava sekciju u memoriju (npr. .text, .data, .bss)'],
        ['SHF_EXECINSTR','Sekcija sadrzi izvrsne instrukcije (npr. .text)'],
    ],
    [52, 134]
)

pdf.h2('sh_link i sh_info - veze izmedju sekcija')
pdf.body(
    'Sekcije koje su tabele (simboli, relokacije) moraju ukazivati na pratecu sekciju. '
    'sh_link i sh_info sluze za tu vezu; njihovo znacenje zavisi od tipa sekcije:'
)
pdf.table(
    ['sh_type', 'sh_link (partner sekcija)', 'sh_info (kontekst)'],
    [
        ['SHT_SYMTAB / SHT_DYNSYM',
         'Indeks .strtab sekcije koja cuva imena simbola',
         'Indeks prvog globalnog simbola (svi pre njega su lokalni)'],
        ['SHT_RELA / SHT_REL',
         'Indeks .symtab sekcije ciji simboli se referenciraju',
         'Indeks sekcije na ciji kod se ove relokacije primenjuju'],
        ['SHT_STRTAB / ostalo', 'SHN_UNDEF (0)', '0'],
    ],
    [52, 70, 64]
)

pdf.h2('Rezervisani indeksi sekcija (sh_shndx)')
pdf.body(
    'Ove vrednosti se koriste u polju st_shndx tabele simbola da oznace specijalne situacije '
    'umesto stvarnog indeksa sekcije:'
)
pdf.table(
    ['Konstanta', 'Vrednost', 'Znacenje'],
    [
        ['SHN_UNDEF', '0',      'Nedefinisan simbol - referenca na spoljni simbol koji jos nije resolvan'],
        ['SHN_ABS',   '0xfff1', 'Apsolutna vrednost - st_value simbola se ne menja ni pri relokaciji'],
        ['SHN_COMMON','0xfff2', 'Common simbol - neunificovani BSS objekat, velicina u st_value'],
    ],
    [52, 22, 112]
)
pdf.legend([
    ('BSS', 'Block Started by Symbol - sekcija za neinicijalizovane podatke; OS ih nuluje pri ucitavanju'),
])

# 6. Specijalne sekcije
pdf.h1('6. Standardne sekcije')
pdf.body(
    'Konvencijom definisana imena i tipovi sekcija. Linker i OS loader ih prepoznaju po imenu i tipu.'
)
pdf.table(
    ['Sekcija', 'Tip', 'Atributi', 'Sadrzaj'],
    [
        ['.text',      'SHT_PROGBITS', 'ALLOC+EXEC',       'Izvrsne instrukcije'],
        ['.data',      'SHT_PROGBITS', 'ALLOC+WRITE',      'Inicijalizovani globalni i staticni podaci'],
        ['.bss',       'SHT_NOBITS',   'ALLOC+WRITE',      'Neinicijalizovani podaci; ne zauzima mesto u fajlu'],
        ['.rodata',    'SHT_PROGBITS', 'ALLOC',            'Konstantni podaci (string literali, const varijable)'],
        ['.symtab',    'SHT_SYMTAB',   '-',                'Tabela svih simbola (lokalni + globalni)'],
        ['.strtab',    'SHT_STRTAB',   '-',                'Imena simbola iz .symtab (null-term. stringovi)'],
        ['.shstrtab',  'SHT_STRTAB',   '-',                'Imena sekcija (referencira e_shstrndx)'],
        ['.rela.text', 'SHT_RELA',     '-',                'Relokacioni zapisi za .text sekciju'],
        ['.rel.text',  'SHT_REL',      '-',                'Relokacioni zapisi bez addend-a za .text'],
    ],
    [28, 32, 32, 94]
)
pdf.legend([
    ('ALLOC', 'skraceno za SHF_ALLOC'),
    ('EXEC',  'skraceno za SHF_EXECINSTR'),
    ('WRITE', 'skraceno za SHF_WRITE'),
])

# 7. Symbol Table
pdf.h1('7. Tabela simbola - Elf64_Sym')
pdf.body(
    'Tabela simbola (.symtab) mapira imena na adrese ili vrednosti. '
    'Svaki simbol ima ime (indeks u .strtab), vrednost (adresa ili offset), velicinu i atribute. '
    'Linker koristi ovu tabelu da razresi medjufajlovske reference: kad jedan .o fajl poziva '
    'funkciju iz drugog, linker trazi njen STB_GLOBAL simbol i upisuje njenu adresu na mesto poziva.'
)
pdf.code_block(
    'typedef struct {\n'
    '    Elf64_Word    st_name;  // offset u .strtab - string sa imenom simbola\n'
    '    unsigned char st_info;  // gornja 4 bita: binding; donja 4 bita: tip\n'
    '    unsigned char st_other; // vidljivost (donja 2 bita su STV_* vrednost)\n'
    '    Elf64_Half    st_shndx; // indeks sekcije kojoj simbol pripada\n'
    '    Elf64_Addr    st_value; // u .o fajlu: offset od pocetka sekcije\n'
    '                            // u exe/so fajlu: virtuelna adresa simbola\n'
    '    Elf64_Xword   st_size;  // velicina simbola u bajtovima; 0 ako nije poznata\n'
    '} Elf64_Sym;\n\n'
    '// Makroi za citanje i pisanje st_info polja:\n'
    '#define ELF64_ST_BIND(val)       (((unsigned char)(val)) >> 4)\n'
    '#define ELF64_ST_TYPE(val)       ((val) & 0xf)\n'
    '#define ELF64_ST_INFO(bind,type) (((bind) << 4) + ((type) & 0xf))'
)
pdf.legend([
    ('STB_*', 'Symbol Table Binding - prefiks konstanti za binding'),
    ('STT_*', 'Symbol Table Type - prefiks konstanti za tip simbola'),
    ('STV_*', 'Symbol Table Visibility - prefiks konstanti za vidljivost'),
])

pdf.h2('Binding - gornja 4 bita st_info')
pdf.body('Odredjuje vidljivost i prioritet simbola pri linkovanju:')
pdf.table(
    ['Konstanta', 'Vred.', 'Znacenje'],
    [
        ['STB_LOCAL', '0',
         'Lokalni simbol. Vidljiv samo unutar fajla u kome je definisan. '
         'Dva razlicita .o fajla mogu imati lokalni simbol istog imena bez konflikta.'],
        ['STB_GLOBAL','1',
         'Globalni simbol. Vidljiv linkeru i svim ostalim .o fajlovima. '
         'Vise definicija istog globalnog simbola je greska pri linkovanju.'],
        ['STB_WEAK',  '2',
         'Slabi simbol. Ako postoji i globalni simbol istog imena, '
         'globalni ima prednost. Korisno za "default" implementacije.'],
    ],
    [40, 14, 132]
)

pdf.h2('Tip simbola - donja 4 bita st_info')
pdf.table(
    ['Konstanta', 'Vred.', 'Znacenje'],
    [
        ['STT_NOTYPE', '0', 'Nespecifikovan tip (npr. simboli definisani u linker skripti)'],
        ['STT_OBJECT', '1', 'Podatak - globalna promenljiva, niz, struct'],
        ['STT_FUNC',   '2', 'Funkcija ili drugi izvrsni kod'],
        ['STT_SECTION','3', 'Sekcija - jedan takav simbol po sekciji, STB_LOCAL binding'],
        ['STT_FILE',   '4', 'Ime izvornog fajla - STB_LOCAL, st_shndx = SHN_ABS'],
    ],
    [46, 14, 126]
)

pdf.h2('Vidljivost - st_other (donja 2 bita)')
pdf.table(
    ['Konstanta', 'Vred.', 'Znacenje'],
    [
        ['STV_DEFAULT',   '0', 'Podrazumevano: globalni i slabi simboli su eksportovani iz .so'],
        ['STV_HIDDEN',    '2', 'Simbol nije vidljiv van .so biblioteke (ne moze se dinamicki resolve-ovati)'],
        ['STV_PROTECTED', '3', 'Vidljiv spolja ali se ne moze preemptovati iz druge biblioteke'],
    ],
    [46, 14, 126]
)

# 8. Relocation Table
pdf.h1('8. Tabela relokacija - Elf64_Rela / Elf64_Rel')
pdf.body(
    'Kad kompajler generise .o fajl, ne zna konacne adrese spoljnih simbola ni adrese sekcija. '
    'Umesto toga, upisuje privremenu vrednost (obicno 0) i kreira relokacioni zapis koji govori linkeru: '
    '"na offsetu X u sekciji Y stoji referenca na simbol Z - izracunaj pravu vrednost i upisj je". '
    'Linker prodje kroz sve relokacione zapise i zakrpi masinski kod.'
)
pdf.code_block(
    '// SHT_RELA - aditivna relokacija: addend je eksplicitno u zapisu\n'
    'typedef struct {\n'
    '    Elf64_Addr   r_offset;  // u .o: offset u sekciji; u exe/so: virtuelna adresa\n'
    '    Elf64_Xword  r_info;    // gornja 32b: indeks simbola; donja 32b: tip relokacije\n'
    '    Elf64_Sxword r_addend;  // konstanta koja se dodaje pri izracunavanju\n'
    '} Elf64_Rela;\n\n'
    '// SHT_REL - relokacija bez addend-a: addend se citao iz masinskg koda\n'
    'typedef struct {\n'
    '    Elf64_Addr  r_offset;\n'
    '    Elf64_Xword r_info;\n'
    '} Elf64_Rel;\n\n'
    '// Makroi za citanje r_info polja:\n'
    '#define ELF64_R_SYM(i)          ((i) >> 32)            // indeks simbola\n'
    '#define ELF64_R_TYPE(i)         ((i) & 0xffffffff)     // tip relokacije\n'
    '#define ELF64_R_INFO(sym, type) ((((Elf64_Xword)(sym)) << 32) + (type))'
)

pdf.h2('Kako linker primenjuje relokaciju')
pdf.body(
    'Za svaki relokacioni zapis linker izracunava "patch vrednost" po formuli specificnoj za '
    'tip relokacije i upisuje je na adresu r_offset. '
    'Uobicajene velicine su oznacene formuli:\n\n'
    '  S = st_value simbola na koji relokacija pokazuje (iz tabele simbola)\n'
    '  A = addend (r_addend za Rela, ili vrednost vec upisana u kod za Rel)\n'
    '  P = adresa lokacije koja se zakrpljuje (r_offset nakon preslikavanja)\n\n'
    'Primeri formula za x86-64:\n'
    '  R_X86_64_PC32:  patch = S + A - P  (PC-relativna vrednost; relativna adresa)\n'
    '  R_X86_64_32S:   patch = S + A      (apsolutna 32-bitna vrednost, sign-extended)\n'
    '  R_X86_64_PLT32: patch = L + A - P  (adresa PLT slota, za pozive deljenih funkcija)'
)

pdf.h2('Tipovi relokacija za x86-64 (izbor)')
pdf.table(
    ['Konstanta', 'Vred.', 'Formula', 'Koristeno za'],
    [
        ['R_X86_64_64',   '1',  'S + A',      'Apsolutna 64-bit adresa'],
        ['R_X86_64_PC32', '2',  'S + A - P',  'PC-relativni poziv/skok (call, jmp) u .o fajlovima'],
        ['R_X86_64_PLT32','4',  'L + A - P',  'Poziv funkcije u deljnoj biblioteci (putem PLT)'],
        ['R_X86_64_32',   '10', 'S + A',      'Apsolutna 32-bit adresa (zero-extended)'],
        ['R_X86_64_32S',  '11', 'S + A',      'Apsolutna 32-bit adresa (sign-extended)'],
    ],
    [46, 14, 30, 96]
)
pdf.legend([
    ('PC',  'Program Counter - registar koji cuva adresu tekuce instrukcije (RIP na x86-64)'),
    ('PLT', 'Procedure Linkage Table - tabela indirekcija za pozive funkcija iz deljenih biblioteka'),
    ('L',   'adresa PLT slota za dati simbol'),
])

# 9. Alati
pdf.h1('9. Korisni alati za inspekciju ELF fajlova')
pdf.code_block(
    'readelf -h file.o          # ELF header (tip, arhitektura, offseti tabela)\n'
    'readelf -S file.o          # Section Header Table (sve sekcije sa atributima)\n'
    'readelf -s file.o          # tabela simbola (.symtab)\n'
    'readelf -r file.o          # tabela relokacija (.rela.* ili .rel.*)\n'
    'readelf -l file            # Program Header Table / segmenti\n'
    'objdump -d file.o          # disassembly .text sekcije\n'
    'objdump -t file.o          # tabela simbola (alternativan prikaz)\n'
    'nm file.o                  # kratki prikaz simbola (T=text, D=data, U=undefined)\n'
    'hd -v file.o | head -4     # hex dump prvih 64 bajta (ELF header)'
)
pdf.body(
    'Napomena: nm oznake: T/t (text), D/d (data), B/b (bss), U (undefined/extern), '
    'W/w (weak). Velika slova = globalni simboli, mala = lokalni.'
)

pdf.output('/home/nikola/projects/AssemblyFinal/docs/skolski_elf_docs.pdf')
print('PDF generisan.')
