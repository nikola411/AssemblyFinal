# Redosled poziva u `Assembly` klasi

Ovaj dokument prati tok izvršavanja kroz `Assembly.cpp`, od pokretanja
asemblera do izlaza. Zasnovan je na trenutnom stanju koda
(`src/assembly/main.cpp`, `src/assembly/AssemblyAdapter.cpp`,
`src/assembly/Assembly.cpp`, `misc/parser.yy`).

## 1. Ulazna tačka — `main.cpp`

```
main()
  ├─ new AssemblyAdapter()
  ├─ Driver driver(debug, adapter)
  ├─ driver.parse(input_file)        ← faza A: parsiranje po liniji
  ├─ adapter->Backpatch()            ← faza B: razrešavanje forward-referenci
  └─ adapter->GenerateOutput(output_file)  ← faza C: ispis
```

`Assembly` objekat živi unutar `AssemblyAdapter` (član `assembly`). Konstruktor
`Assembly::Assembly()` se poziva pri kreiranju adaptera — postavlja
`mCurrentSection` na `.GLOBAL_DATA` i puni tabelu instrukcija
(`PopulateInstructionsMap()`).

## 2. Faza A — parsiranje po liniji

Bison parser (`parser.yy`) za svaku prepoznatu liniju zove, redom:

```
AssemblyAdapter::SetInstruction(...)        // postavlja mCurrentInstruction
AssemblyAdapter::SetOperand(...)            // ili SetMultipleOperands(...)
AssemblyAdapter::FinishInstruction()        // dispečuje dalje
```

`FinishInstruction()` gleda `mCurrentInstruction->type` i poziva tačno JEDNU
od tri `Assembly` metode:

```
FinishInstruction()
  ├─ type == LABEL      → Assembly::HandleLabel()
  ├─ type == DIRECTIVE  → Assembly::HandleDirective()
  └─ (ostalo)           → Assembly::HandleInstruction()
```

Posle poziva, linija se dodaje u `mProgram` (istorija svih instrukcija).

Ako je `mEnd == true` (već obrađena `.END` direktiva), `FinishInstruction()`
odmah izlazi bez ičega — parsiranje posle `.END` se ignoriše.

### 2.1 `HandleLabel()`

- Traži postojeći simbol po imenu (`GetSymbol`); ako je već `defined`, baca
  grešku (`SymbolAlreadyDefined`).
- Ako simbol ne postoji, pravi novi `Symbol` zapis.
- Osigurava da postoji `<sekcija>.pool` zapis u tabeli simbola (ako ne
  postoji, pravi ga).
- Popunjava `entry`: `name`, `section`, `offset` (= `locationCounter`
  trenutne sekcije), `defined = true`.
- Upisuje labelin offset u bazen literala (`InsertLiteralInPool`), postavlja
  `entry->value` i `entry->isBig = true`.

### 2.2 `HandleDirective()`

`switch` po `mCurrentInstruction->identifier`:

| Direktiva | Šta radi |
|---|---|
| `GLOBAL` | Nalazi ili pravi simbol za svaki operand, postavlja `isGlobal = true`. |
| `SECTION` | Zatvara trenutnu sekciju (`mSectionTable.push_back`), pravi novu `mCurrentSection`, pravi simbol za ime nove sekcije (`defined=true`, `offset=0`). |
| `EXTERN` | Za svaki operand pravi NOV simbol (bez provere postojećeg!), `defined=false`, `isExtern=true`. |
| `WORD` | Za svaki operand: ako je literal, upisuje ga direktno; ako je simbol, zove `GetSymbolValue(operand)`, pa upisuje rezultat u sekciju. |
| `SKIP` | Dodaje N nula-bajtova u trenutnu sekciju. |
| `END` | Postavlja `mEnd = true`, zatvara poslednju sekciju. |

### 2.3 `HandleInstruction()`

```
HandleInstruction()
  ├─ DecodeInstructionValues()
  │    └─ za svaki operand instrukcije:
  │         ├─ (ako ima offset) CalculateOperandOffset(operand)
  │         └─ CalculateOperandValue(operand)
  │              ├─ GPR/CSR operand  → direktna konverzija string→enum
  │              ├─ LTR operand      → GetLiteralValue(operand)
  │              └─ SYM operand      → GetSymbolValue(operand)
  └─ WriteInstructionToSection(mCurrentInstruction)
       └─ za svako polje instrukcije (opcode, mod, regA...):
            GetOperandValue(...) → upis u binarni oblik → AppendData u sekciju
```

`GetSymbolValue`/`GetLiteralValue`/`CalculateOperandOffset` su tačke gde se
odlučuje: mala vrednost (upisuje se direktno) ili velika vrednost (ide kroz
bazen literala + generiše relokaciju), i gde se — ako simbol još nije viđen —
zove `GenerateForwardReference(name)` (upisuje u `mForwardRefTable`, da bi se
razrešilo kasnije, u fazi B).

## 3. Faza B — `Backpatch()` → `ContinueParsing()`

Poziva se JEDNOM, tek pošto je ceo fajl pročitan (sve labele i sekcije su
sad poznate).

```
ContinueParsing()
  ├─ za svaku stavku u mForwardRefTable:
  │    ├─ nadji simbol (GetSymbol) — ako ne postoji, baci UndefinedSymbol
  │    ├─ nadji sekciju u kojoj je referenca nastala
  │    ├─ ako simbol pripada DRUGOJ sekciji → generiši REL12_PC (sekcija) +
  │    │    REL32_ABS (bazen) relokaciju
  │    ├─ ako je vrednost mala → upiši direktno (WriteInstructionDisplacement)
  │    └─ inače → upiši kroz bazen + REL12_PC relokacija
  ├─ CheckForErrors()
  │    └─ za svaki simbol: proveri isGlobal+!defined, defined+isExtern,
  │         isGlobal+isExtern — baca AssemblyException ako nešto ne važi
  └─ ispis tabela na std::cout (SymbolTableToString, SectionTableToString)
```

## 4. Faza C — `GenerateOutput(output)`

Piše TEKSTUALNI (ne binarni ELF) prikaz `mSymbolTable`/`mSectionTable` u
izlazni fajl, preko `SymbolTableToString`/`SectionTableToString`.

**Napomena:** ovaj main tok se završava ovde — `Elf::LoadLinkable` (koji
pravi stvarni `.o` ELF fajl) se odavde nigde ne poziva. Vezivanje asemblera
sa ELF slojem nije deo ovog toka.

## Sažetak — ceo redosled od početka do kraja

```
main()
 └─ driver.parse()                         [faza A, po liniji, N puta]
     └─ FinishInstruction()
         ├─ HandleLabel()
         ├─ HandleDirective()
         └─ HandleInstruction()
             ├─ DecodeInstructionValues()
             │   ├─ CalculateOperandOffset()
             │   └─ CalculateOperandValue()
             │       ├─ GetLiteralValue()
             │       └─ GetSymbolValue()  → (moguće) GenerateForwardReference()
             └─ WriteInstructionToSection()
 └─ adapter->Backpatch()                   [faza B, jednom]
     └─ ContinueParsing()
         └─ CheckForErrors()
 └─ adapter->GenerateOutput()              [faza C, jednom]
```
