# prodosutil

ProDos - Utilit�rio para manipula��o de volumes prodos
by F�bio Belavenuto - Copyright 2004

  Este utilit�rio permite a manipula��o de imagens de disco que contenham
o sistema de arquivos ProDos, permitindo a extra��o, inclus�o, remo��o
de arquivos e a visualiza��o de diret�rio.

  A linha de comando segue o formato:

  ProDos.exe <nome da imagem do disco> <comandos e op��es>

  Comandos: (Para cada comando existe op��es espec�ficas)

   -c <-1 | -s <blocos>> <-v <Volume>>
      Cria uma nova imagem

   -l [diretorio]
      Exibe o catalogo do disco

   -i [-f] [-t <Tipo[.aux]>] [-a <Nome_Apple2>] <Arquivo>
      Insere um arquivo no disco

   -o [-p] <-a <Nome_Apple2>>
      Extrai um arquivo do disco

   -d [-f] <-a <Nome_Apple2>>
      Deleta um arquivo do disco

   -m [-f] <-t <Tipo[.aux]>> <-a <Nome_Apple2>>
      Cria um arquivo ou diretorio do disco

 Op��es:
   -1               = Criar uma imagem de 140KB (formato .DSK)
   -s <blocos>      = Criar uma imagem de <blocos> blocos
   -v <Nome_Volume> = Nome do volume no formato do Apple2
   -a <Nome_Apple2> = Nome do arquivo no formato do Apple2
   -t <Tipo[.aux]>  = Tipo do arquivo com o valor auxiliar
   -p               = Processar arquivo de saida
   -f               = Forcar

 
  A op��o "-p" serve para o programa processar o arquivo na hora da
extra��o, como por exemplo "destokenizar" um arquivo BAS.

  A op��o "-f" serve para evitar que ocorra um erro caso o arquivo j�
exista ou ele n�o � achado.

  Segue abaixo a explica��o de cada comando.


Comando -c (Criar nova imagem)
------------------------------

  O comando -c permite a cria��o de um novo volume ProDos, com o setor
de boot ProDos e sem nenhum arquivo. As op��es s�o:

  -1               - Cria uma imagem no formato .DSK (140 KB).
ou
  -s <blocos>      - Cria uma imagem de <blocos> blocos, m�nimo de 10 blocos
                     e m�ximo de 65535 blocos
  -v <Nome_Volume> - Cria a imagem com esse nome de volume.

  Exemplos:

	ProDos imagem.dsk -c -1 -v MEUDISCO
	ProDos prodos.hdv -c -s 8192 -v DISCO4MB

Comando -l (Listar diret�rio)
-----------------------------

  O comando -l permite visualizar o cat�logo de um diret�rio, se n�o
for especificado um diret�rio ele mostrar� o diret�rio principal.

  Exemplos:
    
	ProDos imagem.dsk -l
	ProDos prodosutil.hdv -l examples
	ProDos meuprodos.dsk -l dir1/dir2


Comando -i (Incluir Arquivo)
----------------------------

  O comando -i permite a inclus�o de um arquivo dentro da imagem, as
op��es s�o:
  
  -t <tipo[.aux]>  - Pode ser especificado um nome de 3 letras para o tipo
                     de arquivo, uma lista completa dos tipos est� no fim
                     desse texto. Alguns tipos de arquivos devem conter um
                     valor auxiliar, dependendo do tipo, se n�o for especi-
                     ficado um valor ser� assumido o valor zero. O tipo pode
                     ser especificado por um valor decimal ou hexa. Para
                     especificar um valor coloque um ponto ap�s o tipo e
                     para valores decimais coloque-o em seguida, para valo-
                     res hexa preceda por um cifr�o ($).

  -a <NOME_APPLE2> - O nome do arquivo que ser� salvo dentro da imagem, ele
                     poder� conter o caminho completo do arquivo, caso o
                     arquivo tiver que ser salvo dentro de um diret�rio.

  <Arquivo>        - O arquivo ao qual ser� incluso dentro da imagem. Caso o
                     tipo de arquivo ProDOS e/ou o NOME_APPLE n�o tenha sido
                     especificado na linha de comando, o nome do arquivo pode
                     conter uma extens�o especificando o tipo e/ou valor auxiliar,
                     no formato: 
                     "NOME_APPLE2#(Codigo hexa do tipo)(Codigo hexa do valor_aux)".

  Exemplos:

	ProDos meuprodos.hdv -i BASIC.SYSTEM#FF2000
	ProDos imagem.dsk -i -t BIN.$6000 -a ARQUIVO arquivo.bin
	ProDos prodos.dsk -i -t TXT.512 -a TEXTO texto.txt


