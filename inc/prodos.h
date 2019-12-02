/*  ProDos - Utilitário para manipulação de volumes prodos
 *  by Fabio Belavenuto - Copyright 2004
 *
 *  Este arquivo é distribuido pela Licença Pública Geral GNU.
 *  Veja o arquivo "Licenca.txt" distribuido com este software.
 *
 *  ESTE SOFTWARE NÃO OFERECE NENHUMA GARANTIA
 */

// Definições
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))

#define bitIsSet(nbit, byte) ((byte) & (1 << (nbit)))
#define bitIsNotSet(nbit, byte) (!((byte) & (1 << (nbit))))
#define setBit(nbit, byte)   ((byte) |= (1 << (nbit)))
#define clearBit(nbit, byte) ((byte) &= ~(1 << (nbit)))

//Byte de Acesso:
#define PODESERLIDO     0x01
#define PODESERESCRITO  0x02
#define ARQUIVOMUDADO   0x20
#define PODEMUDARNOME   0x40
#define PODESERDELETADO 0x80

#define PRODOS_TA_DEL   0x00
#define PRODOS_TA_SEED  0x01
#define PRODOS_TA_SAPL  0x02
#define PRODOS_TA_TREE  0x03
#define PRODOS_TA_DIR   0x0D
#define PRODOS_TA_CDIR  0x0E
#define PRODOS_TA_CVOL  0x0F

#define PRODOS_ARQTEXTO 0x04
#define PRODOS_ARQDIR   0x0F
#define PRODOS_ARQBASIC 0xFC

#define PRODOS_MAXARQS  52
#define PRODOS_MAXLENA  15

#define APPLESOFTENDBASE 0x801; // Endereço Base

// Enums
enum Errors {
	ERR_SEMERROS = 0,
	ERR_SEMRECURSOS,
	ERR_ERROLINHACOMANDO,
	ERR_ERROLEITURA,
	ERR_ERROSOMENTELEITURA,
	ERR_NAORECONHECIDO,
	ERR_ARQUIVOEDIRETORIO,
	ERR_ARQUIVONAOACHADO,
	ERR_ARQUIVOJAEXISTE,
	ERR_DIRETORIONAOACHADO,
	ERR_DIRETORIOCHEIO,
	ERR_DISCOCHEIO,
	ERR_ERROAOCRIARARQUIVO,
	ERR_FALTACOMANDO,
	ERR_COMANDONAOIMPLEMENTADO
};

enum Commands {
	CMD_CREATE = 1,
	CMD_DIR,
	CMD_IN,
	CMD_OUT,
	CMD_DELETE,
	CMD_MAKE
};

// Types
typedef unsigned char byte;
typedef unsigned short word;

// Structs
#pragma pack(push, 1)

struct SEntradaDirVol {
	byte tipoArq_LenNome;
	byte nome[PRODOS_MAXLENA];
	byte reservado[8];
	word dataDaCriacao;
	word horaDaCriacao;
	byte versaoProDos;
	byte versaoMinima;
	byte byteDeAcesso;
	byte tamanhoDaEntrada;
	byte entradasPorBloco;
	word entradasAtivas;
	union {
		word dirBlocoParente;				// Usado para diretorio
		word volBlocoBitMap;				// Usado para volume
	};
	union {
		word dirNumTamEntradaParente;		// Usado para diretorio
		word volTotalBlocosDoVolume;		// Usado para volume
	};
};

struct SEntradaArquivo {
	byte tipoArq_LenNome;
	byte nomeArquivo[PRODOS_MAXLENA];
	byte tipoDeArquivo;
	word blocoArquivo;
	word blocosUsados;
	word fimDoArquivoLo;
	byte fimDoArquivoHi;
	word dataCriacao;
	word horaCriacao;
	byte versaoProDos;
	byte versaoMinima;
	byte byteDeAcesso;
	word valorAuxiliar;
	word dataModificacao;
	word horaModificacao;
	word blocoCabecalho;
};

struct SApplesoftLinha {
	word proximoEnd;
	word numeroLinha;
};

#pragma pack(pop)

typedef struct SEntradaDirVol  TEntradaDirVol;
typedef struct SEntradaArquivo TEntradaArquivo;
typedef struct SApplesoftLinha TApplesoftLinha;

struct TTiposArquivo {
	char texto[4];
	int  temEndereco;
};