Comando -o (Extrair Arquivo)
----------------------------

  O comando -o permite a extra��o de arquivos da imagem, as op��es s�o:

  -a <NOME_APPLE2> - O nome do arquivo que est� dentro da imagem, ele poder�
                     conter o caminho completo do arquivo, caso o arquivo
                     estiver dentro de um diret�rio.

  -p               - Processa o arquivo, se for um arquivo BASIC o arquivo �
                     "destokenizado", sendo salvo em texto puro. Se for ar-
                     quivo TXT, o arquivo � filtrado.

  O arquivo � salvo no computador com o mesmo nome do arquivo apple2 adi-
  cionando a informacao do tipo de arquivo prodos e o valor auxiliar, no
  formato: "XXXXX#(hex tipo)(hex valor aux)".

  Exemplos:
    
	ProDos imagem.hdv -o -a PRODOS
	ProDos prodos.hdv -o -a BASIC.SYSTEM
	ProDos meuprodos.dsk -o -a HELPSCREENS


Comando -d (Deletar Arquivo)
----------------------------

  O comando -d permite a exclus�o de um arquivo da imagem, as op��es s�o:

  -a <NOME_APPLE2> - O nome do arquivo que est� dentro da imagem, ele poder�
                     conter o caminho completo do arquivo, caso o arquivo
                     estiver dentro de um diret�rio.

  Exemplos:

	ProDos imagem.dsk -d -a arquivo
	ProDos prodos.dsk -d -a SYSUTIL.SYSTEM


Comando -m (Criar Arquivo ou Diret�rio)
---------------------------------------

  O comando -m permite a cria��o de um arquivo ou diret�rio na imagem,
  as op��es s�o:

  -t <tipo[.aux]>  - Deve ser especificado um nome de 3 letras para o tipo
                     de arquivo, uma lista completa dos tipos est� no fim
                     desse texto. Alguns tipos de arquivos devem conter um
                     valor auxiliar, dependendo do tipo, se n�o for especi-
                     ficado um valor ser� assumido o valor zero. O tipo pode
                     ser especificado por um valor decimal ou hexa. Para
                     especificar um valor coloque um ponto ap�s o tipo e
                     para valores decimais coloque-o em seguida, para valo-
                     res hexa preceda por um cifr�o ($).

  -a <NOME_APPLE2> - O nome do arquivo que ser� criado dentro da imagem, ele
                     poder� conter o caminho completo do arquivo, caso o
                     arquivo tiver que ser criado dentro de um diret�rio.


Lista dos tipos de arquivos:
----------------------------

UNK BAD PCD PTX TXT PDA BIN FNT FOT BA3 DA3 WPF SOS $0D $0E DIR
RPD RPI AFD AFM AFR SCL PFS $17 $18 ADB AWP ASP $1C $1D $1E $1F
TDM IPS UPV $23 $24 $25 $26 $27 $28 3SD 8SC 8OB 8IC 8LD P8C $2F
$30 $31 $32 $33 $34 $35 $36 $37 $38 $39 $3A $3B $3C $3D $3E $3F
$40 OCR FTD $43 $44 $45 $46 $47 $48 $49 $4A $4B $4C $4D $4E $4F
GWP GSS GDB DRW GDP HMD EDU STN HLP COM CFG ANM MUM ENT DVU $5F
PRE $61 $62 $63 $64 $65 $66 $67 $68 $69 $6A BIO $6C DVR PRE HDV
$70 $71 $72 $73 $74 $75 $76 $77 $78 $79 $7A $7B $7C $7D $7E $7F
$80 $81 $82 $83 $84 $85 $86 $87 $88 $89 $8A $8B $8C $8D $8E $8F
$90 $91 $92 $93 $94 $95 $96 $97 $98 $99 $9A $9B $9C $9D $9E $9F
WP_ $A1 $A2 $A3 $A4 $A5 $A6 $A7 $A8 $A9 $AA GSB TDF BDF $AE $AF
SRC OBJ LIB S16 RTL EXE STR TSF NDA CDA TOL DRV LDF FST $BE DOC
PNT PIC ANI PAL $C4 OOG SCR CDV FON FND ICN $CB $CC $CD $CE $CF
$D0 $D1 $D2 $D3 $D4 MUS INS MDI SND $D9 $DA DBM $DC $DD $DE $DF
SHK $E1 DTS $E3 $E4 $E5 $E6 $E7 $E8 $E9 $EA $EB $EC $ED R16 PAS
CMD $F1 $F2 $F3 $F4 $F5 $F6 $F7 $F8 P16 INT IVR BAS VAR REL SYS