const struct TTiposArquivo tiposArquivo[256] = {
	{"UNK\0", 0},				// $00
	{"BAD\0", 0},				// $01
	{"PCD\0", 0},				// $02
	{"PTX\0", 0},				// $03
	{"TXT\0", 1},				// $04
	{"PDA\0", 0},				// $05
	{"BIN\0", 1},				// $06
	{"FNT\0", 0},				// $07
	{"FOT\0", 0},				// $08
	{"BA3\0", 0},				// $09
	{"DA3\0", 0},				// $0A
	{"WPF\0", 0},				// $0B
	{"SOS\0", 0},				// $0C
	{"$0D\0", 0},				// $0D
	{"$0E\0", 0},				// $0E
	{"DIR\0", 0},				// $0F
	{"RPD\0", 0},				// $10
	{"RPI\0", 0},				// $11
	{"AFD\0", 0},				// $12
	{"AFM\0", 0},				// $13
	{"AFR\0", 0},				// $14
	{"SCL\0", 0},				// $15
	{"PFS\0", 0},				// $16
	{"$17\0", 0},				// $17
	{"$18\0", 0},				// $18
	{"ADB\0", 0},				// $19
	{"AWP\0", 0},				// $1A
	{"ASP\0", 0},				// $1B
	{"$1C\0", 0},				// $1C
	{"$1D\0", 0},				// $1D
	{"$1E\0", 0},				// $1E
	{"$1F\0", 0},				// $1F
	{"TDM\0", 0},				// $20
	{"IPS\0", 0},				// $21
	{"UPV\0", 0},				// $22
	{"$23\0", 0},				// $23
	{"$24\0", 0},				// $24
	{"$25\0", 0},				// $25
	{"$26\0", 0},				// $26
	{"$27\0", 0},				// $27
	{"$28\0", 0},				// $28
	{"3SD\0", 0},				// $29
	{"8SC\0", 0},				// $2A
	{"8OB\0", 0},				// $2B
	{"8IC\0", 0},				// $2C
	{"8LD\0", 0},				// $2D
	{"P8C\0", 0},				// $2E
	{"$2F\0", 0},				// $2F
	{"$30\0", 0},				// $30
	{"$31\0", 0},				// $31
	{"$32\0", 0},				// $32
	{"$33\0", 0},				// $33
	{"$34\0", 0},				// $34
	{"$35\0", 0},				// $35
	{"$36\0", 0},				// $36
	{"$37\0", 0},				// $37
	{"$38\0", 0},				// $38
	{"$39\0", 0},				// $39
	{"$3A\0", 0},				// $3A
	{"$3B\0", 0},				// $3B
	{"$3C\0", 0},				// $3C
	{"$3D\0", 0},				// $3D
	{"$3E\0", 0},				// $3E
	{"$3F\0", 0},				// $3F
	{"$40\0", 0},				// $40
	{"OCR\0", 0},				// $41
	{"FTD\0", 0},				// $42
	{"$43\0", 0},				// $43
	{"$44\0", 0},				// $44
	{"$45\0", 0},				// $45
	{"$46\0", 0},				// $46
	{"$47\0", 0},				// $47
	{"$48\0", 0},				// $48
	{"$49\0", 0},				// $49
	{"$4A\0", 0},				// $4A
	{"$4B\0", 0},				// $4B
	{"$4C\0", 0},				// $4C
	{"$4D\0", 0},				// $4D
	{"$4E\0", 0},				// $4E
	{"$4F\0", 0},				// $4F
	{"GWP\0", 0},				// $50
	{"GSS\0", 0},				// $51
	{"GDB\0", 0},				// $52
	{"DRW\0", 0},				// $53
	{"GDP\0", 0},				// $54
	{"HMD\0", 0},				// $55
	{"EDU\0", 0},				// $56
	{"STN\0", 0},				// $57
	{"HLP\0", 0},				// $58
	{"COM\0", 0},				// $59
	{"CFG\0", 0},				// $5A
	{"ANM\0", 0},				// $5B
	{"MUM\0", 0},				// $5C
	{"ENT\0", 0},				// $5D
	{"DVU\0", 0},				// $5E
	{"$5F\0", 0},				// $5F
	{"PRE\0", 0},				// $60
	{"$61\0", 0},				// $61
	{"$62\0", 0},				// $62
	{"$63\0", 0},				// $63
	{"$64\0", 0},				// $64
	{"$65\0", 0},				// $65
	{"$66\0", 0},				// $66
	{"$67\0", 0},				// $67
	{"$68\0", 0},				// $68
	{"$69\0", 0},				// $69
	{"$6A\0", 0},				// $6A
	{"BIO\0", 0},				// $6B
	{"$6C\0", 0},				// $6C
	{"DVR\0", 0},				// $6D
	{"PRE\0", 0},				// $6E
	{"HDV\0", 0},				// $6F
	{"$70\0", 0},				// $70
	{"$71\0", 0},				// $71
	{"$72\0", 0},				// $72
	{"$73\0", 0},				// $73
	{"$74\0", 0},				// $74
	{"$75\0", 0},				// $75
	{"$76\0", 0},				// $76
	{"$77\0", 0},				// $77
	{"$78\0", 0},				// $78
	{"$79\0", 0},				// $79
	{"$7A\0", 0},				// $7A
	{"$7B\0", 0},				// $7B
	{"$7C\0", 0},				// $7C
	{"$7D\0", 0},				// $7D
	{"$7E\0", 0},				// $7E
	{"$7F\0", 0},				// $7F
	{"$80\0", 0},				// $80
	{"$81\0", 0},				// $81
	{"$82\0", 0},				// $82
	{"$83\0", 0},				// $83
	{"$84\0", 0},				// $84
	{"$85\0", 0},				// $85
	{"$86\0", 0},				// $86
	{"$87\0", 0},				// $87
	{"$88\0", 0},				// $88
	{"$89\0", 0},				// $89
	{"$8A\0", 0},				// $8A
	{"$8B\0", 0},				// $8B
	{"$8C\0", 0},				// $8C
	{"$8D\0", 0},				// $8D
	{"$8E\0", 0},				// $8E
	{"$8F\0", 0},				// $8F
	{"$90\0", 0},				// $90
	{"$91\0", 0},				// $91
	{"$92\0", 0},				// $92
	{"$93\0", 0},				// $93
	{"$94\0", 0},				// $94
	{"$95\0", 0},				// $95
	{"$96\0", 0},				// $96
	{"$97\0", 0},				// $97
	{"$98\0", 0},				// $98
	{"$99\0", 0},				// $99
	{"$9A\0", 0},				// $9A
	{"$9B\0", 0},				// $9B
	{"$9C\0", 0},				// $9C
	{"$9D\0", 0},				// $9D
	{"$9E\0", 0},				// $9E
	{"$9F\0", 0},				// $9F
	{"WP_\0", 0},				// $A0
	{"$A1\0", 0},				// $A1
	{"$A2\0", 0},				// $A2
	{"$A3\0", 0},				// $A3
	{"$A4\0", 0},				// $A4
	{"$A5\0", 0},				// $A5
	{"$A6\0", 0},				// $A6
	{"$A7\0", 0},				// $A7
	{"$A8\0", 0},				// $A8
	{"$A9\0", 0},				// $A9
	{"$AA\0", 0},				// $AA
	{"GSB\0", 0},				// $AB
	{"TDF\0", 0},				// $AC
	{"BDF\0", 0},				// $AD
	{"$AE\0", 0},				// $AE
	{"$AF\0", 0},				// $AF
	{"SRC\0", 0},				// $B0
	{"OBJ\0", 0},				// $B1
	{"LIB\0", 0},				// $B2
	{"S16\0", 0},				// $B3
	{"RTL\0", 0},				// $B4
	{"EXE\0", 0},				// $B5
	{"STR\0", 0},				// $B6
	{"TSF\0", 0},				// $B7
	{"NDA\0", 0},				// $B8
	{"CDA\0", 0},				// $B9
	{"TOL\0", 0},				// $BA
	{"DRV\0", 0},				// $BB
	{"LDF\0", 0},				// $BC
	{"FST\0", 0},				// $BD
	{"$BE\0", 0},				// $BE
	{"DOC\0", 0},				// $BF
	{"PNT\0", 0},				// $C0
	{"PIC\0", 0},				// $C1
	{"ANI\0", 0},				// $C2
	{"PAL\0", 0},				// $C3
	{"$C4\0", 0},				// $C4
	{"OOG\0", 0},				// $C5
	{"SCR\0", 0},				// $C6
	{"CDV\0", 0},				// $C7
	{"FON\0", 0},				// $C8
	{"FND\0", 0},				// $C9
	{"ICN\0", 0},				// $CA
	{"$CB\0", 0},				// $CB
	{"$CC\0", 0},				// $CC
	{"$CD\0", 0},				// $CD
	{"$CE\0", 0},				// $CE
	{"$CF\0", 0},				// $CF
	{"$D0\0", 0},				// $D0
	{"$D1\0", 0},				// $D1
	{"$D2\0", 0},				// $D2
	{"$D3\0", 0},				// $D3
	{"$D4\0", 0},				// $D4
	{"MUS\0", 0},				// $D5
	{"INS\0", 0},				// $D6
	{"MDI\0", 0},				// $D7
	{"SND\0", 0},				// $D8
	{"$D9\0", 0},				// $D9
	{"$DA\0", 0},				// $DA
	{"DBM\0", 0},				// $DB
	{"$DC\0", 0},				// $DC
	{"$DD\0", 0},				// $DD
	{"$DE\0", 0},				// $DE
	{"$DF\0", 0},				// $DF
	{"SHK\0", 0},				// $E0
	{"$E1\0", 0},				// $E1
	{"DTS\0", 0},				// $E2
	{"$E3\0", 0},				// $E3
	{"$E4\0", 0},				// $E4
	{"$E5\0", 0},				// $E5
	{"$E6\0", 0},				// $E6
	{"$E7\0", 0},				// $E7
	{"$E8\0", 0},				// $E8
	{"$E9\0", 0},				// $E9
	{"$EA\0", 0},				// $EA
	{"$EB\0", 0},				// $EB
	{"$EC\0", 0},				// $EC
	{"$ED\0", 0},				// $ED
	{"R16\0", 0},				// $EE
	{"PAS\0", 0},				// $EF
	{"CMD\0", 0},				// $F0
	{"$F1\0", 0},				// $F1
	{"$F2\0", 0},				// $F2
	{"$F3\0", 0},				// $F3
	{"$F4\0", 0},				// $F4
	{"$F5\0", 0},				// $F5
	{"$F6\0", 0},				// $F6
	{"$F7\0", 0},				// $F7
	{"$F8\0", 0},				// $F8
	{"P16\0", 0},				// $F9
	{"INT\0", 0},				// $FA
	{"IVR\0", 0},				// $FB
	{"BAS\0", 0},				// $FC
	{"VAR\0", 0},				// $FD
	{"REL\0", 0},				// $FE
	{"SYS\0", 0},				// $FF
};

const int setoresProDos[16] = {
	0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 15
};

/* Apple AppleSoft BASIC tokens. */
const char *applesoftTokens[] = {

	" END ",	" FOR ",	" NEXT ",	" DATA ",
	" INPUT ",	" DEL ",	" DIM ",	" READ ",
	" GR ",		" TEXT ",	" PR#",		" IN#",
	" CALL ",	" PLOT ",	" HLIN ",	" VLIN ",
	" HGR2 ",	" HGR ",	" HCOLOR=",	" HPLOT ",
	" DRAW ",	" XDRAW ",	" HTAB ",	" HOME ",
	" ROT=",	" SCALE=",	" SHLOAD ",	" TRACE ",
	" NOTRACE ",	" NORMAL ",	" INVERSE ",	" FLASH ",
	" COLOR=",	" POP ",	" VTAB ",	" HIMEM:",
	" LOMEM:",	" ONERR ",	" RESUME ",	" RECALL ",
	" STORE ",	" SPEED=",	" LET ",	" GOTO ",
	" RUN ",	" IF ",		" RESTORE ",	" & ",
	" GOSUB ",	" RETURN ",	" REM ",	" STOP ",
	" ON ",		" WAIT ",	" LOAD ",	" SAVE ",
	" DEF ",	" POKE ",	" PRINT ",	" CONT ",
	" LIST ",	" CLEAR ",	" GET ",	" NEW ",
	" TAB(",	" TO ",		" FN ",		" SPC(",
	" THEN ",	" AT ",		" NOT ",	" STEP ",
	" + ",		" - ",		" * ",		" / ",
	" ^ ",		" AND ",	" OR ",		" > ",
	" = ",		" < ",		" SGN ",	" INT ",
	" ABS ",	" USR ",	" FRE ",	" SCRN(",
	" PDL ",	" POS ",	" SQR ",	" RND ",
	" LOG ",	" EXP ",	" COS ",	" SIN ",
	" TAN ",	" ATN ",	" PEEK ",	" LEN ",
	" STR$ ",	" VAL ",	" ASC ",	" CHR$ ",
	" LEFT$ ",	" RIGHT$ ",	" MID$ ",	"  ",
	" SYNTAX ",			" RETURN WITHOUT GOSUB ",
	" OUT OF DATA ",		" ILLEGAL QUANTITY ",
	" OVERFLOW ",			" OUT OF MEMORY ",
	" UNDEF'D STATEMENT ",		" BAD SUBSCRIPT ",
	" REDIM'D ARRAY ",		" DIVISION BY ZERO ",
	" ILLEGAL DIRECT ",		" TYPE MISMATCH ",
	" STRING TOO LONG ",		" FORMULA TOO COMPLEX ",
	" CAN'T CONTINUE ",		" UNDEF'D FUNCTION ",
	" ERROR \a",	"",		"",		""
};


// Prototipos
